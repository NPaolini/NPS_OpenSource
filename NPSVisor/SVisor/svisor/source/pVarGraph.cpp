//------ PVarGraph.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarGraph.h"
#include "p_basebody.h"
#include "p_util.h"
#include "pGraphBmp.h"
//----------------------------------------------------------------------------
#define IX_ALTERN_COLOR 10
#define IX_NEXT_COLOR_GRAPH 4
//----------------------------------------------------------------------------
PVarGraph::PVarGraph(P_BaseBody* owner, uint id) :  baseVar(owner, id),
    Graph(0), Truncate(0), oldValMax(0), centerZero(0), GraphDown(0), GraphUp(0),
    oldPerc(101) { }
//----------------------------------------------------------------------------
PVarGraph::~PVarGraph()
{
  delete Graph;
  delete GraphUp;
  delete GraphDown;
}
//----------------------------------------------------------------------------
static uint toggleBit(uint v, uint bit)
{
  if(v & bit)
    return v & ~bit;
  return v | bit;
}
//----------------------------------------------------------------------------
bool PVarGraph::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int no_up_dn = 0;
  int Rfg = 0;
  int Gfg = 255;
  int Bfg = 0;
  int Rbk = 110;
  int Gbk = 110;
  int Bbk = 110;

  int idMax = 0;
  int idVal = 0;
  int idColors = 0;

  int idBmpBar = 0;
  int flagBar = 0;
  int idBmpBkg = 0;
  int flagBkg = 0;
  int idMin = 0;

  // 0 -> standard, da min a max disegnato alle coordinate normali
  // 1 -> disegnato con riferimento allo zero
  // 2 -> disegnato alle coordinate normali con il grafico che parte da zero verso le estremità
  // 3 -> disegnato con riferimento allo zero con il grafico che parte da zero verso le estremità
  // in pratica il bit zero indica se disegnato normalmente o centrato sullo zero
  // il bit uno indica se va dal min al max o se parte dallo zero
  int center_zero = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &no_up_dn,
                    //&Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &idMax, &idVal, &idColors, &Truncate,
                    &idBmpBar, &flagBar, &idBmpBkg, &flagBkg,
                    &idMin, &center_zero
                    );
  centerZero = center_zero;
  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;
  bool horz = true;
  if(no_up_dn >= 1000) {
    horz = false;
    no_up_dn -= 1000;
    // se verticale scambia le dimensioni
    int t = w;
    w = h;
    h = t;
    }

  bool toRight = true;
  if(no_up_dn >= 100) {
    toRight = false;
    no_up_dn -= 100;
    }

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));
  baseRect = rect;

  setRect(rect);

  PPanel::bStyle type = PPanel::NO;
  switch(no_up_dn) {
    case 0:
      type = PPanel::FILL;
      break;
    case 1:
      type = PPanel::BORDER_FILL;
      break;
    case 2:
      type = PPanel::UP_FILL;
      break;
    case 3:
      type = PPanel::DN_FILL;
      break;
    case 4:
      type = PPanel::NO;
      break;
    }

  makeOtherVar(vMax, idMax);
  makeOtherVar(vMin, idMin);
  makeOtherVar(BaseVar, idVal);

  LPCTSTR p1 = getOwner()->getPageString(getFirstExtendId());
  allocVisibility(p1);

  if(idColors)
    Colors.makeColorsOnRow(getOwner(), idColors, IX_NEXT_COLOR_GRAPH);

  PBitmap* bbar = getOwner()->getBmp4Btn(idBmpBar - ID_INIT_BMP_4_BTN);
  PBitmap* bbkg = getOwner()->getBmp4Btn(idBmpBkg - ID_INIT_BMP_4_BTN);

  // se è un bargraph con le barre che partono dallo zero occorre creare quello base
  // che verrà lasciato sempre con perc=0 e altri due, per up e down, che verranno
  // creati senza bordo né sfondo, il rettangolo verrà aggiustato la prima volta che
  // ci si accede
  if(centerZero & 2) {
    PRect r;
    if(!bbar && !bbkg) {
      Graph = new PGraph(rect, RGB(Rbk, Gbk, Bbk), type, RGB(Rfg, Gfg, Bfg), horz, toRight);
      GraphUp = new PGraph(r, 0, PPanel::NO, RGB(Rfg, Gfg, Bfg), horz, toRight);
      GraphDown = new PGraph(r, 0, PPanel::NO, RGB(Rfg, Gfg, Bfg), horz, !toRight);
      }
    else {
      uint mirrorBarFlag = toggleBit(flagBar, PGraphBmp::bmpMirror);
      uint mirrorBkgFlag = toggleBit(flagBkg, PGraphBmp::bmpMirror);
      Graph = new PGraphBmp(rect, RGB(Rbk, Gbk, Bbk), type, RGB(Rfg, Gfg, Bfg), 0,
            (PGraphBmp::bmpstyle)flagBar, bbkg, (PGraphBmp::bmpstyle)flagBkg, horz, toRight);
      GraphUp = new PGraphBmp(r, 0, PPanel::NO, RGB(Rfg, Gfg, Bfg), bbar,
          (PGraphBmp::bmpstyle)flagBar, 0, (PGraphBmp::bmpstyle)flagBkg, horz, toRight);
      GraphDown = new PGraphBmp(r, 0, PPanel::NO, RGB(Rfg, Gfg, Bfg), bbar,
          (PGraphBmp::bmpstyle)mirrorBarFlag, 0, (PGraphBmp::bmpstyle)mirrorBkgFlag, horz, !toRight);
      }
    }
  else {
    if(!bbar && !bbkg)
      Graph = new PGraph(rect, RGB(Rbk, Gbk, Bbk), type, RGB(Rfg, Gfg, Bfg), horz, toRight);
    else
      Graph = new PGraphBmp(rect, RGB(Rbk, Gbk, Bbk), type, RGB(Rfg, Gfg, Bfg), bbar,
            (PGraphBmp::bmpstyle)flagBar, bbkg, (PGraphBmp::bmpstyle)flagBkg, horz, toRight);
    }
  return true;
}
//----------------------------------------------------------------------------
void PVarGraph::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, vMax);
  addReqVar2(allBits, vMin);
}
//----------------------------------------------------------------------------
PRect PVarGraph::getRectMove()
{
  PRect r;
  if(Graph)
    r |= Graph->getRect();
  if(GraphUp)
    r |= GraphUp->getRect();
  if(GraphDown)
    r |= GraphDown->getRect();
  return r;
}
//----------------------------------------------------------------------------
void PVarGraph::performDraw(HDC hdc)
{
  if(Graph)
    Graph->draw(hdc);

  if(GraphUp && GraphDown) {
    if(GraphUp->getPerc())
      GraphUp->draw(hdc);
    else
      GraphDown->draw(hdc);
    }
}
//----------------------------------------------------------------------------
void PVarGraph::performDraw(HDC hdc, const POINT& offset)
{
  if(Graph)
    drawObjOffset(*Graph, hdc, offset);

  if(GraphUp && GraphDown) {
    if(GraphUp->getPerc())
      drawObjOffset(*GraphUp, hdc, offset);
    else
      drawObjOffset(*GraphDown, hdc, offset);
    }
}
//----------------------------------------------------------------------------
#define MOVE_ZERO  (centerZero & 1)
#define CALC_ZERO  (centerZero & 2)
//----------------------------------------------------------------------------
class calc_graph
{
  public:
    calc_graph(double vMax, double vMin, double vCurr) : vMax(vMax), vMin(vMin), vCurr(vCurr) {}
    virtual ~calc_graph() {}
    virtual int getPerc(bool negValue);
    virtual PRect moveRect(const PRect& source, bool horz, bool toRight) { return source; }
    virtual void adjust(int perc, PRect r, const PRect& baseRect, PGraph* Graph, PGraph* GraphUp, PGraph* GraphDown, COLORREF* pfg)
    {
      Graph->setPerc(perc);
      if(pfg)
        Graph->setColor(*pfg);
    }
  protected:
    double vMax;
    double vMin;
    double vCurr;
};
//----------------------------------------------------------------------------
int calc_graph::getPerc(bool negValue)
{
  double valCurr = vCurr - vMin;
  double valMax = vMax - vMin;
  int perc = (int)(valCurr / valMax * 100.0);
  if(negValue)
    perc = -perc;
  if(perc < 0)
    perc = 0;
  else if(perc > 100)
    perc = 100;
  return perc;
}
//----------------------------------------------------------------------------
class calc_graph_only_move : public calc_graph
{
  private:
    typedef calc_graph baseClass;
  public:
    calc_graph_only_move(double vMax, double vMin, double vCurr) : baseClass(vMax, vMin, vCurr) {}
    virtual PRect moveRect(const PRect& source, bool horz, bool toRight);
    virtual void adjust(int perc, PRect r, const PRect& baseRect, PGraph* Graph, PGraph* GraphUp, PGraph* GraphDown, COLORREF* pfg)
    {
      Graph->setRect(r);
      Graph->setPerc(perc);
      if(pfg)
        Graph->setColor(*pfg);
    }
};
//----------------------------------------------------------------------------
PRect calc_graph_only_move::moveRect(const PRect& source, bool horz, bool toRight)
{
  int offs = 0;
  double v = horz ? source.Width() : source.Height();
  double rg = vMax - vMin;
  if(rg > 0) {
    offs = (int)(v / rg * vMin);
    PRect r(source);
    if(horz) {
      if(toRight)
        r.Offset(offs, 0);
      else
        r.Offset(-offs, 0);
      }
    else {
      if(toRight)
        r.Offset(0, -offs);
      else
        r.Offset(0, offs);
      }
    return r;
    }
  return source;
}
//----------------------------------------------------------------------------
class calc_graph_only_fromZero : public calc_graph_only_move
{
  private:
    typedef calc_graph_only_move baseClass;
  public:
    calc_graph_only_fromZero(double vMax, double vMin, double vCurr) : baseClass(vMax, vMin, vCurr) {}
    virtual int getPerc(bool negValue);
    virtual void adjust(int perc, PRect r, const PRect& baseRect, PGraph* Graph, PGraph* GraphUp, PGraph* GraphDown, COLORREF* pfg);
};
//----------------------------------------------------------------------------
int calc_graph_only_fromZero::getPerc(bool negValue)
{
  double v = negValue ? -vCurr : vCurr;
  if(v < 0)
    return -(int)(-v / -vMin * 100.0);
  else
    return (int)(v / vMax * 100.0);
}
//----------------------------------------------------------------------------
PRect operator -(PRect M, const PRect& m)
{
  if(M.left != m.left)
    M.right = m.left;
  if(M.bottom != m.bottom)
    M.top = m.bottom;
  return M;
}
//----------------------------------------------------------------------------
void calc_graph_only_fromZero::adjust(int perc, PRect r, const PRect& baseRect, PGraph* Graph, PGraph* GraphUp, PGraph* GraphDown, COLORREF* pfg)
{
  PRect rUp = r & baseRect;
  PRect rDown = r - rUp;
  POINT pt = { baseRect.left - r.left, baseRect.top - r.top };
  rUp.Offset(pt.x, pt.y);
  GraphUp->resize(rUp);

  rDown.Offset(pt.x, pt.y);

  GraphDown->resize(rDown);
  if(perc >= 0) {
    GraphUp->setPerc(perc);
    GraphDown->setPerc(0);
    if(pfg)
      GraphUp->setColor(*pfg);
    }
  else {
    GraphUp->setPerc(0);
    GraphDown->setPerc(-perc);
    if(pfg)
      GraphDown->setColor(*pfg);
    }
}
//----------------------------------------------------------------------------
class calc_graph_both : public calc_graph
{
  private:
    typedef calc_graph baseClass;
  public:
    calc_graph_both(double vMax, double vMin, double vCurr) : baseClass(vMax, vMin, vCurr) {}
    virtual int getPerc(bool negValue) { return calc_graph_only_fromZero(vMax, vMin, vCurr).getPerc(negValue); }
    virtual PRect moveRect(const PRect& source, bool horz, bool toRight)
         { return calc_graph_only_move(vMax, vMin, vCurr).moveRect(source, horz, toRight); }
    virtual void adjust(int perc, PRect r, const PRect& baseRect, PGraph* Graph, PGraph* GraphUp, PGraph* GraphDown, COLORREF* pfg);
};
//----------------------------------------------------------------------------
void calc_graph_both::adjust(int perc, PRect r, const PRect& baseRect, PGraph* Graph, PGraph* GraphUp, PGraph* GraphDown, COLORREF* pfg)
{
  // prima calcola senza spostamento
  calc_graph_only_fromZero(vMax, vMin, vCurr).adjust(perc, r, baseRect, Graph, GraphUp, GraphDown, pfg);
  int offsX = r.left - baseRect.left;
  int offsY = r.top - baseRect.top;
  // per quello base è quello passato
  Graph->resize(r);
  r = GraphUp->getRect();
  r.Offset(offsX, offsY);
  GraphUp->resize(r);
  r = GraphDown->getRect();
  r.Offset(offsX, offsY);
  GraphDown->resize(r);
}
//----------------------------------------------------------------------------
bool PVarGraph::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible())
    return upd;

  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }

  prfData dataMax;
  prfData::tResultData resultMax = vMax.getData(getOwner(), dataMax, 0);

  if(prfData::failed >= resultMax) {
    getOwner()->ShowErrorData(vMax.getPrph(), dataMax, resultMax);
    return upd;
    }

  REALDATA valMin = 0;
  bool recalcZero = force;
  if(vMin.getPrph()) {
    prfData dataMin;
    prfData::tResultData result = vMin.getData(getOwner(), dataMin, 0);

    if(prfData::failed >= result) {
      getOwner()->ShowErrorData(vMin.getPrph(), dataMin, result);
      return upd;
      }
    if(prfData::notModified != result)
      force = recalcZero = true;
    valMin = vMin.getNormalizedResult(dataMin);
    }
  force |= upd;

  if(!force && prfData::notModified == result && prfData::notModified == resultMax)
    return upd;
  REALDATA valMax = vMax.getNormalizedResult(dataMax);
  if(valMax != oldValMax) {
    oldValMax = valMax;
    recalcZero = true;
    }
  REALDATA valCurr = BaseVar.getNormalizedResult(data);
  calc_graph* objCalc = 0;
  switch(centerZero) {
    case 0:
      objCalc = new calc_graph(valMax, valMin, valCurr);
      break;
    case 1:
      objCalc = new calc_graph_only_move(valMax, valMin, valCurr);
      break;
    case 2:
      objCalc = new calc_graph_only_fromZero(valMax, valMin, valCurr);
      break;
    case 3:
      objCalc = new calc_graph_both(valMax, valMin, valCurr);
      break;
    default:
      return upd;
    }
  int perc = objCalc->getPerc(toBool(getTruncType()));
  if(perc != oldPerc || recalcZero) {
    COLORREF fg;
    int perc2 = abs(perc);
    COLORREF* pfg = 0;
    if(Colors.getReverseColors(fg, fg, perc2))
      pfg = &fg;

    PRect r = objCalc->moveRect(baseRect, Graph->isHorz(), Graph->isToRight());
    objCalc->adjust(perc, r, baseRect, Graph, GraphUp, GraphDown, pfg);
    invalidate();
    if(1 & centerZero) {
      setRect(r);
      invalidate();
      }
    upd = true;
    }
  delete objCalc;
  return upd;
}
//----------------------------------------------------------------------------
