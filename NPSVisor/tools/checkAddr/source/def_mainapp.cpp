//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
#include <dos.h>
//----------------------------------------------------------------------------
#include "mainDlg.h"
//----------------------------------------------------------------------------
#define MUTEX_NAME _T("np_checkAddr_mutex_onlyone")
//----------------------------------------------------------------------------
static PWin* mainWin = 0;
PWin* getMain() { return mainWin; }
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE, //hPrevInstance,
                     LPSTR,     //lpCmdLine,
                     int       nCmdShow)
{
  HANDLE hMutex = CreateMutex(0, true, MUTEX_NAME);
  if(hMutex && ERROR_ALREADY_EXISTS == GetLastError()) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return 0;
    }

  InitCommonControls();

  PDShowVars t_e(0, IDD_SHOW, hInstance);
  mainWin = &t_e;
  t_e.modal();
  if(hMutex) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    }
  return 0;
}
