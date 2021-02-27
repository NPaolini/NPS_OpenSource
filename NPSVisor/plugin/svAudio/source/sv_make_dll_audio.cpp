//-------- sv_make_dll_audio.cpp --------------------------------------
//---------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "p_util.h"
#include "sv_make_dll.h"
#include "p_file.h"
#include "P_date.h"
#include <stdio.h>
#include <stdlib.h>
#include "audio_dll.h"
#include "PCrt_lck.h"
#include "headerMsg.h"
//------------------------------------------------------------------
static TCHAR Path[_MAX_PATH];
//------------------------------------------------------------------
BOOL initProcess(HINSTANCE hI)
{
  GetCurrentDirectory(SIZE_A(Path), Path);
  return TRUE;
}
//------------------------------------------------------------------
void endProcess() { }
//------------------------------------------------------------------
/*
  il file di testo che accompagna la dll è formato dai seguenti codici:
    10,prph,addr,type,nbit,offs -> se presente ed il valore è != zero -> ricarica il testo con le informazioni
    11,prph,addr,type,nbit,offs per resettare eventi sonori
    12,numero di variabili che seguono associate ad eventi sonori
    13,prph,addr,type,nbit,offs,type,info
    14,prph,addr,type,nbit,offs,type,info
    15,ecc.

    dove type -> 0 -> evento da CD, seguono durata_in_secondi,traccia_di_partenza,numero_di_tracce
         type -> 1 -> evento da file, seguono durata_in_secondi, path_del_file
*/
//------------------------------------------------------------------
struct infoData
{
  int Prph;
  DWORD Mask;
  SV_prfData Data;
  infoData() : Prph(0), Mask(0) { ZeroMemory(&Data, sizeof(Data)); }
};
//------------------------------------------------------------------
static infoData iData;
//------------------------------------------------------------------
static LPCTSTR makeInfoData(LPCTSTR p, infoData& target)
{
  if(!p)
    return 0;
  bool ok = false;
  do {
    target.Prph = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    target.Data.lAddr = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    target.Data.typeVar = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      p = _T("0");
    int nBit = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      p = _T("0");
    int offs = _ttoi(p);
    p = findNextParam(p, 1);
    if(SV_prfData::tBitData == target.Data.typeVar) {
      target.Data.U.dw = MAKELONG(nBit, offs);
      target.Mask = (DWORD)-1;
      }
    else {
      target.Mask = nBit << offs;
      if(!target.Mask)
        target.Mask = (DWORD)-1;
      }
    ok = true;
    } while(false);
  if(!ok)
    target.Prph = 0;
  return p;
}
//------------------------------------------------------------------
static void makeAddrReload(LPCTSTR p)
{
  makeInfoData(p, iData);
}
//------------------------------------------------------------------
static bool isChanged(infoData& idata)
{
  if(!idata.Prph)
    return false;
  SV_prfData t = idata.Data;
  npSV_Get(idata.Prph, &t);
  if(t.U.dw & idata.Mask) {
    t.U.dw &= ~idata.Mask;
    npSV_Set(idata.Prph, &t);
    return true;
    }
  return false;
}
//------------------------------------------------------------------
static bool needReload()
{
  return isChanged(iData);
}
//------------------------------------------------------------------
#define ID_TEXT_RELOAD    10
#define ID_TEXT_RESET     11
#define ID_TEXT_COUNT     12
//------------------------------------------------------------------
class manageAudio;
//------------------------------------------------------------------
#define CM_CLOSE_AUDIO (WM_FW_FIRST_FREE + 50)
#define CM_PLAY_AUDIO (CM_CLOSE_AUDIO + 1)
//------------------------------------------------------------------
class PEventWin : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PEventWin(manageAudio* owner);
    ~PEventWin() { destroy(); }
  protected:
    virtual LPCTSTR getClassName() const { return _T("Audio_Event_Class"); }
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CD_Notify(WPARAM flag, LONG lDevID);
    void closeAudio(int ix);
    void playAudio(int ix);

    manageAudio* Owner;
};
//------------------------------------------------------------------
struct infoPlay
{
  config cfg;
  infoData data;
  pAudioPlayer* pAudio;
  FILETIME startTime;
  infoPlay() : pAudio(0) { ZeroMemory(&cfg, sizeof(cfg)); MK_I64(startTime) = 0; }
};
//------------------------------------------------------------------
class manageAudio
{
  public:
    manageAudio();
    ~manageAudio();

    void load();
    void reset();

    int perform();
    void nextAction_Th(LONG lDevID);

  private:
    PEventWin* wEvent;
    PVect<infoPlay*> setCfg;

    void checkAudio(int ix);
    void checkReset();

    HANDLE hEventClose;
    criticalSect csThread;

    void SetWinMsg(PEventWin* w) { wEvent = w; }
    PEventWin* GetWinMsg() { return wEvent; }
    bool startThread();
    void stopThread();
    friend unsigned FAR PASCAL MsgThread(void* pData);

