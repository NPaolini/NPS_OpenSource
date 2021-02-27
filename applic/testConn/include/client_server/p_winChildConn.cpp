//----------- p_winChildConn.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_winChildConn.h"
//----------------------------------------------------------------------------
p_winChildConn::p_winChildConn(PWin* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst) :
      baseClass(parent, id, 0, 0, 20, 20, 0, hInst), Conn(hc ? new PConnBase(hc) : 0),
      hEventCloseConn(0), hEventHasChar(0), hEventSendChar(0), threadActive(false), idTimer(0)
{
  hEventCloseConn = CreateEvent(0, TRUE, 0, 0);
  hEventHasChar = CreateEvent(0, 0, 0, 0);
  hEventSendChar = CreateEvent(0, 0, 0, 0);
}
//----------------------------------------------------------------------------
p_winChildConn::~p_winChildConn()
{
  stopThread();
  delete Conn;
  Conn = 0;

  CloseHandle(hEventCloseConn);
  CloseHandle(hEventHasChar);
  CloseHandle(hEventSendChar);
  destroy();
}
//----------------------------------------------------------------------------
static bool startConn(p_winChildConn* owner, PConnBase* conn)
{
  conn->changeOwner(owner->getHandle());

  DWORD idThread;
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReaderClientProc, owner, 0, &idThread);
  if(hThread) {
    CloseHandle(hThread);
    conn->reqNotify(true);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool p_winChildConn::create()
{
  Attr.style &= ~WS_VISIBLE;
  if(!baseClass::create())
    return false;
  if(Conn && startConn(this, Conn))
    idTimer = SetTimer(*this, 555, getTimeoutAck(), 0);
  return true;
}
//----------------------------------------------------------------------------
void p_winChildConn::deleteConn()
{
  stopThread();
  delete Conn;
  Conn = 0;
}
//----------------------------------------------------------------------------
void p_winChildConn::addConn(HANDLE_CONN hc)
{
  deleteConn();
  ResetEvent(hEventCloseConn);
  Conn = new PConnBase(hc);
  if(startConn(this, Conn))
    idTimer = SetTimer(*this, 555, getTimeoutAck(), 0);
}
//----------------------------------------------------------------------------
LRESULT  p_winChildConn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_CUSTOM_MSG_CONNECTION == message) {
    switch(LOWORD(wParam)) {
      case CM_MSG_HAS_CHAR:
        do {
        // meglio nessuna altra notifica finché si elabora
          Conn->reqNotify(false);
          SetEvent(hEventHasChar);
          } while(false);
        break;
      default:
        if(0 == HIWORD(wParam))
          PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_REMOVE_CLIENT, 0), (LPARAM)this);
        break;
      }
    }
  else switch(message) {
    case WM_TIMER:
      if(idTimer != wParam)
        break;
      PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_REMOVE_CLIENT, 0), (LPARAM)this);
      break;
    case WM_DESTROY:
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      break;
    case WM_FROM_CLIENT:
      // richiesta interna
      switch(LOWORD(wParam)) {
        case CM_END_THREAD:
          PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_REMOVE_CLIENT, 0), (LPARAM)this);
          break;
        case CM_KILL_TIMER:
          if(idTimer) {
            KillTimer(hwnd, idTimer);
            idTimer = 0;
            }
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void p_winChildConn::stopThread()
{
  SetEvent(hEventCloseConn);
  for(;;) {
    if(!threadActive)
      break;
    Sleep(100);
    }
}
//----------------------------------------------------------------------------
bool p_winChildConn::isOnLine()
{
  return Conn && Conn->isConnected();
}
//----------------------------------------------------------------------------
unsigned FAR PASCAL ReaderClientProc(void* cl)
{
  p_winChildConn *Win = reinterpret_cast<p_winChildConn*>(cl);
  PConnBase* conn = Win->Conn;

  Win->th_setThread();

  bool oldNotifyConn = conn->reqNotify(false);

  bool terminate = false;
  bool needWait = false;

  HANDLE HandlesToWaitFor[] = {
    Win->hEventCloseConn,
    Win->hEventSendChar,
    Win->hEventHasChar,
    };

  SetEvent(Win->hEventHasChar);
//  const DWORD timeout = INFINITE;
  const DWORD timeout = 1000 * 10;
  while(!terminate) {

    DWORD result = WaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, timeout);
    switch(result) {
      case WAIT_OBJECT_0 + 1:
        if(!Win->th_writeConn())
          terminate = true;
        break;
      case WAIT_TIMEOUT:
      case WAIT_OBJECT_0 + 2:
        if(!Win->th_readConn())
          terminate = true;
        else
          if(!conn->has_string())
            conn->reqNotify(true);
        break;

      case WAIT_OBJECT_0:
      default:
        terminate = true;
        break;
      }
    if(!terminate) {
      // verifica se ci sono dati da leggere
      if(conn->has_string())
        SetEvent(HandlesToWaitFor[2]);
      }
    }
  Win->th_resetThread();
  PostMessage(*Win, WM_FROM_CLIENT, MAKEWPARAM(CM_END_THREAD, 0), 0);
  return true;

}
//----------------------------------------------------------------------------
