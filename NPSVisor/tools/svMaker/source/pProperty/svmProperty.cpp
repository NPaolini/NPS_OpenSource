//---------------- svmProperty.cpp --------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmProperty.h"
#include "svmObject.h"
#include "pListbox.h"
#include "p_util.h"
#include "sizer.h"
#include "macro_utils.h"
#include "svmNormal.h"
#include "svmDefObj.h"
#include "p_txt.h"
#include "p_file.h"
#include "common.h"
#include "svmDataProject.h"
//-----------------------------------------------------------
void makeConstValue(LPTSTR buff, uint sz, DWORD value, uint prph, uint type)
{
  if(PRPH_4_CONST == prph && 5 == type) {
    float f = *(float*)&value;
    _stprintf_s(buff, sz, _T("%0.5f"), f);
    zeroTrim(buff);
    }
  else
    wsprintf(buff, _T("%d"), value);
}
//-----------------------------------------------------------
void setConstValue(HWND hwedit, DWORD value, uint prph, uint type)
{
  TCHAR t[128];
  makeConstValue(t, SIZE_A(t), value, prph, type);
  SetWindowText(hwedit, t);
}
//-----------------------------------------------------------
#if 1
static bool fullCloneOnCreate_ = true;
#else
bool Property::fullCloneOnCreate = true;
#endif
//bool Property::useRealCoord = false;
//bool Property::fullCloneOnCreate = false;
//-----------------------------------------------------------
bool Property::getFullCloneOnCreate() { return  fullCloneOnCreate_; }
//-----------------------------------------------------------
void Property::clone(const Property& other)
{
  CLONE(foreground);
  CLONE(background);
  CLONE(style);
  CLONE(Rect);
  CLONE(type1);
  CLONE(type2);
  CLONE(perif);
  CLONE(addr);
  CLONE(typeVal);
  CLONE(nBits);
  CLONE(nDec);
  CLONE(offset);
  CLONE(negative);
  CLONE(normaliz);
  CLONE(idFont);
  CLONE(alignText);

  CLONE(visPerif);
  CLONE(visAddr);
  CLONE(visnBits);
  CLONE(visOffset);
  CLONE(visNegative);

  CLONE(pswLevel);
  CLONE(Border);
  CLONE(BaseNum);
  CLONE(MoveInfo);
}
//-----------------------------------------------------------
static LPCTSTR getInt(LPCTSTR p, int& v)
{
  v = _ttoi(p);
  return findNextParamTrim(p);
}
//-----------------------------------------------------------
static LPCTSTR getColor(LPCTSTR p, COLORREF& v)
{
  int r = 255;
  int g = 0;
  int b = 0;
  while(p) {
    p = getInt(p, r);
    if(!p)
      break;
    p = getInt(p, g);
    if(!p)
      break;
    p = getInt(p, b);
    break;
    }
  v = RGB(r, g, b);
  return p;
}
//-----------------------------------------------------------
static void loadBorder(PPanelBorder& Border, LPCTSTR p)
{
  while(p) {
    int advanced;
    p = getInt(p, advanced);
    for(uint i = 0; i <= PPanelBorder::pbLeft; ++i) {
      if(!p)
        break;
      int style;
      int tickness;
      COLORREF color;
      p = getInt(p, style);
      if(!p)
        break;
      p = getInt(p, tickness);
      if(!p)
        break;
      p = getColor(p, color);
      PPanelBorder::pbInfo info;
      info.style = (PPanelBorder::estyle)style;
      info.tickness = tickness;
      info.color = color;
      Border.setBorder((PPanelBorder::eside)i, info);
      }
    // va messo in fondo perché il settaggio dei lati automaticamente toglie il simple
    // oppure bisognerebbe calcolarlo dai vari parametri (meglio di no, si possono mantenere le impostazioni)
    Border.setSimple(!advanced);
    break;
    }
}
//-----------------------------------------------------------
static void saveBorder(const PPanelBorder& Border, LPTSTR t, size_t dim)
{
  TCHAR buff[1024];
  const PPanelBorder::pbInfo& b1 = Border.getInfo(PPanelBorder::pbTop);
  const PPanelBorder::pbInfo& b2 = Border.getInfo(PPanelBorder::pbRight);
  const PPanelBorder::pbInfo& b3 = Border.getInfo(PPanelBorder::pbBottom);
  const PPanelBorder::pbInfo& b4 = Border.getInfo(PPanelBorder::pbLeft);
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
    !Border.isSimple(),
    b1.style, b1.tickness, GetRValue(b1.color), GetGValue(b1.color), GetBValue(b1.color),
    b2.style, b2.tickness, GetRValue(b2.color), GetGValue(b2.color), GetBValue(b2.color),
    b3.style, b3.tickness, GetRValue(b3.color), GetGValue(b3.color), GetBValue(b3.color),
    b4.style, b4.tickness, GetRValue(b4.color), GetGValue(b4.color), GetBValue(b4.color)
    );
  _tcscpy_s(t, dim, buff);
}
//-----------------------------------------------------------
void Property::loadBorder(LPCTSTR p)
{
  ::loadBorder(Border, p);
}
//-----------------------------------------------------------
void Property::saveBorder(LPTSTR t, size_t dim)
{
  ::saveBorder(Border, t, dim);
}
//-----------------------------------------------------------
void Property::setAlign(align which)
{
  alignText = which;
/*
  if(DT_VCENTER == (DT_VCENTER & align))
    alignText |= DT_SINGLELINE;
  else
    alignText &= ~DT_SINGLELINE;
*/
}
//-----------------------------------------------------------
HFONT Property::getFont(bool linked)
{
  if(linked) {
    svmFont& set = getFontObjLinked();
    return set.getFont(idFont);
    }
  svmFont& set = getFontObj();
  return set.getFont(idFont);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void dataPrf::clone(const dataPrf& other)
{
  CLONE(perif);
  CLONE(addr);
  CLONE(typeVal);
  CLONE(normaliz);
  CLONE(dec);
}
//-----------------------------------------------------------
void savePrf(P_File& pf, LPTSTR buff, int id, const dataPrf& data)
{
  manageObjId moi(id);
  id = moi.getFirstExtendId();
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d\r\n"),
            id,
            data.perif,
            data.addr,
            data.typeVal,
            data.normaliz,
            data.dec
            );
  writeStringChkUnicode(pf, buff);
}
//-----------------------------------------------------------
void loadPrf(uint id, setOfString& set, dataPrf& data)
{
  manageObjId moi(id);
  id = moi.getFirstExtendId();
  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"),
            &data.perif,
            &data.addr,
            &data.typeVal,
            &data.normaliz,
            &data.dec
            );
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmChooseBitmap::svmChooseBitmap(PVect<LPCTSTR>& nameBmp, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), NameBmp(nameBmp)
{
}
//-----------------------------------------------------------
svmChooseBitmap::~svmChooseBitmap()
{
  destroy();
}
//-----------------------------------------------------------
bool svmChooseBitmap::create()
{
  if(!baseClass::create())
    return false;
  fill();
  return true;
}
//-----------------------------------------------------------
LRESULT svmChooseBitmap::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_BUTTON_ADD_BMP:
          add();
          break;
        case IDC_BUTTON_REM_BMP:
          rem();
          break;
        case IDC_BUTTON_UP_BMP:
        case IDC_BUTTON_DN_BMP:
          move(IDC_BUTTON_UP_BMP == LOWORD(wParam));
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmChooseBitmap::CmOk()
{
  flushPAV(NameBmp);

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_BMP);
  int count = SendMessage(hlb, LB_GETCOUNT, 0 ,0);
  if(count > 0) {
    NameBmp.setDim(count);
    TCHAR buff[_MAX_PATH];
    for(int i = 0; i < count; ++i) {
      SendMessage(hlb, LB_GETTEXT, i, (LPARAM)buff);
      NameBmp[i] = str_newdup(buff);
      }
    }
  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmChooseBitmap::fill()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_BMP);
  SendMessage(hlb, LB_RESETCONTENT, 0, 0);
  int nElem = NameBmp.getElem();
  for(int i = 0; i < nElem; ++i)
    SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)NameBmp[i]);
}
//-----------------------------------------------------------
void svmChooseBitmap::add()
{
#if 1
  LPTSTR file = 0;
  if(openFileImageWithCopy(*this, file, true)) {
    TCHAR path[_MAX_PATH];
    LPCTSTR p = getOnlySubImagePath(file);
    if(p)
      _tcscpy_s(path, p);
    else
      *path = 0;
    int len = _tcslen(path);

    p = file + _tcslen(file) + 1;
    HWND hlb = GetDlgItem(*this, IDC_LISTBOX_BMP);
    while(*p) {
      if(len) {
        path[len] = 0;
        appendPath(path, p);
        SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)path);
        }
      else
        SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)p);
      p += _tcslen(p) + 1;
      }
    delete []file;
    }
