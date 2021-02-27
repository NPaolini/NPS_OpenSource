//---------------- svSplashApp.cpp -------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "svSplashApp.h"
#include "svSplashWin.h"
//----------------------------------------------------------------------------
PWin* svSplashApp::initMainWindow(LPCTSTR title, HINSTANCE hInstance)

{
  svSplashWin* mainWin = new svSplashWin(title, hInstance);
  return mainWin;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// not UNICODE
//----------------------------------------------------------------------------
#define USE_GLOBAL_ARGV
//----------------------------------------------------------------------------
#include <dos.h>
//----------------------------------------------------------------------------
static HWND hwSVisor = 0;
//----------------------------------------------------------------------------
HWND getHwSvisor() { return hwSVisor; }
//----------------------------------------------------------------------------
#define IS_OK(v) (1 == (v))
//----------------------------------------------------------------------------
#if !defined(UNICODE) && !defined(_UNICODE)
//----------------------------------------------------------------------------
static int parseArg()
{
  int argc = __argc;
  char **argv = __argv;

  unsigned ok = 0;
  while(--argc >= 0) {
    if(argv[argc][0] == '/' || argv[argc][0] == '-') {
      switch(argv[argc][1]) {
        case 'H':
        case 'h':
          hwSVisor = reinterpret_cast<HWND>(atol(argv[argc]+2));
          ok |= 1;
          break;
        }
      }
    }

  if(!IS_OK(ok)) {
//    ::MessageBox(0,"L'applicazione non può essere avviata da sola",
//          "Errore: Parametri errati",MB_OK);
    return 0;
    }

  return 1;
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
// UNICODE

#include <Shellapi.h>
//----------------------------------------------------------------------------
static int parseArg()
{
  LPTSTR commandLine = GetCommandLine();
  int argc;
  LPTSTR *argv = CommandLineToArgvW(commandLine, &argc);

  //salva per liberarla alla fine
  LPTSTR *save = argv;

  unsigned ok = 0;
  while (--argc >= 0) {
	  if (argv[argc][0] == _T('/') || argv[argc][0] == _T('-')) {
		  switch (argv[argc][1]) {
		  case _T('H'):
		  case _T('h'):
			  hwSVisor = reinterpret_cast<HWND>(_ttol(argv[argc] + 2));
			  ok |= 1;
			  break;
		  }
	  }
  }

  LocalFree(save);

  if(!IS_OK(ok)) {
//    ::MessageBox(0, _T("L'applicazione non può essere avviata da sola"),
//          _T("Errore: Parametri errati"),MB_OK);
    return 0;
    }

  return 1;
}
#endif
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE, //hPrevInstance,
                     LPSTR,     //lpCmdLine,
                     int       nCmdShow)
{
  if(!parseArg())
    return 0;
    
  svSplashApp app(hInstance, nCmdShow);

  int result = app.run(_T(" "));

  return result;
}
