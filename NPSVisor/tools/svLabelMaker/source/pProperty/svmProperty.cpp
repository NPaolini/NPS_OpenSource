//--------------- svmProperty.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmProperty.h"
#include "svmObject.h"
#include "pListbox.h"
#include "pStatic.h"
#include "macro_utils.h"
#include "svmMainWorkArea.h"
#include "svmDataProject.h"
#include "pPrintPage.h"
//-----------------------------------------------------------
#define SET_MODIFIED_ADD(b, v, add) \
    if(uP.flagBits & PROP_BIT_POS(b)) { \
      v = uP.v + add;  modified = true; }
//-----------------------------------------------------------
bool Property::setCommon(const unionProperty& uP)
{
  bool modified = false;
/* eupX, eupY, eupW, eupH, eupDX, eupDY */
  SET_MODIFIED(eupX, Rect.left)
  SET_MODIFIED(eupY, Rect.top)
  SET_MODIFIED_ADD(eupW, Rect.right, Rect.left)
  SET_MODIFIED_ADD(eupH, Rect.bottom, Rect.top)
  if(uP.flagBits & PROP_BIT_POS(eupDX)) {
    Rect.Offset(uP.dX, 0);
    modified = true;
    }
  if(uP.flagBits & PROP_BIT_POS(eupDY)) {
    Rect.Offset(0, uP.dY);
    modified = true;
    }

  modified |= setOtherCommon(uP);
  return modified;
}
//-----------------------------------------------------------
#define SET_INIT_UNION_ADD(b, v, add) \
    if(!(uProp.flagBits & PROP_BIT_POS(b))) { \
      uProp.v = v + add; }
//-----------------------------------------------------------
void Property::initUnion(unionProperty& uProp)
{
  SET_INIT_UNION(eupX, Rect.left)
  SET_INIT_UNION(eupY, Rect.top)
  SET_INIT_UNION_ADD(eupW, Rect.right, Rect.left)
  SET_INIT_UNION_ADD(eupH, Rect.bottom, Rect.top)
}
//-----------------------------------------------------------
void clearDual()
{
  svmFont& f = getFontObj();
  f.flush();
  svmFont& f2 = getFontObjLinked();
  f2.flush();

  dual* bm = getDualBmp();
  bm->reset();
  dualPen* p = getDualPen();
  p->reset();
  dualBrush* b = getDualBrush();
  b->reset();
  lastPropertyInUse* prop = getLastPropertyInUse();
  prop->reset();
}
//-----------------------------------------------------------
class svmFactoryFont
{
  public:
    svmFactoryFont() {}
    ~svmFactoryFont() {}
    svmFont& getFontObj()
    {
      const dataProject& dp = getDataProject();
      return FontObj[dp.currPage];
    }

