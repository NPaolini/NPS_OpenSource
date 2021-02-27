//-------------------- pTextPanel.H ---------------------------
#ifndef PTEXTPANEL_H_
#define PTEXTPANEL_H_
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PPANEL_H_
  #include "pPanel.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PTextPanel : public PPanel
{
  public:

    enum b3DStyle { NO3D, Black = 1 << 0, White = 1 << 1, All = Black | White };

    PTextPanel(LPCTSTR str,
                int x, int y,
                HFONT font,
                COLORREF txt,
                PPanel::bStyle type = PPanel::NO,
                COLORREF bkg = GetSysColor(COLOR_BTNFACE),
                b3DStyle style3d = PTextPanel::NO3D);

    virtual ~PTextPanel();

    PTextPanel(const PTextPanel& other) : PPanel(other), Str(0) { clone(other); }
    const PTextPanel& operator=(const PTextPanel& other) { clone(other); return *this; }

    virtual bool setText(LPCTSTR newString);

    virtual void setTextAndDraw(LPCTSTR newString, HDC hdc);
    virtual void setTextAndDraw(LPCTSTR newString, HWND hwnd);

    void setText(int value);
    void setAlign(int align); // TA_LEFT | TA_CENTER | TA_RIGHT
    void setVAlign(int align); // DT_TOP | DT_VCENTER | DT_BOTTOM

    void setFont(HFONT font);
    void setTextColor(COLORREF color);

    LPCTSTR getText() const;
    HFONT getFont() const;

    virtual void drawText(HDC hdc, LPCTSTR txt);

    COLORREF getTextColor() const;
    b3DStyle get3DStyle() const;
    int getAlign() const;
    int getVAlign() const;
  protected:
    void clone(const PTextPanel& other);
  private:
    COLORREF TxtColor;
    HFONT Font;
    TCHAR *Str;
    b3DStyle Style3D;
    int Align;
    int vAlign;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline void PTextPanel::setTextColor(COLORREF color)
{
  TxtColor = color;
}
//-----------------------------------------------------------
inline void PTextPanel::setFont(HFONT font)
{
  Font = font;
}
//-----------------------------------------------------------
inline void PTextPanel::setAlign(int align)
{
  Align = align;
}
//-----------------------------------------------------------
inline int PTextPanel::getAlign() const
{
  return Align;
}
//-----------------------------------------------------------
inline void PTextPanel::setVAlign(int align)
{
  vAlign = align;
}
//-----------------------------------------------------------
inline int PTextPanel::getVAlign() const
{
  return vAlign;
}
//-----------------------------------------------------------
inline COLORREF PTextPanel::getTextColor() const
{
  return TxtColor;
}
//-----------------------------------------------------------
inline HFONT PTextPanel::getFont() const
{
  return Font;
}
//-----------------------------------------------------------
inline PTextPanel::b3DStyle PTextPanel::get3DStyle() const
{
  return Style3D;
}
//-----------------------------------------------------------
inline LPCTSTR PTextPanel::getText() const
{
  return Str;
}
//-----------------------------------------------------------
inline void PTextPanel::setTextAndDraw(LPCTSTR newString, HDC hdc)
{
  setText(newString);
  draw(hdc);
}
//-----------------------------------------------------------
inline void PTextPanel::setTextAndDraw(LPCTSTR newString, HWND hwnd)
{
  setText(newString);
  draw2(hwnd);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
class IMPORT_EXPORT PTextAutoPanel : public PTextPanel
{
  public:
    PTextAutoPanel(
                LPCTSTR str,
                int x, int y,
                HFONT font,
                COLORREF txt,
                PPanel::bStyle type = PPanel::NO,
                COLORREF bkg = GetSysColor(COLOR_BTNFACE),
                b3DStyle style3d = PTextPanel::NO3D);

    // N.B. non viene usato l'allineamento verticale
    virtual void draw(HDC hdc);

    void setCoord(int x, int y);

    virtual bool setText(LPCTSTR newString);

  protected:
    bool calculated;
    int X;
    int Y;
};
//-----------------------------------------------------------
inline void PTextAutoPanel::setCoord(int x, int y)
{
  X = x;
  Y = y;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
class IMPORT_EXPORT PTextFixedPanel : public PTextPanel
{
  public:
    PTextFixedPanel(LPCTSTR str,
                     const PRect& rect,
                     HFONT font,
                     COLORREF txt,
                     PPanel::bStyle type = DN,
                     COLORREF bkg = GetSysColor(COLOR_BTNFACE),
                     b3DStyle style3d = PTextPanel::NO3D
                     );

    virtual void draw(HDC hdc);
};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
