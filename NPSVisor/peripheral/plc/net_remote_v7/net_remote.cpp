//----------- net_remote.CPP ------------------------------
//---------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------
#include <stdlib.h>

#include "net_remote.h"
#include "p_file.h"
#include "hdrmsg.h"
#include "commidt.h"
#include "prfData.h"

#include "p_txt.h"
#include "conn_dll.h"
#include "p_matchStream.h"
//---------------------------------------------------------
/*
  all'avvio carica i dati necessari ad avviare la connessione
  nel thread di connessione prova ad avviare la connessione, se non
  riesce imposta un timeout per riprovare dopo un tot di tempo.
  Nel thread crea un oggetto window per gestire la segnalazione di
  dati presenti nel buffer.
  Se cade la connessione si chiude la connessione e si
  reimposta il timeout per riprovare.
*/
//----------------------------------------------------------------------------
#define IP1(a) (((a) >> 24) & 0xff)
#define IP2(a) (((a) >> 16) & 0xff)
#define IP3(a) (((a) >> 8) & 0xff)
#define IP4(a) (((a) >> 0) & 0xff)
//----------------------------------------------------------------------------
inline
void makeIpStr(LPTSTR buff, DWORD ip)
{
    wsprintf(buff, _T("%d.%d.%d.%d"), IP1(ip), IP2(ip), IP3(ip), IP4(ip));
}
//---------------------------------------------------------
gestComm *allocGestComm(void* par, uint idPrf)
{
  PWin * w = reinterpret_cast<PWin*>(par);
  if(w) {
    TCHAR buff[100];
    wsprintf(buff, _T("net_remote V7 [%d]"), idPrf);
    w->setCaption(buff);
    }
  TCHAR name[_MAX_PATH];
  MK_NAME_FILE_DATA(name, idPrf);
  return new net_remote(w, name, FLAG_PERIF(idPrf));
}
//-----------------------------------------------------
#define DEF_FILE_CONN _T("infoServer.npt")
//---------------------------------------------------------
net_remote::net_remote(PWin* owner, LPCTSTR file, WORD flagCommand) :
    gestCommgPerif(file, flagCommand), Owner(owner), hEventClose(0), hEventSendChar(0),
    errorByThread(0), idThread(0), Addr4IpRemote(-1),
    realErrorReceive(0), realErrorSend(0), realTotCount(0), Step(0), connState(ncs_none)
#ifdef USE_AVL_DUAL
    ,dualSet(true)
