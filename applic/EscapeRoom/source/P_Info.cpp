//---------- P_Info.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_Info.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "popensave.h"
#include "mainwin.h"
#include "clientWin.h"
#include "p_txt.h"
#include "pMemo.h"
#include "pListbox.h"
#include <Shlwapi.h>
#include <Commdlg.h>
//----------------------------------------------------------------------------
COLORREF makeColor(LPCTSTR p);
//----------------------------------------------------------------------------
class myStatic : public PStatic
{
  private:
    typedef PStatic baseClass;
  public:
    myStatic(PWin * parent, uint resid, HINSTANCE hinst = 0) : baseClass(parent, resid, hinst), Color(RGB(255, 0, 0)) {}
    virtual ~myStatic() { destroy(); }
    virtual void setColor(COLORREF c) { Color = c; }
    virtual COLORREF getColor() { return Color; }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void evPaint(HDC hdc, PRect& r);
    virtual HBRUSH getBrush(bool &needDelete) { needDelete = false; return (HBRUSH)GetStockObject(LTGRAY_BRUSH); }
    COLORREF Color;
};
//----------------------------------------------------------------------------
class myStaticColor : public myStatic
{
  private:
    typedef myStatic baseClass;
  public:
    myStaticColor(PWin * parent, uint resid, HINSTANCE hinst = 0) : baseClass(parent, resid, hinst), HBr(0) {}
    ~myStaticColor()  { if(HBr) DeleteObject(HBr); destroy(); }
    void setColor(COLORREF c) {
      if(Color != c) {
        if(HBr)
          DeleteObject(HBr);
        HBr = CreateSolidBrush(c);
        Color = c;
        }
      }
  protected:
    HBRUSH HBr;
    virtual HBRUSH getBrush(bool &needDelete) {
      needDelete = false;
      if(!HBr)
        HBr = CreateSolidBrush(Color);
      return HBr;
      }
    void evPaint(HDC hdc, PRect& r) {}
};
//----------------------------------------------------------------------------
LRESULT myStatic::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
//    case WM_ERASEBKGND:
//      return 1;
    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        if(!hdc) {
          EndPaint(*this, &ps);
          return 0;
          }
        PRect r;
        GetClientRect(*this, r);
        if(r.Width() && r.Height()) {
          bool needDelete;
          HBRUSH br = getBrush(needDelete);
          FillRect(hdc, r, br);
          evPaint(hdc, r);
          if(needDelete)
            DeleteObject(br);
          }
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void myStatic::evPaint(HDC hdc, PRect& r)
{
  UINT format = DT_CENTER | DT_WORDBREAK | DT_EXPANDTABS;
  int oldMode = SetBkMode(hdc, TRANSPARENT);
  COLORREF old = SetTextColor(hdc, Color);
  HGDIOBJ oldf = SelectObject(hdc, getFont());
  DrawText(hdc, getCaption(), -1, r, format);
  SelectObject(hdc, oldf);
  SetBkMode(hdc, oldMode);
  SetTextColor(hdc, old);
}
//----------------------------------------------------------------------------
class P_Text : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    P_Text(PWin* parent, LPTSTR buff, uint dim, uint resId = IDD_DIALOG_ADD_MOD_TEXT, HINSTANCE hinstance = 0) :
      baseClass(parent, resId, hinstance), Buff(buff), Dim(dim) {}
    virtual ~P_Text() { destroy(); }

    virtual bool create();
  protected:
    virtual void CmOk();
  private:
    LPTSTR Buff;
    uint Dim;
};
//----------------------------------------------------------------------------
bool P_Text::create()
{
  PMemo* m = new PMemo(this, IDC_EDIT_ADD_MOD);
  HFONT hf = DEF_FONT(20);
  m->setFont(hf, true);
  new POwnBtnImageStd(this, IDOK, new PBitmap(IDB_BITMAP_OK, getHInstance()), POwnBtnImageStd::wLeft, true);
  new POwnBtnImageStd(this, IDCANCEL, new PBitmap(IDB_BITMAP_CANC, getHInstance()), POwnBtnImageStd::wLeft, true);
  if(!baseClass::create())
    return false;
  SET_TEXT(IDC_EDIT_ADD_MOD, Buff);
  return true;
}
//----------------------------------------------------------------------------
void P_Text::CmOk()
{
  HWND hwnd = GetDlgItem(*this, IDC_EDIT_ADD_MOD);
  GetWindowText(hwnd, Buff, Dim);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
P_Info::P_Info(bool& force_refresh, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), TextColor(RGB(255,0,0)), forceRefresh(force_refresh)
{
  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_FOLDER, IDB_BITMAP_OK, IDB_BITMAP_CANC, IDB_BITMAP_ADD, IDB_BITMAP_MOD, IDB_BITMAP_REM, IDB_BITMAP_FONT, IDB_BITMAP_COLOR };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDC_BUTTON_FONT, Bmp[6]);
  new POwnBtnImageStd(this, IDC_BUTTON_COLOR, Bmp[7]);

  PListBox* lb = new PListBox(this, IDC_LIST_TEXT);
  HFONT hf = DEF_FONT(20);
  lb->setFont(hf, true);

  new POwnBtnImageStd(this, IDC_BUTTON_ADD, Bmp[3]);
  new POwnBtnImageStd(this, IDC_BUTTON_MOD, Bmp[4]);
  new POwnBtnImageStd(this, IDC_BUTTON_DEL, Bmp[5]);

  new POwnBtnImageStd(this, IDC_BUTTON_IMG, Bmp[0]);

  new POwnBtnImageStd(this, IDOK, Bmp[1]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[2]);
  curr = new myStatic(this, IDC_STATIC_TEXT);
  color = new myStaticColor(this, IDC_STATIC_COLOR);

  HFONT hf2 = DEF_FONT(20);
  curr->setFont(hf2, true);
}
//----------------------------------------------------------------------------
P_Info::~P_Info()
{
  destroy();
  flushPV(Bmp);
}
//----------------------------------------------------------------------------
void P_Info::BNClickedOk()
{
  mainWin* mw = getParentWin<mainWin>(this);
  if(!mw)
    return;
  TCHAR buff[4096];
  getKeyPath(TEXT_FONT, buff);
  TCHAR b[4096];
  GET_TEXT(IDC_EDIT_FONT, b);
  forceRefresh |= toBool(_tcsicmp(buff, b));
  if(forceRefresh)
    setKeyPath(TEXT_FONT, b);
  getKeyPath(TEXT_COLOR, buff);
  COLORREF c = makeColor(buff);
  if(TextColor != c) {
    forceRefresh = true;
    wsprintf(buff, _T("%d,%d,%d"), GetRValue(TextColor), GetGValue(TextColor), GetBValue(TextColor));
    setKeyPath(TEXT_COLOR, buff);
    }
  getKeyPath(TEXT_BORDER, buff);
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBO_BORDER), CB_GETCURSEL, 0, 0);
  if(sel != _ttoi(buff)) {
    forceRefresh = true;
    setKeyParam(TEXT_BORDER, sel);
    }

  sel = 0;
  if(IS_CHECKED(IDC_CHECK_USE_TEXT)) {
    sel = SendMessage(GetDlgItem(*this, IDC_LIST_TEXT), LB_GETCURSEL, 0, 0);
    if(sel >= 0)
      ++sel;
    }

  int old;
  getKeyParam(TEXT_CURR, (LPDWORD)&old);
  if(old != sel) {
    forceRefresh = true;
    setKeyParam(TEXT_CURR, sel);
    }
  else {
    uint nElem = modifRow.getElem();
    for(uint i = 0; i < nElem; ++i) {
      if(sel == modifRow[i]) {
        forceRefresh = true;
        break;
        }
      }
    }
  buff[0] = 0;
  if(IS_CHECKED(IDC_CHECK_USE_IMG)) {
    GET_TEXT(IDC_EDIT_IMG, buff);
    makeRelPath(buff, SIZE_A(buff));
    }
  getKeyPath(IMG_INFO, b);
  if(_tcsicmp(buff, b)) {
    forceRefresh = true;
    setKeyPath(IMG_INFO, buff);
    }

  getKeyPath(IMG_INFO_PERC, buff);
  GET_TEXT(IDC_EDIT_PERC, b);
  if(_tcsicmp(buff, b)) {
    forceRefresh = true;
    setKeyPath(IMG_INFO_PERC, b);
    }

  sel = IS_CHECKED(IDC_RADIO_IMG_UPPER);
  getKeyParam(IMG_INFO_TOP, (LPDWORD)&old);
  if(old != sel) {
    forceRefresh = true;
    setKeyParam(IMG_INFO_TOP, sel);
    }
