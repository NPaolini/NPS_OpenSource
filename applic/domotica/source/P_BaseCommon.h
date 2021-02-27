//-------- P_BaseCommon.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_BASECOMMON_H
#define P_BASECOMMON_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pdialog.h"
#include "connClass.h"
//----------------------------------------------------------------------------
class P_BaseCommon : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    P_BaseCommon(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance = 0);
    virtual ~P_BaseCommon();
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PConnBase* Conn;
    HWND Prev;
    virtual void readConn(WORD v) {}
};
//----------------------------------------------------------------------------
#endif
