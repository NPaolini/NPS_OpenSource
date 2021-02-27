//------------------ PModemCom.cpp ---------------------------
//-----------------------------------------------------------
#include "precompHeader.h"
//-----------------------------------------------------------
#include <process.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "wm_custom.h"
#include "PModemCom.h"
#include "p_util.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PModemCom::PModemCom(HWND hwnd) : baseClass(hwnd, 512 * 256), onLine(false), waitingClose(false),
      lastTick(0), support_RLSD(false), needCheck(false)
{
}
//-----------------------------------------------------------
PModemCom::~PModemCom()
{
  waitingClose = true;
  close();
}
//------------------------------------------------
bool checkString(LPBYTE buff, int len, LPBYTE match, int dim, LPBYTE& target)
{
  while(buff) {
    char* pos = strchr((LPSTR)buff, (int)match[0]);
    if(pos) {
      if(!_memicmp(match, pos, dim)) {
        target = (LPBYTE)pos + dim;
        return true;
        }
      ++pos;
      }
    buff = (LPBYTE)pos;
    }
  return false;
}
//-----------------------------------------------------------
#define STRING_CONN_OK      "0"
#define STRING_CONNECT      "1"
#define STRING_RING         "2"
#define STRING_NO_CARRIER   "3"
#define STRING_ERR          "4"
#define STRING_CONN_1200    "5"
#define STRING_NO_DIAL_TONE "6"
#define STRING_BUSY         "7"
#define STRING_NO_ANSWER    "8"
#define STRING_RINGING      "9"
// prima stringa valida di connessione a velocità xxx
#define STRING_CONNECT_BD  "19"

