//------ common_color.h -----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef common_color_H
#define common_color_H
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "pEdit.h"
#include "pStatic.h"
#include "svmMainClient.h"
//----------------------------------------------------------------------------
#define WM_CUSTOM_SET_FOCUS (WM_FW_FIRST_FREE + 1)
//-----------------------------------------------------------
class PRGBFilter : public PFilter
{
  public:
    bool accept(WPARAM& key, HWND ctrl);
    virtual PFilter* allocCopy()  const { return t_allocCopy<PRGBFilter>(*this); }
};
//----------------------------------------------------------------------------
class P_EditFocus : public PEdit
{
  public:
    P_EditFocus(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, int textlen = 255, HINSTANCE hinst = 0) :
          PEdit(parent, id, r, text, textlen, hinst)
          {
            Attr.style &= ~ES_LEFT;
            Attr.style |= ES_CENTER | ES_MULTILINE | ES_NUMBER;
            setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
          }
    P_EditFocus(PWin * parent, uint id, int textlen = 255, HINSTANCE hinst = 0) :
          PEdit(parent, id, textlen, hinst)
          {
            setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
          }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
#define CYAN_COLOR  RGB(0xcf,0xcf,0xff)
#define bkgColor2 CYAN_COLOR
//----------------------------------------------------------------------------
#define LABEL_COLOR  RGB(218,236,215)
#define bkgColor3 LABEL_COLOR
//----------------------------------------------------------------------------
class PColorBox : public PStatic
{
  public:
    PColorBox(PWin * parent, COLORREF color, uint resid, HINSTANCE hinst = 0);
    PColorBox(PWin * parent, COLORREF color, uint id, const PRect& r, LPCTSTR text = 0, HINSTANCE hinst = 0);
    ~PColorBox();

    COLORREF getColor() const { return Color; }
    HBRUSH getBrush() const { return Brush; }
    void set(COLORREF color);
  private:
    COLORREF Color;
    HBRUSH Brush;
};
//----------------------------------------------------------------------------
#endif
