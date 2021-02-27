//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "make_crypt.h"
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

  InitCommonControls();
  make_crypt(0, hInstance).modal();

  return EXIT_SUCCESS;
}
