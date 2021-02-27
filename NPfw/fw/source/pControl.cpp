//-------------------- pControl.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pControl.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PControl::~PControl()
{
  if(hFont && deleteFont)
    DeleteObject(hFont);
}
//-----------------------------------------------------------
void PControl::setFont(HFONT font, bool autoDelete)
{
  if(hFont && deleteFont)
    DeleteObject(hFont);
  hFont = font;
  deleteFont = autoDelete;
  if(hFont && getHandle())
    SendMessage(getHandle(), WM_SETFONT, (WPARAM)hFont, isFlagSet(pfFromResource));
}
//-----------------------------------------------------------
bool PControl::create()
{
  if(!baseClass::create())
    return false;
  if(hFont)
    SendMessage(getHandle(), WM_SETFONT, (WPARAM)hFont, isFlagSet(pfFromResource));
  return true;
}
//-----------------------------------------------------------
SIZE PControl::getSizeFont()
{
  SIZE sz = { 0, 0 };
  if(!getHandle())
    return sz;

  HDC hdc = GetDC(getHandle());

  HGDIOBJ old;
  if(hFont)
    old = SelectObject(hdc, hFont);

  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);
  if(hFont)
    SelectObject(hdc, old);

  ReleaseDC(getHandle(), hdc);
#define BASE_MUL 50.0
  sz.cx = (LONG)(((tm.tmAveCharWidth * BASE_MUL + tm.tmMaxCharWidth) / (BASE_MUL + 1) + 0.5) * SIMUL_DEC_FONT);
  sz.cy = tm.tmHeight * SIMUL_DEC_FONT;

  return sz;
}
//-----------------------------------------------------------
LRESULT PControl::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static int key;
  if(WM_GETDLGCODE == message) {
    if(-1 != key) {
      key = verifyKey();
      if(key) {
//        if(VK_RETURN != key)
//          up_click(key);
        key = -1;
        return DLGC_WANTALLKEYS;
        }
      }
    }
  else {
    key = 0;
  switch(message) {
#if 0
    case WM_GETDLGCODE:

      // se si controlla se è solo una richiesta si può eliminare up_click()
      // ma poi non funziona come dovrebbe
//      if(lParam) // se lParam == NULL è una query
//        break;

/*    // esempio di verifica tasto premuto
      if(GetKeyState(VK_RETURN)& 0x8000)
        key = VK_RETURN;
      else if(GetKeyState(VK_DOWN)& 0x8000)
        key = VK_DOWN;
      else if(GetKeyState(VK_UP)& 0x8000)
        key = VK_UP;
      else
*/
      key = verifyKey();
      if(key) {
       // simula il rilascio del pulsante affinche' nel prossimo ingresso,
       // trovando ancora il tasto premuto, non torni ancora DLGC_WANTALLKEYS
        up_click(key);
        return DLGC_WANTALLKEYS;
        }
      break;
#endif
    case WM_KEYDOWN:
      if(evKeyDown(wParam))
        return 0;
      break;

    case WM_KEYUP:
      if(evKeyUp(wParam))
        return 0;
      break;

    case WM_CHAR:
      if(evChar(wParam))
        return 0;
      break;
    }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
#define ADD_SCAN(a) ((a) | KEYEVENTF_EXTENDEDKEY)
//-----------------------------------------------------------
// simula la pressione e rilascio del tasto [virtual_key]
void click(WPARAM virtual_key)
{
    BYTE scan = static_cast<BYTE>(MapVirtualKey(virtual_key, 0));
    keybd_event(static_cast<BYTE>(virtual_key), scan, ADD_SCAN(0), 0);
    Sleep(0);
    keybd_event(static_cast<BYTE>(virtual_key), scan, ADD_SCAN(KEYEVENTF_KEYUP), 0);
}
//-----------------------------------------------------------
// simula la pressione e rilascio del tasto [virtual_key]
void down_click(WPARAM virtual_key)
{
    BYTE scan = static_cast<BYTE>(MapVirtualKey(virtual_key, 0));
    keybd_event(static_cast<BYTE>(virtual_key), scan, ADD_SCAN(0), 0);
}
//-----------------------------------------------------------
// simula il rilascio del tasto [virtual_key]
void up_click(WPARAM virtual_key)
{
    BYTE scan = static_cast<BYTE>(MapVirtualKey(virtual_key, 0));
    keybd_event(static_cast<BYTE>(virtual_key), scan, ADD_SCAN(KEYEVENTF_KEYUP), 0);
}
//-----------------------------------------------------------
// simula la pressione e rilascio del tasto [virtual_key] premuto
// insieme ad un tasto modificatore (shift, alt, ctrl)
#if 0
void special_click(WPARAM virtual_key, UINT special_key, int repeat)
{
  down_click(special_key);
//  Sleep(0);

  do {
    click(virtual_key);
//    Sleep(0);
    } while(repeat-- > 0);

  up_click(special_key);
}
#else
void special_click(WPARAM virtual_key, WPARAM special_key, int repeat)
{
    BYTE scan = static_cast<BYTE>(MapVirtualKey(virtual_key, 0));
    BYTE scan_sp =static_cast<BYTE>(MapVirtualKey(special_key, 0));

    keybd_event(static_cast<BYTE>(special_key), 0, ADD_SCAN(0), 0);
//    keybd_event(static_cast<BYTE>(special_key), scan_sp, ADD_SCAN(0), 0);

    do {
      // preme e rilascia per (repeat + 1) volte
      keybd_event(static_cast<BYTE>(virtual_key), scan, ADD_SCAN(0), 0);
      keybd_event(static_cast<BYTE>(virtual_key), scan, ADD_SCAN(KEYEVENTF_KEYUP), 0);

      } while(repeat-- > 0);

    keybd_event(static_cast<BYTE>(special_key), 0, ADD_SCAN(KEYEVENTF_KEYUP), 0);
//    keybd_event(static_cast<BYTE>(special_key), scan_sp, ADD_SCAN(KEYEVENTF_KEYUP), 0);
}
#endif
//-----------------------------------------------------------
// simula la pressione e rilascio del tasto [virtual_key] premuto
// insieme al tasto shift
void shift_click(WPARAM virtual_key)
{
    special_click(virtual_key,VK_SHIFT);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
