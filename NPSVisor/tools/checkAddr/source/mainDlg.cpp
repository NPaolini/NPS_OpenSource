//----------- mainDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "manageCryptPage.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "mainDlg.h"
#include "test_Addr.h"
#include "HeaderMsg.h"
#include "p_mappedfile.h"
//----------------------------------------------------------------------------
uint registeredMsg = RegisterWindowMessage(_T("NP_CHECK_ADDR_TO_SV_MAKER"));
//----------------------------------------------------------------------------
extern void checkBasePage();
//----------------------------------------------------------------------------
PDShowVars::PDShowVars(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), onJob(jobbingVars),
    ImageList(0), MP(0), forSvMaker(0)
{
  checkBasePage();

  clientVars = new PD_AllVars(this);
  clientPages = new PD_AllPages(this);
  clientNorm = new PD_AllVarNorm(this);
  clientWarn = new PD_WarnObj(this);
  clientUsedPage = new PD_pageList(this);
  clientUsedImage = new PD_imageList(this);
}
//----------------------------------------------------------------------------
PDShowVars::~PDShowVars()
{
  unsetHotKey();
  destroy();
  if(ImageList)
    ImageList_Destroy(ImageList);
  delete forSvMaker;
  delete MP;
}
//----------------------------------------------------------------------------
#define CX_IMAGE 32
#define CY_IMAGE 32

#define W_BTN 80
//----------------------------------------------------------------------------
void PDShowVars::resize()
{
  if(!getHandle())
    return;
  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  static PRect rTab;
  if(!rTab.Width()) {
    GetWindowRect(hTab, rTab);
    MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)rTab, 2);
    }

  PRect cl;
  GetClientRect(*this, cl);

  PRect rc(0, rTab.Height() + 2, cl.right, cl.bottom);
  SetWindowPos(hTab, 0, 0, 0, cl.right, rTab.Height() + 1, SWP_NOZORDER);

  clientVars->setWindowPos(0, rc, SWP_NOZORDER);
  clientPages->setWindowPos(0, rc, SWP_NOZORDER);
  clientNorm->setWindowPos(0, rc, SWP_NOZORDER);
  clientWarn->setWindowPos(0, rc, SWP_NOZORDER);
  clientUsedPage->setWindowPos(0, rc, SWP_NOZORDER);
//  rc.top += 50;
  clientUsedImage->setWindowPos(0, rc, SWP_NOZORDER);
}
//----------------------------------------------------------------------------
#define BASE_DIV 20
//----------------------------------------------------------------------------
static void setTitleByPath(HWND hwnd)
{
  TCHAR path[_MAX_PATH] = _T("\0");
  ::getPath(path);
  TCHAR t[_MAX_PATH];
  if(*path)
    wsprintf(t, _T("Variables Control - %s"), path);
  else
    _tcscpy(t, _T("Variables Control"));
  SetWindowText(hwnd, t);
}
//----------------------------------------------------------------------------
bool PDShowVars::create()
{
  Attr.style |= WS_CLIPCHILDREN;
  if(!baseClass::create())
    return false;
  int w = GetSystemMetrics(SM_CXFULLSCREEN) / BASE_DIV;
  int h = GetSystemMetrics(SM_CYFULLSCREEN) / BASE_DIV;

  PRect r(0, 0, w * (BASE_DIV - 1), h * (BASE_DIV - 1));
  r.MoveTo(w / 2, h / 2);
  setWindowPos(0, r, SWP_NOZORDER);

  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));

  setTitleByPath(*this);

  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  ImageList = ImageList_Create(CX_IMAGE, CY_IMAGE, ILC_COLOR | ILC_MASK, 6, 0);

  PBitmap bmp(IDB_TABS, getHInstance());
  ImageList_AddMasked(ImageList, bmp, RGB(192, 192, 192));

  TabCtrl_SetImageList(hTab, ImageList);
  PRect rt;
  TabCtrl_GetItemRect(hTab, 1, rt);
  TabCtrl_SetItemSize(hTab, rt.Width(), CY_IMAGE);

  LPTSTR tabTitle[] = {
    _T("Variables"),
    _T("Pages"),
    _T("Normalizer"),
    _T("Pages list"),
    _T("Images list"),
    _T("Warning"),
    _T("New (F3)"),
    _T("Refresh (F5)"),
    };
  for(uint i = 0; i < SIZE_A(tabTitle); ++i) {
    TC_ITEM ti;
    memset(&ti, 0, sizeof(ti));
    ti.mask = TCIF_TEXT | TCIF_IMAGE;
    ti.pszText = tabTitle[i];
    ti.cchTextMax = _tcslen(tabTitle[i]);
    ti.iImage = i;
    TabCtrl_InsertItem(hTab, i, &ti);
    }


  resize();
  TabCtrl_SetCurSel(hTab, 0);

  ShowWindow(*clientPages, SW_HIDE);
  ShowWindow(*clientNorm, SW_HIDE);
  ShowWindow(*clientWarn, SW_HIDE);
  ShowWindow(*clientVars, SW_SHOWNORMAL);
  ShowWindow(*clientUsedPage, SW_HIDE);
  ShowWindow(*clientUsedImage, SW_HIDE);
  setHotKey();

  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static HHOOK HookHandle;