#else
  TCHAR file[_MAX_PATH];
  if(openFileImageWithCopy(*this, file)) {
    HWND hlb = GetDlgItem(*this, IDC_LISTBOX_BMP);
    SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)file);
    }
#endif
}
//-----------------------------------------------------------
void svmChooseBitmap::rem()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_BMP);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  SendMessage(hlb, LB_DELETESTRING, sel, 0);
  SendMessage(hlb, LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
void svmChooseBitmap::move(bool up)
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_BMP);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  SendMessage(hlb, LB_SETCURSEL, sel, 0);
  if(!sel && up)
    return;
  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  if(sel == count - 1 && !up)
    return;

  TCHAR buff[500];
  SendMessage(hlb, LB_GETTEXT, sel, (LPARAM)buff);
  SendMessage(hlb, LB_DELETESTRING, sel, 0);
  sel += up ? -1 : 1;
  SendMessage(hlb, LB_INSERTSTRING, sel, (LPARAM)buff);
  SendMessage(hlb, LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
//static uint gCurrPage;
//void gSetCurrPage(uint ix) { gCurrPage = ix; }
//-----------------------------------------------------------
#if 1
class svmFactoryFont
{
  public:
    svmFactoryFont() {}//  { for(int i = 0; i < SIZE_A(FontObj); ++i) FontObj[i] = 0; }
    ~svmFactoryFont() {}// { for(int i = 0; i < SIZE_A(FontObj); ++i) delete FontObj[i]; }
    svmFont& getFontObj()
    {
      const dataProject& dp = getDataProject();
      return FontObj[dp.currPage];
    }

  private:
    svmFont FontObj[MAX_PAGES];
};
#else
class svmFactoryFont
{
  public:
    svmFactoryFont()  { for(int i = 0; i < SIZE_A(FontObj); ++i) FontObj[i] = 0; }
    ~svmFactoryFont() { for(int i = 0; i < SIZE_A(FontObj); ++i) delete FontObj[i]; }
    svmFont& getFontObj()
    {
      const dataProject& dp = getDataProject();
      if(!FontObj[dp.currPage])
        FontObj[dp.currPage] = new svmFont;
      return *FontObj[dp.currPage];
    }

  private:
    svmFont* FontObj[MAX_PAGES];
};
#endif
//-----------------------------------------------------------
static svmFactoryFont staticFactoryFont;
//-----------------------------------------------------------
svmFont& getFontObj()
{
  return staticFactoryFont.getFontObj();
}
//-----------------------------------------------------------
static svmFactoryFont staticFactoryFontLinked;
//-----------------------------------------------------------
svmFont& getFontObjLinked()
{
  return staticFactoryFontLinked.getFontObj();
}
//-----------------------------------------------------------
typeFont::~typeFont()
{
  delete [] name;
  if(hFont)
    DeleteObject(hFont);
}
//-----------------------------------------------------------
svmFont::svmFont()
{
  init();
}
//-----------------------------------------------------------
svmFont::~svmFont()
{
  flushPV(fontSet);
}
//-----------------------------------------------------------
void svmFont::init()
{
  typeFont* tpf = new typeFont;
  tpf->name = str_newdup(_T("arial"));
#if 1
  tpf->h = (WORD)16;
#else
  sizer::init();
  tpf->h = R__Y((WORD)16);
#endif
  tpf->hFont = D_FONT(tpf->h, 0, 0, tpf->name);
  fontSet[0] = tpf;
}
//-----------------------------------------------------------
void svmFont::addFont(typeFont* tpF)
{
  int nelem = fontSet.getElem();
  fontSet[nelem] = tpF;
}
//-----------------------------------------------------------
HFONT svmFont::getFont(uint id)
{
  uint nelem = fontSet.getElem();
  if(id >= nelem)
    id = 0;
  return fontSet[id]->hFont;
}
//-----------------------------------------------------------
void svmFont::flush()
{
  flushPV(fontSet);
  init();
}
//-----------------------------------------------------------
const typeFont* svmFont::getType(uint id) const
{
  uint nelem = fontSet.getElem();
  if(id >= nelem)
    id = 0;
  return fontSet[id];
}
//-----------------------------------------------------------
void svmFont::remove(uint id)
{
  uint nelem = fontSet.getElem();
  if(id >= nelem)
    return;
  typeFont* tpF = fontSet.remove(id);
  delete tpF;
}
//-----------------------------------------------------------
void svmFont::replace(typeFont* tpF, uint id)
{
  uint nelem = fontSet.getElem();
  if(id >= nelem)
    return;
  typeFont* t = fontSet[id];
  fontSet[id] = tpF;
  delete t;
}
//-----------------------------------------------------------
void svmFont::allocFont(setOfString& set)
{
  flushPV(fontSet);
  int i;
  for(i = 0; ; ++i) {
    uint id = i + ID_INIT_FONT;
    LPCTSTR p = set.getString(id);
    if(!p)
      break;
    int h = R__Y(16);
    int w = 0;
    int flag = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &h, &w, &flag);
    LPCTSTR t = p + _tcslen(p) - 1;
    while(t != p) {
      if(_T(',') == *t) {
        ++t;
        break;
        }
      --t;
      }
    typeFont* tf = new typeFont;
    tf->name = str_newdup(t);
    tf->h = R__Y(h);
    tf->w = R__X(w);
    tf->hFont = D_FONT(tf->h, tf->w, flag, t);
//    tf->h = h;
//    tf->w = w;
//    tf->hFont = D_FONT(R__Y(h), R__X(w), flag, t);
    tf->italic = toBool(flag & fITALIC);
    tf->bold = toBool(flag & fBOLD);
    tf->underl = toBool(flag & fUNDERLINED);
    tf->light = toBool(flag & fLIGHT);
    id = getElem();
    addFont(tf);
    }
  if(!i)
    init();
}
//-----------------------------------------------------------
static void copyFontInfo(typeFont* t, const typeFont* s)
{
  t->name = str_newdup(s->name);
  t->h = s->h;
  t->w = s->w;
  t->italic = s->italic;
  t->bold = s->bold;
  t->underl = s->underl;
  t->light = s->light;
}
//-----------------------------------------------------------
typeFont* svmFont::getCopyType(uint id)
{
  const typeFont* t = getType(id);
  typeFont* tf = new typeFont;
  copyFontInfo(tf, t);
  return tf;
}
//-----------------------------------------------------------
#define RET_DIFF(v) if(f1.v != f2.v) return false
//-----------------------------------------------------------
bool operator ==(const typeFont& f1, const typeFont& f2)
{
  if(_tcsicmp(f1.name, f2.name))
    return false;
  RET_DIFF(h);
  RET_DIFF(w);
  RET_DIFF(italic);
  RET_DIFF(bold);
  RET_DIFF(underl);
  RET_DIFF(light);
  return true;
}
//-----------------------------------------------------------
bool svmFont::getId_add(const typeFont* tf, uint& id)
{
  int nElem = fontSet.getElem();
  for(int i = 0; i < nElem; ++i) {
    const typeFont* ctf = fontSet[i];
    if(*ctf == *tf) {
      id = i;
      return true;
      }
    }
  typeFont* t = new typeFont;
  copyFontInfo(t, tf);
  int flag = t->italic ? fITALIC : 0;
  if(t->bold)
    flag |= fBOLD;
  else if(t->light)
    flag |= fLIGHT;
  if(t->underl)
    flag |= fUNDERLINED;

  t->hFont = D_FONT(t->h, t->w, flag, t->name);
  fontSet[nElem] = t;
  id = nElem;
  return false;
}
//-----------------------------------------------------------
#undef RET_DIFF
//-----------------------------------------------------------
bool svmFont::saveFont(P_File& pf)
{
  int nElem = fontSet.getElem();
  for(int i = 0; i < nElem; ++i) {
    const typeFont* tf = fontSet[i];

    int flag = tf->italic ? fITALIC : 0;
    if(tf->bold)
      flag |= fBOLD;
    else if(tf->light)
      flag |= fLIGHT;
    if(tf->underl)
      flag |= fUNDERLINED;

    TCHAR buff[100];
    wsprintf(buff, _T("%d,%d,%d,%d,%s\r\n"),
              ID_INIT_FONT + i,
//              tf->h, tf->w,
              REV__X(tf->h), REV__Y(tf->w),
              flag, tf->name);
    if(!writeStringChkUnicode(pf, buff))
      return false;
    }
  return true;
}
//-----------------------------------------------------------
/*
uint svmFont::checkFont(uint idFont, setOfString& set)
{
  if(!getFont(idFont)) {
    uint id = idFont + ID_INIT_FONT;
    LPCTSTR p = set.getString(id);
    if(!p)
      return 0;
    int h = 16;
    int w = 0;
    int flag = 0;
    _stscanf(p, _T("%d,%d,%d"), &h, &w, &flag);
    LPCTSTR t = p + _tcslen(p) - 1;
    while(t != p) {
      if(_T(',') == *t) {
        ++t;
        break;
        }
      --t;
      }
    typeFont* tf = new typeFont;
    tf->name = str_newdup(t);
    tf->h = h;
    tf->w = w;
    tf->hFont = D_FONT(R__Y(h), R__X(w), flag, t);
    tf->italic = flag & fITALIC;
    tf->bold = flag & fBOLD;
    tf->underl = flag & fUNDERLINED;
    tf->light = flag & fLIGHT;
    idFont = getElem();
    addFont(tf);
    }
  return idFont;
}
*/
//-----------------------------------------------------------
textAndColor::~textAndColor() { delete []text; }
//-----------------------------------------------------------
void textAndColor::clone(const textAndColor& other)
{
  if(&other == this)
    return;
  delete []text;
  text = 0;
  if(other.text)
    text = str_newdup(other.text);

  fgColor = other.fgColor;
  bgColor = other.bgColor;
  value = other.value;
}
//-----------------------------------------------------------
svmBaseDialogProperty::~svmBaseDialogProperty()
{
  if(fg)
    DeleteObject(fg);
  if(bg)
    DeleteObject(bg);
  delete tmpProp;
  destroy();
}
//-----------------------------------------------------------
bool svmBaseDialogProperty::create()
{
  if(useFont())
    new PStatic(this, IDC_STATICTEXT_FONT);
  if(!baseClass::create())
    return false;
  if(!Prop)
    CmCancel();

  tmpProp = allocProperty();
  *tmpProp = *Prop;
  fg = CreateSolidBrush(Prop->foreground);
  bg = CreateSolidBrush(Prop->background);

  uint idCtrl;
  if(Prop->style & Property::DN)
    idCtrl = IDC_RADIOBUTTON_DOWN_BORDER;
  else if(Prop->style & Property::BORDER)
    idCtrl = IDC_RADIOBUTTON_SIMPLE_BORDER;
  else if(Prop->style & Property::UP)
    idCtrl = IDC_RADIOBUTTON_UP_BORDER;
  else
    idCtrl = IDC_RADIOBUTTON_STYLE_NONE;
  SET_CHECK(idCtrl);

  uint align = Prop->alignText;
  if(Property::aLeft == (align & Property::aLeft))
    idCtrl = IDC_RADIOBUTTON_A_LEFT;
  else if(Property::aRight == (align & Property::aRight))
    idCtrl = IDC_RADIOBUTTON_A_RIGHT;
  else
    idCtrl = IDC_RADIOBUTTON_A_CENTER;
  SET_CHECK(idCtrl);

  if(Prop->style & Property::FILL)
    SET_CHECK(IDC_CHECKBOX_FILL_PANEL);

  if(Prop->style & Property::TRANSP)
    SET_CHECK(IDC_CHECKBOX_TRANSP_PANEL);

//  if(Property::useRealCoord) {
//    SET_CHECK(IDC_CHECKBOX_REAL_COORD);
//    SET_INT(IDC_EDIT_X, Prop->Rect.left);
//    SET_INT(IDC_EDIT_Y, Prop->Rect.top);
//    SET_INT(IDC_EDIT_W, Prop->Rect.Width());
//    SET_INT(IDC_EDIT_H, Prop->Rect.Height());
//    }
//  else {
    SET_INT(IDC_EDIT_X, REV__X(Prop->Rect.left));
    SET_INT(IDC_EDIT_Y, REV__Y(Prop->Rect.top));
    SET_INT(IDC_EDIT_W, REV__X(Prop->Rect.Width()));
    SET_INT(IDC_EDIT_H, REV__Y(Prop->Rect.Height()));
//    }

  svmFont& f = getFontObj();
  const typeFont* tpf = f.getType(tmpProp->idFont);
  setNameFont(tpf);
//  SET_TEXT(IDC_STATICTEXT_FONT, tpf->name);

  fillPrf();
  fillTypeVal();
  fillPswLevel();
  SET_INT(IDC_EDIT_ADDR, Prop->addr);
  SET_INT(IDC_EDIT_NBITS, Prop->nBits);
  SET_INT(IDC_EDIT_OFFSET, Prop->offset);
  SET_INT(IDC_EDIT_NORMALIZ, Prop->normaliz);
  SET_INT(IDC_EDIT_NDEC, Prop->nDec);

  if(Prop->negative)
    SET_CHECK(IDC_CHECKBOX_NEGATIVE);

  SET_INT(IDC_EDIT_VIS_ADDR, Prop->visAddr);
  SET_INT(IDC_EDIT_VIS_NBITS, Prop->visnBits);
  SET_INT(IDC_EDIT_VIS_OFFSET, Prop->visOffset);

  if(Prop->visNegative)
    SET_CHECK(IDC_CHECKBOX_VIS_NEGATIVE);

  smartPointerConstString t = getTitle();
  if(&t) {
    LPCTSTR p = getCaption();
    size_t len = _tcslen(&t);
    len += _tcslen(p);
    len += 32;
    LPTSTR buff = new TCHAR[len];
    if(Owner)
      wsprintf(buff, _T("%s - %s(%d)"), p, &t, Owner->get_zOrder());//getId());
    else
      wsprintf(buff, _T("%s - %s"), p, &t);
    setCaption(buff);
    delete []buff;
    }
  checkBitOrValueVis();
  if(Prop->MoveInfo.isEnabled())
    SET_CHECK(IDC_CHECKBOX_MOV);
  checkEnabledMove();
  return true;
}
//----------------------------------------------------------------------------
Property* svmBaseDialogProperty::allocProperty()
{
  return Owner->allocProperty();
}
//----------------------------------------------------------------------------
void addStringToComboBox(HWND hList, LPCTSTR str, uint addSize)
{
  int wid = extent(hList, str);
  int awi = SendMessage(hList, CB_GETDROPPEDWIDTH, 0, 0);

  if(wid + (int)addSize > awi)
    ::SendMessage(hList, CB_SETDROPPEDWIDTH, wid + addSize, 0);
  ::SendMessage(hList, CB_ADDSTRING, 0, LPARAM(str));
}
//-----------------------------------------------------------
void fillCBCTRL_Action(HWND hwnd, int select)
{
  LPCTSTR prf[] = {
    _T("Setup"),
    _T("Scelta-lista"),
    _T("Scelta-albero"),
    _T("Lingua"),
    _T("Risoluzione"),
    };
  // poiché ci sarà sicuramente lo scroll verticale aggiungiamo
  // senza controllare. Altrimenti sarebbe da verificare
//  int add = GetSystemMetrics(SM_CXVSCROLL);
  int add = 0;
  for(uint i = 0; i < SIZE_A(prf); ++i)
    addStringToComboBox(hwnd, prf[i], add);
  if(select < 0 || (uint)select >= SIZE_A(prf))
    select = 0;
  SendMessage(hwnd, CB_SETCURSEL, select, 0);
}
//-----------------------------------------------------------
void fillCBPerifEx(HWND hwnd, int select, bool addConstPrph)
{
  LPCTSTR prf[] = {
    _T("00-Prf.None"),
    _T("01-Prf.Mem"),
    _T("02-Prf.PLC"),
    _T("03-Prf.Gen"),
    _T("04-Prf.Gen"),
    _T("05-Prf.Gen"),
    _T("06-Prf.Gen"),
    _T("07-Prf.Gen"),
    _T("08-Prf.Gen"),
    _T("09-Prf.Gen"),

    _T("10-Prf.Mem+"),
    _T("11-Prf.Mem+"),
    _T("12-Prf.Mem+"),
    _T("13-Prf.Mem+"),
    _T("14-Prf.Mem+"),
    _T("15-Prf.Mem+"),
    _T("16-Prf.Mem+"),
    _T("17-Prf.Mem+"),
    _T("18-Prf.Mem+"),
    _T("19-Prf.Mem+"),
    _T("20-Prf.Mem+"),
    _T("Use Const"),
    };
  // poiché ci sarà sicuramente lo scroll verticale aggiungiamo
  // senza controllare. Altrimenti sarebbe da verificare
//  int add = GetSystemMetrics(SM_CXVSCROLL);
  int add = 0;
  const uint tot =  SIZE_A(prf) - 1 + addConstPrph;
  for(uint i = 0; i < tot; ++i)
    addStringToComboBox(hwnd, prf[i], add);
  if(select < 0 || (uint)select >= tot)
    select = 0;
  SendMessage(hwnd, CB_SETCURSEL, select, 0);
}
//-----------------------------------------------------------
void fillCBPerif(HWND hwnd, int select)
{
  fillCBPerifEx(hwnd, select, false);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::fillPrf()
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS);
  if(hwnd)
    fillCBPerif(hwnd, Prop->perif);
  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_VIS_PERIFS);
  if(hwnd)
    fillCBPerif(hwnd, Prop->visPerif);
}
//-----------------------------------------------------------
void fillCBPswLevel(HWND hwnd, int select, bool full)
{
  LPCTSTR cbPsw[] = {
    _T("   0 - nulla"),
    _T("   1 - operatore"),
    _T("   2 - master"),
    _T("   3 - supervisor"),
    _T("1000 - solo conferma"),
    };
  int nElem = SIZE_A(cbPsw) - 1 + full;
  for(int i = 0; i < nElem; ++i)
    addStringToComboBox(hwnd, cbPsw[i]);
  if(select < 0 || select >= nElem)
    select = 0;
  SendMessage(hwnd, CB_SETCURSEL, select, 0);
}
//-----------------------------------------------------------
void fillCBTypeVal(HWND hwnd, int select)
{
  LPCTSTR cbType[] = {
    _T(" 0 - null"),
    _T(" 1 - bit"),
    _T(" 2 - byte"),
    _T(" 3 - word"),
    _T(" 4 - dword"),
    _T(" 5 - float"),
    _T(" 6 - int64"),
    _T(" 7 - real"),
    _T(" 8 - dateHour"),
    _T(" 9 - Date"),
    _T("10 - Hour"),
    _T("11 - char"),
    _T("12 - short"),
    _T("13 - long"),
    _T("14 - string"),
    };
  for(uint i = 0; i < SIZE_A(cbType); ++i)
    addStringToComboBox(hwnd, cbType[i]);
  if(select < 0 || (uint)select >= SIZE_A(cbType))
    select = 0;
  SendMessage(hwnd, CB_SETCURSEL, select, 0);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::fillTypeVal()
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  if(hwnd)
    fillCBTypeVal(hwnd, Prop->typeVal);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::fillPswLevel()
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PSWLEVEL);
  if(hwnd)
    fillCBPswLevel(hwnd, Prop->pswLevel);
}
//-----------------------------------------------------------
smartPointerConstString svmBaseDialogProperty::getTitle() const { return Owner ? Owner->getTitle4Prop() : smartPointerConstString(0, 0); }
//-----------------------------------------------------------
LRESULT svmBaseDialogProperty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_FG:
        case IDC_BUTTON_CHOOSE_BG:
          chooseColor(IDC_BUTTON_CHOOSE_FG == LOWORD(wParam));
          break;
        case IDC_BUTTON_CHOOSE_FONT:
          chooseFont();
          break;
        case IDC_BUTTON_NORMALIZ:
          chooseNormaliz(IDC_EDIT_NORMALIZ);
          break;

        case IDC_CHECKBOX_MOV:
          checkEnabledMove();
          break;
        case IDC_BUTTON_MOVIM:
          movingData();
          break;
        }
      switch(HIWORD(wParam)) {
        case CBN_SELCHANGE:
          CBChanged(LOWORD(wParam));
          if(IDC_COMBOBOX_VIS_PERIFS == LOWORD(wParam))
            checkBitOrValueVis();
          break;
        case EN_KILLFOCUS:
        case EN_CHANGE:
          if(IDC_EDIT_VIS_NBITS == LOWORD(wParam))
            checkBitOrValueVis();
          break;
        }
      break;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
