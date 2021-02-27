//-------------------- pAviCap.cpp --------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pAviCap.h"
#include "p_File.h"
#include "digitalv.h"
#include "p_util.h"
#include "p_date.h"
#include <mciavi.h>
#include <mmsystem.h>
//-----------------------------------------------------------
void aviCapInfo::init(LPCTSTR filename)
{
  if(filename)
    if(load(filename)) {
      Connected = false;
//      frameRate = 1000 / 10;
      return;
      }
  ZeroMemory(this, sizeof(*this));
  frameRate = 1000 / 10;
}
//-----------------------------------------------------------
bool aviCapInfo::load(LPCTSTR filename)
{
  P_File pf(filename, P_READ_ONLY);
  if(pf.P_open())
    return sizeof(*this) == pf.P_read(this, sizeof(*this));
  return false;
}
//-----------------------------------------------------------
bool aviCapInfo::save(LPCTSTR filename)
{
  P_File pf(filename, P_CREAT);
  if(pf.P_open())
    return sizeof(*this) == pf.P_write(this, sizeof(*this));
  return false;
}
//-----------------------------------------------------------
#define MAX_DEVICE 10
//#define GET_DEV_NAME capDriverGetName
//#define GET_DEV_NAME capGetMCIDeviceName
#define GET_DEV_NAME(a, b, c) capGetDriverDescription(a, b, c, 0, 0)
//  WORD wDriverIndex,
//  LPSTR lpszName,
//  INT cbName,
//  LPSTR lpszVer,
//  INT cbVer );

