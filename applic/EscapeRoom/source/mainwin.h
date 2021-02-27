//------------ mainWin.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef mainWin_H
#define mainWin_H
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "idTextLang.h"
#include "manage_audio.h"
#include "pManTimer.h"
//----------------------------------------------------------------------------
class clientWin;
class extMonitor;
//----------------------------------------------------------------------------
#define NAME_SVBASE _T("EscapeRoom.dll")
#define NO_USE_KEY
//----------------------------------------------------------------------------
LPCTSTR getStringOrDef(uint code, LPCTSTR def);
void setLang(HWND hwnd, uint idText);
void setLang(HWND hwnd, uint idc, uint idText);
//----------------------------------------------------------------------------
class mainWin : public PMainWin
{
  private:
    typedef PMainWin baseClass;
  public:
    mainWin(HINSTANCE hInstance);
    ~mainWin();

    bool create();

    bool preProcessMsg(MSG& msg);
    clientWin* getClientImage() { return clientFile; }
    extMonitor* getMonitor() { return extMon; }

    void refreshFolderImg();
    void refreshAudioBase(uint ix);
    void refreshAudioBaseTime(uint ix);
    void resetTimer();
    void refreshInfo(bool byCbx = false);
    const manageAudio* getManAudio() { return ManAudio; }
  protected:
    LPCTSTR getClassName() const { return _T("Escape_Room_Class"); };
    class pMenuBtn* menuBtn;
    class PSplitter* mainSplit;
    clientWin* clientFile;
    class fv_miniat_images* FMI;
    class PSplitWin* clientSplitWin;
    class P_TimerLed* tl;

    class extMonitor* extMon;
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void moveMon();
    void chooseImgFolder();
    uint idTimer;
    uint idTimerTime;
    manageAudio* ManAudio;
    int getVolumeAudio();

    manageTimer ManTimer;
    bool isRunning() { return toBool(idTimerTime); }
    void pauseTimer(bool all = true);
    void resumeTimer();
    void checkTimer();
    void changeAudio(bool baseSound);
    void addTips();
    void changeTips();

    DWORD lastTick;
};
//------------------------------------------------------------------------------
clientWin* getClientImage(PWin* child);
extMonitor* getMonitor(PWin* child);

#define SUB_BASE_KEY _T("Escape_Room")
#define INI_NAME _T("setup.ini")

extern void setKeyParam(LPCTSTR keyName, DWORD value);
extern void setKeyPath(LPCTSTR keyName, LPCTSTR path);
extern void getKeyParam(LPCTSTR keyName, LPDWORD value);
extern void getKeyPath(LPCTSTR keyName, LPTSTR path);

extern void makeKeyPath(LPTSTR path, size_t dim);
extern void makeRelPath(LPTSTR path, size_t dim);

#define IMG_PATH _T("ImgPath")
#define AUDIO_BASE _T("audioBase")
#define AUDIO_ALERT _T("audioAlert")
#define AUDIO_BASE_FOLDER _T("audioBaseFolder")
#define AUDIO_ALERT_FOLDER _T("audioAlertFolder")
#define AUDIO_ALERT_TIME _T("audioAlertTime")
#define MAX_TIME_TIMER _T("max_time_timer")

#define TEXT_BASE _T("text_")
#define TEXT_CURR _T("text_current")
//------------------------------------------------------------------------------
#endif
