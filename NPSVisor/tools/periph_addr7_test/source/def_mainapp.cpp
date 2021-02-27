//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "mainDlg.h"
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE, //hPrevInstance,
                     LPSTR,     //lpCmdLine,
                     int       nCmdShow)
{

  InitCommonControls();

#if 1
  PD_Address pa(0, IDD_CLIENT, hInstance);
  pa.create();
  PAppl appl(hInstance, 0);
  do {
    ::WaitMessage();
    appl.pumpMessages();
    } while(pa.getHandle());

#else
  PD_Address(0, IDD_CLIENT, hInstance).modal();
#endif
  return 0;
}
