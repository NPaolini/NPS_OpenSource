//----------- s7_libnodave_svr_main.cpp --------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include "s7_libnodave_common.h"
#include "sv_baseServer.h"
#include "p_txt.h"
#include "p_vect.h"
//----------------------------------------------------------
static TCHAR Password[_MAX_PATH];
static TCHAR ServerName[_MAX_PATH];
static DWORD TimeoutAck;
//----------------------------------------------------------
static void setPassord(LPCTSTR psw)
{
  if(psw)
    _tcscpy_s(Password, psw);
}
//----------------------------------------------------------
static void setTimeoutAck(LPCTSTR time)
{
  TimeoutAck = 20000;
  if(time)
    TimeoutAck = _ttoi(time) * 1000;
}
//----------------------------------------------------------
LPCTSTR getPassword()
{
  return Password;
}
//----------------------------------------------------------
LPCTSTR getServerName()
{
  return ServerName;
}
//----------------------------------------------------------
DWORD getTimeoutAck()
{
  return TimeoutAck;
}
//----------------------------------------------------------
#define FILENAME_INFO_ _T("s7_libnodave_server.txt")
//----------------------------------------------------------
#define ID_PSW     1000
#define ID_TIMEOUT 1001
//----------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE, //hPrevInstance,
                     LPSTR,     //lpCmdLine,
                     int       nCmdShow)
{
  HANDLE hMutex = 0;
  do {
    GetModuleFileName(0, ServerName, SIZE_A(ServerName));
    TCHAR txtMutex[_MAX_PATH] = { 0 };
    makeServerMutexName(txtMutex, ServerName);
    hMutex = CreateMutex(0, true, txtMutex);
    } while(false);

  if(hMutex && ERROR_ALREADY_EXISTS == GetLastError()) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return 5;
    }

  if(nCmdShow != SW_HIDE)
    nCmdShow = (nCmdShow != SW_SHOWMINNOACTIVE) ?
//            SW_SHOWNORMAL :       // normale
            SW_SHOWMINNOACTIVE :  // o iconizzata
            nCmdShow;

  WORD wVersionRequested = MAKEWORD(1,1);
  WSADATA wsaData;

  int nRet = WSAStartup(wVersionRequested, &wsaData);
  if(nRet || wsaData.wVersion != wVersionRequested) {
    if(hMutex) {
      ReleaseMutex(hMutex);
      CloseHandle(hMutex);
      }
    return 5;
    }
  PAppl app(hInstance, nCmdShow);
  PVect<svServer*> sv_server;
  do {
    setOfString set(FILENAME_INFO_);
    do {
      uint id = set.getCurrId();
      if(id > MAX_ID)
        break;
      LPCTSTR p = set.getCurrString();
      uint nElem = sv_server.getElem();
      svServer* svs = allocServer();
      if(svs) {
        if(svs->start(id, p))
          sv_server[nElem] = svs;
        else
          delete svs;
        }
      } while(set.setNext());
    setPassord(set.getString(ID_PSW));
    setTimeoutAck(set.getString(ID_TIMEOUT));
    } while(false);

  int result = 0;
  int nElem = sv_server.getElem();
  if(nElem) {
    result = app.run(ServerName);
    for(int i = nElem - 1; i >= 0; --i) {
      sv_server[i]->stop();
      delete sv_server[i];
      }
    }
  if(hMutex) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    }
  WSACleanup();
  return result;
}

