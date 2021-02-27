//-------- fullView.cpp ---------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "fullView.h"
//----------------------------------------------------------------------------
void fullView::init()
{
  Attr.style |= WS_CHILD;
  Attr.exStyle |= WS_EX_CONTROLPARENT;
}
//----------------------------------------------------------------------------
void fullView::resize()
{
  HWND hwnd = getHWClient();
  if(!hwnd)
    return;
  PRect r;
  GetClientRect(*this, r);
  SetWindowPos(hwnd, 0, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);
}
//----------------------------------------------------------------------------
LRESULT fullView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      resize();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------