//-----------------------------------------------------------
//-----------------------------------------------------------
#define MAX_DELAY_BYTE (1000 * 30)
//-----------------------------------------------------------
void PModemCom::checkConn(DWORD len)
{
  if(isOnLine() || waitingClose || !needCheck)
    return;
  BYTE buff[100];

  do {
    criticalLock crtLck(cS);
    len = cL->stored();
    len = min(len, sizeof(buff) - 1);
    cL->getBytesNoDestroy(buff, len);
    } while(false);

  buff[len] = 0;
  LPCSTR p = (LPCSTR)buff;
  while(true) {
    int code = atoi(p);
    if(code) {
      if(1 == code || code > 9) {
        onLine = true;
        needCheck = false;
        }
      if(getHWin())
      ::PostMessage(getHWin(), WM_CUSTOM_MSG_CONNECTION,
            MAKEWPARAM(CM_BY_MODEM, ID_MODEM_CONNECTED), (LPARAM)code);
      Sleep(100);
      }
    while(*p && isdigit((unsigned)*p))
      ++p;
    if(!*p)
      break;
    while(*p && !isdigit((unsigned)*p))
      ++p;
    if(!*p)
      break;
    }
}
//-----------------------------------------------------------
DWORD PModemCom::readString(bool& onError)
{
  DWORD len = baseClass::readString(onError);
  if(len)
    checkConn(len);
  return len;
}
//------------------------------------------------
bool PModemCom::checkOK(DWORD check)
{
  BYTE buff[50];
  bool success = false;
  for(int i = 0; i < 60 && !success; ++i) {
    Sleep(50);
    DWORD len = has_string();
    if(len) {
      len = min(len, sizeof(buff));
      cL->getBytesNoDestroy(buff, len);
      LPBYTE p = 0;
      if(check & cNumber) {
        success = checkString(buff, len, (LPBYTE)"0\r", 2, p);
        if(!success)
          success = checkString(buff, len, (LPBYTE)"0\n", 2, p);
        }
      if(!success && check & cString) {
        success = checkString(buff, len, (LPBYTE)"OK\r", 3, p);
        if(!success)
          success = checkString(buff, len, (LPBYTE)"OK\n", 3, p);
        }
      if(p) {
        if(success)
          p += 2 + (check & cNumber ? 0 : 1);
        cL->getBytes(buff, p - buff);
        }
      else if(len > 3)
        cL->getBytes(buff, len - 3);
      }
    }
  return success;
}
//-----------------------------------------------------------
bool PModemCom::sendInitString(LPCTSTR initString, int sleepTime)
{
  if(!initString || !*initString)
    return true;
  int len = _tcslen(initString);
  char t;
  for (int i = 0; i < len; ++i) {
    t = char(initString[i]);
    if(!write_string(&t, 1))
      return false;
    }
  write_string("\r", 1);
  Sleep(sleepTime);
  return checkOK(cString);
}
//------------------------------------------------
bool PModemCom::startModem(LPCTSTR address, LPCTSTR initString, int useTone, int sleepTime)
{
  needCheck = false;
  bool old = reqNotify(false);

  support_RLSD = true;
  COMMPROP cp;
  ZeroMemory(&cp, sizeof(cp));
  if(GetCommProperties(getIdCom(), &cp))
    support_RLSD = toBool(PCF_RLSD & cp.dwProvCapabilities);

  cL->reset();
  write_string("ATZ\r", 4);
  Sleep(sleepTime);
  if(!checkOK(cString | cNumber))
    return false;

  write_string("ATV1\r", 5);
  Sleep(sleepTime);

  // no echo
//  write_string("ATE0\r", 5);
//  Sleep(100);
  // no protezione in isdn, standard ITU V.22
//  write_string("ATB00\r", 6);
//  Sleep(300);
  // Abilita il controllo di flusso RTS/CTS
//  write_string("AT&K3\r", 6);
//  Sleep(300);

  // Abilita codici risultato estesi e rilevamento segnale di occupato;
  // Disabilita rilevamento segnale di selezione.
//  write_string("ATX3\r", 5);
//  Sleep(300);

  cL->reset();
  if(!sendInitString(initString, sleepTime)) {
    MessageBox(getHWin(), _T("Errore nell'invio della stringa di inizializzazione"),
        _T("Errore"), MB_OK | MB_ICONSTOP);
    return false;
    }

  cL->reset();
  // no echo
  write_string("ATE0\r", 5);
  Sleep(sleepTime);
  checkOK(cString);

  // scarica tutto il buffer
  cL->reset();
//  reset();
  // numeri anzichè testi
  write_string("ATV0\r", 5);
  Sleep(sleepTime);

  BYTE buff[10] = { 0, 0, 0, 0 };
#if 1

  int i;
  for(i = 0; i < 5; ++i) {
    read_string(buff, 2);
    if('A' == buff[0] && 'T' == buff[1]) {
      read_string(buff, 3);
      if(!checkOK(cNumber))
        return false;
      }
    else if('0' == buff[0] && '\r' == buff[1])
      break;
    Sleep(sleepTime);
    }
  if(5 <= i)
    return false;

#else

  read_string(buff, 2);
  if('A' == buff[0] && 'T' == buff[1]) {
    read_string(buff, 3);
    if(!checkOK(true))
      return false;
    }
  else if('0' != buff[0] || '\r' != buff[1])
    return false;

#endif
  Sleep(sleepTime);
  // scarica tutto il buffer
  cL->reset();
//  reset();

  needCheck = true;
  if(address && (isdigit(*address) || _T(',') == *address)) {
//    useCall = true;
    switch(useTone) {
      case 0:
        write_string("ATDP", 4);
        break;
      case 1:
        write_string("ATDT", 4);
        break;
      case 2:
        write_string("ATDT", 4);
//        write_string("ATDI", 4);
        break;
      }
    int len = _tcslen(address);
    for (int i = 0; i < len; ++i) {
      char t = char(address[i]);
      write_string(&t, 1);
      }
    write_string("\r", 1);
    }
  else
    write_string("ATS0=1\r", 7);
  Sleep(sleepTime);
//  Sleep(2000);
  cL->reset();
  reqNotify(old);
  return true;
}
//------------------------------------------------
void PModemCom::closeLine()
{
  onLine = false;
  write_string("+++", 3);
  Sleep(1000);
  write_string("ATH0\r", 5);
  Sleep(100);
}
//------------------------------------------------
void PModemCom::close()
{
  waitingClose = true;
  if(isOnLine())
    closeLine();
  baseClass::close();
}
//------------------------------------------------
DWORD PModemCom::write_string(const void *buff, DWORD len)
{
  if(isOnLine() && support_RLSD) {
    DWORD statModem;
    if(GetCommModemStatus(getIdCom(), &statModem)) {
      if(!(statModem & MS_RLSD_ON)) {
        onLine = false;
        waitingClose = 0;
        bool old = reqNotify(true);
        notify(WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_BY_MODEM, ID_MODEM_ERR_RLSD), 0);
        reqNotify(old);
        Sleep(100);
        return 0;
        }
      }
    }
  return baseClass::write_string(buff, len);
}
//------------------------------------------------
void PModemCom::notify(int msg, WPARAM wp, LPARAM lp)
{
  if(CM_MSG_HAS_CHAR != LOWORD(wp))
    wp = MAKEWPARAM(CM_BY_MODEM, HIWORD(wp));
  baseClass::notify(msg, wp, lp);
}
