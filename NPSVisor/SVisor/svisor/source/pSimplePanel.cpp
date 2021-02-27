//------ PSimplePanel.cpp ---------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "PSimplePanel.h"
#include "p_util.h"
//----------------------------------------------------------------------------
void loadBorder(LPCTSTR p, PPanelBorder& Border);
//----------------------------------------------------------------------------
PSimplePanel::PSimplePanel(P_BaseBody* owner, uint id) : baseSimple(owner, id),
           Panel(0){}
//----------------------------------------------------------------------------
PSimplePanel::~PSimplePanel()
{
  delete Panel;
}
//----------------------------------------------------------------------------
bool PSimplePanel::allocObj(LPVOID)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int R = 0;
  int G = 0;
  int B = 0;
  int up_dn_fill = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &up_dn_fill,
                    &R, &G, &B);
  p = findNextParamTrim(p, 8);
  PPanelBorder border;
  loadBorder(p, border);

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PPanel::bStyle type;
  switch(up_dn_fill) {
    case 0:
      type = PPanel::UP;
      break;
    case 1:
      type = PPanel::DN;
      break;
    case 2:
      type = PPanel::UP_FILL;
      break;
    case 3:
      type = PPanel::DN_FILL;
      break;
    case 4:
      type = PPanel::BORDER;
      break;
    case 5:
      type = PPanel::BORDER_FILL;
      break;
    default:
      type = PPanel::FILL;
      break;
    }

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  Panel = new PPanel(rect, RGB(R, G, B), type);
  Panel->setAllBorder(border);
  uint id2 = getSecondExtendId();
  p = getOwner()->getPageString(id2);
  if(p) {
    DWORD tick = _ttoi(p);
    if(tick)
      allocBlink(tick);
    }
  allocVisibility(p);

  return true;
}
//----------------------------------------------------------------------------
bool PSimplePanel::update(bool force)
{
  if(baseSimple::update(force)) {
    invalidate();
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void PSimplePanel::performDraw(HDC hdc)
{
  Panel->setRect(get_Rect());
  Panel->paint(hdc);
}
//-----------------------------------------------------------
static LPCTSTR getInt(LPCTSTR p, int& v)
{
  v = _ttoi(p);
  return findNextParamTrim(p);
}
//-----------------------------------------------------------
static LPCTSTR getColor(LPCTSTR p, COLORREF& v)
{
  int r = 255;
  int g = 0;
  int b = 0;
  while(p) {
    p = getInt(p, r);
    if(!p)
      break;
    p = getInt(p, g);
    if(!p)
      break;
    p = getInt(p, b);
    break;
    }
  v = RGB(r, g, b);
  return p;
}
//----------------------------------------------------------------------------
void loadBorder(LPCTSTR p, PPanelBorder& Border)
{
  while(p) {
    int advanced;
    p = getInt(p, advanced);
    for(uint i = 0; i <= PPanelBorder::pbLeft; ++i) {
      if(!p)
        break;
      int style;
      int tickness;
      COLORREF color;
      p = getInt(p, style);
      if(!p)
        break;
      p = getInt(p, tickness);
      if(!p)
        break;
      p = getColor(p, color);
      PPanelBorder::pbInfo info;
      info.style = (PPanelBorder::estyle)style;
      info.tickness = tickness;
      info.color = color;
      Border.setBorder((PPanelBorder::eside)i, info);
      }
    // va messo in fondo perché il settaggio dei lati automaticamente toglie il simple
    // oppure bisognerebbe calcolarlo dai vari parametri (meglio di no, si possono mantenere le impostazioni)
    Border.setSimple(!advanced);
    break;
    }
}
