//---------- p_lan.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_lan.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "base64.h"
#include "p_param.h"
#include "p_param_v.h"
//----------------------------------------------------------------------------
P_Lan::P_Lan(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(conn, parent, resId, hinstance)
{
  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_SEND, IDB_BITMAP_DONE };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDOK, Bmp[0]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[1]);
}
//----------------------------------------------------------------------------
P_Lan::~P_Lan()
{
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
DWORD getAddr(LPCTSTR address)
{
  int p1;
  int p2;
  int p3;
  int p4;
  TCHAR tmp[64];
  _tcscpy_s(tmp, address);
  _stscanf_s(tmp, _T("%d.%d.%d.%d"), &p1, &p2, &p3, &p4);
  DWORD addr = p1 | (p2 << 8) | (p3 << 16) | (p4 << 24);
  return addr;
}
//------------------------------------------------------------------
#define IP4(a) (((a) >> 24) & 0xff)
#define IP3(a) (((a) >> 16) & 0xff)
#define IP2(a) (((a) >> 8) & 0xff)
#define IP1(a) (((a) >> 0) & 0xff)
//------------------------------------------------------------------
static
void setAddr(LPTSTR address, DWORD val)
{
  wsprintf(address, _T("%d.%d.%d.%d"), IP1(val), IP2(val), IP3(val), IP4(val));
}
//----------------------------------------------------------------------------
static LPSTR replacePoint(LPSTR dest, LPCSTR source)
{
  while(*source) {
    if('.' == *source)
      *dest = ',';
    else
      *dest = *source;
    ++source;
    ++dest;
    }
  *dest++ = ',';
  *dest = 0;
  return dest;
}
//----------------------------------------------------------------------------
void P_Lan::BNClickedOk()
{
  char buff[2048] = {};
  char address[20];
  connSend cs(Conn);

  strcpy_s(buff, "cmd=ping:");
  GetDlgItemTextA(*this, IDC_EDIT_PING_64, buff + strlen(buff), SIZE_A(buff) - strlen(buff));
  strcat_s(buff, "#");
  Conn->passThrough(cReset, 0);
  cs.send(buff, strlen(buff));

  strcpy_s(buff, "cmd=getping:#");
  Conn->passThrough(cReset, 0);
  cs.send(buff, strlen(buff));
  parseResponce(buff, SIZE_A(buff) - 1);

  strcpy_s(buff, "cmd=port:");
  GetDlgItemTextA(*this, IDC_EDIT_PORT, address, SIZE_A(address));
  strcat_s(buff, address);
  strcat_s(buff, "#");
  Conn->passThrough(cReset, 0);
  cs.send(buff, strlen(buff));
  Sleep(500);
  strcpy_s(buff, "cmd=ip:");
  GetDlgItemTextA(*this, IDC_IPADDRESS, address, SIZE_A(address));
  LPSTR p = buff + strlen(buff);
  p = replacePoint(p, address);
  GetDlgItemTextA(*this, IDC_IPADDRESS_DNS, address, SIZE_A(address));
  p = replacePoint(p, address);
  GetDlgItemTextA(*this, IDC_IPADDRESS_GATEWAY, address, SIZE_A(address));
  p = replacePoint(p, address);
  *p++ = '#';
  *p++ = 0;
  Conn->passThrough(cReset, 0);
  cs.send(buff, strlen(buff));

  strcpy_s(buff, "cmd=getip:#");
  Conn->passThrough(cReset, 0);
  cs.send(buff, strlen(buff));
  parseResponce(buff, SIZE_A(buff) - 1);

  Sleep(500);
  setting s;
  fillSetting(s);
  if (s.useWiFi) {
    strcpy_s(buff, "cmd=wifi:");
    GetDlgItemTextA(*this, IDC_EDIT_SSID, buff + strlen(buff), SIZE_A(buff) - strlen(buff));
    strcat_s(buff, ",");
    GetDlgItemTextA(*this, IDC_EDIT_WIFI_PSW, buff + strlen(buff), SIZE_A(buff) - strlen(buff));
    strcat_s(buff, ",#");
    Conn->passThrough(cReset, 0);
    cs.send(buff, strlen(buff));
    parseResponce(buff, SIZE_A(buff) - 1);
  }
}
//----------------------------------------------------------------------------
bool P_Lan::create()
{
  if(!baseClass::create())
    return false;
  Conn->passThrough(cReset, 0);
  char buff[4096] = "cmd=getip:#";
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
  buff[readed] = 0;
  parseIP(buff);
  strcpy_s(buff, "cmd=getping:#");
  cs.send(buff, strlen(buff));
  readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
  buff[readed] = 0;
  parsePing(buff);
  setting s;
  fillSetting(s);
  ENABLE(IDC_EDIT_SSID, s.useWiFi);
  ENABLE(IDC_EDIT_WIFI_PSW, s.useWiFi);
  if (s.useWiFi) {
    strcpy_s(buff, "cmd=getwifi:#");
    cs.send(buff, strlen(buff));
    readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
    buff[readed] = 0;
    parseWiFi(buff);
    }

  return true;
}
//----------------------------------------------------------------------------
void P_Lan::refreshBase64()
{
  char buff[256];
  char b64[256];
  GetDlgItemTextA(*this, IDC_EDIT_PING, buff, SIZE_A(buff));
  uint dim = base64_encode(b64, buff, strlen(buff));
  b64[dim] = 0;
  SetDlgItemTextA(*this, IDC_EDIT_PING_64, b64);
}
//----------------------------------------------------------------------------
LRESULT P_Lan::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          BNClickedOk();
          break;
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        case IDC_EDIT_PING:
          if(EN_CHANGE == HIWORD(wParam))
            refreshBase64();
          break;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MATCH_IP "r=getip:"
#define MATCH_PING "r=getping:"
#define MATCH_WIFI "r=getwifi:"
#define DIM_MATCH_IP (strlen(MATCH_IP))
#define DIM_MATCH_PING (strlen(MATCH_PING))
#define DIM_MATCH_WIFI (strlen(MATCH_WIFI))
//----------------------------------------------------------------------------
#define SET_TEXT_A(idc, txt) SetDlgItemTextA(*this, idc, txt)
//----------------------------------------------------------------------------
void P_Lan::parseIP(LPCSTR buff)
{
  int pos = checkPos(buff, MATCH_IP, DIM_MATCH_IP);
  if(pos < 0)
    return;
  pvvCharA target;
  uint n = splitParamA(target, buff + pos);
  if(n != 4)
    return;
  SET_TEXT_A(IDC_IPADDRESS, &target[0]);
  SET_TEXT_A(IDC_IPADDRESS_DNS, &target[1]);
  SET_TEXT_A(IDC_IPADDRESS_GATEWAY, &target[2]);
  SET_TEXT_A(IDC_EDIT_PORT, &target[3]);
}
//----------------------------------------------------------------------------
void P_Lan::parsePing(LPCSTR buff)
{
  int pos = checkPos(buff, MATCH_PING, DIM_MATCH_PING);
  if(pos < 0)
    return;
  char t[256];
  LPCSTR p = buff + pos;
  for(int i = 0; i < SIZE_A(t); ++i) {
    t[i] = p[i];
    if(!p[i])
      break;
    }
  SET_TEXT_A(IDC_EDIT_PING_64, t);
  LPSTR pt = t + strlen(t) + 1;
  uint l = base64_decode(pt, t, strlen(t));
  pt[l] = 0;
  SET_TEXT_A(IDC_EDIT_PING, pt);
}
//----------------------------------------------------------------------------
void P_Lan::parseWiFi(LPCSTR buff)
{
  int pos = checkPos(buff, MATCH_WIFI, DIM_MATCH_WIFI);
  if (pos < 0)
    return;
  pvvCharA target;
  uint n = splitParamA(target, buff + pos);
  if (n != 2)
    return;
  SET_TEXT_A(IDC_EDIT_SSID, &target[0]);
  SET_TEXT_A(IDC_EDIT_WIFI_PSW, &target[1]);
}
//----------------------------------------------------------------------------
#define WAIT_REPEAT 100
#define WAIT_DELAY  50
void P_Lan::parseResponce(LPSTR buff, int dim)
{
  for (uint i = 0; i < WAIT_REPEAT; ++i) {
    if (Conn->has_string())
      break;
    Sleep(WAIT_DELAY);
    }
  DWORD readed = storeUntil(Conn, '#', buff, dim);
  buff[readed] = 0;

  int pos = checkPos(buff, "r=", 2);
  int pos2 = checkPos(buff, ":", 1);
  if (pos < 0 || pos2 < 0)
    MessageBox(*this, _T("No Responce"), _T("Error"), MB_OK | MB_ICONSTOP);

  else {
    buff[pos2 - 1] = 0;
    MessageBoxA(*this, buff + pos2, buff + pos, MB_OK | MB_ICONINFORMATION);
  }
}
