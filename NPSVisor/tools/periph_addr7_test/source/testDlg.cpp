//------------ testDlg.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <Tlhelp32.h>
//----------------------------------------------------------------------------
#include "testDlg.h"
#include "mainDlg.h"
#include "fillDlg.h"
#include "verifyDlg.h"
#include "winInfoList.h"
#include "p_mappedFile.h"
//----------------------------------------------------------------------------
#define RegSTR_WM_PRPH(msg, id)  \
  { \
    TCHAR buff[50]; \
    wsprintf(buff, _T("WM_PRPH_%d"), id); \
    msg = RegisterWindowMessage(buff);  \
  }
//----------------------------------------------------------------------------
#define MSG_CHANGED_DATA            9
//----------------------------------------------------------------------------
PD_Test::PD_Test(PWin* parent, uint id_adr, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), idAdr(id_adr), currPos(0),
    maxShow(getNumOfRow(id_adr)), baseAddr(1), nByteBase(4),
    Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)), Brush3(CreateSolidBrush(bkgColor3)),
    oldStat(1), autoRefresh(false), noConvert(false), WM_PRPH(0), idFocus(-1)
{
  int idBmp[] = { IDB_WRITE, IDB_READ, IDB_REFRESH, IDB_FILL_HANDLE, IDB_DONE };
  int idBtn[] = { IDC_BUTTON_WRITE, IDC_BUTTON_READ, IDC_BUTTON_AUTO_REFRESH, IDC_BUTTON_SVISOR_SEARCH, ID_EXIT_APP };

  for(uint i = 0; i < SIZE_A(Bmp); ++i) {
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());
    POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], Bmp[i]);
    }
  for(uint i = 0; i < MAX_GROUP; ++i)
    Rows[i] = 0;

  loadFile();
  RegSTR_WM_PRPH(WM_PRPH, idAdr);
}
//----------------------------------------------------------------------------
PD_Test::~PD_Test()
{
  for(uint i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
  for(uint i = 0; i < SIZE_A(Bmp); ++i)
    delete Bmp[i];
  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
  DeleteObject(HGDIOBJ(Brush3));
}
//----------------------------------------------------------------------------
static void full_path(LPTSTR path)
{
  TCHAR t[_MAX_PATH];
  _tcscpy(t, path);
  _tfullpath(path, t, _MAX_PATH);
}
//----------------------------------------------------------------------------
void PD_Test::loadFile()
{
  ZeroMemory(Values, sizeof(Values));
  TCHAR Filename[_MAX_PATH];
  if(1 == idAdr)
    _tcscpy(Filename, _T("job_base.dat"));
  else
    wsprintf(Filename, _T("prph_%d_data.dat"), idAdr);

  full_path(Filename);
  p_MappedFile pMF(Filename, mP_READ_ONLY);
  if(pMF.P_open()) {
    LPBYTE currBuff = (LPBYTE)pMF.getAddr();
    size_t szFile = (size_t)pMF.getDim();
    if(szFile > 0) {
      memcpy(Values, currBuff, szFile);
      checkNumRow(szFile / sizeof(DWORD));
      return;
      }
    }
  P_File pf(Filename, P_READ_ONLY);
  for(uint i = 0; i < 5; ++i) {
    if(pf.P_open()) {
      pf.P_read(Values, sizeof(Values));
      break;
      }
    Sleep(100);
    }
}
//----------------------------------------------------------------------------
void PD_Test::adviseSVisor()
{
  PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
  HWND HwndSV = 0;
  if(pda)
    HwndSV = pda->getSvHandle();
  if(HwndSV && -1 != (LONG)HwndSV)
    PostMessage(HwndSV, WM_PRPH, MAKEWPARAM(MSG_CHANGED_DATA, 0), 0);
}
//----------------------------------------------------------------------------
void PD_Test::saveFile()
{
  saveCurrData();
  TCHAR Filename[_MAX_PATH];
  if(1 == idAdr)
    _tcscpy(Filename, _T("job_base.dat"));
  else
    wsprintf(Filename, _T("prph_%d_data.dat"), idAdr);
  full_path(Filename);

  __int64 dimMapped = -1;

  do {
    p_MappedFile pMF(Filename, mP_READ_ONLY);
    if(pMF.P_open())
      dimMapped = pMF.getDim();
    } while(false);
  if(dimMapped > 0) {
    p_MappedFile pMF(Filename);
    if(pMF.P_open(dimMapped)) {
      LPBYTE currBuff = (LPBYTE)pMF.getAddr();
      memcpy(currBuff, Values, (size_t)dimMapped);
      FlushViewOfFile(currBuff, (size_t)dimMapped);
      }
    else
      dimMapped = -1;
    }
  if(dimMapped < 0) {
    P_File pf(Filename);
    for(uint i = 0; i < 5; ++i) {
      if(pf.P_open()) {
        pf.P_write(Values, sizeof(Values));
        break;
        }
      Sleep(100);
      }
    }
  adviseSVisor();
}
//----------------------------------------------------------------------------
bool PD_Test::create()
{
  POINT pt = { X_INIT, Y_INIT };
  Rows[0] = new PRowTest(this, IDC_EDIT_BYTE1_1, pt, -1);

  pt.y += H_EDIT - 1;
  for(int i = 1; i < MAX_GROUP - 1; ++i) {
    Rows[i] = new PRowTest(this, IDC_EDIT_BYTE1_1 + i, pt, 0);
    pt.y += H_EDIT - 1;
    }

  Rows[MAX_GROUP - 1] = new PRowTest(this, IDC_EDIT_BYTE1_1 + MAX_GROUP - 1, pt, 1);

  new PEdit(this, IDC_EDIT_MAX_ROW);
  new PEditCR(this, IDC_EDIT_GOTO_ROW, IDC_BUTTON_GOTO_ROW);
  new PEditCR(this, IDC_EDIT_SEARCH_TEXT, IDC_BUTTON_SEARCH_TEXT);

  PStatic* ps = new PStatic(this, IDC_STATIC_HW_SV);
  ps->setFont(D_FONT(20, 0, fBOLD, _T("comic sans MS")), true);
  if(!baseClass::create())
    return false;
#if 1
  PRect r;
  GetWindowRect(GetDlgItem(*this, IDC_STATIC_IP_PORT), r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);

  int dx = r.left - X_INIT - 3;
//  int dy = r.bottom - Y_INIT + 3;

  GetWindowRect(GetDlgItem(*this, IDC_STATIC_DESCR), r);
  double scaleX = r.left;
  GetWindowRect(GetDlgItem(*this, IDC_EDIT_BYTE1_1 + OFFSET_DESCR_TEST), r);
  scaleX /= r.left;

  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1), r);
  double scaleY = r.Height();
  GetWindowRect(GetDlgItem(*this, IDC_EDIT_BYTE1_1), r);
