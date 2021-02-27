//-------- P_Status.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "P_Status.h"
#include "macro_utils.h"
//----------------------------------------------------------------------------
void infoStatus::clear()
{
  CTS_ON = 0;
  DSR_ON = 0;
  RING_ON = 0;
  RLSD_ON = 0;
  CTS_W = 0;
  DSR_W = 0;
  RLSD_W = 0;
  XOFF_R = 0;
  XOFF_T = 0;
  InQueue = 0;
  OutQueue = 0;
}
//----------------------------------------------------------------------------
void infoStatus::refresh(PConnBase* conn)
{
  clear();
  HANDLE h = (HANDLE)conn->passThrough(cGetHandle, 0);
  if(!h)
    return;
  DWORD ms;
  if(GetCommModemStatus(h, &ms)) {
    CTS_ON = toBool(ms & MS_CTS_ON);
    DSR_ON = toBool(ms & MS_DSR_ON);
    RING_ON = toBool(ms & MS_RING_ON);
    RLSD_ON = toBool(ms & MS_RLSD_ON);
    }
  COMSTAT cstat;
  ClearCommError(h, &ms, &cstat);
  CTS_W = cstat.fCtsHold;
  DSR_W = cstat.fDsrHold;
  RLSD_W = cstat.fRlsdHold;
  XOFF_R = cstat.fXoffHold;
  XOFF_T = cstat.fXoffSent;
  InQueue = cstat.cbInQue;
  OutQueue = cstat.cbOutQue;
}
//----------------------------------------------------------------------------
#define CRED RGB(0xff, 0x00, 0x00)
#define CGREEN RGB(0x00, 0xff, 0x00)
//----------------------------------------------------------------------------
P_Status::P_Status(PWin* parent, PConnBase* conn, uint resId, HINSTANCE hinstance) :
        baseClass(parent, resId, hinstance), Conn(conn),
        hbRed(CreateSolidBrush(CRED)), hbGreen(CreateSolidBrush(CGREEN)),
        idTimer(0)
{}
//----------------------------------------------------------------------------
P_Status::~P_Status()
{
  destroy();
  DeleteObject(hbRed);
  DeleteObject(hbGreen);
}
//----------------------------------------------------------------------------
static bool checkedRTS_high = true;
static bool checkedDTR_high = true;
static bool autoRefresh = true;
static DWORD autoRefreshMSec = 1000;
//----------------------------------------------------------------------------
bool P_Status::create()
{
  if(!baseClass::create())
    return false;
  CmOk();
  if(checkedRTS_high)
    SET_CHECK(IDC_RADIO_RTS_ON);
  else
    SET_CHECK(IDC_RADIO_RTS_OFF);
  if(checkedDTR_high)
    SET_CHECK(IDC_RADIO_DTR_ON);
  else
    SET_CHECK(IDC_RADIO_DTR_OFF);
  SET_CHECK_SET(IDC_CHECK_AUTO_REFRESH, autoRefresh);
  if(autoRefreshMSec < 300)
    autoRefreshMSec = 300;
  SET_INT(IDC_EDIT_AUTO_REFRESH, autoRefreshMSec);
  if(autoRefresh)
    SetTimer(*this, idTimer = 11, autoRefreshMSec, 0);
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_Status::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      break;
    case WM_TIMER:
      if(11 == wParam)
        CmOk();
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RTS_FORCE:
          sendRTS();
          break;
        case IDC_DTR_FORCE:
          sendDTR();
          break;
        case IDC_RADIO_RTS_ON:
          checkedRTS_high = true;
          break;
        case IDC_RADIO_RTS_OFF:
          checkedRTS_high = false;
          break;
        case IDC_RADIO_DTR_ON:
          checkedDTR_high = true;
          break;
        case IDC_RADIO_DTR_OFF:
          checkedDTR_high = false;
          break;
        case IDC_CHECK_AUTO_REFRESH:
          if(IS_CHECKED(IDC_CHECK_AUTO_REFRESH))
            SetTimer(*this, idTimer = 11, autoRefreshMSec, 0);
          else if(idTimer) {
            KillTimer(*this, idTimer);
            idTimer = 0;
            }
          break;
        case IDC_EDIT_AUTO_REFRESH:
          if(EN_KILLFOCUS == HIWORD(wParam)) {
            DWORD v;
            GET_INT(IDC_EDIT_AUTO_REFRESH, v);
            if(v < 300) {
              v = 300;
              SET_INT(IDC_EDIT_AUTO_REFRESH, v);
              }
            }
          break;
        }
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HBRUSH P_Status::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  switch(id) {
    case IDC_STATIC_CTS_ON:
      if(Par.CTS_ON) {
        SetBkColor(hdc, CGREEN);
        return (hbGreen);
        }
      break;
    case IDC_STATIC_DSR_ON:
      if(Par.DSR_ON) {
        SetBkColor(hdc, CGREEN);
        return (hbGreen);
        }
      break;
    case IDC_STATIC_RING_ON:
      if(Par.RING_ON) {
        SetBkColor(hdc, CGREEN);
        return (hbGreen);
        }
      break;
    case IDC_STATIC_RLSD_ON:
      if(Par.RLSD_ON) {
        SetBkColor(hdc, CGREEN);
        return (hbGreen);
        }
      break;
    case IDC_STATIC_CTS_W:
      if(Par.CTS_W) {
        SetBkColor(hdc, CRED);
        SetTextColor(hdc, RGB(0xff, 0xff, 0x00));
        return (hbRed);
        }
      break;
    case IDC_STATIC_DSR_W:
      if(Par.DSR_W) {
        SetBkColor(hdc, CRED);
        SetTextColor(hdc, RGB(0xff, 0xff, 0x00));
        return (hbRed);
        }
      break;
    case IDC_STATIC_RLSD_W:
      if(Par.RLSD_W) {
        SetBkColor(hdc, CRED);
        SetTextColor(hdc, RGB(0xff, 0xff, 0x00));
        return (hbRed);
        }
      break;
    case IDC_STATIC_XOFF_R:
      if(Par.XOFF_R) {
        SetBkColor(hdc, CRED);
        SetTextColor(hdc, RGB(0xff, 0xff, 0x00));
        return (hbRed);
        }
      break;
    case IDC_STATIC_XOFF_T:
      if(Par.XOFF_T) {
        SetBkColor(hdc, CRED);
        SetTextColor(hdc, RGB(0xff, 0xff, 0x00));
        return (hbRed);
        }
      break;
    case IDC_STATIC_Q_IN:
      if(Par.InQueue) {
        SetBkColor(hdc, CGREEN);
        return (hbGreen);
        }
      break;
    case IDC_STATIC_Q_OUT:
      if(Par.OutQueue) {
        SetBkColor(hdc, CGREEN);
        return (hbGreen);
        }
      break;
    }
  return 0;
}
//----------------------------------------------------------------------------
void P_Status::CmOk()
{
  Par.refresh(Conn);
  InvalidateRect(*this, 0, false);
//  InvalidateRect(*this, 0, true);
};
//----------------------------------------------------------------------------
void P_Status::sendRTS()
{
  HANDLE h = (HANDLE)Conn->passThrough(cGetHandle, 0);
  if(!h)
    return;
  if(IS_CHECKED(IDC_RADIO_RTS_ON))
    EscapeCommFunction(h, SETRTS);
  else
    EscapeCommFunction(h, CLRRTS);
}
//----------------------------------------------------------------------------
void P_Status::sendDTR()
{
  HANDLE h = (HANDLE)Conn->passThrough(cGetHandle, 0);
  if(!h)
    return;
  if(IS_CHECKED(IDC_RADIO_DTR_ON))
    EscapeCommFunction(h, SETDTR);
  else
    EscapeCommFunction(h, CLRDTR);
}
//----------------------------------------------------------------------------

