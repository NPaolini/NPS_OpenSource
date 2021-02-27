//------------- manage_audio.h ----------------------------------------
#ifndef manage_audio_H_
#define manage_audio_H_
//---------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "p_util.h"
#include "p_file.h"
#include "P_date.h"
#include <stdio.h>
#include <stdlib.h>
#include "audio_dll.h"
#include "PCrt_lck.h"
#include "headerMsg.h"
//------------------------------------------------------------------
class PEventWin;
//------------------------------------------------------------------
struct infoPlay
{
  audio_config cfg;
  pAudioPlayer* pAudio;
  FILETIME startTime;
  infoPlay() : pAudio(0) { ZeroMemory(&cfg, sizeof(cfg)); MK_I64(startTime) = 0; }
};
//------------------------------------------------------------------
class manageAudio
{
  public:
    manageAudio() : wEvent(0), hEventClose(CreateEvent(0, TRUE, 0, 0)) { }
    ~manageAudio() { reset(); CloseHandle(hEventClose); }


    void load(const PVect<LPCTSTR>& p);
    void reset();

    void start(uint ix);
    void toggle(uint ix);
    void stop(uint ix);
    void close(uint ix);
    void pause(uint ix);
    void resume(uint ix);
    void nextAction_Th(LONG lDevID);
    void setVolume(uint ix, int value);
    void setPos(uint ix, int value);
    void setSecToPlay(uint ix, uint value);

    const audio_config& getCfg(uint ix) { return setCfg[ix]->cfg; }
    void setFileAudio(LPCTSTR file, uint ix);
    bool isRunning(uint ix);
    bool isClosed(uint ix);

  private:
    PEventWin* wEvent;
    PVect<infoPlay*> setCfg;

    void checkAudio_Th(PWin* Win);

    HANDLE hEventClose;
    criticalSect csThread;

    void SetWinMsg(PEventWin* w) { wEvent = w; }
    PEventWin* GetWinMsg() { return wEvent; }
    bool startThread();
    void stopThread();
    friend unsigned FAR PASCAL MsgThread(void* pData);

    void setVolume(int value);
    void close_Th(int ix);
    void stop_Th(int ix);
    void play_Th(int ix);
    void pause_Th(int ix);
    void resume_Th(int ix);
    friend class PEventWin;
};
//------------------------------------------------------------------
#endif
