//----------- winClientConn.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "winClientConn.h"
#include "winServerConn.h"
#include "svIserverConn.h"
#include <stdio.h>
//----------------------------------------------------------------------------
#pragma comment(lib, ".\\libnodave.lib")
//----------------------------------------------------------
#ifdef TEST_NO_CONN
//----------------------------------------------------------
//----------------------------------------------------------
static int DB_READ(daveConnection*,int,int,int,void*) { return 0; }
static int DB_WRITE(daveConnection*,int,int,int,void*) { return 0; }
//----------------------------------------------------------
#else
//----------------------------------------------------------
//----------------------------------------------------------
#define DB_READ(dc, DB, start, len, buffer)   \
    daveReadManyBytes(dc, daveDB, DB, start, len, buffer)

#define DB_WRITE(dc, DB, start, len, buffer)   \
    daveWriteManyBytes(dc, daveDB, DB, start, len, buffer)
//----------------------------------------------------------
#endif
//----------------------------------------------------------------------------
winClientConn::winClientConn(winServerConn* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst) :
      baseClass(parent, id, 0, 0, 20, 20, 0, hInst), Conn(new PConnBase(hc)), idPlc(0),
      hEventCloseConn(0), hEventHasChar(0), threadActive(false),
      idTimer(0), hEventDaveConn(0)
{
  hEventCloseConn = CreateEvent(0, TRUE, 0, 0);
  hEventHasChar = CreateEvent(0, 0, 0, 0);
  hEventDaveConn = CreateEvent(0, 0, 0, 0);
}
//----------------------------------------------------------------------------
winClientConn::~winClientConn()
{
  stopThread();
  delete Conn;
  Conn = 0;

  CloseHandle(hEventCloseConn);
  CloseHandle(hEventHasChar);
  CloseHandle(hEventDaveConn);
  destroy();
}
//----------------------------------------------------------------------------
bool winClientConn::create()
{
  Attr.style &= ~WS_VISIBLE;
  if(!baseClass::create())
    return false;
  Conn->passThrough(cChangeHwndOwner, (LPDWORD)getHandle());
  do {
    PWinThread* par = getParentWin<PWinThread>(this);
    if(par)
      idPlc = par->getIdPlc();
    } while(false);

  DWORD idThread;
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReaderClientProc,
                  this, 0, &idThread);
  if(hThread) {
    CloseHandle(hThread);
    Conn->reqNotify(true);
    }
  else
    return false;
  idTimer = SetTimer(*this, 555, getTimeoutAck(), 0);
  return true;
}
//----------------------------------------------------------------------------
LRESULT  winClientConn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
void winClientConn::stopThread()
{
  SetEvent(hEventCloseConn);
  for(;;) {
    if(!threadActive)
      break;
    Sleep(100);
    }
}
//----------------------------------------------------------------------------
bool winClientConn::readConn()
{
  int dim = Conn->has_string();
  if(!dim)
    return true;

  dataSend* packet = getFreeData();

  bool success = SZ_STRU_MIN == Conn->read_string((LPBYTE)packet, SZ_STRU_MIN);

  if(!success) {
    releaseFreeData(packet);
    return true;
    }
  if(packet->id != idPlc) {
    releaseFreeData(packet);
    return true;
    }

  switch(packet->cmd) {
    case DB_READ_LIBNODAVE:
    case DB_LOGOUT_LIBNODAVE:
      break;
    default:
      Conn->read_string(packet->buffer, packet->len);
      break;
    }
  // len viene usato come codice di ritorno di errore, quindi viene salvato
  dim = packet->len;
  switch(packet->cmd) {
    case DB_READ_LIBNODAVE:
      fillReadToSend(packet);
      Conn->write_string((LPBYTE)packet, SZ_STRU_MAX(dim));
      break;
    case DB_WRITE_LIBNODAVE:
      Send(packet);
      Conn->write_string((LPBYTE)packet, SZ_STRU_MIN);
      break;
    case DB_LOGIN_LIBNODAVE:
      PostMessage(*this, WM_FROM_CLIENT, MAKEWPARAM(CM_KILL_TIMER, 0), (LPARAM)this);
      do {
        int len = _tcslen(getPassword()) + 1;
        WORD low = (len == packet->len && !cmpStr(packet->buffer, getPassword(), packet->len)) ? CM_OK_CLIENT : CM_REMOVE_CLIENT;
        PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(low, 0), (LPARAM)this);
        packet->len = 0;
        packet->cmd = CM_OK_CLIENT == low ? DB_LOGGED_LIBNODAVE : DB_ERROR_LIBNODAVE;
        Conn->write_string((LPBYTE)packet, SZ_STRU_MIN);
        } while(false);
      break;
    case DB_LOGOUT_LIBNODAVE:
      PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_REMOVE_CLIENT, 0), (LPARAM)this);
      break;
    }
  releaseFreeData(packet);
  return true;
}
//----------------------------------------------------------------------------
bool winClientConn::fillReadToSend(dataSend* packet)
{
  serializeConn srzC(hEventDaveConn);
  getDaveConn(srzC);
  int res = 0;
  while(srzC.dC) {
    if((res = DB_READ(srzC.dC, packet->db, packet->start, packet->len, packet->buffer)) != 0)
      break;
    releaseDaveConn(srzC);
    packet->cmd = DB_READED_LIBNODAVE;
    return true;
    }
  releaseDaveConn(srzC);
  packet->cmd = DB_ERROR_LIBNODAVE;
  packet->len = res;
  return false;
}
//----------------------------------------------------------------------------
bool winClientConn::Send(dataSend* packet)
{
  serializeConn srzC(hEventDaveConn);
  getDaveConn(srzC);
  int res = 0;
  while(srzC.dC) {
    if((res = DB_WRITE(srzC.dC, packet->db, packet->start, packet->len, packet->buffer)) != 0)
      break;
    releaseDaveConn(srzC);
    packet->cmd = DB_WRITED_LIBNODAVE;
    return true;
    }
  releaseDaveConn(srzC);
  packet->cmd = DB_ERROR_LIBNODAVE;
  packet->len = res;
  return false;
}
//----------------------------------------------------------------------------
void winClientConn::getDaveConn(serializeConn& srzC)
{
  PWinThread* par = getParentWin<PWinThread>(this);
  if(!par)
    return;
  par->getDaveConn(srzC);
  WaitForSingleObject(srzC.hWait, INFINITE);
}
//----------------------------------------------------------------------------
void winClientConn::releaseDaveConn(serializeConn& srzC)
{
  PWinThread* par = getParentWin<PWinThread>(this);
  if(!par)
    return;
  par->releaseDaveConn(srzC);
}
//----------------------------------------------------------------------------
unsigned FAR PASCAL ReaderClientProc(void* cl)
{
  winClientConn *Win = reinterpret_cast<winClientConn*>(cl);
  PConnBase* conn = Win->Conn;

  Win->setThread();

  bool oldNotifyConn = conn->reqNotify(false);

  bool terminate = false;
  bool needWait = false;

  HANDLE HandlesToWaitFor[] = {
    Win->hEventCloseConn,
    Win->hEventHasChar,
    };

  SetEvent(Win->hEventHasChar);
  const DWORD timeout = INFINITE;
//  const DWORD timeout = 1000 * 10;
  while(!terminate) {

    DWORD result = WaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, timeout);
    switch(result) {
      case WAIT_TIMEOUT:
      case WAIT_OBJECT_0 + 1:
        if(!Win->readConn())
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
        SetEvent(HandlesToWaitFor[1]);
      }
    }
  Win->resetThread();
  PostMessage(*Win, WM_FROM_CLIENT, MAKEWPARAM(CM_END_THREAD, 0), 0);
  return true;

}
//----------------------------------------------------------------------------
