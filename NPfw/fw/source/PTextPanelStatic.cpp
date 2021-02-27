//-------------------- PTextPanelStatic.cpp ------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "PTextPanelStatic.h"
//-----------------------------------------------------------
bool PTextPanelStatic::create()
{
  if(!baseClass::create())
    return false;
  PRect r;
  GetWindowRect(*this, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
  Text = new PTextFixedPanel(0, r, getFont(), RGB(0, 0, 0), PPanel::BORDER_FILL, RGB(0xca,0xca,0xff));
#if 0
// esempio creazione sfondo e bordi
  PPanelBorder::pbInfo info;
  info.style = PPanelBorder::pbOn;
  info.tickness = 2;
  info.color = RGB(100, 100, 255);
  Text->setBorder(PPanelBorder::pbTop, info);
  info.color = RGB(255, 100, 100);
  Text->setBorder(PPanelBorder::pbBottom, info);
  Text->setAlign(TA_CENTER); // TA_LEFT | TA_CENTER | TA_RIGHT
  Text->setVAlign(DT_VCENTER); // DT_TOP | DT_VCENTER | DT_BOTTOM
#endif

  return true;
}
//-----------------------------------------------------------
LRESULT PTextPanelStatic::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if(!hdc) {
          EndPaint(hwnd, &ps);
          break;
          }
        if(Text) {
          TCHAR buff[1024];
          GetWindowText(*this, buff, SIZE_A(buff));
          Text->drawText(hdc, buff);
          }
        EndPaint(hwnd, &ps);

        } while(false);
      return !Text;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
