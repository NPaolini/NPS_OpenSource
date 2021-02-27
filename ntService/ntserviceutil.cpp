//------------- ntServiceUtil.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include "ntServiceUtil.h"
//----------------------------------------------------------------------------
using namespace ntServiceUtil;
//----------------------------------------------------------------------------
static bool openProcess(LPCTSTR name)
{
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags =  STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;
  si.wShowWindow = SW_HIDE;

  PROCESS_INFORMATION pi;
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, name);
  if(CreateProcess(0, path, 0, 0, 0, NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi)) {
    WaitForInputIdle(pi.hProcess, 3 * 1000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
DWORD ntServiceUtil::controlSampleService(DWORD fdwControl, LPCTSTR serviceName)
{
  DWORD access[] = {
    SC_MANAGER_ALL_ACCESS,
    GENERIC_EXECUTE | GENERIC_WRITE | GENERIC_READ,
    GENERIC_EXECUTE | GENERIC_READ,
    GENERIC_READ,
    };
  SC_HANDLE schSCManager = 0;
  for(uint i = 0; i < SIZE_A(access); ++i) {
    schSCManager = OpenSCManager(0, 0, access[i]);
    if(schSCManager)
      break;
    }
  if(!schSCManager)
    return 0;

  // The required service object access depends on the control.
  DWORD fdwAccess;
  switch (fdwControl) {
    case SERVICE_CONTROL_STOP:
      fdwAccess = SERVICE_STOP;
      break;

    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
      fdwAccess = SERVICE_PAUSE_CONTINUE;
      break;

    case SERVICE_CONTROL_INTERROGATE:
      fdwAccess = SERVICE_INTERROGATE;
      break;

    default:
      fdwAccess = SERVICE_INTERROGATE;
    }
  // Open a handle to the service.
  SC_HANDLE schService = OpenService(schSCManager, serviceName, fdwAccess);
  if(!schService) {
    CloseServiceHandle(schSCManager);
    return 0;
    }

  SERVICE_STATUS ssStatus;
  // Send a control value to the service.
  if(!ControlService(schService, fdwControl, &ssStatus))
    ssStatus.dwCurrentState = 0;

  CloseServiceHandle(schService);
  CloseServiceHandle(schSCManager);
  return ssStatus.dwCurrentState;
}
//----------------------------------------------------------------------------
bool ntServiceUtil::isInstalled(LPCTSTR serviceName)
{
  bool inst = false;
  TCHAR gszAppRegKey[_MAX_PATH] = TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
  _tcscat_s(gszAppRegKey, serviceName);
  HKEY hKey = 0;
  if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, gszAppRegKey, 0, KEY_READ, &hKey)) {
    inst = true;
    RegCloseKey(hKey);
    }
  return inst;
}
//----------------------------------------------------------------------------
bool ntServiceUtil::isRunning(LPCTSTR serviceName)
{
  return SERVICE_RUNNING == controlSampleService(SERVICE_CONTROL_INTERROGATE, serviceName);
}
//----------------------------------------------------------------------------
bool ntServiceUtil::install(LPCTSTR serviceExecName, bool add, LPCTSTR user, LPCTSTR psw)
{
  TCHAR buff[512];
  if(!add) {
    if(user)
      wsprintf(buff, _T("%s -ur -x"), serviceExecName);
    else
      wsprintf(buff, _T("%s -u -x"), serviceExecName);
    }
  else {
    if(user && psw)
      wsprintf(buff, _T("%s -l\"%s\" -p\"%s\" -i -x"), serviceExecName, user, psw);
    else if(user && !psw)
      wsprintf(buff, _T("%s -l\"%s\" -i -x"), serviceExecName, user);
    else
      wsprintf(buff, _T("%s -i -x"), serviceExecName);
    }
  return openProcess(buff);
}
//----------------------------------------------------------------------------
bool ntServiceUtil::start(LPCTSTR serviceExecName, bool run)
{
  TCHAR buff[512];
  if(run)
    wsprintf(buff, _T("%s -s"), serviceExecName);
  else
    wsprintf(buff, _T("%s -e"), serviceExecName);
  return openProcess(buff);
}
//----------------------------------------------------------------------------
