//------------------- commap.h -----------------------------------------------
//----------------------------------------------------------------------------
#ifndef COMMAP_H_
#define COMMAP_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pWin.h"
#include "pAppl.h"
//----------------------------------------------------------------------------
class gCommApp : public PAppl
{
  public:
    gCommApp(HINSTANCE hInstance, int nCmdShow) :
      PAppl(hInstance, nCmdShow) { }
  protected:
    virtual PWin* initMainWindow(LPCTSTR title, HINSTANCE hInstance);
};
//----------------------------------------------------------------------------
#endif
