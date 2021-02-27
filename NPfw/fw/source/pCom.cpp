//-------------------- pcom.cpp ----------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <process.h>
//-----------------------------------------------------------
#include "pcom.h"
#include "p_util.h"
//-----------------------------------------------------------
#ifdef ONE_BYTE
//-----------------------------------------------------------
// implementa un vettore circolare, scrive in testa e legge dalla coda
// la lettura/scrittura deve avvenire un byte alla volta
/*
class P_CList
{
  public:
    explicit P_CList(DWORD n_el) : Buff(new BYTE[n_el]), Max(n_el),
        Head(0), Tail(0)    {   }

    ~P_CList()    {     delete []Buff;    }

    // numero di byte caricati
    DWORD stored() { int t = Head - Tail; return t < 0 ? Max + t : t; }
    // numero di byte liberi, uno in meno per evitare che
    // Head e Tail si sovrappongano
    DWORD free() { int t = Tail - Head; return t <= 0 ? Max + t - 1 : t - 1; }

    void reset() { Head = 0; Tail = 0; }

    // non dovrebbe essere necessario la tecnica difensiva, vengono
    // letti/scritti byte solo dopo aver controllato la disponibilità
//#define DIFENSIVE
  #ifdef DIFENSIVE
    // controllano validità
    bool putByte(BYTE b);
    bool  getByte(BYTE *b);
  #else
    void putByte(BYTE b);
    void getByte(BYTE *b);
  #endif
  private:
    P_CList();
    P_CList operator =(P_CList&);
    BYTE *Buff;
    DWORD Max;
    DWORD Head;
    DWORD Tail;
};
*/
//------------------------------------------------
  #ifdef DIFENSIVE
bool P_CList::putByte(BYTE b)
{
  // verifica overflow
  if(Head == Tail -1 || Head == Max - 1 && !Tail)
    return false;
  Buff[Head] = b;
  if(++Head >= Max)
    Head -= Max;
  return true;
}
bool P_CList::getByte(BYTE *b)
{
  // coda vuota
  if(Tail == Head)
    return false;
  *b = Buff[Tail];
  if(++Tail >= Max)
    Tail -= Max;
  return true;
}
  #else
void P_CList::putByte(BYTE b)
{
  #define IS_THIS_BETTER_
  #ifdef IS_THIS_BETTER_
// il salto condizionato svuota la coda di prefetch,
// ma con il prediction branch....
  Buff[Head] = b;
  if(++Head >= Max)
    Head -= Max;
  #else
// il modulo comporta più cicli di una comparazione e sottrazione,
// ma con le pipeline....
  Buff[Head++] = b;
  Head %= Max;
  #endif
}
void P_CList::getByte(BYTE *b)
{
  #ifdef IS_THIS_BETTER_
  *b = Buff[Tail];
  if(++Tail >= Max)
    Tail -= Max;
  #else
  *b = Buff[Tail++];
  Tail %= Max;
  #endif
}
  #endif
