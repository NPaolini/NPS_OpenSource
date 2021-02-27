//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
#include <dos.h>
//----------------------------------------------------------------------------
#include "test_Expr.h"
//----------------------------------------------------------------------------
PWin* getMain()
{
  return getAppl()->getMainWindow();
}
//-----------------------------------------------------------
class makerApp : public PAppl
{
  public:
    makerApp(HINSTANCE hInstance, int nCmdShow) :
      PAppl(hInstance, nCmdShow) { }
  protected:
    virtual PWin* initMainWindow(LPCTSTR title, HINSTANCE hInstance)
    {
      return new test_Expr(title, hInstance);
    }
};
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE, //hPrevInstance,
                     LPSTR,     //lpCmdLine,
                     int       nCmdShow)
{

  InitCommonControls();
  extern bool needMaximized();
  if(needMaximized())
    nCmdShow = SW_SHOWMAXIMIZED;
  int result = makerApp(hInstance, nCmdShow).run(_T("Express_Script Maker"));
  return result;
}
