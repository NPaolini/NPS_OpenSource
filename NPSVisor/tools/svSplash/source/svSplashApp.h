//------------------- svSplashApp.h ------------------------------------------
//----------------------------------------------------------------------------
#ifndef svSplashApp_H_
#define svSplashApp_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pWin.h"
#include "pAppl.h"
//----------------------------------------------------------------------------
class svSplashApp : public PAppl
{
  public:
    svSplashApp(HINSTANCE hInstance, int nCmdShow) :
      PAppl(hInstance, nCmdShow) { }
  protected:
    virtual PWin* initMainWindow(LPCTSTR title, HINSTANCE hInstance);
};
//----------------------------------------------------------------------------
#endif