//------------------------------------------------
void P_CList::getBytesNoDestroy(LPBYTE b, int len)
{
  DWORD oldTail = Tail;
  DWORD oldHead = Head;
  for(int i = 0; i < len; ++i)
    getByte(b + i);
  Tail = oldTail;
  Head = oldHead;
}
//------------------------------------------------
void P_CList::putBytes(LPBYTE b, int len)
{
  for(int i = 0; i < len; ++i)
    putByte(b[i]);
}
//------------------------------------------------
void P_CList::getBytes(LPBYTE b, int len)
{
  for(int i = 0; i < len; ++i)
    getByte(b + i);
}
//------------------------------------------------
#else
//------------------------------------------------
/*
class P_CList
{
  public:
    explicit P_CList(DWORD n_el) : Buff(new BYTE[n_el]), Max(n_el),
        Head(0), Tail(0)    {   }

    ~P_CList()    {     delete []Buff;    }

    // numero di byte caricati
    DWORD stored() { int t = Head - Tail; return t < 0 ? Max + t : t; }
    // numero di byte liberi, uno in meno per evitare che
    // Head e Tail si sovrappongano
    DWORD free() { int t = Tail - Head; return t <= 0 ? Max + t - 1 : t - 1; }

    void reset() { Head = 0; Tail = 0; }

    void putByte(LPBYTE b, int len);
    void getByte(LPBYTE b, int len);
    void getByteNoDestroy(LPBYTE b, int len);
  private:
    P_CList();
    P_CList operator =(P_CList&);
    LPBYTE Buff;
    DWORD Max;
    DWORD Head;
    DWORD Tail;
};
*/
//------------------------------------------------
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
//------------------------------------------------
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
//------------------------------------------------
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
//------------------------------------------------
#endif
//------------------------------------------------
/*
class sectLock
{
  public:
    sectLock(CRITICAL_SECTION &cs) : cS(cs) { EnterCriticalSection(&cS); }
    ~sectLock() { LeaveCriticalSection(&cS); }
  private:
    CRITICAL_SECTION &cS;

    sectLock(const sectLock &sl);
    sectLock operator=(const sectLock &sl);
};
*/
//------------------------------------------------
//------------------------------------------------
PComBase *allocCom(HWND hw) { return new PCom(hw); }
//------------------------------------------------
//------------------------------------------------
static const int vBaudRate[] =
{ CBR_110,  CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800,
  CBR_9600, CBR_14400, CBR_19200, CBR_38400, CBR_56000,
  CBR_57600, CBR_115200, CBR_128000, CBR_256000, CBR_230400, CBR_460800, CBR_921600
};
//------------------------------------------------
static const int vParity[] = {  EVENPARITY, MARKPARITY, NOPARITY, ODDPARITY, SPACEPARITY };
//------------------------------------------------
static const int vBitStop[] = { ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS };
//------------------------------------------------
static const int vBitLen[] =  { 5, 6, 7, 8 };
//------------------------------------------------
static const int ASCII_XON = 0x11;
static const int ASCII_XOFF = 0x13;
//------------------------------------------------
//------------------------------------------------
// per test rotazione buffer vettore e alternanza tra le due 'read_string()'
//#define DIM_BUFF 24

// a 115200 baud questo buffer è insufficiente
//#define DIM_BUFF 256


PCom::PCom(HWND hwnd, DWORD dimBuffer) : PComBase(hwnd),
            cL(new P_CList(dimBuffer)), connected(false),
            //hThread(0),
            idThread(0),
            onClosing(false), hEventCloseConn(CreateEvent(0, TRUE, 0, 0))
{
//  InitializeCriticalSection(&criticalSection);
  ZeroMemory(&oRead, sizeof(oRead));
  ZeroMemory(&oWrite, sizeof(oWrite));
  oRead.hEvent = CreateEvent(0, 1, 0, 0);
  oWrite.hEvent = CreateEvent(0, 1, 0, 0);
}
//------------------------------------------------
PCom::~PCom()
{
  close();
  CloseHandle(oWrite.hEvent);
  CloseHandle(oRead.hEvent);
  CloseHandle(hEventCloseConn);
  delete cL;
//  DeleteCriticalSection(&criticalSection);
}
//------------------------------------------------
#define PURGE_ALL (PURGE_TXABORT | PURGE_RXABORT | \
                   PURGE_TXCLEAR | PURGE_RXCLEAR)
