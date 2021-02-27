//----------- dManageStdMsg.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "common.h"
//----------------------------------------------------------------------------
#include "dManageStdMsg.h"
#include "pOwnBtnImageStd.h"
#include "pBitmap.h"
//----------------------------------------------------------------------------
basePage::basePage(const setOfString& set, PWin* parent, uint id, HINSTANCE hInst) :
        baseClass(parent, id, hInst), Set(set), Dirty(0),
        Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
        Brush3(CreateSolidBrush(bkgColor3)) {}
//----------------------------------------------------------------------------
basePage::~basePage()
{
  destroy();
  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
  DeleteObject(HGDIOBJ(Brush3));
}
//----------------------------------------------------------------------------
uint basePage::reqSave()
{
  return MessageBox(*this, _T("Vuoi salvare i dati prima di procedere?"),
              _T("I dati sono stati modificati"),
              MB_YESNOCANCEL);
}
//----------------------------------------------------------------------------
extern basePage* allocStdLang(const setOfString& Set, PWin* parent, HINSTANCE hinstance = 0);
extern basePage* allocStdText(const setOfString& Set, PWin* parent, HINSTANCE hinstance = 0);
extern basePage* allocStdVars1(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
extern basePage* allocStdVars2(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
extern basePage* allocStdVars3(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
extern basePage* allocStdAlarm(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
//extern basePage* allocStdMaint(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
extern basePage* allocStdCauseStop(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
extern basePage* allocStdTrend(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
extern basePage* allocStdPrint1(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
extern basePage* allocStdPrint2(const setOfString& Set, PWin* parent, HINSTANCE hInst = 0);
//----------------------------------------------------------------------------
void makeStdMsgSet(setOfString& set)
{
  dataProject& dp = getDataProject();
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, dp.newPath);
  appendPath(path, dp.systemPath);
  LPTSTR p = path + _tcslen(path);
  appendPath(path, STD_MSG_NAME);

  set.add(path);
  *p = 0;
  appendPath(path, _T("#") STD_MSG_NAME);
  manageCryptPage mcp;
  bool crypted = mcp.isCrypted(path);
  if(crypted)
    mcp.makePathAndDecrypt(path);
  set.add(path);
  if(crypted)
    mcp.releaseFileDecrypt(path);
  if(!crypted && mcp.isCrypted()) {
    MessageBox(0, _T("Base_Data non abilitato"), _T("Errore"), MB_OK | MB_ICONSTOP);
    set.reset();
    }
}
//----------------------------------------------------------------------------
static int useExtKeyb = -1;
//----------------------------------------------------------------------------
bool getUseExtKeyb()
{
  if(-1 == useExtKeyb) {
    setOfString set;
    makeStdMsgSet(set);
    useExtKeyb = 0;
    LPCTSTR p = set.getString(ID_USE_EXTENTED_KEYB);
    if(p)
      useExtKeyb = _ttoi(p);
    }
  return toBool(useExtKeyb);
}
//----------------------------------------------------------------------------
void setUseExtKeyb(bool use)
{
  useExtKeyb = use;
}
//----------------------------------------------------------------------------
static DWORD GlobalCount = MIN_INIT_STD_MSG_TEXT;
//----------------------------------------------------------------------------
DWORD getGlobalStdMsgText()
{
  return GlobalCount++;
}
//----------------------------------------------------------------------------
dManageStdMsg::dManageStdMsg(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  currPage(0), ImageList(0),
    changedRes(false)
{
  GlobalCount = MIN_INIT_STD_MSG_TEXT;
  makeStdMsgSet(Set);
  Page[0] = allocStdLang(Set, this, hinstance);
  Page[1] = allocStdText(Set, this, hinstance);
  Page[2] = allocStdVars1(Set, this, hinstance);
  Page[3] = allocStdVars2(Set, this, hinstance);
  Page[4] = allocStdVars3(Set, this, hinstance);
  Page[5] = allocStdAlarm(Set, this, hinstance);
//  Page[6] = allocStdMaint(Set, this, hinstance);
  Page[6] = allocStdCauseStop(Set, this, hinstance);
  Page[7] = allocStdTrend(Set, this, hinstance);
  Page[8] = allocStdPrint1(Set, this, hinstance);
  Page[9] = allocStdPrint2(Set, this, hinstance);

  int idBmp[] = { IDB_BITMAP_OK, IDB_BITMAP_CANC };
  int idBtn[] = { IDC_BUTTON_SAVE, IDCANCEL };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp[i]);
    }
}
//----------------------------------------------------------------------------
extern void loadCurrLang(bool& hideTitle);
//----------------------------------------------------------------------------
dManageStdMsg::~dManageStdMsg()
{
  dataProject& dp = getDataProject();
  bool old = dp.hideTitle;
  loadCurrLang(dp.hideTitle);
  svmMainClient* mc = getMainClient(this);
  if(mc) {
    InvalidateRect(*mc, 0, 0);
    if(changedRes || old != dp.hideTitle)
      mc->resizeByTitle(changedRes);
    }

  destroy();
  for(uint i = 0; i < SIZE_A(Bmp); ++i)
    delete Bmp[i];
  if(ImageList)
    ImageList_Destroy(ImageList);
}
//----------------------------------------------------------------------------
#define CX_IMAGE 16
#define CY_IMAGE 16
//----------------------------------------------------------------------------
bool dManageStdMsg::create()
{
  if(!baseClass::create())
    return false;

  LPTSTR tabTitle[] = {
    _T(""),
    _T("Text"),
    _T("Var1"),
    _T("Var2"),
    _T("Var3"),
    _T("Allarmi"),
//    _T("Manut."),
    _T("Cause Stop"),
    _T("Trend"),
    _T("Prt1"),
    _T("Prt2"),
    };

  HWND hTab = GetDlgItem(*this, IDC_TABCONTROL1);
  ImageList = ImageList_Create(CX_IMAGE, CY_IMAGE, ILC_COLOR | ILC_MASK, 5, 0);

  int idiIcons[] = {
      IDI_ICON_FLAG,
      IDI_ICON_STR,
      IDI_ICON_PROP,
      IDI_ICON_PROP,
      IDI_ICON_PROP,
      IDI_ICON_ALARM,
//      IDI_ICON_MAINT,
      IDI_ICON_RCP_FILE,
      IDI_ICON_TREND,
      IDI_ICON_PRINT,
      IDI_ICON_PRINT2
      };

  for(uint i = 0; i < SIZE_A(idiIcons); ++i) {
    HICON hi = LoadIcon(getHInstance(), MAKEINTRESOURCE(idiIcons[i]));
    ImageList_AddIcon(ImageList, hi);
    }

  TabCtrl_SetImageList(hTab, ImageList);

  for(int i = 0; i < MAX_TAB_PAGES; ++i) {
    TC_ITEM ti;
    memset(&ti, 0, sizeof(ti));
    ti.mask = TCIF_TEXT | TCIF_IMAGE;
    ti.pszText = tabTitle[i];
    ti.cchTextMax = _tcslen(tabTitle[i]);
    ti.iImage = i;
    TabCtrl_InsertItem(hTab, i, &ti);
    }
  PRect r;
  TabCtrl_GetItemRect(hTab, 0, r);

  PRect r2;
  GetWindowRect(hTab, r2);
  for(int i = 0; i < MAX_TAB_PAGES; ++i)
    SetWindowPos(*Page[i], 0, 0, r.Height() + r2.Height(), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);

  ShowWindow(*Page[0], SW_SHOWNORMAL);

  return true;
}
//----------------------------------------------------------------------------
LRESULT dManageStdMsg::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
/*
    case WM_INITDIALOG:
      do {
        DWORD style = SetClassLong(hwnd, GCL_STYLE);
        style
        HGDIOBJ br = GetStockObject(BLACK_BRUSH);
//        HGDIOBJ br = GetStockObject(LTGRAY_BRUSH);
        SetClassLong(hwnd, GCL_HBRBACKGROUND, (DWORD)br);
        } while(false);
      break;
*/

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_SAVE:
          Cm_Ok();
          break;
        }
      break;
    case WM_NOTIFY:
      if(((NMHDR*)lParam)->code == TCN_SELCHANGE)
        chgPage(TabCtrl_GetCurSel(((NMHDR*)lParam)->hwndFrom));
      break;
/*
    case WM_ERASEBKGND:
      if(wParam) {
        HDC hdc = (HDC)wParam;

        HGDIOBJ br = GetStockObject(LTGRAY_BRUSH);
        PRect r;
        GetClientRect(*this, r);
        HGDIOBJ old = SelectObject(hdc, br);
        PatBlt(hdc, r.left, r.top, r.Width(), r.Height(), PATCOPY);
        SelectObject(hdc, old);
        return 1;
        }
      break;
*/
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void dManageStdMsg::chgPage(int page)
{
  ShowWindow(*Page[currPage], SW_HIDE);
  currPage = page;
  ShowWindow(*Page[currPage], SW_SHOWNORMAL);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static
void restoreFile(LPTSTR path, P_File& pf)
{
  pf.P_close();
  _tcscpy_s(path, _MAX_PATH, pf.get_name());
  P_File::chgExt(path, _T(".bak"));
  P_File pf2(path);
  pf2.P_rename(pf.get_name());
}
//----------------------------------------------------------------------------
static
void restoreFile(LPTSTR path)
{
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, _MAX_PATH, path);
  P_File::chgExt(t, _T(".bak"));
  P_File pf2(t);
  pf2.P_rename(path);
}
//----------------------------------------------------------------------------
void dManageStdMsg::CmCancel()
{
  for(int i = 0; i < MAX_TAB_PAGES; ++i) {
    uint res = Page[i]->needSaveBeforeClose();
    if(IDYES == res) {
      PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_SAVE, 0), 0);
      return;
      }
    else if(IDCANCEL == res)
      return;
    else if(IDNO == res)
      break;
    }
  baseClass::CmCancel();
}
//----------------------------------------------------------------------------
void dManageStdMsg::Cm_Ok()
{
  dataProject& dp = getDataProject();
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, dp.newPath);
  appendPath(path, dp.systemPath);
  LPTSTR p = path + _tcslen(path);
  appendPath(path, STD_MSG_NAME);

  do {
    P_File pfClear(path);
    pfClear.reback(_T(".bak"));
    } while(false);

  P_File pfClear(path, P_CREAT);

  *p = 0;
  appendPath(path, _T("#") STD_MSG_NAME);

  do {
    P_File pfCrypt(path);
    pfCrypt.reback(_T(".bak"));
    } while(false);

  manageCryptPage mcp;
  bool crypted = mcp.isCrypted();
  if(crypted)
    mcp.makePathCrypt(path);

  P_File pfCrypt(path, P_CREAT);

  if(pfClear.P_open() && pfCrypt.P_open()) {
    for(int i = 0; i < MAX_TAB_PAGES; ++i)
      if(!Page[i]->save(pfCrypt, pfClear)) {
        restoreFile(path, pfClear);
        if(crypted) {
          *p = 0;
          appendPath(path, _T("#") STD_MSG_NAME);
          restoreFile(path);
          }
        else
          restoreFile(path, pfCrypt);
        return;
        }
    }
  pfCrypt.P_close();
  if(crypted) {
    *p = 0;
    appendPath(path, _T("#") STD_MSG_NAME);
    mcp.releaseFileCrypt(path);
    }
  svmMainClient* par = getParentWin<svmMainClient>(getParent());
  if(par)
    par->invalidate();
  baseClass::CmOk();
}

//----------------------------------------------------------------------------
LRESULT basePage::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
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
