//----------- p_winClientConn.cpp --------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_winClientConn.h"
#include "p_winChildConn.h"
#include <stdio.h>

//----------------------------------------------------------------------------
#define IP4(a) (((a) >> 24) & 0xff)
#define IP3(a) (((a) >> 16) & 0xff)
#define IP2(a) (((a) >> 8) & 0xff)
#define IP1(a) (((a) >> 0) & 0xff)
//----------------------------------------------------------------------------
static LPTSTR makeStrIp(DWORD addr)
{
  TCHAR t[32];
  wsprintf(t, _T("%d.%d.%d.%d"), IP1(addr), IP2(addr), IP3(addr), IP4(addr));
  return str_newdup(t);
}
//----------------------------------------------------------------------------
p_winClientConn::p_winClientConn(PWin* parent, uint id, uint port, LPCTSTR ip, HINSTANCE hInst) :
      baseClass(parent, id, 0, 0, 20, 20, 0, hInst), IP(str_newdup(ip)), Port(port), Child(0)
{}
//----------------------------------------------------------------------------
p_winClientConn::p_winClientConn(PWin* parent, uint id, uint port, DWORD ip, HINSTANCE hInst) :
      baseClass(parent, id, 0, 0, 20, 20, 0, hInst), IP(makeStrIp(ip)), Port(port), Child(0)
{}
//----------------------------------------------------------------------------
p_winClientConn::~p_winClientConn()
{
  delete []IP;
  destroy();
}
//----------------------------------------------------------------------------
void p_winClientConn::changeIP(DWORD newIP)
{
  delete []IP;
  IP = makeStrIp(newIP);
}
//----------------------------------------------------------------------------
bool p_winClientConn::create()
{
  Attr.style &= ~WS_VISIBLE;
  if(!baseClass::create())
    return false;

  return true;
}
//----------------------------------------------------------------------------
bool p_winClientConn::createConn()
{
  if(!Child)
    return createChild();
  Child->deleteConn();
  PConnBase conn(IP, Port, 0, false);
  HANDLE_CONN hc = 0;
  if(conn.open())
    hc = conn.detach();
  if(!toBool(hc))
    return false;
  Child->addConn(hc);
  return true;
}
//----------------------------------------------------------------------------
bool p_winClientConn::deleteConn()
{
  if(Child) {
    Child->deleteConn();
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool p_winClientConn::createChild(bool startConn)
{
  HANDLE_CONN hc = 0;
  if(startConn) {
    PConnBase conn(IP, Port, 0, false);
    if(conn.open())
      hc = conn.detach();
    }
  p_winChildConn* w = allocChild(this, 12345, hc);
  if(!w->create()) {
    delete w;
    return false;
    }
  Child = w;
  return true;
}
//----------------------------------------------------------------------------
bool p_winClientConn::deleteChild()
{
  if(Child) {
    delete Child;
    Child = 0;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
LRESULT  p_winClientConn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_FROM_CLIENT:
      switch(LOWORD(wParam)) {
        case CM_REMOVE_CLIENT:
          Child->deleteConn();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
bool p_winClientConn::hasClient() { return toBool(Child); }
//----------------------------------------------------------------------------
bool p_winClientConn::isOnLine() { return Child? Child->isOnLine() : false; }
//----------------------------------------------------------------------------
