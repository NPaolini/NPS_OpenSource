//------ svmPropertyTrendColor.h ---------------------------------------------
//----------------------------------------------------------------------------
#ifndef svmPropertyTrendColor_H_
#define svmPropertyTrendColor_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "pModDialog.h"
#include "pEdit.h"
#include "pStatic.h"
#include "svmMainClient.h"
#include "common_color.h"
#include "svmPropertyTrend.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PSimpleColorRowTrend
{
  public:
    enum checkFocus { LOSS = -1, NOCHANGE, GET };

    PSimpleColorRowTrend(PWin* parent, uint first_id, int x);
    virtual ~PSimpleColorRowTrend() {}

    // passando 0 come HWND si forza il settaggio con il ridisegno
    virtual checkFocus setFocus(uint firstIdSB, HWND hwnd = 0);
    void setColor(COLORREF color);
    void setColorBySelf(uint firstIdSB);
    COLORREF getColor();
    PColorBox* getBox() { return cbx; }
    bool hasFocus() { return Focus; }

    virtual void setAll(LPCTSTR p);
  protected:
    PEdit* rgb[3];
    PColorBox* cbx;
    uint firstId;
    bool Focus;
};
//----------------------------------------------------------------------------
class PColorRowTrend : public PSimpleColorRowTrend
{
  private:
    typedef PSimpleColorRowTrend baseClass;
  public:

    PColorRowTrend(PWin* parent, uint first_id, int x);

    // passando 0 come HWND si forza il settaggio con il ridisegno
    checkFocus setFocus(uint firstIdSB, HWND hwnd = 0);

    WORD getPtScr();
    void setPtScr(WORD val);

    void setAll(LPCTSTR p);
  protected:
    PEdit* ptScr;
};
//----------------------------------------------------------------------------
class svmPropertyTrendColor : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmPropertyTrendColor(PWin* parent, PVect<colorLine>& cl, uint resId = IDD_TREND_CHOOSE_COLOR, HINSTANCE hinstance = 0);
    virtual ~svmPropertyTrendColor();

    virtual bool create();
    void setCurrColor(COLORREF c) { CurrColor->set(c); }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();
    virtual HWND getFirstFocus();
  private:
    PVect<PColorRowTrend*> Rows;
    PColorBox* BaseColor[3];
    PColorBox* CurrColor;

    PVect<colorLine>& ColorLine;

    void evHScrollBar(HWND child, int flags, int pos);
};
//-------------------------------------------------------------------
#endif
