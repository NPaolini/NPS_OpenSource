//------ audio_dll.h --------------------------------------------------
#ifndef audio_dll_H_
#define audio_dll_H_
//---------------------------------------------------------------------
#include "precHeader.h"
#include <mmsystem.h>
//---------------------------------------------------------------------
struct audio_config
{
  bool useFile4Audio;
  uint startTrack;
  uint numOfTrack;
  uint secToPlay;
  uint Volume;
  TCHAR fileAudio[_MAX_PATH];
};
//---------------------------------------------------------------------
class pAudioPlayer
{
  public:
    pAudioPlayer();
    ~pAudioPlayer();

    bool open(const audio_config& cfg);
    bool play(const audio_config& cfg, HWND hWndNotify);
    bool openAndPlay(const audio_config& cfg, HWND hWndNotify);

    void stop();
    void close();
    void pause();
    void resume();

    bool nextAction(const audio_config& cfg, HWND hWndNotify, LONG lDevID);
    UINT getId() const { return wDeviceID; }

    bool isRunning() const { return running; }
    bool isClosed() const { return !wDeviceID; }

    void setVolume(int value, HWND hWndNotify);
    void setPos(DWORD value) { lastPos = value; }
  private:
    UINT wDeviceID;
    BYTE currTrack;
    bool running;
    DWORD lastPos;
};
//---------------------------------------------------------------------
#endif
