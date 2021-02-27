//-------- mainapp.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <float.h>
#include "mainApp.h"
#include "mainclient.h"
#include "perif.h"
#include "memperif.h"

#include "set_cfg.h"
#include "config.h"
#include "def_dir.h"
#include "1.h"
#include "manageSplashScreen.h"
//----------------------------------------------------------------------------
// se attiva non permette l'avvio di più applicazioni
//bool ONLY_ONE = false;
//----------------------------------------------------------------------------
mainClient* allocMainClient(PWin* parent, HINSTANCE hInstance)
{
  _control87(MCW_EM,MCW_EM);
#define ALARM_CRIPTED
#ifdef ALARM_CRIPTED
  if(isCrypted()) {
    load_CriptedTxt(getDecryptCode());
    load_CriptedAlarm(getDecryptCode());
    }
  else {
    load_Txt();
    load_Alarm();
    }
#else
  if(isCrypted())
    load_CriptedTxt(getDecryptCode());
  else
    load_Txt();
  load_Alarm();
#endif

  LPCTSTR path = getString(ID_REMOTE_PATH);
  if(path)
    setBasePath(path);
  else {
    TCHAR p[_MAX_PATH];
    GetCurrentDirectory(SIZE_A(p), p);
    setBasePath(p);
    }
  makeAllDir();
  load_Global_Txt();

  mainClient* client = new mainClient(parent, IDD_CLIENT, hInstance);
  LPCTSTR p = getString(ID_TIMER_APP);
  if(p)
    client->setTimeForTimer(_ttoi(p));

  return client;
}
//----------------------------------------------------------------------------
void customizeMain(PWin *w)
{
  w->Attr.style &= ~(WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE | WS_MAXIMIZEBOX);
  LPCTSTR p = getString(ID_HIDE_BAR);
  if(p && 1 == _ttoi(p)) {
    w->Attr.style &= ~WS_CAPTION;
    w->Attr.style |= WS_POPUP;
    w->Attr.exStyle &= ~WS_EX_APPWINDOW;
    }

//  w->Attr.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
}
//----------------------------------------------------------------------------
// deve tornare il nome della classe
LPCTSTR getClassName()
{
  return _T("NPS_BaseClass");
}
//----------------------------------------------------------------------------
static void run(LPTSTR path, uint delay)
{
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;
//  si.wShowWindow = SW_HIDE;
  si.wShowWindow = SW_SHOWMINNOACTIVE;

//  si.wShowWindow = SW_SHOWNA;

  PROCESS_INFORMATION pi;

  if(CreateProcess(0, path, 0, 0, 0, NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi)) {
//    WaitForInputIdle(pi.hProcess, 10 * 1000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    Sleep(delay);
    }
}
//----------------------------------------------------------------------------
static bool isExe(LPCTSTR name)
{
  int len = _tcslen(name) - 4;
  if(len <= 0)
    return false;
  return !_tcsicmp(_T(".exe"), name + len);
}
//----------------------------------------------------------------------------
extern LPCTSTR getPrphOnMemName();
static bool isOnMemPeriph(LPCTSTR name)
{
  return !_tcsicmp(getPrphOnMemName(), name);
}
//----------------------------------------------------------------------------
// handle per inviare info sul caricamento delle periferiche
//extern HWND getHWSplashScreen();
extern bool sendMessageSplash(int id, LPCTSTR msg);
//----------------------------------------------------------------------------
// nuova specifica per plc diverso dalla prima periferica
uint WM_REAL_PLC = WM_PLC;
//----------------------------------------------------------------------------
#define PERIFS_USED 8
//----------------------------------------------------------------------------
// avvia gli applicativi esterni per la comunicazione diretta con le
// periferiche ed instanzia gli oggetti interni per la comunicazione che
// passa poi alla finestra di dialogo principale
void runPerif(PWin* main)
{
/**/
  mainClient *Main = dynamic_cast<mainClient*>(main);
  if(!Main)
    return;

  // se richiamato durante l'esecuzione deve eliminare le periferiche caricate
  perifsSet *pSet = Main->getPerifs();
  pSet->removeAll();

  // la prima volta deve allocare un oggetto per l'inizializzazione
  config cCfg;
  cfg sCfg = cCfg.getAll();

  long hwnd = reinterpret_cast<long>(main->getHandle());

  LPCTSTR p = getString(ID_READ_ONLY_PROGRAM);
  bool readOnly = toBool(p && _ttoi(p));

  p = getString(ID_NUM_PRF_PLC);
  if(p) {
    WM_REAL_PLC = _ttoi(p);
    if(WM_REAL_PLC < WM_PLC || WM_REAL_PLC >= (WM_PLC + PERIFS_USED))
      WM_REAL_PLC = WM_PLC;
    }

  uint delay = 1000;
  p = getString(ID_DELAY_BETWEEN_PRPH);
  if(p)
    delay = _ttoi(p);
  for(int i = 0; i < PERIFS_USED; ++i) {
    if(sCfg.Perif[i]) {    // se è attivato il flag
      TCHAR path[_MAX_PATH];
      copyStrZ(path, sCfg.pathPerif[i]);

      bool onMemPrf = isOnMemPeriph(path);
      if(readOnly || onMemPrf || isExe(path)) { // se c'è il nome dell'eseguibile

        sendMessageSplash(WMC_LOADING_PRF_0 + i + WM_PLC, path);
        gestPerif *prf;
        // se è il plc carica l'oggetto specifico
        if(i + WM_PLC == WM_REAL_PLC && !onMemPrf)
          prf = new perifPLC(Main);
        else {
          if(onMemPrf)
            prf = new memPerif(Main, WM_PLC + i);
          else
            prf = new perif(Main, WM_PLC + i);
          }
        Main->addPerif(prf);

        // se è solo lettura è inutile avviare il driver
        // se è in memoria non esiste un driver da avviare
        if(readOnly || onMemPrf) {
          sendMessageSplash(WMC_LOADED_PRF_0 + i + WM_PLC, path);
          continue;
          }
        // i parametri possono essere racchiusi in un intero
        // per la decifrazione occorre utilizzare la stessa procedura
        // presente nel file D_Param.cpp
        uint param = *reinterpret_cast<uint*>(&(sCfg.parPerif[i]));

        p = getString(ID_TYPE_ADDRESS_PLC + i);
        p = findNextParamTrim(p, 2);
        bool noMMAPFile = false;
        if(p)
          noMMAPFile = toBool(_ttoi(p));
        int offsPrf = sCfg.OffsPrf__(i);
        TCHAR buff[500];
        if(sCfg.noShowPeriph)
          wsprintf(buff, _T("%s /H%ld /C%u /O%d /I%d /N"), path, hwnd, param, offsPrf, WM_PLC + i);
        else
          wsprintf(buff, _T("%s /H%ld /C%u /O%d /I%d"), path, hwnd, param, offsPrf, WM_PLC + i);
        if(noMMAPFile)
          _tcscat_s(buff, _T(" /M"));
        run(buff, delay);
        }
      }
    }
  int prfExt = min(sCfg.__ExtendedPeriphOnMem, MAX_EXT_ONMEM);
  for(int i = 0; i < prfExt; ++i) {
    static TCHAR tmp[_MAX_PATH];
    _tcscpy_s(tmp, getPrphOnMemName());
    sendMessageSplash(WMC_LOADING_PRF_0 + i + WM_FIRST_EXT_ONMEM, tmp);
    gestPerif *prf = new memPerif(Main, WM_FIRST_EXT_ONMEM + i);
    Main->addPerif(prf);
    sendMessageSplash(WMC_LOADED_PRF_0 + i + WM_FIRST_EXT_ONMEM, tmp);
    }
}
//---------------------------------------------------------------------
uint getIdPerif(uint prf)
{
  return prf;
}
//---------------------------------------------------------
#define PSW_LEVEL_3 14
#define PSW_LEVEL_2 13
#define PSW_LEVEL_1 12
/*
int getLevelPswPerif(PWin* )
{
  return 0;
}
*/