//  GetWindowRect(GetDlgItem(*this, IDC_EDIT_BYTE1_1 + OFFSET_VALUE), r);
  scaleY /= r.Height();
//  if(scaleY < 1)
//    scaleY = 1;
  for(int i = 0; i < MAX_GROUP; ++i) {
    GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1 + i), r);
    int dy = r.top;
    GetWindowRect(GetDlgItem(*this, IDC_EDIT_BYTE1_1 + i), r);
    dy -= r.top;
    Rows[i]->offset(dx, dy, scaleX, scaleY);
    }
#else
  PRect r;
  GetWindowRect(GetDlgItem(*this, IDC_STATIC_IP_PORT), r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);

  int dx = r.left - X_INIT - 3;
  int dy = r.bottom - Y_INIT + 3;
  for(int i = 0; i < MAX_GROUP; ++i)
    Rows[i]->offset(dx, dy);
#endif
  SET_TEXT(IDC_STATIC_HW_SV, _T(""));

  SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_BYTE), BM_SETCHECK, BST_CHECKED, 0);
  loadData();

  setScrollRange();

//  SendMessage(GetDlgItem(*this, IDC_CHECKBOX_ZERO), BM_SETCHECK, BST_CHECKED, 0);
  changeBase();

  SetTimer(*this, 1, 5000, 0);
  SetDlgItemInt(*this, IDC_EDIT_MAX_ROW, maxShow, 0);
  PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
  if(pda) {
    HWND hwSv = pda->getSvHandle();
    if(!hwSv)
      searchSVisor();
    if(!pda->getSvHandle())
      pda->setSvHandle((HWND)-1, 0);
    }
  return true;
}
//-----------------------------------------------------------------------------
bool PD_Test::isHex()
{
  return IS_CHECKED(IDC_CHECK_HEX) ^ noConvert;
}
//-----------------------------------------------------------------------------
void PD_Test::setScrollRange()
{
  HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
  if(hsb) {
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nPage = PAGE_SCROLL_LEN;
    si.nMin = 0;
    si.nMax = maxShow - 1;
    SendMessage(hsb, SBM_SETSCROLLINFO, 1, (LPARAM)&si);
    }
}
//----------------------------------------------------------------------------
bool PD_Test::preProcessMsg(MSG& msg)
{
  switch(msg.message) {
    case WM_KEYDOWN:
      switch(msg.wParam) {
        case VK_NEXT:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
          break;
        case VK_PRIOR:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
          break;
/*
        case VK_HOME:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_LEFT, 0);
          break;
        case VK_END:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_RIGHT, 0);
          break;
*/
        }
      break;
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
void PD_Test::getInfo(commonInfo& ci)
{
  saveCurrData();
  ci.base = true; //IS_CHECKED(IDC_CHECKBOX_ZERO);
  if(IS_CHECKED(IDC_RADIOBUTTON_WORD))
    ci.baseAddr = 2;
  else if(IS_CHECKED(IDC_RADIOBUTTON_DWORD))
    ci.baseAddr = 4;
  else
    ci.baseAddr = 1;

  GET_INT(IDC_EDIT_MAX_ROW, ci.totRow);
  ci.currRow = currPos;
  GET_INT(IDC_EDIT_GOTO_ROW, ci.currGotoRow);

}
//----------------------------------------------------------------------------
void PD_Test::setInfo(const commonInfo& ci)
{
/*
  bool zero = IS_CHECKED(IDC_CHECKBOX_ZERO);
  if(ci.base != zero) {
    SET_CHECK_SET(IDC_CHECKBOX_ZERO, ci.base);
    changeBase();
    }
*/
  DWORD totRow;
  GET_INT(IDC_EDIT_MAX_ROW, totRow);
  if(ci.totRow != totRow) {
    SET_INT(IDC_EDIT_MAX_ROW, ci.totRow);
    changeNumRow();
    }
  SET_INT(IDC_EDIT_GOTO_ROW, ci.currGotoRow);

  if(currPos != ci.currRow) {
    HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
    evVScrollBar(child, SB_THUMBTRACK, ci.currRow);
    }
  if(!IS_CHECKED(IDC_RADIOBUTTON_WORD) && ci.baseAddr == 2) {
    SET_CHECK(IDC_RADIOBUTTON_WORD);
    SET_CHECK_SET(IDC_RADIOBUTTON_DWORD, false);
    SET_CHECK_SET(IDC_RADIOBUTTON_BYTE, false);
    setBaseAddr(IDC_RADIOBUTTON_WORD);
    }
  else if(!IS_CHECKED(IDC_RADIOBUTTON_DWORD) && ci.baseAddr == 4) {
    SET_CHECK(IDC_RADIOBUTTON_DWORD);
    SET_CHECK_SET(IDC_RADIOBUTTON_WORD, false);
    SET_CHECK_SET(IDC_RADIOBUTTON_BYTE, false);
    setBaseAddr(IDC_RADIOBUTTON_DWORD);
    }
  else if(!IS_CHECKED(IDC_RADIOBUTTON_BYTE) && ci.baseAddr == 1) {
    SET_CHECK(IDC_RADIOBUTTON_BYTE);
    SET_CHECK_SET(IDC_RADIOBUTTON_WORD, false);
    SET_CHECK_SET(IDC_RADIOBUTTON_DWORD, false);
    setBaseAddr(IDC_RADIOBUTTON_BYTE);
    }
  nByteBase = ci.nByteBase;
  loadData();
}
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//-----------------------------------------------------------------------------
void PD_Test::evMouseWheel(short delta, short /*x*/, short /*y*/)
{
  int tD = delta;
  tD *= PAGE_SCROLL_LEN;
  tD /= WHEEL_DELTA;
  uint msg = WM_VSCROLL;
  HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);

  if(IsWindowEnabled(child)) {
    int curr = SendMessage(child, SBM_GETPOS, 0, 0);
    curr -= tD;
    if(curr < 0)
      curr = 0;
    else if(curr > (int)MAX_V_SCROLL)
      curr = MAX_V_SCROLL;
    SendMessage(*this, msg, MAKEWPARAM(SB_THUMBTRACK, curr), (LPARAM)child);
    }
}
//----------------------------------------------------------------------------
LRESULT PD_Test::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_CLIENT_WHEEL:
      evMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
      return 0;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            return 0;
        case ID_EXIT_APP:
          PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(ID_EXIT_APP, 0), 0);
          break;
        case IDC_BUTTON_WRITE:
          saveFile();
          break;
        case IDC_BUTTON_AUTO_REFRESH:
          do {
            PWin* w = PWin::getWindowPtr(GetDlgItem(*this, IDC_BUTTON_AUTO_REFRESH));
            if(autoRefresh) {
              autoRefresh = false;
              w->setCaption(_T("Manuale"));
              }
            else {
              autoRefresh = true;
              w->setCaption(_T("Auto"));
              }
            } while(false);
          break;
        case IDC_BUTTON_READ:
          loadFile();
          loadData();
          break;
