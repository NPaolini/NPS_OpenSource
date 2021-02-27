//---------- mainWin.cpp -----------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "mainwin.h"
#include "PModDialog.h"
//#include "P_ModListFiles.h"
#include "pMenuBtn.h"
#include "pSplitwin.h"
#include "popensave.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "pregistry.h"
#include "macro_utils.h"
#include "fv_miniat_images.h"
#include "clientWin.h"
#include "extMonitor.h"
#include "P_Setup.h"
#include "P_Info.h"
#include "pNumberImg.h"
//----------------------------------------------------------------------------
#include "p_manage_ini.h"
//----------------------------------------------------------------------------
#define REL_SYSTEM_PATH _T("LastPath")
#define SELECT_PATH _T("Selected")
#define CLIENT_PERC _T("Perc")
//----------------------------------------------------------------------------
#define COORDS_PATH _T("coords")
#define SIZE_PATH   _T("size")
//----------------------------------------------------------------------------
#define AUDIO_BASE_TIME _T("audioBaseTime")
#define AUDIO_BASE_VOLUME _T("audioBaseVolume")

#define AUDIO_ALERT_VOLUME _T("audioAlertVolume")
//----------------------------------------------------------------------------
void makeKeyPath(LPTSTR path, size_t dim)
{
  GetModuleDirName(dim, path);
#ifdef _DEBUG
  dirName(path);
#endif
  appendPath(path, INI_NAME);
}
//----------------------------------------------------------------------------
static bool compare(LPCTSTR p1, LPCTSTR p2, size_t len)
{
  for(uint i = 0; i < len; ++i)
    if(p1[i] != p2[i])
      return false;
  return true;
}
//----------------------------------------------------------------------------
void makeRelPath(LPTSTR path, size_t dim)
{
  TCHAR t[_MAX_PATH];
  t[0] = 0;
  makeTrueFullPath(t, SIZE_A(t));

  TCHAR full[_MAX_PATH];
  _tcscpy_s(full, path);
  makeTrueFullPath(full, SIZE_A(full));
  uint len = _tcslen(t);
  if(compare(t, full, len))
    _tcscpy_s(path, dim, full + len + 1);
}
//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR path)
{
  TCHAR file[_MAX_PATH];
  makeKeyPath(file, SIZE_A(file));
  manageIni mi(file);
  mi.parse();
  ini_param ip;
  ip.name = keyName;
  ip.value = path;

  if(!mi.existBlock(SUB_BASE_KEY))
    mi.createBlock(SUB_BASE_KEY);

  if(!mi.changeValue(ip, SUB_BASE_KEY))
    mi.createParam(ip, SUB_BASE_KEY);
  mi.save();
}
//----------------------------------------------------------------------------
void setKeyParam(LPCTSTR keyName, DWORD value)
{
  TCHAR buff[_MAX_PATH];
  wsprintf(buff, _T("%d"), (long)value);
  setKeyPath(keyName, buff);
}
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPTSTR path)
{
  path[0] = 0;
  TCHAR file[_MAX_PATH];
  makeKeyPath(file, SIZE_A(file));
  manageIni mi(file);
  mi.parse();
  LPCTSTR value = mi.getValue(keyName, SUB_BASE_KEY);
  if(value)
    copyStrZ(path, value);
}
//----------------------------------------------------------------------------
void getKeyParam(LPCTSTR keyName, LPDWORD value)
{
  *value = 0;
  TCHAR buff[_MAX_PATH];
  getKeyPath(keyName, buff);
  if(buff[0])
    *value = (DWORD)_ttol(buff);
}
//----------------------------------------------------------------------------
void setPath(LPCTSTR path)
{
  setKeyPath(REL_SYSTEM_PATH, path);
}
//----------------------------------------------------------------------------
void getPath(LPTSTR path)
{
  getKeyPath(REL_SYSTEM_PATH, path);
}
//----------------------------------------------------------------------------
void setImgPath(LPCTSTR path)
{
  setKeyPath(IMG_PATH, path);
}
//----------------------------------------------------------------------------
void getImgPath(LPTSTR path)
{
  getKeyPath(IMG_PATH, path);
}
//----------------------------------------------------------------------------
bool isMaximized(HWND hwnd)
{
  WINDOWPLACEMENT wp;
  ZeroMemory(&wp, sizeof(wp));
  wp.length = sizeof(wp);
  if(GetWindowPlacement(hwnd, &wp))
    return SW_SHOWMAXIMIZED == (SW_SHOWMAXIMIZED & wp.showCmd); // MaximizedWin;
  return false;
}
//----------------------------------------------------------------------------
bool needMaximized()
{
  DWORD coords = 0;
  getKeyParam(COORDS_PATH, &coords);
  return (DWORD)-1 == coords;
}
//-----------------------------------------------------------
static void checkOut(int& x, int& y)
{
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);

  if(x < 0)
    x = 0;
  else if(x > width - 100)
    x = width - 100;
  if(y < 0)
    y = 0;
  else if(y > height - 100)
    y = height - 100;
}
//----------------------------------------------------------------------------
struct idc_id
{
  uint idObj;
  uint idText;
};
//----------------------------------------------------------------------------
static setOfString gSet;
//----------------------------------------------------------------------------
LPCTSTR getStringOrDef(uint code, LPCTSTR def)
{
  LPCTSTR p = gSet.getString(code);
  return p ? p : def;
}
//-------------------------------------------------------------------
void setLang(HWND hwnd, uint idText)
{
  LPCTSTR txt = gSet.getString(idText);
  if(txt)
    SetWindowText(hwnd, txt);
}
//-------------------------------------------------------------------
void setLang(HWND hwnd, uint idc, uint idText)
{
  LPCTSTR txt = gSet.getString(idText);
  if(txt)
    SetDlgItemText(hwnd, idc, txt);
}
//------------------------------------------------------------------------------
static void saveAudioParam(PVect<audio_config>& set)
{
  do {
    const audio_config& cfg = set[0];
    setKeyParam(AUDIO_BASE_TIME, cfg.secToPlay);
    setKeyParam(AUDIO_BASE_VOLUME, cfg.Volume);
  } while(false);
  do {
    const audio_config& cfg = set[1];
    setKeyParam(AUDIO_ALERT_TIME, cfg.secToPlay);
    uint vol = cfg.Volume;
    if(vol < 500 || vol > 1000)
      vol = 1000;
    setKeyParam(AUDIO_ALERT_VOLUME, vol);
  } while(false);
}
//-----------------------------------------------------------
static uint idTimerCheck = 124578;
//---------------------------------------------------------------------
unsigned FAR PASCAL ReaderDllProc(void*)
{
  HMODULE hsv = LoadLibrary(NAME_SVBASE);
  if(hsv)
    FreeLibrary(hsv);
  return 0;
}
//---------------------------------------------------------------------
static void checkKey(HWND hwnd)
{
#ifdef NO_USE_KEY
#else
  DWORD idThread;
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReaderDllProc, 0, 0, &idThread);
  if(hThread)
    CloseHandle(hThread);
  uint tm = rand() % 30 + 10;
  tm *= 60 * 1000;
  KillTimer(hwnd, idTimerCheck);
  SetTimer(hwnd, idTimerCheck, tm, 0);
