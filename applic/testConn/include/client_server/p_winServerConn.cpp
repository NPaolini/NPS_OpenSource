//----------- p_winServerConn.cpp --------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_winServerConn.h"
#include "p_winChildConn.h"
#include <stdio.h>
//----------------------------------------------------------------------------
p_winServerConn::p_winServerConn(PWin* parent, uint id, uint port, HINSTANCE hInst) :
      baseClass(parent, id, 0, 0, 20, 20, 0, hInst)
{
  listenPort[0] = port;
}
//----------------------------------------------------------------------------
p_winServerConn::p_winServerConn(PWin* parent, uint id, const PVect<WORD>& port, HINSTANCE hInst) :
      baseClass(parent, id, 0, 0, 20, 20, 0, hInst), listenPort(port)
{ }
//----------------------------------------------------------------------------
p_winServerConn::~p_winServerConn()
{
  flushPV(serverConn);

  removeAllClient();
  destroy();
}
//----------------------------------------------------------------------------
bool p_winServerConn::create()
{
  Attr.style &= ~WS_VISIBLE;
  if(!baseClass::create())
    return false;
  return createConn();
}
//--------------------------------------------------------------------------------
bool p_winServerConn::createConn()
{
  uint nElem = listenPort.getElem();
  for(uint i = 0, j = 0; i < nElem; ++i) {
    serverConn[j] = new PConnBase(listenPort[i], getHandle(), false);
    if(!serverConn[j]->open()) {
      delete serverConn[j];
      serverConn.remove(j);
      }
    else ++j;
    }
  return toBool(serverConn.getElem());
}
//----------------------------------------------------------------------------
LRESULT  p_winServerConn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
    case WM_FROM_CLIENT:
      switch(LOWORD(wParam)) {
        case CM_REMOVE_CLIENT:
          removeClient((p_winChildConn*)lParam);
          break;
        case CM_OK_CLIENT:
          addClient((p_winChildConn*)lParam);
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
void p_winServerConn::addNewClient(HANDLE_CONN hc)
{
  criticalLock crtLck(csMsg);
  if(Waiting.getElem() + Clients.getElem() > MAX_CONN) {
    PConnBase dummy(hc);
    return;
    }
  p_winChildConn* w = allocChild(this, getNewGlobalId(), hc);
  if(!w->create()) {
    delete w;
    return;
    }
  int nElem = Waiting.getElem();
  Waiting[nElem] = w;
}
//----------------------------------------------------------------------------
static
bool remove_client(PVect<p_winChildConn*>& set, p_winChildConn* client)
{
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(client == set[i]) {
      set.remove(i);
      delete client;
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
void p_winServerConn::removeAllClient()
{
  criticalLock crtLck(csMsg);
  flushPV(Waiting);
  flushPV(Clients);
}
//----------------------------------------------------------------------------
void p_winServerConn::removeClient(p_winChildConn* client)
{
  criticalLock crtLck(csMsg);
  if(!remove_client(Waiting, client))
    remove_client(Clients, client);
}
//----------------------------------------------------------------------------
void p_winServerConn::addClient(p_winChildConn* client)
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
//--------------------------------------------------
uint p_winServerConn::getConnStatus()
{
  if(!serverConn.getElem())
    return 2;
   // ci sono connessioni attive
  if(Clients.getElem())
    return 1;
  // tutto tace
  return 0;
}
//----------------------------------------------------------------------------
