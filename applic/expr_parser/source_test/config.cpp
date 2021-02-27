//---------------- config.cpp ---------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <commdlg.h>
#include "config.h"
#include "POwnBtnImageStd.h"
#include "defin.h"
#include "macro_utils.h"
//-----------------------------------------------------------
template <typename T>
void safeDeleteObject(T& obj)
{
  if(obj) {
    DeleteObject((HGDIOBJ)obj);
    obj = 0;
    }
}
//-------------------------------------------------------------------
class colorStatic : public PStatic
{
  private:
    typedef PStatic baseClass;
  public:
    colorStatic(PWin * parent, uint resid, HINSTANCE hinst = 0) :
      baseClass(parent, resid, hinst), Brush(0), Fg(0), Bg(RGB(255, 255, 255))
    {   }
    ~colorStatic() { destroy(); safeDeleteObject(Brush); }

    HBRUSH evCtlColor(HDC hdc)
    {
      if(Brush) {
        SetTextColor(hdc, Fg);
        SetBkColor(hdc, Bg);
        }
      return Brush;
      }
    COLORREF getFg() const { return Fg; }
    COLORREF getBg() const { return Bg; }
    void setFg(COLORREF fg) {
      if(Fg != fg) {
        Fg = fg;
        InvalidateRect(*this, 0, 1);
        }
      }
    void setBg(COLORREF bg) {
      if(Bg != bg || !Brush) {
        Bg = bg;
        safeDeleteObject(Brush);
        Brush = CreateSolidBrush(Bg);
        InvalidateRect(*this, 0, 1);
        }
      }
//    bool create();
  protected:
    HBRUSH Brush;
    COLORREF Fg;
    COLORREF Bg;
};
//-------------------------------------------------------------------
static uint idcStatic[] = {IDC_STATIC1, IDC_STATIC2, IDC_STATIC3, IDC_STATIC4, IDC_STATIC5 };
//-------------------------------------------------------------------
class PDialogBaseColor : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PDialogBaseColor(PWin* parent, uint id, infoEdit& iedit, DWORD enableBits);
    ~PDialogBaseColor() { destroy();  }
    bool create();
    bool save();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC hdc, HWND hwChild, UINT ctlType);
    void chooseColor(int grp);
    void chooseFont();
    colorStatic* Color[SIZE_A(idcStatic)];
    infoEdit& iEdit;
    DWORD enableBits;
};
//-------------------------------------------------------------------
static bool choose_Color(HWND parent, COLORREF& target)
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
    return true;
    }
  return false;
}
//-----------------------------------------------------------
static
void setNameFont(PWin* owner, uint idc, const infoEdit& iedit, bool usefont)
{
  TCHAR buff[200];
  TCHAR style[20] = _T("\0");
  if(iedit.fontStyle & fBOLD)
    _tcscpy_s(style, _T("Bold"));
  else if(fLIGHT & iedit.fontStyle)
    _tcscpy_s(style, _T("Light"));
  else if(iedit.fontStyle & fITALIC)
    _tcscpy_s(style, _T("Italic"));

  if(!style[0]) {
    if(iedit.fontStyle & fITALIC)
      _tcscpy_s(style, _T("Italic"));
    else
      _tcscpy_s(style, _T("Normal"));
    }
  else if(_tcscmp(style, _T("Italic")))
    if(iedit.fontStyle & fITALIC)
      _tcscat_s(style, _T("-Italic"));

  wsprintf(buff, _T("%s - %d - %s"), iedit.fontName, iedit.fontHeight, style);
  SetDlgItemText(*owner, idc, buff);
  if(usefont) {
    PStatic* t = dynamic_cast<PStatic*>(PWin::getWindowPtr(GetDlgItem(*owner, idc)));
    if(t)
      t->setFont(D_FONT(iedit.fontHeight, 0, iedit.fontStyle, iedit.fontName),true);
    }
}
//-----------------------------------------------------------
static void fillLogFont(HWND hwnd, LOGFONT& lf, const infoEdit& iedit)
{
  ZeroMemory(&lf, sizeof(lf));
  HDC hdc = GetDC(hwnd);
  lf.lfHeight = -MulDiv(iedit.fontHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
  ReleaseDC(hwnd, hdc);
  lf.lfItalic = fITALIC & iedit.fontStyle;
  lf.lfWeight = (iedit.fontStyle & fBOLD) ? FW_BOLD :
                (fLIGHT & iedit.fontStyle) ? FW_LIGHT : FW_NORMAL;
  _tcscpy_s(lf.lfFaceName, iedit.fontName);
}
//-----------------------------------------------------------
static void unfillLogFont(HWND hwnd, const LOGFONT& lf, infoEdit& iedit)
{
  HDC hdc = GetDC(hwnd);
  iedit.fontHeight = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
  ReleaseDC(hwnd, hdc);
  iedit.fontStyle = 0;
  if(lf.lfItalic)
    iedit.fontStyle |= fITALIC;
  switch(lf.lfWeight) {
    case FW_BOLD:
      iedit.fontStyle |= fBOLD;
      break;
    case FW_LIGHT:
      iedit.fontStyle |= fLIGHT;
      break;
    case FW_NORMAL:
    default:
      break;
    }
  _tcscpy_s(iedit.fontName, lf.lfFaceName);
}
//-----------------------------------------------------------
#define FLAGS_FONT CF_SCALABLEONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT
//#define FLAGS_FONT CF_SCALABLEONLY | CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT
//-----------------------------------------------------------
static bool chooseFont(HWND hwnd, infoEdit& iedit)
{
  LOGFONT lf;
  fillLogFont(hwnd, lf, iedit);

  CHOOSEFONT cf;
  memset(&cf, 0, sizeof(cf));
  cf.lStructSize = sizeof(cf);
  cf.hwndOwner = hwnd;
  cf.lpLogFont = &lf;
  cf.Flags = FLAGS_FONT;
  cf.nFontType = SCREEN_FONTTYPE | REGULAR_FONTTYPE | ITALIC_FONTTYPE | BOLD_FONTTYPE;
  if(ChooseFont(&cf)) {
    unfillLogFont(hwnd, lf, iedit);
    return true;
    }
  return false;
}
//-------------------------------------------------------------------
PDialogBaseColor::PDialogBaseColor(PWin* parent, uint id, infoEdit& iedit, DWORD enableBits) :
    baseClass(parent, id), iEdit(iedit), enableBits(enableBits)
{
}
//-------------------------------------------------------------------
bool PDialogBaseColor::create()
{
  for(uint i = 0; i < SIZE_A(idcStatic); ++i)
    Color[i] = new colorStatic(this, idcStatic[i]);
  if(!baseClass::create())
    return false;
  Color[0]->setFg(iEdit.fg[0]);
  Color[0]->setBg(iEdit.bg);
  for(uint i = 1; i < SIZE_A(idcStatic); ++i) {
    Color[i]->setFg(iEdit.fg[i - 1]);
    Color[i]->setBg(iEdit.bg);
    }
  uint idcb[] = {IDC_BUTTON1, IDC_BUTTON2, IDC_BUTTON3, IDC_BUTTON4, IDC_BUTTON5 };
  DWORD bits = enableBits;
  for(uint i = 0; i < SIZE_A(idcStatic); ++i, bits >>= 1) {
    if(!(bits & 1)) {
      ENABLE(idcb[i], false);
      SET_TEXT(idcStatic[i], _T("--------"));
      }
    }
  setNameFont(this, IDC_STATIC_FONT, iEdit, false);
  return true;
}
//-------------------------------------------------------------------
bool PDialogBaseColor::save()
{
  iEdit.bg = Color[0]->getBg();
  for(uint i = 0; i < SIZE_A(iEdit.fg); ++i)
    iEdit.fg[i] = Color[i + 1]->getFg();
  return true;
}
//-------------------------------------------------------------------
HBRUSH PDialogBaseColor::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  PWin* w = getWindowPtr(hWndChild);
  if(!w)
    return 0;
  colorStatic* cS = getParentWin<colorStatic>(w);
  if(cS)
    return cS->evCtlColor(hdc);
  return 0;
}
//-------------------------------------------------------------------
LRESULT PDialogBaseColor::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_FONT:
          chooseFont();
          break;
        case IDC_BUTTON1:
          chooseColor(0);
          break;
        case IDC_BUTTON2:
          chooseColor(1);
          break;
        case IDC_BUTTON3:
          chooseColor(2);
          break;
        case IDC_BUTTON4:
          chooseColor(3);
          break;
        case IDC_BUTTON5:
          chooseColor(4);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