bool PCom::open()
{
  if(getIdCom() != INVALID_HANDLE_VALUE)
    close();
  if(getIdCom() != INVALID_HANDLE_VALUE)
    return false;
  ResetEvent(hEventCloseConn);
  paramCom parCom = getParam();
  TCHAR t[20];
  wsprintf(t, _T("\\\\.\\COM%d"), parCom.getPort() + 1);
  SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), 0 , true };
  HANDLE id = CreateFile(t, GENERIC_READ | GENERIC_WRITE, 0, &sa,
      OPEN_EXISTING,  FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL  , NULL);
  if(id == INVALID_HANDLE_VALUE)
    return false;
  SetCommMask(id, EV_RXCHAR);
  SetupComm(id, 8192, 8192);//1024);

  PurgeComm(id, PURGE_ALL);

  COMMTIMEOUTS sTimeoutEvent={ 0xffffffff, 0, 1000,
          2 * CBR_9600 / vBaudRate[parCom.b.baud], 0 };

  SetCommTimeouts(id, &sTimeoutEvent);

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
//------------------------------------------------
void PCom::resizeBuffer(DWORD dimBuffer)
{
  criticalLock crtLck(cS);
  if(cL->getDim() != dimBuffer) {
    delete cL;
    cL = new P_CList(dimBuffer);
    }
}
//------------------------------------------------
void PCom::fillDCB(DCB &dcb) const
{
  paramCom parCom = getParam();
  dcb.DCBlength = sizeof(dcb);
  GetCommState(getIdCom(), &dcb);
  dcb.fBinary   = true;
//  dcb.fParity   = true;
  dcb.fDsrSensitivity = false;

  dcb.BaudRate  = vBaudRate[parCom.b.baud];
  dcb.ByteSize  = static_cast<BYTE>(vBitLen[parCom.b.len]);
  dcb.Parity    = static_cast<BYTE>(vParity[parCom.b.parity]);
  dcb.StopBits  = static_cast<BYTE>(vBitStop[parCom.b.stop]);

  dcb.fParity = dcb.Parity != NOPARITY;

  BYTE v = static_cast<BYTE>(parCom.b.dtr_dsr);
  switch(v) {
    case paramCom::NO_DTR_DSR:
      dcb.fOutxDsrFlow = 0;
      dcb.fDtrControl = DTR_CONTROL_DISABLE;
      break;
    case paramCom::ON_DTR_DSR:
      dcb.fOutxDsrFlow = 0;
      dcb.fDtrControl = DTR_CONTROL_ENABLE;
      break;
    case paramCom::HANDSHAKE_DTR_DSR:
      dcb.fOutxDsrFlow = 1;
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
      break;
    }

  v = static_cast<BYTE>(parCom.b.rts_cts);
  switch(v) {
    case paramCom::NO_RTS_CTS:
      dcb.fOutxCtsFlow = 0;
      dcb.fRtsControl = RTS_CONTROL_DISABLE;
      break;
    case paramCom::ON_RTS_CTS:
      dcb.fOutxCtsFlow = 0;
      dcb.fRtsControl = RTS_CONTROL_ENABLE;
      break;
    case paramCom::HANDSHAKE_RTS_CTS:
      dcb.fOutxCtsFlow = 1;
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
      break;
    case paramCom::TOGGLE_RTS_CTS:
      dcb.fOutxCtsFlow = 0;
      dcb.fRtsControl = RTS_CONTROL_TOGGLE;
      break;
    }
  v = static_cast<BYTE>(parCom.b.xon_xoff);
  dcb.fInX = v;
  dcb.fOutX = v;
  dcb.XonChar = ASCII_XON;
  dcb.XoffChar = ASCII_XOFF;
  dcb.XonLim = 100;
  dcb.XoffLim = 100;
}
//------------------------------------------------
#if 0
#ifndef min
template <class T>
T min(T a, T b)
{
  return a > b ? b : a;
}
#endif
//------------------------------------------------
#ifndef max
template <class T>
T max(T a, T b)
{
  return a < b ? b : a;
}
#endif
#endif
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
//------------------------------------------------
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
#ifdef ONE_BYTE
      for(DWORD i = 0; i < curr; ++i) {
  #ifdef DIFENSIVE
        if(!cL->getByte(p + i + currRead)) {
          curr = i;
          break;
          }
  #else
        cL->getByte(p + i + currRead);
  #endif
        }
#else
      cL->getBytes(p + currRead, curr);
#endif
      }  // fine visibilita' e vita di crtLck
    // se ha letto qualcosa deve riazzerare il contatore per il timeout
    if(curr) {
      count = 0;
      currRead += curr;
      }
    if(currRead == len || ++count >= getTOcount()) // ha terminato
      break;
    // aspetta un po' e da' tempo al thread di leggere dalla
    // seriale lasciando sbloccato il buffer per i dati
