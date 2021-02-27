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
#define MAX_PORT 99
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
#define IS_CHECKED(idc) \
    (BST_CHECKED == SendMessage(::GetDlgItem(*this, idc), BM_GETCHECK, 0, 0))
//----------------------------------------------------------------------------
#define SET_CHECK(idc) \
    SendMessage(::GetDlgItem(*this, idc), BM_SETCHECK, BST_CHECKED, 0)
//----------------------------------------------------------------------------
#define SET_UNCHECK(idc) \
    SendMessage(::GetDlgItem(*this, idc), BM_SETCHECK, BST_UNCHECKED, 0)
//----------------------------------------------------------------------------
void P_Param::codeToCtrl()
{
  TCHAR t[32];
  for (uint i = 0; i < DIM_HEADER; ++i)
    t[i] = Par.Code[i];
  t[DIM_HEADER] = 0;
  SET_TEXT(IDC_EDIT_HEADER, t);
  SET_TEXT(IDC_EDIT_CODE, _T("0000000000"));

  for (uint i = 0; i < DIM_VER; ++i)
    t[i] = Par.Code[OFFS_VER + i];
  t[DIM_HEADER] = 0;
  SET_TEXT(IDC_EDIT_VER, t);
}
//----------------------------------------------------------------------------
void P_Param::codeFromCtrl()
{
  TCHAR t[32];
  GET_TEXT(IDC_EDIT_HEADER, t);
  for (uint i = 0; i < DIM_HEADER; ++i)
    Par.Code[i] = t[i];
  for(uint i = 0; i < DIM_CODE; ++i)
    Par.Code[OFFS_CODE + i] = _T('0');
  GET_TEXT(IDC_EDIT_VER, t);
  for (uint i = 0; i < DIM_VER; ++i)
    Par.Code[OFFS_VER + i] = t[i];

  Par.Code[DIM_FULL_CODE] = 0;
}
//----------------------------------------------------------------------------
void P_Param::BNClickedOk()
{
  SET_PORT(IDC_COMBOBOX_COM_PORT)
  SET_PAR_(IDC_COMBOBOX_BAUDRATE, vBaudRate, baud)
  SET_PAR_(IDC_COMBOBOX_BITSTOP,  vBitStop,  stop)
  SET_PAR_(IDC_COMBOBOX_DATA_LEN,  vBitLen,   len)
  SET_PAR_(IDC_COMBOBOX_PARITY,   vParity,   parity)

  SET_PAR_(IDC_COMBOBOX_DTR_DSR,   vDTR_DSR,   dtr_dsr)
  SET_PAR_(IDC_COMBOBOX_RTS_CTS,   vRTS_CTS,   rts_cts)
  SET_PAR_(IDC_COMBOBOX_XON_XOFF,   vXON_XOFF,   xon_xoff)

  codeFromCtrl();
  GET_INT(IDC_EDIT_DELAY, Par.delay);
  GET_INT(IDC_EDIT_PACKET, Par.packet);
  Par.useWiFi = IS_CHECKED(IDC_CHECK_WIFI);
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
  return true;
}
//----------------------------------------------------------------------------
void P_Param::fillCtrl()
{
  SET_CBX_2(IDC_COMBOBOX_COM_PORT, _T("Com"), MAX_PORT)
  SET_CBX_(IDC_COMBOBOX_BAUDRATE, vBaudRate, baud)
  SET_CBX_(IDC_COMBOBOX_BITSTOP,  vBitStop,  stop)
  SET_CBX_(IDC_COMBOBOX_DATA_LEN,  vBitLen,   len)
  SET_CBX_(IDC_COMBOBOX_PARITY,   vParity,   parity)

  SET_CBX_(IDC_COMBOBOX_DTR_DSR,   vDTR_DSR,   dtr_dsr)
  SET_CBX_(IDC_COMBOBOX_RTS_CTS,   vRTS_CTS,   rts_cts)
  SET_CBX_(IDC_COMBOBOX_XON_XOFF,   vXON_XOFF,   xon_xoff)
  codeToCtrl();
  SET_INT(IDC_EDIT_DELAY, Par.delay);
  SET_INT(IDC_EDIT_PACKET, Par.packet);
  SET_CHECK_SET(IDC_CHECK_WIFI, Par.useWiFi);
}
//----------------------------------------------------------------------------
LRESULT P_Param::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_DEFAULT:
          Par.par = getDefaultparamCom();
          _tcscpy_s(Par.Code, MY_CODE_RFID);
          fillCtrl();
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
connSend::connSend(PConnBase* conn, progressBar* pbar) : Conn(conn), pBar(pbar), delay(0), packet(0)
{
  setting s;
  fillSetting(s);
  delay = s.delay;
  packet = s.packet;
}
//----------------------------------------------------------------------------
void sendDim(PConnBase* conn, int dim)
{
  char t[32];
  wsprintfA(t, "%d#", dim);
  conn->write_string((LPCBYTE)t, strlen(t));
}
//----------------------------------------------------------------------------
bool connSend::sendWaitAck(LPVOID buff, int len)
{
  LPCBYTE p = (LPCBYTE)buff;
  while(len > 0) {
    int pck = min(len, DIM_WAIT_ACK);
    sendDim(Conn, pck);
    if(Conn->write_string(p, pck) != pck)
      return false;
    if(pBar) {
      if(!pBar->addCount(pck))
        return false;
      while(true) {
        if(!getAppl()->pumpMessages())
          break;
        }
      }
    if(!connFindMatch(Conn, ACK_, strlen(ACK_)))
      return false;
    len -= pck;
    p += pck;
    }
  return true;
}
//----------------------------------------------------------------------------
bool connSend::send(LPVOID buff, int len)
{
  LPCBYTE p = (LPCBYTE)buff;
  int pck = len;
  if(packet) {
    pck = min(packet, len);
    int delay2 = delay;
    if(!delay2)
      delay2 = 50;
    while(len > 0) {
      if(Conn->write_string(p, pck) != pck)
        return false;
      if(pBar) {
        if(!pBar->addCount(pck))
          return false;
        while(true) {
          if(!getAppl()->pumpMessages())
            break;
          }
        }
      len -= pck;
      p += pck;
      pck = min(packet, len);

      Sleep(delay2);
      }
    }
  else if(delay) {
    while(len > 0) {
      for(int i = 0; i < pck; ++i) {
        if(!Conn->write_string(p + i, 1))
          return false;
        if(pBar) {
          if(!pBar->addCount(pck))
            return false;
          while(true) {
            if(!getAppl()->pumpMessages())
              break;
            }
          }
        Sleep(delay);
        }
      }
    }
  else
    if(Conn->write_string(p, pck) != pck)
      return false;
  return true;
}
//----------------------------------------------------------------------------
//-------------------------------------------------------------------
bool readUntil(PConnBase* conn, char c)
{
  DWORD count = 0;
  while(conn->isConnected()) {
    if(conn->has_string()) {
      BYTE t;
      conn->read_string(&t, 1);
      if(c == t)
        return true;
      }
  else {
    if (100 <= ++count)
    break;
    Sleep(10);
    }
    }
  return false;
}
//-------------------------------------------------------------------
DWORD storeUntil(PConnBase* conn, char c, LPSTR buff, DWORD len)
{
  DWORD count = 0;
  DWORD pos = 0;
  while(conn->isConnected()) {
    if(conn->has_string()) {
      BYTE t;
      conn->read_string(&t, 1);
      if(c == t)
        return pos;
      buff[pos++] = t;
      if(pos == len)
        return pos;
      }
  else {
    if(100 <= ++count)
      break;
    Sleep(10);
    }
  }
  return 0;
}
//----------------------------------------------------------------------------
int checkPos(LPCSTR buff, LPCSTR m, int dim)
{
  consumer cons(buff, strlen(buff));
  int found = cons.checkMatch(m, dim);
  if (found >= 0)
    found += dim;
  return found;
}
//-------------------------------------------------------------------
bool consumerConn::has_data()
{
  for (uint i = 0; i < 100; ++i) {
    if (Conn->has_string())
      return true;
    Sleep(10);
    }
  return false;
}
//-------------------------------------------------------------------
void consumerConn::read_one_data(BYTE& buff) { Conn->read_string(&buff, 1); }
//----------------------------------------------------------------------------
bool connFindMatch(PConnBase* conn, LPCSTR m, int dim)
{
  consumerConn cons(conn);
  int found = cons.checkMatch((LPCBYTE)m, dim);
  return found >= 0;
}
//-------------------------------------------------------------------
