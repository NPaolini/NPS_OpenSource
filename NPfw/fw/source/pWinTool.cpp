//-------------------- pWinTool.cpp -------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//-----------------------------------------------------------
//#include "resource.h"
#include "pWinTool.h"
#include "pToolBar.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PWinTools::PWinTools(PWin * parent, PToolBarInfo *info, LPCTSTR title, UINT idBkg, HINSTANCE hInstance) :
      baseClass(parent, title, idBkg, hInstance)
{
  tools = new PToolBar(this, info, hInstance);
}
//-----------------------------------------------------------
PWinTools::~PWinTools()
{
  destroy();
}
//-----------------------------------------------------------
LPTSTR PWinTools::getTips(uint id)
{
  return const_cast<LPTSTR>(tools->getTips(id));
}
//-----------------------------------------------------------
LRESULT PWinTools::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      PostMessage(*getParent(), WM_C_CHANGED_SELECTION, LOWORD(wParam), 0);
      break;

    case WM_NOTIFY:
      switch (((LPNMHDR)lParam)->code) {
        case TTN_NEEDTEXT:
          {
          LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT)lParam;
          lpToolTipText->lpszText = getTips((uint)lpToolTipText->hdr.idFrom);
//          lpToolTipText->lpszText = const_cast<LPTSTR>(tools->getTips(lpToolTipText->hdr.idFrom));
          break;
          }
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void PWinTools::enableBtn(int id, bool enable)
{
  tools->enableBtn(id, enable);
}
//-----------------------------------------------------------
void PWinTools::selectBtn(int id, bool sel)
{
  tools->selectBtn(id, sel);
}
//-----------------------------------------------------------
void PWinTools::setHotItem(int id)
{
  tools->setHotItem(id);
}
//-----------------------------------------------------------