//    Sleep(getTOms());
    SleepEx(getTOms(), false);
    }
  return currRead;
}
//------------------------------------------------
#define SLEEP_REPEAT_CHECK_READ 50
#define MAX_REPEAT_CHECK_READ  (10000 / SLEEP_REPEAT_CHECK_READ)
//------------------------------------------------
#define MAX_TIMEOUT_RECEIVE 10000
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

  ClearCommError(getIdCom(), &err, &stat);

  DWORD len = min(DWORD(stat.cbInQue), free);
  len = min(len, (DWORD)sizeof(buff));
  if(!len)
    return 0;

  if(!ReadFile(getIdCom(), buff, len, &len, &oRead)) {
    bool success = false;
    if(ERROR_IO_PENDING == GetLastError()) {
      HANDLE waitFor[] = {  oRead.hEvent,  hEventCloseConn };
      DWORD result = WaitForMultipleObjects(SIZE_A(waitFor), waitFor, 0, MAX_TIMEOUT_RECEIVE);
      if(WAIT_OBJECT_0 == result) {
        DWORD dummy;
        success = true;
        uint count = 0;
        while(!GetOverlappedResult(getIdCom(), &oRead, &dummy, false)) {
          if(ERROR_IO_INCOMPLETE == GetLastError()) {
            if(WAIT_TIMEOUT != WaitForSingleObject(hEventCloseConn, SLEEP_REPEAT_CHECK_READ)) {
              success = false;
              break;
              }
            if(++count >= MAX_REPEAT_CHECK_READ)
              break;
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
  if(onError)
    return 0;

  if(len) {
    criticalLock crtLck(cS);

#ifdef ONE_BYTE
    for(DWORD i = 0; i < len; ++i)
  #ifdef DIFENSIVE
      if(!cL->putByte(buff[i])) {
        len = i;
        break;
        }
  #else
      cL->putByte(buff[i]);
  #endif
#else
    cL->putBytes(buff, len);
#endif
    notify(ID_MSG_HAS_CHAR, (WPARAM)cL->stored(), (LPARAM)getIdCom());
    }
  return len;
}
//------------------------------------------------
#define MAX_TIMEOUT_SEND 3000
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
//        ResetEvent(oWrite.hEvent);

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
    }
  if(allWritten != oldLen)
    ResetEvent(oWrite.hEvent);
  return allWritten;
}
//------------------------------------------------
void PCom::setBreak(DWORD ms)
{
  if(getIdCom() != INVALID_HANDLE_VALUE) {
    SetCommBreak(getIdCom());
    SleepEx(ms, false);
    ClearCommBreak(getIdCom());
    }
}
//------------------------------------------------
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

  if(!SetCommMask(id, EV_RXCHAR)) {
    Com->resetThread();
    return false;
    }

  bool onError = !Com->isConnected();
  while(!onError) {
    DWORD mask = 0;
    WaitCommEvent(id, &mask, &Com->oRead);

    if(EV_RXCHAR == (mask & EV_RXCHAR))
      while(Com->isConnected() && Com->readString(onError))
        Sleep(50);
/**/
    else {
      HANDLE waitFor[] = {  Com->oRead.hEvent,  Com->hEventCloseConn };
      DWORD result = WaitForMultipleObjects(SIZE_A(waitFor), waitFor, 0, INFINITE);
//      DWORD result = WaitForSingleObject(Com->oRead.hEvent, INFINITE);
      if(WAIT_OBJECT_0 == result) {
        while(Com->isConnected() && Com->readString(onError))
          Sleep(0);
        }
      }
/**/
    if(!Com->isConnected())
      break;
    }

  Com->resetThread();
  Com->close();
  return true;
}
//------------------------------------------------
DWORD PCom::has_string()
{
  if(getIdCom() == INVALID_HANDLE_VALUE)
    return 0;
  criticalLock crtLck(cS);
  return cL->stored();
}
//------------------------------------------------
void PCom::reset()
{
  criticalLock crtLck(cS);
  PurgeComm(getIdCom(), PURGE_RXCLEAR);
  cL->reset();
}
//------------------------------------------------
//------------------------------------------------