  private:
    svmFont FontObj[MAX_PAGES];
};
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
#define RET_ON_LESS(a) if(a < other.a) return true
//-----------------------------------------------------------
bool typeFont::operator <(const typeFont& other)
{
  RET_ON_LESS(h);
  RET_ON_LESS(w);
  RET_ON_LESS(Angle);
  RET_ON_LESS(italic);
  RET_ON_LESS(bold);
  RET_ON_LESS(underl);
  RET_ON_LESS(light);
  if(name && other.name)
    return _tcsicmp(name, other.name) != 0;
  return false;
}
//-----------------------------------------------------------
void typeFont::setFlag(uint flag)
{
  italic = toBool(flag & fITALIC);
  bold = toBool(flag & fBOLD);
  underl = toBool(flag & fUNDERLINED);
  light = toBool(flag & fLIGHT);
}
//-----------------------------------------------------------
uint typeFont::getFlag() const
{
  int flag = 0;
  if(italic)
    flag |= fITALIC;
  if(underl)
    flag |= fUNDERLINED;
  if(bold)
    flag |= fBOLD;
  if(light)
    flag |= fLIGHT;
  return flag;
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
  tpf->h = (WORD)12;
  tpf->hFont = D_FONT(tpf->h, 0, 0, tpf->name);
  fontSet[0] = tpf;
}
//-----------------------------------------------------------
typedef typeFont* pTypeFont;
//-----------------------------------------------------------
int fz_cmp_font(const pTypeFont& ins, const pTypeFont& v)
{
  if(*v < *ins)
    return 1;
  if(*ins < *v)
    return -1;
  return 0;
}
//-----------------------------------------------------------
int svmFont::addFont(typeFont* tpF)
{
  uint pos;
  if(fontSet.find(tpF, pos, fz_cmp_font, false)) {
    delete tpF;
    }
  else {
    pos = fontSet.getElem();
    fontSet[pos] = tpF;
    }
  return pos;
}
//-----------------------------------------------------------
HFONT svmFont::getFont(uint id)
{
  uint nelem = fontSet.getElem();
  if(id >= nelem)
    id = 0;
  const typeFont* tpf = fontSet[id];
  if(!tpf->hFont)
    fontSet[id]->hFont = D_FONT(tpf->h, tpf->w, tpf->getFlag(), tpf->name);
  return tpf->hFont;
}
//-----------------------------------------------------------
HFONT svmFont::getFont2(uint id, short angle)
{
  uint nelem = fontSet.getElem();
  if(id >= nelem)
    id = 0;
  typeFont* tpf = fontSet[id];

  int flag = tpf->getFlag();
#if 1
#define INCH_TO_MM(h) MulDiv((h), 254, 100)
  POINT pt = { INCH_TO_MM(tpf->w), -INCH_TO_MM(tpf->h) };
#undef INCH_TO_MM
#else
  HDC hdc = GetDC(NULL);
  SetMapMode(hdc, MM_LOMETRIC);
  POINT pt = { tpf->w, tpf->h };
  DPtoLP(hdc, &pt, 1);
  SetMapMode(hdc, MM_TEXT);
  ReleaseDC(NULL, hdc);
#endif
  return D_FONT_ORIENT(pt.y, pt.x, angle, flag, tpf->name);
}
//-----------------------------------------------------------
HFONT svmFont::getFont3(uint id, short angle)
{
  uint nelem = fontSet.getElem();
  if(id >= nelem)
    id = 0;
  typeFont* tpf = fontSet[id];

  int flag = tpf->getFlag();
#define INCH_TO_MM_R(h) ROUND_REAL((h) * 2.54)
  POINT pt = { INCH_TO_MM_R(tpf->w), -INCH_TO_MM_R(tpf->h) };
#undef INCH_TO_MM_R
  return D_FONT_ORIENT(pt.y, pt.x, angle, flag, tpf->name);
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
//-----------------------------------------------------------
void chooseColor(HWND hwnd, uint idc, HBRUSH& hb, COLORREF& color)
{
  COLORREF old = color;
  if(choose_Color(hwnd, old) && old != color) {
    color = old;
    DeleteObject(hb);
    hb = CreateSolidBrush(color);
    InvalidateRect(GetDlgItem(hwnd, idc), 0, 1);
    }
}
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
    UINT state = DFCS_MENUBULLET | DFCS_TRANSPARENT;
    r2.Inflate(-1, -2);
    DrawFrameControl(hDC, r2, DFC_MENU, state);
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
#define LEN_TYPEFONT (SIZE_A(sBOLD) + SIZE_A(sITALIC) + SIZE_A(sUNDERL) - 2)

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
          invalidateWorkArea(this);
/*
          do {
            PWin* mc = this;
            while(mc->getParent()->getParent())
              mc = mc->getParent();
            InvalidateRect(*mc, 0, 0);
            } while(false);
*/
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
  HDC hdc = GetDC(hwnd);
  lf.lfHeight = -MulDiv(tpf->h, GetDeviceCaps(hdc, LOGPIXELSY), 72);
  ReleaseDC(hwnd, hdc);
  lf.lfItalic = tpf->italic;
  lf.lfWeight = tpf->bold ? FW_BOLD : tpf->light ? FW_LIGHT : FW_NORMAL;
  _tcscpy_s(lf.lfFaceName, tpf->name);
}
//-----------------------------------------------------------
#define FLAGS_FONT CF_SCALABLEONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT

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
    int flag = tpf->getFlag();
    tpf->hFont = D_FONT(tpf->h, tpf->w, flag, tpf->name);

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
  childScrollWorkArea* mc = getParentWin<childScrollWorkArea>(this);
  if(mc && mc->isUsedFontId(sel))
    return;
  svmFont& f = getFontObj();
  f.remove(sel);
  SendMessage(*LB, LB_DELETESTRING, sel, 0);
}
//-----------------------------------------------------------
void svmChooseFont::addToLb(const typeFont* tpf, int pos)
{
  TCHAR buff[MAX_LEN_LISTFONT + 2];
  LPTSTR p = buff;
  fillStr(p, _T(' '), SIZE_A(buff));
  if(pos < 0)
    pos = SendMessage(*LB, LB_GETCOUNT, 0, 0);

  childScrollWorkArea* mc = getParentWin<childScrollWorkArea>(this);
  if(mc && mc->isUsedFontId(pos))
    *p = _T('1');
  p += LEN_DIMFLAG;
  *p++ = TAB;

  TCHAR t[30];
  wsprintf(t, _T("%d"), tpf->h);
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
//-----------------------------------------------------------
template <typename T>
class svmFactoryObjPage
{
  public:
    svmFactoryObjPage() {}
    ~svmFactoryObjPage() {}
    T& getObjPage()
    {
      const dataProject& dp = getDataProject();
      return ObjPage[dp.currPage];
    }

  private:
    T ObjPage[MAX_PAGES];
};
//-----------------------------------------------------------
template <typename T, int idInit, int preserveFirst>
int dualT<T, idInit, preserveFirst>::addAndGetId(T v)
{
  int nElem = Lnk.getElem();
  for(int i = 0; i < nElem; ++i)
    if(!cmp(v, Lnk[i].var))
      return Lnk[i].id;
  Lnk[nElem].id = currCount;
  Lnk[nElem].var = clone(v);
  return currCount++;
}
//-----------------------------------------------------------
template <typename T, int idInit, int preserveFirst>
int dualT<T, idInit, preserveFirst>::findId(T v) const
{
  int nElem = Lnk.getElem();
  for(int i = 0; i < nElem; ++i)
    if(!cmp(v, Lnk[i].var))
      return Lnk[i].id;
  return 0;
}
//-----------------------------------------------------------
template <typename T, int idInit, int preserveFirst>
T dualT<T, idInit, preserveFirst>::getIt(int id) const
{
  int nElem = Lnk.getElem();
  for(int i = 0; i < nElem; ++i)
    if(Lnk[i].id == id)
      return Lnk[i].var;
  return 0;
}
//-----------------------------------------------------------
template <typename T, int idInit, int preserveFirst>
void dualT<T, idInit, preserveFirst>::reset()
{
  int nElem = Lnk.getElem();
  for(int i = preserveFirst; i < nElem; ++i)
    freeVar(Lnk[i].var);
  Lnk.setElem(preserveFirst);
  currCount = idInit + preserveFirst;
}
//-----------------------------------------------------------
template <typename T, int idInit, int preserveFirst>
void dualT<T, idInit, preserveFirst>::releaseAll()
{
  int nElem = Lnk.getElem();
  for(int i = 0; i < nElem; ++i)
    freeVar(Lnk[i].var);
  Lnk.reset();
  currCount = idInit + preserveFirst;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
class dualBmp : public dual
{
  private:
    typedef dual baseClass;
  public:
    dualBmp() {}
    virtual ~dualBmp() { releaseAll(); }

//    virtual bool save(P_File& pf);
//    virtual bool load(setOfString& set);
  protected:
    virtual LPCTSTR clone(LPCTSTR var) { return str_newdup(var); }
    virtual int cmp(LPCTSTR v1, LPCTSTR v2) const { return _tcsicmp(v1, v2); }
    virtual void freeVar(LPCTSTR v) { delete []v; }
};
//-----------------------------------------------------------
static svmFactoryObjPage<dualBmp> Dual_Bmp;
//-----------------------------------------------------------
dual* getDualBmp()
{
  return &Dual_Bmp.getObjPage();
}
//-----------------------------------------------------------
infoPen::infoPen(const infoPen& other) : hpen(0)
{
  CLONE(tickness);
  CLONE(color);
  CLONE(style);
  hpen = CreatePen(style, tickness, color);
}
//-----------------------------------------------------------
const infoPen& infoPen::operator =(const infoPen& other)
{
  if(this == &other)
    return *this;
  if(hpen)
    DeleteObject(hpen);
  CLONE(tickness);
  CLONE(color);
  CLONE(style);
  hpen = CreatePen(style, tickness, color);
  return *this;
}
//-----------------------------------------------------------
class dual_Pen : public dualPen
{
  private:
    typedef dualPen baseClass;
  public:
    dual_Pen() { infoPen ip; addAndGetId(&ip); }
    virtual ~dual_Pen() { releaseAll(); }

//    virtual bool save(P_File& pf);
//    virtual bool load(setOfString& set);
  protected:
    virtual pInfoPen clone(pInfoPen var) { return new infoPen(*var); }
    virtual int cmp(pInfoPen v1, pInfoPen v2) const;
    virtual void freeVar(pInfoPen v) { delete v; }
};
//-----------------------------------------------------------
#define CMP_RET(v) if(v1->v < v2->v) return -1; if(v2->v < v1->v) return 1
//-----------------------------------------------------------
int dual_Pen::cmp(pInfoPen v1, pInfoPen v2) const
{
  CMP_RET(color);
  CMP_RET(tickness);
  CMP_RET(style);
  return 0;
}
//-----------------------------------------------------------
static svmFactoryObjPage<dual_Pen> Dual_Pen;
//-----------------------------------------------------------
dualPen* getDualPen()
{
  return &Dual_Pen.getObjPage();
}
//-----------------------------------------------------------
infoBrush::infoBrush(const infoBrush& other) : hbrush(0)
{
  CLONE(color);
  CLONE(style);
  if(cNULL_COLOR == color)
    hbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
  else if(style < 0)
    hbrush = CreateSolidBrush(color);
  else
    hbrush = CreateHatchBrush(style, color);
}
//-----------------------------------------------------------
const infoBrush& infoBrush::operator =(const infoBrush& other)
{
  if(this == &other)
    return *this;
  if(hbrush)
    DeleteObject(hbrush);
  CLONE(color);
  CLONE(style);
  if(cNULL_COLOR == color)
    hbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
  else if(style < 0)
    hbrush = CreateSolidBrush(color);
  else
    hbrush = CreateHatchBrush(style - 1, color);
  return *this;
}
//-----------------------------------------------------------
class dual_Brush : public dualBrush
{
  private:
    typedef dualBrush baseClass;
  public:
    dual_Brush() { infoBrush ib; addAndGetId(&ib);  }
    virtual ~dual_Brush() { releaseAll(); }

  protected:
    virtual pInfoBrush clone(pInfoBrush var) { return new infoBrush(*var); }
    virtual int cmp(pInfoBrush v1, pInfoBrush v2) const;
    virtual void freeVar(pInfoBrush v) { delete v; }
};
//-----------------------------------------------------------
int dual_Brush::cmp(pInfoBrush v1, pInfoBrush v2) const
{
  CMP_RET(color);
  CMP_RET(style);
  return 0;
}
//-----------------------------------------------------------
static svmFactoryObjPage<dual_Brush> Dual_Brush;
//-----------------------------------------------------------
dualBrush* getDualBrush()
{
  return &Dual_Brush.getObjPage();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
Property* lastPropertyInUse::getLast(uint idType)
{
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(set[i].idType == idType) {
      return set[i].prop;
      }
    }
  return 0;
}
//-----------------------------------------------------------
void lastPropertyInUse::replaceLast(svmObject* obj)
{
  int nElem = set.getElem();
  int i = 0;
  for(; i < nElem; ++i) {
    if(set[i].idType == obj->getIdType()) {
      delete set[i].prop;
      break;
      }
    }
  set[i].prop = obj->allocProperty();
  set[i].idType = obj->getIdType();
  *set[i].prop = *obj->getProp();
}
//-----------------------------------------------------------
void lastPropertyInUse::reset()
{
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i)
    delete set[i].prop;
  set.reset();
}
//-----------------------------------------------------------
static svmFactoryObjPage<lastPropertyInUse> PropertyInUse;
//-----------------------------------------------------------
lastPropertyInUse* getLastPropertyInUse()
{
  return &PropertyInUse.getObjPage();
}
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
svmBaseDialogProperty::~svmBaseDialogProperty()
{
  delete tmpProp;
  destroy();
}
//-----------------------------------------------------------
bool svmBaseDialogProperty::create()
{
  if(!baseClass::create())
    return false;
  if(!Prop)
    CmCancel();

  tmpProp = allocProperty();
  *tmpProp = *Prop;

  return true;
}
//----------------------------------------------------------------------------
Property* svmBaseDialogProperty::allocProperty()
{
  return Owner->allocProperty();
}
//-----------------------------------------------------------
LRESULT svmBaseDialogProperty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
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
void svmBaseDialogProperty::CmOk()
{
  *Prop = *tmpProp;
  baseClass::CmOk();
}
//-----------------------------------------------------------
static LPCTSTR lineType[] =
{
  _T("Continua"),
  _T("Tratteggio"),
  _T("Punti"),
  _T("Tratto punto"),
  _T("Tratto punto punto"),
  _T("No linea"),
};
//-----------------------------------------------------------
void fillCB_PenType(HWND hCBx, bool alsoNoLine)
{
  for(uint i = 0; i < SIZE_A(lineType) - 1 + alsoNoLine; ++i)
    addStringToComboBox(hCBx, lineType[i]);
}
//-----------------------------------------------------------
static LPCTSTR brushType[] =
{
  _T("Trasparente"),
  _T("Pieno"),
  _T("Orizzontale ------"),
  _T("Verticale ||||||||"),
  _T("Diagonale \\\\\\\\\\\\\\\\"),
  _T("Diagonale ////////"),
  _T("Incrocio ++++++++"),
  _T("Incrocio xxxxxxxx"),
};
//-----------------------------------------------------------
void fillCB_BrushType(HWND hCBx)
{
  for(int i = 0; i < SIZE_A(brushType); ++i)
    addStringToComboBox(hCBx, brushType[i]);
}
//-----------------------------------------------------------
svmCommonDialog::~svmCommonDialog()
{
  destroy();
}
//-----------------------------------------------------------
#define ENABLE_CHECK(c, idc)  EnableWindow(GetDlgItem(*this, idc), toBool(bitsChange & BIT_POS(unionProperty::c)))
//-----------------------------------------------------------
bool svmCommonDialog::create()
{
  if(!baseClass::create())
    return false;
  do {
    TCHAR tit[_MAX_PATH];
    wsprintf(tit, _T("%d oggetti"), Prop.totObject);
    SetWindowText(*this, tit);
    } while(false);

  ENABLE_CHECK(eupX, IDC_CHECK_X);
  ENABLE_CHECK(eupY, IDC_CHECK_Y);
  ENABLE_CHECK(eupW, IDC_CHECK_W);
  ENABLE_CHECK(eupH, IDC_CHECK_H);
  ENABLE_CHECK(eupDX, IDC_CHECK_dX);
  ENABLE_CHECK(eupDY, IDC_CHECK_dY);
  ENABLE_CHECK(eupCX, IDC_CHECK_cX);
  ENABLE_CHECK(eupCY, IDC_CHECK_cY);
  ENABLE_CHECK(eupFG, IDC_CHECK_TEXT_COLOR);
  ENABLE_CHECK(eupPen, IDC_CHECK_PEN_COLOR);
  ENABLE_CHECK(eupBrush, IDC_CHECK_BRUSH_COLOR);
  ENABLE_CHECK(eupFont, IDC_CHECK_TEXT_FONT);
  ENABLE_CHECK(eupHAlign, IDC_CHECK_TEXT_AL_HORZ);
  ENABLE_CHECK(eupVAlign, IDC_CHECK_TEXT_AL_VERT);

  fillCB_PenType(GetDlgItem(*this, IDC_COMBO_PEN_TYPE), true);
  fillCB_BrushType(GetDlgItem(*this, IDC_COMBO_BRUSH_TYPE));

  SET_INT(IDC_EDIT_X, Prop.Rect.left);
  SET_INT(IDC_EDIT_Y, -Prop.Rect.top);
  SET_INT(IDC_EDIT_W, Prop.Rect.right);
  SET_INT(IDC_EDIT_H, -Prop.Rect.bottom);

  SET_INT(IDC_EDIT_cX2, Prop.cX);
  SET_INT(IDC_EDIT_cY2, Prop.cY);

  if(bitsChange & BIT_POS(unionProperty::eupFG)) {
    iColor[0].col = Prop.foreground;
    if(iColor[0].hb)
      DeleteObject(iColor[0].hb);
    iColor[0].hb = CreateSolidBrush(Prop.foreground);
    }

  if(bitsChange & BIT_POS(unionProperty::eupFont))
    idFont = Prop.idFont;

  if(PROP_BIT_POS(eupPen) & bitsChange) {
    dualPen* dp = getDualPen();
    const infoPen* ip = dp->getIt(Prop.idPen);
    if(ip) {
      SET_INT(IDC_EDIT_PEN_TICKNESS, ip->tickness);
      iColor[1].col = ip->color;
      if(iColor[1].hb)
        DeleteObject(iColor[1].hb);
      iColor[1].hb = CreateSolidBrush(ip->color);
      SendMessage(GetDlgItem(*this, IDC_COMBO_PEN_TYPE), CB_SETCURSEL, ip->style, 0);
      }
    }

  if(PROP_BIT_POS(eupBrush) & bitsChange) {
    dualBrush* db = getDualBrush();
    const infoBrush* ib = db->getIt(Prop.idBrush);
    if(ib) {
      iColor[2].col = ib->color;
      if(iColor[2].hb)
        DeleteObject(iColor[2].hb);
      iColor[2].hb = CreateSolidBrush(ib->color);
      SendMessage(GetDlgItem(*this, IDC_COMBO_BRUSH_TYPE), CB_SETCURSEL, ib->style + 2, 0);
      }
    }

  if(PROP_BIT_POS(eupHAlign) & bitsChange) {
    switch(Prop.hAlign) {
      case 0:
        SET_CHECK(IDC_RADIO_TEXT_SX);
        break;
      case 1:
        SET_CHECK(IDC_RADIO_TEXT_CENTER);
        break;
      default:
        SET_CHECK(IDC_RADIO_TEXT_DX);
        break;
      }
    }
  if(PROP_BIT_POS(eupVAlign) & bitsChange) {
    switch(Prop.vAlign) {
      case 0:
        SET_CHECK(IDC_RADIO_TEXT_TOP);
        break;
      case 1:
        SET_CHECK(IDC_RADIO_TEXT_MIDDLE);
        break;
      default:
        SET_CHECK(IDC_RADIO_TEXT_BOTTOM);
        break;
      }
    }
  checkEnabled();

  return true;
}

//-----------------------------------------------------------
static const uint idAllCtrl[] =
{
    IDC_EDIT_X,
    IDC_EDIT_Y,
    IDC_EDIT_W,
    IDC_EDIT_H,
    IDC_EDIT_dX,
    IDC_EDIT_dY,
    IDC_EDIT_cX2,
    IDC_EDIT_cY2,
    IDC_BUTTON_TEXT_COLOR,
    IDC_BUTTON_TEXT_FONT,
};
//-----------------------------------------------------------
static const uint idCtrlPen[] =
{
    IDC_BUTTON_PEN_COLOR,
    IDC_COMBO_PEN_TYPE,
    IDC_EDIT_PEN_TICKNESS,
};
//-----------------------------------------------------------
static const uint idCtrlBrush[] =
{
    IDC_BUTTON_BRUSH_COLOR,
    IDC_COMBO_BRUSH_TYPE,
};
//-----------------------------------------------------------
static const uint idCtrlAlign[] =
{
    IDC_RADIO_TEXT_SX,
    IDC_RADIO_TEXT_CENTER,
    IDC_RADIO_TEXT_DX,
};
//-----------------------------------------------------------
static const uint idCtrlAlign2[] =
{
    IDC_RADIO_TEXT_TOP,
    IDC_RADIO_TEXT_MIDDLE,
    IDC_RADIO_TEXT_BOTTOM,
};
//-----------------------------------------------------------
struct ctrlEnabledBy
{
  uint check;
  int num;
  const uint* set;
};
//-----------------------------------------------------------
/* N.B. deve mantenere lo stesso ordine dell'enum
    eupX, eupY, eupW, eupH, eupDX, eupDY, eupCX, eupCY,
    eupFG, eupPen, eupBrush, eupFont,
    eupHAlign, eupVAlign,
*/
static const ctrlEnabledBy EnableBy[] = {
  {  IDC_CHECK_X, 1, idAllCtrl + 0 },
  {  IDC_CHECK_Y, 1, idAllCtrl + 1 },
  {  IDC_CHECK_W, 1, idAllCtrl + 2 },
  {  IDC_CHECK_H, 1, idAllCtrl + 3 },
  {  IDC_CHECK_dX, 1, idAllCtrl + 4 },
  {  IDC_CHECK_dY, 1, idAllCtrl + 5 },
  {  IDC_CHECK_cX, 1, idAllCtrl + 6 },
  {  IDC_CHECK_cY, 1, idAllCtrl + 7 },
  {  IDC_CHECK_TEXT_COLOR, 1, idAllCtrl + 8 },

  {  IDC_CHECK_PEN_COLOR, SIZE_A(idCtrlPen), idCtrlPen },
  {  IDC_CHECK_BRUSH_COLOR, SIZE_A(idCtrlBrush), idCtrlBrush },
  {  IDC_CHECK_TEXT_FONT, 1, idAllCtrl + 9 },
  {  IDC_CHECK_TEXT_AL_HORZ, SIZE_A(idCtrlAlign), idCtrlAlign },
  {  IDC_CHECK_TEXT_AL_VERT, SIZE_A(idCtrlAlign2), idCtrlAlign2 },
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
  for(int i = 0; i < SIZE_A(EnableBy); ++i)
    if(-1 == ctrl || EnableBy[i].check == (uint)ctrl)
      enableCtrl(*this, EnableBy[i]);
}
//-----------------------------------------------------------
LRESULT svmCommonDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECK_X:
        case IDC_CHECK_Y:
        case IDC_CHECK_W:
        case IDC_CHECK_H:
        case IDC_CHECK_dX:
        case IDC_CHECK_dY:
        case IDC_CHECK_cX:
        case IDC_CHECK_cY:
        case IDC_CHECK_TEXT_COLOR:
        case IDC_CHECK_PEN_COLOR:
        case IDC_CHECK_BRUSH_COLOR:
        case IDC_CHECK_TEXT_FONT:
        case IDC_CHECK_TEXT_AL_HORZ:
        case IDC_CHECK_TEXT_AL_VERT:
          checkEnabled(LOWORD(wParam));
          break;
        case IDC_BUTTON_TEXT_COLOR:
          chooseColor(*this, IDC_STATIC_TEXT_COLOR, iColor[0].hb, iColor[0].col);
          break;
        case IDC_BUTTON_PEN_COLOR:
          chooseColor(*this, IDC_STATIC_PEN_COLOR, iColor[1].hb, iColor[1].col);
          break;
        case IDC_BUTTON_BRUSH_COLOR:
          chooseColor(*this, IDC_STATIC_BRUSH_COLOR, iColor[2].hb, iColor[2].col);
          break;

        case IDC_BUTTON_TEXT_FONT:
          chooseFont();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
HBRUSH svmCommonDialog::evCtlColor(HDC dc, HWND hWndChild)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  switch(id) {
    case IDC_STATIC_TEXT_COLOR:
      return iColor[0].hb;
    case IDC_STATIC_PEN_COLOR:
      return iColor[1].hb;
    case IDC_STATIC_BRUSH_COLOR:
      return iColor[2].hb;
    }
  return 0;
}
//-----------------------------------------------------------
void svmCommonDialog::chooseFont()
{
  if(IDOK == svmChooseFont(idFont, this).modal()) {
    svmFont& f = getFontObj();
    const typeFont* tf = f.getType(idFont);
    setNameFont(this, IDC_STATIC_TEXT_FONT, tf, true);
    }
}
//-----------------------------------------------------------
void svmCommonDialog::CmOk()
{
  getEnabled();

  GET_INT(IDC_EDIT_X, Prop.Rect.left);
  GET_INT(IDC_EDIT_Y, Prop.Rect.top);
  Prop.Rect.top = -Prop.Rect.top;
  GET_INT(IDC_EDIT_W, Prop.Rect.right);
  GET_INT(IDC_EDIT_H, Prop.Rect.bottom);
  Prop.Rect.bottom = -Prop.Rect.bottom;

  GET_INT(IDC_EDIT_cX2, Prop.cX);
  GET_INT(IDC_EDIT_cY2, Prop.cY);

  GET_INT(IDC_EDIT_dX, Prop.dX);
  GET_INT(IDC_EDIT_dY, Prop.dY);
  Prop.dY = -Prop.dY;

  Prop.foreground = iColor[0].col;

  if(idFont >= 0 && (PROP_BIT_POS(eupFont) & Prop.flagBits))
    Prop.idFont = idFont;

  if(PROP_BIT_POS(eupPen) & Prop.flagBits) {
    infoPen ip;
    GET_INT(IDC_EDIT_PEN_TICKNESS, ip.tickness);
    ip.color = iColor[1].col;
    ip.style = SendMessage(GetDlgItem(*this, IDC_COMBO_PEN_TYPE), CB_GETCURSEL, 0, 0);
    dualPen* dp = getDualPen();
    Prop.idPen = dp->addAndGetId(&ip);
    }

  if(PROP_BIT_POS(eupBrush) & Prop.flagBits) {
    infoBrush ib;
    ib.color = iColor[2].col;
    int t = SendMessage(GetDlgItem(*this, IDC_COMBO_BRUSH_TYPE), CB_GETCURSEL, 0, 0);
    --t;
    if(t < 0)
      ib.color = cNULL_COLOR;
    --t;
    ib.style = t;
    dualBrush* db = getDualBrush();
    Prop.idBrush = db->addAndGetId(&ib);
    }

  if(PROP_BIT_POS(eupHAlign) & Prop.flagBits)
    Prop.hAlign = IS_CHECKED(IDC_RADIO_TEXT_SX) ? 0 :
               IS_CHECKED(IDC_RADIO_TEXT_CENTER) ? 1 : 2;

  if(PROP_BIT_POS(eupVAlign) & Prop.flagBits)
    Prop.vAlign = IS_CHECKED(IDC_RADIO_TEXT_TOP) ? 0 :
               IS_CHECKED(IDC_RADIO_TEXT_MIDDLE) ? 1 : 2;

  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmCommonDialog::getEnabled()
{
  Prop.flagBits = 0;
  for(int i = 0; i < SIZE_A(EnableBy); ++i)
    if(IS_CHECKED(EnableBy[i].check))
      Prop.flagBits |= 1 << i;
}
