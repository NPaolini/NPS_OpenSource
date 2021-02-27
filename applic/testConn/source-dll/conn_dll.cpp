//-------------------- conn_dll.cpp -------------------------
//-----------------------------------------------------------
#ifndef MAKE_CONN_DLL
  #define MAKE_CONN_DLL 1
#endif
//-----------------------------------------------------------
#include "precompHeader.h"
//-----------------------------------------------------------
#include "pModemCom.h"
#include "pLanComm.h"
#include "SocketUtil.h"
#include "conn_dll.h"
#include "p_util.h"
#include "wm_custom.h"
//----------------------------------------------------------------------------
REG_WM(CUSTOM_MSG_CONNECTION);
//------------------------------------------------------------------
struct HandleConn
{
  LPVOID pConn;
  ConnType type;
  HWND Owner;
  DWORD p1; // porta
  UINT_PTR p2; // hwnd client per serverLan, ipRemote per clientLan
  TCHAR info1[_MAX_PATH];
  TCHAR info2[_MAX_PATH];
  HandleConn() {   ZeroMemory(this, sizeof(*this)); }
};
//------------------------------------------------------------------
typedef HandleConn* pHandleConn;
//------------------------------------------------------------------
HANDLE_CONN makeHandle(HWND owner, LPVOID conn, ConnType type, DWORD p1, UINT_PTR p2)
{
  pHandleConn ph = new HandleConn;
  ph->pConn = conn;
  ph->type = type;
  ph->Owner = owner;
  ph->p1 = p1;
  ph->p2 = p2;
  return (HANDLE_CONN)ph;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
inline void infoToHandle(pHandleConn ph, const pConnInfo pI, LPVOID conn, ConnType type)
{
  ph->pConn = conn;
  ph->Owner = pI->Owner;
  ph->p1 = pI->paramPort;
  ph->type = type;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT makeDirectSerial(pConnInfo pI)
{
  pHandleConn ph = new HandleConn;

  PCom* Com = new PCom(pI->Owner);
  paramConn pc(pI->paramPort);
  Com->setParam(pc);
  if(pI->t2) {
    PComBase::time_out to(20, pI->t2);
    Com->setTimeOut(to);
    }
  infoToHandle(ph, pI, (LPVOID)Com, cDirectSerial);
  return (LRESULT)ph;
}
//------------------------------------------------------------------
static
LRESULT makeModem(pConnInfo pI, bool call)
{
  pHandleConn ph = new HandleConn;

  PModemCom* Com = new PModemCom(pI->Owner);
  paramConn pc(pI->paramPort);
  Com->setParam(pc);
  ph->p2 = pI->t1;

  if(pI->t2) {
    PComBase::time_out to(20, pI->t2);
    Com->setTimeOut(to);
    }

  infoToHandle(ph, pI, (LPVOID)Com, call ? cModemCall : cModemWait);

  int len = _tcslen(pI->data);
  LPTSTR p = ph->info1;
  uint i = 0;
  if(call) {
    for(; i < SIZE_A(ph->info1) && pI->data[i]; ++i) {
      if(_T('#') == pI->data[i])
        break;
      *p++ = pI->data[i];
      }
    ++i;
    }
  p = ph->info2;
  for(; i < SIZE_A(ph->info1) && pI->data[i]; ++i)
    *p++ = pI->data[i];
  *p = 0;
  return (LRESULT)ph;
}
//------------------------------------------------------------------
static
LRESULT makeLanServer(pConnInfo pI, bool udp)
{
  pHandleConn ph = new HandleConn;
  customServerLanData* Com = new customServerLanData(pI->Owner, (HWND)pI->t1);
  ph->p2 = pI->t1;

  infoToHandle(ph, pI, (LPVOID)Com, udp ? cLanServerUDP : cLanServer);
  return (LRESULT)ph;
}
//------------------------------------------------------------------
static
LPCTSTR validNameAddr(LPCTSTR name)
{
  LPCTSTR p = name;
  while(*p) {
    if(_T('.') != *p && !isdigit(*p))
      return name;
    ++p;
    }
  return 0;
}
//----------------------------------------------------------------------------
static
DWORD get_Addr(LPCTSTR address)
{
  if(validNameAddr(address))
    return 0;
  int p1;
  int p2;
  int p3;
  int p4;
  _stscanf_s(address, _T("%d.%d.%d.%d"), &p1, &p2, &p3, &p4);
  DWORD addr = p1 | (p2 << 8) | (p3 << 16) | (p4 << 24);
  return addr;
}
//------------------------------------------------------------------
static
LRESULT makeLanClient(pConnInfo pI, bool udp)
{
  pHandleConn ph = new HandleConn;
  DWORD addr = get_Addr(pI->data);
  PLanComm* Com = new PLanComm(pI->paramPort, pI->Owner, udp);
  paramConn pc(addr);
  Com->setParam(pc);
  if(pI->t2) {
    PComBase::time_out to(20, pI->t2);
    Com->setTimeOut(to);
    }
  _tcscpy_s(ph->info1, SIZE_A(ph->info1), pI->data);
  ph->p2 = addr;
  infoToHandle(ph, pI, (LPVOID)Com, cLanClient);
  return (LRESULT)ph;
}
//------------------------------------------------------------------
static
LRESULT makeTypedConn(pConnInfo pI)
{
  ConnType type = (ConnType)pI->connType;
  switch(type) {
    case cDirectSerial:
      return makeDirectSerial(pI);
    case cModemCall:
      return makeModem(pI, true);
    case cModemWait:
      return makeModem(pI, false);
    case cLanServer:
      return makeLanServer(pI, false);
    case cLanServerUDP:
      return makeLanServer(pI, true);
    case cLanClient:
      return makeLanClient(pI, false);
    case cLanClientUDP:
      return makeLanClient(pI, true);
    }
  return 0;
}
//------------------------------------------------------------------
#define COPY_F(a) pt.a = pI->a
//------------------------------------------------------------------
#ifdef UNICODE
//------------------------------------------------------------------
static
LRESULT makeConn(pConnInfoA pI)
{
  ConnInfoW pt;
  COPY_F(connType);
  COPY_F(paramPort);
  COPY_F(t1);
  COPY_F(t2);
  copyStrZ(pt.data, pI->data);
  COPY_F(Owner);
  return makeTypedConn(&pt);
}
//------------------------------------------------------------------
static
LRESULT makeConn(pConnInfoW pI)
{
  return makeTypedConn(pI);
}
//------------------------------------------------------------------
#else
//------------------------------------------------------------------
static
LRESULT makeConn(pConnInfoA pI)
{
  return makeTypedConn(pI);
}
//------------------------------------------------------------------
static
LRESULT makeConn(pConnInfoW pI)
{
  ConnInfoA pt;
  COPY_F(connType);
  COPY_F(paramPort);
  COPY_F(t1);
  COPY_F(t2);
  copyStrZ(pt.data, pI->data);
  COPY_F(Owner);
  return makeTypedConn(&pt);
}
#endif // UNICODE
//------------------------------------------------------------------
#undef COPY_F
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT openDirectSerial(pHandleConn pH)
{
  PCom* Com = (PCom*)pH->pConn;
  return (LRESULT) Com->open();
}
//------------------------------------------------------------------
static
LRESULT openModem(pHandleConn pH, bool call)
{
  PModemCom* Com = (PModemCom*)pH->pConn;
  if(!Com->open())
    return 0;

  int tone = 1;
  int offs = 0;
  if(call) {
    if(!isdigit((unsigned)pH->info1[0])) {
      if(_T('P') == pH->info1[0])
        tone = 0;
      ++offs;
      }
    }
  int sleep = pH->p2;
  if(!sleep)
    sleep = 200;
  LRESULT result;
  LPTSTR p2 = pH->info2;
  if(call) {
    LPTSTR p = pH->info1 + offs;
    result = (LRESULT) Com->startModem(p, p2, tone, sleep);
    }
  else
    result = (LRESULT) Com->startModem(0, p2, tone, sleep);
  return result;
}
//------------------------------------------------------------------
static
LRESULT openLanServer(pHandleConn pH, bool udp)
{
  customServerLanData* Com = (customServerLanData*)pH->pConn;
  return (LRESULT) Com->run((WORD)pH->p1,  udp ? SOCK_DGRAM : SOCK_STREAM);
}
//------------------------------------------------------------------
static
LRESULT openLanClient(pHandleConn pH)
{
  PLanComm* Com = (PLanComm*)pH->pConn;
  LPCTSTR name = validNameAddr(pH->info1);
  return (LRESULT) Com->open(name);
}
//------------------------------------------------------------------
static
LRESULT openConn(HANDLE_CONN hc)
{
  pHandleConn ph = (pHandleConn)hc;
  switch(ph->type) {
    case cDirectSerial:
      return openDirectSerial(ph);
    case cModemCall:
      return openModem(ph, true);
    case cModemWait:
      return openModem(ph, false);
    case cLanServer:
      return openLanServer(ph, false);
    case cLanServerUDP:
      return openLanServer(ph, true);
    case cLanClient:
    case cLanClientUDP:
      return openLanClient(ph);
    }
  return 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static
void destroyDirectSerial(pHandleConn pH)
{
  PCom* Com = (PCom*)pH->pConn;
  Com->close();
  delete Com;
}
//------------------------------------------------------------------
static
void destroyModem(pHandleConn pH)
{
  PModemCom* Com = (PModemCom*)pH->pConn;
  Com->close();
  delete Com;
}
//------------------------------------------------------------------
static
void destroyLanServer(pHandleConn pH)
{
  customServerLanData* Com = (customServerLanData*)pH->pConn;
  delete Com;
}
//------------------------------------------------------------------
static
void destroyLanClient(pHandleConn pH)
{
  PLanComm* Com = (PLanComm*)pH->pConn;
  Com->close();
  delete Com;
}
//------------------------------------------------------------------
static
LRESULT destroyConn(HANDLE_CONN hc)
{
  pHandleConn ph = (pHandleConn)hc;
  switch(ph->type) {
    case cDirectSerial:
      destroyDirectSerial(ph);
      break;
    case cModemCall:
    case cModemWait:
      destroyModem(ph);
      break;
    case cLanServer:
    case cLanServerUDP:
      destroyLanServer(ph);
      break;
    case cLanClient:
    case cLanClientUDP:
      destroyLanClient(ph);
      break;
    }
  delete ph;
  return 0;
}
//------------------------------------------------------------------
static
LRESULT closeConn(HANDLE_CONN hc)
{
  pHandleConn ph = (pHandleConn)hc;
  if(ph->pConn) {
    PComBase* Com = (PComBase*)ph->pConn;
    Com->close();
    }
  return 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT getAvailCommon(pHandleConn pH)
{
  PCom* Com = (PCom*)pH->pConn;
  return (LRESULT) Com->has_string();
}
//------------------------------------------------------------------
static
LRESULT getAvail(HANDLE_CONN hc)
{
  pHandleConn ph = (pHandleConn)hc;
  switch(ph->type) {
    case cDirectSerial:
    case cModemCall:
    case cModemWait:
    case cLanClient:
    case cLanClientUDP:
      return getAvailCommon(ph);

    case cLanServer:
      return 0;
    }
  return 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT receiveCommon(pHandleConn pH, pBuffConn pbc)
{
  PCom* Com = (PCom*)pH->pConn;
  DWORD len = Com->read_string(pbc->buff, pbc->len);
  bool success = len == pbc->len;
  pbc->len = len;
  return (LRESULT) success;
}
//------------------------------------------------------------------
static
LRESULT receive(HANDLE_CONN hc, pBuffConn pbc)
{
  pHandleConn ph = (pHandleConn)hc;
  switch(ph->type) {
    case cDirectSerial:
    case cModemCall:
    case cModemWait:
    case cLanClient:
    case cLanClientUDP:
      return receiveCommon(ph, pbc);

    case cLanServer:
      return 0;
    }
  return 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT sendCommon(pHandleConn pH, pBuffConn pbc)
{
  PCom* Com = (PCom*)pH->pConn;
  DWORD len = Com->write_string(pbc->buff, pbc->len);
  bool success = len == pbc->len;
  pbc->len = len;
  return (LRESULT) success;
}
//------------------------------------------------------------------
static
LRESULT send(HANDLE_CONN hc, pBuffConn pbc)
{
  pHandleConn ph = (pHandleConn)hc;
  switch(ph->type) {
    case cDirectSerial:
    case cModemCall:
    case cModemWait:
    case cLanClient:
    case cLanClientUDP:
      return sendCommon(ph, pbc);

    case cLanServer:
      return 0;
    }
  return 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT reqNotify(HANDLE_CONN hc, bool req)
{
  pHandleConn ph = (pHandleConn)hc;
  PCom* Com = (PCom*)ph->pConn;
  return (LRESULT) Com->reqNotify(req);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT reset(HANDLE_CONN hc)
{
  pHandleConn ph = (pHandleConn)hc;
  PCom* Com = (PCom*)ph->pConn;
  Com->reset();
  return 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT isOnLine(HANDLE_CONN hc)
{
  pHandleConn ph = (pHandleConn)hc;
  if(cModemCall == ph->type || cModemWait == ph->type) {
    PModemCom* Com = (PModemCom*)ph->pConn;
    return (LRESULT)(Com->isOnLine() ? 1 : 0);
    }
  return (LRESULT) -1;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static
LRESULT receiveNoRemCommon(pHandleConn pH, pBuffConn pbc)
{
  PCom* Com = (PCom*)pH->pConn;
  DWORD len = Com->readStringNoRemove(pbc->buff, pbc->len);
  bool success = len == pbc->len;
  pbc->len = len;
  return (LRESULT) success;
}
//------------------------------------------------------------------
static
LRESULT receiveNoRemove(HANDLE_CONN hc, pBuffConn pbc)
{
  pHandleConn ph = (pHandleConn)hc;
  return receiveNoRemCommon(ph, pbc);
}
//------------------------------------------------------------------
static
LRESULT setNewHWND(HANDLE_CONN hc, HWND hwnd)
{
  bool oldReq = toBool(reqNotify(hc, false));
  pHandleConn ph = (pHandleConn)hc;
  HWND old = ph->Owner;
  ph->Owner = hwnd;
  if(cLanServer != ph->type) {
    PCom* Com = (PCom*)ph->pConn;
    Com->setNewHWND(hwnd);
    }
  reqNotify(hc, oldReq);
  return (LRESULT) old;
}
//------------------------------------------------------------------
static
LRESULT isConnected(HANDLE_CONN hc)
{
  pHandleConn ph = (pHandleConn)hc;
  PComBase* Com = (PComBase*)ph->pConn;
  if(!Com)
    return 0;
  switch(ph->type) {
    case cDirectSerial:
    case cModemCall:
    case cModemWait:
      return Com->getIdCom() && INVALID_HANDLE_VALUE != Com->getIdCom();
    case cLanServer:
    case cLanClient:
    case cLanServerUDP:
    case cLanClientUDP:
      return Com->getIdCom() && INVALID_SOCKET != (SOCKET)Com->getIdCom() && INVALID_HANDLE_VALUE != Com->getIdCom();
    }
  return 0;
}
//------------------------------------------------------------------
static
LRESULT isThis(HANDLE_CONN hc, HANDLE idCode)
{
  pHandleConn ph = (pHandleConn)hc;
  PCom* Com = (PCom*)ph->pConn;
  return (LRESULT)(Com->getIdCom() == idCode);
}
//------------------------------------------------------------------
static
LRESULT getAddrPort(HANDLE_CONN hc, LPDWORD buff, bool remote)
{
  pHandleConn ph = (pHandleConn)hc;
  SOCKET sock;
  if(!remote && cLanServer == ph->type)
    sock = ((customServerLanData*)ph->pConn)->getSocket();
  else if(cLanClient == ph->type || cLanClientUDP == ph->type)
    sock = ((PLanComm*)ph->pConn)->getSocket();
  else
    return (LRESULT)-1;

  if(getAddrAndPort(sock, buff[0], buff[1], remote))
    return (LRESULT)1;

  return (LRESULT)-1;
}
//------------------------------------------------------------------
#ifdef UNICODE
//------------------------------------------------------------------
static
LRESULT getName(HANDLE_CONN hc, LPWSTR buff, bool remote)
{

  pHandleConn ph = (pHandleConn)hc;
  SOCKET sock;
  if (!remote && cLanServer == ph->type)
    sock = ((customServerLanData*)ph->pConn)->getSocket();
  else if (cLanClient == ph->type)
    sock = ((PLanComm*)ph->pConn)->getSocket();
  else
    return (LRESULT)-1;
  if (getConnName(sock, buff, remote))
    return (LRESULT)1;
  return (LRESULT)-1;
}
//------------------------------------------------------------------
static
LRESULT getName(HANDLE_CONN hc, LPSTR buff, bool remote)
{
  wchar_t tmp[_MAX_PATH];
  LRESULT result = getName(hc, tmp, remote);
  if ((LRESULT)-1 != result)
    copyStrZ(buff, tmp);

  return result;
}
#else
static
LRESULT getName(HANDLE_CONN hc, LPSTR buff, bool remote)
{
  pHandleConn ph = (pHandleConn)hc;
  SOCKET sock;
  if (!remote && cLanServer == ph->type)
    sock = ((customServerLanData*)ph->pConn)->getSocket();
  else if (cLanClient == ph->type)
    sock = ((PLanComm*)ph->pConn)->getSocket();
  else
    return (LRESULT)-1;
  if (getConnName(sock, buff, remote))
    return (LRESULT)1;
  return (LRESULT)-1;
}
//------------------------------------------------------------------
static
LRESULT getName(HANDLE_CONN hc, LPWSTR buff, bool remote)
{
  char tmp[_MAX_PATH];
  LRESULT result = getName(hc, tmp, remote);
  if ((LRESULT)-1 != result)
    copyStrZ(buff, tmp);

  return result;
}
#endif
//--------------------------------------------------------------------------------
#define IP1(a) (((a) >> 24) & 0xff)
#define IP2(a) (((a) >> 16) & 0xff)
#define IP3(a) (((a) >> 8) & 0xff)
#define IP4(a) (((a) >> 0) & 0xff)
//------------------------------------------------------------------
static
LRESULT getAddrPortString(HANDLE_CONN pConn, LPWSTR buff, bool remote)
{
  DWORD t[2];
  LRESULT result = getAddrPort(pConn, t, remote);
  if(-1 == (long)result)
    return result;
#ifndef UNICODE
  char tmp[64];
  wsprintf(tmp, "%d.%d.%d.%d:%d", IP1(t[0]), IP2(t[0]), IP3(t[0]), IP4(t[0]), t[1]);
  copyStrZ(buff, tmp);
#else
  wsprintf(buff, _T("%d.%d.%d.%d:%d"), IP1(t[0]), IP2(t[0]), IP3(t[0]), IP4(t[0]), t[1]);
#endif
  return result;
}
//------------------------------------------------------------------
static
LRESULT getAddrPortString(HANDLE_CONN pConn, LPSTR buff, bool remote)
{
  DWORD t[2];
  LRESULT result = getAddrPort(pConn, t, remote);
  if(-1 == (long)result)
    return result;
#ifdef UNICODE
  wchar_t t2[64];
  wsprintf(t2, _T("%d.%d.%d.%d:%d"), IP1(t[0]), IP2(t[0]), IP3(t[0]), IP4(t[0]), t[1]);
  copyStrZ(buff, t2);
#else
  wsprintf(buff, _T("%d.%d.%d.%d:%d"), IP1(t[0]), IP2(t[0]), IP3(t[0]), IP4(t[0]), t[1]);
#endif
  return result;
}
//------------------------------------------------------------------
static
LRESULT setCallBack(HANDLE_CONN hc, pInfoCallBack pICB)
{
  pHandleConn ph = (pHandleConn)hc;
  switch(ph->type) {
    case cLanServer:
    case cLanServerUDP:
      do {
        customServerLanData* Com = (customServerLanData*)ph->pConn;
        Com->setCallBack(*pICB);
        return (LRESULT) 1;
        } while(false);
      break;
    }
  return (LRESULT) -1;
}
//------------------------------------------------------------------
static
LRESULT getTrueHandle(HANDLE_CONN hc)
{
  pHandleConn ph = (pHandleConn)hc;
  return (LRESULT)((PComBase*)ph->pConn)->getIdCom();
}
//------------------------------------------------------------------
CONN_IMPORT_EXPORT
LRESULT __stdcall ConnProcDLLstd(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn)
{
  return ConnProcDLL(codeMsg, p1, pConn);
}
//------------------------------------------------------------------
CONN_IMPORT_EXPORT
LRESULT ConnProcDLL(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn)
{
  if(!pConn)
    return 0;
  switch(codeMsg) {
    case cMakeConnA:
      return makeConn((pConnInfoA)pConn);
    case cMakeConnW:
      return makeConn((pConnInfoW)pConn);

    case cOpenConn:
      return openConn(pConn);

    case cCloseConn:
      return closeConn(pConn);

    case cDestroyConn:
      return destroyConn(pConn);

    case cGetAvail:
      return getAvail(pConn);

    case cReceiveBytes:
      return receive(pConn, (pBuffConn)p1);

    case cSendBytes:
      return send(pConn, (pBuffConn)p1);

    case cReqNotify:
      return reqNotify(pConn, toBool(p1));

    case cReset:
      return reset(pConn);

    case cIsThis:
      return isThis(pConn, (HANDLE)p1);

    case cIsOnLine:
      return isOnLine(pConn);

    case cReceiveNoRemove:
      return receiveNoRemove(pConn, (pBuffConn)p1);

    case cChangeHwndOwner:
      return setNewHWND(pConn, (HWND)p1);

    case cIsConnected:
      return isConnected(pConn);

    case cGetLocalAddrAndPort:
      return getAddrPort(pConn, p1, false);

    case cGetLocalAddrAndPortStringA:
      return getAddrPortString(pConn, (LPSTR)p1, false);
    case cGetLocalAddrAndPortStringW:
      return getAddrPortString(pConn, (LPWSTR)p1, false);

    case cGetRemoteAddrAndPort:
      return getAddrPort(pConn, p1, true);

    case cGetRemoteAddrAndPortStringA:
      return getAddrPortString(pConn, (LPSTR)p1, true);
    case cGetRemoteAddrAndPortStringW:
      return getAddrPortString(pConn, (LPWSTR)p1, true);

    case cGetLocalNameA:
      return getName(pConn, (LPSTR)p1, false);
    case cGetLocalNameW:
      return getName(pConn, (LPWSTR)p1, false);

    case cGetRemoteNameA:
      return getName(pConn, (LPSTR)p1, true);
    case cGetRemoteNameW:
      return getName(pConn, (LPWSTR)p1, true);

    case cSetCallBack:
      return setCallBack(pConn, (pInfoCallBack)p1);

    case cGetHandle:
      return getTrueHandle(pConn);
    case cFromExplicitToDWord:
      do {
        paramConn pc((paramConn::ePort)(p1[0]), (paramConn::eBaudRate)(p1[1]), (paramConn::eBitStop)(p1[2]), (paramConn::eParity)(p1[3]),
            (paramConn::eBitLen)(p1[4]), (paramConn::eDtrDsr)(p1[5]), (paramConn::eRtsCts)(p1[6]), (paramConn::eXonXoff)(p1[7]));
        DWORD r = pc;
        return r;
        } while(false);
    }
  return 0;
}
//------------------------------------------------------------------
