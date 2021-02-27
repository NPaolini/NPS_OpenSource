//-------------------- pAviCap.h ----------------------------
#ifndef PAVICAP_H_
#define PAVICAP_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "PCrt_lck.h"
//-----------------------------------------------------------
#ifndef PWIN_H_
  #include "pWin.h"
#endif

#if 0
#if !defined(_INC_VFW)
    #define NODRAWDIB
    #define NOAVIFMT
    #define NOMMREG
    #define NOAVIFILE
    #define NOMCIWND
    #define NOMSACM
    #include "vfw.h"
#endif
#else
    #include "vfw.h"
#endif
//-----------------------------------------------------------
#define DEFAULT_CAPTURE_DRIVER  0
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class aviCapInfo
{
  public:
    aviCapInfo(LPCTSTR filename = 0) : Connected(false) { init(filename); }

    bool connect(HWND hwnd);
    bool closeConn(HWND hwnd);

    bool VideoFormatDialog(HWND hwnd);
    bool VideoSourceDialog(HWND hwnd);
    bool VideoDisplayDialog(HWND hwnd);
    bool startPreview(HWND hwnd);
    bool stopPreview(HWND hwnd);

    bool setCompressionType(HWND hwnd);
    bool startCaptureFile(HWND hwnd, LPCTSTR filename);
    bool stopCaptureFile(HWND hwnd);

    bool setRatePreview(HWND hwnd, DWORD_PTR msec);
    DWORD_PTR getRatePreview() { return frameRate; }

    bool load(LPCTSTR filename);
    bool save(LPCTSTR filename);

    bool isConnected() { return Connected; }
    enum Stat {
        No_action = 0,
        onPreview = 1,
        onOverlay = 2,
        onCapture = 4,
        onCapturePreview = (onPreview | onCapture),
        onCaptureOverlay = (onOverlay | onCapture),
        onView = (onPreview | onOverlay)
        };
    DWORD_PTR getCurrStat(HWND hwnd);

    DWORD_PTR preDialog(HWND hwnd);
    void postDialog(HWND hwnd, DWORD_PTR stat);

  private:
    TCHAR deviceName[120];
    DWORD_PTR frameRate;
    CAPDRIVERCAPS caps;
    CAPTUREPARMS params;

    void init(LPCTSTR filename);
    void reload(HWND hwnd);
    bool Connected;
};
//-----------------------------------------------------------
/*
struct aviCapInfo
{
  int device;
  int frameRate;

  aviCapInfo(int dev = DEFAULT_CAPTURE_DRIVER) : device(dev), frameRate(100) {}
};
*/
//-----------------------------------------------------------
class IMPORT_EXPORT PAviCap : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PAviCap(PWin * parent, uint id, LPCTSTR filename, int x, int y, int w, int h, LPCTSTR title = 0, HINSTANCE hinst = 0);
    PAviCap(PWin * parent, uint id, LPCTSTR filename, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0);
    ~PAviCap ();

    virtual bool create();
    bool VideoFormatDialog() { return ACI.VideoFormatDialog(*this); }
    bool VideoSourceDialog() { return ACI.VideoSourceDialog(*this); }
    bool VideoDisplayDialog() { return ACI.VideoDisplayDialog(*this); }
    bool startPreview() { return ACI.startPreview(*this); }
    bool stopPreview() { return ACI.stopPreview(*this); }

    bool setCompressionType() { return ACI.setCompressionType(*this); }
    bool startCaptureFile(LPCTSTR filename);
    bool stopCaptureFile();

    bool setRatePreview(DWORD_PTR msec) { return ACI.setRatePreview(*this, msec); }
    DWORD_PTR getRatePreview() { return ACI.getRatePreview(); }

    bool isValid() { return ACI.isConnected(); }

    bool load(LPCTSTR filename) { return isValid() ? false : ACI.load(filename); }
    bool save(LPCTSTR filename) { return ACI.save(filename); }

    DWORD_PTR getCurrStat() { return ACI.getCurrStat(*this); }

    DWORD_PTR preDialog() { return ACI.preDialog(*this); }
    void postDialog(DWORD_PTR stat) { ACI.postDialog(*this, stat); }

    bool connect() { return getHandle() ? ACI.connect(*this) : false; }
    bool disconnect() { return getHandle() ? ACI.closeConn(*this) : false; }

//    void Show(const PRect& r);
  protected:

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR getClassName() const  {  return _T("pAviCap_ClassName");   }
    virtual HWND performCreate();
  private:
    void evDestroy(HWND hwnd);
    aviCapInfo ACI;
    int inExec;

};
//-----------------------------------------------------------
inline
PAviCap::PAviCap(PWin * parent, uint id, LPCTSTR filename, int x, int y, int w, int h, LPCTSTR title, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, title, hinst), ACI(filename)
{
  Attr.style |= WS_CHILD;
}
//-----------------------------------------------------------
inline
PAviCap::PAviCap(PWin * parent, uint id, LPCTSTR filename, const PRect& r, LPCTSTR title, HINSTANCE hinst):
    baseClass(parent, id, r, title, hinst), ACI(filename)
{
  Attr.style |= WS_CHILD;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
class IMPORT_EXPORT PAviPlay : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PAviPlay(PWin * parent, uint id, int x, int y, int w, int h, LPCTSTR title = 0, HINSTANCE hinst = 0);
    PAviPlay(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0);
    ~PAviPlay ();

    virtual bool create();

    bool isStretchable() { return canStretch; }
    bool setStretch(bool set, bool refresh = true);
    bool open(LPCTSTR filename);
    bool close();
    bool startPlay(LPCTSTR filename = 0, DWORD from = (DWORD)-1, DWORD to = (DWORD)-1);
    bool stopPlay(LPDWORD lpCurpos = 0);

    DWORD_PTR getCurrFrame();
    DWORD_PTR getTotalFrame();
    bool setFrameRate(DWORD rate);
//    void Show(const PRect& r);
  protected:

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR getClassName() const  {  return _T("pAviPlay_ClassName");   }
    virtual void getWindowClass(WNDCLASS& wcl);
    virtual void evPaint(HDC hdc, const RECT& rc);
  private:
    bool evSize();
    MCIDEVICEID DeviceID;
    bool canStretch;
    bool bySelf;
};
//-----------------------------------------------------------
inline
PAviPlay::PAviPlay(PWin * parent, uint id, int x, int y, int w, int h, LPCTSTR title, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, title, hinst), DeviceID(0), canStretch(true), bySelf(false)
{
  Attr.style |= WS_CHILD | WS_BORDER;
}
//-----------------------------------------------------------
inline
PAviPlay::PAviPlay(PWin * parent, uint id, const PRect& r, LPCTSTR title, HINSTANCE hinst):
    baseClass(parent, id, r, title, hinst), DeviceID(0), canStretch(true), bySelf(false)
{
  Attr.style |= WS_CHILD | WS_BORDER;
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
//-----------------------------------------------------------
