//------ progressBar.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PROGRESSBAR_H_
#define PROGRESSBAR_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PDIALOG_H_
  #include "pdialog.h"
#endif
#include "resource.h"
#include "p_vect.h"
#include "pbitmap.h"
#ifndef CLR_DEFAULT
  #define CLR_DEFAULT             0xFF000000L
#endif
//-------------------------------------------------------------------
class progressBar : public PDialog
{
  public:
    progressBar(PWin* parent, LPCTSTR message, DWORD max_count, COLORREF bar = CLR_DEFAULT,
        uint resId = IDD_PROGRESS, HINSTANCE hInstance = 0);
    virtual ~progressBar();

    virtual bool create();

//    void setMax(DWORD maxCount);
    bool setCount(DWORD count);
    DWORD getMax() { return maxCount; }
    DWORD getCurr() { return currPos; }
  protected:
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
  private:

    typedef PDialog baseClass;
    bool Aborted;
    LPTSTR Message;
    DWORD maxCount;
    DWORD currPos;
    COLORREF barColor;
    HWND oldFocus;
};
//----------------------------------------------------------------------------
#endif
