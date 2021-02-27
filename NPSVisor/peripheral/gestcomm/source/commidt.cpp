//--------------- commidt.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
//-----------------------------------------------------------
#include "commidt.h"
#include "PServerDriver.h"
//------------------------------------------------------------------
commIdentity::commIdentity() :
    HSVisor(0), pCom(0), okArg(0), HWServer(0), WM_PRF(0),
    numPerif(0), genData(0), UseMappedFile(true), Hided(false)
{
  OSVERSIONINFO ver;
  ZeroMemory(&ver, sizeof(ver));
  ver.dwOSVersionInfoSize = sizeof(ver);
  if(GetVersionEx(&ver)) {
    if(VER_PLATFORM_WIN32_NT != ver.dwPlatformId)
      UseMappedFile = false;
    }
}
//------------------------------------------------------------------
void commIdentity::parseArg(LPCTSTR arg)
{
  switch(arg[0]) {

    case _T('I'):
    case _T('i'):
      numPerif = _ttol(arg + 1);
      addOk(1 << 2);
      if(numPerif)
        RegSTR_WM_PRPH(WM_PRF, numPerif);
      break;

    case _T('O'):
    case _T('o'):
      do {
        unsigned long t;
        _stscanf_s(arg + 1, _T("%ul"), &t);
        genData = t;
        addOk(1 << 3);
        } while(false);
      break;
    case _T('N'):
    case _T('n'):
      Hided = true;
      break;
    case _T('M'):
    case _T('m'):
      UseMappedFile = false;
      break;
    }
}
//----------------------------------------------------------------------------
int commIdentity::setHWND(HWND h, DWORD id)
{
  if(!id)
    HSVisor = h;
  else {
    if(!HWServer) {
      HWServer = h;
      idReq[0] = id;
      }
    else {
      HWServer = h;
      int nElem = idReq.getElem();
      for(int i = 0; i < nElem; ++i)
        if(id == idReq[i])
          return 0;
      idReq[nElem] = id;
      }
    }
  return 1;
}
//----------------------------------------------------------------------------
void commIdentity::removeId(DWORD id)
{
  int nElem = idReq.getElem();
  for(int i = nElem; i >= 0; --i)
    if(id == idReq[i]) {
      idReq.remove(i);
      break;
      }
}
//----------------------------------------------------------------------------
void commIdentity::addId(DWORD id)
{
  int nElem = idReq.getElem();
  for(int i = nElem; i >= 0; --i)
    if(id == idReq[i])
      return;
  idReq[nElem] = id;
}
//----------------------------------------------------------------------------
void commIdentity::flushQueueMsg()
{
  bool wantMore;
  getAppl()->pumpOnlyOneMessage(wantMore);
}
//----------------------------------------------------------------------------
struct dataForSend
{
  dataForServer data;
  HWND HWServer;
  HWND HWOwner;
};
//----------------------------------------------------------------------------
#if 0
unsigned FAR PASCAL sendProc(void* pData)
{
  dataForSend* pSend = (dataForSend*)pData;

  dataForServer* p = &pSend->data;

  dataForSend send = *pSend;

  p->id = 0;

  COPYDATASTRUCT cds;
  cds.lpData = &send.data;
  cds.dwData = send.data.Msg.Msg;
  cds.cbData = sizeof(send.data);
  SendMessage(send.HWServer, WM_COPYDATA, (WPARAM)send.HWOwner, (LPARAM)&cds);
  return 0;
}
//----------------------------------------------------------------------------
void commIdentity::sendToServer(HWND hwnd, DWORD Msg, WPARAM wp, LPARAM lp, DWORD id)
{

  dataForSend send;
  send.data.Msg.Msg = Msg;
  send.data.Msg.wP = wp;
  send.data.Msg.lP = lp;
  send.HWServer = HWServer;
  send.HWOwner = hwnd;

  ULONG idThread;

  if(SEND_TO_ALL == id) {
    int nElem = idReq.getElem();
    for(int i = 0; i < nElem; ++i) {
      send.data.id = idReq[i];
      HANDLE hThread = (HANDLE)_beginthreadex(0, 0, sendProc, &send, 0, &idThread);
      if(!hThread)
        break;
      CloseHandle(hThread);
      while(send.data.id == idReq[i])
        SleepEx(10, 0);
      }
    }
  else {
    send.data.id = id;
    HANDLE hThread = (HANDLE)_beginthreadex(0, 0, sendProc, &send, 0, &idThread);
    if(hThread)
      CloseHandle(hThread);
    while(send.data.id == id)
      SleepEx(10, 0);
    }
}
#endif
//----------------------------------------------------------------------------
LRESULT commIdentity::sendMessage(HWND hwnd, DWORD registeredMsg,
                WPARAM wp, LPARAM lp, DWORD id, bool post)
{
  if(id && HWServer) {
#if 0
    sendToServer(hwnd, registeredMsg, wp, lp, id);
#else

    if(SEND_TO_ALL == id) {
      if(MSG_RESPONCE == LOWORD(wp)) {
        PostMessage(HWServer, registeredMsg, wp, id);
        }
      else {
//      if(MSG_CHANGED_DATA == LOWORD(wp)) {
        PostMessage(HWServer, registeredMsg, wp, lp);
        }
      }
    else {
      if(MSG_RESPONCE == LOWORD(wp)) {
        PostMessage(HWServer, registeredMsg, wp, id);
        }
      else {
        dataForServer data;
        data.Msg.Msg = registeredMsg;
        data.Msg.wP = wp;
        data.Msg.lP = lp;

        COPYDATASTRUCT cds;
        cds.lpData = &data;
        cds.dwData = registeredMsg;
        cds.cbData = sizeof(data);
        data.id = id;
        return SendMessage(HWServer, WM_COPYDATA, (WPARAM)hwnd, (LPARAM)&cds);
        }
      }
#endif
    }

  if(SEND_TO_ALL == id || !id) {
    if(post)
      PostMessage(HSVisor, registeredMsg, wp, lp);
    else
      return SendMessage(HSVisor, registeredMsg, wp, lp);
    }
  return 1;
}
//----------------------------------------------------------------------------
#define MAX_NAME 50
LPCTSTR commIdentity::getClassName() const
{
  static TCHAR cl[MAX_NAME];
  MK_NAME_CLASS(cl, getNum());
  return cl;
}
//----------------------------------------------------------------------------

