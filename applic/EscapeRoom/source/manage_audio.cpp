//------------- manage_audio.cpp --------------------------------------
//---------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "manage_audio.h"
#include "fullView.h"
//------------------------------------------------------------------
#define CM_CLOSE_AUDIO (WM_FW_FIRST_FREE + 50)
#define CM_PLAY_AUDIO (CM_CLOSE_AUDIO + 1)
#define CM_STOP_AUDIO (CM_PLAY_AUDIO + 1)
#define CM_PAUSE_AUDIO (CM_STOP_AUDIO + 1)
#define CM_RESUME_AUDIO (CM_PAUSE_AUDIO + 1)
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
    void stopAudio(int ix);
    void pauseAudio(int ix);
    void resumeAudio(int ix);

    manageAudio* Owner;
};
//------------------------------------------------------------------
static void copyParam(LPTSTR target, LPCTSTR source, size_t dim)
{
  GetModuleDirName(dim, target);
#ifdef _DEBUG
  dirName(target);
#endif
  target += _tcslen(target);
  *target++ = _T('\\');

  while(*source && *source != _T(','))
    *target++ = *source++;
  *target++ =  _T('\\');
  ++source;

  while(*source && *source != _T(','))
    *target++ = *source++;
  *target = 0;
}
//------------------------------------------------------------------
void manageAudio::load(const PVect<LPCTSTR>& vals)
{
  if(setCfg.getElem())
    return;
  uint nElem = vals.getElem();;

  for(uint i = 0; i < nElem; ++i) {
    LPCTSTR p = vals[i];
    if(!p)
      break;
    infoPlay* inf = new infoPlay;
    bool success = false;
    while(p) {
      audio_config& cfg = inf->cfg;
      uint use = _ttoi(p);
      if(use > 1)
        break;

      cfg.useFile4Audio = toBool(use);
      p = findNextParamTrim(p);
      if(!p)
        break;
      cfg.secToPlay = _ttoi(p);

      p = findNextParamTrim(p);
      if(!p)
        break;
      cfg.Volume = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      switch(use) {
        case 0:
          cfg.startTrack = _ttoi(p);
          p = findNextParamTrim(p);
          if(p)
            cfg.numOfTrack = _ttoi(p);
          break;
        case 1:
          copyParam(cfg.fileAudio, p, SIZE_A(cfg.fileAudio));
          break;
        }
      success = true;
      break;
      }
    if(!success)
      delete inf;
    else {
      inf->pAudio = new pAudioPlayer;
      setCfg[setCfg.getElem()] = inf;
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
void manageAudio::checkAudio_Th(PWin* Win)
{
  criticalLock crtLck(csThread);
  uint nElem = setCfg.getElem();
  for(uint ix = 0; ix < nElem; ++ix) {
    infoPlay* ip = setCfg[ix];
    if(ip->pAudio->isRunning() && ip->cfg.secToPlay && (uint)-1 != ip->cfg.secToPlay) {
      FILETIME ft = getFileTimeCurr() - ip->startTime;
      if(MK_I64(ft) > ip->cfg.secToPlay * SECOND_TO_I64) {
        PostMessage(*Win, CM_CLOSE_AUDIO, ix, 0);
        return;
        }
      }
    }
}
//------------------------------------------------------------------
void manageAudio::start(uint ix)
{
  criticalLock crtLck(csThread);
  PWin* Win = GetWinMsg();
  if(Win) {
    if(ix)
      setCfg[ix]->pAudio->setPos(0);
    PostMessage(*Win, CM_PLAY_AUDIO, ix, 0);
    }
}
//------------------------------------------------------------------
void manageAudio::toggle(uint ix)
{
  criticalLock crtLck(csThread);
  PWin* Win = GetWinMsg();
  if(Win) {
    infoPlay* ip = setCfg[ix];
    if(ip->pAudio->isRunning())
      PostMessage(*Win, CM_CLOSE_AUDIO, ix, 0);
    else
      PostMessage(*Win, CM_PLAY_AUDIO, ix, 0);
    }
}
//------------------------------------------------------------------
void manageAudio::stop(uint ix)
{
  criticalLock crtLck(csThread);
  PWin* Win = GetWinMsg();
  if(Win)
    PostMessage(*Win, CM_STOP_AUDIO, ix, 0);
}
//------------------------------------------------------------------
void manageAudio::pause(uint ix)
{
  criticalLock crtLck(csThread);
  PWin* Win = GetWinMsg();
  if(Win)
    PostMessage(*Win, CM_PAUSE_AUDIO, ix, 0);
}
//------------------------------------------------------------------
void manageAudio::resume(uint ix)
{
  criticalLock crtLck(csThread);
  PWin* Win = GetWinMsg();
  if(Win)
    PostMessage(*Win, CM_RESUME_AUDIO, ix, 0);
}
//------------------------------------------------------------------
void manageAudio::close(uint ix)
{
  criticalLock crtLck(csThread);
  PWin* Win = GetWinMsg();
  if(Win)
    PostMessage(*Win, CM_CLOSE_AUDIO, ix, 0);
}
//------------------------------------------------------------------
void manageAudio::setVolume(uint ix, int value)
{
  bool running = setCfg[ix]->pAudio->isRunning();
  stop(ix);
  setCfg[ix]->cfg.Volume = value;
  if(running)
    start(ix);
}
//------------------------------------------------------------------
void manageAudio::setPos(uint ix, int value)
{
  setCfg[ix]->pAudio->setPos(value);
}
//------------------------------------------------------------------
void manageAudio::setVolume(int value)
{
  setCfg[0]->pAudio->setVolume(value, *GetWinMsg());
}
//------------------------------------------------------------------
void manageAudio::setFileAudio(LPCTSTR file, uint ix)
{
  _tcscpy_s(setCfg[ix]->cfg.fileAudio, file);
  makeTrueFullPath(setCfg[ix]->cfg.fileAudio, SIZE_A(setCfg[ix]->cfg.fileAudio));
}
//------------------------------------------------------------------
void manageAudio::setSecToPlay(uint ix, uint value)
{
  setCfg[ix]->cfg.secToPlay = value;
}
//------------------------------------------------------------------
bool manageAudio::isRunning(uint ix)
{
  return setCfg[ix]->pAudio->isRunning();
}
//------------------------------------------------------------------
bool manageAudio::isClosed(uint ix)
{
  return setCfg[ix]->pAudio->isClosed();
}
//------------------------------------------------------------------
void manageAudio::stop_Th(int ix)
{
  if(-1 == ix) {
    uint nElem = setCfg.getElem();
    for(uint i = 0; i < nElem; ++i)
      setCfg[i]->pAudio->stop();
    }
  else
    setCfg[ix]->pAudio->stop();
}
//------------------------------------------------------------------
void manageAudio::pause_Th(int ix)
{
  if(-1 == ix) {
    uint nElem = setCfg.getElem();
    for(uint i = 0; i < nElem; ++i)
      setCfg[i]->pAudio->pause();
    }
  else
    setCfg[ix]->pAudio->pause();
}
//------------------------------------------------------------------
void manageAudio::resume_Th(int ix)
{
  if(-1 == ix) {
    uint nElem = setCfg.getElem();
    for(uint i = 0; i < nElem; ++i)
      setCfg[i]->pAudio->resume();
    }
  else
    setCfg[ix]->pAudio->resume();
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
  const audio_config& cfg = setCfg[ix]->cfg;
  if(setCfg[ix]->pAudio->open(cfg)) {
    setCfg[ix]->pAudio->setVolume(cfg.Volume, *GetWinMsg());
    setCfg[ix]->pAudio->play(cfg, *GetWinMsg());
    }
  setCfg[ix]->startTime = getFileTimeCurr();
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
  baseClass(0, _T("Audio_Event"), getHInstance()), Owner(owner)
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
    case CM_STOP_AUDIO:
      stopAudio((int)wParam);
      break;
    case CM_PAUSE_AUDIO:
      pauseAudio((int)wParam);
      break;
    case CM_RESUME_AUDIO:
      resumeAudio((int)wParam);
      break;
  }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PEventWin::stopAudio(int ix)
{
  Owner->stop_Th(ix);
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
void PEventWin::pauseAudio(int ix)
{
  Owner->pause_Th(ix);
}
//----------------------------------------------------------------------------
void PEventWin::resumeAudio(int ix)
{
  Owner->resume_Th(ix);
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
    DWORD timeout = 300;
    if(!terminate) {
      DWORD result = MsgWaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, timeout, QS_ALLINPUT);
      if(result == (WAIT_OBJECT_0 + SIZE_A(HandlesToWaitFor)))
         continue;
      switch(result) {
        case WAIT_TIMEOUT:
          man_Audio->checkAudio_Th(W);
          break;
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
