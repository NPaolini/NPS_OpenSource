//----------- winClientConn.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "winClientConn.h"
#include "winServerConn.h"
#include <stdio.h>
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
winClientConn::winClientConn(winServerConn* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst) :
      baseClass(parent, id, 0, 0, 20, 20, 0, hInst), Conn(new PConnBase(hc)), idPrph(0),
      hEventCloseConn(0), hEventHasChar(0), hEventSendChar(0), threadActive(false),
      idTimer(0), readOnly(false)
{
  ZeroMemory(keySend, sizeof(keySend));
  hEventCloseConn = CreateEvent(0, TRUE, 0, 0);
  hEventHasChar = CreateEvent(0, 0, 0, 0);
  hEventSendChar = CreateEvent(0, 0, 0, 0);
}
//----------------------------------------------------------------------------
winClientConn::~winClientConn()
{
  stopThread();
  delete Conn;
  Conn = 0;

  CloseHandle(hEventCloseConn);
  CloseHandle(hEventHasChar);
  CloseHandle(hEventSendChar);
  setFreeItem(CmdSet);
  destroy();
}
//----------------------------------------------------------------------------
extern bool isReadOnly(uint prph);
//----------------------------------------------------------------------------
bool winClientConn::create()
{
  Attr.style &= ~WS_VISIBLE;
  if(!baseClass::create())
    return false;
  Conn->passThrough(cChangeHwndOwner, (LPDWORD)getHandle());

  DWORD idThread;
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReaderClientProc,
                  this, 0, &idThread);
  if(hThread) {
    CloseHandle(hThread);
    Conn->reqNotify(true);
    }
  else
    return false;
  idTimer = SetTimer(*this, 555, getTimeoutAck(), 0);
  return true;
}
//----------------------------------------------------------------------------
LRESULT  winClientConn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_CUSTOM_MSG_CONNECTION == message) {
    switch(LOWORD(wParam)) {
      case CM_MSG_HAS_CHAR:
        do {
        // meglio nessuna altra notifica finché si elabora
          Conn->reqNotify(false);
          SetEvent(hEventHasChar);
          } while(false);
        break;
      default:
        if(0 == HIWORD(wParam))
          PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_REMOVE_CLIENT, 0), (LPARAM)this);
        break;
      }
    }
  else switch(message) {
    case WM_TIMER:
      if(idTimer != wParam)
        break;
      PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_REMOVE_CLIENT, 0), (LPARAM)this);
    case WM_DESTROY:
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      break;
    case WM_FROM_CLIENT:
      // richiesta interna
      switch(LOWORD(wParam)) {
        case CM_END_THREAD:
          PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_REMOVE_CLIENT, 0), (LPARAM)this);
          break;
        case CM_KILL_TIMER:
          if(idTimer) {
            KillTimer(hwnd, idTimer);
            idTimer = 0;
            }
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void winClientConn::stopThread()
{
  SetEvent(hEventCloseConn);
  for(;;) {
    if(!threadActive)
      break;
    Sleep(100);
    }
}
//----------------------------------------------------------------------------
bool winClientConn::flushConn()
{
  Conn->passThrough(cReset, 0);
  return true;
}
//----------------------------------------------------------------------------
#define MAX_BUFF_CLIENT 256
//----------------------------------------------------------------------------
bool winClientConn::flushConn(int size)
{
  BYTE buff[MAX_BUFF_CLIENT];
  while(size > sizeof(buff)) {
    int readed = Conn->read_string(buff, sizeof(buff));
    if(!readed)
      return true;
    size -= readed;
    }
  if(size)
    Conn->read_string(buff, size);
  return true;
}
//----------------------------------------------------------------------------
bool winClientConn::sendModifiedData(LPDWORD buff)
{
  bool needSend = mrData.sendModifiedData(buff);
  if(needSend)
    setEventSend();
  return needSend;
}
//----------------------------------------------------------------------------
bool winClientConn::writeConn()
{
  mrData.sendModifiedData(Conn, getKeySend());
  return true;
}
//-----------------------------------------------------
// n.b. non devono esserci sequenze ripetute nel magic
static bool matchMagic(LPBYTE magic, uint len, PConnBase* Conn)
{
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
}
//-----------------------------------------------------
DWORD cryptDWord(DWORD s, DWORD key, uint ix)
{
  DWORD t = (ix & 1) ? (key ^ ~(ix * 13)) : (key ^ ~(ix * 17));
  return s ^ t;
}
//----------------------------------------------------------------------------
static void transformKeySend(LPBYTE target, LPBYTE source, int l)
{
  crypt_noZ(source, target, 0xcafebabe, l);
}
//----------------------------------------------------------------------------
bool winClientConn::readConn()
{
  int dim = Conn->has_string();
  if(!dim)
    return true;
  DWORD magic = SV_MAGIC;
  if(!matchMagic(LPBYTE(&magic), sizeof(magic), Conn))
    return true;

  info_cmd_data* packet = getFreeInfoCmdData();
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
    releaseInfoCmdData(packet);
    return true;
    }

  switch(Head.Code) {
    case header::hPsw:
      PostMessage(*this, WM_FROM_CLIENT, MAKEWPARAM(CM_KILL_TIMER, 0), (LPARAM)this);
      do {
        handshake* hs = (handshake*)packet;
        WORD low = hs->comparePsw(getPassword()) ? CM_OK_CLIENT : CM_REMOVE_CLIENT;
        transformKeySend(keySend, hs->key, DIM_KEY_SEND);
        idPrph = hs->prph;
        readOnly = isReadOnly(idPrph);
        if(idPrph <= 1 || idPrph > 20)
          low = CM_REMOVE_CLIENT;

        PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(low, 0), (LPARAM)this);
        } while(false);
      break;

    case header::hVarToWrite:
      if(!readOnly)
        fillToWrite(packet);
      break;
    }
  releaseInfoCmdData(packet);
  return true;
}
//----------------------------------------------------------------------------
#define SEND_FRAME 1
#define SEND_INFO  2
#define MAX_TIMEOUT_SEND
//----------------------------------------------------------------------------
bool winClientConn::fillToWrite(info_cmd_data* packet)
{
  FreePacket& packetFree = getFreePacket();
  LPDWORD p = (LPDWORD)packet->data;
  criticalLock CL(CS);
  uint nElem = CmdSet.getElem();
  for(uint i = 0; i < packet->numinfodata; ++i) {
    infocmd* item = (infocmd*)packetFree.get();
    *item = *(infocmd*)p;
    LPDWORD p2 = LPDWORD(LPBYTE(p) + sizeof(infocmd));
    if((DWORD)(LPBYTE(p2) - LPBYTE(packet)) > (DWORD)(MAX_DIM_BLOCK - item->numdata * sizeof(DWORD)))
      return false;
    cryptBufferNoZ((LPBYTE)item->data, LPBYTE(((infocmd*)p)->data), item->numdata * sizeof(DWORD), getKeySend(), DIM_KEY_SEND, 1);
/*
    item->data[0] =  cryptDWord(item->data[0], getKeySend(), 0);
    for(uint j = 1; j < item->numdata; ++j)
      item->data[j] = cryptDWord(*p++, getKeySend(), j);
*/
    CmdSet[nElem++] = item;
    }
  return true;
}
//----------------------------------------------------------------------------
static uint getTotVars(const PVect<infocmd*>& set)
{
  uint tot = 0;
  uint nElem = set.getElem();
  for(uint i = 0; i < nElem; ++i) {
    const infocmd* id = set[i];
    tot += id->numdata;
    }
  return tot;
}
//----------------------------------------------------------------------------
void winClientConn::writeCommand()
{
  criticalLock CL(CS);
  uint tot = getTotVars(CmdSet);
  if(!tot)
    return;
  pSV_prfData data = new SV_prfData[tot];
  uint nElem = CmdSet.getElem();
  uint k = 0;
  for(uint i = 0; i < nElem; ++i) {
    const infocmd* id = CmdSet[i];
    for(uint j = 0; j < id->numdata; ++j, ++k) {
      data[k].typeVar = id->typevar;
      data[k].U.dw = id->data[j];
      data[k].lAddr = id->addr + j;
      }
    }
  npSV_MultiSet(getIdPrph(), data, tot);
  delete []data;
  setFreeItem(CmdSet);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
unsigned FAR PASCAL ReaderClientProc(void* cl)
{
  winClientConn *Win = reinterpret_cast<winClientConn*>(cl);
  PConnBase* conn = Win->Conn;

  Win->setThread();

  bool oldNotifyConn = conn->reqNotify(false);

  bool terminate = false;
  bool needWait = false;

  HANDLE HandlesToWaitFor[] = {
    Win->hEventCloseConn,
    Win->hEventSendChar,
    Win->hEventHasChar,
    };

  SetEvent(Win->hEventHasChar);
  const DWORD timeout = INFINITE;
//  const DWORD timeout = 1000 * 10;
  while(!terminate) {

    DWORD result = WaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, timeout);
    switch(result) {
      case WAIT_OBJECT_0 + 1:
        if(!Win->writeConn())
          terminate = true;
        break;
      case WAIT_TIMEOUT:
      case WAIT_OBJECT_0 + 2:
        if(!Win->readConn())
          terminate = true;
        else
          if(!conn->has_string())
            conn->reqNotify(true);
        break;

      case WAIT_OBJECT_0:
      default:
        terminate = true;
        break;
      }
    if(!terminate) {
      // verifica se ci sono dati da leggere
      if(conn->has_string())
        SetEvent(HandlesToWaitFor[2]);
      }
    }
  Win->resetThread();
  PostMessage(*Win, WM_FROM_CLIENT, MAKEWPARAM(CM_END_THREAD, 0), 0);
  return true;

}
//----------------------------------------------------------------------------
