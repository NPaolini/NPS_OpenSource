//------------ svSplashWin.h -------------------------------------------------
//----------------------------------------------------------------------------
#ifndef svSplashWin_H_
#define svSplashWin_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_Vect.h"
#include "pBitmap.h"
#include "CWaterRoutine.h"
#include "CFireRoutine.h"
//----------------------------------------------------------------------------
#define WMC_SPLASH_HWND     1
#define WMC_CLOSE           2
#define WMC_MSG             3
#define WMC_REMOVE_SPLASH   4
#define WMC_HIDE_THIS       5
#define WMC_TIMEOUT_SPLASH  6
#define WMC_END_INIT        7
#define WMC_FAILED_INIT     8

#define WMC_START_TIMER     9

#define WMC_LOADING_PRF_0 100
#define WMC_LOADED_PRF_0  200
#define WMC_FAILED_PRF_0  300

#define MAX_PERIPH_INFO_SPLASH (WMC_LOADED_PRF_0 - WMC_LOADING_PRF_0)
//----------------------------------------------------------------------------
class svSplashWin : public PMainWin
{
  public:
    svSplashWin(LPCTSTR title = 0, HINSTANCE hInst = 0);
    virtual ~svSplashWin();

    virtual bool create();

  protected:
    UINT idTimer;
    UINT idTimerEffect;
    DWORD Time;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void evPaint(HDC hdc);
    void Destroy(HWND hwnd);
    void EvTimer(UINT timerId);
    LRESULT processSVisorMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    typedef PMainWin baseClass;
    struct info
    {
      uint id;
      int loaded;
      TCHAR name[_MAX_PATH];

      info() : id(0), loaded(false) { *name = 0; }
    };

    bool resize();
    void addInfo(uint idCode, LPCTSTR p);
    void paintEvent(HDC hdc);
    void performPaint(HDC hdc);
    int calcLenBuff();

    PVect<info> Info;
    PRect infoRect;
    PBitmap* Bkg;
    COLORREF iColor;

    CWaterRoutine* m_myWater;
    CFireRoutine* m_myFire;
    void renderEffect();
    void mouseEffect(int which, const POINTS& pt);

    LPBYTE Bits1;
    LPBYTE Bits2;
};
//----------------------------------------------------------------------------
#endif
