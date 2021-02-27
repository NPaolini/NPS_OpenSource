//------------------ svmRule.h ------------------------
//-----------------------------------------------------------
#ifndef svmRule_H_
#define svmRule_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "p_util.h"
#include "pRect.h"
#include "svmManZoom.h"
//-----------------------------------------------------------
#define MUL_Z(z) (z >> 16)
#define DIV_Z(z) (z & 0xff)
#define MAKE_Z(m, d) (((m) << 16) | ((d) & 0xff))
#define BKG_COLOR_MDC_CUSTOM RGB(255, 255, 240)
//-----------------------------------------------------------
class mdcCustom
{
  public:
    mdcCustom() : mdc(0), hBmpTmp(0),
      bkg(0), cBkg(BKG_COLOR_MDC_CUSTOM),
      pen((HPEN)GetStockObject(BLACK_PEN)),
      oldMode(0), oldBkg(0), oldColor(0), oldAlign(0), oldPen(0)
      {}
    ~mdcCustom() { clear(); }

    void clear();
    HDC getMdc(PWin* owner, HDC hdc);

    void setBkg(COLORREF c) { cBkg = c; }
  private:
    PWin* Owner;
    HDC mdc;
    HBITMAP hBmpTmp;
    HGDIOBJ oldBmp;

    HBRUSH bkg;
    uint oldBkg;
    COLORREF cBkg;

    HPEN pen;
    HGDIOBJ oldPen;

    uint oldAlign;
    uint oldColor;
    uint oldMode;
};
//-----------------------------------------------------------
class PRule : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PRule(PWin* parent, uint id, HINSTANCE hInstance) : Start(0), Zoom(svmManZoom::zOne),
        isHorz(true), baseClass(parent, id, PRect(), 0, hInstance)
    {
      Attr.style = WS_VISIBLE | WS_CHILD;
    }
    virtual ~PRule() { destroy(); }

    virtual bool create();

    void setStartLg(long start);
    void setStartPx(long start);
    void setZoom(svmManZoom::zoomX zoom, bool force = false);

  protected:
    virtual LPCTSTR getClassName() const { return _T("svmRule_ClassName"); }
    virtual void getWindowClass(WNDCLASS& wcl);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void evPaint(HDC hdc) = 0;
    long Start;
    svmManZoom::zoomX Zoom;
    mdcCustom Mdc;
    bool isHorz;
};
//-----------------------------------------------------------
class PRuleX : public PRule
{
  private:
    typedef PRule baseClass;
  public:
    PRuleX(PWin* parent, uint id, HINSTANCE hInstance) : baseClass(parent, id, hInstance) { /*Start = 50;*/ }
    virtual ~PRuleX() { destroy(); }

  protected:
    virtual void evPaint(HDC hdc);
};
//-----------------------------------------------------------
class PRuleY : public PRule
{
  private:
    typedef PRule baseClass;
  public:
    PRuleY(PWin* parent, uint id, HINSTANCE hInstance) : baseClass(parent, id, hInstance) { isHorz = false; }
    virtual ~PRuleY() { destroy(); }

  protected:
    virtual void evPaint(HDC hdc);
};
//-----------------------------------------------------------
#endif
