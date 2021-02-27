//-------------------- PAppl.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pAppl.h"
//-----------------------------------------------------------
static PAppl* mainApp;
//-----------------------------------------------------------
PAppl* getAppl()
{
  return mainApp;
}
//-----------------------------------------------------------
HINSTANCE PAppl::hInstance;
//-----------------------------------------------------------
PAppl::PAppl(HINSTANCE hInst, int nCmdShow) :
    howShow(nCmdShow),
    idleCount(0), endLoop(false), mainWindow(0)
{
  hInstance = hInst;
  mainApp = this;
}
//-----------------------------------------------------------
PAppl::~PAppl()
{
  delete mainWindow;
}
#define TRACE(a)  MessageBox(0, _T(a), " ", MB_OK)

//-----------------------------------------------------------
WPARAM PAppl::run(LPCTSTR title)
{
  if(!mainWindow) {
    mainWindow = initMainWindow(title, hInstance);
    if(!mainWindow)
      return EXIT_FAILURE;

    if(!mainWindow->create())
      return EXIT_FAILURE;

    ShowWindow(*mainWindow, howShow);
    }

  UpdateWindow(*mainWindow);

  WPARAM result = 0;
  while(!endLoop) {
    if(!mainWindow->idle(idleCount++)) {
      ::WaitMessage();
      idleCount = 0;
      }
    result = pumpMessages();
    }
  return result;
}
//-------------------------------------------------
PWin* PAppl::initMainWindow(LPCTSTR title, HINSTANCE hInst)
{
  return new PMainWin(title, hInst);
}
//-------------------------------------------------
WPARAM PAppl::pumpMessages()
{
  bool wantMore;
  WPARAM result;
  do {
    result = pumpOnlyOneMessage(wantMore);
    } while(wantMore);
  return result;
}
//-------------------------------------------------
WPARAM PAppl::pumpOnlyOneMessage(bool &wantMore)
{
  wantMore = false;
  WPARAM result = 0;
  MSG msg;
  if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    if(WM_QUIT == msg.message) {
      endLoop = true;
      result = end(msg.wParam);
      }
    if(!preProcessMessage(msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      }
    wantMore = !endLoop;
    }
  return result;
}
//-------------------------------------------------
bool PAppl::preProcessMessage(MSG& msg)
{
  for(HWND hWnd = msg.hwnd; hWnd; hWnd = ::GetParent(hWnd)) {
#if 0
    if (hWnd == GetDesktopWindow())
      break;
#else
    class _TDesktopWindow
    {
      public:
       _TDesktopWindow() : hwnd(::GetDesktopWindow()){}
        operator  HWND() const { return hwnd; }
        HWND      hwnd;
    };

    const static _TDesktopWindow deskTopHwnd;
    if(hWnd == deskTopHwnd)
      break;
#endif
    PWin* win = PWin::getWindowPtr(hWnd);

    if(win && win->preProcessMsg(msg))
      return true;
    }
  return false;
}


//-------------------------------------------------
//-------------------------------------------------
