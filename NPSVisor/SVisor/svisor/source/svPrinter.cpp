//--------------- svPrinter.cpp ----------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "svPrinter.h"
#include "id_btn.h"
//-----------------------------------------------------------------------------
LRESULT svPreviewPage::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(wParam) {
        case IDC_BUTTON_F1:
          postAndSel(IDM_DONE);
          break;
        case IDC_BUTTON_F2:
          postAndSel(IDM_PREVIOUS);
          break;
        case IDC_BUTTON_F3:
          postAndSel(IDM_NEXT);
          break;
        case IDC_BUTTON_F4:
          postAndSel(IDM_ZOOM_IN);
          break;
        case IDC_BUTTON_F5:
          postAndSel(IDM_ZOOM_OUT);
          break;
        case IDC_BUTTON_F6:
          postAndSel(IDM_PRINT);
          break;
        default:
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------------------------
static
LPCTSTR checkUseAltBitmap()
{
  LPCTSTR p = getString(ID_USE_ALTERNATE_BITMAP_4_TOOL);
  if(p)
    return p;
  return 0;
}
//-----------------------------------------------------------------------------
PPreviewPage* svPrintPage::allocPreviewPage(PWin* parent, PPrintPage* page, bool& print, LPCTSTR alternateBitmapOrID4Tool)
{
  if(!alternateBitmapOrID4Tool)
    alternateBitmapOrID4Tool = checkUseAltBitmap();
  return new svPreviewPage(parent, page, print, alternateBitmapOrID4Tool);
}
//-----------------------------------------------------------------------------
PPreviewPage* svPrintFile::allocPreviewPage(PWin* parent, PPrintPage* page, bool& print, LPCTSTR alternateBitmapOrID4Tool)
{
  if(!alternateBitmapOrID4Tool)
    alternateBitmapOrID4Tool = checkUseAltBitmap();
  return new svPreviewPage(parent, page, print, alternateBitmapOrID4Tool);
}
