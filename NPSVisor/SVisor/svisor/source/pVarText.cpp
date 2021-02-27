//------ PVarText.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "1.h"
#include "PVarText.h"
#include "p_basebody.h"
#include "p_util.h"
#include "pVisualTextEdit.h"
//----------------------------------------------------------------------------
int PVarText::useSeparator = -1;
//----------------------------------------------------------------------------
void loadBorder(LPCTSTR p, PPanelBorder& Border);
//----------------------------------------------------------------------------
PVarText::PVarText(P_BaseBody* owner, uint id) :  baseVar(owner, id),
    Text(0), idColor(0), TypeShow(tsNorm), idTextOrDec(0), useZeroPad(0),
    AlternateTextFile(0), TextVar(0)
{
  if(useSeparator < 0) {
    LPCTSTR p = getString(USE_NUMBER_SEPARATOR);
    useSeparator = p && _ttoi(p);
    }
}
//----------------------------------------------------------------------------
PVarText::~PVarText()
{
  delete Text;
  delete []AlternateTextFile;
  delete TextVar;
}
//----------------------------------------------------------------------------
bool PVarText::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = 0;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int no_up_dn = 0;
  int align = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &no_up_dn, &align, &idColor);

  p = findNextParamTrim(p, 14);
  PPanelBorder border;
  loadBorder(p, border);
  // codificato a mano :-( purtroppo, per adesso lasciamo così
  p = findNextParamTrim(p, 21);
  if(p) {
    uint useAltText = _ttoi(p);
    while(useAltText) {
      p = getOwner()->getPageString(useAltText);
      if(!p)
        break;
      uint prph = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      if(!prph)
        AlternateTextFile = str_newdup(p);
      else {
        uint addr = _ttoi(p);
        p = findNextParamTrim(p);
        if(!p)
          break;
        uint dim = _ttoi(p);
        TextVar = new pVariable(prph, addr, prfData::tStrData, 0, 0, 0, dim);
        }
      break;
      }
    }
  idfont -= ID_INIT_FONT;

  PPanel::bStyle type = no_up_dn == 0 ? PPanel::NO  :
                        no_up_dn == 1 ? PPanel::UP_FILL :
                        no_up_dn == 2 ? PPanel::DN_FILL :
                        no_up_dn == 3 ? PPanel:: FILL   :

                        no_up_dn == 4 ? PPanel::UP :
                        no_up_dn == 5 ? PPanel::DN :
                        no_up_dn == 6 ? PPanel::BORDER_FILL :
                                        PPanel::BORDER;

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  HFONT font = getOwner()->getFont(idfont);

  Text = new PTextFixedPanel(0, rect, font, RGB(Rfg, Gfg, Bfg), type, RGB(Rbk, Gbk, Bbk), PTextPanel::NO3D);
  Text->setAllBorder(border);
#if 1
  enum  { aCenter, aLeft, aRight, aTop = 4, aMid = 8, aBottom = 16 };

  UINT vAlign = align & ~3;
  align &= 3;

  vAlign = aBottom == (vAlign & aBottom) ? DT_BOTTOM :
         aMid == (vAlign & aMid) ? DT_VCENTER :
                                     DT_TOP;

  align = aLeft == (align & aLeft) ? TA_LEFT :
         aRight == (align & aRight) ? TA_RIGHT :
                                     TA_CENTER;

  Text->setAlign(align);
  Text->setVAlign(vAlign);
#else
  Text->setAlign(TA_TOP | (!align ? TA_CENTER : 1 == align ? TA_LEFT : TA_RIGHT));
