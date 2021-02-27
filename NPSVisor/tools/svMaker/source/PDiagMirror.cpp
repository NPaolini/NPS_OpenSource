//----------- PDiagMirror.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "PDiagMirror.h"
//----------------------------------------------------------------------------
PDiagMirror::PDiagMirror(PWin* parent, bool& all, uint& choose, bool hasSelected, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  All(all), Choose(choose), hasSelected(hasSelected)
{
}
//----------------------------------------------------------------------------
PDiagMirror::~PDiagMirror()
{
}
//----------------------------------------------------------------------------
bool PDiagMirror::create()
{
  if(!baseClass::create())
    return false;
  if(!hasSelected) {
    ENABLE(IDC_SELECTED, false);
    SET_CHECK(IDC_MIRROR_ALL);
    }
  else if(All)
    SET_CHECK(IDC_MIRROR_ALL);
  else
    SET_CHECK(IDC_SELECTED);

  if(!Choose)
    SET_CHECK(IDC_HORZ);
  else {
    if(Choose & emtVert)
      SET_CHECK(IDC_VERT);
    if(Choose & emtHorz)
      SET_CHECK(IDC_HORZ);
    }
  return true;
}
//----------------------------------------------------------------------------
LRESULT PDiagMirror::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_VERT:
          if(!IS_CHECKED(IDC_VERT))
            SET_CHECK(IDC_HORZ);
          break;
        case IDC_HORZ:
          if(!IS_CHECKED(IDC_HORZ))
            SET_CHECK(IDC_VERT);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PDiagMirror::CmOk()
{
  if(!IS_CHECKED(IDC_VERT) && !IS_CHECKED(IDC_HORZ))
    return;
  Choose = 0;
  if(IS_CHECKED(IDC_VERT))
    Choose = emtVert;
  if(IS_CHECKED(IDC_HORZ))
    Choose |= emtHorz;
  All = IS_CHECKED(IDC_MIRROR_ALL);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
