//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "resource.h"
#include "mainClient.h"
#include "mainApp.h"
//----------------------------------------------------------------------------
#define MY_TITLE _T("Domotica by NP")
//----------------------------------------------------------------------------
class mainWin : public PMainWin
{
  public:
    mainWin(HINSTANCE hInstance) :
      PMainWin(MY_TITLE, hInstance)
    {
      client = allocMainClient(this, hInstance);
      customizeMain(this);
    }
    virtual ~mainWin()
    {
      destroy();
    }

    virtual bool create() {
      if(!PMainWin::create())
        return false;
    #ifdef IDI_ICON1
      HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
      SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
      SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));
    #endif

      resize();
      return true;
      }

  protected:
    PWin *client;
    LPCTSTR getClassName() const { return ::getClassName(); }
    void resize();
};
//----------------------------------------------------------------------------
static PRect getRectWin(HWND hwnd)
{
  PRect r;
  GetWindowRect(hwnd, r);

  int borderX = GetSystemMetrics(SM_CXEDGE) * 3;
  int borderY = GetSystemMetrics(SM_CYEDGE) * 3;
  borderY += GetSystemMetrics(SM_CYCAPTION);

  r.right += borderX;
  r.bottom += borderY;
  return r;
}
//-----------------------------------------------------------
void mainWin::resize()
{
  ::LockWindowUpdate(*this);

  PRect r = getRectWin(*client);
  setWindowPos(0, r, SWP_NOZORDER | SWP_NOMOVE);

  GetWindowRect(*client, r);
  r.MoveTo(0, 0);

  client->setWindowPos(0, r, SWP_NOZORDER | SWP_SHOWWINDOW);
  ::LockWindowUpdate(0);
}
//----------------------------------------------------------------------------
class genApp : public PAppl
{
  public:
    genApp(HINSTANCE hInstance, int nCmdShow) :
      PAppl(hInstance, nCmdShow) { }
  protected:
    virtual PWin* initMainWindow(LPCTSTR title, HINSTANCE hInstance)
    {
      return new mainWin(hInstance);
    }
};
//----------------------------------------------------------------------------
extern bool ONLY_ONE;
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  HANDLE hMutex = 0;
  if(ONLY_ONE) {
    hMutex = CreateMutex(0, true, getClassName());

    if(hMutex && ERROR_ALREADY_EXISTS == GetLastError()) {
      int i;
      for(i = 0; i < 5; ++i) {
        HWND hWnd = ::FindWindow(getClassName(), NULL);
        if(hWnd) {
        // se è già avviata, la chiude
        ::SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
          break;
          }
        SleepEx(200, false);
        }
      if(5 == i) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
        }
      }
    }

  InitCommonControls();
  int nRet = genApp(hInstance, nCmdShow).run(MY_TITLE);
  if(ONLY_ONE) {
    if(hMutex) {
      ReleaseMutex(hMutex);
      CloseHandle(hMutex);
      }
    }

  return nRet;
}