#endif
}
//--------------------------------------------------------------------
mainWin::mainWin(HINSTANCE hInstance) :
      PMainWin(_T("NpS Escape Room"), hInstance), mainSplit(0), idTimer(0), idTimerTime(0), ManAudio(0), lastTick(0)
{
  TCHAR path[_MAX_PATH];
  GetModuleDirName(SIZE_A(path), path);
  appendPath(path, _T("Escape_Room.lng"));
  gSet.add(path);
}
//----------------------------------------------------------------------------
mainWin::~mainWin()
{
  // prima andava in crash alla chiusura, errore nella GDI+, forse perché questo oggetto puntava all'immagine di base
  // dell'oggetto client che veniva distrutto prima ...
  // Per lo stesso motivo ho usato myPSplitWin per far si che venga distrutta prima la finestra che usa l'immagine base
  // e quindi l'oggetto (clientWin) che ha l'immagine viene distrutto per ultimo
  delete extMon;
  PVect<audio_config> cfg;
  cfg[0] = ManAudio->getCfg(0);
  cfg[1] = ManAudio->getCfg(1);
  saveAudioParam(cfg);
  delete ManAudio;
  SetThreadExecutionState(ES_CONTINUOUS);
  destroy();
  delete mainSplit;
}
//------------------------------------------------------------------------------
int mainWin::getVolumeAudio()
{
  return (int)ManAudio->getCfg(0).Volume;
}
//------------------------------------------------------------------------------
clientWin* getClientImage(PWin* child)
{
  mainWin* mv = getParentWin<mainWin>(child);
  return mv->getClientImage();
}
//------------------------------------------------------------------------------
extMonitor* getMonitor(PWin* child)
{
  mainWin* mv = getParentWin<mainWin>(child);
  return mv->getMonitor();
}
//-----------------------------------------------------------
#define IDD_MENU_BTN 1
#define IDD_MAIN_SPLIT 2
#define IDD_CLIENT_SPLIT 3
#define IDD_CODE 4
#define IDD_FILE 5
//---------------------------------------------------------
#define MIN_WIDTH  (960 + 120 + 100)
#define MIN_HEIGHT 540
//------------------------------------------------------------------------------
static void appendAudioParam(manageIni& mi, LPTSTR target, size_t dim, LPCTSTR param, LPCTSTR ifNotFound = 0)
{
  _tcscat_s(target, dim, _T(","));
  LPCTSTR value = mi.getValue(param, SUB_BASE_KEY);
  if(!value)
    value = ifNotFound ? ifNotFound : _T("0");
  _tcscat_s(target, dim, value);
}
//------------------------------------------------------------------------------
static void makeAudioParam(PVect<LPCTSTR>& set)
{
  TCHAR file[_MAX_PATH];
  GetModuleDirName(SIZE_A(file), file);
#ifdef _DEBUG
  dirName(file);
#endif
  appendPath(file, INI_NAME);
  manageIni mi(file);
  mi.parse();
  TCHAR buff[_MAX_PATH * 2] = _T("1");

  appendAudioParam(mi, buff, SIZE_A(buff), AUDIO_BASE_TIME);
  appendAudioParam(mi, buff, SIZE_A(buff), AUDIO_BASE_VOLUME);
  appendAudioParam(mi, buff, SIZE_A(buff), AUDIO_BASE_FOLDER, _T("sound"));
  appendAudioParam(mi, buff, SIZE_A(buff), AUDIO_BASE, _T("base.mp3"));

  set[0] = str_newdup(buff);
  _tcscpy_s(buff, _T("1"));

  appendAudioParam(mi, buff, SIZE_A(buff), AUDIO_ALERT_TIME);
  appendAudioParam(mi, buff, SIZE_A(buff), AUDIO_ALERT_VOLUME);
  appendAudioParam(mi, buff, SIZE_A(buff), AUDIO_ALERT_FOLDER, _T("effect"));
  appendAudioParam(mi, buff, SIZE_A(buff), AUDIO_ALERT, _T("alert.mp3"));

  set[1] = str_newdup(buff);
}
//------------------------------------------------------------------------------
bool mainWin::create()
{
  PSplitWin* clientSplit = new PSplitWin(this, IDD_CLIENT_SPLIT, 200, 5, PSplitter::PSW_VERT);
  menuBtn = new pMenuBtn(clientSplit, IDD_MENU_BTN);

  FMI = new fv_miniat_images(this, 1111, 0, 0, 500, 300);

  clientSplitWin = new myPSplitWin(clientSplit, IDD_FILE + 1, 200, 5, PSplitter::PSW_VERT);

  clientFile = new clientWin(clientSplitWin, IDD_FILE);
  tl = new P_TimerLed(clientSplitWin, ManTimer, IDD_FILE + 2);

  clientSplitWin->setWChild(tl, clientFile);
  DWORD perc;
  getKeyParam(CLIENT_PERC, &perc);
  if(perc)
    clientSplitWin->getSplitter()->setPerc(perc);

  clientSplit->setWChild(menuBtn, clientSplitWin);

  PSplitter* spl = clientSplit->getSplitter();
//  spl->setDim1(OFFS_BTN * 2 + SIZE_BTN_V, true);
  spl->setDim1(OFFS_BTN * 2 + SIZE_BTN_V + OFFS_ROW * 2 + SIZE_ROW, true);
  spl->setLock(PSplitter::PSL_FIRST);
  spl->setDrawStyle(PSplitter::PSD_LEFT);


  mainSplit = new PSplitter(this, FMI, clientSplit);

  mainSplit->setDim1(160 * 2 + 24, false);
  mainSplit->setLock(PSplitter::PSL_FIRST);
  mainSplit->setDrawStyle(PSplitter::PSD_LEFT);
  DWORD coords = 0;
  getKeyParam(COORDS_PATH, &coords);
  if(coords && (DWORD)-1 != coords) {
    Attr.x = (short)LOWORD(coords);
    Attr.y = (short)HIWORD(coords);
    checkOut(Attr.x, Attr.y);
    getKeyParam(SIZE_PATH, &coords);
    if(coords && (DWORD)-1 != coords) {
      Attr.w = (short)LOWORD(coords);
      Attr.h = (short)HIWORD(coords);
      }
    }
  extMon = new extMonitor(this, ManTimer);
  extMon->getSplitter()->setPerc(perc);
  ManTimer.reset();
  ManTimer.addRefresh(tl);
  ManTimer.addRefresh(extMon->getNumberLed());
  if(!baseClass::create())
    return false;
  TCHAR path[_MAX_PATH];
  getImgPath(path);
  if(*path)
    FMI->setBasePath(path);
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));

  bool success = mainSplit->create();
  DWORD selected;
  getKeyParam(SELECT_PATH, &selected);
  FMI->setSelected((int)selected);
  idTimer = SetTimer(*this, 2348, 3 * 1000, 0);
  
  do {
    DWORD vol;
    getKeyParam(AUDIO_ALERT_VOLUME, &vol);
    if(vol < 500 || vol > 1000)
      vol = 1000; 
    setKeyParam(AUDIO_ALERT_VOLUME, vol);
    } while(false);

  PVect<LPCTSTR> t;
  makeAudioParam(t);
  ManAudio = new manageAudio();
  ManAudio->load(t);
  flushPAV(t);
  SetTimer(*this, idTimerCheck, 5 * 60 * 1000, 0);
  menuBtn->getSlider()->setSelections(0, getVolumeAudio());
