//----------- PDSetForeground.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
//#define _WIN32_WINNT 0x0600
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_util.h"
#include "sv_make_dll.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "PDSetForeground.h"
#include "pListbox.h"
#include "macro_utils.h"
//----------------------------------------------------------------------------
extern void checkBasePage();
//----------------------------------------------------------------------------
#define ID_LIST_COLOR     12
#define ID_LIST_SEL_COLOR 13
#define ID_LIST_FONT      14

#define ID_LIST_BEHAVIOUR 15

#define ID_TEXT_TITLE 20
#define ID_TEXT_F1    21
#define ID_TEXT_F2    22

#define ID_IMG_F1    25
#define ID_IMG_F2    26

#define ID_BUTTON_FONT 30

#define ID_LIST_ACCEPT    50
//----------------------------------------------------------------------------
static HFONT makeFont(LPCTSTR p)
{
  HFONT font = 0;
  while(p) {
    int h = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    int flag = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    font = D_FONT(h, 0, flag, p);
    break;
    }
  return font;
}
//----------------------------------------------------------------------------
PDSetForeground::PDSetForeground(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{
  npSV_GetBodyRoutine(eSV_MANAGE_DIALOG_FUNCT_KEY, LPDWORD(1), 0);
  int idBmp[] = { IDB_F1, IDB_F2 };
  int idBtn[] = { IDC_BUTTON_SET, IDC_BUTTON_CANC };
  int idAltImg[] = { ID_IMG_F1, ID_IMG_F2 };

  HFONT hf = makeFont(npSV_GetLocalString(ID_BUTTON_FONT, getHinstance()));
  HINSTANCE hi = getHInstance();
  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* bmp = 0;
    LPCTSTR p = npSV_GetLocalString(idAltImg[i], getHinstance());
    if(p)
      bmp = new PBitmap(p);
    else
      bmp = new PBitmap(idBmp[i], hi);
    Btn[i] = new POwnBtnImageStd(this, idBtn[i], bmp, POwnBtnImageStd::wLeft, true);
    if(hf)
      Btn[i]->setFont(hf, !i);
    }
}
//----------------------------------------------------------------------------
PDSetForeground::~PDSetForeground()
{
  npSV_GetBodyRoutine(eSV_MANAGE_DIALOG_FUNCT_KEY, 0, 0);
  destroy();
}
//----------------------------------------------------------------------------
static void getColor(LPCTSTR p, COLORREF* col)
{
  for(uint i = 0; i < 2; ++i) {
    while(p) {
      int r = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      int g = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      int b = _ttoi(p);
      col[i] = RGB(r, g, b);
      p = findNextParamTrim(p);
      break;
      }
    }
}
//----------------------------------------------------------------------------
#define DIM_HWND 32
#define DIM_EXEC _MAX_PATH

#define DIM_LB (DIM_HWND + DIM_EXEC + 3)
//----------------------------------------------------------------------------
bool PDSetForeground::create()
{
  PListBox* lb = new PListBox(this, IDC_LIST1);
  int tabs[] = { DIM_EXEC, DIM_HWND };
  int shows[] = { 1, 0 };
  lb->SetTabStop(SIZE_A(tabs), tabs, shows);
  LPCTSTR p = npSV_GetLocalString(ID_LIST_COLOR, getHinstance());
  if(p) {
    COLORREF col[2] = {0};
    getColor(p, col);
    lb->SetColor(col[0], col[1]);
    }
  p = npSV_GetLocalString(ID_LIST_SEL_COLOR, getHinstance());
  if(p) {
    COLORREF col[2] = {0};
    getColor(p, col);
    lb->SetColorSel(col[0], col[1]);
    }
  HFONT font = makeFont(npSV_GetLocalString(ID_LIST_FONT, getHinstance()));
  if(font)
    lb->setFont(font, true);
  if(!baseClass::create())
    return false;

  p = npSV_GetLocalString(ID_TEXT_TITLE, getHinstance());
  if(p)
    setCaption(p);
  p = npSV_GetLocalString(ID_TEXT_F1, getHinstance());
  if(p)
    Btn[0]->setCaption(p);
  p = npSV_GetLocalString(ID_TEXT_F2, getHinstance());
  if(p)
    Btn[1]->setCaption(p);

  fillAppl();
  lb->setIntegralHeight();
  return true;
}
//-----------------------------------------------------------------------------
// copiati da resource.h del sVisor
#define IDC_BUTTON_F1                   2101
#define IDC_BUTTON_F2                   2102
//-----------------------------------------------------------------------------
LRESULT PDSetForeground::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_SET:
        case IDC_BUTTON_F1:
          if(!setForeground())
            break;
        case IDC_BUTTON_CANC:
        case IDC_BUTTON_F2:
          baseClass::CmCancel();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD (WINAPI *prtGetModuleFileNameExW)(
    HANDLE hProcess,
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    );

typedef DWORD (WINAPI *prtGetProcessImageFileNameW)(
    HANDLE hProcess,
    LPWSTR lpFilename,
    DWORD nSize
    );

