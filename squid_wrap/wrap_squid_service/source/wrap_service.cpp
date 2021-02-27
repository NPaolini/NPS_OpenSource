//--------- wrap_service.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "WrapSquidServer.h"
#include "p_manage_ini.h"
#include "queueLog.h"
//----------------------------------------------------------------------------
static HANDLE hEvent;
//----------------------------------------------------------------------------
int startMain()
{
//  MessageBox(0, _T("TTT"), _T("Waiting"), MB_OK); // only for debugging while in console mode
  uint port = 0;
  do {
    TCHAR tmp[_MAX_PATH];
    GetModuleDirName(SIZE_A(tmp), tmp);
    appendPath(tmp, INI_FILE);
    manageIni ini(tmp);
    if(!ini.parse())
      return false;
    LPCTSTR p = ini.getValue(INI_FILE_KEY, INI_FILE_BLK);
    if(!p)
      return 0;
    port = _ttoi(p);
    } while(false);
  PMainWin mw(_T("Hidden"));
  mw.Attr.style &= ~WS_VISIBLE;
  myServerConn W(&mw, 1111, port, GetModuleHandle(NULL));
  if(!mw.create()) 
    return 0;
  logQueue& q = getLogQueue();
  q.set_Owner(&W);
  hEvent = CreateEvent(0, 0, 0, 0);

  HANDLE HandlesToWaitFor[] = { hEvent };

  DWORD timeout = INFINITE;
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
      DWORD result = MsgWaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, timeout, QS_ALLINPUT);
      if(result == (WAIT_OBJECT_0 + SIZE_A(HandlesToWaitFor)))
         continue;
      switch(result) {
        case WAIT_TIMEOUT:
          break;
        case WAIT_OBJECT_0:
        default:
          terminate = true;
          break;
        }
      }
    }
  return 0;
}
//----------------------------------------------------------------------------
int stopMain()
{
  SetEvent(hEvent);
  return 1;
}
//----------------------------------------------------------------------------
