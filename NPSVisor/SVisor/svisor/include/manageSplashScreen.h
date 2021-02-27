//--------- manageSplashScreen.h ---------------------------------------------
//----------------------------------------------------------------------------
#ifndef manageSplashScreen_H_
#define manageSplashScreen_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
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
#define WMC_START_PRPH     10

#define WMC_LOADING_PRF_0 100
#define WMC_LOADED_PRF_0  200
#define WMC_FAILED_PRF_0  300

#define MAX_PERIPH_INFO_SPLASH (WMC_LOADED_PRF_0 - WMC_LOADING_PRF_0)
#define WMC_MSG_IS_UNICODE 0x80000000
//----------------------------------------------------------------------------
class manageSplashScreen
{
  public:
    manageSplashScreen(HWND hwMain = 0) : HWMain(hwMain), hwSplash(0), totPrf(0),
        loadedPrf(0), failedPrf(0)
    {    }
    ~manageSplashScreen();

    bool run();

    void setHwSplash(HWND hw) { hwSplash = hw; }
    HWND getHWSplashScreen() { return hwSplash; }
    int getRemainPrf() { return totPrf - loadedPrf - failedPrf; }
    void sendMessageSplash(int id, LPCTSTR msg);
    void waitEnd();
  private:
    HWND HWMain;
    HWND hwSplash;
    int totPrf;
    int loadedPrf;
    int failedPrf;
};
//----------------------------------------------------------------------------
UINT getRegMessageSplash();
void closeHWSplashScreen();
bool sendMessageSplash(int id, LPCTSTR msg);
//----------------------------------------------------------------------------
#endif
