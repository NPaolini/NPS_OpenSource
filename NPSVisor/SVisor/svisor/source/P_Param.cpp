//---------- p_param.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_param.h"
#include "config.h"


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
  { CBR_256000, _T("256.000")   }
};
static value_name vParity[] =
{
  { EVENPARITY, _T("EVENPARITY")  },
  { MARKPARITY, _T("MARKPARITY")  },
  { NOPARITY,   _T("NOPARITY")    },
  { ODDPARITY,  _T("ODDPARITY")   },
  { SPACEPARITY,_T("SPACEPARITY") }
};
static value_name vBitStop[] =
{
  { ONESTOPBIT,   _T("ONESTOPBIT")    },
  { ONE5STOPBITS, _T("ONE5STOPBITS")  },
  { TWOSTOPBITS,  _T("TWOSTOPBITS")   }
};

static value_name vBitLen[] =
{
  { 5,  _T("5") },
  { 6,  _T("6") },
  { 7,  _T("7") },
  { 8,  _T("8") },
};

static value_name vDTR_DSR[] =
{
  { 0, _T("NO_DTR_DSR") },
  { 1, _T("ON_DTR_DSR") },
  { 2, _T("HANDSHAKE_DTR_DSR") },
};

static value_name vRTS_CTS[] =
{
  { 0, _T("NO_RTS_CTS") },
  { 1, _T("ON_RTS_CTS") },
  { 2, _T("HANDSHAKE_RTS_CTS") },
  { 3, _T("TOGGLE_RTS_CTS") },
};

static value_name vXON_XOFF[] =
{
  { 0, _T("NO_XON_XOFF") },
  { 1, _T("XON_XOFF") },
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
P_Param::P_Param(paramCom& par, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Par(par)
{
}
//----------------------------------------------------------------------------
P_Param::~P_Param()
{
  destroy();
}
//----------------------------------------------------------------------------
extern void findAndSetText(HWND cbx, LPCTSTR txt);

#define SET_CBX_(bx, vn, p) \
    { \
    HWND hcbx = ::GetDlgItem(getHandle(), bx); \
    fillCBX(hcbx,vn, SIZE_A(vn));   \
    findAndSetText(hcbx, vn[Par.b.p].name); \
    }

//     ::SetWindowText(hcbx, vn[Par.b.p].name);

//----------------------------------------------------------------------------
#define SET_PAR_(bx, vn, p) \
    { \
    HWND hcbx = ::GetDlgItem(getHandle(), bx);  \
    Par.b.p = static_cast<WORD>(fillPar(hcbx, SIZE_A(vn))); \
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
void P_Param::BNClickedOk()
{
  SET_PAR_(IDC_COMBOBOX_BAUDRATE, vBaudRate, baud)
  SET_PAR_(IDC_COMBOBOX_BITSTOP,  vBitStop,  stop)
  SET_PAR_(IDC_COMBOBOX_DATA_LEN,  vBitLen,   len)
  SET_PAR_(IDC_COMBOBOX_PARITY,   vParity,   parity)

  SET_PAR_(IDC_COMBOBOX_DTR_DSR,   vDTR_DSR,   dtr_dsr)
  SET_PAR_(IDC_COMBOBOX_RTS_CTS,   vRTS_CTS,   rts_cts)
  SET_PAR_(IDC_COMBOBOX_XON_XOFF,   vXON_XOFF,   xon_xoff)
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

  SET_CBX_(IDC_COMBOBOX_BAUDRATE, vBaudRate, baud)
  SET_CBX_(IDC_COMBOBOX_BITSTOP,  vBitStop,  stop)
  SET_CBX_(IDC_COMBOBOX_DATA_LEN,  vBitLen,   len)
  SET_CBX_(IDC_COMBOBOX_PARITY,   vParity,   parity)

  SET_CBX_(IDC_COMBOBOX_DTR_DSR,   vDTR_DSR,   dtr_dsr)
  SET_CBX_(IDC_COMBOBOX_RTS_CTS,   vRTS_CTS,   rts_cts)
  SET_CBX_(IDC_COMBOBOX_XON_XOFF,   vXON_XOFF,   xon_xoff)
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_Param::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
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
