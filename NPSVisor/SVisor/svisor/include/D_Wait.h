//---------- d_wait.h --------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_WAIT_H_
#define D_WAIT_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef SVDIALOG_H_
  #include "svdialog.h"
#endif
#include "setPack.h"
//----------------------------------------------------------------------------
class TD_Wait : public svDialog {
  public:
    TD_Wait(LPCTSTR msg, PWin* parent, uint resId = IDD_WAIT, HINSTANCE hinstance = 0);
    virtual ~TD_Wait();
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
    void evPaint(HDC hdc);
    bool evTimer(uint idTimer);

  private:
    LPCTSTR Msg;
    class PTraspBitmap* tbmp[3];
//    class PBitmap* bmp;
    class PBitmap* ball;
    POINT Pt;
    int count;
    int dir;
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

