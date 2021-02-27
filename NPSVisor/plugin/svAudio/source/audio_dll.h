//------ audio_dll.h --------------------------------------------------
#ifndef audio_dll_H_
#define audio_dll_H_
//---------------------------------------------------------------------
#include "precHeader.h"
#include <mmsystem.h>
//---------------------------------------------------------------------
struct config
{
  bool useFile4Audio;
  uint startTrack;
  uint numOfTrack;
  uint secToPlay;
  TCHAR fileAudio[_MAX_PATH];
};
//---------------------------------------------------------------------
class pAudioPlayer
{
  public:
    pAudioPlayer();
    ~pAudioPlayer();

    bool open(const config& cfg);
    bool play(const config& cfg, HWND hWndNotify);
    bool openAndPlay(const config& cfg, HWND hWndNotify);

    void stop();
    void close();

    bool nextAction(const config& cfg, HWND hWndNotify, LONG lDevID);
    UINT getId() const { return wDeviceID; }

    bool isRunning() const { return running; }
  private:
    UINT wDeviceID;
    BYTE currTrack;
    bool running;
};
//---------------------------------------------------------------------
#endif
