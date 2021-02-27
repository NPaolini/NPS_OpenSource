//-------- PSliderWin.h ----------------------------------------------
//-------------------------------------------------------------------
#ifndef PSliderWin_H
#define PSliderWin_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pVarTrend.h"
//----------------------------------------------------------------------------
class PSliderWin : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PSliderWin(PWin * parent, uint id, const PRect& r, PBitmap* bkg, const PVect<infoVar>& varName,
        const infoTrend& iTrend, DWORD bitShow, LPCTSTR title = 0, HINSTANCE hinst = 0);

    ~PSliderWin ();

    virtual bool create();
    void setLimits(int left, int right);
    void getLimits(int& left, int& right) { right = Right; left = Left; }

  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evPaint(HDC hdc, bool eraseBkg);
    void evPaintBkg(HDC hdc);
    void evMouse(UINT message, WPARAM wParam, LPARAM lParam);

    void evMouseMove(const POINT& pt);
    void evMouseLDown(const POINT& pt);
    void evMouseLUp(const POINT& pt);
  private:
    class PShowValueWin* ChildPop;
    PBitmap* Bkg;
    int Left;
    int Right;

    enum whichDrag { dNone, dLeft, dRight };
    DWORD onDragging;
    DWORD onAnchor;
    COLORREF LineShow;

    bool isOverObject(int curr, const POINT& pt);
    void paintSlider(int x, HDC hdc = 0);
    void setPointer(uint type);
    void movePix(int dir, whichDrag set, bool refresh = false);
    void evKeyMove(int x);
    void fillAndSet(int x, DWORD which_drag);

    friend class myBtn;
};
//-------------------------------------------------------------------
#endif
