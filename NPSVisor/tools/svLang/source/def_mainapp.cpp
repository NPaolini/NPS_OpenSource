//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "mainwin.h"
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
extern bool needMaximized();
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  InitCommonControls();
  if(needMaximized())
    nCmdShow = SW_SHOWMAXIMIZED;
  int result= genApp(hInstance, nCmdShow).run(_T("Appl"));

  return result;
}
