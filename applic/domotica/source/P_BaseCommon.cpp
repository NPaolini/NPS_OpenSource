//---------- P_BaseCommon.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_BaseCommon.h"
//----------------------------------------------------------------------------
P_BaseCommon::P_BaseCommon(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), Conn(conn), Prev(0) {}
//----------------------------------------------------------------------------
P_BaseCommon::~P_BaseCommon()
{
  Conn->changeOwner(Prev);
  destroy();
}
//----------------------------------------------------------------------------
bool P_BaseCommon::create()
{
  if(!baseClass::create())
    return false;
  Prev = Conn->changeOwner(getHandle());
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_BaseCommon::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_CUSTOM_MSG_CONNECTION == message) {
    switch(LOWORD(wParam)) {
      case CM_MSG_HAS_CHAR:
        readConn(HIWORD(wParam));
        break;
      }
    return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
