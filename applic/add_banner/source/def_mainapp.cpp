//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#define OEMRESOURCE
#include <commctrl.h>
#include <dos.h>
#include <string>
#include <iostream>
//----------------------------------------------------------------------------
#include "mainDlg.h"
//----------------------------------------------------------------------------
static PWin* mainWin = 0;
PWin* getMain() { return mainWin; }
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE, //hPrevInstance,
                     LPSTR,     //lpCmdLine,
                     int       nCmdShow)
{
  InitCommonControls();

  PDShowVars t_e(0, IDD_CLIENT, hInstance);
  mainWin = &t_e;
  t_e.modal();
  return 0;
}
