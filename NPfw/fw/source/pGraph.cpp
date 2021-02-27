//-------------------- pGraph.CPP ---------------------------
#include "precHeader.h"

#include "pGraph.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PGraph::PGraph(const PRect& rect, COLORREF colorBkg, bStyle type,
      COLORREF colorFrg, bool horz, bool to_right)
  : PPanel(rect, colorBkg, type), bar(colorFrg), perc(0),
    Horz(horz), toRight(to_right)  { }
//-----------------------------------------------------------
void PGraph::setPerc(int p)
{
  if(p < 0)
    p = 0;
  else if(p > 100)
    p = 100;
  if(p != perc) {
    perc = p;
//    invalidate();
    }
}
//-----------------------------------------------------------
void PGraph::drawBkg(HDC hdc)
{
  PPanel::draw(hdc);
}
//-----------------------------------------------------------
void PGraph::drawBar(HDC hdc)
{
  if(!perc)
    return;
  PRect rect = getRect();
  double p = perc / 100.0;
  if(Horz) {
    if(toRight)
      rect.right = rect.left + (int)(rect.Width() * p + 0.5);
    else
      rect.left = rect.right - (int)(rect.Width() * p + 0.5);
    }
  else {
    if(toRight)
      rect.top = rect.bottom - (int)(rect.Height() * p + 0.5);
    else
      rect.bottom = rect.top + (int)(rect.Height() * p + 0.5);
    }
  rect.Inflate(-1, -1);

  HBRUSH br = CreateSolidBrush(bar);
  FillRect(hdc, rect, br);
  DeleteObject(HGDIOBJ(br));
}
//-----------------------------------------------------------
void PGraph::draw(HDC hdc)
{
  drawBkg(hdc);
  drawBar(hdc);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
PLinearGraph::PLinearGraph(const PRect& rect, COLORREF colorBkg, bStyle type,
      COLORREF colorFrg, bool has_axis)
  : PPanel(rect, colorBkg, type), linear(colorFrg), tick(1), axis(has_axis)
{  }
//---------------------------------------------------------
#define RED_COMP(v)    (DWORD)(GetRValue(v) * 0.299)
#define GREEN_COMP(v)  (DWORD)(GetGValue(v) * 0.587)
#define BLUE_COMP(v)   (DWORD)(GetBValue(v) * 0.114)
//-----------------------------------------------------------
static bool highGray(COLORREF col)
{
  DWORD v = RED_COMP(col) + GREEN_COMP(col) + BLUE_COMP(col);
  return v >= 127;
}
//-----------------------------------------------------------
#define BLACK RGB(0, 0, 0)
#define WHITE RGB(0xff, 0xff, 0xff)
//-----------------------------------------------------------
void PLinearGraph::drawAx(HDC hdc)
{
  // se lo sfondo è scuro, la linea centrale è bianca, altrimenti nera
  HPEN pen = CreatePen(PS_SOLID, 0, highGray(getColor()) ? BLACK : WHITE);
  HGDIOBJ old = SelectObject(hdc, pen);
  int y = (getRect().top + getRect().bottom) / 2;
  MoveToEx(hdc, getRect().left + 1, y, 0);
  LineTo(hdc, getRect().right - 1, y);
  SelectObject(hdc, old);
  DeleteObject(pen);
}
//-----------------------------------------------------------
void PLinearGraph::drawLines(HDC hdc)
{
  HPEN pen = CreatePen(PS_SOLID, tick, linear);
  HGDIOBJ old = SelectObject(hdc, pen);

  int hMax = getRect().Height() - 2;

  int wMax = nPoints;
  if(wMax <= 0)
    wMax = getRect().Width() - 2;

  sPoint init = getNext();

  int width = getRect().Width() - 2;
  if(init.y <= 0)
    init.y = 1;
  else if(init.y >= hMax)
    init.y = lgCoord(hMax - 1);

  if(init.x <= 0)
    init.x = 1;
  else if(init.x >= width)
    init.x = lgCoord(width - 1);

  int left = getRect().left + 2;
  int top = getRect().top;

  POINT* pp = new POINT[wMax];
  pp[0].x = init.x + left;
  pp[0].y = init.y + top;

  int i = 1;
  for(; i < wMax; ++i) {
    init = getNext();
    if(init.y <= 0)
      break;

    else if(init.y >= hMax)
      init.y = lgCoord(hMax - 1);
    if(init.x <= 0)
      break;

    else if(init.x >= width)
      init.x = lgCoord(width - 1);

    pp[i].x = init.x + left;
    pp[i].y = init.y + top;
    }
  Polyline(hdc, pp, i);
  delete []pp;
  SelectObject(hdc, old);
  DeleteObject(pen);
}
//-----------------------------------------------------------
void PLinearGraph::draw(HDC hdc)
{
  PPanel::draw(hdc);
  if(axis)
    drawAx(hdc);

  drawLines(hdc);
}
