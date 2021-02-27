//----------- d_empty.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_EMPTY_H_
#define D_EMPTY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_DEFBODY_H_
  #include "P_DefBody.h"
#endif
//----------------------------------------------------------------------------
#include "setPack.h"

class TD_EMPTY : public P_DefBody {
  public:
    TD_EMPTY(int idPar, PWin* parent, uint resId = IDD_EMPTY, HINSTANCE hinst = 0);
    virtual ~TD_EMPTY();
    P_Body* pushedBtn(int idBtn);
    virtual bool create();

  protected:
    virtual void getFileStr(LPTSTR path);
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

