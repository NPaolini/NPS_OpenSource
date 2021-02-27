//-------------------- svmObjText.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "resource.h"
#include "p_File.h"
#include "pPanel.h"
#include "pModDialog.h"
#include "svmObjText.h"
#include "svmObjEdit.h"
#include "svmDefObj.h"
#include "svmPropertyText.h"
//-----------------------------------------------------------
#define DEF_H_TEXT 20
//-----------------------------------------------------------
bool isObjText4Choose(svmObject* t)
{
  svmObjText* obj = dynamic_cast<svmObjText*>(t);
  if(!obj)
    return false;
  switch(obj->getProp()->simpleVarText) {
    case PropertyText::var_text:
    case PropertyText::var_text_value:
    case PropertyText::var_text_value_text:
      return true;
    }
  return false;
}
//-----------------------------------------------------------
bool isObjText4Edit(svmObject* t)
{
  svmObjText* obj = dynamic_cast<svmObjText*>(t);
  if(!obj)
    return false;
  switch(obj->getProp()->simpleVarText) {
    case PropertyText::var_value:
      return true;
    }
  return false;
}
//-----------------------------------------------------------
svmObject* allocObjTextByEdit(svmObject* ed, int id) { return new svmObjText(ed, id); }
svmObject* allocObjTextByChoose(svmObject* choose, int id) { return new svmObjText(choose, id); }
//-----------------------------------------------------------
static PropertyText staticPT;
//-----------------------------------------------------------
svmObjText::svmObjText(PWin* par, int id, int x1, int y1, int x2, int y2) :
  svmObject(par, id, oTEXT, x1, y1, x2, DATA_NOT_USED == y2 ? DEF_H_TEXT + y1 : y2)
{
  init();
}
//-----------------------------------------------------------
svmObjText::svmObjText(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oTEXT, r)
{
  init();
}
//-----------------------------------------------------------
svmObjText::svmObjText(svmObject* other, int id) :
  svmObject(other->getParent(), id, oTEXT, other->getRect())
{
  Prop = new PropertyText;
  *Prop = *other->getProp();
  Prop->style = Property::UP_FILL;
  const svmObjEdit* po = dynamic_cast<const svmObjEdit*>(other);
  if(po)
    Prop->simpleVarText = PropertyText::var_value;
}
//-----------------------------------------------------------
void svmObjText::init()
{
  Prop = new PropertyText;
  PropertyText* po = dynamic_cast<PropertyText*>(Prop);
  po->setText(_T("Txt"));
  Prop->Rect = getFirstRect();
  Prop->style = Property::UP_FILL;
}
//-----------------------------------------------------------
void svmObjText::DrawObject(HDC hdc)
{
  PPanel panel(getRect(), Prop->background, (PPanel::bStyle)Prop->style);
  panel.setAllBorder(Prop->Border);
  panel.draw(hdc);
}
//-----------------------------------------------------------
LPCTSTR svmObjText::getObjName() const
{
  PropertyText* pt = dynamic_cast<PropertyText*>(Prop);
  if(PropertyText::simple == pt->simpleVarText)
    return get_objName(oSIMPLE_TEXT);
  return baseClass::getObjName();
}
//-----------------------------------------------------------
void svmObjText::fillTips(LPTSTR tips, int size)
{
  PropertyText* pt = dynamic_cast<PropertyText*>(Prop);
  if(PropertyText::simple == pt->simpleVarText) {
    wsprintf(tips, _T("Obj -> %s, id=%d, zOrder=%d"), getObjName(), getId(), zOrder);
    addTipsVis(tips, size);
    }
  else
    baseClass::fillTips(tips, size);
}
//-----------------------------------------------------------
static bool addOffs(LPTSTR buff, LPCTSTR source, int offs)
{
  LPCTSTR p = source;
  while(*p && !_istdigit((unsigned)*p))
    ++p;
  if(!*p)
    return false;

  copyStrZ(buff, source, p - source);
  int v = _ttoi(p) + offs;
  while(*p && _istdigit((unsigned)*p))
    ++p;
  LPTSTR ppre = buff + _tcslen(buff);
  wsprintf(ppre, _T("%d%s"), v, p);
  return true;
}
//-----------------------------------------------------------
void svmObjText::setOffsetLang(const infoOffsetReplace& ior)
{
  PropertyText* pt = (PropertyText*)Prop;
  pvvChar source;
  uint nElem = splitParam(source,  pt->getText() + 1, _T('§'));
  pvvChar target;
  target.setDim(MAX_LANG);
  nElem = min(nElem, MAX_LANG);
  for(uint i = 0; i < nElem; ++i) {
    TCHAR buff[4096] = _T("");
    if(addOffs(buff, &source[i], ior.offsTxt))
      target[i].append(buff, _tcslen(buff));
    }
  pvChar result;
  unionParam(result, target,  _T('§'));
  result.insert(_T('§'), 0);
  pt->setText(&result);
  invalidate();
}
//-----------------------------------------------------------
void svmObjText::setOffsetAddr(const infoOffsetReplace& ior)
{
  baseClass::setOffsetAddr(ior);
  PropertyText* pt = (PropertyText*)Prop;
  if(!pt->isSimpleText())
    return;

  if(!isSetBitf(eoUseSimpleText, ior.flag))
    return;
  if(_tcschr(pt->getText(), _T('§')))
    setOffsetLang(ior);
  else {
    TCHAR buff[4096] = _T("");
    if(addOffs(buff, pt->getText(), ior.offsTxt))
      pt->setText(buff);
    }
  invalidate();
}
//-----------------------------------------------------------
void svmObjText::setCommonProperty(const Property* prop, DWORD bits, LPCTSTR simpleText)
{
  baseClass::setCommonProperty(prop, bits, simpleText);
  PropertyText* pt = (PropertyText*)Prop;
  if(!pt->isSimpleText())
    return;
  if(simpleText && (1 << CHANGED_SIMPLE_TEXT) & bits)
    pt->setText(simpleText);
}
//-----------------------------------------------------------
extern void makeTitleHex(LPTSTR buff, size_t dim);
extern void makeTitleBinary(LPTSTR buff, size_t dim);
//-----------------------------------------------------------
smartPointerConstString svmObjText::getTitle() const
{
  PropertyText* pt = dynamic_cast<PropertyText*>(Prop);
  if(PropertyText::var_value == pt->simpleVarText) {
    static TCHAR buff[40];// = _T("0.1234567890");
    int nDec = pt->nDec;
    if(nDec > 6)
      nDec = 6;

    if(Prop->typeVal != 5 && Prop->typeVal != 7 && Prop->BaseNum) {
      switch(Prop->BaseNum) {
        case ebBinary:
          makeTitleBinary(buff, SIZE_A(buff));
          break;
        case ebHex:
          makeTitleHex(buff, SIZE_A(buff));
          break;
        }
      }
    else {
      if(nDec) {
        _tcscpy_s(buff, _T("0.1234567890"));
        buff[nDec + 2] = 0;
        }
      else
        _tcscpy_s(buff, _T("1234"));
      }
    return smartPointerConstString(buff, 0);
    }
  if(pt->forTitle) {
    smartPointerConstString sp(_T("*** T I T L E ***"), false);
    return sp;
    }
  smartPointerConstString sp = getStringByLangSimple(pt->getText());
  return sp;
}
//-----------------------------------------------------------
Property* svmObjText::allocProperty() { return new PropertyText; }
//-----------------------------------------------------------
PRect svmObjText::getRectTitle(HDC, LPCTSTR /*title*/) const
{
#if 1
  return Prop->Rect;
#else
  PRect r(Prop->Rect);
  r.Inflate(-2, 0);
  return r;
#endif
}
//-----------------------------------------------------------
svmObject* svmObjText::makeClone()
{
  svmObjText* obj = new svmObjText(getParent(), getId(), getRect());
//  if(oldProp)
//    *obj->Prop = *oldProp;
//  else
    *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjText::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int svmObjText::getIdType() const
{
  if((dynamic_cast<PropertyText*>(Prop))->isSimpleText())
    return oSIMPLE_TEXT;
  return baseClass::getIdType();
}
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjText::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyText*>(Prop))->cloneMinusProperty(staticPT);
      }
    else {
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      }
    }
  else {
    if(Prop->normaliz < -2)
      Prop->normaliz = -Prop->normaliz;
    else if(Prop->normaliz <= 2)
      Prop->normaliz = 0;

    if(!oldProp)
      oldProp = allocProperty();
    *oldProp = *Prop;
    if(IDOK == svmDialogText(this, Prop, getParent()).modal()) {
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
void svmObjText::makeDef4Cam(uint prf, uint addr)
{
  PropertyText* pt = dynamic_cast<PropertyText*>(Prop);
  if(!pt)
    return;
  pt->simpleVarText = PropertyText::var_value;
  Prop->perif = prf;
  Prop->addr = addr;
  Prop->typeVal = 10;
}
//-----------------------------------------------------------
namespace prfData
{
  enum { tNegative = (1 << 30) };
}
//-----------------------------------------------------------
bool svmObjText::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, oSIMPLE_TEXT == getIdType() ? ID_INIT_SIMPLE_TXT : ID_INIT_VAR_TXT);
  uint id = moi.calcAndSetExtendId(order);
  PropertyText* pt = dynamic_cast<PropertyText*>(Prop);
  if(!pt)
    return false;

  TCHAR buff[1000];
  int style = getStyle();

  int align = Prop->alignText;

  int idMsgOrColor = 0;

  svmObjCount& objCount = getObjCount();

  if(oSIMPLE_TEXT == getIdType()) {
    if(pt->forTitle)
      idMsgOrColor = 1;
    else {
      LPCTSTR txt = pt->getText();
      idMsgOrColor = objCount.getGlobalIdCount();
      wsprintf(buff, _T("%d,%s\r\n"), idMsgOrColor, txt);
      writeStringChkUnicode(pfClear, pfCrypt, buff);
      }
    }

  else {
    const setOfPTextColor& set = pt->getSet();
    int nElem = set.getElem();
    // se è testo variabile il normalizzatore deve essere negativo e al
    // posto dei decimali viene inserito l'id del primo testo
    if(PropertyText::var_text == pt->simpleVarText) {
      if(nElem > 0) {
        // deve lasciare un buco per terminare la lista dei testi
        int idMsg = objCount.getGlobalIdCount(nElem + 1);
        if(pt->usingColor())
          idMsgOrColor = objCount.getGlobalIdCount(nElem + 1);
        pt->normaliz = -1;
        pt->nDec = idMsg;
        for(int i = 0; i < nElem; ++i) {
          const textAndColor* tac = set[i];
          wsprintf(buff, _T("%d,%s\r\n"), idMsg + i, tac->text);
          writeStringChkUnicode(pfClear, pfCrypt, buff);

          if(pt->usingColor()) {
            wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d\r\n"),
                  idMsgOrColor + i,
                  GetRValue(tac->fgColor),
                  GetGValue(tac->fgColor),
                  GetBValue(tac->fgColor),

                  GetRValue(tac->bgColor),
                  GetGValue(tac->bgColor),
                  GetBValue(tac->bgColor));
            writeStringChkUnicode(pfCrypt, buff);
            }
          }
        }
      }
    else if(PropertyText::var_text_value == pt->simpleVarText || PropertyText::var_text_value_text == pt->simpleVarText) {
      if(nElem > 0) {
        int idMsg = objCount.getGlobalIdCount(nElem + 1);
        idMsgOrColor = objCount.getGlobalIdCount();
        int idColors = objCount.getGlobalIdCount(nElem + 1);

        if(pt->normaliz <= 2)
          pt->normaliz = -2;
        else
          pt->normaliz = -pt->normaliz;
        pt->nDec = idMsg;

        wsprintf(buff, _T("%d,%d\r\n"), idMsgOrColor, nElem);
        writeStringChkUnicode(pfCrypt, buff);

        for(int i = 0; i < nElem; ++i) {
          const textAndColor* tac = set[i];

          wsprintf(buff, _T("%d,%s\r\n"), idMsg + i, tac->text);
          writeStringChkUnicode(pfClear, pfCrypt, buff);

          _stprintf_s(buff, SIZE_A(buff), _T("%d,%f,%d,%d,%d,%d,%d,%d\r\n"),
              idColors + i,
              tac->value,
              GetRValue(tac->fgColor),
              GetGValue(tac->fgColor),
              GetBValue(tac->fgColor),

              GetRValue(tac->bgColor),
              GetGValue(tac->bgColor),
              GetBValue(tac->bgColor));
          writeStringChkUnicode(pfCrypt, buff);
          }

        if(PropertyText::var_text_value_text == pt->simpleVarText)
          idMsgOrColor = -idMsgOrColor;
        }
      }

      // se non è testo variabile il normalizzatore non deve essere negativo e
      // idColor identifica i valori più i colori
    else if(pt->usingColor()) {
      if(nElem > 0) {
        idMsgOrColor = objCount.getGlobalIdCount();
        int idColors = objCount.getGlobalIdCount(nElem + 1);
        wsprintf(buff, _T("%d,%d\r\n"), idMsgOrColor, nElem);
        writeStringChkUnicode(pfCrypt, buff);

        for(int i = 0; i < nElem; ++i) {
          const textAndColor* tac = set[i];

          _stprintf_s(buff, _T("%d,%f,%d,%d,%d,%d,%d,%d\r\n"),
              idColors + i,
              tac->value,
              GetRValue(tac->fgColor),
              GetGValue(tac->fgColor),
              GetBValue(tac->fgColor),

              GetRValue(tac->bgColor),
              GetGValue(tac->bgColor),
              GetBValue(tac->bgColor));
          writeStringChkUnicode(pfCrypt, buff);
          }
        }
      }
    }

  uint useAltText = 0;
  alternateTextSet& at = pt->getAltObj();

  if(at.getFileName() || at.getPrph()) {
    useAltText = objCount.getGlobalIdCount(1);
    at.save(pfCrypt, useAltText);
    }

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"),
            id,
            REV__X(Prop->Rect.left),
            REV__X(Prop->Rect.top),
            REV__X(Prop->Rect.Width()),
            REV__X(Prop->Rect.Height()),

            Prop->idFont + ID_INIT_FONT,

            GetRValue(Prop->foreground),
            GetGValue(Prop->foreground),
            GetBValue(Prop->foreground),

            GetRValue(Prop->background),
            GetGValue(Prop->background),
            GetBValue(Prop->background),

            style,

            align,
            idMsgOrColor);
  writeStringChkUnicode(pfCrypt, buff);
  Prop->saveBorder(buff, SIZE_A(buff));
  writeStringChkUnicode(pfCrypt, buff);
  if(useAltText) {
    wsprintf(buff, _T(",%d\r\n"), useAltText);
    writeStringChkUnicode(pfCrypt, buff);
    }
  else
    writeStringChkUnicode(pfCrypt, _T("\r\n"));

  if(oSIMPLE_TEXT != getIdType()) {
    if(Prop->negative)
      Prop->typeVal |= prfData::tNegative;
    saveVar(pfCrypt, id, pt->getBlinkTime());
    }
  else //if(Prop->visPerif)
    saveOnlyVisVar(pfCrypt, id, pt->getBlinkTime());

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
bool svmObjText::load(uint id, setOfString& set)
{
  PropertyText* pt = dynamic_cast<PropertyText*>(Prop);
  if(!pt)
    return false;
  manageObjId moi(id);
  bool simple = ID_INIT_SIMPLE_TXT == moi.getBaseId();
//  bool simple = ID_INIT_SIMPLE_TXT <= id && id < ID_INIT_SIMPLE_TXT + ADD_INIT_VAR * 2;
  if(simple)
    pt->simpleVarText = PropertyText::simple;
  else
    pt->simpleVarText = PropertyText::var_value;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = ID_INIT_FONT;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int no_up_dn = 0;
  int align = 0;
  int idColor = 0;

  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &no_up_dn, &align, &idColor);

  p = findNextParamTrim(p, 14);
  Prop->loadBorder(p);
  // codificato a mano :-( purtroppo, per adesso lasciamo così
  p = findNextParamTrim(p, 21);
  if(p) {
    uint useAltText = _ttoi(p);
    if(useAltText) {
      alternateTextSet& at = pt->getAltObj();
      at.load(set, useAltText);
      }
    }
  idfont -= ID_INIT_FONT;

  PPanel::bStyle type = no_up_dn == 0 ? PPanel::TRANSP  :
                        no_up_dn == 1 ? PPanel::UP_FILL :
                        no_up_dn == 2 ? PPanel::DN_FILL :
                        no_up_dn == 3 ? PPanel:: FILL   :

                        no_up_dn == 4 ? PPanel::UP_TRANSP :
                        no_up_dn == 5 ? PPanel::DN_TRANSP :
                        no_up_dn == 6 ? PPanel::BORDER_FILL :
                                        PPanel::BORDER_TRANSP;
  Prop->foreground = RGB(Rfg, Gfg, Bfg);
  Prop->background = RGB(Rbk, Gbk, Bbk);
  Prop->style = type;
  Prop->idFont = idfont;
  Prop->alignText = align;
  Prop->Rect = PRect(0, 0, R__X(w), R__Y(h));
  Prop->Rect.MoveTo(R__X(x), R__Y(y));

  if(simple) {
    if(1 == idColor)
      pt->forTitle = true;
    else {
      p = set.getString(idColor);
      if(p) {
        LPTSTR buff = new TCHAR[_tcslen(p) + 2];
        translateFromCRNL(buff, p);
        pt->setText(buff);
        delete []buff;
        }
      }
    uint tick = 0;
    loadOnlyVisVar(id, set, &tick);
    pt->setBlinkTime(tick);
    }
  else {
    uint tick = 0;
    loadVar(id, set, &tick);
    pt->setBlinkTime(tick);
    if(Prop->typeVal & prfData::tNegative) {
      Prop->negative = true;
      Prop->typeVal &= ~prfData::tNegative;
      }
    setOfPTextColor& sTC = pt->getSet();
    flushPV(sTC);
    pt->setUseColor(toBool(idColor));
    if(-1 == Prop->normaliz) {
//    if(Prop->normaliz < 0) {
      pt->simpleVarText = PropertyText::var_text;
      int idMsg = Prop->nDec;
      int i;
      for(i = 0; ; ++i) {
        p = set.getString(idMsg + i);
        if(!p)
          break;
        TCHAR buff[500];
        translateFromCRNL(buff, p);
        textAndColor* tac = new textAndColor(buff);
        if(idColor) {
          p = set.getString(idColor + i);
          if(p) {
            _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk);
            tac->fgColor = RGB(Rfg, Gfg, Bfg);
            tac->bgColor = RGB(Rbk, Gbk, Bbk);
            }
          }
        sTC[i] = tac;
        }
      if(i)
        pt->setText(sTC[0]->text);
      else
        pt->setText(_T("Esempio"));
      }
    else if(Prop->normaliz < -1) {
      if(idColor < 0) {
        idColor = -idColor;
        pt->simpleVarText = PropertyText::var_text_value_text;
        }
      else
        pt->simpleVarText = PropertyText::var_text_value;
      int idMsg = Prop->nDec;
      int i = 0;
      if(Prop->normaliz >= -2)
        Prop->normaliz = 0;
      else
        Prop->normaliz = -Prop->normaliz;

      p = set.getString(idColor);
      if(p) {
        int nElem = _ttoi(p);

        for(i = 0; i < nElem; ++i) {
          p = set.getString(idMsg + i);
          if(!p)
            break;
          TCHAR buff[500];
          translateFromCRNL(buff, p);
          p = set.getString(idColor + i + 1);
          if(!p)
            break;
          textAndColor* tac = new textAndColor(buff);
          tac->value = (float)_tstof(p);
          p = findNextParam(p, 1);
          _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk);
          tac->fgColor = RGB(Rfg, Gfg, Bfg);
          tac->bgColor = RGB(Rbk, Gbk, Bbk);
          sTC[i] = tac;
          }
        }
      if(i)
        pt->setText(sTC[0]->text);
      else
        pt->setText(_T("Esempio"));
      }
    else {
      p = set.getString(idColor);
      if(p) {
        int nElem = _ttoi(p);

        for(int i = 0; i < nElem; ++i) {
          p = set.getString(idColor + i + 1);
          if(!p)
            break;
          textAndColor* tac = new textAndColor;
          tac->value = (float)_tstof(p);
          p = findNextParam(p, 1);
          if(!p)
            break;
          _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"),
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk);
          tac->fgColor = RGB(Rfg, Gfg, Bfg);
          tac->bgColor = RGB(Rbk, Gbk, Bbk);
          sTC[i] = tac;
          }
        }
      pt->setText(_T("Esempio"));
      }
    }
  return true;
}


