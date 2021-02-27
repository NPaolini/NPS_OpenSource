//------ def_mainapp.cpp -----------------------------------------------------
#define OEMRESOURCE
#include "precHeader.h"
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "make_keydat.h"
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

  InitCommonControls();
  make_keydat(0, hInstance).modal();

  return EXIT_SUCCESS;
}
