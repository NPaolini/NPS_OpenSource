//----------- winServerConn.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "winServerConn.h"
#include <stdio.h>
//----------------------------------------------------------------------------
winServerConn::winServerConn(PWin* parent, uint id, LPCTSTR name, HINSTANCE hInst) :
      baseClass(parent, name, hInst), serverConn(0), idTimer(0)
{
}
//----------------------------------------------------------------------------
winServerConn::~winServerConn()
{
  delete serverConn;
  serverConn = 0;

  removeAllClient();
  destroy();
}
//----------------------------------------------------------------------------
bool winServerConn::create()
{
  if(!baseClass::create())
    return false;
  return createConn();
}
//----------------------------------------------------------------------------
#define ID_TIMER 1345
//----------------------------------------------------------------------------
static
void killTimer(HWND hwnd, uint& tm)
{
  if(tm) {
    KillTimer(hwnd, tm);
    tm = 0;
    }
}
//----------------------------------------------------------------------------
static
int setTimer(HWND hwnd, uint tm)
{
  killTimer(hwnd, tm);
  return SetTimer(hwnd, ID_TIMER, 30 * 1000, 0);
}
//----------------------------------------------------------------------------
static
void closeApp()
{
  HWND hwmain = *getAppl()->getMainWindow();
  DWORD id = GetWindowThreadProcessId(hwmain, 0);
  if(!PostThreadMessage(id, WM_QUIT, 0, 0))
    PostMessage(hwmain, WM_QUIT, 0, 0);
}
//----------------------------------------------------------------------------
LRESULT  winServerConn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_CUSTOM_MSG_CONNECTION == message) {
    switch(LOWORD(wParam)) {
      case CM_BY_LAN_SERVER:
        switch(HIWORD(wParam)) {
          case HIWP_ADD_CLIENT:
            addNewClient((HANDLE_CONN)lParam);
            break;
          }
        break;
      }
    }

  else switch(message) {

    case WM_TIMER:
      if(idTimer && idTimer == wParam)
        closeApp();
        // fall trough
      else
        break;
    case WM_DESTROY:
      KillTimer(hwnd, idTimer);
      idTimer = 0;
      break;
    case WM_FROM_CLIENT:
      switch(LOWORD(wParam)) {

        case CM_REMOVE_CLIENT:
          removeClient((winClientConn*)lParam);
          break;

        case CM_OK_CLIENT:
          addClient((winClientConn*)lParam);
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static int getNewGlobalId()
{
  static int ix;
  return (++ix & 0x7fff) + 100;
}
//----------------------------------------------------------------------------
#define MAX_CONN 100
//----------------------------------------------------------------------------
static long gClientCount = 0;
//----------------------------------------------------------------------------
void winServerConn::addNewClient(HANDLE_CONN hc)
{
  criticalLock crtLck(csMsg);
  if(Waiting.getElem() + Clients.getElem() > MAX_CONN) {
    PConnBase dummy(hc);
    return;
    }
  winClientConn* w = new winClientConn(this, getNewGlobalId(), hc);
  if(!w->create()) {
    delete w;
    return;
    }
  killTimer(*this, idTimer);
  int nElem = Waiting.getElem();
  Waiting[nElem] = w;
  InterlockedIncrement(&gClientCount);
}
//----------------------------------------------------------------------------
bool winServerConn::remove_client(PVect<winClientConn*>& set, winClientConn* client)
{
  int nElem = set.getElem();
  for(int i = nElem - 1; i >= 0; --i) {
    if(client == set[i]) {
      set.remove(i);
      delete client;
      if(!InterlockedDecrement(&gClientCount))
        idTimer = setTimer(*this, idTimer);
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
void winServerConn::removeAllClient()
{
  criticalLock crtLck(csMsg);
  int nElem = Waiting.getElem();
  for(int i = nElem - 1; i >= 0; --i)
    remove_client(Waiting, Waiting[i]);

  nElem = Clients.getElem();
  for(int i = nElem - 1; i >= 0; --i)
    remove_client(Clients, Clients[i]);
}
//----------------------------------------------------------------------------
void winServerConn::removeClient(winClientConn* client)
{
  criticalLock crtLck(csMsg);
  if(!remove_client(Waiting, client))
    remove_client(Clients, client);
}
//----------------------------------------------------------------------------
void winServerConn::addClient(winClientConn* client)
{
  criticalLock crtLck(csMsg);
  int nElem = Waiting.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(client == Waiting[i]) {
      Waiting.remove(i);
      nElem = Clients.getElem();
      Clients[nElem] = client;
      break;
      }
    }
}
//----------------------------------------------------------------------------
