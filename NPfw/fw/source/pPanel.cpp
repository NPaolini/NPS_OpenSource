//-------------------- pPanel.CPP ---------------------------
#include "precHeader.h"

#include "ppanel.h"
//-----------------------------------------------------------
PPanel::PPanel(const PRect& rect, COLORREF color, bStyle type) :
    Rect(rect), Style(type), Color(color), Hide(false), Bkg(0)
{ }
//-----------------------------------------------------------

//-----------------------------------------------------------
void PPanel::clone(const PPanel& other)
{
  if(&other != this) {
    delete Bkg;
    Bkg = 0;
    CLONE(Hide);
    CLONE(Rect);
    CLONE(Color);
    CLONE(Style);
    CLONE(Border);
    }
}
//-----------------------------------------------------------
void PPanel::draw2(HWND win)
{
  if(isHided())
    return;

  HDC hdc = GetDC(win);
  draw(hdc);
  ReleaseDC(win, hdc);

//  ValidateRect(win, getRect());
}
//-----------------------------------------------------------
void PPanel::paint(HDC hdc)
{
  if(TRANSP & Style) {
    POINT pt = { Rect.left, Rect.top };
    SIZE sz = { Rect.Width(), Rect.Height() };
    delete Bkg;

    Bkg = new PBitmap(hdc, pt, sz);
    }
  draw(hdc);
}
//-----------------------------------------------------------
void PPanel::draw(HDC hdc)
{
  if(isHided())
    return;

  if(TRANSP & Style) {
    if(Bkg) {
      POINT pt = { Rect.left, Rect.top };
      Bkg->draw(hdc, pt);
      }
    }
  if(FILL & Style) {
    HBRUSH br = CreateSolidBrush(Color);
#if 1
    FillRect(hdc, Rect, br);
#else
    PRect r(Rect);
    r.Inflate(-1, -1);
    FillRect(hdc, r, br);
#endif
    DeleteObject(HGDIOBJ(br));
    }

  if((DN | UP | BORDER) & Style) {
    if(BORDER & Style && !Border.isSimple())
      Border.paint(hdc, Rect);
    else {
      int oldMode = ::SetBkMode(hdc, TRANSPARENT);
      HPEN penBlack = (HPEN)GetStockObject(BLACK_PEN);
      HPEN penWhite = (HPEN)GetStockObject(WHITE_PEN);

      HPEN p1 = penBlack;
      HPEN p2 = penBlack;
      if(UP & Style)
        p1 = penWhite;

      else if(DN & Style)
        p2 = penWhite;

      HGDIOBJ old = SelectObject(hdc, p1);

      MoveToEx(hdc, Rect.left, Rect.bottom - 1, 0);
      LineTo(hdc, Rect.left, Rect.top);
      LineTo(hdc, Rect.right - 1, Rect.top);

      SelectObject(hdc, p2);
      LineTo(hdc, Rect.right - 1, Rect.bottom - 1);
      LineTo(hdc, Rect.left, Rect.bottom - 1);

      SelectObject(hdc, old);
      SetBkMode(hdc, oldMode);
      }
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
static HPEN createPen(const PPanelBorder::pbInfo& info)
{
  if(PPanelBorder::pbOn == (info.style & PPanelBorder::pbOn))
    return CreatePen(PS_SOLID, info.tickness, info.color);
  return 0;
}
//-----------------------------------------------------------
void PPanelBorder::paint(HDC hdc, const PRect& rect)
{
  int oldMode = ::SetBkMode(hdc, TRANSPARENT);
  HPEN pen[SIZE_A_c(Info)] = { 0 };
  pen[pbTop] = createPen(Info[pbTop]);
  pen[pbRight] = createPen(Info[pbRight]);
  pen[pbBottom] = createPen(Info[pbBottom]);
  pen[pbLeft] = createPen(Info[pbLeft]);

  HGDIOBJ old = 0;
  PRect r(rect);
  --r.bottom;
  --r.right;
  if(pen[pbTop]) {
    old = SelectObject(hdc, pen[pbTop]);
    MoveToEx(hdc, r.left, r.top, 0);
    LineTo(hdc, r.right, r.top);
    }
  if(pen[pbRight]) {
    HGDIOBJ t = SelectObject(hdc, pen[pbRight]);
    if(!old)
      old = t;
    MoveToEx(hdc, r.right, r.top, 0);
    LineTo(hdc, r.right, r.bottom);
    }
  if(pen[pbBottom]) {
    HGDIOBJ t = SelectObject(hdc, pen[pbBottom]);
    if(!old)
      old = t;
    MoveToEx(hdc, r.right, r.bottom, 0);
    LineTo(hdc, r.left, r.bottom);
    }
  if(pen[pbLeft]) {
    HGDIOBJ t = SelectObject(hdc, pen[pbLeft]);
    if(!old)
      old = t;
    MoveToEx(hdc, r.left, r.bottom, 0);
    LineTo(hdc, r.left, r.top);
    }

  if(old)
    SelectObject(hdc, old);

  for(uint i = 0; i < SIZE_A(pen); ++i)
    if(pen[i])
      DeleteObject(pen[i]);
  SetBkMode(hdc, oldMode);
}
/*
// esempio nuova classe derivata da PStatic che usa il textPanel
//-------------------------------------------
//----------------------------------------------------------------------------
class testText : public PStatic
{
  private:
    typedef PStatic baseClass;
  public:
    testText(PWin * parent, uint resid, HINSTANCE hinst = 0) : baseClass(parent, resid, hinst), Text(0) {}
    ~testText() { destroy(); delete Text; }
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PTextFixedPanel* Text;
};
//----------------------------------------------------------------------------
bool testText::create()
{
  if(!baseClass::create())
    return false;
  PRect r;
  GetWindowRect(*this, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
  Text = new PTextFixedPanel(_T("prova"), r, getFont(), RGB(0, 0, 0), PPanel::BORDER_FILL, RGB(0xca,0xca,0xff));
  PPanelBorder::pbInfo info;
  info.style = PPanelBorder::pbOn;
  info.tickness = 2;
  info.color = RGB(100, 100, 255);
  Text->setBorder(PPanelBorder::pbTop, info);
#if 0
  info.tickness = 2;
  info.color = RGB(127, 255, 255);
  Text->setBorder(PPanelBorder::pbLeft, info);
  info.color = RGB(0, 0, 200);
  Text->setBorder(PPanelBorder::pbBottom, info);
  info.tickness = 4;
#endif
  info.color = RGB(255, 100, 100);
  Text->setBorder(PPanelBorder::pbBottom, info);
  Text->setAlign(TA_CENTER); // TA_LEFT | TA_CENTER | TA_RIGHT
  Text->setVAlign(DT_VCENTER); // DT_TOP | DT_VCENTER | DT_BOTTOM

  return true;
}
//----------------------------------------------------------------------------
LRESULT testText::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if(!hdc) {
          EndPaint(hwnd, &ps);
          break;
          }
        if(Text)
          Text->draw(hdc);
        EndPaint(hwnd, &ps);

        } while(false);
//      SetWindowLong(hwnd, DWL_MSGRESULT, 0);

      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}

*/