#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------
prtGetModuleFileNameExW GetModuleFileNameEx = 0;
prtGetProcessImageFileNameW GetProcessImageFileNameW = 0;
LPCTSTR libName = _T("PSAPI.DLL");
LPCSTR procGetModuleFileNameExW = "_GetModuleFileNameExW";
LPCSTR procGetProcessImageFileNameW = "_GetProcessImageFileNameW";
HMODULE hDLL = 0;
//------------------------------------------------------------------
bool loadProc()
{
  hDLL = LoadLibrary(libName);
  while(hDLL) {
    FARPROC f = 0;
    f = GetProcAddress(hDLL, procGetModuleFileNameExW);
    if(!f)
      f = GetProcAddress(hDLL, procGetModuleFileNameExW + 1);
    if(!f)
      break;
    GetModuleFileNameEx = (prtGetModuleFileNameExW)f;

    f = GetProcAddress(hDLL, procGetProcessImageFileNameW);
    if(!f)
      f = GetProcAddress(hDLL, procGetProcessImageFileNameW + 1);
    if(f)
      GetProcessImageFileNameW = (prtGetProcessImageFileNameW)f;
    return true;
    }
  if(hDLL) {
    FreeLibrary(hDLL);
    hDLL = 0;
    }
  return false;
}
//----------------------------------------------------------------------------
static bool getExecutableNameV6(HWND hwnd, LPTSTR target, uint sz)
{
  if(!GetProcessImageFileNameW)
    return false;
  OSVERSIONINFO ver;
  ZeroMemory(&ver, sizeof(ver));
  ver.dwOSVersionInfoSize = sizeof(ver);
  if(!GetVersionEx(&ver))
    return false;
  if(VER_PLATFORM_WIN32_NT != ver.dwPlatformId)
    return false;
  if(ver.dwMajorVersion < 6)
    return false;

  DWORD dwProcess = 0;
  DWORD dwThread = GetWindowThreadProcessId(hwnd, &dwProcess);
  if(!dwProcess)
    return false;
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, dwProcess);
  if(!hProcess)
    return false;
  bool success = GetProcessImageFileNameW(hProcess, target, sz) > 0;
  CloseHandle(hProcess);
  return success;
}
//----------------------------------------------------------------------------
static bool getExecutableName(HWND hwnd, LPTSTR target, uint sz)
{
  if(!hwnd)
    return false;
  DWORD dwProcess = 0;
  DWORD dwThread = GetWindowThreadProcessId(hwnd, &dwProcess);
  if(!dwProcess)
    return false;
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwProcess);
  if(!hProcess)
    return getExecutableNameV6(hwnd, target, sz);
  bool success = GetModuleFileNameEx(hProcess, NULL, target, sz) > 0;
  CloseHandle(hProcess);
  if(!success)
    success = getExecutableNameV6(hwnd, target, sz);
  return success;
}
//----------------------------------------------------------------------------
static bool accept(pvvChar& filter, LPCTSTR name)
{
  uint n = filter.getElem();
  if(!n)
    return true;

  for(uint i = 0; i < n; ++i)
    if(!_tcsicmp(name, &(filter[i])))
      return true;
  return false;
}
//----------------------------------------------------------------------------
static bool makeString(LPTSTR t, LPCTSTR caption, int behaviour, pvvChar& p)
{
  TCHAR drive[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];
  TCHAR fname[_MAX_FNAME];
  TCHAR ext[_MAX_EXT];
  _tsplitpath_s(t, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
  if(!accept(p, fname))
    return false;
  if(1 == behaviour)
    wsprintf(t, _T("%s"), caption);
  else if(2 == behaviour)
    wsprintf(t, _T("%s%s"), fname, ext);
  else
    wsprintf(t, _T("[%s%s] %s"), fname, ext, caption);
  return true;
}
//----------------------------------------------------------------------------
static int getBehaviour()
{
  LPCTSTR p = npSV_GetLocalString(ID_LIST_BEHAVIOUR, getHinstance());
  return p ? _ttoi(p) : 0;
}
//----------------------------------------------------------------------------
void PDSetForeground::fillAppl()
{
  wl.owner = *this;
  wl.set.reset();
  HWND hwl = GetDlgItem(*this, IDC_LIST1);
  SendMessage(hwl, LB_RESETCONTENT, 0, 0);
  if(!getListHwnd(wl))
    return;
  bool loadedDll = loadProc();
  if(!loadedDll)
    return;
  int behaviour = getBehaviour();
  const setOfInfo& set = wl.set;
  uint nElem = set.getElem();
  TCHAR t[_MAX_PATH];
  LPCTSTR p = npSV_GetLocalString(ID_LIST_ACCEPT, getHinstance());
  pvvChar target;
  if(p)
    splitParamTrim(target, p);
  for(uint i = 0; i < nElem; ++i) {
    if(loadedDll && getExecutableName(set[i].hwnd, t, SIZE_A(t))) {
      if(makeString(t, set[i].caption, behaviour, target)) {
        TCHAR buff[DIM_LB];
        fillStr(buff, _T(' '), SIZE_A(buff));

        LPTSTR pt = buff;
        copyStr(pt, t, _tcslen(t));
        pt += DIM_EXEC;
        *pt++ = _T('\t');

        TCHAR tt[DIM_HWND];
        wsprintf(tt, _T("%d"), set[i].hwnd);
        copyStr(pt, tt, _tcslen(tt));
        pt += DIM_HWND;
        *pt++ = 0;
        SendMessage(hwl, LB_ADDSTRING, 0, LPARAM(buff));
        }
      }
    }
  SendMessage(hwl, LB_SETCURSEL, 0, 0);
  FreeLibrary(hDLL);
  hDLL = 0;
}
//----------------------------------------------------------------------------
bool PDSetForeground::setForeground()
{
  HWND hwl = GetDlgItem(*this, IDC_LIST1);
  int sel = SendMessage(hwl, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return false;
  TCHAR t[DIM_LB];
  SendMessage(hwl, LB_GETTEXT, sel, (LPARAM)t);
  HWND hw = (HWND)_ttoi(t + DIM_EXEC + 1);
  if(!IsWindow(hw)) {
    fillAppl();
    return false;
    }
  if(IsIconic(hw))
    ShowWindow(hw, SW_RESTORE);
  SetForegroundWindow(hw);
  return true;
}
