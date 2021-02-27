//-------------------- pcom.cpp -----------------------------
//-----------------------------------------------------------
#include "precompHeader.h"
//-----------------------------------------------------------
#include <process.h>
//-----------------------------------------------------------
#include "pcom.h"
#include "p_util.h"
#include "wm_custom.h"
//-----------------------------------------------------------
void P_CList::putBytes(LPBYTE b, int len)
{
  int avail = Max - Head;
  if(avail > len)
    avail = len;
  memcpy(Buff + Head, b, avail);
  len -= avail;
  if(len) {
    memcpy(Buff, b + avail, len);
    Head = len;
    }
  else
    Head += avail;
}
//-----------------------------------------------------------
void P_CList::getBytes(LPBYTE b, int len)
{
  int avail = Max - Tail;
  if(avail > len)
    avail = len;
  memcpy(b, Buff + Tail, avail);
  len -= avail;
  if(len) {
    memcpy(b + avail, Buff, len);
    Tail = len;
    }
  else
    Tail += avail;
}
//-----------------------------------------------------------
void P_CList::getBytesNoDestroy(LPBYTE b, int len)
{
  int avail = Max - Tail;
  if(avail > len)
    avail = len;
  memcpy(b, Buff + Tail, avail);
  len -= avail;
  if(len)
    memcpy(b + avail, Buff, len);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PComBase *allocCom(HWND hw) { return new PCom(hw); }
//-----------------------------------------------------------
//-----------------------------------------------------------
static const int vBaudRate[] =
{ CBR_110,  CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800,
  CBR_9600, CBR_14400, CBR_19200, CBR_38400, CBR_56000,
  CBR_57600, CBR_115200, CBR_128000, CBR_256000,
};
//-----------------------------------------------------------
static const int vParity[] = {  EVENPARITY, MARKPARITY, NOPARITY, ODDPARITY, SPACEPARITY };
//-----------------------------------------------------------
static const int vBitStop[] = { ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS };
//-----------------------------------------------------------
static const int vBitLen[] =  { 5, 6, 7, 8 };
//-----------------------------------------------------------
static const int ASCII_XON = 0x11;
static const int ASCII_XOFF = 0x13;
//-----------------------------------------------------------
PCom::PCom(HWND hwnd, DWORD dimBuffer) : PComBase(hwnd),
            cL(new P_CList(dimBuffer)), connected(false),
            idThread(0), onClosing(false), hEventCloseConn(CreateEvent(0, TRUE, 0, 0))
{
  ZeroMemory(&oRead, sizeof(oRead));
  ZeroMemory(&oWrite, sizeof(oWrite));
  oRead.hEvent = CreateEvent(0, 1, 0, 0);
  oWrite.hEvent = CreateEvent(0, 0, 0, 0);
}
//-----------------------------------------------------------
PCom::~PCom()
{
  close();
  CloseHandle(oWrite.hEvent);
  CloseHandle(oRead.hEvent);
  CloseHandle(hEventCloseConn);
  delete cL;
}
//-----------------------------------------------------------
#define PURGE_ALL (PURGE_TXABORT | PURGE_RXABORT)
//#define PURGE_ALL (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)
//-----------------------------------------------------------
bool PCom::open()
{
  if(getIdCom() != INVALID_HANDLE_VALUE)
    close();
  if(getIdCom() != INVALID_HANDLE_VALUE)
    return false;
  ResetEvent(hEventCloseConn);
  paramConn parCom = getParam();
  TCHAR t[20];
  wsprintf(t, _T("\\\\.\\COM%d"), parCom.getPort() + 1);
#if true
  HANDLE id = CreateFile(t, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, NULL);
#else
  SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), 0 , true };
  HANDLE id = CreateFile(t, GENERIC_READ | GENERIC_WRITE, 0, &sa, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, NULL);
#endif
  if(id == INVALID_HANDLE_VALUE)
    return false;
#if false
  SetCommMask(id, EV_RXCHAR);
#else
  #define EV_COMM_MASK (EV_RLSD|EV_RXFLAG)
  SetCommMask(id, EV_COMM_MASK);
#endif
  SetupComm(id, 8192, 8192);

  PurgeComm(id, PURGE_ALL);
#if true
// 31/12/2019
//  COMMTIMEOUTS sTimeoutEvent={ 100, 0, 1000, 0, 5000 };
  COMMTIMEOUTS sTimeoutEvent={ 10, 0, 0, 0, 5000 };
