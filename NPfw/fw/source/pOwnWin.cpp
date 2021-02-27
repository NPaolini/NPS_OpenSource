//-------------------- pOwnWin.cpp --------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnWin.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
void POwnWin::getWindowClass(WNDCLASS& wcl)
{
  PWin::getWindowClass(wcl);
  wcl.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);//0;
/**/
#if 0
  wcl.style |= CS_OWNDC;
  wcl.style &= ~(CS_SAVEBITS | CS_PARENTDC);
#else
  wcl.style |= CS_PARENTDC;
  wcl.style &= ~(CS_OWNDC | CS_SAVEBITS);
#endif
/**/
}
//-----------------------------------------------------------
void POwnWin::invalidate()
{
  PRect r = getRect();
  InvalidateRect(*getParent(), r, 0);
}
//-----------------------------------------------------------
LRESULT POwnWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DRAWITEM:
      if(drawItem((uint)wParam, (LPDRAWITEMSTRUCT) lParam))
        return TRUE;
      break;
    case WM_MEASUREITEM:
      if(measureItem((uint)wParam, (LPMEASUREITEMSTRUCT) lParam))
        return TRUE;
      break;
    case WM_KILLFOCUS:
      invalidate();
      break;

    case WM_NCPAINT:
      return 0;

    case WM_ERASEBKGND:
      return 1;

    }
  return PControl::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
bool POwnWin::create()
{
  if(!PControl::create())
    return false;
  if(isFlagSet(pfFromResource)) {
    PRect r;
    GetWindowRect(*this, r);
    MapWindowPoints(HWND_DESKTOP, *getParent(), (LPPOINT)(RECT*)r, 2);
    setCoord(r);
#define RECREATE_ALWAYS
    // se ha lo stile ownerDraw non viene comunque disegnata bene,
    // si distrugge comunque e si ricrea al volo
#ifndef RECREATE_ALWAYS
    if(!isOwnedDraw()) {
#endif
      HWND prev = GetNextWindow(*this, GW_HWNDPREV);

      // se non ha già un titolo, recupera quello del controllo
      if(!getCaption()) {
        TCHAR title[260] = _T("\0");
        GetWindowText(*this, title, SIZE_A(title));
        if(title[0])
          setCaption(title);
        }
//      DWORD val = GetClassLong(*this, GCL_STYLE);
      destroy();
      Attr.x = r.left;
      Attr.y = r.top;
      Attr.w = r.Width();
      Attr.h = r.Height();
      clearFlag(pfFromResource);
      if(PControl::create()) {
/**/
        DWORD val = GetClassLong(*this, GCL_STYLE);
#if 0
        val &= ~(CS_OWNDC | CS_SAVEBITS | CS_PARENTDC);
#else
        val &= ~(CS_OWNDC | CS_SAVEBITS);
        val |= CS_PARENTDC;
//        val &= ~(CS_PARENTDC | CS_SAVEBITS);
//        val |= CS_OWNDC;
#endif
/**/
        SetClassLong(*this, GCL_STYLE, val);
        if(prev)
          SetWindowPos(*this, prev, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return true;
        }
      return false;

#ifndef RECREATE_ALWAYS
      }
    setWindowPos(0, r, SWP_NOZORDER);
/**/
    DWORD val = GetClassLong(*this, GCL_STYLE);
    val &= ~(CS_OWNDC | CS_SAVEBITS);
    val |= CS_PARENTDC;
    SetClassLong(*this, GCL_STYLE, val);
    InvalidateRect(*this, 0, 1);
/**/
#endif
    }
/*
  else if(isFlagSet(pfPredefinite)) {
    DWORD val = GetClassLong(*this, GCL_STYLE);
#if 1
    val |= CS_OWNDC;
    val &= ~(CS_SAVEBITS | CS_PARENTDC);
#else
    val &= ~(CS_OWNDC | CS_SAVEBITS);
    val |= CS_PARENTDC;
#endif
    SetClassLong(*this, GCL_STYLE, val);
    }
*/
  return true;

}
//-----------------------------------------------------------
/*
bool POwnWin::EvPaint(HDC hdc, LPPAINTSTRUCT ps)
{
#if 1
  DRAWITEMSTRUCT dis;
  makeStruct(hdc, &dis, ps);
  return drawItem(Attr.id, &dis);
#else
  return false;
#endif
}
//-----------------------------------------------------------
void POwnWin::makeStruct(HDC hdc, LPDRAWITEMSTRUCT dis, LPPAINTSTRUCT ps)
{
  ZeroMemory(dis, sizeof(*dis));
  dis->CtlID = Attr.id;
  dis->itemID = 0;
  dis->itemAction = ODA_DRAWENTIRE;
  if(GetFocus() == *this) {
    dis->itemState = ODS_FOCUS;
    dis->itemAction |= ODA_FOCUS;
    }
  if(!IsWindowEnabled(*this))
    dis->itemState |= ODS_DISABLED;
  else
    dis->itemState |= ODS_DEFAULT;

  dis->hDC = hdc;
  dis->rcItem = ps->rcPaint;
}
*/
