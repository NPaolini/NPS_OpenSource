//------ audio_dll.cpp ------------------------------------------------
//---------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "audio_dll.h"
#include <Vfw.h>
#include <digitalv.h>
//---------------------------------------------------------------------
#define USE_POS
//---------------------------------------------------------------------
void showMCI_ERR(uint pos, DWORD err)
{
#ifdef _DEBUG
  TCHAR errMsg[_MAX_PATH];
  mciGetErrorString(err, errMsg, SIZE_A(errMsg));
  TCHAR t[_MAX_PATH * 2];
  wsprintf(t, _T("[%d][%d] %s"), pos, err, errMsg);
  MessageBox(0, t, _T("Error"), MB_OK | MB_ICONSTOP);
#endif
}
//---------------------------------------------------------------------
static DWORD setTimeFormat(UINT wDeviceID)
{
  MCI_WAVE_SET_PARMS mciSetParms = {0};
  mciSetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
  return mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT | MCI_WAIT, (DWORD)(LPVOID) &mciSetParms);
}
//---------------------------------------------------------------------
DWORD getCurrPos(UINT wDeviceID)
{
  DWORD dwReturn = setTimeFormat(wDeviceID);
  if (dwReturn)
    showMCI_ERR(9, dwReturn);

//  return 0;
  MCI_STATUS_PARMS parms = {0};
  parms.dwItem = MCI_STATUS_POSITION;

  dwReturn = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD) (LPVOID) &parms);
  if(dwReturn) {
    showMCI_ERR(1, dwReturn);
    return 0;
    }
  return parms.dwReturn;
}
//---------------------------------------------------------------------
DWORD closeAudio(UINT& wDeviceID)
{
  DWORD pos = getCurrPos(wDeviceID);
  mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
  wDeviceID = 0;
  return pos;
}
//---------------------------------------------------------------------
DWORD stopAudio(UINT wDeviceID)
{
  DWORD pos = getCurrPos(wDeviceID);
  DWORD dwReturn = mciSendCommand(wDeviceID, MCI_STOP, MCI_WAIT, NULL);
  if(dwReturn)
    showMCI_ERR(2, dwReturn);
  return pos;
}
//---------------------------------------------------------------------
DWORD pauseAudio(UINT wDeviceID)
{
  DWORD pos = getCurrPos(wDeviceID);
  DWORD dwReturn = mciSendCommand(wDeviceID, MCI_PAUSE, MCI_WAIT, NULL);
  if(dwReturn)
    showMCI_ERR(11, dwReturn);
  return pos;
}
//---------------------------------------------------------------------
void resumeAudio(UINT wDeviceID)
{
  DWORD dwReturn = mciSendCommand(wDeviceID, MCI_RESUME, MCI_WAIT, NULL);
  if(dwReturn)
    showMCI_ERR(10, dwReturn);
}
//---------------------------------------------------------------------
void seekWave(UINT wDeviceID, DWORD pos)
{
  DWORD dwReturn = setTimeFormat(wDeviceID);
  if (dwReturn)
    showMCI_ERR(3, dwReturn);

  MCI_SEEK_PARMS parms = {0};
  parms.dwTo = pos;
  dwReturn = mciSendCommand(wDeviceID, MCI_SEEK, MCI_TO | MCI_WAIT, (DWORD)(LPVOID) &parms);
  if(dwReturn)
    showMCI_ERR(4, dwReturn);
}
//---------------------------------------------------------------------
DWORD playWave(HWND hWndNotify, UINT wDeviceID, DWORD from)
{
  DWORD dwReturn = setTimeFormat(wDeviceID);
  if(dwReturn)
    showMCI_ERR(5, dwReturn);

  MCI_PLAY_PARMS mciPlayParms = {0};
  mciPlayParms.dwCallback = (DWORD) hWndNotify;
  mciPlayParms.dwFrom = from;
  return mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY | MCI_FROM, (DWORD)(LPVOID) &mciPlayParms);
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
    showMCI_ERR(6, dwReturn);
  return dwReturn;
}
//---------------------------------------------------------------------
DWORD openAndPlayWave(HWND hWndNotify, LPCTSTR filename, UINT* DevID, DWORD from)
{
  UINT wDeviceID = 0;
  DWORD dwReturn = openWave(&wDeviceID, filename);
  if(dwReturn)
    return dwReturn;

  dwReturn = playWave(hWndNotify, wDeviceID, from);
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
  return mciSendCommand(wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO | MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms);
}
//---------------------------------------------------------------------
DWORD openCDAudio(UINT* DevID)
{
  MCI_OPEN_PARMS mciOpenParms;
  ZeroMemory(&mciOpenParms, sizeof(mciOpenParms));
    // Open the CD audio device by specifying the device name.
  mciOpenParms.lpstrDeviceType = _T("cdaudio");
  DWORD dwReturn = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE,  (DWORD)(LPVOID) &mciOpenParms);
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
void set_volume(HWND hWndNotify, int value, UINT wDeviceID)
{
  MCI_DGV_SETAUDIO_PARMS parms = {0};
  parms.dwValue = value; // 0 <-> 1000
  parms.dwItem = MCI_DGV_SETAUDIO_VOLUME;
  DWORD dwReturn = mciSendCommand(wDeviceID, MCI_SETAUDIO, MCI_DGV_SETAUDIO_ITEM | MCI_DGV_SETAUDIO_VALUE | MCI_WAIT, (DWORD)(LPVOID) &parms);
  if (dwReturn)
    showMCI_ERR(7, dwReturn);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
pAudioPlayer::pAudioPlayer() : wDeviceID(0), currTrack(1), running(false), lastPos(0) {}
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
bool pAudioPlayer::open(const audio_config& cfg)
{
  if(wDeviceID)
    return true;
  DWORD result;
  if(cfg.useFile4Audio && *cfg.fileAudio) {
    result = openWave(&wDeviceID, cfg.fileAudio);
    if(lastPos)
      seekWave(wDeviceID, lastPos);
    }
  else {
    result = openCDAudio(&wDeviceID);
    currTrack = cfg.startTrack;
    }
  return !result;
}
//---------------------------------------------------------------------
bool pAudioPlayer::play(const audio_config& cfg, HWND hWndNotify)
{
  if(!wDeviceID && !open(cfg))
    return false;
  DWORD result;
  if(cfg.useFile4Audio && *cfg.fileAudio) {
    result = playWave(hWndNotify, wDeviceID, lastPos);
    }
  else
    result = playCDTrack(hWndNotify, currTrack, wDeviceID);
  if(result) {
    showMCI_ERR(0, result);
    }
  running = !result;
  lastPos = 0;
  return running;
}
//---------------------------------------------------------------------
bool pAudioPlayer::openAndPlay(const audio_config& cfg, HWND hWndNotify)
{
  if(open(cfg))
    return play(cfg, hWndNotify);
  return false;
}
//---------------------------------------------------------------------
static void msgPos(uint pos, LPCTSTR from)
{
  TCHAR t[256];
  wsprintf(t, _T("[%s] -> %d"), from, pos);
  MessageBox(0, t, _T("Pos"), MB_OK);
}
//---------------------------------------------------------------------
void pAudioPlayer::stop()
{
  if(running) {
    lastPos = stopAudio(wDeviceID);
//    if(lastPos > 0)
//      msgPos(lastPos, _T("Stop"));
    }
  running = false;
}
//---------------------------------------------------------------------
void pAudioPlayer::close()
{
  if(wDeviceID) {
    stop();
    DWORD pos = closeAudio(wDeviceID);
//    if(pos > 0)
//      msgPos(pos, _T("Close"));
    if(pos > lastPos)
      lastPos = pos;
    }
}
//---------------------------------------------------------------------
void pAudioPlayer::pause()
{
  if(running)
    lastPos = pauseAudio(wDeviceID);
  running = false;
}
//---------------------------------------------------------------------
void pAudioPlayer::resume()
{
  resumeAudio(wDeviceID);
}
//---------------------------------------------------------------------
void pAudioPlayer::setVolume(int value, HWND hWndNotify)
{
  if(!wDeviceID)
    return;
  set_volume(hWndNotify, value, wDeviceID);
}
//---------------------------------------------------------------------
bool pAudioPlayer::nextAction(const audio_config& cfg, HWND hWndNotify, LONG lDevID)
{
  if(lDevID != wDeviceID || !running)
    return false;

  if(!cfg.useFile4Audio) {
    if(++currTrack >= (uint)cfg.startTrack + cfg.numOfTrack)
      currTrack = cfg.startTrack;
    }
  if((uint)-1 == cfg.secToPlay) {
    stop();
    lastPos = 0;
    return false;
    }
  lastPos = 0;
  return play(cfg, hWndNotify);
}
