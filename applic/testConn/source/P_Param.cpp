//---------- p_param.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_param.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include <Shlwapi.h>
#pragma comment(linker, "/defaultlib:Shlwapi.lib")
//----------------------------------------------------------------------------
extern paramConn getDefaultparamCom();
//----------------------------------------------------------------------------
#define MAX_PORT 255
//----------------------------------------------------------------------------
static value_name vBaudRate[] =
{
  { CBR_110,        _T("110")   },
  { CBR_300,        _T("300")   },
  { CBR_600,        _T("600")   },
  { CBR_1200,     _T("1.200")   },
  { CBR_2400,     _T("2.400")   },
  { CBR_4800,     _T("4.800")   },
  { CBR_9600,     _T("9.600")   },
  { CBR_14400,   _T("14.400")   },
  { CBR_19200,   _T("19.200")   },
  { CBR_38400,   _T("38.400")   },
  { CBR_56000,   _T("56.000")   },
  { CBR_57600,   _T("57.600")   },
  { CBR_115200, _T("115.200")   },
  { CBR_128000, _T("128.000")   },
  { CBR_256000, _T("256.000")   },
  { CBR_230400, _T("230.400")   },
  { CBR_460800, _T("460.800")   },
  { CBR_921600, _T("921.600")   },
};
//----------------------------------------------------------------------------
static value_name vParity[] =
{
  { EVENPARITY, _T("EVENPARITY")  },
  { MARKPARITY, _T("MARKPARITY")  },
  { NOPARITY,   _T("NOPARITY")    },
  { ODDPARITY,  _T("ODDPARITY")   },
  { SPACEPARITY,_T("SPACEPARITY") }
};
//----------------------------------------------------------------------------
static value_name vBitStop[] =
{
  { ONESTOPBIT,   _T("ONESTOPBIT")    },
  { ONE5STOPBITS, _T("ONE5STOPBITS")  },
  { TWOSTOPBITS,  _T("TWOSTOPBITS")   }
};
//----------------------------------------------------------------------------
static value_name vBitLen[] =
{
  { 5,  _T("5") },
  { 6,  _T("6") },
  { 7,  _T("7") },
  { 8,  _T("8") },
};
//----------------------------------------------------------------------------
static value_name vDTR_DSR[] =
{
  { 0, _T("NO_DTR_DSR") },
  { 1, _T("ON_DTR_DSR") },
  { 2, _T("HANDSHAKE_DTR_DSR") },
};
//----------------------------------------------------------------------------
static value_name vRTS_CTS[] =
{
  { 0, _T("NO_RTS_CTS") },
  { 1, _T("ON_RTS_CTS") },
  { 2, _T("HANDSHAKE_RTS_CTS") },
  { 3, _T("TOGGLE_RTS_CTS") },
};
//----------------------------------------------------------------------------
static value_name vXON_XOFF[] =
{
  { 0, _T("NO_XON_XOFF") },
  { 1, _T("XON_XOFF") },
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
P_Param::P_Param(setting& par, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Par(par)
{
  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_DEF, IDB_BITMAP_OK, IDB_BITMAP_CANC };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDC_BUTTON_DEFAULT, Bmp[0]);
  new POwnBtnImageStd(this, IDOK, Bmp[1]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[2]);
}
//----------------------------------------------------------------------------
P_Param::~P_Param()
{
  destroy();
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//---------------------------------------------------------
static void findAndSetText(HWND cbx, LPCTSTR txt)
{
  int count = SendMessage(cbx, CB_GETCOUNT, 0, 0);
  TCHAR t[500];
  for(int i = 0; i < count; ++i) {
    SendMessage(cbx, CB_GETLBTEXT, i, (LPARAM)t);
    if(!_tcscmp(t, txt)) {
      SendMessage(cbx, CB_SETCURSEL, i, 0);
      break;
      }
    }
}
//----------------------------------------------------------------------------
#define SET_CBX_(bx, vn, p) \
    { \
    HWND hcbx = ::GetDlgItem(getHandle(), bx); \
    fillCBX(hcbx,vn, SIZE_A(vn));   \
    findAndSetText(hcbx, vn[Par.par.b.p].name); \
    }
//----------------------------------------------------------------------------
#define SET_CBX_2(bx, txt, dim) \
    { \
    HWND hcbx = ::GetDlgItem(getHandle(), bx); \
    int sel = fillCBX(hcbx,txt,dim,Par.par.getPort());   \
    SendMessage(hcbx, CB_SETCURSEL, sel, 0); \
    }
//----------------------------------------------------------------------------
#define SET_PAR_(bx, vn, p) \
    { \
    HWND hcbx = ::GetDlgItem(getHandle(), bx);  \
    Par.par.b.p = static_cast<WORD>(fillPar(hcbx, SIZE_A(vn))); \
    }
//----------------------------------------------------------------------------
#define SET_PORT(bx) \
    { \
    HWND hcbx = ::GetDlgItem(getHandle(), bx);  \
    Par.par.setPort(static_cast<WORD>(getPortNum(hcbx))); \
    }
//----------------------------------------------------------------------------
static int getPortNum(HWND hcbx)
{
  TCHAR txt[50];
  ::GetWindowText(hcbx, txt, SIZE_A(txt));
  LPTSTR p = txt;
  while(*p) {
    if(*p >= '0' && *p <= '9')
      break;
    ++p;
    }
  int port = 0;
  if(*p) {
    port = _ttoi(p) - 1;
    if(port < 0)
      port = 0;
    }
  return port;
}
//----------------------------------------------------------------------------
static int fillCBX(HWND hcbx, LPCTSTR v, int num, int onuse)
{
  ::SendMessage(hcbx, CB_RESETCONTENT, 0, 0);

  TCHAR t[128];
  TCHAR result[409];
  uint curr = 0;
  int sel = -1;
  for(int i = 0; i < num; ++i) {
    wsprintf(t, _T("%s%d"), v, i + 1);
    if(QueryDosDevice(t, result, SIZE_A(result))) {
      if(StrStr(result, _T("USB")))
        _tcscat_s(t, _T(" (USB)"));
      ::SendMessage(hcbx, CB_ADDSTRING, 0, (LPARAM)t);
      if(onuse == i)
        sel = curr;
      ++curr;
      }
    }
  return sel;
}
//----------------------------------------------------------------------------
static int fillPar(HWND hcbx, int num)
{
  TCHAR txt[50];
  ::GetWindowText(hcbx, txt, SIZE_A(txt));
  for(int i=0; i < num; ++i) {
    TCHAR t[50];
    ::SendMessage(hcbx, CB_GETLBTEXT, (WPARAM)i, (LPARAM)t);
    if(!_tcscmp(txt,t))
      return i;
    }
  return 0;
}
//----------------------------------------------------------------------------
#define IS_CHECKED(idc) \
    (BST_CHECKED == SendMessage(::GetDlgItem(*this, idc), BM_GETCHECK, 0, 0))
//----------------------------------------------------------------------------
#define SET_CHECK(idc) \
    SendMessage(::GetDlgItem(*this, idc), BM_SETCHECK, BST_CHECKED, 0)
//----------------------------------------------------------------------------
#define SET_UNCHECK(idc) \
    SendMessage(::GetDlgItem(*this, idc), BM_SETCHECK, BST_UNCHECKED, 0)
//----------------------------------------------------------------------------
void P_Param::BNClickedOk()
{
  SET_PORT(IDC_COMBOBOX_COM_PORT)
//  SET_PAR_(IDC_COMBOBOX_COM_PORT, vPort, port)
  SET_PAR_(IDC_COMBOBOX_BAUDRATE, vBaudRate, baud)
  SET_PAR_(IDC_COMBOBOX_BITSTOP,  vBitStop,  stop)
  SET_PAR_(IDC_COMBOBOX_DATA_LEN,  vBitLen,   len)
  SET_PAR_(IDC_COMBOBOX_PARITY,   vParity,   parity)

  SET_PAR_(IDC_COMBOBOX_DTR_DSR,   vDTR_DSR,   dtr_dsr)
  SET_PAR_(IDC_COMBOBOX_RTS_CTS,   vRTS_CTS,   rts_cts)
  SET_PAR_(IDC_COMBOBOX_XON_XOFF,   vXON_XOFF,   xon_xoff)

  Par.delay = GetDlgItemInt(*this, IDC_EDIT_MS_DELAY, 0, 0);
  Par.packet = GetDlgItemInt(*this, IDC_EDIT_DIM_PACKET, 0, 0);
  if(IS_CHECKED(IDC_CHECKBOX_MODEM))
    Par.connType = 1;
  else if(IS_CHECKED(IDC_CHECKBOX_LAN))
    Par.connType = 2;
  else
    Par.connType = 0;
  Par.proto = IS_CHECKED(IDC_RADIOBUTTON_UDP);
  Par.on485 = IS_CHECKED(IDC_CHECKBOX_485);

  if(Par.on485)
    Par.delay = 0;
  Par.modemType = IS_CHECKED(IDC_RADIOBUTTON_TONE);
  Par.lanType = IS_CHECKED(IDC_RADIOBUTTON_SERVER);
  GET_TEXT(IDC_EDIT_INIT_MODEM, Par.initModem);
}
//----------------------------------------------------------------------------
static void fillCBX(HWND hcbx, value_name *v, int num)
{
  ::SendMessage(hcbx, CB_RESETCONTENT, 0, 0);

  for(int i = 0; i < num; ++i)
    ::SendMessage(hcbx, CB_ADDSTRING, 0, (LPARAM)(v[i].name));
}
//----------------------------------------------------------------------------
bool P_Param::create()
{
  if(!baseClass::create())
    return false;

  fillCtrl();
  SetDlgItemInt(*this, IDC_EDIT_MS_DELAY, Par.delay, 0);
  SetDlgItemInt(*this, IDC_EDIT_DIM_PACKET, Par.packet, 0);
  switch(Par.connType) {
    case 1:
      SET_CHECK(IDC_CHECKBOX_MODEM);
      break;
    case 2:
      SET_CHECK(IDC_CHECKBOX_LAN);
      break;
    }
  if(Par.modemType)
    SET_CHECK(IDC_RADIOBUTTON_TONE);
  else
    SET_CHECK(IDC_RADIOBUTTON_PULSE);

  if(Par.lanType)
    SET_CHECK(IDC_RADIOBUTTON_SERVER);
  else
    SET_CHECK(IDC_RADIOBUTTON_CLIENT);

  if(Par.proto)
    SET_CHECK(IDC_RADIOBUTTON_UDP);
  else
    SET_CHECK(IDC_RADIOBUTTON_TCP);
  SET_CHECK_SET(IDC_CHECKBOX_485, Par.on485);

  checkEnable(0);
  SET_TEXT(IDC_EDIT_INIT_MODEM, Par.initModem);
  if(!Par.connType)
    checkEnableDelay();
  return true;
}
//----------------------------------------------------------------------------
void P_Param::fillCtrl()
{
//  SET_CBX_(IDC_COMBOBOX_COM_PORT, vPort, port)
  SET_CBX_2(IDC_COMBOBOX_COM_PORT, _T("Com"), MAX_PORT)
  SET_CBX_(IDC_COMBOBOX_BAUDRATE, vBaudRate, baud)
  SET_CBX_(IDC_COMBOBOX_BITSTOP,  vBitStop,  stop)
  SET_CBX_(IDC_COMBOBOX_DATA_LEN,  vBitLen,   len)
  SET_CBX_(IDC_COMBOBOX_PARITY,   vParity,   parity)

  SET_CBX_(IDC_COMBOBOX_DTR_DSR,   vDTR_DSR,   dtr_dsr)
  SET_CBX_(IDC_COMBOBOX_RTS_CTS,   vRTS_CTS,   rts_cts)
  SET_CBX_(IDC_COMBOBOX_XON_XOFF,   vXON_XOFF,   xon_xoff)
}
//----------------------------------------------------------------------------
#define ENABLE(idc, set) \
    EnableWindow(::GetDlgItem(*this, idc), set)
//----------------------------------------------------------------------------
void P_Param::checkEnable(int type)
{
  ENABLE(IDC_EDIT_MS_DELAY, true);
  ENABLE(IDC_EDIT_DIM_PACKET, true);
  switch(type) {
    case 0:
      if(IS_CHECKED(IDC_CHECKBOX_MODEM))
        SET_UNCHECK(IDC_CHECKBOX_LAN);
      break;
    case 1:
      if(IS_CHECKED(IDC_CHECKBOX_LAN))
        SET_UNCHECK(IDC_CHECKBOX_MODEM);
      break;
    }
  bool enableModem = IS_CHECKED(IDC_CHECKBOX_MODEM);
  ENABLE(IDC_RADIOBUTTON_TONE, enableModem);
  ENABLE(IDC_RADIOBUTTON_PULSE, enableModem);

  bool enableLan = IS_CHECKED(IDC_CHECKBOX_LAN);
  ENABLE(IDC_RADIOBUTTON_SERVER, enableLan);
  ENABLE(IDC_RADIOBUTTON_CLIENT, enableLan);
  ENABLE(IDC_RADIOBUTTON_UDP, enableLan);
  ENABLE(IDC_RADIOBUTTON_TCP, enableLan);

  bool enable = !(enableModem || enableLan);
  ENABLE(IDC_COMBOBOX_BAUDRATE, enable);
  ENABLE(IDC_COMBOBOX_BITSTOP, enable);
  ENABLE(IDC_COMBOBOX_DATA_LEN, enable);
  ENABLE(IDC_COMBOBOX_PARITY, enable);
  ENABLE(IDC_COMBOBOX_DTR_DSR, enable);
  ENABLE(IDC_COMBOBOX_RTS_CTS, enable);
  ENABLE(IDC_COMBOBOX_XON_XOFF, enable);
  ENABLE(IDC_CHECKBOX_485, enable);

  ENABLE(IDC_BUTTON_DEFAULT, enable);
  ENABLE(IDC_COMBOBOX_COM_PORT, !enableLan);
  if(enable)
    checkEnableDelay();
}
//----------------------------------------------------------------------------
void P_Param::checkEnableDelay()
{
  bool enable = !IS_CHECKED(IDC_CHECKBOX_485);
  ENABLE(IDC_EDIT_MS_DELAY, enable);
  ENABLE(IDC_COMBOBOX_RTS_CTS, enable);
}
//----------------------------------------------------------------------------
LRESULT P_Param::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_DEFAULT:
          Par.par = getDefaultparamCom();
          fillCtrl();
          break;
        case IDC_CHECKBOX_MODEM:
          checkEnable(0);
          break;
        case IDC_CHECKBOX_LAN:
          checkEnable(1);
          break;

        case IDC_CHECKBOX_485:
          checkEnableDelay();
          break;

        case IDOK:
          BNClickedOk();
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