//----------------------------------------------------------------------------
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
  if(code < 0)
    return CallNextHookEx(HookHandle, code, wParam, lParam);
  if(!(HIWORD(lParam) & KF_UP)) {
    switch(wParam) {
      case VK_F3:
        PostMessage(*getMain(), WM_COMMAND, MAKEWPARAM(IDC_BTN_NEW, 0), 0);
        break;
      case VK_F5:
        PostMessage(*getMain(), WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
        break;
      }
    }
  return CallNextHookEx(HookHandle, code, wParam, lParam);
}
//----------------------------------------------------------------------------
void PDShowVars::setHotKey()
{
  HookHandle = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, 0, ::GetCurrentThreadId());
}
//----------------------------------------------------------
void PDShowVars::unsetHotKey()
{
  UnhookWindowsHookEx(HookHandle);
}
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//-----------------------------------------------------------------------------
LRESULT PDShowVars::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(registeredMsg == message) {
    switch(LOWORD(wParam)) {
      case CM_INIT_COMM:
        SetWindowLong(hwnd, DWL_MSGRESULT, (LRESULT)lParam);
        return (LRESULT)lParam;

      case CM_START_COMM:
        if(1 == HIWORD(wParam)) {
          refreshCheck();
          SetForegroundWindow(HWND(lParam));
          }
        return initObjComm(HWND(lParam));

      case CM_END_COMM:
        destroyObjComm();
        break;
      case CM_EXPAND:
        expandObjComm(HTREEITEM(lParam));
        break;
      }
    }
  switch(message) {
    case WM_NOTIFY:
      if(((NMHDR*)lParam)->code == TCN_SELCHANGE)
        chgPage(TabCtrl_GetCurSel(((NMHDR*)lParam)->hwndFrom));
      break;
    case WM_SIZE:
      resize();
      break;
    case WM_CHAR:
      if(VK_F5 == wParam)
        refreshCheck();
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BTN_NEW:
          newCheck();
          break;
        case IDC_BTN_REFRESH:
          refreshCheck();
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
LRESULT PDShowVars::initObjComm(HWND htree)
{
  destroyObjComm();
//  if(!MP)
//    return 0;
  delete forSvMaker;
  forSvMaker = new PD_AllVarsBysvMaker(this, htree);
  forSvMaker->setManPage(MP);
  return (LRESULT)(HWND)*this;
}
//----------------------------------------------------------------------------
void PDShowVars::destroyObjComm()
{
  delete forSvMaker;
  forSvMaker = 0;
}
//----------------------------------------------------------------------------
void PDShowVars::expandObjComm(HTREEITEM htree)
{
  if(!forSvMaker)
    return;
  forSvMaker->actionExpand(htree);
}
//----------------------------------------------------------------------------
void PDShowVars::newCheck()
{
  if(IDOK != test_Addr(this).modal())
    return;

  setTitleByPath(*this);
  manageCryptPage().reset();

  refreshCheck();
}
//----------------------------------------------------------------------------
void PDShowVars::refreshCheck()
{
  SetCursor(LoadCursor(0, IDC_WAIT));
  delete MP;
  TCHAR path[_MAX_PATH];
  ::getPath(path);
  MP = new managePages(path, FIRST_PAGE);

  if(!MP->makeAll(this)) {
    SetCursor(LoadCursor(0, IDC_ARROW));
    return;
    }
  clientVars->setManPage(MP);
  clientPages->setManPage(MP);
  clientNorm->setManPage(MP);
  clientWarn->setManPage(MP);
  clientUsedPage->setManPage(MP);
  clientUsedImage->setManPage(MP);
  if(forSvMaker)
    forSvMaker->setManPage(MP);
  SetCursor(LoadCursor(0, IDC_ARROW));
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
void PDShowVars::chgPage(int page)
{
  bool canShow = false;
  switch(page) {
    case newPrj:
      PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDC_BTN_NEW, 0), 0);
      break;
    case refresh:
      PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
      break;
    default:
      canShow = true;
    }
  if(!canShow) {
    HWND hTab = GetDlgItem(*this, IDC_TAB1);
    TabCtrl_SetCurSel(hTab, onJob);
    return;
    }
  PWin* client[] = { clientVars, clientPages, clientNorm, clientUsedPage, clientUsedImage, clientWarn };

  SetWindowPos(*client[page], HWND_TOP, 0, 0, 0, 0, STD_FLG_SHOW);
  for(uint i = 0; i < maxWhich; ++i) {
    if(i == page)
      continue;
    if(client[i])
      HIDE_W(*client[i]);
    }
  onJob = (which)page;
}