//        case IDC_CHECKBOX_ZERO:
//          changeBase();
//          break;
        case IDC_CHECK_HEX:
          changeValue();
          break;
        case IDC_BUTTON_SVISOR_SEARCH:
          searchSVisor(true);
          break;
        case IDC_RADIOBUTTON_BYTE:
        case IDC_RADIOBUTTON_WORD:
        case IDC_RADIOBUTTON_DWORD:
          setBaseAddr(LOWORD(wParam));
          break;
        case IDC_BUTTON_GOTO_ROW:
          do {
            DWORD pos;
            GET_INT(IDC_EDIT_GOTO_ROW, pos);
            HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
            evVScrollBar(child, SB_THUMBTRACK, pos);
            } while(false);
          break;
        case IDC_BUTTON_SEARCH_TEXT:
          searchText();
          break;
        }
      switch(HIWORD(wParam)) {
        case EN_KILLFOCUS:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_MAX_ROW:
              if(SendMessage((HWND)lParam, EM_GETMODIFY, 0, 0))
                changeNumRow();
              break;
            }
          break;
        case EN_SETFOCUS:
          checkRowWithFocus(LOWORD(wParam));
          break;
        }
      break;

    case WM_CUSTOM_VSCROLL:
      lParam = (LPARAM)GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
      // passa sotto
    case WM_VSCROLL:
      evVScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
      break;

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

    case WM_DESTROY:
      KillTimer(hwnd, 1);
      break;

    case WM_TIMER:
      if(autoRefresh) {
        loadFile();
        loadData();
        }
      checkSVisor();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
