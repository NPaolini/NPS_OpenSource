//---------- connClass.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <winsock2.h>
#include "connClass.h"
#include "wm_custom.h"
//----------------------------------------------------------------------------
REG_WM(CUSTOM_MSG_CONNECTION);
//----------------------------------------------------------------------------
#ifdef USE_CUSTOM_LOAD_CLASS
//----------------------------------------------------------------------------
  #include "customLoadClass.cpp"
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
// implementazione standard, la classe deve chiamarsi così ed avere il membro statico
class pDll
{
  public:
    ~pDll();

    static prtConnProcDLL ConnProcDLL;
  private:
    // non si deve instanziare
    pDll();
    static pDll dummy;
    static HMODULE hDLL;
    static int errorWSA;
    static bool loadProc();
    static LRESULT ConnProcDummy(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn) { return 0; }
};
//------------------------------------------------------------------
pDll pDll::dummy;
HMODULE pDll::hDLL;
int pDll::errorWSA = -1;
prtConnProcDLL pDll::ConnProcDLL;
//------------------------------------------------------------------
#ifdef _WIN64
#define NAME_DLL _T("nps_conn64")
#else
#define NAME_DLL _T("nps_conn")
#endif
//------------------------------------------------------------------
pDll::pDll()
{
  if(!loadProc())
    MessageBox(0, _T("Unable to load [") NAME_DLL _T(".dll]"), _T("Error"), MB_OK | MB_ICONSTOP);
  else {
    WORD versionCheck[] = { MAKEWORD(2, 2),  MAKEWORD(2, 1),  MAKEWORD(2, 0),  MAKEWORD(1, 1) };
    WSADATA wsaData;
    for(uint i = 0; i < SIZE_A(versionCheck); ++i) {
      errorWSA = WSAStartup(versionCheck[i], &wsaData);
      if(!errorWSA)
        break;
      }
    }
}
//------------------------------------------------------------------
pDll::~pDll()
{
  if(hDLL)
    FreeLibrary(hDLL);
  if(!errorWSA)
    WSACleanup();

}
//------------------------------------------------------------------
static LPCTSTR libName = NAME_DLL _T(".dll");
//------------------------------------------------------------------
static LPCSTR procName = "_ConnProcDLL";
//------------------------------------------------------------------
bool pDll::loadProc()
{
  hDLL = LoadLibrary(libName);
  if(hDLL) {
    FARPROC f = GetProcAddress(hDLL, procName);
    if(!f)
      f = GetProcAddress(hDLL, procName + 1);
    if(f) {
      ConnProcDLL = (prtConnProcDLL)f;
      return true;
      }
    }
  ConnProcDLL = ConnProcDummy;
  return false;
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
#define DEF_DELAY_4_485 5000
//----------------------------------------------------------------------------
PConnBase::PConnBase(HANDLE_CONN client) : Conn(client), delayFor485(DEF_DELAY_4_485)
{ ZeroMemory(&CI, sizeof(CI)); }
//----------------------------------------------------------------------------
#define IP4(a) (((a) >> 24) & 0xff)
#define IP3(a) (((a) >> 16) & 0xff)
#define IP2(a) (((a) >> 8) & 0xff)
#define IP1(a) (((a) >> 0) & 0xff)
//----------------------------------------------------------------------------
PConnBase::PConnBase(DWORD port, HWND hwnd, bool udp) : Conn(0), delayFor485(DEF_DELAY_4_485)
{
  ZeroMemory(&CI, sizeof(CI));
  CI.connType = udp ? cLanServerUDP : cLanServer;
  CI.paramPort = port;
  CI.t1 = (UINT_PTR)hwnd;
  CI.Owner = hwnd;
}
//----------------------------------------------------------------------------
PConnBase::PConnBase(DWORD addr, DWORD port, HWND hwnd, bool udp) : Conn(0), delayFor485(DEF_DELAY_4_485)
{
  ZeroMemory(&CI, sizeof(CI));
  CI.connType = udp ? cLanClientUDP : cLanClient;
  CI.paramPort = port;
  CI.Owner = hwnd;
  wsprintf(CI.data, _T("%d.%d.%d.%d"), IP1(addr), IP2(addr), IP3(addr), IP4(addr));
}
//----------------------------------------------------------------------------
PConnBase::PConnBase(LPCTSTR addrOrName, DWORD port, HWND hwnd, bool udp) : Conn(0), delayFor485(DEF_DELAY_4_485)
{
  ZeroMemory(&CI, sizeof(CI));
  CI.connType = udp ? cLanClientUDP : cLanClient;
  CI.paramPort = port;
  CI.Owner = hwnd;
  _tcscpy_s(CI.data, addrOrName);
}
//----------------------------------------------------------------------------
PConnBase::PConnBase(HWND hwnd, paramConn pc, LPCTSTR number, LPCTSTR init, DWORD modemType)
     : Conn(0), delayFor485(DEF_DELAY_4_485)
{
  ZeroMemory(&CI, sizeof(CI));
  if(number && *number) {
    CI.connType = cModemCall;
    wsprintf(CI.data, _T("%c%s#%s"), modemType ? _T('P') : _T('T'), number, init);
    }
  else {
    CI.connType = cModemWait;
    wsprintf(CI.data, _T("%s"), init);
    }
  CI.paramPort = pc;
  CI.Owner = hwnd;
}
//----------------------------------------------------------------------------
PConnBase::PConnBase(paramConn pc, HWND hwnd, bool on485) : Conn(0), delayFor485(DEF_DELAY_4_485)
{
  ZeroMemory(&CI, sizeof(CI));
  CI.connType = cDirectSerial;
  if(on485)
    pc.b.rts_cts = paramConn::NO_RTS_CTS;

  CI.paramPort = pc;
  CI.t1 = on485;
  CI.Owner = hwnd;
}
//----------------------------------------------------------------------------
PConnBase::~PConnBase()
{
  if(Conn)
    pDll::ConnProcDLL(cDestroyConn, 0, Conn);
}
//----------------------------------------------------------------------------
bool PConnBase::open()
{
  if(Conn) {
    if(isConnected())
      return true;
    close();
    }
  if(!Conn)
    Conn = (HANDLE_CONN)pDll::ConnProcDLL(cMakeConn, 0, (HANDLE_CONN)&CI);
  bool success = false;
  if(toBool(Conn))
    success = toBool(pDll::ConnProcDLL(cOpenConn, 0, Conn));
  if(success && cDirectSerial == CI.connType && CI.t1) {
    HANDLE h = (HANDLE)passThrough(cGetHandle, 0);
    DCB dcb;
    if(GetCommState(h, &dcb)) {
      delayFor485 = ((long)DEF_DELAY_4_485 * CBR_9600) / dcb.BaudRate;
      if(delayFor485 < DEF_DELAY_4_485)
        delayFor485 = DEF_DELAY_4_485;
      else if(delayFor485 > 50000)
        delayFor485 = 50000;
      }
    EscapeCommFunction(h, CLRRTS);
    }
  if(!success && Conn)
    closeAndDestroy();
  return success;
}
//----------------------------------------------------------------------------
bool PConnBase::chgLanConn(DWORD addr, DWORD port)
{
  TCHAR addrName[SIZE_A_c(CI.data) + 1];
  wsprintf(addrName, _T("%d.%d.%d.%d"), IP1(addr), IP2(addr), IP3(addr), IP4(addr));
  return chgLanConn(addrName, port);
}
//----------------------------------------------------------------------------
bool PConnBase::chgLanConn(LPCTSTR addrOrName, DWORD port)
{
  DWORD oldPort = CI.paramPort;
  CI.paramPort = port;
  TCHAR oldAddr[SIZE_A_c(CI.data) + 1];
  _tcscpy_s(oldAddr, CI.data);
  _tcscpy_s(CI.data, addrOrName);
  closeAndDestroy();
  Sleep(100);
  if(!open()) {
    CI.paramPort = oldPort;
    _tcscpy_s(CI.data, oldAddr);
    open();
    return false;
    }
  return true;
}
//----------------------------------------------------------------------------
LRESULT PConnBase::passThrough(uint codeMsg, LPDWORD p1)
{
  return pDll::ConnProcDLL(codeMsg, p1, Conn);
}
//----------------------------------------------------------------------------
HWND PConnBase::changeOwner(HWND newOwner)
{
  HWND prev = (HWND)passThrough(cChangeHwndOwner, (LPDWORD)newOwner);
  CI.Owner = newOwner;
  return prev;
}
//----------------------------------------------------------------------------
void PConnBase::removeClient(HANDLE_CONN conn)
{
  pDll::ConnProcDLL(cDestroyConn, 0, conn);
}
//----------------------------------------------------------------------------
void PConnBase::closeAndDestroy()
{
  if(Conn) {
    removeClient(Conn);
    Conn = 0;
    }
}
//----------------------------------------------------------------------------
void PConnBase::close()
{
  if(Conn)
    pDll::ConnProcDLL(cCloseConn, 0, Conn);
}
//----------------------------------------------------------------------------
bool PConnBase::isConnected()
{
  if(!Conn)
    return false;
  return toBool(passThrough(cIsConnected, 0));
}
//----------------------------------------------------------------------------
bool PConnBase::iAm(LPARAM lParam)
{
  return toBool(pDll::ConnProcDLL(cIsThis, (LPDWORD)lParam, Conn));
}
//----------------------------------------------------------------------------
DWORD PConnBase::has_string()
{
  return (DWORD)pDll::ConnProcDLL(cGetAvail, 0, Conn);
}
//----------------------------------------------------------------------------
DWORD PConnBase::read_stringNoRemove(LPBYTE buff, DWORD len)
{
  BuffConn bc;
  bc.len = len;
  bc.buff = buff;
  pDll::ConnProcDLL(cReceiveNoRemove, (LPDWORD)&bc, Conn);
  return bc.len;
}
//----------------------------------------------------------------------------
DWORD PConnBase::read_string(LPBYTE buff, DWORD len)
{
  BuffConn bc;
  bc.len = len;
  bc.buff = buff;
  pDll::ConnProcDLL(cReceiveBytes, (LPDWORD)&bc, Conn);
  return bc.len;
}
//----------------------------------------------------------------------------
static void mySleep(__int64 musec)
{
  static __int64 f = -1;
  if(!f) {
    Sleep((DWORD)(musec / 1000));
    return;
    }

  if(-1 == f) {
    LARGE_INTEGER Freq;
    if(!QueryPerformanceFrequency(&Freq))
      f = 1;
    else
       f = Freq.QuadPart / 1000000;
    if(!f)
      f = 1;
    }

  LARGE_INTEGER Count;
  if(!QueryPerformanceCounter(&Count)) {
    f = 0;
    Sleep((DWORD)(musec / 1000));
    return;
    }
  __int64 c = Count.QuadPart;
  musec *= f;
  while(true) {
    QueryPerformanceCounter(&Count);
    if(Count.QuadPart - c >= musec)
      break;
    }
}
//----------------------------------------------------------------------------
DWORD PConnBase::write_string(const BYTE* buff, DWORD len)
{
  BuffConn bc;
  bc.len = len;
  bc.buff = (LPBYTE)buff;
  if(cDirectSerial == CI.connType && CI.t1) {
    HANDLE h = (HANDLE)passThrough(cGetHandle, 0);
    EscapeCommFunction(h, SETRTS);
    pDll::ConnProcDLL(cSendBytes, (LPDWORD)&bc, Conn);
    mySleep(delayFor485);
    EscapeCommFunction(h, CLRRTS);
    }
  else
    pDll::ConnProcDLL(cSendBytes, (LPDWORD)&bc, Conn);
  return bc.len;
}
//----------------------------------------------------------------------------
bool PConnBase::reqNotify(bool req)
{
  return toBool(pDll::ConnProcDLL(cReqNotify, (LPDWORD)req, Conn));
}
//----------------------------------------------------------------------------
void PConnBase::setCallBack(pInfoCallBack pICB)
{
  pDll::ConnProcDLL(cSetCallBack, (LPDWORD)pICB, Conn);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT callBackConn(uint codeMsg, LPVOID msg, LPVOID custom)
{
  customServerLanData* pCl = (customServerLanData*)custom;
  PostMessage(pCl->Owner, WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_BY_LAN_SERVER, codeMsg), (LPARAM)msg);
  return (LRESULT)1;
}
//----------------------------------------------------------------------------
customServerLanData::~customServerLanData()
{
  delete Conn;
}
//----------------------------------------------------------------------------
bool customServerLanData::run(DWORD port)
{
  Conn = new PConnBase(port, Owner, Udp);
  bool success = Conn->open();
  // deve essere messo dopo l'apertura, perché deve avere un handle valido e deve
  // poter distinguerlo tra lan server e gli altri casi
  InfoCallBack icb = { callBackConn, this };
  Conn->setCallBack(&icb);
  return success;
}
//----------------------------------------------------------------------------
void customServerLanData::remove(HANDLE_CONN client)
{
  Conn->removeClient(client);
}
//----------------------------------------------------------------------------
