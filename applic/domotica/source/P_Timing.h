//-------- P_Timing.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_Timing_H
#define P_Timing_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PDIALOG_H_
  #include "pdialog.h"
#endif

#include "resource.h"
#include "p_vect.h"
#include "pbitmap.h"
#include "paramconn.h"
#include "connClass.h"
#include "P_BaseCommon.h"
//----------------------------------------------------------------------------
class P_Timing : public P_BaseCommon
{
  private:
    typedef P_BaseCommon baseClass;
  public:
    P_Timing(PConnBase* conn, PWin* parent, uint resId = IDD_DIALOG_TIMING, HINSTANCE hinstance = 0);
    virtual ~P_Timing();
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void parseTimingAlarm(LPCSTR buff);
    void parseDatetime(LPCSTR buff);

    void loadCurrTime();
    void loadCurrTiming();
    void syncTime();

  private:
    void BNClickedOk();
    PVect<PBitmap*> Bmp;
};
//----------------------------------------------------------------------------
#endif
