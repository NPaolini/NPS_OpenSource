//----------- common_color.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdio.h>
//----------------------------------------------------------------------------
#include "Macro_utils.h"
#include "common_color.h"
#include "pEdit.h"
#include "p_Util.h"
#include "headerMsg.h"
//----------------------------------------------------------------------------
bool PRGBFilter::accept(WPARAM& key, HWND ctrl)
{
  if(isCommonKey(key, ctrl))
    return true;

  int len = SendMessage(ctrl, WM_GETTEXTLENGTH, 0, 0);
  if(len > 0) {
    TCHAR buff[50];
    GetWindowText(ctrl, buff, SIZE_A(buff));
    int init;
    int end;
    if(!getSel(ctrl, init, end))
      return false;
    TCHAR t[50];
    memset(t, 0, sizeof(t));
    for(int i = 0; i < init; ++i)
      t[i] = buff[i];
    t[init] = key;

    for(int i = init + 1, j = end; buff[j]; ++i, ++j)
      t[i] = buff[j];
    if(_ttoi(t) > 255)
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_EditFocus::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SETFOCUS:
      PostMessage(*getParent(), WM_CUSTOM_SET_FOCUS, Attr.id, 0);
      break;
    }
  return PEdit::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
PColorBox::PColorBox(PWin * parent, COLORREF color, uint resid, HINSTANCE hinst) :
      PStatic(parent, resid, hinst), Color(color), Brush(CreateSolidBrush(Color))
{ }
//----------------------------------------------------------------------------
PColorBox::PColorBox(PWin * parent, COLORREF color, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PStatic(parent, id, r, text, hinst), Color(color), Brush(CreateSolidBrush(Color))
{ }
//----------------------------------------------------------------------------
PColorBox::~PColorBox()
{
  if(Brush)
    DeleteObject(HGDIOBJ(Brush));
}
//----------------------------------------------------------------------------
void PColorBox::set(COLORREF color)
{
  Color = color;
  if(Brush)
    DeleteObject(HGDIOBJ(Brush));
  Brush = CreateSolidBrush(Color);
  InvalidateRect(*this, 0, 0);
}
//----------------------------------------------------------------------------
