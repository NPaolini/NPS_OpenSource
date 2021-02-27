//--------- d_wait.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "d_wait.h"
#include "p_base.h"
#include "ptraspbitmap.h"
#include "PTextPanel.h"
//----------------------------------------------------------------------------
POINT operator +(const POINT& pt1, const POINT& pt2)
{
  POINT pt = { pt1.x + pt2.x, pt1.y + pt2.y };
  return pt;
}
//----------------------------------------------------------------------------
#define DIM_BALL 9
//#define DIM_BALL 14
#define DX (-(DIM_BALL / 2))
#define DY (-(DIM_BALL / 2))
#define POFFS(x, y) { (x) + DX, (y) + DY }

#if (DIM_BALL < 10)
  #define OFFS_Y 10
#else
  #define OFFS_Y DIM_BALL
#endif

#define Y1 56
#define Y2 40
#define Y3 22

#define OFFS_SUB (Y3 - OFFS_Y)
#define OFFS_ADD (Y3 + OFFS_Y)

static const POINT howMove1[] = {
      POFFS(40,Y1),
      POFFS(52,Y1),
      POFFS(64,Y1),
      POFFS(76,Y1),
      POFFS(63,Y2),
      POFFS(50,Y3),
      POFFS(62,Y3),
      POFFS(74,Y3),
      POFFS(87,Y3),
      POFFS(97,Y3),
      };
static const POINT howMove2[] = {
      POFFS(80,OFFS_SUB), POFFS(90,OFFS_SUB),
      POFFS(94,OFFS_ADD), POFFS(104,OFFS_ADD)
      };
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
enum tDir { LEFT, RIGHT };
//----------------------------------------------------------------------------
TD_Wait::TD_Wait(LPCTSTR msg, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Msg(msg), count(0), dir(LEFT)
{
  ball = new PBitmap(IDB_BALL, getHInstance());

//  bmp = new PBitmap(IDB_BKG_WAIT, getHInstance());

  POINT pt = howMove1[0];

  for(int i = 0; i < SIZE_A(tbmp); ++i) {
    tbmp[i] = new PTraspBitmap(this, ball, pt);
    if(i)
      tbmp[i]->setEnable(false);
    }
}
//----------------------------------------------------------------------------
TD_Wait::~TD_Wait()
{
  destroy();
  for(int i = 0; i < SIZE_A(tbmp); ++i)
    delete tbmp[i];
  delete ball;
}
//----------------------------------------------------------------------------
#define OFFS_Y_GEN 5
bool TD_Wait::create()
{
  Pt.x = -1;
  if(!baseClass::create())
    return false;

//  SetDlgItemText(*this, IDC_STATICTEXT_WAIT, Msg);
//  ShowWindow(GetDlgItem(*this, IDC_STATICTEXT_WAIT), SW_HIDE);
  PBitmap bmp(IDB_BKG_WAIT, getHInstance());
  PRect r;
  GetClientRect(*this, r);
  SIZE sz = bmp.getSize();
  Pt.x = (r.Width() - sz.cx) / 2;
  Pt.y = OFFS_Y_GEN;

  POINT pt = Pt + howMove1[0];
  tbmp[0]->moveToSimple(pt);

  SetTimer(*this, 100, 100, 0);
  InvalidateRect(*this, 0, true);
  return true;
}
//----------------------------------------------------------------------------
LRESULT TD_Wait::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      KillTimer(hwnd, 100);
      break;


#define USE_MEMDC
#ifdef USE_MEMDC

    case WM_ERASEBKGND:
      return 1;
#endif
    case WM_PAINT:
      if(GetUpdateRect(hwnd, 0, 0)) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if(!hdc) {
          EndPaint(hwnd, &ps);
          break;
          }

#ifdef USE_MEMDC
        PRect r;
        GetClientRect(*this, r);
        if(r.Width() && r.Height()) {
          HBITMAP hBmpWork = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
          if(hBmpWork) {
            HDC mdcWork = CreateCompatibleDC(hdc);
            HGDIOBJ oldObjWork = SelectObject(mdcWork, hBmpWork);
            COLORREF bkg = GetSysColor(COLOR_BTNFACE);
            HBRUSH br = CreateSolidBrush(bkg);
            FillRect(mdcWork, r, br);
            DeleteObject(br);
            evPaint(mdcWork);
            BitBlt(hdc, r.left, r.top, r.Width(), r.Height(), mdcWork, r.left, r.top, SRCCOPY);
            SelectObject(mdcWork, oldObjWork);
            DeleteDC(mdcWork);
            DeleteObject(hBmpWork);
            }
          }
#else
        evPaint(hdc);
#endif
        EndPaint(hwnd, &ps);

        } while(false);

      return 0;
/*
    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        evPaint(hdc);
        EndPaint(*this, &ps);
        } while(false);
      return 0;
//      break;
*/
    case WM_TIMER:
      if(evTimer(wParam))
        return 0;
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void TD_Wait::evPaint(HDC hdc)
{
  if(Pt.x < 0 || !hdc)
    return;
#if 1
  PBitmap bmp(IDB_BKG_WAIT, getHInstance());
  PTraspBitmap bkg(this, &bmp, Pt);
  bkg.Draw(hdc);
#endif
  for(int i = 0; i < SIZE_A(tbmp); ++i)
    tbmp[i]->Draw(hdc);

  PRect rect;
  GetClientRect(*this, rect);
  rect.top = rect.bottom - 22;
  PTextFixedPanel Text(Msg, rect, (HFONT)GetStockObject(DEFAULT_GUI_FONT), RGB(0, 0, 127), PPanel::NO, 0, PTextPanel::NO3D);
  Text.setAlign(TA_CENTER);
  Text.setVAlign(DT_CENTER);
  Text.draw(hdc);
}
//----------------------------------------------------------------------------
bool TD_Wait::evTimer(uint idTimer)
{
//  if(true)
  if(100 != idTimer)
    return false;
  if(SIZE_A(howMove1) - 2 == count) {
    for(int i = 1; i < SIZE_A(tbmp); ++i) {
      tbmp[i]->setEnable(true);
      POINT pt = howMove2[(i - 1) * 2] + Pt;
      tbmp[i]->moveToSimple(pt);
      }
    }
  else if(SIZE_A(howMove1) - 2 < count) {
    for(int i = 1; i < SIZE_A(tbmp); ++i) {
      POINT pt = howMove2[(i - 1) * 2 + 1] + Pt;
      tbmp[i]->moveToSimple(pt);
      }
    }
  else if(SIZE_A(howMove1) - 3 == count) {
    for(int i = 1; i < SIZE_A(tbmp); ++i) {
      tbmp[i]->setEnable(false);
      }
    }

  POINT pt = howMove1[count] + Pt;
  tbmp[0]->moveToSimple(pt);

  if(LEFT == dir)
    ++count;
  else
    --count;
  if(SIZE_A(howMove1) == count) {
    dir = RIGHT;
    count -= 2;
//    --count;
    }
  else if(count < 0) {
    dir = LEFT;
    count += 2;
//    ++count;
    }
  InvalidateRect(*this, 0, 0);
  return true;
}