extern int gSearchText(HWND edit, const baseRow* cfg, int currPos);
//----------------------------------------------------------------------------
void PD_Test::searchText()
{
  PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
  if(!pda)
    return;
  const baseRow* cfg = pda->getRows();
  int pos = gSearchText(GetDlgItem(*this, IDC_EDIT_SEARCH_TEXT), cfg, currPos);
  if(pos >= 0)
    evVScrollBar(GetDlgItem(*this, IDC_SCROLLBAR_COUNT), SB_THUMBTRACK, pos);
}
//----------------------------------------------------------------------------
void PD_Test::checkRowWithFocus(uint id)
{
  int idF = -1;
  for(uint i = 0; i < SIZE_A(Rows); ++i) {
    if(Rows[i]->hasId(id)) {
      idF = i;
      break;
      }
    }
  if(idF >= 0 && idFocus != idF) {
    if(idFocus >= 0) {
      Rows[idFocus]->invalidate();
      InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + idFocus), 0 ,1);
      }
    idFocus = idF;
    Rows[idFocus]->invalidate();
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + idFocus), 0 ,1);
    }
}
//----------------------------------------------------------------------------
#define ID_TITLE 1
//----------------------------------------------------------------------------
bool seemToBeSv(LPTSTR file)
{
  LPTSTR p = _tcsrchr(file, _T('\\'));
  ++p;
  int len = _tcslen(p) - 4;
  LPCTSTR check[] = {
    _T("NPS_"),
    };
  for(uint j = 0; j < SIZE_A(check); ++j) {
    uint l = _tcslen(check[j]);
    for(int i = 0; i < len; ++i)
      if(!_tcsnicmp(p + i, check[j], l))
        return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool sameFile(LPTSTR file, LPCTSTR path)
{
  return !_tcsicmp(file, path);
}
//----------------------------------------------------------------------------
//#define PATH_DEBUG
//----------------------------------------------------------------------------
bool samePath(LPTSTR file, LPCTSTR path)
{
  LPTSTR p = _tcsrchr(file, _T('\\'));
  if(!p)
    return false;

  *p = 0;
#ifdef PATH_DEBUG
  LPTSTR p2 = _tcsrchr(file, _T('\\'));
  *p2 = 0;
  bool same = !_tcsicmp(file, path);
  *p2 = _T('\\');
#else
  bool same = !_tcsicmp(file, path);
#endif
  *p = _T('\\');
  return same;
}
//------------------------------------------------------------------
prtEnumProcesses EnumProcesses = 0;
prtEnumProcessModules EnumProcessModules = 0;
prtGetModuleFileNameExW GetModuleFileNameEx = 0;
LPCTSTR libName = _T("PSAPI.DLL");
LPCSTR procEnumProcesses = "_EnumProcesses";
LPCSTR procEnumProcessModules = "_EnumProcessModules";
LPCSTR procGetModuleFileNameExW = "_GetModuleFileNameExW";

HMODULE hDLL = 0;
//------------------------------------------------------------------
bool loadProc()
{
  hDLL = LoadLibrary(libName);
  while(hDLL) {
    FARPROC f = GetProcAddress(hDLL, procEnumProcesses);
    if(!f)
      f = GetProcAddress(hDLL, procEnumProcesses + 1);
    if(!f)
      break;
    EnumProcesses = (prtEnumProcesses)f;

    f = GetProcAddress(hDLL, procEnumProcessModules);
    if(!f)
      f = GetProcAddress(hDLL, procEnumProcessModules + 1);
    if(!f)
      break;
    EnumProcessModules = (prtEnumProcessModules)f;

    f = GetProcAddress(hDLL, procGetModuleFileNameExW);
    if(!f)
      f = GetProcAddress(hDLL, procGetModuleFileNameExW + 1);
    if(!f)
      break;
    GetModuleFileNameEx = (prtGetModuleFileNameExW)f;
    return true;
    }
  if(hDLL) {
    FreeLibrary(hDLL);
    hDLL = 0;
    }
  return false;
}
//----------------------------------------------------------------------------
typedef
bool (*prtCheckWinSV)(const winInfo& wi, LPCTSTR path, PVect<uint>& found, LPCTSTR execRunning, uint ix);
prtCheckWinSV checkWinSV = 0;
//----------------------------------------------------------------------------
bool checkWinSV_9x(const winInfo& wi, LPCTSTR path, PVect<uint>& found, LPCTSTR execRunning, uint ix)
{
  TCHAR file[_MAX_PATH];

  DWORD pID = 0;
  GetWindowThreadProcessId(wi.hwnd, &pID);

  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, pID);
  if(INVALID_HANDLE_VALUE == hSnap) {
    DWORD err = GetLastError();
    DisplayErrorString(err);
    return false;
    }
  PROCESSENTRY32 pEntry;
  pEntry.dwSize = sizeof(pEntry);
  if(Process32First(hSnap, &pEntry)) {
    do {
      if(pEntry.th32ProcessID == pID) {
        _tcscpy(file, pEntry.szExeFile);
        break;
        }
      } while(Process32Next(hSnap, &pEntry));
    }
  CloseHandle(hSnap);

  if(execRunning) {
    if(!_tcsicmp(file, execRunning)) {
      found[0] = ix;
      return true;
      }
    }
  else {
    if(samePath(file, path) && seemToBeSv(file)) {
      uint n = found.getElem();
      found[n] = ix;
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool checkWinSV_NT(const winInfo& wi, LPCTSTR path, PVect<uint>& found, LPCTSTR execRunning, uint ix)
{
  TCHAR file[_MAX_PATH] = _T("");
  DWORD pID = 0;
  GetWindowThreadProcessId(wi.hwnd, &pID);

  HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pID);
  if(INVALID_HANDLE_VALUE == hProc) {
    DWORD err = GetLastError();
    DisplayErrorString(err);
    return false;
    }
  HMODULE hModule;
  DWORD cbReturned;
  if(EnumProcessModules( hProc, &hModule, sizeof(hModule), &cbReturned ))
    GetModuleFileNameEx(hProc, hModule, file, SIZE_A(file));
  CloseHandle(hProc);

  if(execRunning) {
    if(!_tcsicmp(file, execRunning)) {
      found[0] = ix;
      return true;
      }
    }
  else {
    if(samePath(file, path) && seemToBeSv(file)) {
      uint n = found.getElem();
      found[n] = ix;
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
// ci sono più applicazioni avviate che hanno un nome simile al SVisor
// visualizza quelle dubbie e torna la scelta dell'utente
int getChooiceSvHandle(PWin* owner, const setOfInfo& set, PVect<uint>& found)
{
  PD_chooseHandle ch(set, found, owner);
  if(IDOK == ch.modal())
    return ch.getResult();
  return -1;
}
//----------------------------------------------------------------------------
// ricerca nel path tutti gli eseguibili e li mostra in una listbox
// se l'utente sceglie di avviarlo lo avvia, attende che sia pronto e torna true
bool getChooiceSvExe(PWin* owner, LPTSTR path)
{
  PD_chooseFile cf(path, owner);

  if(IDOK == cf.modal()) {


    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_FORCEONFEEDBACK;

    PROCESS_INFORMATION pi;

    if(CreateProcess(0, path, 0, 0, 0, NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi)) {
      WaitForInputIdle(pi.hProcess, 10 * 1000);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      Sleep(3000);
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
void PD_Test::checkSVisor()
{
  PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
  if(!pda)
    return;
  HWND h = pda->getSvHandle();
  if((LONG)h == -1)
    h = 0;
  if(h) {
    if(!IsWindow(h))
      h = 0;
    }
  if(!h)
    pda->setSvHandle(0, 0);

  SET_TEXT(IDC_STATIC_HW_SV, pda->getSvName());
}
//----------------------------------------------------------------------------
void PD_Test::searchSVisor(bool req, LPCTSTR running)
{
  bool onNT = isWinNT() || isWin2000orLater();
  if(onNT && !loadProc())
    return;
  checkWinSV = onNT ? checkWinSV_NT : checkWinSV_9x;
  performSearchSVisor(req, running);
  if(onNT) {
    FreeLibrary(hDLL);
    hDLL = 0;
    }
}
//----------------------------------------------------------------------------
void PD_Test::performSearchSVisor(bool req, LPCTSTR running)
{
  PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
  if(!pda)
    return;
  winList WList;
  WList.owner = *pda;

  TCHAR path[_MAX_PATH];
  GetCurrentDirectory(SIZE_A(path), path);
  {
  PVect<uint> found;
  if(getListHwnd(WList)) {
    const setOfInfo& set = WList.set;
    uint nElem = set.getElem();
    for(uint i = 0; i < nElem; ++i)
      if(checkWinSV(set[i], path, found, running, i))
        break;
    nElem = found.getElem();
    if(1 == nElem) {
      uint ix = found[0];
      pda->setSvHandle(set[ix].hwnd, set[ix].caption);
      SET_TEXT(IDC_STATIC_HW_SV, set[ix].caption);
      return;
      }
    if(nElem > 1 && req) {
      int result = getChooiceSvHandle(this, set, found);
      if(result >= 0) {
        uint ix = found[result];
        pda->setSvHandle(set[ix].hwnd, set[ix].caption);
        SET_TEXT(IDC_STATIC_HW_SV, set[ix].caption);
        }
      return;
      }
    }
  if(!req)
    return;
  }
  // cerca l'eseguibile (eventualmente con una scelta) nel path
  // corrente e chiede se avviarlo. In caso affermativo in path
  // viene tornato il path compreso il file e si richiama per cercare l'handle
  if(getChooiceSvExe(this, path)) {
    Sleep(500);
    performSearchSVisor(false, path);
    }
}
//----------------------------------------------------------------------------
void PD_Test::checkNumRow(DWORD nrow)
{
  DWORD currmax = GetDlgItemInt(*this, IDC_EDIT_MAX_ROW, 0, 0);
  if(currmax > nrow) {
    currmax = nrow;
    SetDlgItemInt(*this, IDC_EDIT_MAX_ROW, nrow, 0);
    maxShow = currmax;
    setScrollRange();
    }
}
//----------------------------------------------------------------------------
void PD_Test::changeNumRow()
{
  maxShow = GetDlgItemInt(*this, IDC_EDIT_MAX_ROW, 0, 0);
  if(maxShow > MAX_ADDRESSES) {
    maxShow = MAX_ADDRESSES;
    SetDlgItemInt(*this, IDC_EDIT_MAX_ROW, maxShow, 0);
    }
  setScrollRange();
  loadData();
}
//----------------------------------------------------------------------------
void PD_Test::changeBase()
{
  int zeroBased = 0; //IS_CHECKED(IDC_CHECKBOX_ZERO) ? 0 : 1;
  int pos = currPos + zeroBased;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);
    }
}
//------------------------------------------------------------------------------
void PD_Test::changeValue()
{
  DWORD pos = currPos;
  noConvert = true;
  saveCurrData();
  noConvert = false;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos)
    Rows[i]->setValue(Values[pos]);
}
//------------------------------------------------------------------------------
void PD_Test::evVScrollBar(HWND child, int flags, int pos)
{
  int curr = SendMessage(child, SBM_GETPOS, 0, 0);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = MAX_V_SCROLL;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - PAGE_SCROLL_LEN;
      break;
    case SB_PAGERIGHT:
      pos = curr + PAGE_SCROLL_LEN;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if(pos > (int)MAX_V_SCROLL)
    pos = MAX_V_SCROLL;
  SendMessage(child, SBM_SETPOS, pos, true);

  saveCurrData();
  currPos = pos;
  loadData();
}
//------------------------------------------------------------------------------
bool PD_Test::saveData()
{
  return true;
}
//------------------------------------------------------------------------------
void PD_Test::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos)
    Values[pos] = Rows[i]->getValue();
}
//------------------------------------------------------------------------------
void PD_Test::loadData()
{
  int pos = currPos;

  int zeroBased = 0; //IS_CHECKED(IDC_CHECKBOX_ZERO) ? 0 : 1;
  const baseRow* Cfg;
  PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
  if(pda)
    Cfg = pda->getRows();
  else
    return;

  bool old = setIgnoreKillFocus(true);
  for(int i = 0; i < MAX_GROUP && pos < MAX_ADDRESSES; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos + zeroBased);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);

    rangeLP rlp = {
        Cfg[pos].vMin,
        Cfg[pos].vMax,
        Cfg[pos].vMinP,
        Cfg[pos].vMaxP
        };
    Rows[i]->setAll(Cfg[pos].ipAddr, Cfg[pos].port, Cfg[pos].db, Cfg[pos].addr / baseAddr, Cfg[pos].type,
        Cfg[pos].action, rlp, Values[pos]);
    Rows[i]->setDescr(Cfg[pos].text);
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + i), 0, 0);
    }
  setIgnoreKillFocus(old);
}
//----------------------------------------------------------------------------
bool PD_Test::isRowEmpty(uint pos)
{
  const baseRow* Cfg;
  PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
  if(pda)
    Cfg = pda->getRows();
  else
    return true;
  pos += currPos;
  return !Cfg[pos].type;
}
//----------------------------------------------------------------------------
#define CHECK_ZERO_CTRL(id, base) (DEF_OFFSET_CTRL + IDC_EDIT_BYTE1_1 + base <= (id) || \
                                  (id) < IDC_EDIT_BYTE1_1 + base)