//    case WM_CTLCOLOREDIT:
//    case WM_CTLCOLORLISTBOX:
//    case WM_CTLCOLORMSGBOX:
//    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::checkEnabledMove()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_MOV);
  ENABLE(IDC_BUTTON_MOVIM, enable);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::movingData()
{
  SIZE sz = { tmpProp->Rect.Width(), tmpProp->Rect.Height() };
  tmpProp->MoveInfo.config(this, sz);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::checkBitOrValueVis()
{
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_VIS_PERIFS), CB_GETCURSEL, 0, 0);
  if(sel <= 0) {
    SET_TEXT(IDC_STATICTEXT4_VIS, _T("Offset"));
    SET_TEXT(IDC_STATICTEXT3_VIS, _T("N Bits"));
    return;
    }
  int value;
  GET_INT(IDC_EDIT_VIS_NBITS, value);
  if(!value) {
    SET_TEXT(IDC_STATICTEXT4_VIS, _T("Valore"));
    SET_TEXT(IDC_STATICTEXT3_VIS, _T("Type 3-4"));
    }
  else {
    SET_TEXT(IDC_STATICTEXT4_VIS, _T("Offset"));
    SET_TEXT(IDC_STATICTEXT3_VIS, _T("N Bits"));
    }
}
//-----------------------------------------------------------
#if 0
void svmBaseDialogProperty::chgCoordType(uint idCtrl, bool isReal)
{
  int val;
  GET_INT(idCtrl, val);
  if(isReal)
    val = R__X(val);
  else
    val = REV__X(val);
  SET_INT(idCtrl, val);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::changeRealCoord()
{
  bool isReal = IS_CHECKED(IDC_CHECKBOX_REAL_COORD);
  Property::useRealCoord = isReal;
  chgCoordType(IDC_EDIT_X, isReal);
  chgCoordType(IDC_EDIT_Y, isReal);
  chgCoordType(IDC_EDIT_W, isReal);
  chgCoordType(IDC_EDIT_H, isReal);
}
#endif
//-----------------------------------------------------------
void svmBaseDialogProperty::CBChanged(uint idCtrl)
{
  int sel = SendMessage(GetDlgItem(*this, idCtrl), CB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  switch(idCtrl) {
    case IDC_COMBOBOX_PERIFS:
      tmpProp->perif = sel;
      break;
    case IDC_COMBOBOX_TYPEVAL:
      tmpProp->typeVal = sel;
      break;
    }
}
//-----------------------------------------------------------
void svmBaseDialogProperty::CmOk()
{
  *Prop = *tmpProp;

  int t;
  GET_INT(IDC_EDIT_X, t);
//  if(Property::useRealCoord)
//    Prop->Rect.left = t;
//  else
    Prop->Rect.left = R__X(t);

  GET_INT(IDC_EDIT_Y, t);
//  if(Property::useRealCoord)
//    Prop->Rect.top = t;
//  else
    Prop->Rect.top = R__Y(t);

  GET_INT(IDC_EDIT_W, t);
//  if(Property::useRealCoord)
//    Prop->Rect.right = Prop->Rect.left + t;
//  else
    Prop->Rect.right = Prop->Rect.left + R__X(t);

  GET_INT(IDC_EDIT_H, t);
//  if(Property::useRealCoord)
//    Prop->Rect.bottom = Prop->Rect.top + t;
//  else
    Prop->Rect.bottom = Prop->Rect.top + R__Y(t);

  if(IS_CHECKED(IDC_RADIOBUTTON_STYLE_NONE))
    Prop->style = Property::NO;
  else if(IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_BORDER))
    Prop->style = Property::BORDER;
  else if(IS_CHECKED(IDC_RADIOBUTTON_UP_BORDER))
    Prop->style = Property::UP;
  else
    Prop->style = Property::DN;

  if(IS_CHECKED(IDC_RADIOBUTTON_A_LEFT))
    Prop->alignText = Property::aLeft;
  else if(IS_CHECKED(IDC_RADIOBUTTON_A_RIGHT))
    Prop->alignText = Property::aRight;
  else
    Prop->alignText = Property::aCenter;

  if(IS_CHECKED(IDC_CHECKBOX_FILL_PANEL))
    Prop->style |= Property::FILL;

  if(IS_CHECKED(IDC_CHECKBOX_TRANSP_PANEL))
    Prop->style |= Property::TRANSP;

  getPrf();
  getTypeVal();
  getPswLevel();
  GET_INT(IDC_EDIT_ADDR, Prop->addr);
  GET_INT(IDC_EDIT_NBITS, Prop->nBits);
  GET_INT(IDC_EDIT_OFFSET, Prop->offset);
  GET_INT(IDC_EDIT_NORMALIZ, Prop->normaliz);
  GET_INT(IDC_EDIT_NDEC, Prop->nDec);

  Prop->negative = IS_CHECKED(IDC_CHECKBOX_NEGATIVE);

  GET_INT(IDC_EDIT_VIS_ADDR, Prop->visAddr);
  GET_INT(IDC_EDIT_VIS_NBITS, Prop->visnBits);
  GET_INT(IDC_EDIT_VIS_OFFSET, Prop->visOffset);

  Prop->visNegative = IS_CHECKED(IDC_CHECKBOX_VIS_NEGATIVE);

  if(!IS_CHECKED(IDC_CHECKBOX_MOV) || !Prop->MoveInfo.getObj()) {
    wrapMoveInfo dummy;
    Prop->MoveInfo = dummy;
    }

  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmBaseDialogProperty::getPrf()
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS);
  Prop->perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_VIS_PERIFS);
  Prop->visPerif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::getTypeVal()
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  Prop->typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::getPswLevel()
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PSWLEVEL);
  Prop->pswLevel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
}
//-----------------------------------------------------------
HBRUSH svmBaseDialogProperty::evCtlColor(HDC dc, HWND hWndChild, UINT /*ctlType*/)
{
  // l'impostazione del codice è dovuta al test per cambiare i colori
  // ai pulsanti. Se non sono ownerDraw non cambiano
  int type = 0;
  if(GetDlgItem(*this, IDC_STATICTEXT_FOREGROUND) == hWndChild)
    type = 1;
  else if(GetDlgItem(*this, IDC_BUTTON_CHOOSE_FG) == hWndChild)
    type = 1;

  else if(GetDlgItem(*this, IDC_STATICTEXT_BACKGROUND) == hWndChild)
    type = 2;
  else if(GetDlgItem(*this, IDC_BUTTON_CHOOSE_BG) == hWndChild)
    type = 2;

  if(0 == type)
    return 0;

//  SetBkMode(dc, TRANSPARENT);

  if(1 == type)  {
    SetTextColor(dc, tmpProp->background);
    SetBkColor(dc, tmpProp->foreground);
    return fg;
    }
  if(2 == type)  {
    SetTextColor(dc, tmpProp->foreground);
    SetBkColor(dc, tmpProp->background);
    return bg;
    }

  return 0;
}
//-----------------------------------------------------------
void svmBaseDialogProperty::chooseNormaliz(uint idCtrl)
{
  uint result;
  GET_INT(idCtrl, result);
  if(svmSetupNormal(this, result))
    SET_INT(idCtrl, result);

}
//-----------------------------------------------------------
bool choose_Color(HWND parent, COLORREF& target)
{
  const DWORD flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;
  CHOOSECOLOR cc;
  memset(&cc, 0, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = parent;
  cc.rgbResult = target;
  cc.Flags = flags;// | CC_ANYCOLOR;
  static COLORREF custom[16];
  cc.lpCustColors = custom;

  if(ChooseColor(&cc)) {
    target = cc.rgbResult;
//    flags = cc.Flags;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
void svmBaseDialogProperty::updateColor(bool fgnd)
{
  if(fgnd) {
    DeleteObject(fg);
    fg = CreateSolidBrush(tmpProp->foreground);
    }
  else {
    DeleteObject(bg);
    bg = CreateSolidBrush(tmpProp->background);
    }
  uint idCtrls[] = {
    IDC_STATICTEXT_FOREGROUND,
    IDC_STATICTEXT_BACKGROUND,
    IDC_BUTTON_CHOOSE_FG,
    IDC_BUTTON_CHOOSE_BG,
    };
  for(uint i = 0; i < SIZE_A(idCtrls); ++i)
    InvalidateRect(GetDlgItem(*this, idCtrls[i]), 0, 0);
}
//-----------------------------------------------------------
void svmBaseDialogProperty::chooseColor(bool fgnd)
{
  if(choose_Color(*this, fgnd ? tmpProp->foreground : tmpProp->background))
    updateColor(fgnd);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmListFont : public PListBox
{
  private:
    typedef PListBox baseClass;
  public:
    svmListFont(PWin* parent, uint id = IDC_LISTBOX_FONT_ALLOCATED, int len = 255, HINSTANCE hinst = 0):
       PListBox(parent, id, len, hinst) { }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void findFont(int item);
    virtual void paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo);
    virtual void paintCustom(HDC hDC, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);

};
//-----------------------------------------------------------
void svmListFont::paintCustom(HDC hDC, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  baseClass::paintCustom(hDC, r, ix, drawInfo);
  if(!ix && _T('1') == *Buff) {
    PRect r2(r);
#if 1
    UINT state = DFCS_MENUBULLET | DFCS_TRANSPARENT;
    r2.Inflate(-1, -2);
    DrawFrameControl(hDC, r2, DFC_MENU, state);
#else
    PBitmap bmp(IDB_BITMAP_OK, getHInstance());
    if(bmp.isValid()) {
      SIZE sz = bmp.getSize();
      double scaleX = r2.Width();
      scaleX /= (double)sz.cx;
      double scaleY = r2.Height();
      scaleY /= (double)sz.cy;
      double scale = scaleX > scaleY ? scaleY : scaleX;

      POINT pt = { r2.left, r2.top };
      PTraspBitmap tb(this, &bmp, pt);
      tb.setScale(scale);
      tb.Draw(hDC);
      }
#endif
    }
}
//-----------------------------------------------------------
void svmListFont::paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(ix) {
    svmFont& f = getFontObj();
    HGDIOBJ old = SelectObject(hDC, f.getFont(drawInfo.itemID));
    baseClass::paintTxt(hDC, x, y, buff, ix, drawInfo);
    SelectObject(hDC, old);
    }
}
//-----------------------------------------------------------
void svmListFont::findFont(int item)
{
  if(item < 0)
    return;
  svmFont& f = getFontObj();
  setFontNoReduce(f.getFont(item), false);
}
//-----------------------------------------------------------
LRESULT svmListFont::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_MEASUREITEM:
      do {
        hItem = 0;
        MEASUREITEMSTRUCT *lpmis = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
        findFont(lpmis->itemID);
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);

}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
void modifyPageFont(PWin* parent)
{
  uint dummy = (uint)-1;
  svmChooseFont(dummy, parent).modal();
}
//-----------------------------------------------------------
#define LEN_NAMEFONT LF_FACESIZE
#define LEN_DIMFLAG   1
#define LEN_DIMFONT   6
#define sBOLD _T("Bold")
#define sTHIN _T("Thin")
#define sNORM _T("Norm")
#define sITALIC _T(" -I")
#define sUNDERL _T(" -U")
#define LEN_TYPEFONT (SIZE_A_c(sBOLD) + SIZE_A_c(sITALIC) + SIZE_A_c(sUNDERL) - 2)

#define MAX_LEN_LISTFONT (LEN_DIMFLAG + LEN_NAMEFONT + LEN_DIMFONT + LEN_TYPEFONT + 5)
//-----------------------------------------------------------
bool svmChooseFont::create()
{
  LB = new svmListFont(this);
  int tabs[] = { LEN_DIMFLAG, LEN_DIMFONT, LEN_TYPEFONT, LEN_NAMEFONT };
  LB->SetTabStop(SIZE_A(tabs), tabs, 0);
  LB->setAlign(0, PListBox::aCenter);

  if(!baseClass::create())
    return false;
  if((uint)-1 != idFont)
    ShowWindow(GetDlgItem(*this, IDC_BUTTON_UPD_FONT), SW_HIDE);
  else {
    ShowWindow(GetDlgItem(*this, IDOK), SW_HIDE);
    SetWindowText(GetDlgItem(*this, IDCANCEL), _T("Finito"));
    }
  svmFont& f = getFontObj();
  const setOfPFont& set = f.getSet();
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i)
    addToLb(set[i], -1);

  SendMessage(*LB, LB_SETCURSEL, idFont, 0);
  return true;
}
//-----------------------------------------------------------
void svmChooseFont::CmOk()
{
  int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(sel >= 0) {
    idFont = sel;
    baseClass::CmOk();
    }
}
//-----------------------------------------------------------
LRESULT svmChooseFont::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_FONT:
          chooseFont(false);
          break;
        case IDC_BUTTON_MOD_FONT:
          chooseFont(true);
          break;
        case IDC_BUTTON_REM_FONT:
          remFont();
          break;
        case IDC_BUTTON_UPD_FONT:
          do {
            svmMainClient* mc = getMainClient(this);
            if(mc)
              mc->invalidate();
            } while(false);
          break;
        }
      break;

    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