void PDialogBaseColor::chooseFont()
{
  if(::chooseFont(*this, iEdit)) {
    setNameFont(this, IDC_STATIC_FONT, iEdit, false);
    InvalidateRect(GetDlgItem(*this, IDC_STATIC_FONT), 0, 1);
    }
}
//-------------------------------------------------------------------
void PDialogBaseColor::chooseColor(int ix)
{
  if((uint)ix >= SIZE_A(Color))
    return;
  COLORREF col = 0 == ix ? Color[ix]->getBg() : Color[ix]->getFg();
  if(choose_Color(*this, col)) {
    switch(ix) {
      case 0:
        for(uint i = 0; i < SIZE_A(Color); ++i)
          Color[i]->setBg(col);
        break;
      default:
        Color[ix]->setFg(col);
      }
    }
}
//-------------------------------------------------------------------
PDialogColor::PDialogColor(PWin* parent, uint id, infoEdit* iedit, HINSTANCE hInst)
  : baseClass(parent, id, hInst), iEdit(iedit), currPage(0) {}
//----------------------------------------------------------------------------
void PDialogColor::moveChild(PWin* child)
{
  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  PRect r;
  GetWindowRect(hTab, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
  SetWindowPos(*child, 0, 0, r.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
}
//-------------------------------------------------------------------
#define E_ALL     31
#define E_COMMENT 19
#define E_RESULT   3
//-------------------------------------------------------------------
bool PDialogColor::create()
{
  DWORD bitsEnable[] = { E_COMMENT, E_COMMENT, E_ALL, E_ALL, E_COMMENT, E_RESULT };
  for(uint i = 0; i < SIZE_A(Pages); ++i)
    Pages[i] = new PDialogBaseColor(this, IDD_BASE_SETUP, iEdit[i], bitsEnable[i]);
  if(!baseClass::create())
    return false;

  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  PRect r;
  GetWindowRect(hTab, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);

  TC_ITEM ti;
  memset(&ti, 0, sizeof(ti));
  ti.mask = TCIF_TEXT;

  LPTSTR tit[] = { _T("Vars"), _T("Init"), _T("Function"), _T("Calc"), _T("Test"), _T("Result") };
  for(uint i = 0; i < SIZE_A(tit); ++i) {
    ti.pszText = tit[i];
    ti.cchTextMax = _tcslen(ti.pszText);
    TabCtrl_InsertItem(hTab, i, &ti);
    SetWindowPos(*Pages[i], 0, 0, r.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
    }
  chgPage(currPage);
  TabCtrl_SetCurSel(hTab, currPage);
  return true;
}
//-----------------------------------------------------------------------------
LRESULT PDialogColor::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_NOTIFY:
      if(((NMHDR*)lParam)->code == TCN_SELCHANGE)
        chgPage(TabCtrl_GetCurSel(((NMHDR*)lParam)->hwndFrom));
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
#define STD_FLG \
  (SWP_NOMOVE | SWP_NOSIZE)
//----------------------------------------------------------------------------
#define STD_FLG_HIDE \
  (STD_FLG | SWP_HIDEWINDOW | SWP_NOZORDER)
//----------------------------------------------------------------------------
#define STD_FLG_SHOW \
  (STD_FLG | SWP_SHOWWINDOW)
//----------------------------------------------------------------------------
#define SHOW_W(a, p) \
  SetWindowPos((a), (p), 0, 0, 0, 0, STD_FLG_SHOW)
//----------------------------------------------------------------------------
#define HIDE_W(a) \
  ShowWindow((a), SW_HIDE)
//----------------------------------------------------------------------------
#define SHOW_HIDE(c, p) \
  {\
  HIDE_W(p);\
  SHOW_W(c, HWND_TOP);\
  }
//----------------------------------------------------------------------------
void PDialogColor::chgPage(int page)
{
  LockWindowUpdate(*this);
  if((uint)page < SIZE_A(Pages)) {
    SHOW_HIDE(*Pages[page], *Pages[currPage]);
    currPage = page;
    }
  LockWindowUpdate(0);
}
//----------------------------------------------------------------------------
void PDialogColor::CmOk()
{
  for(uint i = 0; i < SIZE_A(Pages); ++i)
    if(!Pages[i]->save())
      return;
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
DWORD PDialogSearchReplace::flag = 3;
DWORD PDialogSearchReplace::target = 2;
TCHAR PDialogSearchReplace::lastWord[512];
TCHAR PDialogSearchReplace::lastReplace[512];
//----------------------------------------------------------------------------
PDialogSearchReplace::PDialogSearchReplace(PWin* parent, uint id, exp_BaseContainer** owners, bool read_only, HINSTANCE hInst) :
  baseClass(parent, IDD_FIND_REPLACE, hInst), Owners(owners), readOnly(read_only)
{
}
//----------------------------------------------------------------------------
bool PDialogSearchReplace::create()
{
  if(!baseClass::create())
    return false;
  if(exp_BaseContainer::Case & flag)
    SET_CHECK(IDC_CHECK_CASE);
  if(exp_BaseContainer::Wholeword & flag)
    SET_CHECK(IDC_CHECK_WHOLE_WORD);
  if(!target)
    SET_CHECK(IDC_RADIO_VARS);
  else if(1 == target)
    SET_CHECK(IDC_RADIO_INIT);
  else if(2 == target)
    SET_CHECK(IDC_RADIO_CALC);
  SET_TEXT(IDC_EDIT_FIND, lastWord);
  SET_TEXT(IDC_EDIT_REPLACE, lastReplace);
  checkReadOnly();

  return true;
}
//----------------------------------------------------------------------------
void PDialogSearchReplace::checkReadOnly()
{
  uint ids[] = { IDC_REPLACE, IDC_REPLACE_NEXT, IDC_REPLACE_ALL, IDC_EDIT_REPLACE };
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], !readOnly);
}
//----------------------------------------------------------------------------
void PDialogSearchReplace::getFlag()
{
  flag = 0;
  if(IS_CHECKED(IDC_CHECK_CASE))
    flag |= exp_BaseContainer::Case;

  if(IS_CHECKED(IDC_CHECK_WHOLE_WORD))
    flag |= exp_BaseContainer::Wholeword;

  target = 0;
  if(IS_CHECKED(IDC_RADIO_INIT))
    target = 1;
  else if(IS_CHECKED(IDC_RADIO_CALC))
    target = 2;
  else if(IS_CHECKED(IDC_RADIO_FUNCT))
    target = 3;
}
//----------------------------------------------------------------------------
bool PDialogSearchReplace::preProcessMsg(MSG& msg)
{
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
LRESULT PDialogSearchReplace::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_FIND:
          find(false);
          break;
        case IDC_FIND_NEXT:
          find(true);
          break;
        case IDC_REPLACE:
          replace(false);
          break;
        case IDC_REPLACE_NEXT:
          replace(true);
          break;
        case IDC_REPLACE_ALL:
          replaceAll();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PDialogSearchReplace::setCurrSel(bool read_only)
{
  readOnly = read_only;
  checkReadOnly();
  getFlag();
  LPTSTR buff = Owners[target]->getSel();
  if(buff && *buff)
    SET_TEXT(IDC_EDIT_FIND, buff);
  delete []buff;
}
//----------------------------------------------------------------------------
void PDialogSearchReplace::find(bool next)
{
  TCHAR word[256];
  GET_TEXT(IDC_EDIT_FIND, word);
  if(!*word)
    return;
  getFlag();
  if(!Owners[target]->select(word, flag, next)) {
    if(next)
      MessageBox(*this, _T("Ricerca finita!"), _T("Attenzione"), MB_OK | MB_ICONINFORMATION);
    else
      MessageBox(*this, _T("Parola non trovata!"), _T("Attenzione"), MB_OK | MB_ICONSTOP);
    }
}
//----------------------------------------------------------------------------
void PDialogSearchReplace::replace(bool next)
{
  TCHAR word[256];
  GET_TEXT(IDC_EDIT_FIND, word);
  if(!*word)
    return;
  getFlag();
  TCHAR newword[256];
  GET_TEXT(IDC_EDIT_REPLACE, newword);
  if(!Owners[target]->replace(newword, word, flag, next)) {
    if(next)
      MessageBox(*this, _T("Ricerca finita!"), _T("Attenzione"), MB_OK | MB_ICONINFORMATION);
    else
      MessageBox(*this, _T("Parola non trovata!"), _T("Attenzione"), MB_OK | MB_ICONSTOP);
    }
}
//----------------------------------------------------------------------------
void PDialogSearchReplace::replaceAll()
{
  TCHAR word[256];
  GET_TEXT(IDC_EDIT_FIND, word);
  if(!*word)
    return;
  getFlag();
  TCHAR newword[256];
  GET_TEXT(IDC_EDIT_REPLACE, newword);
  if(!Owners[target]->replaceAll(newword, word, flag))
    MessageBox(*this, _T("Parola non trovata!"), _T("Attenzione"), MB_OK | MB_ICONSTOP);
}
//----------------------------------------------------------------------------
#include "test_expr.h"
//----------------------------------------------------------------------------
void PDialogSearchReplace::CmCancel()
{
  getFlag();
  GET_TEXT(IDC_EDIT_FIND, lastWord);
  GET_TEXT(IDC_EDIT_REPLACE, lastReplace);

  test_Expr* te = getParentWin<test_Expr>(this);
  if(te)
    te->deleteSearchReplace();
  else
    baseClass::CmCancel();
}
