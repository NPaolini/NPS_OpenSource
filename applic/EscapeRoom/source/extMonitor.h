//------------------- extMonitor.h --------------------------
//-----------------------------------------------------------
#ifndef extMonitor_H_
#define extMonitor_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "clientWin.h"
#include "pNumberImg.h"
#include "PSplitter.h"
//-----------------------------------------------------------
class extMonitor : public PWin
{
  public:
    extMonitor(PWin* parent, const manageTimer& mt);

    virtual ~extMonitor();

    virtual bool create();
    PSplitter* getSplitter() { return Split; }
    void setBackground(LPCTSTR path);
    void setPerc(uint perc) { Split->setPerc(perc); }
    void toggleFullScreen();

    void reset();
    void refreshInfo();
    PWin* getNumberLed() { return tml; }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void getWindowClass(WNDCLASS& wcl);
  private:
    typedef PWin baseClass;
    PSplitter* Split;

    P_TimerLed* tml;
    clientWin* cw;
    void rightClick();
};
//-----------------------------------------------------------
#endif