static void fillLogFont(HWND hwnd, LOGFONT& lf, int sel)
{
  svmFont& f = getFontObj();
  const typeFont* tpf = f.getType(sel);
  if(!tpf)
    return;
//    GetObject(tpf->hFont, sizeof(lf), &lf);

//  lf.lfHeight = REV__Y(tpf->h);
  HDC hdc = GetDC(hwnd);
//  lf.lfHeight = -MulDiv(tpf->h, GetDeviceCaps(hdc, LOGPIXELSY), 72);
  lf.lfHeight = -MulDiv(REV__Y(tpf->h), GetDeviceCaps(hdc, LOGPIXELSY), 72);
  ReleaseDC(hwnd, hdc);
  lf.lfItalic = tpf->italic;
  lf.lfWeight = tpf->bold ? FW_BOLD : tpf->light ? FW_LIGHT : FW_NORMAL;
  _tcscpy_s(lf.lfFaceName, tpf->name);
}
//-----------------------------------------------------------
#define FLAGS_FONT CF_SCALABLEONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT
//#define FLAGS_FONT CF_SCALABLEONLY | CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT

void svmChooseFont::chooseFont(bool replace)
{
  int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  static LOGFONT lf;
  if(replace) {
    if(sel < 0)
      return;
    fillLogFont(*this, lf, sel);
    }
  if(!lf.lfHeight)
    fillLogFont(*this, lf, 0);

  CHOOSEFONT cf;
  memset(&cf, 0, sizeof(cf));
  cf.lStructSize = sizeof(cf);
  cf.hwndOwner = getHandle();
  cf.lpLogFont = &lf;
  cf.Flags = FLAGS_FONT;
  cf.nFontType = SCREEN_FONTTYPE | REGULAR_FONTTYPE | ITALIC_FONTTYPE | BOLD_FONTTYPE;
  if(ChooseFont(&cf)) {
    typeFont* tpf = new typeFont;
    tpf->name = str_newdup(lf.lfFaceName);
//    tpf->h = R__Y(lf.lfHeight);
//    if((short)tpf->h < 0)
//      tpf->h = -(short)tpf->h;
    tpf->italic = toBool(lf.lfItalic);
    tpf->underl = toBool(lf.lfUnderline);
    tpf->bold = lf.lfWeight == FW_BOLD;
    tpf->light = lf.lfWeight == FW_LIGHT;

    HDC hdc = GetDC(*this);
    int nHeight = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
    ReleaseDC(*this, hdc);

    tpf->h = (WORD)nHeight;
    if(nHeight < 0)
      tpf->h = (WORD)-nHeight;
    int flag = 0;
    if(tpf->italic)
      flag |= fITALIC;
    if(tpf->underl)
      flag |= fUNDERLINED;
    if(tpf->bold)
      flag |= fBOLD;
    if(tpf->light)
      flag |= fLIGHT;

    tpf->h = R__Y(tpf->h);
    tpf->w = R__X(tpf->w);
    tpf->hFont = D_FONT(tpf->h, tpf->w, flag, tpf->name);
//    tpf->hFont = D_FONT(R__Y(tpf->h), R__X(tpf->w), flag, tpf->name);
//    tpf->hFont = CreateFontIndirect(&lf);

    svmFont& f = getFontObj();
    if(replace) {
      f.replace(tpf, sel);
      SendMessage(*LB, LB_DELETESTRING, sel, 0);
      }
    else {
      f.addFont(tpf);
      sel = -1;
      }
    addToLb(tpf, sel);
    }
}
//-----------------------------------------------------------
void svmChooseFont::remFont()
{
  int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(sel <= 0)
    return;
  svmMainClient* mc = getMainClient(this);
  if(mc && mc->isUsedFontId(sel))
    return;
  svmFont& f = getFontObj();
  f.remove(sel);
  SendMessage(*LB, LB_DELETESTRING, sel, 0);
  if(mc)
    mc->decreaseFontIfAbove(sel);
}
//-----------------------------------------------------------
void svmChooseFont::addToLb(const typeFont* tpf, int pos)
{
  svmMainClient* mc = getMainClient(this);
  if(pos < 0)
    pos = SendMessage(*LB, LB_GETCOUNT, 0, 0);
  TCHAR buff[MAX_LEN_LISTFONT + 2];
  LPTSTR p = buff;
  fillStr(p, _T(' '), SIZE_A(buff));

  if(mc && mc->isUsedFontId(pos))
    *p = _T('1');
  p += LEN_DIMFLAG;
  *p++ = TAB;

  TCHAR t[30];
  wsprintf(t, _T("%d"), tpf->h);
//  wsprintf(t, _T("%d"), REV__Y(tpf->h));
  copyStr(p, t, _tcslen(t));
  p += LEN_DIMFONT;
  *p++ = TAB;
  LPTSTR p2 = p;
  if(tpf->bold) {
    copyStr(p, sBOLD, SIZE_A(sBOLD) - 1);
    p +=  SIZE_A(sBOLD) - 1;
    }
  else if(tpf->light) {
    copyStr(p, sTHIN, SIZE_A(sTHIN) - 1);
    p +=  SIZE_A(sTHIN) - 1;
    }
  else {
    copyStr(p, sNORM, SIZE_A(sNORM) - 1);
    p +=  SIZE_A(sNORM) - 1;
    }

  if(tpf->italic) {
    copyStr(p, sITALIC, SIZE_A(sITALIC) - 1);
    p +=  SIZE_A(sITALIC) - 1;
    }
  if(tpf->underl) {
    copyStr(p, sUNDERL, SIZE_A(sUNDERL) - 1);
//    p +=  SIZE_A(sUNDERL) - 1;
    }
  p2 += LEN_TYPEFONT;
  *p2++ = TAB;

  int len = _tcslen(tpf->name);
  if(len > LEN_NAMEFONT)
    len = LEN_NAMEFONT;
  copyStr(p2, tpf->name, len);
  p2 += LEN_NAMEFONT;
  *p2++ = TAB;
  *p2 = 0;
  int count = SendMessage(*LB, LB_INSERTSTRING, pos, (LPARAM)buff);
  SendMessage(*LB, LB_SETCURSEL, count, 0);
}
//-----------------------------------------------------------
void setNameFont(PWin* owner, uint idc, const typeFont* tpf, bool usefont)
{
  TCHAR buff[200];
  TCHAR style[20] = _T("\0");
  if(tpf->bold)
    _tcscpy_s(style, _T("Bold"));
  else if(tpf->light)
    _tcscpy_s(style, _T("Light"));
  else if(tpf->italic)
    _tcscpy_s(style, _T("Italic"));

  if(!style[0]) {
    if(tpf->italic)
      _tcscpy_s(style, _T("Italic"));
    else
      _tcscpy_s(style, _T("Normal"));
    }
  else if(_tcscmp(style, _T("Italic")))
    if(tpf->italic)
      _tcscat_s(style, _T("-Italic"));

  wsprintf(buff, _T("%s - %d - %s"), tpf->name, tpf->h, style);
  SetDlgItemText(*owner, idc, buff);
//  SET_TEXT(IDC_STATICTEXT_FONT, buff);
  if(usefont) {
    PStatic* t = dynamic_cast<PStatic*>(PWin::getWindowPtr(GetDlgItem(*owner, idc)));
    if(t)
      t->setFont(tpf->hFont);
    }
}
//-----------------------------------------------------------
void svmBaseDialogProperty::setNameFont(const typeFont* tpf)
{
  ::setNameFont(this, IDC_STATICTEXT_FONT, tpf, useFont());
}
//-----------------------------------------------------------
void svmBaseDialogProperty::chooseFont()
{
  uint idFont = tmpProp->idFont;
  if(IDOK == svmChooseFont(idFont, this).modal()) {
    svmFont& f = getFontObj();
    const typeFont* tpf = f.getType(idFont);
    setNameFont(tpf);
    tmpProp->idFont = idFont;
/*
    TCHAR buff[200];
    wsprintf(buff, _T("%s - %d - %c%c"),
          tpf->name, tpf->h, tpf->bold ? _T('B') : _T(' '),
          tpf->italic ? _T('I') : _T(' '));
    SET_TEXT(IDC_STATICTEXT_FONT, buff);
*/
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
Property* svmCommonDialog::allocProperty()
{
  return new Property;
}
//-----------------------------------------------------------
svmCommonDialog::~svmCommonDialog()
{
  destroy();
}
//-----------------------------------------------------------
smartPointerConstString svmCommonDialog::getTitle() const
{
//  wsprintf(title, _T(" - %d objects"), Prop->type1);
  return smartPointerConstString(title, 0);
}
//-----------------------------------------------------------
bool svmCommonDialog::create()
{
  new langEditDefCR(this, IDC_EDIT_SAMPLE_TEXT);

  if(!baseClass::create())
    return false;

  Prop->type1 = 0; // per dx
  Prop->type2 = 0; // per dy

  uint align = Prop->alignText;
  uint idCtrl;
  if(Property::aMid == (align & Property::aMid))
    idCtrl = IDC_RADIOBUTTON_A_CENTER2;
  else if(Property::aBottom == (align & Property::aBottom))
    idCtrl = IDC_RADIOBUTTON_A_RIGHT2;
  else
    idCtrl = IDC_RADIOBUTTON_A_LEFT2;
  SET_CHECK(idCtrl);

  checkEnabled();

  return true;
}
//-----------------------------------------------------------
static const uint idNormalizCtrl[] =
{
  IDC_EDIT_NORMALIZ,
  IDC_BUTTON_NORMALIZ,
};
//-----------------------------------------------------------
static const uint idAllCtrl[] =
{
  IDC_BUTTON_CHOOSE_FG,
  IDC_BUTTON_CHOOSE_BG,
  IDC_COMBOBOX_PERIFS,
  IDC_EDIT_ADDR,
  IDC_COMBOBOX_TYPEVAL,
  IDC_EDIT_NDEC,
  IDC_EDIT_NBITS,
  IDC_EDIT_OFFSET,
  IDC_CHECKBOX_NEGATIVE,
  IDC_EDIT_X,
  IDC_EDIT_Y,
  IDC_EDIT_W,
  IDC_EDIT_H,
  IDC_BUTTON_CHOOSE_FONT,
  IDC_EDIT_DX,
  IDC_EDIT_DY,
  IDC_COMBOBOX_PSWLEVEL,

  IDC_COMBOBOX_VIS_PERIFS,
  IDC_EDIT_VIS_ADDR,
  IDC_EDIT_VIS_NBITS,
  IDC_EDIT_VIS_OFFSET,
  IDC_CHECKBOX_VIS_NEGATIVE,

  IDC_EDIT_SAMPLE_TEXT

};
//-----------------------------------------------------------
static const uint idCtrlStyle[] =
{
  IDC_RADIOBUTTON_STYLE_NONE,
  IDC_RADIOBUTTON_SIMPLE_BORDER,
  IDC_RADIOBUTTON_UP_BORDER,
  IDC_RADIOBUTTON_DOWN_BORDER,
  IDC_CHECKBOX_FILL_PANEL,
  IDC_CHECKBOX_TRANSP_PANEL
};
//-----------------------------------------------------------
static const uint idCtrlAlign[] =
{
  IDC_RADIOBUTTON_A_LEFT,
  IDC_RADIOBUTTON_A_RIGHT,
  IDC_RADIOBUTTON_A_CENTER,
};
//-----------------------------------------------------------
static const uint idCtrlAlign2[] =
{
  IDC_RADIOBUTTON_A_LEFT2,
  IDC_RADIOBUTTON_A_RIGHT2,
  IDC_RADIOBUTTON_A_CENTER2,
};
//-----------------------------------------------------------
static const uint idCtrlMove[] =
{
  IDC_CHECKBOX_MOV,
  IDC_BUTTON_MOVIM,
};
//-----------------------------------------------------------
struct ctrlEnabledBy
{
  uint check;
  int num;
  const uint* set;
};
//-----------------------------------------------------------
static const ctrlEnabledBy EnableBy[] = {
  {  IDC_CHECKBOX_ENABLE_FG, 1, idAllCtrl + CHANGED_FG },
  {  IDC_CHECKBOX_ENABLE_BG, 1, idAllCtrl + CHANGED_BG },
  {  IDC_CHECKBOX_ENABLE_PRF, 1, idAllCtrl + CHANGED_PRF },
  {  IDC_CHECKBOX_ENABLE_ADDR, 1, idAllCtrl + CHANGED_ADDR },
  {  IDC_CHECKBOX_ENABLE_VALUETYPE, 1, idAllCtrl + CHANGED_VALUETYPE },
  {  IDC_CHECKBOX_ENABLE_NDEC, 1, idAllCtrl + CHANGED_NDEC },
  {  IDC_CHECKBOX_ENABLE_NBITS, 1, idAllCtrl + CHANGED_NBITS },
  {  IDC_CHECKBOX_ENABLE_OFFSET, 1, idAllCtrl + CHANGED_OFFSET },
  {  IDC_CHECKBOX_ENABLE_NEGATIVE, 1, idAllCtrl + CHANGED_NEGATIVE },
  {  IDC_CHECKBOX_ENABLE_X, 1, idAllCtrl + CHANGED_X },
  {  IDC_CHECKBOX_ENABLE_Y, 1, idAllCtrl + CHANGED_Y },
  {  IDC_CHECKBOX_ENABLE_W, 1, idAllCtrl + CHANGED_W },
  {  IDC_CHECKBOX_ENABLE_H, 1, idAllCtrl + CHANGED_H },
  {  IDC_CHECKBOX_ENABLE_FONT, 1, idAllCtrl + CHANGED_FONT },
  {  IDC_CHECKBOX_ENABLE_DX, 1, idAllCtrl + CHANGED_DX },
  {  IDC_CHECKBOX_ENABLE_DY, 1, idAllCtrl + CHANGED_DY },
  {  IDC_CHECKBOX_PSWLEVEL, 1, idAllCtrl + CHANGED_PSW_LEVEL },

  {  IDC_CHECKBOX_ENABLE_PRF_V, 1, idAllCtrl + CHANGED_PRF_V },
  {  IDC_CHECKBOX_ENABLE_ADDR_V, 1, idAllCtrl + CHANGED_ADDR_V },
  {  IDC_CHECKBOX_ENABLE_NBITS_V, 1, idAllCtrl + CHANGED_NBITS_V },
  {  IDC_CHECKBOX_ENABLE_OFFSET_V, 1, idAllCtrl + CHANGED_OFFSET_V },
  {  IDC_CHECKBOX_ENABLE_NEG_V, 1, idAllCtrl + CHANGED_NEG_V },

  {  IDC_CHECKBOX_ENABLE_SIMPLE_TXT, 1, idAllCtrl + CHANGED_SIMPLE_TEXT },

  {  IDC_CHECKBOX_ENABLE_NORMALIZ, SIZE_A(idNormalizCtrl), idNormalizCtrl },
  {  IDC_CHECKBOX_ENABLE_ALIGN, SIZE_A(idCtrlAlign), idCtrlAlign },
  {  IDC_CHECKBOX_ENABLE_ALIGN2, SIZE_A(idCtrlAlign2), idCtrlAlign2 },
  {  IDC_CHECKBOX_ENABLE_BORDER, SIZE_A(idCtrlStyle), idCtrlStyle },
  {  IDC_CHECK_ENABLE_MOV, SIZE_A(idCtrlMove), idCtrlMove },

};
//-----------------------------------------------------------
static void enableCtrl(HWND hdlg, const ctrlEnabledBy& ceb)
{
  bool enable = IS_CHECKED_HW(hdlg, ceb.check);
  for(int i = 0; i < ceb.num; ++i)
    ENABLE_HW(hdlg, ceb.set[i], enable);
}
//-----------------------------------------------------------
void svmCommonDialog::checkEnabled(int ctrl)
{
  for(uint i = 0; i < SIZE_A(EnableBy); ++i)
    if(-1 == ctrl || EnableBy[i].check == (uint)ctrl)
      enableCtrl(*this, EnableBy[i]);
}
//-----------------------------------------------------------
LRESULT svmCommonDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECKBOX_ENABLE_FG:
        case IDC_CHECKBOX_ENABLE_BG:
        case IDC_CHECKBOX_ENABLE_PRF:
        case IDC_CHECKBOX_ENABLE_ADDR:
        case IDC_CHECKBOX_ENABLE_VALUETYPE:
        case IDC_CHECKBOX_ENABLE_NDEC:
        case IDC_CHECKBOX_ENABLE_NBITS:
        case IDC_CHECKBOX_ENABLE_OFFSET:
        case IDC_CHECKBOX_ENABLE_NORMALIZ:
        case IDC_CHECKBOX_ENABLE_NEGATIVE:
        case IDC_CHECKBOX_ENABLE_BORDER:
        case IDC_CHECKBOX_ENABLE_X:
        case IDC_CHECKBOX_ENABLE_Y:
        case IDC_CHECKBOX_ENABLE_W:
        case IDC_CHECKBOX_ENABLE_H:
        case IDC_CHECKBOX_ENABLE_DX:
        case IDC_CHECKBOX_ENABLE_DY:
        case IDC_CHECKBOX_ENABLE_FONT:
        case IDC_CHECKBOX_ENABLE_ALIGN:
        case IDC_CHECKBOX_ENABLE_ALIGN2:
        case IDC_CHECKBOX_PSWLEVEL:

        case IDC_CHECKBOX_ENABLE_PRF_V:
        case IDC_CHECKBOX_ENABLE_ADDR_V:
        case IDC_CHECKBOX_ENABLE_NBITS_V:
        case IDC_CHECKBOX_ENABLE_OFFSET_V:
        case IDC_CHECKBOX_ENABLE_NEG_V:
        case IDC_CHECKBOX_ENABLE_SIMPLE_TXT:
        case IDC_CHECK_ENABLE_MOV:
          checkEnabled(LOWORD(wParam));
          break;

        }
      break;

    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmCommonDialog::CmOk()
{
  getEnabled();

  int t;
  GET_INT(IDC_EDIT_DX, t);
  tmpProp->type1 = R__X(t);

  GET_INT(IDC_EDIT_DY, t);
  tmpProp->type2 = R__X(t);

  uint valign = 0;
  if(IS_CHECKED(IDC_RADIOBUTTON_A_LEFT2))
    valign = Property::aTop;
  else if(IS_CHECKED(IDC_RADIOBUTTON_A_RIGHT2))
    valign = Property::aBottom;
  else
    valign = Property::aMid;

  baseClass::CmOk();
  Prop->alignText |= valign;
  if(bitsChange & ( 1 << CHANGED_SIMPLE_TEXT)) {
    TCHAR t[4096];
    GET_TEXT(IDC_EDIT_SAMPLE_TEXT, t);
    simpleText = str_newdup(t);
    }
}
//-----------------------------------------------------------
void svmCommonDialog::getEnabled()
{
  bitsChange = 0;
  for(uint i = 0; i < SIZE_A(EnableBy); ++i)
    if(IS_CHECKED(EnableBy[i].check))
      bitsChange |= 1 << i;
}
