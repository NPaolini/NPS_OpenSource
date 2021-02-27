//-------- lgraph2-zoom.h ----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef lgraph2_zoom_H_
#define lgraph2_zoom_H_
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "Commonlgraph.h"
#include "svdialog.h"
#include "p_Vect.h"
//----------------------------------------------------------------------------
class linear;
//----------------------------------------------------------------------------
struct zoomInfoVar
{
  double minVal;
  double maxVal;
  const lgCoord* sourceY;

  COLORREF frg;
};
//----------------------------------------------------------------------------
struct zoomInfo
{
  PRect full;
  PRect sel;

  PVect<zoomInfoVar> infoVar;
  FILETIME init;
  FILETIME end;
  const lgCoord* sourceX;

  uint totPoint;

  int Dec;
  int nLabel;

  uint ixActive;

  COLORREF bkg;
  COLORREF textColor;

  HFONT hFont;

};
//----------------------------------------------------------------------------
class lgraph2_zoom : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    lgraph2_zoom(PWin* parent, const zoomInfo& sourceInfo);
    virtual ~lgraph2_zoom();
    virtual bool create();
    virtual bool preProcessMsg(MSG& msg);

  protected:
    void evPaint(HDC hdc);
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    struct varInfo {
      linear *lGraph;
      lgCoord* yGraph;
      varInfo() : lGraph(0), yGraph(0) {}
      };
    PVect<varInfo> VarInfo;
    lgCoord* XCoord;
    uint totPoint;
    PRect gfxRect;
    uint ixForeground;

    FILETIME ftInit;
    FILETIME ftEnd;

    double minVal;
    double maxVal;
    int nLabel;
    int Dec;
    COLORREF textColor;
    COLORREF bkgColor;

    HFONT hFont;

    void drawYInfo(HDC hdc);
};
//----------------------------------------------------------------------------
class manage_coord
{
  public:
    manage_coord(const PRect& inside) : Stat(eNoStat), inside(inside) {}
    virtual ~manage_coord() {}

    // torna uno se è stato disegnato un rettangolo, meno uno se sta disegnando il rettangolo, zero negli altri casi
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    const PRect& getRect() const { return frame; }

  private:
    enum { eNoStat, eClicked, eOnDrag, eReleased };
    uint Stat;
    PRect frame;
    PRect inside;
    bool evMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam);
    bool evMouseLDown(HWND hwnd, WPARAM wParam, LPARAM lParam);
    bool evMouseLUp(HWND hwnd, WPARAM wParam, LPARAM lParam);
};
//----------------------------------------------------------------------------
struct infoPointedValue
{
  FILETIME ft;
  TCHAR name[MAX_LINEAR_GRAPH][MAX_DESCR + 1];
  double value[MAX_LINEAR_GRAPH];
  COLORREF color[MAX_LINEAR_GRAPH];
  uint nValue;
};
//----------------------------------------------------------------------------
class lgraph2_ShowValue : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    lgraph2_ShowValue(PWin* parent, const infoPointedValue& sourceInfo);
    virtual ~lgraph2_ShowValue();
    virtual bool create();
    virtual bool preProcessMsg(MSG& msg);

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC hdc, HWND hwnd, uint message);

  private:
    const infoPointedValue& sourceInfo;
    HBRUSH hBrush[MAX_LINEAR_GRAPH];
};
//----------------------------------------------------------------------------
#endif
