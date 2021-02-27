//-------- pFullView.cpp ---------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pFullView.h"
//----------------------------------------------------------------------------
void pFullView::init()
{
  Attr.style &= ~WS_CHILD;
  Attr.style |= WS_OVERLAPPED | WS_THICKFRAME;
  Attr.exStyle = WS_EX_TOOLWINDOW;
}
//----------------------------------------------------------------------------
void pFullView::resize()
{
  HWND hwnd = getHWClient();
  if(!hwnd)
    return;
  PRect r;
  GetClientRect(*this, r);
  SetWindowPos(hwnd, 0, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);
}
//----------------------------------------------------------------------------
LRESULT pFullView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      resize();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------