//----------------------------------------------------------------------------
bool PD_Test::hasValidZeroValue(HWND hWndChild)
{
  long id = GetWindowLong(hWndChild, GWL_ID);
  if(CHECK_ZERO_CTRL(id, OFFSET_ADDR) && CHECK_ZERO_CTRL(id, OFFSET_VMIN_P) && CHECK_ZERO_CTRL(id, OFFSET_VMIN))
    return false;
  id += DEF_OFFSET_CTRL;
  TCHAR buff[500];
  hWndChild = GetDlgItem(*this, id);
  GetWindowText(hWndChild, buff, SIZE_A(buff));
  return toBool(_tstof(buff));
}
//----------------------------------------------------------------------------
HBRUSH PD_Test::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  long id = GetWindowLong(hWndChild, GWL_ID);
  if(idFocus >= 0) {
    if(Rows[idFocus]->hasId(id)) {
      SetTextColor(hdc, ACTIVE_ROW_COLOR_TXT);
      SetBkColor(hdc, bkgColor3);
      return (Brush3);
      }
    }
  if(WM_CTLCOLORSTATIC == ctlType) {
    if(IDC_STATIC_HW_SV == id) {
      SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
      SetTextColor(hdc, RGB(0, 120, 220));
      return GetSysColorBrush(COLOR_BTNFACE);
      }
    if(IDC_STATICTEXT_1 <= id && id < IDC_STATICTEXT_1 + MAX_GROUP) {
      if(idFocus == id - IDC_STATICTEXT_1) {
        SetTextColor(hdc, ACTIVE_ROW_COLOR_TXT);
        SetBkColor(hdc, bkgColor3);
        return (Brush3);
        }
      if(isRowEmpty(id - IDC_STATICTEXT_1)) {
        SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
        return GetSysColorBrush(COLOR_BTNFACE);
        }
      SetTextColor(hdc, RGB(0, 0, 128));
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
    if(IDC_EDIT_BYTE1_1 <= id && id < IDC_EDIT_BYTE1_1 + OFFSET_DESCR_TEST + DEF_OFFSET_CTRL_TEST) {
      long id2 = (id - IDC_EDIT_BYTE1_1) % DEF_OFFSET_CTRL_TEST;
      if(isRowEmpty(id2)) {
        SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
        return GetSysColorBrush(COLOR_BTNFACE);
        }
      SetTextColor(hdc, RGB(0, 0, 0));
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
    return 0;
    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    if(IDC_EDIT_MAX_ROW == id || IDC_EDIT_GOTO_ROW == id || IDC_EDIT_SEARCH_TEXT == id) {
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
    if(IDC_EDIT_BYTE1_1 + OFFSET_VALUE <= id && id <  IDC_EDIT_BYTE1_1 + OFFSET_VALUE + DEF_OFFSET_CTRL_TEST) {
      long id2 = (id - IDC_EDIT_BYTE1_1) % DEF_OFFSET_CTRL_TEST;
      if(isRowEmpty(id2)) {
        SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
        return GetSysColorBrush(COLOR_BTNFACE);
        }
      SetTextColor(hdc, RGB(0, 0, 128));
      SetBkColor(hdc, bkgColor);
      return (Brush);
      }
    }
  return 0;
}
//----------------------------------------------------------------------------
void PD_Test::setBaseAddr(uint /*idCtrl*/)
{
  saveCurrData();
  if(IS_CHECKED(IDC_RADIOBUTTON_WORD))
    baseAddr = 2;
  else if(IS_CHECKED(IDC_RADIOBUTTON_DWORD))
    baseAddr = 4;
  else
    baseAddr = 1;
  loadData();
}
//----------------------------------------------------------------------------
