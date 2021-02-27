//-------- sv_make_dll_server.cpp ----------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "sv_make_dll.h"
#include "p_file.h"
#include "PCrt_lck.h"
#include "sv_baseServer.h"
#include "pdllComm.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
//------------------------------------------------------------------
#define ID_SERVER_PORT    10
#define ID_SERVER_NAME    11
#define ID_SERVER_PSW     12
#define ID_SERVER_TIMEOUT_ACK 13

#define ID_STATIC_SERVER_4_IP 14

#define ID_READ_ONLY_PRPH 15
//------------------------------------------------------------------
#define DEF_TIMEOUT_ACK (1000 * 60 * 2)
#define MIN_TIMEOUT_ACK (1000)
#define MAX_TIMEOUT_ACK (1000 * 60 * 15)
//------------------------------------------------------------------
static TCHAR gSvServerName[_MAX_PATH];
static uint gPort;
static char gSvPassword[MAX_PASSWORD + 1];
static DWORD gSvTimeoutAck;
static staticIp4DynamicIp InfoDynamicIP;
//------------------------------------------------------------------
DWORD getTimeoutAck() { return gSvTimeoutAck; }
//------------------------------------------------------------------
LPCTSTR getServerName() { return gSvServerName; }
//------------------------------------------------------------------
LPCSTR getPassword() { return gSvPassword; }
//------------------------------------------------------------------
staticIp4DynamicIp getInfoDynamicIP() { return InfoDynamicIP; }
//------------------------------------------------------------------
static svServer* gSvServer;
//------------------------------------------------------------------
static bool WSA_Started;
//------------------------------------------------------------------
static PVect<uint> readOnlySet;
bool isReadOnly(uint prph)
{
  uint nElem = readOnlySet.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(prph == readOnlySet[i])
      return true;
  return false;
}
//------------------------------------------------------------------
BOOL initProcess(HINSTANCE hI)
{
/*
  non si possono richiamare stringhe della dll, il file non è stato ancora caricato
*/
  return TRUE;
}
//------------------------------------------------------------------
void endProcess()
{
}
//------------------------------------------------------------------
static void nothing();
static void Refresh();
static void initRefresh();
//------------------------------------------------------------------
static void (*perform)(void) = nothing;
//------------------------------------------------------------------
static void nothing() {}
//------------------------------------------------------------------
static void Refresh()
{
  gSvServer->perform();
}
//------------------------------------------------------------------
static bool pDll_initialized = false;
//------------------------------------------------------------------
static
DWORD get_Addr(LPCTSTR address)
{
  int p1;
  int p2;
  int p3;
  int p4;
  _stscanf_s(address, _T("%d.%d.%d.%d"), &p1, &p2, &p3, &p4);
  DWORD addr = p1 | (p2 << 8) | (p3 << 16) | (p4 << 24);
  return addr;
}
//------------------------------------------------------------------
static void initRefresh()
{
  perform = nothing;
  if(!pDll::init())
    return;
  pDll_initialized = true;

  LPCTSTR p = npSV_GetLocalString(ID_SERVER_PORT, getHinstance());
  if(!p)
    return;
  gPort = _ttoi(p);
  if(!gPort)
    return;
  p = npSV_GetLocalString(ID_SERVER_PSW, getHinstance());
  if(!p)
    return;
  copyStrZ(gSvPassword, p, MAX_PASSWORD - 1);

  p = npSV_GetLocalString(ID_SERVER_NAME, getHinstance());
  if(!p)
    _tcscpy_s(gSvServerName, _T("SvServer"));
  else
    _tcsncpy_s(gSvServerName, p, SIZE_A(gSvServerName) - 1);

  gSvTimeoutAck = DEF_TIMEOUT_ACK;
  p = npSV_GetLocalString(ID_SERVER_TIMEOUT_ACK, getHinstance());
  if(p) {
    gSvTimeoutAck = _ttoi(p);
    if(gSvTimeoutAck < MIN_TIMEOUT_ACK)
      gSvTimeoutAck = DEF_TIMEOUT_ACK;
    else if(gSvTimeoutAck > MAX_TIMEOUT_ACK)
      gSvTimeoutAck = DEF_TIMEOUT_ACK;
    }
  p = npSV_GetLocalString(ID_STATIC_SERVER_4_IP, getHinstance());
  if(p) {
    InfoDynamicIP.Ip = get_Addr(p);
    p = findNextParamTrim(p);
    if(p)
      InfoDynamicIP.Port = _ttoi(p);
    p = findNextParamTrim(p);
    if(p) {
      uint pingMinute = _ttoi(p);
      if(pingMinute)
        InfoDynamicIP.pingMinute = pingMinute;
      }
    InfoDynamicIP.localPort = gPort;
    }
  p = npSV_GetLocalString(ID_READ_ONLY_PRPH, getHinstance());
  while(p) {
    readOnlySet[readOnlySet.getElem()] = _ttoi(p);
    p = findNextParamTrim(p);
    }
  WORD wVersionRequested = MAKEWORD(1,1);
  WSADATA wsaData;

  int nRet = WSAStartup(wVersionRequested, &wsaData);
  if(nRet || wsaData.wVersion != wVersionRequested)
    return;

  WSA_Started = true;

  gSvServer = allocServer();

  if(gSvServer && gSvServer->start(gPort, gSvServerName))
    perform = Refresh;
}
//------------------------------------------------------------------
static void endRefresh()
{
  perform = nothing;

  if(gSvServer) {
    gSvServer->stop();
    delete gSvServer;
    gSvServer = 0;
    }
  if(pDll_initialized)
    pDll::end();

  if(WSA_Started) {
    WSA_Started = false;
    WSACleanup();
    }
}
//------------------------------------------------------------------
ExpImp_DLL
void npSV_System(DWORD msg)
{
  switch(msg) {
    case eINIT_DLL:
      initRefresh();
      break;
    case eEND_DLL:
      endRefresh();
      break;
    }
}
//------------------------------------------------------------------
ExpImp_DLL
void npSV_Refresh()
{
  perform();
}
//------------------------------------------------------------------
