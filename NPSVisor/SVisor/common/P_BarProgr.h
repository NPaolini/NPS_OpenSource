//----------- P_BarProgr.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_BARPROGR_H_
#define P_BARPROGR_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "svdialog.h"
//--------------------------------------------------------------------------
class BarProgr : public svDialog
{
  public:
    BarProgr(PWin* par, DWORD maxStep, bool pumpMsg = true);
    ~BarProgr() { destroy(); }
    bool setPos(DWORD ix);
    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    DWORD MaxStep;
    bool stopped;
    uint idTimer;
    bool pumpMsg;
  private:
    typedef svDialog baseClass;
};
//--------------------------------------------------------------------------
#endif