#endif
{
  ZeroMemory(&iServer, sizeof(iServer));
  ZeroMemory(&iDynIp, sizeof(iDynIp));
  ZeroMemory(Buff_Thread, sizeof(Buff_Thread));
  ZeroMemory(keySend, sizeof(keySend));
  commIdentity* ci = getIdentity();
  Step = ci->getData();
  if(!Step)
    Step = getTypeSize();

  setOfString set(DEF_FILE_CONN);
  LPCTSTR p = set.getString(ci->getNum());
  p = findNextParamTrim(p, 5);
  if(p && _ttoi(p))
    Step = 1;
}
//---------------------------------------------------------
net_remote::~net_remote()
{
  if(hEventClose) {
    for(;;) {
      SetEvent(hEventClose);
      if(!idThread)
        break;
      Sleep(300);
      }
    CloseHandle(hEventClose);
    hEventClose = 0;
    }
  if(hEventSendChar) {
    CloseHandle(hEventSendChar);
    hEventSendChar = 0;
    }
}
//-----------------------------------------------------
// maxLen non contempla lo zero, quindi target deve avere spazio anche per quello
static void copyPart(LPTSTR target, LPCTSTR source, uint maxLen)
{
  for(uint i = 0; i < maxLen; ++i) {
    if(!*source || _T(',') == *source)
      break;
    *target++ = *source++;
    }
  *target = 0;
}
//-----------------------------------------------------
gestComm::ComResult net_remote::Init()
{
  gestComm::ComResult res = baseClass::Init();
  if(gestComm::NoErr > res)
    return res;

  do {
    P_File pfl(FileName, P_READ_ONLY);
    if(pfl.P_open())
      pfl.P_read(Buff_Thread, MAX_DIM_PERIF);
    } while(false);

  commIdentity* ci = getIdentity();
  setOfString set(DEF_FILE_CONN);
  LPCTSTR p = set.getString(ci->getNum());
  if(!p)
    return gestComm::OpenComErr;

  srand(GetTickCount() * ci->getNum());

  pvvChar v;
  int nParam = splitParam(v, p);
  while(nParam >= 4) {
    iServer.id_prph_req = _ttoi(&v[0]);
    if(!iServer.id_prph_req)
      iServer.id_prph_req = ci->getNum();
    else {
      TCHAR buff[100];
      wsprintf(buff, _T("net_remote [%d -> %d]"), ci->getNum(), iServer.id_prph_req);
      Owner->setCaption(buff);
      }

    copyStrZ(iServer.Addr[0].ip, &v[1], SIZE_A(iServer.Addr[0].ip) - 1);
    iServer.Addr[0].port = _ttoi(&v[2]);
    copyStrZ(iServer.psw, &v[3], SIZE_A(iServer.psw) - 1);
    if(nParam > 4)
      iServer.readOnly = toBool(_ttoi( &v[4]));

    p = set.getString(ci->getNum() + OFFSET_DYN_IP);
    if(p) {
      pvvChar vip;
      int nParam = splitParam(vip, p);
      if(nParam >= 3) {
        copyStrZ(iDynIp.hostname, &vip[0], SIZE_A(iDynIp.hostname) - 1);
        copyStrZ(iDynIp.username, &vip[1], SIZE_A(iDynIp.username) - 1);
        copyStrZ(iDynIp.password, &vip[2], SIZE_A(iDynIp.password) - 1);
        }
      }
    p = set.getString(ci->getNum() + OFFSET_DYN_IP * 2);
    if(p)
      Addr4IpRemote = _ttoi(p);

    uint idAltern = 1 + ci->getNum() * 1000;
    for(uint i = 1;; ++i, ++idAltern) {
      p = set.getString(idAltern);
      if(!p)
        break;
      iServer.Addr[i].port = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p) {
        iServer.Addr.setElem(i);
        break;
        }
      copyPart(iServer.Addr[i].ip, p, SIZE_A(iServer.Addr[0].ip) - 1);
      }

    hEventClose = CreateEvent(0, TRUE, 0, 0);
    if(!hEventClose)
      break;
    hEventSendChar = CreateEvent(0, 0, 0, 0);
    if(!hEventSendChar)
      break;

    HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReaderClientProc, this, 0, &idThread);
    if(!hThread)
      break;
    CloseHandle(hThread);
    return gestComm::NoErr;
    }
  idThread = 0;
  if(hEventClose) {
    CloseHandle(hEventClose);
    hEventClose = 0;
    }
  return gestComm::OpenComErr;
}
//-------------------------------------------------------
void net_remote::copyRemoteIp(LPCTSTR ip)
{
  if(Addr4IpRemote >= 0 && Addr4IpRemote < MAX_DWORD_PERIF - MAX_DWORD_IP) {
    criticalLock crtLckW(criticalSectionBuff);
    char ip2[8 + sizeof(DWORD) * MAX_DWORD_IP];
    for (uint i = 0; i < sizeof(DWORD) * MAX_DWORD_IP; ++i)
      ip2[i] = (char)ip[i];
    LPDWORD pdw = (LPDWORD)ip2;
    LPDWORD pdw1 = (LPDWORD)Buff_Lav;
    pdw1 += Addr4IpRemote;
    LPDWORD pdw2 = (LPDWORD)Buff_Perif;
    pdw2 += Addr4IpRemote;
    for(uint i = 0; i < MAX_DWORD_IP; ++i)
      pdw1[i] = pdw2[i] = pdw[i];
    }
}
//-------------------------------------------------------
void net_remote::makeDual(setOfString& set)
{
  const uint nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
  set.setFirst();
  alternAddress tmpAddr;
  do {
    if(set.getCurrId() >= nElem)
      break;
    LPCTSTR p = set.getCurrString();
    stringToAddr(1, 1, p, &tmpAddr);

#ifdef USE_AVL_DUAL
    dual* d = new dual(tmpAddr.addr, tmpAddr.ipAddr, tmpAddr.port, tmpAddr.db, set.getCurrId() - 1);
    dualSet.Add(d);
#else
    dual d(tmpAddr.addr, tmpAddr.ipAddr, tmpAddr.port, tmpAddr.db, set.getCurrId() - 1);
    dualSet.insert(d);
#endif
    } while(set.setNext());

  set.setFirst();
}
//-------------------------------------------------------
void net_remote::verifySet(setOfString& set)
{
  // se ci sono dati esiste un file .adr, si usa quello
  if(set.setFirst()) {
    makeDual(set);
    return;
    }
  // altrimenti occorre creare tutto il set di indirizzi ed il codice per il tipo di dato
  set.replaceString(ID_DATATYPE, str_newdup(_T("4")), true);
  set.replaceString(ID_VER_ADR, str_newdup(CUR_VERSION_TXT), true);

  int nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
  TCHAR buff[250];
  for(int i = 0; i < nElem; ++i) {
    wsprintf(buff, _T("0,%d,4,1,0,0,0,0,0,0"), i);
    set.replaceString(i + 1, str_newdup(buff), true);
    }
  makeDual(set);
}
//-------------------------------------------------------
gestComm::ComResult net_remote::Send(const addrToComm* pAddr, Perif_TypeCommand t)
{
  return gestComm::noAction;
}
//-------------------------------------------------------
bool net_remote::getDualAddr(const addrToComm* pAddr, DWORD& laddr, DWORD offset)
{
  dual d(pAddr->addr + offset, pAddr->ipAddr, pAddr->port, pAddr->db);
#ifdef USE_AVL_DUAL
  if(dualSet.Search(&d)) {
    laddr = dualSet.getCurrData()->logicAddr;
    return true;
    }
#else
  uint pos;
  if(dualSet.find(d, pos)) {
    laddr = dualSet[pos].logicAddr;
    return true;
    }
#endif
  return false;
}
//-------------------------------------------------------
gestComm::ComResult net_remote::SendData(addrToComm* pAddr)
{
  if(Logoff)
    return gestComm::NoErr;
  if(iServer.readOnly)
    return gestComm::SendErr;

  uint size = GET_NBYTE(pAddr->type);
  if(!size || prfData::tStrData == pAddr->type)
    size = getTypeSize();
  do {
    criticalLock crtLckW(criticalSectionSend);
    uint nElem = CmdSet.getElem();
    LPBYTE source = pAddr->buff;
    uint trueStep = 1 == Step ? 1 : size;
    uint offset = 0;
    for(uint i = 0; i < pAddr->len; ++i, offset += trueStep) {
      infocmd* id = getFreeCmdData();
      if(!getDualAddr(pAddr, id->addr, offset)) {
        releaseCmdData(id);
        continue;
        }
      id->numdata = 1;
      id->typevar = pAddr->type;
      memcpy(id->data, source, size);
      source += size;
      CmdSet[nElem++] = id;
      }
    SetEvent(hEventSendChar);
    } while(false);

    do {
    criticalLock crtLckW(criticalSectionCount);
    if(errorByThread) {
      --errorByThread;
      ++realErrorSend;
      return gestComm::SendErr;
      }
    } while(false);
  ++realTotCount;
  return gestComm::NoErr;
}
//-------------------------------------------------------
gestComm::ComResult net_remote::Receive(const addrToComm* pAddr, BDATA* buff)
{
  if(Logoff)
    return gestComm::NoErr;

  uint size = GET_NBYTE(pAddr->type);
  if(!size || prfData::tStrData == pAddr->type)
    size = getTypeSize();
  uint trueStep = 1 == Step ? 1 : size;
  do {
    criticalLock crtLckW(criticalSectionBuff);
    uint offset = 0;
    for(uint i = 0; i < pAddr->len; ++i, offset += trueStep) {
      DWORD laddr;
      DWORD v = 0;
      if(getDualAddr(pAddr, laddr, offset))
        v = Buff_Thread[laddr];
      memcpy(buff, &v, size);
      buff += size;
      }
    } while(false);

  do {
    criticalLock crtLckW(criticalSectionCount);
    if(errorByThread) {
      --errorByThread;
      ++realErrorReceive;
      return gestComm::RecErr;
      }
    } while(false);
  ++realTotCount;
  return gestComm::NoErr;
}
//-----------------------------------------------------
bool net_remote::manageCounter(int& countSend, int& countRec, int& countTot, int& consErr)
{
  criticalLock crtLck(criticalSectionCount);
  countRec = realErrorReceive;
  countSend = realErrorSend;
  countTot = realTotCount;
  // il driver non deve riavviarsi, ci pensa da solo a riprovare la connessione
  consErr = 0;
  return true;
}
//-----------------------------------------------------------
#define MAX_DIM_CHECK (sizeof(SV_MAGIC) + 2)
//-----------------------------------------------------------
#define MAX_WAIT_TIMEOUT (20 * 1000)
//-----------------------------------------------------------
#define WAIT_TIME 50
#define WAIT_REPEAT (MAX_WAIT_TIMEOUT / WAIT_TIME)
//-----------------------------------------------------------
class consumer : public p_matchStream<BYTE, MAX_DIM_CHECK>
{
  private:
    typedef p_matchStream<BYTE, MAX_DIM_CHECK> baseClass;
  public:
    consumer(PConnBase& conn) : Conn(conn) { }
  protected:
    virtual bool has_data();
    virtual void read_one_data(BYTE& buff);
    virtual BYTE converter(BYTE readed) { return readed; }
  private:
    PConnBase& Conn;
};
//-----------------------------------------------------------
bool consumer::has_data()
{
  for(uint i = 0; i < WAIT_REPEAT; ++i) {
    if(Conn.has_string())
      return true;
    if(!Conn.isConnected())
      return false;
    Sleep(WAIT_TIME);
    }
  return false;
}
//-----------------------------------------------------------
void consumer::read_one_data(BYTE& buff)
{
  Conn.read_string(&buff, 1);
}
//-----------------------------------------------------
static bool matchMagic(LPBYTE magic, uint len, PConnBase* Conn)
{
  consumer cons(*Conn);
  return cons.checkMatch(magic, len) >= 0;
/*
// n.b. non devono esserci sequenze ripetute nel magic
  uint match = 0;
  BYTE t;
  int dim;
  do {
    Conn->read_string(&t, 1);
    if(magic[match] == t)
      ++match;
    else if(magic[0] == t)
      match = 1;
    dim = Conn->has_string();
    } while(dim && match < len);

  return match == len;
*/
}
//-----------------------------------------------------
bool net_remote::thReadConn(PConnBase* Conn)
{
  int dim = Conn->has_string();
  if(!dim)
    return Conn->isConnected();
  DWORD magic = SV_MAGIC;
  if(!matchMagic(LPBYTE(&magic), sizeof(magic), Conn))
    return true;

  info_modified_data* packet = getFreeData();
  header& Head = packet->Head;

  Head.Magic = SV_MAGIC;
  Conn->read_string(((LPBYTE)&Head) + sizeof(SV_MAGIC), sizeof(Head) - sizeof(SV_MAGIC));

  bool success = false;
  do {
    if(Head.Len + sizeof(Head) > MAX_DIM_BLOCK)
      break;
    if(Head.Len != Conn->read_string(((LPBYTE)packet) + sizeof(Head), Head.Len))
      break;
    if(Head.ChkSum != Head.getChkSum(&Head))
      break;
    success = true;
    } while(false);

  if(!success) {
    releaseFreeData(packet);
    return true;
    }

  switch(Head.Code) {
    case header::hVarChg:
      thFillChanged(packet);
      break;
    case header::hChgServer:
      thChangeConn((info_cmd_chgserver*)packet, Conn);
      break;
    }
  releaseFreeData(packet);
  return true;
}
//------------------------------------------------------------------
void net_remote::thChangeConn(info_cmd_chgserver* packet, PConnBase* Conn)
{
  uint nElem = iServer.Addr.getElem();
  bool found = false;
  TCHAR ip[24];
  makeIpStr(ip, reverseIP(packet->Ip));
  for(uint i = 0; i < nElem; ++i) {
    // se già esiste, usiamo forzatamente la porta memorizzata, magari l'altro server ne usa una diversa
    if(!_tcsicmp(iServer.Addr[i].ip, ip)) {
      iServer.currServer = i;
      found = true;
      packet->Port = iServer.Addr[i].port;
      break;
      }
    }
  // accettiamo solo quelli presenti nel file txt
  if(!found)
    return;
  if(Conn->chgLanConn(packet->Ip, packet->Port)) {
#if 1
    copyRemoteIp(ip);
#else
    const ConnInfo& ci = Conn->getInfo();
    copyRemoteIp(ci.data);
#endif
    connState = ncs_Actived;
    }
  if(Conn->isConnected())
    thSendAck(Conn);
  else
    resetRemoteIp();
}
//------------------------------------------------------------------
void net_remote::thFillChanged(info_modified_data* packet)
{
  header* pHead = &packet->Head;
  if(pHead->ChkSum != pHead->getChkSum(pHead))
    return;

  infodata* curr = packet->data;

  criticalLock crtLckW(criticalSectionBuff);
  LPCBYTE kSend = getKeySend();
  for(uint i = 0; i < packet->numinfodata; ++i) {
    if(curr->addr + curr->numdata > MAX_DWORD_PERIF || !curr->numdata)
      break;
    cryptBufferNoZ((LPBYTE)(Buff_Thread + curr->addr), (LPCBYTE)curr->data, curr->numdata * sizeof(DWORD), kSend, DIM_KEY_SEND, 1);
    curr = (infodata*)(LPBYTE(curr) + sizeof(infodata) + (curr->numdata - 1) * sizeof(curr->data[0]));
    }
}
//------------------------------------------------------------------
static
void initializeData(info_cmd_data* data)
{
  data->Head = header(header::hVarToWrite);
  data->numinfodata = 0;
}
//------------------------------------------------------------------
static
void finalizeData(info_cmd_data* data, uint dim)
{
  data->Head.Len = dim - sizeof(data->Head);
  data->Head.makeChkSum();
}
//------------------------------------------------------------------
static
bool addInfo(info_cmd_data* data, const infocmd* toAdd, LPBYTE& lastPtr, LPCBYTE key)
{
  infocmd* curr = (infocmd*)lastPtr;
  uint onAdd = sizeof(*toAdd) + sizeof(DWORD) * (toAdd->numdata - 1);
  if(((LPBYTE)curr - (LPBYTE)data) + onAdd >= MAX_BUFF_DATA)
    return false;
  *curr = *toAdd;
  cryptBufferNoZ((LPBYTE)curr->data, (LPCBYTE)(toAdd->data), toAdd->numdata * sizeof(DWORD), key, DIM_KEY_SEND, 1);
  lastPtr = (LPBYTE)curr + sizeof(*toAdd) + sizeof(DWORD) * (toAdd->numdata - 1);
  ++data->numinfodata;
  return true;
}
//------------------------------------------------------------------
static
uint fillData(info_cmd_data* data, PVect<infocmd*>& set, uint lastpos, LPBYTE& lastPtr, LPCBYTE key)
{
  uint i = lastpos;
  uint nElem = set.getElem();
  for(; i < nElem; ++i) {
    if(!addInfo(data, set[i], lastPtr, key))
      break;
    }
  return i;
}
//----------------------------------------------------------------------------
bool net_remote::thWriteConn(PConnBase* Conn)
{
  uint nElem = CmdSet.getElem();
  if(!nElem)
    return Conn->isConnected();

  info_cmd_data* data = getFreeInfoCmdData();

  uint lastpos = 0;
  LPCBYTE kSend = getKeySend();
  while(lastpos < nElem) {
    LPBYTE lastPtr = (LPBYTE)data->data;
    initializeData(data);
    lastpos = fillData(data, CmdSet, lastpos, lastPtr, kSend);
    uint dimBlock = lastPtr - (LPBYTE)data;
    finalizeData(data, dimBlock);
    if(dimBlock != Conn->write_string((LPBYTE)data, dimBlock))
      break;
    }
  setFreeItem(CmdSet);
  releaseInfoCmdData(data);
  return lastpos >= nElem;
}
//----------------------------------------------------------------------------
static void transformKeySend(LPBYTE target, LPBYTE source, int l)
{
  crypt_noZ(source, target, 0xcafebabe, l);
}
//----------------------------------------------------------------------------
void net_remote::thSendAck(PConnBase* Conn)
{
  handshake hs;
  hs.Head.Code = header::hPsw;
  hs.Head.Len = sizeof(hs) - sizeof(hs.Head);
  hs.prph = iServer.id_prph_req;
  LPWORD p = (LPWORD)hs.key;
  *p++ = rand();
  *p++ = rand();
  *p++ = rand();
  *p = rand();
  transformKeySend(keySend, hs.key, DIM_KEY_SEND);
  hs.makePsw(iServer.psw);
  hs.Head.makeChkSum();
  Conn->write_string((LPBYTE)&hs, sizeof(hs));
}
//----------------------------------------------------------------------------
void net_remote::thSetErrThread()
{
  criticalLock crtLck(criticalSectionCount);
  errorByThread = (DWORD)-1;
}
//----------------------------------------------------------------------------
void net_remote::thResetErrThread()
{
  criticalLock crtLck(criticalSectionCount);
  errorByThread = 0;
}
//----------------------------------------------------------------------------
void net_remote::resetRemoteIp()
{
  if(connState != ncs_Down) {
    connState = ncs_Down;
    TCHAR t[sizeof(DWORD) * MAX_DWORD_IP] = {0};
    copyRemoteIp(t);
    }
}
//----------------------------------------------------------------------------
void net_remote::showRemoteIp(PConnBase* Conn)
{
  if(connState != ncs_Actived) {
    connState = ncs_Actived;
    const ConnInfo& ci = Conn->getInfo();
    copyRemoteIp(ci.data);
    }
}
//---------------------------------------------------------
class PClientWinThread : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PClientWinThread();
    ~PClientWinThread();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HANDLE hEventHasChar;
    HANDLE hEventDisconnected;
    friend unsigned FAR PASCAL ReaderClientProc(void*);
    virtual LPCTSTR getClassName() const { return _T("svPrphNetRemote"); }
};
//----------------------------------------------------------------------------
PClientWinThread::PClientWinThread() :
      baseClass(0, 0, 0),
      hEventHasChar(CreateEvent(0, 0, 0, 0)),
      hEventDisconnected(CreateEvent(0, 0, 0, 0))
{
  Attr.style = WS_OVERLAPPEDWINDOW;
}
//----------------------------------------------------------------------------
PClientWinThread::~PClientWinThread()
{
  CloseHandle(hEventHasChar);
  CloseHandle(hEventDisconnected);
  destroy();
}
//----------------------------------------------------------------------------
LRESULT PClientWinThread::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_CUSTOM_MSG_CONNECTION == message) {
    switch(LOWORD(wParam)) {
      case CM_MSG_HAS_CHAR:
        SetEvent(hEventHasChar);
        break;
      default:
        if(0 == HIWORD(wParam))
          SetEvent(hEventDisconnected);
        break;
      }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------
class makeTrueConn
{
  public:
    makeTrueConn(infoServer& is, const infoDynamicIP_Req& ir, PWin* W) : Is(is), Ir(ir), W(W) {}
    PConnBase* makeConn(PConnBase* old, net_remote* net_r);
  private:
    infoServer& Is;
    const infoDynamicIP_Req& Ir;
    PWin* W;
//----------------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <pshpack2.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(push, 2)

#else
  #error Occorre definire l'allineamento a 2 byte per le strutture seguenti
#endif

    struct packetReqIp {
      header Head;
      infoDynamicIP_Req req;
      packetReqIp() : Head(header::hDynamicIP_Request, sizeof(req)) {}
      };
    struct packetReqIpResponce {
      DWORD ip;
      DWORD port;
      packetReqIpResponce() : ip(0), port(0) {}
      };
    struct packetReceiveIp {
      header Head;
      packetReqIpResponce resp;
      packetReceiveIp() : Head(header::hDynamicIP_Request, sizeof(resp)) {}
      };
//----------------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <poppack.h>
#elif defined(_MSC_VER)
  #pragma pack(pop)
#endif
//----------------------------------------------------------------------------
    int readConn(PConnBase* Conn, packetReqIpResponce& resp);
};
//----------------------------------------------------------------------------
PConnBase* makeTrueConn::makeConn(PConnBase* old, net_remote* net_r)
{
  net_r->resetRemoteIp();
  if(!Ir.hostname[0]) {
    // se c'è un solo server, inutile distruggere la connessione
    if(old && 1 == Is.Addr.getElem())
      return old;
    delete old;
    if(old) {
      ++Is.currServer;
      if(Is.currServer >= Is.Addr.getElem())
        Is.currServer = 0;
      }
    return new PConnBase(Is.Addr[Is.currServer].ip, Is.Addr[Is.currServer].port, *W, false);
    }

  delete old;
  int result = 0;
  packetReqIpResponce resp;
  do {
    PConnBase conn(Is.Addr[0].ip, Is.Addr[0].port, 0, false);
    if(!conn.open())
      return 0;

    packetReqIp pIp;
    pIp.req = Ir;
    conn.write_string((LPBYTE)&pIp, sizeof(pIp));
    for(uint i = 0; !result && i < 1000; ++i) {
      result = readConn(&conn, resp);
      Sleep(100);
      }
    } while(false);

  if(result > 0)
    return new PConnBase(resp.ip, resp.port, *W, false);
  return 0;
}
//-----------------------------------------------------
int makeTrueConn::readConn(PConnBase* Conn, packetReqIpResponce& resp)
{
  if(!Conn->isConnected())
    return -1;
  int dim = Conn->has_string();
  if(!dim)
    return 0;
  DWORD magic = SV_MAGIC;
  if(!matchMagic(LPBYTE(&magic), sizeof(magic), Conn))
    return -1;

  packetReceiveIp packet;
  header& Head = packet.Head;

  Head.Magic = SV_MAGIC;
  Conn->read_string(((LPBYTE)&Head) + sizeof(SV_MAGIC), sizeof(Head) - sizeof(SV_MAGIC));

  bool success = false;
  do {
    if(Head.Len + sizeof(Head) > sizeof(packet))
      break;
    if(Head.Len != Conn->read_string((LPBYTE)&packet.resp, Head.Len))
      break;
    if(Head.ChkSum != Head.getChkSum())
      break;
    success = true;
    } while(false);

  if(!success)
    return -1;

  switch(Head.Code) {
    case header::hDynamicIP_Request:
      resp = packet.resp;
      break;
    default:
      return -1;
    }
  return 1;
}
//---------------------------------------------------------
#define RECHECK_TIMEOUT_WAIT_CONN (1000 * 1)
#define CHECK_ON_TIMEOUT_WAIT_CONN (1000 * 1)

#define PREDEF_TIMEOUT_WAIT_CONN (1000 * 1)
//---------------------------------------------------------
unsigned FAR PASCAL ReaderClientProc(void* cl)
{
  net_remote* Cl = reinterpret_cast<net_remote*>(cl);

  PClientWinThread* W = new PClientWinThread();

  if(!W->create()) {
    delete W;
    return 0;
    }

  HANDLE HandlesToWaitFor[] = {
    W->hEventHasChar,
    Cl->hEventSendChar,
    W->hEventDisconnected,
    Cl->hEventClose
    };

  ResetEvent(Cl->hEventClose);

  // la prima volta deve entrare subito per aprire la connessione
  DWORD timeout = 0;
  bool terminate = false;
  infoServer& iServer = Cl->iServer;
  const infoDynamicIP_Req& iReq = Cl->iDynIp;
  PConnBase* Conn = makeTrueConn(iServer, iReq, W).makeConn(0, Cl);
  PostMessage(*W, WM_NULL, 0, 0);
  bool onClosed = true;
  while(!terminate) {

    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) {
        terminate = true;
        break;
        }
      DispatchMessage(&msg);
      }
    if(!terminate) {
      DWORD result = MsgWaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, timeout, QS_ALLINPUT);
      if(result == (WAIT_OBJECT_0 + SIZE_A(HandlesToWaitFor)))
        continue;
      switch(result) {
        case WAIT_TIMEOUT:
          // probabilmente non serve più, era un po' una pezza al problema del server
          // ma non dovrebbe appesantire troppo verificare se ci sono dati da ricevere e/o
          // inviare ogni CHECK_ON_TIMEOUT_WAIT_CONN msec
          if(!onClosed && Conn) {
            SetEvent(HandlesToWaitFor[0]);
            SetEvent(HandlesToWaitFor[1]);
            break;
            }
          // riprova la connessione, imposta il timeout a RECHECK_TIMEOUT_WAIT_CONN in caso di successo,
          // al valore predefinito, per riprovare, altrimenti
          Cl->thSetErrThread();
          if(!Conn || (!Conn->isConnected() && !Conn->open())) {
            Conn = makeTrueConn(iServer, iReq, W).makeConn(Conn, Cl);
            timeout = PREDEF_TIMEOUT_WAIT_CONN;
            }
          else {
            Cl->showRemoteIp(Conn);
            Cl->thResetErrThread();
            Cl->thSendAck(Conn);
            timeout = CHECK_ON_TIMEOUT_WAIT_CONN;
            onClosed = false;
            }
          break;

        case WAIT_OBJECT_0:
          Conn->reqNotify(false);
          if(!Cl->thReadConn(Conn)) {
            Cl->thSetErrThread();
            Conn->close();
            Conn = makeTrueConn(iServer, iReq, W).makeConn(Conn, Cl);
            timeout = RECHECK_TIMEOUT_WAIT_CONN;
            onClosed = true;
            }
          else {
            if(Conn->has_string())
              SetEvent(HandlesToWaitFor[0]);
            else
              Conn->reqNotify(true);
            Cl->showRemoteIp(Conn);
            }
          break;

        case WAIT_OBJECT_0 + 1:
          if(!Cl->thWriteConn(Conn)) {
            Cl->thSetErrThread();
            if(Conn)
              Conn->close();
            Conn = makeTrueConn(iServer, iReq, W).makeConn(Conn, Cl);
            timeout = RECHECK_TIMEOUT_WAIT_CONN;
            onClosed = true;
            }
          break;

        case WAIT_OBJECT_0 + 2:
          Cl->thSetErrThread();
          if(Conn)
            Conn->close();
          Conn = makeTrueConn(iServer, iReq, W).makeConn(Conn, Cl);
          timeout = RECHECK_TIMEOUT_WAIT_CONN;
          onClosed = true;
          break;

        case WAIT_FAILED:
//          DisplayErrorString(GetLastError());
        default:
          terminate = true;
          break;
        }
      }
    }
  delete Conn;
  delete W;
  Cl->idThread = 0;
  return 1;
}