//-----------------------------------------------------------
void aviCapInfo::reload(HWND hwnd)
{
  capDriverGetCaps(hwnd, &caps, sizeof(caps));
  capCaptureGetSetup(hwnd, &params, sizeof(params));
  TCHAR name[SIZE_A_c(deviceName)];
  if(capGetDriverDescription(caps.wDeviceIndex, name, SIZE_A_c(name), 0, 0))
    _tcscpy_s(deviceName, SIZE_A(deviceName), name);
}
//-----------------------------------------------------------
bool aviCapInfo::connect(HWND hwnd)
{
  if(Connected)
    if(!closeConn(hwnd))
      return false;
  int device = caps.wDeviceIndex;
  Connected = toBool(capDriverConnect(hwnd, device));
  if(!Connected) {
    for(int i = 0; i < MAX_DEVICE; ++i) {
      if(device == i)
        continue;
      if(capDriverConnect(hwnd, i)) {
        TCHAR name[SIZE_A_c(deviceName)];
        if(capGetDriverDescription(i, name, SIZE_A_c(name), 0, 0)) {
          if(!_tcsicmp(deviceName, name)) {
            Connected = true;
            break;
            }
          }
        capDriverDisconnect(hwnd);
        }
      }
    }
  if(!Connected)
    return false;

  reload(hwnd);

  capPreview(hwnd, FALSE);
  capPreviewRate(hwnd, frameRate);
  capPreviewScale(hwnd, TRUE);
  return true;
}
//-----------------------------------------------------------
bool aviCapInfo::closeConn(HWND hwnd)
{
  if(!Connected)
    return false;
  bool result = stopCaptureFile(hwnd);
  result &= stopPreview(hwnd);
  result &= toBool(capDriverDisconnect(hwnd));
  Connected = false;
  return result;
}
//-----------------------------------------------------------
DWORD_PTR aviCapInfo::getCurrStat(HWND hwnd)
{
  if(!Connected)
    return No_action;
  CAPSTATUS cs;
  capGetStatus(hwnd, &cs, sizeof(cs));
  if(cs.fOverlayWindow) {
    if(cs.fCapturingNow)
      return onCaptureOverlay;
    return onOverlay;
    }

  if(cs.fLiveWindow) {
    if(cs.fCapturingNow)
      return onCapturePreview;
    return onPreview;
    }
  if(cs.fCapturingNow)
    return onCapture;

  return No_action;
}
//-----------------------------------------------------------
DWORD_PTR aviCapInfo::preDialog(HWND hwnd)
{
  if(!Connected)
    return 0;
  DWORD_PTR stat = getCurrStat(hwnd);
  stopCaptureFile(hwnd);
  stopPreview(hwnd);
  return stat;
}
//-----------------------------------------------------------
void aviCapInfo::postDialog(HWND hwnd, DWORD_PTR stat)
{
  if(!Connected)
    return;
  if(stat & onView)
    startPreview(hwnd);
  if(stat & onCapture) {
    TCHAR filename[_MAX_PATH];
    if(capFileGetCaptureFile(hwnd, filename, SIZE_A(filename)))
      startCaptureFile(hwnd, filename);
    }
}
//-----------------------------------------------------------
bool aviCapInfo::VideoFormatDialog(HWND hwnd)
{
  if(!Connected)
    return false;

  if(!caps.fHasDlgVideoFormat)
    return false;

  bool result = toBool(capDlgVideoFormat(hwnd));
  if(result)
    reload(hwnd);

  return result;
}
//-----------------------------------------------------------
bool aviCapInfo::VideoSourceDialog(HWND hwnd)
{
  if(!Connected)
    return false;
  if(!caps.fHasDlgVideoSource)
    return false;

  bool result = toBool(capDlgVideoSource(hwnd));
  if(result)
    reload(hwnd);
  return result;
}
//-----------------------------------------------------------
bool aviCapInfo::VideoDisplayDialog(HWND hwnd)
{
  if(!Connected)
    return false;
  if(!caps.fHasDlgVideoDisplay)
    return false;

  bool result = toBool(capDlgVideoDisplay(hwnd));
  if(result)
    reload(hwnd);
  return result;
}
//-----------------------------------------------------------
bool aviCapInfo::startPreview(HWND hwnd)
{
  if(!Connected)
    return false;
  CAPSTATUS cs;
  capGetStatus(hwnd, &cs, sizeof(cs));
  if(cs.fOverlayWindow || cs.fLiveWindow)
    return true;
/*

capVideoStreamCallback
The capVideoStreamCallback function is the callback function used with streaming capture
to optionally process a frame of captured video.
It is registered by the capSetCallbackOnFrame macro.

The capVideoStreamCallback callback function is a placeholder for the application-supplied function name.

LRESULT CALLBACK capVideoStreamCallback(
  HWND hWnd,
  LPVIDEOHDR lpVHdr
);
Parameters
hWnd
Handle to the capture window associated with the callback function.
lpVHdr
Pointer to a VIDEOHDR structure containing information about the captured frame.
Remarks
The capture window calls a video stream callback function when a video buffer is
marked done by the capture driver.
When capturing to disk, this will precede the disk write operation.

*/
//  params.fYield = FALSE;
//  params.fYield = TRUE;
//  if(!capCaptureSetSetup(hwnd, &params, sizeof(params)))
//    return false;
  POINT pt = { 0, 0 };
  capSetScrollPos(hwnd, &pt);
  if(caps.fHasOverlay)
    return toBool(capOverlay(hwnd, TRUE));
  return toBool(capPreview(hwnd, TRUE));
}
//-----------------------------------------------------------
bool aviCapInfo::stopPreview(HWND hwnd)
{
  if(!Connected)
    return false;
  CAPSTATUS cs;
//  ZeroMemory(&cs, sizeof(cs));
  capGetStatus(hwnd, &cs, sizeof(cs));
  if(cs.fOverlayWindow)
    return toBool(capOverlay(hwnd, FALSE));
  if(cs.fLiveWindow)
    return toBool(capPreview(hwnd, FALSE));
  return true;
}
//-----------------------------------------------------------
bool aviCapInfo::setCompressionType(HWND hwnd)
{
  if(!Connected)
    return false;
  bool result = toBool(capDlgVideoCompression(hwnd));
  if(result)
    reload(hwnd);
  return result;
}
//-----------------------------------------------------------
//#define SHOW_DROPPED
#ifdef SHOW_DROPPED
  #define IS_DROPPED_FRAME(x) 0
#else
  #define IS_DROPPED_FRAME(x) (513 == x)