#endif
  if(!makeStdVars())
    return false;

  do {
    uint id2 = getFirstExtendId();
    p = getOwner()->getPageString(id2);
    p = findNextParamTrim(p, 5);
    if(p) {
      DWORD tick = _ttoi(p);
      if(tick)
        allocBlink(tick);
      }
    } while(false);
  idTextOrDec = BaseVar.getDec();

  if(idColor) {
    if(-1 == BaseVar.getNorm()) {
      BaseVar.setNorm(0);
      Colors.makeColors(getOwner(), idColor);
      TypeShow = tsIxColor;
      }
    else  {
      if(-2 >= BaseVar.getNorm()) {
        BaseVar.setDec(0);
        if(idColor < 0) {
          idColor = -idColor;
          TypeShow = tsValColorAndTextExact;
          }
        else
          TypeShow = tsValColorAndText;
        if(-2 == BaseVar.getNorm())
          BaseVar.setNorm(0);
        else
          BaseVar.setNorm(-BaseVar.getNorm());
        }
      else
        TypeShow = tsValColor;
      VarColors.makeColors(getOwner(), idColor);
      }
    }
  else if(-1 == BaseVar.getNorm()) {
    BaseVar.setNorm(0);
    TypeShow = tsIxText;
    }

  bool usePad = tsNorm == TypeShow && BaseVar.getDec() && !BaseVar.getNorm() && ebTen == BaseVar.getBaseNum();
  if(usePad) {
    switch( BaseVar.getType()) {
      case prfData::tBData:
      case prfData::tWData:
      case prfData::tBitData:
      case prfData::tDWData:

      case prfData::tBsData:
      case prfData::tWsData:
      case prfData::tDWsData:
        break;
      default:
        usePad = false;
      }
    }
  if(usePad) {
    useZeroPad = BaseVar.getDec();
    BaseVar.setDec(0);
    }
  return true;
}
//----------------------------------------------------------------------------
bool PVarText::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible())
    return upd;
  force |= upd;

  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }

  if(!force && prfData::notModified == result)
    return upd;

  COLORREF oldfg = Text->getTextColor();
  COLORREF oldbg = Text->getColor();

  TCHAR oldbuff[4096 * 4];
  _tcscpy_s(oldbuff, Text->getText());

  VarColors.resetLastIx();
  int needCheckAltern = -1;
  TCHAR buff[4096 * 4] = _T("\0");
  if(tsValColorAndText == TypeShow) {
    COLORREF fg;
    COLORREF bg;
    if(VarColors.getColors(fg, bg, BaseVar.getNormalizedResult(data), getOwner(), idTextOrDec, buff, SIZE_A(buff))) {
      Text->setTextColor(fg);
      Text->setBkgColor(bg);
      }
    }
  else if(tsValColorAndTextExact == TypeShow) {
    COLORREF fg;
    COLORREF bg;
    VarColors.getExactColors(fg, bg, BaseVar.getNormalizedResult(data), getOwner(), idTextOrDec, buff, SIZE_A(buff));
    Text->setTextColor(fg);
    Text->setBkgColor(bg);
    }
  else {
    BaseVar.makeString(buff, SIZE_A(buff), data);
    switch(TypeShow) {
      case tsValColor:
        do {
          COLORREF fg;
          COLORREF bg;
          if(VarColors.getColors(fg, bg, BaseVar.getNormalizedResult(data))) {
            Text->setTextColor(fg);
            Text->setBkgColor(bg);
            }
          } while(false);
        break;
      case tsNorm:
        break;

      case tsIxColor:
      case tsIxText:
        do {
          int ix = BaseVar.getIxResult(data);
          if(tsIxColor == TypeShow) {
            COLORREF fg;
            COLORREF bg;
            if(Colors.getColors(fg, bg, ix)) {
              Text->setTextColor(fg);
              Text->setBkgColor(bg);
              }
            }
          smartPointerConstString sp = getOwner()->getPageStringLang(idTextOrDec + ix);
          needCheckAltern = ix;
          if(sp)
            _tcscpy_s(buff, SIZE_A(buff), sp);
          } while(false);
        break;
      }
    }
  int lastIx = VarColors.getLastIx();
  if(lastIx < 0)
    lastIx = needCheckAltern;
  if(lastIx >= 0)
    getAlternateText(buff, SIZE_A(buff), lastIx);

  int type = BaseVar.getType();
  if(useZeroPad) {
    int len = _tcslen(buff);
    if(len >= useZeroPad)
      Text->setText(buff);
    else {
      TCHAR t[64] = { _T('0') };
      fillStr(t, _T('0'), SIZE_A(t) - 1);
      t[useZeroPad] = 0;
      int dim = useZeroPad - len;
      --len;
      for(int i = useZeroPad - 1; i >= dim; --i, --len)
        t[i] = buff[len];
      Text->setText(t);
      }
    }
  else {
    if(useSeparator)
      if(type !=  prfData::tStrData && BaseVar.getDec() && BaseVar.getNorm())
        type = prfData::tFRData;
    switch(type) {
      case prfData::tBData:
      case prfData::tWData:
      case prfData::tBitData:
      case prfData::tDWData:

      case prfData::tBsData:
      case prfData::tWsData:
      case prfData::tDWsData:
        if(tsNorm == TypeShow || tsValColor == TypeShow) {
          TCHAR t[1024] = { 0 };
          switch(BaseVar.getBaseNum()) {
            case ebTen:
              if(useSeparator) {
                TCHAR point[8];
                getLocaleThousandSep(point, SIZE_A(point));
                pVisualTextEditSep::addSep(t, buff, point[0], 3);
                }
              else
                _tcscpy_s(t, buff);
              break;
            case ebBinary:
              pVisualTextEditSep::addSep(t, buff, _T('.'), 4);
              break;
            case ebHex:
              pVisualTextEditSep::addSep(t, buff, _T('-'), 2);
              break;
            }
          Text->setText(t);
          }
        else
          Text->setText(buff);
        break;
      case prfData::tFRData:
      case prfData::tRData:
        if(useSeparator && (tsNorm == TypeShow || tsValColor == TypeShow)) {
          replaceWithComma(buff);
          TCHAR t[1024] = { 0 };
          TCHAR point[8];
          getLocaleDecimalPoint(point, SIZE_A(point));
          TCHAR dec = point[0];
          getLocaleThousandSep(point, SIZE_A(point));
          pVisualTextEditSep::addSep(t, buff, point[0], dec, 3);
          Text->setText(t);
          }
        else
          Text->setText(buff);
        break;
      default:
        Text->setText(buff);
        break;
      }
    }

  if(oldfg != Text->getTextColor() || oldbg != Text->getColor() || _tcscmp(Text->getText(), oldbuff))
    invalidate();
  return true;
}
//----------------------------------------------------------------------------
bool PVarText::getAlternateText(LPTSTR buff, uint sz, uint ix)
{
  if(AlternateTextFile) {
    setOfString set(AlternateTextFile);
    LPCTSTR p = set.getString(ix + 1);
    if(!p)
      return false;
    smartPointerConstString sp = getStringByLangSimple(p);
    _tcscpy_s(buff, sz, &sp);
    return true;
    }
  if(!TextVar)
    return false;
  prfData data;
  prfData::tResultData result = TextVar->getData(getOwner(), data, getOffs());
  if(prfData::failed >= result)
    return false;
  TCHAR t[_MAX_PATH];
  copyStrZ(t, data.U.str.buff, data.U.str.len);
  AlternateTextFile = t;
  bool success = getAlternateText(buff, sz, ix);
  AlternateTextFile = 0;
  return success;
}
//----------------------------------------------------------------------------