    void close_Th(int ix);
    void play_Th(int ix);
    friend class PEventWin;
    uint behavior;
    enum { alwaysRestart, toggleRun };
};
//------------------------------------------------------------------
infoPlay* makeInfoPlay(LPCTSTR p)
{
  infoPlay* inf = new infoPlay;
  p = makeInfoData(p, inf->data);
  while(p) {
    config& cfg = inf->cfg;
    uint use = _ttoi(p);
    if(use > 1)
      break;

    cfg.useFile4Audio = toBool(use);
    p = findNextParam(p);
    if(!p)
      break;
    cfg.secToPlay = _ttoi(p);

    p = findNextParam(p);
    if(!p)
      break;
    switch(use) {
      case 0:
        cfg.startTrack = _ttoi(p);
        p = findNextParam(p);
        if(p)
          cfg.numOfTrack = _ttoi(p);
        break;
      case 1:
        _tcscpy_s(cfg.fileAudio, p);
        break;
      }

    return inf;
    }
  delete inf;
  return 0;
}
//------------------------------------------------------------------
manageAudio::manageAudio() : wEvent(0), hEventClose(CreateEvent(0, TRUE, 0, 0)), behavior(0)
{ }
//------------------------------------------------------------------
manageAudio::~manageAudio()
{
  reset();
  CloseHandle(hEventClose);
}
//------------------------------------------------------------------
void manageAudio::load()
{
  if(setCfg.getElem())
    return;

  LPCTSTR p = npSV_GetLocalString(ID_TEXT_COUNT, getHinstance());
  if(p) {
    uint nElem = _ttoi(p);
    p = findNextParamTrim(p);
    if(p)
      behavior = _ttoi(p);
    for(uint i = 0; i < nElem; ++i) {
      p =  npSV_GetLocalString(ID_TEXT_COUNT + 1 + i, getHinstance());
      if(!p)
        continue;
      infoPlay* inf = makeInfoPlay(p);
      if(inf) {
        inf->pAudio = new pAudioPlayer;
        setCfg[setCfg.getElem()] = inf;
        }
      }
    }
  startThread();
}
//------------------------------------------------------------------
bool manageAudio::startThread()
{
  DWORD idThread;
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MsgThread, this, 0, &idThread);
  if(!hThread)
    return false;
  CloseHandle(hThread);
  for(;;) {
    bool started = false;
    {
      criticalLock crtLck(csThread);
      started = toBool(GetWinMsg());
    }
    if(started)
      break;
    Sleep(100);
    }

  // segnalazione di errore in init da parte del thread
  if(-1 == (LONG)GetWinMsg()) {
    SetWinMsg(0);
    return false;
    }
  return true;
}
//------------------------------------------------------------------
void manageAudio::stopThread()
{
  SetEvent(hEventClose);
  for(;;) {
    bool closed = false;
    {
      criticalLock crtLck(csThread);
      closed = !toBool(GetWinMsg());
    }
    if(closed)
      break;
    Sleep(100);
    }
}
//------------------------------------------------------------------
void manageAudio::reset()
{
  PWin* Win = GetWinMsg();
  if(Win)
    SendMessage(*Win, CM_CLOSE_AUDIO, -1, 0);
  stopThread();
  uint nElem = setCfg.getElem();
  for(uint i = 0; i < nElem; ++i)
    delete setCfg[i]->pAudio;
  setCfg.reset();
}
//------------------------------------------------------------------
void manageAudio::checkAudio(int ix)
{
  criticalLock crtLck(csThread);
  PWin* Win = GetWinMsg();
  if(!Win)
    return;
  infoPlay* ip = setCfg[ix];
  if(ip->pAudio->isRunning()) {
    FILETIME ft = getFileTimeCurr() - ip->startTime;
    if(MK_I64(ft) > ip->cfg.secToPlay * SECOND_TO_I64) {
      PostMessage(*Win, CM_CLOSE_AUDIO, ix, 0);
      return;
      }
    }
  if(isChanged(ip->data)) {
    if(toggleRun == behavior && ip->pAudio->isRunning()) {
      PostMessage(*Win, CM_CLOSE_AUDIO, ix, 0);
      }
    else {
      PostMessage(*Win, CM_PLAY_AUDIO, ix, 0);
      ip->startTime = getFileTimeCurr();
      }
    }
}
//------------------------------------------------------------------
void manageAudio::checkReset()
{
  LPCTSTR p = npSV_GetLocalString(ID_TEXT_RESET, getHinstance());
  if(!p)
    return;
  infoData idata;
  makeInfoData(p, idata);
  if(isChanged(idata)) {
    criticalLock crtLck(csThread);
    PWin* Win = GetWinMsg();
    if(!Win)
      return;
    PostMessage(*Win, CM_CLOSE_AUDIO, -1, 0);
    }
}
//------------------------------------------------------------------
int manageAudio::perform()
{
  uint nElem = setCfg.getElem();
  if(!nElem)
    return 0;
  do {
    criticalLock crtLck(csThread);
    PWin* Win = GetWinMsg();
    if(!Win)
      return 0;
    } while(false);
  for(uint i = 0; i < nElem; ++i)
    checkAudio(i);

  checkReset();
  return 1;
}
//------------------------------------------------------------------
void manageAudio::close_Th(int ix)
{
  if(-1 == ix) {
    uint nElem = setCfg.getElem();
    for(uint i = 0; i < nElem; ++i)
      setCfg[i]->pAudio->close();
    }
  else
    setCfg[ix]->pAudio->close();
}
//------------------------------------------------------------------
void manageAudio::play_Th(int ix)
{
  setCfg[ix]->pAudio->openAndPlay(setCfg[ix]->cfg, *GetWinMsg());
}
//------------------------------------------------------------------
void manageAudio::nextAction_Th(LONG lDevID)
{
  criticalLock crtLck(csThread);
  PWin* Win = GetWinMsg();
  if(!Win)
    return;
  uint nElem = setCfg.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(setCfg[i]->pAudio->getId() == lDevID) {
      setCfg[i]->pAudio->nextAction(setCfg[i]->cfg, *Win, lDevID);
      break;
      }
    }
}
//------------------------------------------------------------------
PEventWin::PEventWin(manageAudio* owner) :
  baseClass(0, _T("Audio_Event"), getHinstance()), Owner(owner)
{
  Attr.style = WS_POPUP;
}
//------------------------------------------------------------------
LRESULT PEventWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case MM_MCINOTIFY:
      CD_Notify(wParam, (LONG)lParam);
      break;
    case CM_CLOSE_AUDIO:
      closeAudio((int)wParam);
      break;
    case CM_PLAY_AUDIO:
      playAudio((int)wParam);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PEventWin::closeAudio(int ix)
{
  Owner->close_Th(ix);
}
//----------------------------------------------------------------------------
void PEventWin::playAudio(int ix)
{
  Owner->play_Th(ix);
}
//----------------------------------------------------------------------------
void PEventWin::CD_Notify(WPARAM flag, LONG lDevID)
{
  switch(flag) {
    case MCI_NOTIFY_ABORTED:
    case MCI_NOTIFY_FAILURE:
      break;
    case MCI_NOTIFY_SUPERSEDED:
      break;
    case MCI_NOTIFY_SUCCESSFUL:
      Owner->nextAction_Th(lDevID);
      break;
    }
}
//------------------------------------------------------------------
static manageAudio* Man_Audio;
//------------------------------------------------------------------
ExpImp_DLL
void npSV_System(DWORD msg)
{
  switch(msg) {
    case eINIT_DLL:
      Man_Audio = new manageAudio;
      break;
    case eEND_DLL:
      delete Man_Audio;
      Man_Audio = 0;
      break;
    }
}
//------------------------------------------------------------------
ExpImp_DLL
void npSV_Refresh()
{
  if(!Man_Audio)
    return;

  static int checked = 0;
  if(needReload()) {
    npSV_GetBodyRoutine(eSV_RELOAD_TEXT_SET, LPDWORD(getHinstance()), 0);
    checked = 0;
    }
  if(!checked) {
    checked = -1;
    LPCTSTR p = npSV_GetLocalString(ID_TEXT_RELOAD, getHinstance());
    makeAddrReload(p);
    Man_Audio->reset();
    Man_Audio->load();
    checked = 1;
    }
  if(-1 == checked)
    return;
  checked = Man_Audio->perform() ? 1 : -1;
}
//--------------------------------------------------------------------------------
unsigned FAR PASCAL MsgThread(void* pData)
{
  manageAudio* man_Audio = (manageAudio*)pData;
  PEventWin* W = new PEventWin(man_Audio);

  if(!W->create()) {
    delete W;
    criticalLock crtLck(man_Audio->csThread);
    man_Audio->SetWinMsg((PEventWin*)-1);
    return 0;
    }

  do {
    criticalLock crtLck(man_Audio->csThread);
    man_Audio->SetWinMsg(W);
    } while(false);

  HANDLE HandlesToWaitFor[] = {
    man_Audio->hEventClose,
    };

  ResetEvent(man_Audio->hEventClose);

  bool terminate = false;

  while(!terminate) {

    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) {
        terminate = true;
        break;
        }
      DispatchMessage(&msg);
      }
    if(!terminate) {
      DWORD result = MsgWaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, INFINITE, QS_ALLINPUT);
      if(result == (WAIT_OBJECT_0 + SIZE_A(HandlesToWaitFor)))
         continue;
      switch(result) {
        case WAIT_OBJECT_0:
        default:
          terminate = true;
          break;
        }
      }
    }
  do {
    criticalLock crtLck(man_Audio->csThread);
    delete W;
    man_Audio->SetWinMsg(0);
    } while(false);
  return 1;
}
//------------------------------------------------------------------
