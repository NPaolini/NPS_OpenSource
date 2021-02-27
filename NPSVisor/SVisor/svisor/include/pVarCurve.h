//-------- PVarCurve.h --------------------------------------------------------
#ifndef PVARCURVE_H_
#define PVARCURVE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

#include "pstatic.h"
#include "setPack.h"
//----------------------------------------------------------------------------
struct infoCurve
{
  COLORREF Fg;
  COLORREF Bkg;
  COLORREF ball;
  COLORREF grid;
  COLORREF axe;
  COLORREF lineShow;

  uint nRow;
  uint nPoints;
  uint perc1stSpace;
  uint nPoints1stSpace;
  uint readOnly;

  DWORD maxX;
  TCHAR fileShow[_MAX_PATH];
};
//----------------------------------------------------------------------------
union uShow
{
  struct dataShow
  {
    DWORD x;
    float v;
  } ds;
  POINT pt;
};
//----------------------------------------------------------------------------
class PVarCurve : public PWin, virtual public baseActive
{
  private:
    typedef baseActive baseClass;
    typedef PWin baseWinClass;
  public:
    PVarCurve(P_BaseBody* owner, uint id);

    virtual ~PVarCurve();

    bool evCtlColor(HBRUSH& result, HDC hdc, HWND hwnd, uint message);

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    bool create();
    virtual void setVisibility(bool set);

    void addReqVar(PVect<P_Bits*>& allBits);
    void resetClip() { clipped = false; }
  protected:
    virtual void performDraw(HDC hdc) {  }
    virtual uint getBaseId() const { return ID_INIT_VAR_CURVE; }

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HWND getHwnd() { return getHandle(); }

    HWND evSetAroundFocus(UINT key);

    void sendCurrData();
    bool reload(bool force = false);
    virtual void mouseLeave(const POINT& pt, uint flag);
  private:
    HBRUSH Brush;
    infoCurve IC;

    int Hand;
    int oldx;
    int oldy;
    uint Style;
    double* Wave;
    double* stepX;

    FILETIME ftLastMod;

    void evPaint(HDC hdc);
    void paint(HDC hdc, const PRect& r);

    void evMouse(uint message, uint key, POINT pt);

    void EvMouseLUp(uint modKeys, POINT point);
    void EvMouseRUp(uint modKeys, POINT point);
    void EvMouseLDn(uint modKeys, POINT point);
    void EvMouseRDn(uint modKeys, POINT point);
    void EvMouseMove (uint modKeys, POINT point);
    void MouseMove(uint modKeys, POINT point, int ok);


    void calcGrid();
    void drawBorder(HDC hdc, PRect r);
    void Grid(HDC hdc, PRect r);
    void trasf_point(POINT& point);
    void restore_point(POINT& point);
    void restore_point(POINT& point, uint H, double y);
    void draw_wave(HDC hdc);
    int check_point(const POINT& point);

    void new_ancor(const POINT& point, int rem_old = 1);
    void draw_line(const POINT& point);

    bool isChangedFileShow();
    void drawFileShow(HDC hdc);
    void recToPoint(uShow& ds, double mulX, double mulY, int height);

    void updateCoord(POINT pt);
    bool clipped;
    bool addToClipped(bool add);

    bool allocBmpWork(HDC hdc);
    bool allocBmpBkgWork(HDC hdc);
    void removeAllBmpWork(uint bits);

    HBITMAP hBmpWork;
    HDC mdcWork;
    HGDIOBJ oldObjWork;

    HBITMAP hBmpBkgWork;
    HDC mdcBkgWork;
    HGDIOBJ oldObjBkgWork;

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

