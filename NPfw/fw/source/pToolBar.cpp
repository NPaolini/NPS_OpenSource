//-------------------- pToolBar.cpp -------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//-----------------------------------------------------------
#include "pToolBar.h"
#include "PRect.h"
#include "PEdit.h"
//#include "resource.h"
//-----------------------------------------------------------
#ifndef TBSTYLE_FLAT
// il borland non è aggiornato sugli ultimi stili
  #define TBSTYLE_LIST        0x1000
  #define TBSTYLE_FLAT        0x0800
#endif

#define TB_STYLE \
      TBSTYLE_WRAPABLE | \
      TBSTYLE_TOOLTIPS | \
      TBSTYLE_FLAT

//      TBSTYLE_TRANSPARENT |\
//-----------------------------------------------------------
HWND PToolBar::performCreate()
{
  HINSTANCE hInst = getHInstance();
  UINT_PTR idBmp = Info->idBmp;
  if(!idBmp) {
    idBmp = (UINT_PTR)Info->hBitmap;
    hInst = 0;
    }
  HWND hWndToolBar = CreateToolbarEx(*getParent(),
        WS_CHILD | TB_STYLE  | WS_CLIPCHILDREN |
        WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | WS_VISIBLE,
        Info->idToolBar,
        Info->nButtons,
        hInst,
        idBmp,
        Info->pButtons,
        Info->nButtons,
        Info->szBtn.cx, Info->szBtn.cy,
        Info->szBmp.cx, Info->szBmp.cy,
        sizeof(TBBUTTON));
  if(hWndToolBar) {
    HDC hdc = GetDC(*this);
    RealizePalette(hdc);
    ReleaseDC(*this, hdc);

    if(Info->startSelection >= 0) {
      TBBUTTON *tbb = &Info->pButtons[Info->startSelection];
      if(TBSTYLE_CHECKGROUP == tbb->fsStyle)
        SendMessage(hWndToolBar, TB_CHECKBUTTON, tbb->idCommand, (LPARAM) MAKELONG(true, 0));
      }
    RECT rect;
    SendMessage(hWndToolBar, TB_GETITEMRECT, 0, (LPARAM)&rect);
    PRect pr(rect);
    pr.MoveTo(0, pr.top);
#if 1
    pr.right *= Info->nButtons / Info->nRow + (Info->nButtons % Info->nRow ? 1 : 0);
#else
    pr.right *= Info->nButtons / Info->nRow;
#endif
    pr.bottom *= Info->nRow;
    SetWindowPos(hWndToolBar, 0, 0, 0, pr.right, pr.bottom, SWP_NOZORDER);
    RECT rAll;
    GetClientRect(*getParent(), &rAll);
    rAll.right -= pr.right;
    rAll.bottom -= pr.bottom;
    GetWindowRect(*getParent(), &rect);
    rect.right -= rAll.right;
    rect.bottom -= rAll.bottom;
    PRect rThis(rect);
    SetWindowPos(*getParent(), 0, 0, 0, rThis.Width(), rThis.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
    }
  return hWndToolBar;
}
//-----------------------------------------------------------
LPCTSTR PToolBar::getTips(int idCommand)
{
  if(Info->tips) {
    for(uint i = 0; i < Info->nButtons; ++i)
      if(Info->pButtons[i].idCommand == idCommand)
        return Info->tips[i];
    }
  return 0;
}
//-----------------------------------------------------------
void PToolBar::enableBtn(int id, bool enable)
{
  SendMessage(*this, TB_ENABLEBUTTON,
          Info->pButtons[id].idCommand, (LPARAM) MAKELONG(enable, 0));
}
//-----------------------------------------------------------
//#if (_WIN32_IE >= 0x0400)
#define TB_MARKBUTTON           (WM_USER + 6)
#define TB_ISBUTTONHIGHLIGHTED  (WM_USER + 14)
#define TB_SETANCHORHIGHLIGHT   (WM_USER + 73)
#define TB_SETHOTITEM           (WM_USER + 72)
//#endif
void PToolBar::selectBtn(int id, bool sel)
{
//  SendMessage(*this, TB_SETHOTITEM, id, 0);

//  SendMessage(*this, TB_PRESSBUTTON, Info->pButtons[id].idCommand,
//        (LPARAM) MAKELONG(true, 0));

  SendMessage(*this, TB_CHECKBUTTON, Info->pButtons[id].idCommand,
        (LPARAM) MAKELONG(sel, 0));
}
//-----------------------------------------------------------
void PToolBar::setHotItem(int id)
{
  SendMessage(*this, TB_SETHOTITEM, id, 0);

}
//-----------------------------------------------------------
/*
LRESULT PToolBar::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      MessageBox(0, "Destroy", 0, MB_OK);
      break;
    }
  return PWin::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
*/