#endif
//-----------------------------------------------------------
LRESULT CALLBACK CaptureErrorCallback(HWND hWnd, int nID, LPCSTR lpsz)
{
  TCHAR buf[256];

  if (!nID || IS_DROPPED_FRAME(nID)) return 0;

  static bool inExec;
  if(inExec)
    return 0;
  inExec = true;
  static FILETIME lastFt;
  static int oldID = -1;
  bool needShow = true;
  FILETIME ft = getFileTimeCurr();
  if(oldID == nID) {
    if(cMK_I64(ft) - cMK_I64(lastFt) < (5 * SECOND_TO_I64))
      needShow = false;
    }
  else
    oldID = nID;
  lastFt = ft;
  if(needShow) {
    wsprintf(buf, _T("Error %d: %s"), nID, lpsz);
    MessageBox(GetParent(hWnd), buf, _T("sVisor capture error"), MB_OK);
    }
  inExec = false;
//  _RPT1(0,"%s\n",buf);

  return 0;
}
//-----------------------------------------------------------
#define REG_HOT_KEY_ID 11111
//-----------------------------------------------------------
bool aviCapInfo::startCaptureFile(HWND hwnd, LPCTSTR filename)
{
  if(!Connected)
    return false;
  CAPSTATUS cs;
  capGetStatus(hwnd, &cs, sizeof(cs));
  if(cs.fCapturingNow)
    return true;
  bool success = false;
  do {
    if(!capFileSetCaptureFile(hwnd, filename))
      break;
//    if(!RegisterHotKey(hwnd, REG_HOT_KEY_ID, 0, VK_ESCAPE))
//      break;
    params.fLimitEnabled = FALSE;
    params.fYield = TRUE;
//    params.fMCIControl = TRUE;
//    params.dwRequestMicroSecPerFrame = 40000;
//    params.fCaptureAudio = TRUE;
//    cp.fStepCaptureAt2x = TRUE;
    if(!capCaptureSetSetup(hwnd, &params, sizeof(params)))
      break;
    success = toBool(capCaptureSequence(hwnd));
    } while(false);
//  if(!success)
//    UnregisterHotKey(hwnd, REG_HOT_KEY_ID);
  return success;
}
//-----------------------------------------------------------
bool aviCapInfo::stopCaptureFile(HWND hwnd)
{
  if(!Connected || !IsWindow(hwnd))
    return false;
  CAPSTATUS cs;
  capGetStatus(hwnd, &cs, sizeof(cs));
  if(!cs.fCapturingNow)
    return true;
//  return capCaptureAbort(hwnd);
//  UnregisterHotKey(hwnd, REG_HOT_KEY_ID);
  bool success = toBool(capCaptureStop(hwnd));
  return success;
}
//-----------------------------------------------------------
bool aviCapInfo::setRatePreview(HWND hwnd, DWORD_PTR msec)
{
  if(!Connected)
    return false;
  frameRate = msec;
  return toBool(capPreviewRate(hwnd, frameRate));
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
PAviCap::~PAviCap ()
{
  destroy();
}
//-----------------------------------------------------------
bool PAviCap::create()
{
  if(!baseClass::create())
    return false;

  capSetCallbackOnError(*this, (LPVOID)CaptureErrorCallback);

//  ACI.connect(*this);
  return true;
}
//----------------------------------------------------------------------------
HWND PAviCap::performCreate()
{
  TCHAR buff[100];
  wsprintf(buff, _T("PAviCap Capture Window (%d)"), Attr.id);
  return capCreateCaptureWindow(buff, WS_CHILD | WS_VISIBLE | WS_BORDER, Attr.x, Attr.y, Attr.w, Attr.h, (HWND)*getParent(), (int) 0);
}
//----------------------------------------------------------------------------
LRESULT PAviCap::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      evDestroy(hwnd);
      break;
/*
    case WM_SIZE:
      evSize();
      break;
*/
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
/*
void PAviCap::evSize()
{
}
*/
//-----------------------------------------------------------
void PAviCap::evDestroy(HWND hwnd)
{
  ACI.closeConn(hwnd);
  capSetCallbackOnError(hwnd, 0);
}
//-----------------------------------------------------------
/*
void PAviCap::Show(const PRect& r)
{

}
*/
//-----------------------------------------------------------
bool PAviCap::startCaptureFile(LPCTSTR filename)
{
  if(inExec)
    return false;
  inExec = true;
  return ACI.startCaptureFile(*this, filename);
}
//-----------------------------------------------------------
bool PAviCap::stopCaptureFile()
{
  inExec = false;
  return ACI.stopCaptureFile(*this);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
PAviPlay::~PAviPlay()
{
  close();
  destroy();
}
//-----------------------------------------------------------
void PAviPlay::getWindowClass(WNDCLASS& wcl)
{
  baseClass::getWindowClass(wcl);
  wcl.hbrBackground  = (HBRUSH) GetStockObject(BLACK_BRUSH);
}
//-----------------------------------------------------------
bool PAviPlay::create()
{
  return baseClass::create();
}
//-----------------------------------------------------------
bool PAviPlay::setStretch(bool set, bool refresh)
{
  bool old = canStretch;
  canStretch = set;
  if(refresh)
    evSize();
  return old;
}
//-----------------------------------------------------------
DWORD_PTR PAviPlay::getTotalFrame()
{
  MCI_DGV_SET_PARMS mciSet;
  MCI_DGV_STATUS_PARMS mciStatus;

  // Put in frame mode.
  mciSet.dwTimeFormat = MCI_FORMAT_FRAMES;
  mciSendCommand(DeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSet);

  mciStatus.dwItem = MCI_STATUS_LENGTH;
  mciSendCommand(DeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);
  return mciStatus.dwReturn;
}
//-----------------------------------------------------------
DWORD_PTR PAviPlay::getCurrFrame()
{
  MCI_DGV_STATUS_PARMS mciStatus;

  mciStatus.dwItem = MCI_STATUS_MODE;
  mciSendCommand(DeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);

  // If device is playing, get the position.
  if(mciStatus.dwReturn == MCI_MODE_PLAY || mciStatus.dwReturn == MCI_MODE_PAUSE) {
    MCI_DGV_SET_PARMS mciSet;
    // Put in frame mode.
    mciSet.dwTimeFormat = MCI_FORMAT_FRAMES;
    mciSendCommand(DeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSet);

    mciStatus.dwItem = MCI_STATUS_POSITION;
    mciSendCommand(DeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);
    return mciStatus.dwReturn;
    }
  return 0;
}
//-----------------------------------------------------------
/*
static bool getFrameRate(LPCTSTR filename, DWORD_PTR& target)
{
  AVIFileInit();
  PAVIFILE af = 0;
  target = 0;
  if(!AVIFileOpen(&af, filename, OF_READ, 0)) {
    AVIFILEINFO afi;
    ZeroMemory(&afi, sizeof(afi));
    if(!AVIFileInfo(af, &afi, sizeof(afi))) {
      if(afi.dwScale) {
        double t = afi.dwRate;
        t /= afi.dwScale;
        t *= 1000 / 25;
        target = ROUND_REAL(t);
        }
      }
    AVIFileRelease(af);
    }
  AVIFileExit();
  return toBool(target);
}
*/
//-----------------------------------------------------------
static bool getFrameRate(MCIDEVICEID DeviceID, DWORD_PTR& target)
{
  DWORD res;
/*
  MCI_DGV_SET_PARMS mciSet;
  mciSet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
  res = mciSendCommand(DeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (MCIDEVICEID)(LPSTR)&mciSet);
*/
  target = 0;
  MCI_DGV_STATUS_PARMS mciStatus;
  mciStatus.dwItem = MCI_DGV_STATUS_FRAME_RATE;// | MCI_DGV_STATUS_NOMINAL;
  res = mciSendCommand(DeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);
  if(!res) {
    target = mciStatus.dwReturn;
    if(25000 == target)
      target = 0;
    }
  return toBool(target);
}
//-----------------------------------------------------------
bool PAviPlay::setFrameRate(DWORD rate)
{
  MCI_DGV_SET_PARMS mciSet;
  rate /= 25;
  mciSet.dwSpeed = rate;
  DWORD res = mciSendCommand(DeviceID, MCI_SET, MCI_DGV_SET_SPEED, (DWORD_PTR)&mciSet);
  return toBool(res);
}
//-----------------------------------------------------------
bool PAviPlay::startPlay(LPCTSTR filename, DWORD from, DWORD to)
{
//  DWORD_PTR framerate;
  if(filename && *filename) {
    close();
    if(!open(filename))
      return false;
    }
  if(!DeviceID)
    return false;
/*
  getFrameRate(DeviceID, framerate);
  if(framerate) {
    setFrameRate(framerate);
    getFrameRate(DeviceID, framerate);
    }
*/
//  setFrameRate(1000);
//  setFrameRate(25000);

  MCI_DGV_WINDOW_PARMS mciWindowParms;
//  mciWindowParms.hWnd = *getParent();
  mciWindowParms.hWnd = getHandle();
  DWORD dwReturn = mciSendCommand(DeviceID, MCI_WINDOW, MCI_DGV_WINDOW_HWND, (DWORD_PTR)&mciWindowParms);

  if(dwReturn) {
//    ErrorHandler(dwReturn);
    return false;
    }

  // Use the MCI_PUT command to place the video in the window
  if(!evSize())
    return false;

  // Put in frame mode.
  MCI_DGV_SET_PARMS mciSet;
  mciSet.dwTimeFormat = MCI_FORMAT_FRAMES;
  mciSendCommand(DeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSet);
  MCI_DGV_PLAY_PARMS mciPlayParms;
  mciPlayParms.dwCallback = (DWORD_PTR)getHandle();
  mciPlayParms.dwFrom = from;
  mciPlayParms.dwTo = to;
  DWORD flag = MCI_NOTIFY | MCI_DGV_PLAY_REPEAT | MCI_MCIAVI_PLAY_WINDOW;
  if((DWORD)-1 != from)
    flag |= MCI_FROM;
  if((DWORD)-1 != to)
    flag |= MCI_TO;
  dwReturn = mciSendCommand(DeviceID, MCI_PLAY, flag, (DWORD_PTR) &mciPlayParms);
  if(dwReturn) {
//    ErrorHandler(dwReturn);
    return false;
    }
  return true;
}
//-----------------------------------------------------------
bool PAviPlay::stopPlay(LPDWORD lpCurPos)
{
  if(DeviceID) {
    if(lpCurPos)
      *lpCurPos = (DWORD)getCurrFrame();
    DWORD dwReturn = mciSendCommand(DeviceID, MCI_STOP, 0, 0);
    if(dwReturn) {
//      ErrorHandler(dwReturn);
      return false;
      }
    }
  return true;
}
//-----------------------------------------------------------
bool PAviPlay::open(LPCTSTR filename)
{
  MCI_DGV_OPEN_PARMS mciOpenParms;
  ZeroMemory(&mciOpenParms, sizeof(mciOpenParms));
  mciOpenParms.lpstrElementName = (LPTSTR)filename;
  DWORD dwReturn = mciSendCommand(NULL, MCI_OPEN,
                     MCI_WAIT |
                    //MCI_NOTIFY |
                    MCI_DGV_OPEN_32BIT |
//                    MCI_OPEN_TYPE |
//                    MCI_OPEN_TYPE_ID |
//                    MCI_DEVTYPE_OVERLAY |
                    MCI_OPEN_ELEMENT |
                    //MCI_DGV_OPEN_NOSTATIC,
                    // | MCI_DGV_OPEN_PARENT,
//                    MCI_OPEN_TYPE | MCI_OPEN_ELEMENT
                    0,
                    (DWORD_PTR)&mciOpenParms);
  if(dwReturn) {
//    ErrorHandler(dwReturn);
    return false;
    }
  DeviceID = mciOpenParms.wDeviceID;
  return true;
}
//-----------------------------------------------------------
bool PAviPlay::close()
{
  if(DeviceID) {
    stopPlay();
    DWORD dwReturn = mciSendCommand(DeviceID, MCI_CLOSE, 0, NULL);
    DeviceID = 0;
    if(dwReturn) {
//      ErrorHandler(dwReturn);
      return false;
      }
    }
  return true;
}
//-----------------------------------------------------------
bool PAviPlay::evSize()
{
  if(bySelf)
    return true;
  bySelf = true;
  PRect r;
  GetClientRect(*this, r);
  MCI_DGV_PUT_PARMS mciPutParms;
  bool canS = canStretch;
  if(!canS) {
    DWORD_PTR flag = MCI_DGV_WHERE_SOURCE;// | MCI_DGV_WHERE_MAX;
    MCI_DGV_RECT_PARMS mdrp;
    DWORD dwReturn = mciSendCommand(DeviceID, MCI_WHERE, flag, (DWORD_PTR)&mdrp);
    if(dwReturn || !mdrp.rc.right || !mdrp.rc.bottom || !r.Width() || !r.Height())
      canS = true;
    else {
      double rWidth = mdrp.rc.right;
      rWidth /= r.Width();
      double rHeight = mdrp.rc.bottom;
      rHeight /= r.Height();

      if(rWidth > rHeight) {
        mciPutParms.rc.left = 0;
        mciPutParms.rc.right = r.Width();
        double ratio = mdrp.rc.bottom;
        ratio /= rWidth;
        int dy = (int)((r.Height() - ratio) / 2);
//        mciPutParms.rc.top = dy;
        mciPutParms.rc.top = 0;
        mciPutParms.rc.bottom = (LONG)ratio;
//        mciPutParms.rc.bottom = r.Height() - dy;
        }
      else {
        mciPutParms.rc.top = 0;
        mciPutParms.rc.bottom = r.Height();

        double ratio = mdrp.rc.right;
        ratio /= rHeight;
        int dx = (int)((r.Width() - ratio) / 2);
        mciPutParms.rc.left = 0;
//        mciPutParms.rc.left = dx;
        mciPutParms.rc.right = (LONG)ratio;
//        mciPutParms.rc.right = r.Width() - dx;
        }
      PRect rp;
      GetClientRect(*getParent(), rp);
      int x = (rp.Width() - mciPutParms.rc.right) / 2;
      int y = (rp.Height() - mciPutParms.rc.bottom) / 2;
      SetWindowPos(*this, 0, x, y, mciPutParms.rc.right, mciPutParms.rc.bottom, SWP_NOZORDER);
      }
    }
  if(canS) {
    mciPutParms.rc.left = 0;
    mciPutParms.rc.right = r.Width();
    mciPutParms.rc.top = 0;
    mciPutParms.rc.bottom = r.Height();
    }
//  DWORD_PTR dwReturn = mciSendCommand(DeviceID, MCI_PUT, MCI_DGV_PUT_DESTINATION, (DWORD_PTR)(LPVOID)&mciPutParms);
  DWORD dwReturn = mciSendCommand(DeviceID, MCI_PUT, MCI_DGV_PUT_DESTINATION, (DWORD_PTR)&mciPutParms);
  bySelf = false;
  if(dwReturn) {
//    ErrorHandler(dwReturn);
    return false;
    }
  return true;
}
//-----------------------------------------------------------
void PAviPlay::evPaint(HDC hdc, const RECT& rc)
{
  if(DeviceID) {
    MCI_DGV_UPDATE_PARMS mdup;
    mdup.hDC = hdc;
    PRect r;
    GetClientRect(*this, r);
//    PRect r(rc);
//    r.right = r.Width();
//    r.bottom = r.Height();
    mdup.rc.left = r.left;
    mdup.rc.top = r.top;
    mdup.rc.right = r.Width();
    mdup.rc.bottom = r.Height();
    DWORD_PTR flag = MCI_DGV_UPDATE_HDC | MCI_DGV_RECT | MCI_DGV_UPDATE_PAINT;
    mciSendCommand(DeviceID, MCI_UPDATE, flag, (DWORD_PTR)&mdup);
    }
}
//-----------------------------------------------------------
LRESULT PAviPlay::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      close();
      break;
/**/
    case WM_ERASEBKGND:
/*
      do {
        RECT r;
        GetClientRect(*this, &r);
        evPaint((HDC)wParam, r);
        } while(false);
*/
      return 1;
/**/
    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        evPaint(hdc, ps.rcPaint);
        EndPaint(*this, &ps);
        } while(false);
      return 0;
/*
      case MM_MCINOTIFY:
        if (wParam == MCI_NOTIFY_FAILURE)
            MessageBox(NULL, "Failure." , "MM_MCINOTIFY error", MB_OK);
          break;
*/
      case WM_SIZE:
        evSize();
        break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
