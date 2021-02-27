//------ svEdit.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "svEdit.h"
#include "mainClient.h"
#include "1.h"
//----------------------------------------------------------------------------
extern bool onTouchKeyb(PWin* child);
//----------------------------------------------------------------------------
LRESULT svEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      do {
        enum eTypeUseTouch { eUT_NoChecked, eUT_NoUse, eUT_OnUse };
        static eTypeUseTouch eUseTouch = eUT_NoChecked;
        if(eUT_NoUse == eUseTouch)
          break;
        if(eUT_NoChecked == eUseTouch) {
          LPCTSTR p = getString(ID_USE_TOUCH_KEYB);
          if(p && _ttoi(p))
            eUseTouch = eUT_OnUse;
          else {
            eUseTouch = eUT_NoUse;
            break;
            }
          }
        if(!onTouchKeyb(this)) {
          mainClient* mc = getMain();
          PostMessage(*mc, WM_CUSTOM, MAKEWPARAM(MSG_SHOW_KEYB, Attr.id), (LPARAM)this);
          }
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void svEdit::makeSingleLine()
{
/*
  DWORD val = GetClassLong(*this, GCL_STYLE);
  if(!(val & ES_MULTILINE))
    return;
*/
  PRect r;
  GetWindowRect(*this, r);
  MapWindowPoints(HWND_DESKTOP, *getParent(), (LPPOINT)(RECT*)r, 2);
  // se non ha già un titolo, recupera quello del controllo
  if(!getCaption()) {
    TCHAR title[260] = _T("\0");
    GetWindowText(*this, title, SIZE_A(title));
    if(title[0])
      setCaption(title);
    }
  destroy();
  Attr.x = r.left;
  Attr.y = r.top;
  Attr.w = r.Width();
  Attr.h = r.Height();
  clearFlag(pfFromResource);
  Attr.style &= ~ES_MULTILINE;
  PControl::create();
  InvalidateRect(*this, 0, 1);
}
//----------------------------------------------------------------------------
void svEdit::copyFilter(HWND from)
{
  PWin* w = PWin::getWindowPtr(from);
  if(!w)
    return;
  svEdit* ed = dynamic_cast<svEdit*>(w);
  if(!ed)
    return;
  const PFilter* cflt = ed->getFilter();
  if(cflt) {
    PFilter* flt = cflt->allocCopy();
    if(flt) {
      setFilter(flt);
      flt->initCtrl(*this);
      }
    }
}
