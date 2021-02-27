//-------------------- PSplitWin.cpp ------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "PSplitWin.h"
//-----------------------------------------------------------
PSplitWin::PSplitWin(PWin* parent, uint id, uint perc, uint width, uint style, HINSTANCE hInst) :
     baseClass(parent, id, 0, 0, 0, 0, 0, hInst), Split(0)
{
//  Attr.style |= WS_CLIPCHILDREN;
  Attr.exStyle |= WS_EX_CONTROLPARENT;
  Split = new PSplitter(this, 0, 0, perc, width, style);
}
//---------------------------------------------------------
PSplitWin::~PSplitWin()
{
  destroy();
  delete Split;
}
//---------------------------------------------------------
bool PSplitWin::create()
{
  if(!baseClass::create())
    return false;
  PRect r;
  GetClientRect(*getParent(), r);
  setWindowPos(0, r, SWP_NOZORDER);
  return Split->create();
}
//---------------------------------------------------------
LRESULT PSplitWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(Split)
    Split->windowProcSplit(hwnd, message, wParam, lParam);
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PSplitWin::replaceSplit(PSplitter* other, bool preserveData)
{
  if(other != Split) {
    if(other && preserveData)
      other->copyDataFrom(Split);
    delete Split;
    Split = other;
    }
}
//----------------------------------------------------------------------------