//  menuBtn->getSlider()->setValue(getVolumeAudio());
  resumeTimer();
  return success;
}
//----------------------------------------------------------------------------
bool mainWin::preProcessMsg(MSG& msg)
{
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
// #define getStringOrDef(n,t) t
//----------------------------------------------------------------------------
#define SAVE_QUERY_MSG getStringOrDef(IDT_SAVE_QUERY, _T("Il file è stato modificato\r\nVuoi salvare prima di proseguire?"))
#define SAVE_QUERY_TIT getStringOrDef(IDT_GENERIC_WARNING, _T("Attenzione"))
#define SAVE_QUERY_FLAG (MB_YESNOCANCEL | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
#define UPD_QUERY_MSG getStringOrDef(IDT_UPD_QUERY, _T("Il file è stato modificato\r\nProseguendo perderai tutte le modifiche\r\nVuoi proseguire?"))
#define UPD_QUERY_FLAG (MB_YESNO | MB_ICONINFORMATION)
//-----------------------------------------------------------
LRESULT mainWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  mainSplit->windowProcSplit(hwnd, message, wParam, lParam);
  if(WM_CUSTOM_MSG_SLIDER == message) {
    switch(LOWORD(wParam)) {
//      case CM_MOVE_SLIDER:
//      case CM_MOVE_LEFT_SLIDER:
//      case CM_MOVE_RIGHT_SLIDER:
      case CM_END_MOVE_SLIDER:
//      case CM_END_MOVE_LEFT_SLIDER:
      case CM_END_MOVE_RIGHT_SLIDER:
        ManAudio->setVolume(0, (DWORD)lParam);
        break;
      }
    }
  switch(message) {
#if 1
    case WM_SYSCOMMAND:
      switch(wParam & 0xFFF0) {
        case SC_MINIMIZE:
          do {
            bool isMax = isMaximized(*extMon);
      #if true
            // non c'è modo di evitare il flickering del minimize/restore della finestra monitor ...
            PostMessage(*this, WM_CUSTOM_BY_BTN, MAKEWPARAM(CM_BTN_FULL_SCREEN, isMax ? 1 : 2), 0);
      #else
            PWin t(0, _T("test"));
            extMon->setParent(&t);
//            LockWindowUpdate(*extMon);
            LRESULT res = baseClass::windowProc(hwnd, message, wParam, lParam);
            SendMessage(*this, WM_CUSTOM_BY_BTN, MAKEWPARAM(CM_BTN_FULL_SCREEN, isMax ? 1 : 2), 0);
            extMon->setParent(this);
//            LockWindowUpdate(0);
            return res;
      #endif
          } while(false);
          break;
        }
      break;
#endif
    case WM_DESTROY:
      if(isMaximized(hwnd)) {
        setKeyParam(COORDS_PATH, (DWORD)-1);
        }
      else {
        PRect r;
        GetWindowRect(hwnd, r);
        DWORD t = MAKELONG(r.left, r.top);
        setKeyParam(COORDS_PATH, t);
        t = MAKELONG(r.Width(), r.Height());
        setKeyParam(SIZE_PATH, t);
        }
      do {
        DWORD value = FMI->getSelected();
        setKeyParam(SELECT_PATH, value);
        value = clientSplitWin->getSplitter()->getPerc();
        setKeyParam(CLIENT_PERC, value);
        } while(false);
      if(idTimerTime) {
        KillTimer(hwnd, idTimerTime);
        idTimerTime = 0;
        }
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      if(idTimerCheck) {
        KillTimer(hwnd, idTimerCheck);
        idTimerCheck = 0;
        }
      break;
    case WM_GETMINMAXINFO:
      do {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
        lpmmi->ptMinTrackSize.x = MIN_WIDTH;
        lpmmi->ptMinTrackSize.y = MIN_HEIGHT;
        } while(false);
      break;
    case WM_CUSTOM_BY_BTN:
      switch(LOWORD(wParam)) {
        case CM_BTN_DONE:
          PostQuitMessage(0);
          break;
        case CM_BTN_SETUP:
          P_Setup(this).modal();
          break;
        case CM_BTN_FULL_SCREEN:
          switch(HIWORD(wParam)) {
            case 0:
            default:
              extMon->toggleFullScreen();
              break;
            case 1:
              ShowWindow(*extMon, SW_MAXIMIZE);
              break;
            case 2:
              ShowWindow(*extMon, SW_RESTORE);
              break;
            }
          break;
        case CM_BTN_MOVE_SCREEN:
          moveMon();
          break;

        case CM_BTN_AUDIO_BASE:
//          ManAudio->setPos(0, 0);
          ManAudio->toggle(0);
          break;
        case CM_BTN_AUDIO_ALERT:
          ManAudio->setPos(1, 0);
          ManAudio->toggle(1);
          break;
        case CM_BTN_INFO:
          do {
            bool needRefresh = false;
            P_Info(needRefresh, this).modal();
            if(needRefresh)
              refreshInfo();
          } while(false);
          break;

        case CM_BTN_RESET_TIME:
          resetTimer();
          break;
        case CM_BTN_TIMER_PAUSE_RESUME:
          if(isRunning())
            pauseTimer();
          else
            resumeTimer();
          break;

        case IDC_BTN_TIPS:
          addTips();
          break;
        case IDC_CBX_TIPS:
          changeTips();
          break;
        case IDC_CBX_SOUND:
        case IDC_CBX_ALERT:
          switch(HIWORD(wParam)) {
            case CBN_SELCHANGE:
              changeAudio(IDC_CBX_SOUND == LOWORD(wParam));
              break;
            }
          break;
        }
      break;
    case WM_COMMAND:
      break;
    case WM_TIMER:
      if(idTimerTime == wParam)
        checkTimer();
      else {
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
        if(idTimerCheck == wParam)
          checkKey(hwnd);
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void mainWin::addTips()
{
  if(menuBtn->saveCurrTips())
    refreshInfo();
}
//-----------------------------------------------------------
void mainWin::changeTips()
{
  int sel = menuBtn->getCurrSel(2) + 1;
  if(!sel)
    return;
  int old = 0;
  getKeyParam(TEXT_CURR, (LPDWORD)&old);
  if(old != sel) {
    setKeyParam(TEXT_CURR, sel);
    refreshInfo(true);
    }
}
//-----------------------------------------------------------
void mainWin::changeAudio(bool baseSound)
{
  TCHAR saved[_MAX_PATH];
  TCHAR curr[_MAX_PATH];

  if(baseSound) {
    getKeyPath(AUDIO_BASE, saved);
    menuBtn->getCurrSel(curr, 0);
    }
  else {
    getKeyPath(AUDIO_ALERT, saved);
    menuBtn->getCurrSel(curr, 1);
    }
  if(_tcsicmp(saved, curr)) {
    if(baseSound) {
      setKeyPath(AUDIO_BASE, curr);
      getKeyPath(AUDIO_BASE_FOLDER, saved);
      }
    else {
      setKeyPath(AUDIO_ALERT, curr);
      getKeyPath(AUDIO_ALERT_FOLDER, saved);
      }
    LPTSTR p = saved + _tcslen(saved);
    *p++ = _T('\\');
    *p = 0;
    _tcscat_s(saved, curr);
    uint ix = !baseSound;
    bool running = ManAudio->isRunning(ix);
    ManAudio->stop(ix);
    ManAudio->setPos(ix, 0);
    ManAudio->close(ix);
    Sleep(100);
    ManAudio->setFileAudio(saved, ix);
    ManAudio->setPos(ix, 0);
    if(running) {
      ManAudio->start(ix);
      }
    }
}
//-----------------------------------------------------------
void mainWin::moveMon()
{
  PRect r;
  GetWindowRect(*extMon, r);
  int w = GetSystemMetrics(SM_CXSCREEN);
  if((r.right + r.left) / 2 > w)
    r.MoveTo(r.left - w, r.top);
  else
    r.MoveTo(w + r.left, r.top);
  extMon->setWindowPos(0, r, SWP_NOZORDER);
}
//-----------------------------------------------------------
void mainWin::refreshFolderImg()

{
  TCHAR path[_MAX_PATH];
  getImgPath(path);
  FMI->setBasePath(path);
}
//-----------------------------------------------------------
void mainWin::refreshAudioBase(uint ix)
{
  bool running = ManAudio->isRunning(ix);
  ManAudio->close(ix);
  TCHAR path[_MAX_PATH];
  if(!ix) {
    getKeyPath(AUDIO_BASE_FOLDER, path);
    LPTSTR p = path + _tcslen(path);
    *p++ = _T('\\');
    getKeyPath(AUDIO_BASE, p);
    running &= menuBtn->refreshCbx(0, p);
    }
  else {
    getKeyPath(AUDIO_ALERT_FOLDER, path);
    LPTSTR p = path + _tcslen(path);
    *p++ = _T('\\');
    getKeyPath(AUDIO_ALERT, p);
    running &= menuBtn->refreshCbx(1, p);
    }
  do {
    if (ManAudio->isClosed(ix))
      break;
    Sleep(50);
  } while (true);
  ManAudio->setFileAudio(path, ix);
  ManAudio->setPos(ix, 0);
  if(running) {
    ManAudio->start(ix);
    }
}
//-----------------------------------------------------------
void mainWin::refreshAudioBaseTime(uint ix)
{
  DWORD value;
  getKeyParam(AUDIO_ALERT_TIME, &value);
  ManAudio->setSecToPlay(ix, value);
}
//-----------------------------------------------------------
void mainWin::refreshInfo(bool byCbx)
{
  InvalidateRect(*clientSplitWin, 0, 0);
  extMon->refreshInfo();
  if(!byCbx) {
    int sel = 0;
    getKeyParam(TEXT_CURR, (LPDWORD)&sel);
    menuBtn->reload(2);
    menuBtn->setCurrSel(sel - 1, 2);
    }
}
//-----------------------------------------------------------
void mainWin::resetTimer()
{
  ManTimer.reset();
  pauseTimer(false);
  tl->reset();
  extMon->reset();
  lastTick = 0;
}
//-----------------------------------------------------------
void mainWin::resumeTimer()
{
  pauseTimer();
  idTimerTime = SetTimer(*this, 2358, 151, 0);
//  tl->resume();
//  extMon->resume();
}
//-----------------------------------------------------------
void mainWin::pauseTimer(bool all)
{
  if(idTimerTime) {
    KillTimer(*this, idTimerTime);
    idTimerTime = 0;
    }
  lastTick = 0;
/*
  if(all) {
    tl->pause();
    extMon->pause();
    }
*/
}
//-----------------------------------------------------------
void mainWin::checkTimer()
{
  DWORD tick = GetTickCount();
  if(tick - lastTick >= 1000) {
    if(lastTick) {
      ManTimer.calcNow();
      ManTimer.refreshAll();
      lastTick += 1000;
      int currVal = ManTimer.getValue();
      if(!currVal)
        pauseTimer(false);
      }
    else
      lastTick = tick;
    }
 }
//----------------------------------------------------------------------------
int getResultMenu(PWin* owner, const menuInfo* mInfo, uint nElem)
{
  HMENU hmenu = CreatePopupMenu();
  if(!hmenu)
    return -1;
  const int firstId = 10001;
  for(uint i = 0; i < nElem; ++i)
    AppendMenu(hmenu, MF_STRING | (mInfo[i].grayed ? MF_GRAYED : 0), firstId + i, mInfo[i].item);

  UINT flag = TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN;
  POINT Pt;
  GetCursorPos(&Pt);
  int result = TrackPopupMenu(hmenu, flag, Pt.x, Pt.y, 0, *owner, 0);
  DestroyMenu(hmenu);
  result -= firstId;
  if(result < 0 || (uint)result >= nElem)
    return -1;
  return result;
}
//----------------------------------------------------------------------------