#else
  COMMTIMEOUTS sTimeoutEvent={ 0xffffffff, 0, 1000,
          2 * CBR_9600 / vBaudRate[parCom.b.baud], 0 };
#endif
  SetCommTimeouts(id,  &sTimeoutEvent);

  DCB dcb;
  ZeroMemory(&dcb, sizeof(dcb));
  fillDCB(dcb);

  if(!SetCommState(id, &dcb)) {
    CloseHandle(id);
    return false;
    }
  connected = true;

#if 0
  HANDLE hThread = (HANDLE)_beginthreadex(0, 0, CommProc, this, 0, &idThread);
#else
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CommProc,
                  this, 0, (LPDWORD)&idThread);
#endif
  if(!hThread) {
    connected = false;
    CloseHandle(id);
    return false;
    }
//#define USE_PRIORITY
#ifdef USE_PRIORITY
// da provare
  SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
//  SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
#endif
  CloseHandle(hThread);

  EscapeCommFunction(id, SETDTR);

  criticalLock lock(cS);
  setIdCom(id);
  return true;
}
//-----------------------------------------------------------
void PCom::close()
{
  if(!onClosing && getIdCom() != INVALID_HANDLE_VALUE) {
    onClosing = true;
    HANDLE id = getIdCom();
    connected = false;
    SetCommMask(id, 0);
    SetEvent(oRead.hEvent);
    SetEvent(hEventCloseConn);
    while(idThread != 0) {
      Sleep(100);
      }

    ResetEvent(hEventCloseConn);
    id = getIdCom();
    if(id != INVALID_HANDLE_VALUE) {
      setIdCom(INVALID_HANDLE_VALUE);
      EscapeCommFunction(id, CLRDTR);
      PurgeComm(id, PURGE_ALL);
      CloseHandle(id);
      }
    onClosing = false;
    }
}
//-----------------------------------------------------------
void PCom::resizeBuffer(DWORD dimBuffer)
{
  criticalLock crtLck(cS);
  if(cL->getDim() != dimBuffer) {
    delete cL;
    cL = new P_CList(dimBuffer);
    }
}
//-----------------------------------------------------------
void PCom::fillDCB(DCB &dcb) const
{
  paramConn parCom = getParam();
  dcb.DCBlength = sizeof(dcb);
  GetCommState(getIdCom(), &dcb);
  dcb.fBinary   = true;
//  dcb.fParity   = true;
  dcb.fDsrSensitivity = false;
  dcb.fAbortOnError = true;
  dcb.fTXContinueOnXoff = true;

  dcb.BaudRate  = vBaudRate[parCom.b.baud];
  dcb.ByteSize  = static_cast<BYTE>(vBitLen[parCom.b.len]);
  dcb.Parity    = static_cast<BYTE>(vParity[parCom.b.parity]);
  dcb.StopBits  = static_cast<BYTE>(vBitStop[parCom.b.stop]);

  dcb.fParity = dcb.Parity != NOPARITY;

  BYTE v = static_cast<BYTE>(parCom.b.dtr_dsr);
  switch(v) {
    case paramConn::NO_DTR_DSR:
      dcb.fOutxDsrFlow = 0;
      dcb.fDtrControl = DTR_CONTROL_DISABLE;
      break;
    case paramConn::ON_DTR_DSR:
      dcb.fOutxDsrFlow = 0;
      dcb.fDtrControl = DTR_CONTROL_ENABLE;
      break;
    case paramConn::HANDSHAKE_DTR_DSR:
      dcb.fOutxDsrFlow = 1;
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
      break;
    }

  v = static_cast<BYTE>(parCom.b.rts_cts);
  switch(v) {
    case paramConn::NO_RTS_CTS:
      dcb.fOutxCtsFlow = 0;
      dcb.fRtsControl = RTS_CONTROL_DISABLE;
      break;
    case paramConn::ON_RTS_CTS:
      dcb.fOutxCtsFlow = 0;
      dcb.fRtsControl = RTS_CONTROL_ENABLE;
      break;
    case paramConn::HANDSHAKE_RTS_CTS:
      dcb.fOutxCtsFlow = 1;
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
      break;
    case paramConn::TOGGLE_RTS_CTS:
      dcb.fOutxCtsFlow = 0;
      dcb.fRtsControl = RTS_CONTROL_TOGGLE;
      break;
    }
  v = static_cast<BYTE>(parCom.b.xon_xoff);
  dcb.fInX = v;
  dcb.fOutX = v;
  dcb.XonChar = ASCII_XON;
  dcb.XoffChar = ASCII_XOFF;
  dcb.XonLim = 80;
  dcb.XoffLim = 200;
}
//-----------------------------------------------------------
DWORD PCom::appendToBuffer(LPBYTE buff, DWORD len)
{
  criticalLock crtLck(cS);
  DWORD free= cL->free();
  if(!free)
    return 0;

  DWORD toCopy = min(len, free);
  cL->putBytes(buff, toCopy);
  notify(WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_MSG_HAS_CHAR, cL->stored()), (LPARAM)getIdCom());
  return toCopy;
}
//-----------------------------------------------------------
DWORD PCom::readStringNoRemove(LPBYTE buff, DWORD len)
{
  DWORD dim = has_string();
  if(dim) {
    dim = min(len, dim);
    cL->getBytesNoDestroy(buff, dim);
    }
  return dim;
}
//-----------------------------------------------------------
DWORD PCom::read_string(void *buff, DWORD len)
{
  if(getIdCom() == INVALID_HANDLE_VALUE)
    return 0;
  DWORD currRead = 0;
  DWORD count = 0;

  while(WAIT_TIMEOUT == WaitForSingleObject(hEventCloseConn, 0)) {
    DWORD curr;
      {
      criticalLock crtLck(cS);
      DWORD stored = cL->stored();
      curr = min(len - currRead, stored);
      BYTE *p = reinterpret_cast<BYTE*>(buff);
      cL->getBytes(p + currRead, curr);
      }
    // se ha letto qualcosa deve riazzerare il contatore per il timeout
    if(curr) {
      count = 0;
      currRead += curr;
      }
    if(currRead == len || ++count >= getTOcount()) // ha terminato
      break;
    // aspetta un po' e da' tempo al thread di leggere dalla
    // seriale lasciando sbloccato il buffer per i dati
    Sleep(getTOms());
    }
  return currRead;
}
//-----------------------------------------------------------
DWORD PCom::readString(bool& onError)
{
  if(getIdCom() == INVALID_HANDLE_VALUE)
    return 0;

  COMSTAT stat;
  DWORD err;
  BYTE buff[DIM_BUFF];
  DWORD free;

  do {
    criticalLock crtLck(cS);
    free = cL->free();
    if(!free) {
      reset();
      free = cL->free();
      }
    } while(false);

  DWORD len = 0;

  do {
//    criticalLock crtLck(cSOneWay);
    ClearCommError(getIdCom(), &err, &stat);

    len = min(DWORD(stat.cbInQue), free);
    len = min(len, (DWORD)sizeof(buff));
    if(!len)
      return 0;

    if(!ReadFile(getIdCom(), buff, len, &len, &oRead)) {
      bool success = false;
      if(ERROR_IO_PENDING == GetLastError()) {
        HANDLE waitFor[] = {  oRead.hEvent,  hEventCloseConn };
        DWORD result = WaitForMultipleObjects(SIZE_A(waitFor), waitFor, 0, INFINITE);
        if(WAIT_OBJECT_0 == result) {
          DWORD dummy;
          success = true;
          while(!GetOverlappedResult(getIdCom(), &oRead, &dummy, false)) {
            if(ERROR_IO_INCOMPLETE == GetLastError()) {
              if(WAIT_TIMEOUT != WaitForSingleObject(hEventCloseConn, 0)) {
                success = false;
                break;
                }
              continue;
              }
            else {
              success = false;
              break;
              }
            }
          }
        }
      if(!success) {
        len = 0;
        onError = true;
        ClearCommError(getIdCom(), &err, &stat);
        }
      }
    } while(false);
  ResetEvent(oRead.hEvent);
  if(onError)
    return 0;
  if(len) {
    criticalLock crtLck(cS);
    cL->putBytes(buff, len);
    notify(WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_MSG_HAS_CHAR, cL->stored()), (LPARAM)getIdCom());
    }
  return len;
}
//-----------------------------------------------------------
#define MAX_TIMEOUT_SEND 3000
//-----------------------------------------------------------
DWORD PCom::write_string(const void *buff, DWORD len)
{
  if(getIdCom() == INVALID_HANDLE_VALUE)
    return 0;
  DWORD allWritten = 0;
  static const int MAX_DIM_SEND = 256;
  LPBYTE pBuff = (LPBYTE)buff;
  DWORD oldLen = len;
  while(len) {
    int toWrite = min(len, MAX_DIM_SEND);
    DWORD written;
    do {
//      criticalLock crtLck(cSOneWay);
      if(!WriteFile(getIdCom(), pBuff, toWrite, &written, &oWrite)) {
        bool success = false;
        do {
          if(ERROR_IO_PENDING != GetLastError())
            break;

          HANDLE waitFor[] = {  oWrite.hEvent,  hEventCloseConn };
          DWORD result = WaitForMultipleObjects(SIZE_A(waitFor), waitFor, 0, MAX_TIMEOUT_SEND);
  //        DWORD result = WaitForSingleObject(oWrite.hEvent, MAX_TIMEOUT_SEND);
          if(WAIT_OBJECT_0 != result)
            break;

          if(!GetOverlappedResult(getIdCom(), &oWrite, &written, false)) {
            if(WAIT_TIMEOUT != WaitForSingleObject(hEventCloseConn, 0))
              break;
            continue;
            }
          success = true;
          } while(!success);
        if(!success) {
          COMSTAT stat;
          DWORD err;
          ClearCommError(getIdCom(), &err, &stat);
          return 0;
          }
        len -= toWrite;
        pBuff += toWrite;
        allWritten += written;
        }
      } while(false);
    }
//  if(allWritten != oldLen)
//    ResetEvent(oWrite.hEvent);
  return allWritten;
}
//-----------------------------------------------------------
void PCom::setBreak(DWORD ms)
{
  criticalLock crtLck(cS);
//  criticalLock crtLck(cSOneWay);
  if(getIdCom() != INVALID_HANDLE_VALUE) {
    SetCommBreak(getIdCom());
    SleepEx(ms, false);
    ClearCommBreak(getIdCom());
    }
}
//-----------------------------------------------------------
unsigned FAR PASCAL CommProc(void* cl)
{
  PCom *Com = reinterpret_cast<PCom*>(cl);
  HANDLE id;
  for(;;) {
    do {
      criticalLock crtLck(Com->cS);
      id = Com->getIdCom();
      } while(false);
    if(INVALID_HANDLE_VALUE != id)
      break;
    Sleep(100);
    }

/*
#define EV_COMM_MASK (EV_RXCHAR|EV_BREAK|EV_ERR|EV_CTS|EV_DSR|EV_RING|EV_RLSD|EV_RXFLAG|EV_TXEMPTY)
// su hyperterminal
//#define EV_COMM_MASK (EV_RLSD|EV_RXFLAG)

  if(!SetCommMask(id, EV_COMM_MASK)) {
    Com->resetThread();
    return false;
    }
*/
  bool onError = !Com->isConnected();
  while(!onError) {
    DWORD mask = 0;
    WaitCommEvent(id, &mask, &Com->oRead);

    if(EV_RXCHAR == (mask & EV_RXCHAR))
      while(Com->isConnected() && Com->readString(onError))
        Sleep(0);
    else {
      HANDLE waitFor[] = {  Com->oRead.hEvent,  Com->hEventCloseConn };
      DWORD result = WaitForMultipleObjects(SIZE_A(waitFor), waitFor, 0, INFINITE);
//      DWORD result = WaitForSingleObject(Com->oRead.hEvent, INFINITE);
      if(WAIT_OBJECT_0 == result) {
        while(Com->isConnected() && Com->readString(onError))
          Sleep(0);
        }
      if(WAIT_OBJECT_0 + 1 == result)
        break;
      }
    if(!Com->isConnected())
      break;
    }

  Com->resetThread();
  Com->close();
  Com->notify(WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_BY_SERIAL_DIRECT, 0), 0);
  return true;
}
//-----------------------------------------------------------
DWORD PCom::has_string()
{
  if(getIdCom() == INVALID_HANDLE_VALUE)
    return 0;
  criticalLock crtLck(cS);
  return cL->stored();
}
//-----------------------------------------------------------
void PCom::reset()
{
  criticalLock crtLck(cS);
//  criticalLock crtLck(cSOneWay);
  PurgeComm(getIdCom(), PURGE_RXCLEAR);
  cL->reset();
}
//-----------------------------------------------------------