//  if(needRefresh)
//    mw->refreshInfo();
}
//----------------------------------------------------------------------------
bool P_Info::create()
{
  if(!baseClass::create())
    return false;

  fillCtrl();
  return true;
}
//----------------------------------------------------------------------------
static int addString(HWND hList, LPCTSTR str, bool isLB = true)
{
  int wid = extent(hList, str);
  int msgGet;
  int msgSet;
  if(isLB) {
    msgGet = LB_GETHORIZONTALEXTENT;
    msgSet = LB_SETHORIZONTALEXTENT;
    }
  else  {
    msgGet = CB_GETDROPPEDWIDTH;
    msgSet = CB_SETDROPPEDWIDTH;
    }

  int awi = SendMessage(hList, msgGet, 0, 0);
  if(wid > awi) {
    SendMessage(hList, msgSet, wid, 0);
    }
  return SendMessage(hList, isLB ? LB_ADDSTRING : CB_ADDSTRING, 0, LPARAM(str));
}
//----------------------------------------------------------------------------
void P_Info::fillCtrl()
{
  TCHAR buff[4096];
  getKeyPath(TEXT_FONT, buff);
  SET_TEXT(IDC_EDIT_FONT, buff);

  getKeyPath(TEXT_COLOR, buff);
  TextColor = makeColor(buff);
  color->setColor(TextColor);
  curr->setColor(TextColor);

  getKeyPath(TEXT_BORDER, buff);
  HWND hwnd = GetDlgItem(*this, IDC_COMBO_BORDER);
  LPCTSTR infoBorder[] = { _T("No borders"), _T("A black border"), _T("Two borders, B&W") };
  for(uint i = 0; i < SIZE_A(infoBorder); ++i)
    addString(hwnd, infoBorder[i], false);
  SendMessage(hwnd, CB_SETCURSEL, _ttoi(buff), 0);

  TCHAR t[128];
  hwnd = GetDlgItem(*this, IDC_LIST_TEXT);
  for(uint i = 1;; ++i) {
    wsprintf(t, _T("%s%d"), TEXT_BASE, i);
    getKeyPath(t, buff);
    if(!*buff)
      break;
    addString(hwnd, buff, true);
    }
  int sel;
  getKeyParam(TEXT_CURR, (LPDWORD)&sel);
  if(sel) {
    SendMessage(hwnd, LB_SETCURSEL, sel -1, 0);
    SendMessage(hwnd, LB_GETTEXT, sel - 1, (LPARAM)buff);
    TCHAR b[4096];
    translateToCRNL(b, buff);
    curr->setCaption(b);
    SET_CHECK(IDC_CHECK_USE_TEXT);
    }

  getKeyPath(IMG_INFO, buff);
  if(*buff) {
    SET_TEXT(IDC_EDIT_IMG, buff);
    SET_CHECK(IDC_CHECK_USE_IMG);
    }

  getKeyPath(IMG_INFO_PERC, buff);
  SET_TEXT(IDC_EDIT_PERC, buff);

  getKeyPath(IMG_INFO_TOP, buff);
  if(_ttoi(buff))
    SET_CHECK(IDC_RADIO_IMG_UPPER);
  else
    SET_CHECK(IDC_RADIO_TEXT_UPPER);
  checkEnable();
}
//----------------------------------------------------------------------------
void P_Info::checkEnable()
{
  uint idcText[] = { IDC_EDIT_FONT, IDC_BUTTON_FONT, IDC_BUTTON_COLOR, IDC_COMBO_BORDER, IDC_LIST_TEXT, IDC_BUTTON_ADD, IDC_BUTTON_MOD, IDC_BUTTON_DEL };
  uint idcImg[] = { IDC_EDIT_IMG, IDC_BUTTON_IMG, IDC_EDIT_PERC, IDC_RADIO_TEXT_UPPER, IDC_RADIO_IMG_UPPER };

  bool enableText = IS_CHECKED(IDC_CHECK_USE_TEXT);
  bool enableImg = IS_CHECKED(IDC_CHECK_USE_IMG);
  for(uint i = 0; i < SIZE_A(idcText); ++i)
    ENABLE(idcText[i], enableText);
  for(uint i = 0; i < SIZE_A(idcImg); ++i)
    ENABLE(idcImg[i], enableImg);
}
//----------------------------------------------------------------------------
LRESULT P_Info::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case LBN_SELCHANGE:
          selChange();
          break;
        }
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_IMG:
          SelectImg(IDC_EDIT_IMG, IMG_INFO);
          break;

        case IDC_BUTTON_FONT:
          chooseFont();
          break;
        case IDC_BUTTON_COLOR:
          chooseColor();
          break;
        case IDC_CHECK_USE_TEXT:
        case IDC_CHECK_USE_IMG:
          checkEnable();
          break;
        case IDC_BUTTON_ADD:
          addText();
          break;
        case IDC_BUTTON_MOD:
          modText();
          break;
        case IDC_BUTTON_DEL:
          delText();
          break;

        case IDOK:
          BNClickedOk();
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void P_Info::addText()
{
  TCHAR buff[4096];
  buff[0] = 0;
  if(IDOK == P_Text(this, buff, SIZE_A(buff)).modal()) {
    if(!buff[0])
      return;
    HWND hwnd = GetDlgItem(*this, IDC_LIST_TEXT);
    int count = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
    TCHAR b[4096];
    translateFromCRNL(b, buff);
    addString(hwnd, b, true);

    wsprintf(buff, _T("%s%d"), TEXT_BASE, count + 1);
    setKeyPath(buff, b);
    modifRow[modifRow.getElem()] = count + 1;
    SendMessage(hwnd, LB_SETCURSEL, count, 0);
    forceRefresh = true;
    }
}
//----------------------------------------------------------------------------
void P_Info::modText()
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_TEXT);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[4096];
  SendMessage(hlb, LB_GETTEXT, sel, (LPARAM)buff);
  TCHAR b[4096];
  translateToCRNL(b, buff);
  if(IDOK == P_Text(this, b, SIZE_A(b)).modal()) {
    translateFromCRNL(buff, b);
    if(buff[0]) {
      SendMessage(hlb, LB_DELETESTRING, sel, 0);
      SendMessage(hlb, LB_INSERTSTRING, sel, (LPARAM)buff);
      wsprintf(b, _T("%s%d"), TEXT_BASE, sel + 1);
      setKeyPath(b, buff);
      modifRow[modifRow.getElem()] = sel + 1;
      SendMessage(hlb, LB_SETCURSEL, sel, 0);
      forceRefresh = true;
      }
    else
      delText();
    }
}
//----------------------------------------------------------------------------
void P_Info::delText()
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_TEXT);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR file[_MAX_PATH];
  makeKeyPath(file, SIZE_A(file));
  manageIni ini(file);
  ini.parse();

  TCHAR buff[128];
  wsprintf(buff, _T("%s%d"), TEXT_BASE, sel + 1);
  if(ini.deleteParam(buff, SUB_BASE_KEY)) {
    SendMessage(hlb, LB_DELETESTRING, sel, 0);
    int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
    SendMessage(hlb, LB_SETCURSEL, min(sel, count - 1), 0);
    for(++sel;; ++sel) {
      TCHAR buff2[128];
      wsprintf(buff2, _T("%s%d"), TEXT_BASE, sel + 1);
      if(!ini.getValue(buff2, SUB_BASE_KEY))
        break;
      ini.renameParam(SUB_BASE_KEY, buff, buff2);
      _tcscpy_s(buff, buff2);
      modifRow[modifRow.getElem()] = sel + 1;
      }
    ini.save();
    forceRefresh = true;
    }
}
//-----------------------------------------------------------
static bool choose_Color(HWND parent, COLORREF& target)
{
  const DWORD flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;
  CHOOSECOLOR cc;
  memset(&cc, 0, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = parent;
  cc.rgbResult = target;
  cc.Flags = flags;
  static COLORREF custom[16];
  cc.lpCustColors = custom;

  if(ChooseColor(&cc)) {
    target = cc.rgbResult;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void P_Info::chooseColor()
{
  COLORREF c = color->getColor();
  if(choose_Color(*this, c)) {
    if(c != color->getColor()) {
      TextColor = c;
      color->setColor(c);
      InvalidateRect(*color, 0, 0);
      curr->setColor(c);
      InvalidateRect(*curr, 0, 0);
      }
    }
}
//----------------------------------------------------------------------------
void P_Info::selChange()
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_TEXT);
  TCHAR buff[4096];
  buff[0] = 0;
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel >= 0)
    SendMessage(hlb, LB_GETTEXT, sel, (LPARAM) buff);
  TCHAR b[4096];
  translateToCRNL(b, buff);
  curr->setCaption(b);
  InvalidateRect(*curr, 0, 0);
}
//----------------------------------------------------------------------------
#define FLAGS_FONT CF_SCALABLEONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT
//----------------------------------------------------------------------------
void P_Info::chooseFont()
{
  LOGFONT lf;
  ZeroMemory(&lf, sizeof(lf));
  GET_TEXT(IDC_EDIT_FONT, lf.lfFaceName);
  lf.lfHeight = 22;
  CHOOSEFONT cf;
  memset(&cf, 0, sizeof(cf));
  cf.lStructSize = sizeof(cf);
  cf.hwndOwner = getHandle();
  cf.lpLogFont = &lf;
  cf.Flags = FLAGS_FONT;
  cf.nFontType = SCREEN_FONTTYPE | REGULAR_FONTTYPE | ITALIC_FONTTYPE | BOLD_FONTTYPE;
  if(ChooseFont(&cf))
    SET_TEXT(IDC_EDIT_FONT, lf.lfFaceName);
}
//----------------------------------------------------------------------------
LPCTSTR filterExt[] = { _T(".png"), _T(".jpg"), _T(".jpeg"), _T(".gif"), _T(".bmp"), _T(".tif"), _T(".emf"), _T(".wmf"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpen =
  _T("File immagini (jfif, png, gif, bmp, tif, emf, wmf)\0")
  _T("*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.tif;*.emf;*.wmf\0");
//----------------------------------------------------------------------------
static DWORD lastIx;
//----------------------------------------------------------------------------
bool openFileImg(HWND owner, LPTSTR file)
{
  infoOpenSave Info(filterExt, filterOpen, infoOpenSave::OPEN_F, lastIx, file);
  POpenSave open(owner);

  if(open.run(Info)) {
    lastIx = Info.ixFilter;
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void P_Info::SelectImg(uint idc, LPCTSTR key)
{
  TCHAR t[_MAX_PATH];
  getKeyPath(key, t);
  if(openFileImg(*this, t))
    SET_TEXT(idc, t);
}
