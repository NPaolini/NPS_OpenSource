//------ audio_dll.cpp ------------------------------------------------
//---------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "audio_dll.h"
#include <Vfw.h>
//---------------------------------------------------------------------
void showMCI_ERR(DWORD err)
{
#if 0
//#ifdef _DEBUG
  TCHAR errMsg[_MAX_PATH];
  mciGetErrorString(err, errMsg, SIZE_A(errMsg));
  MessageBox(0, errMsg, _T("Error"), MB_OK | MB_ICONSTOP);
#endif
}
//---------------------------------------------------------------------
void closeAudio(UINT& wDeviceID)
{
  mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
  wDeviceID = 0;
}
//---------------------------------------------------------------------
void stopAudio(UINT wDeviceID)
{
  mciSendCommand(wDeviceID, MCI_STOP, MCI_WAIT, NULL);
}
//---------------------------------------------------------------------
DWORD playWave(HWND hWndNotify, UINT wDeviceID)
{
  MCI_PLAY_PARMS mciPlayParms;
  mciPlayParms.dwFrom = 0;
  mciPlayParms.dwCallback = (DWORD) hWndNotify;
  DWORD flags = MCI_FROM | (hWndNotify ? MCI_NOTIFY : 0);
  return mciSendCommand(wDeviceID, MCI_PLAY, flags, (DWORD)(LPVOID) &mciPlayParms);
}
//---------------------------------------------------------------------
DWORD openWave(UINT* DevID, LPCTSTR filename)
{
  MCI_OPEN_PARMS mciOpenParms;
  ZeroMemory(&mciOpenParms, sizeof(mciOpenParms));
  mciOpenParms.lpstrElementName = filename;
  DWORD dwReturn = mciSendCommand(NULL, MCI_OPEN, MCI_WAIT | MCI_OPEN_ELEMENT, (DWORD) (LPVOID) &mciOpenParms);

  if(!dwReturn)
    *DevID = mciOpenParms.wDeviceID;
  else
    showMCI_ERR(dwReturn);
  return dwReturn;
}
//---------------------------------------------------------------------
DWORD openAndPlayWave(HWND hWndNotify, LPCTSTR filename, UINT* DevID)
{
  UINT wDeviceID = 0;
  DWORD dwReturn = openWave(&wDeviceID, filename);
  if(dwReturn)
    return dwReturn;

  dwReturn = playWave(hWndNotify, wDeviceID);
  if(dwReturn) {
    mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
    return dwReturn;
    }
  *DevID = wDeviceID;
  return (0L);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
DWORD playCDTrack(HWND hWndNotify, BYTE bTrack, UINT wDeviceID)
{
  MCI_SET_PARMS mciSetParms;
  // Set the time format to track/minute/second/frame (TMSF).
  mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;
  DWORD dwReturn = mciSendCommand(wDeviceID, MCI_SET,
        MCI_SET_TIME_FORMAT, (DWORD)(LPVOID) &mciSetParms);
  if(dwReturn)
    return (dwReturn);

  MCI_PLAY_PARMS mciPlayParms;
  mciPlayParms.dwFrom = MCI_MAKE_TMSF(bTrack, 0, 0, 0);
  mciPlayParms.dwTo = MCI_MAKE_TMSF(bTrack + 1, 0, 0, 0);
  mciPlayParms.dwCallback = (DWORD) hWndNotify;
  return mciSendCommand(wDeviceID, MCI_PLAY,
        MCI_FROM | MCI_TO | MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms);
}
//---------------------------------------------------------------------
DWORD openCDAudio(UINT* DevID)
{
  MCI_OPEN_PARMS mciOpenParms;
  ZeroMemory(&mciOpenParms, sizeof(mciOpenParms));
    // Open the CD audio device by specifying the device name.
  mciOpenParms.lpstrDeviceType = _T("cdaudio");
  DWORD dwReturn = mciSendCommand(NULL, MCI_OPEN,
        MCI_OPEN_TYPE
        //| MCI_NOTIFY
        , (DWORD)(LPVOID) &mciOpenParms);
  if(!dwReturn)
    *DevID = mciOpenParms.wDeviceID;

  return dwReturn;
}
//---------------------------------------------------------------------
DWORD openAndPlayCDTrack(HWND hWndNotify, BYTE bTrack, UINT* DevID)
{
  UINT wDeviceID = 0;
  DWORD dwReturn = openCDAudio(&wDeviceID);
  if(dwReturn)
    return dwReturn;

  dwReturn = playCDTrack(hWndNotify, bTrack, wDeviceID);
  if(dwReturn) {
    mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
    return dwReturn;
    }
  *DevID = wDeviceID;
  return (0L);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
pAudioPlayer::pAudioPlayer() : wDeviceID(0), currTrack(1), running(false) {}
//---------------------------------------------------------------------
pAudioPlayer::~pAudioPlayer()
{
  if(wDeviceID) {
    if(running)
      stopAudio(wDeviceID);
    closeAudio(wDeviceID);
    }
}
//---------------------------------------------------------------------
bool pAudioPlayer::open(const config& cfg)
{
  if(wDeviceID)
    return true;
  DWORD result;
  if(cfg.useFile4Audio && *cfg.fileAudio)
    result = openWave(&wDeviceID, cfg.fileAudio);
  else {
    result = openCDAudio(&wDeviceID);
    currTrack = cfg.startTrack;
    }
  return !result;
}
//---------------------------------------------------------------------
bool pAudioPlayer::play(const config& cfg, HWND hWndNotify)
{
  if(!wDeviceID && !open(cfg))
    return false;
  DWORD result;
  if(cfg.useFile4Audio && *cfg.fileAudio)
    result = playWave(hWndNotify, wDeviceID);
  else
    result = playCDTrack(hWndNotify, currTrack, wDeviceID);
  if(result) {
    showMCI_ERR(result);
    }
  running = !result;
  return running;
}
//---------------------------------------------------------------------
bool pAudioPlayer::openAndPlay(const config& cfg, HWND hWndNotify)
{
  if(open(cfg))
    return play(cfg, hWndNotify);
  return false;
}
//---------------------------------------------------------------------
void pAudioPlayer::stop()
{
  if(running)
    stopAudio(wDeviceID);
  running = false;
}
//---------------------------------------------------------------------
void pAudioPlayer::close()
{
  if(wDeviceID) {
    stop();
    closeAudio(wDeviceID);
    }
}
//---------------------------------------------------------------------
bool pAudioPlayer::nextAction(const config& cfg, HWND hWndNotify, LONG lDevID)
{
  if(lDevID != wDeviceID || !running)
    return false;

  if(!cfg.useFile4Audio) {
    if(++currTrack >= (uint)cfg.startTrack + cfg.numOfTrack)
      currTrack = cfg.startTrack;
    }
  return play(cfg, hWndNotify);
}
