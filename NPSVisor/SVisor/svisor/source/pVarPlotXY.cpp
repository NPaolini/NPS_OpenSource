//------ PVarPlotXY.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//----------------------------------------------------------------------------
#include "PVarPlotXY.h"
#include "p_basebody.h"
#include "mainclient.h"
#include "p_util.h"
#include "newnormal.h"
//----------------------------------------------------------------------------
#define NO_COLOR(c) (((c) & 0xff000000) == 0xff000000)
#define MAKE_NO_COLOR(c) ((c) | 0xff000000)
#define MAKE_NO_COLORS(r,g,b) (MAKE_NO_COLOR(RGB(abs(r), abs(g), abs(b))))
//----------------------------------------------------------------------------
#define MAX_Y_DEC (IC.maxVal_Y - IC.minVal_Y)
#define MAX_X_DEC (IC.maxVal_X - IC.minVal_X)
//#define MAX_Y 100.0

//#define REM_DEC(a) ((a) * MAX_Y / MAX_Y_DEC)
//#define MUL_DEC(a) ((a) / MAX_Y * MAX_Y_DEC)
#define MUL_DEC(a) (a)
//----------------------------------------------------------------------------
PVarPlotXY::PVarPlotXY(P_BaseBody* owner, uint id) :  baseVar(owner, id),
    Panel(0), currPos(0), Wave_Y(0), Wave_X(0), Buff(0)
{}
//----------------------------------------------------------------------------
PVarPlotXY::~PVarPlotXY()
{
  delete Panel;
  delete []Wave_Y;
  delete []Wave_X;
  delete []Buff;
}
//----------------------------------------------------------------------------
bool PVarPlotXY::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int flag = 0;
  int idColors = 0;
  int idMaxNumData = 0;
  int idEnableRead = 0;

  int idMinY = 0;
  int idMaxY = 0;
  int idMinX = 0;
  int idMaxX = 0;
  int idInitX = 0;

  int idCurr = 0;
  int typeX = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &flag, &idColors, &IC.nRow, &IC.nCol,
                    &idMaxNumData,

                    &idMinY, &idMaxY,
                    &idInitX, &idMinX, &idMaxX,
                    &idCurr,
                    &idEnableRead,
                    &typeX
                    );

  IC.relative_Y = toBool(typeX & 1);
  IC.relative_X = toBool(typeX & 2);

  PPanel::bStyle type;
  switch(flag) {
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
    case 6:
      type = PPanel::FILL;
      break;
    default:
      type = PPanel::NO;
      break;
    }

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);
  makeOtherVar(vInitX, idInitX);
  makeOtherVar(vCurr, idCurr);

  makeOtherVar(vTotData, idMaxNumData);
  makeOtherVar(vEnableRead, idEnableRead);
  makeOtherVar(vMinVal_X, idMinX);
  makeOtherVar(vMaxVal_X, idMaxX);
  makeOtherVar(vMinVal_Y, idMinY);
  makeOtherVar(vMaxVal_Y, idMaxY);

  loadColors(idColors);

  if(PPanel::NO != type)
    Panel = new PPanel(rect, IC.Bkg, type);


  if(!makeStdVars())
    return false;

  return true;
}
//----------------------------------------------------------------------------
static
LPCTSTR loadCol(COLORREF& target, LPCTSTR p)
{
  int r = 0;
  int g = 0;
  int b = 0;
  _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);

  if(r < 0 || g < 0 || b < 0)
    target = MAKE_NO_COLORS(r, g, b);
  else
    target = RGB(r, g, b);
  return findNextParamTrim(p, 3);
}
//----------------------------------------------------------------------------
void PVarPlotXY::loadColors(int idColors)
{
  LPCTSTR p = getOwner()->getPageString(idColors);
  while(p) {
    p = loadCol(IC.Bkg, p);
    if(!p)
      break;
    p = loadCol(IC.grid, p);
    if(!p)
      break;
    p = loadCol(IC.axe, p);
    if(!p)
      break;
    p = loadCol(IC.lineShow1, p);
    if(!p)
      break;
    p = loadCol(IC.lineShow2, p);
    break;
    }
}
//----------------------------------------------------------------------------
bool PVarPlotXY::canCacheReq(const PVect<bool>& sPrph)
{
  int prph = BaseVar.getPrph() - WM_PLC;
  if(prph < 0)
    return true;
  // se la periferica invia il flag che non ha variabili a richiesta, inutile
  // far ricaricare
  if(!sPrph[prph])
    return true;
  return false;
}
//----------------------------------------------------------------------------
void PVarPlotXY::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, vTotData);
  addReqVar2(allBits, vEnableRead);
  addReqVar2(allBits, vMinVal_X);
  addReqVar2(allBits, vMaxVal_X);
  addReqVar2(allBits, vMinVal_Y);
  addReqVar2(allBits, vMaxVal_Y);
  addReqVar2(allBits, vInitX);
  addReqVar2(allBits, vCurr);
  PVect<bool> sPrph;
  mainClient* mc = getMain();
  if(mc && mc->getListPrph(sPrph)) {
    int prph = BaseVar.getPrph() - WM_PLC;
    if(prph < 0 || !sPrph[prph])
      return;
    }
  uint addr = BaseVar.getAddr() + 1;

  prfData data;
  prfData::tResultData result = vTotData.getData(getOwner(), data, getOffs());
  if(data.U.dw > 1000)
    data.U.dw = 1000;
  for(int i = 1; i < (int)data.U.dw; ++i, ++addr)
    addReqVar2(allBits, BaseVar.getPrph(), addr);
}
//----------------------------------------------------------------------------
#define MAX_NUM_POINT IC.totData
#define NUM_BLOCK   (IC.nCol ? IC.nCol : 1)
#define CURR_POS_DIFF(a) (a)
#define CURR_POS(a) ((uint)(a) < MAX_NUM_POINT ? CURR_POS_DIFF(a) : MAX_NUM_POINT - 1)

#define COL_AX      IC.axe
#define COL_GRID    IC.grid
#define COL_LINE1   IC.lineShow1
#define COL_LINE2   IC.lineShow2
#define COL_BKG     IC.Bkg
//----------------------------------------------------------------------------
#define MAX_DATA_TO_SHOW 2000
#define MAX_BLOCK_TO_SHOW 100
//----------------------------------------------------------------------------
#define dPRECISION 0.0000001
//----------------------------------------------------------------------------
#define IS_DIFF(v1, v2) (fabs((v1) - (v2)) > dPRECISION)
//----------------------------------------------------------------------------
#define READ_ALL
//----------------------------------------------------------------------------
bool PVarPlotXY::checkMinMax(bool& force, pVariable& var, double& val)
{
  if(!var.getPrph()) {
    val = 0;
    return true;
    }
  prfData data;
  prfData::tResultData result = var.getData(getOwner(), data, getOffs());
  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(var.getPrph(), data, result);
    return false;
    }
  double v = var.getNormalizedResult(data);
  if(IS_DIFF(val, v))
    force = true;
  val = v;
  return true;
}
//----------------------------------------------------------------------------
bool PVarPlotXY::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible())
    return upd;


  prfData data;
  double v = 0;
  prfData::tResultData result;
  force |= upd;

  if(!checkMinMax(force, vMinVal_Y, IC.minVal_Y))
    return upd;
  if(!checkMinMax(force, vMinVal_X, IC.minVal_X))
    return upd;
  if(!checkMinMax(force, vMaxVal_Y, IC.maxVal_Y))
    return upd;
  if(!checkMinMax(force, vMaxVal_X, IC.maxVal_X))
    return upd;

  if(IC.maxVal_Y - IC.minVal_Y <= 0)
    IC.maxVal_Y = IC.minVal_Y + 1;
  if(IC.maxVal_X - IC.minVal_X <= 0)
    IC.maxVal_X = IC.minVal_X + 1;

  result = vEnableRead.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vTotData.getPrph(), data, result);
    return upd;
    }

  uint bit = vEnableRead.getNorm();
  // non è abilitata
  if(!data.U.dw & (1 << bit)) {
    if(force && !upd)
      invalidate();
    return upd;
    }

  result = vTotData.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vTotData.getPrph(), data, result);
    return upd;
    }

  bool redim = false;
  v = vTotData.getNormalizedResult(data);
  data.U.dw = ROUND_POS_REAL(v);

  if(data.U.dw > MAX_DATA_TO_SHOW)
    data.U.dw = MAX_DATA_TO_SHOW;
  if(data.U.dw != IC.totData) {
    IC.totData = data.U.dw;
    redim = true;
    }
  if(!IC.totData)
    return false;

  result = vCurr.getData(getOwner(), data, getOffs());
  v = vCurr.getNormalizedResult(data);
  data.U.dw = ROUND_POS_REAL(v);

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vCurr.getPrph(), data, result);
    return upd;
    }
  force |= prfData::isChanged == result;
  currPos = data.U.dw;

  if(redim) {
    delete []Wave_Y;
    delete []Wave_X;
    delete []Buff;
    Wave_Y = new double[IC.totData];
    Wave_X = new double[IC.totData];
    Buff = new DWORD[IC.totData];
    readWaveX(true);
    readWaveY(true);
    invalidate();
    return true;
    }

  force |= readWaveY(force);
  force |= readWaveX(force);

  if(force) {
    invalidate();
    return true;
    }
  return upd;
}
//----------------------------------------------------------------------------
template <typename T>
bool convToWave(double* target, T* source, uint ndw, double norm)
{
  bool changed = false;
  int i;
  for(i = ndw - 1; i >= 0; --i) {
    double v = MUL_DEC(getNormalized(source[i], norm));
    if(IS_DIFF(v, target[i])) {
      changed = true;
      break;
      }
    target[i] = v;
    }
  for(; i >= 0; --i)
    target[i] = MUL_DEC(getNormalized(source[i], norm));

  return changed;
}
//----------------------------------------------------------------------------
template <typename T>
void convToWaveNoCheck(double* target, T* source, uint ndw, double norm)
{
  for(int i = ndw - 1; i >= 0; --i)
    target[i] = MUL_DEC(getNormalized(source[i], norm));
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <typename T>
void convToWaveSingle(double* target, T source)
{
  *target = MUL_DEC(source);
}
//----------------------------------------------------------------------------
template <typename T>
void relToAbsolute(T* set, int nElem)
{
  for(int i = 1; i < nElem; ++i)
    set[i] += set[i - 1];
}
//----------------------------------------------------------------------------
bool PVarPlotXY::readWaveY(bool needRefresh)
{
  if(!BaseVar.getPrph()) {
    if(needRefresh)
      fillWave(Wave_Y, IC.minVal_Y, IC.maxVal_Y);
    return needRefresh;
    }
  return readWave(Wave_Y, BaseVar, IC.relative_Y, needRefresh);
}
//----------------------------------------------------------------------------
bool PVarPlotXY::readWaveX(bool needRefresh)
{
  if(!vInitX.getPrph()) {
    if(needRefresh)
      fillWave(Wave_X, IC.minVal_X, IC.maxVal_X);
    return needRefresh;
    }
  return readWave(Wave_X, vInitX, IC.relative_X, needRefresh);
}
//----------------------------------------------------------------------------
void PVarPlotXY::fillWave(double* target, double minV, double maxV)
{
  if(!IC.totData)
    return;
  double step = (maxV - minV) / IC.totData;
  target[0] = minV;
  for(uint i = 1; i < IC.totData; ++i)
    target[i] = target[i - 1] + step;
}
//----------------------------------------------------------------------------
bool PVarPlotXY::readWave(double* target, pVariable& var, bool relative, bool needRefresh)
{

  genericPerif* prph = getOwner()->getGenPerif(var.getPrph());
  if(!prph)
    return false;

  bool isReal = prfData::tFRData == var.getType() || prfData::tRData == var.getType();

  bool gChanged = false;
  int totData = IC.totData;

  prph->getBlockData(Buff, totData, var.getAddr());

  if(relative) {
    if(isReal)
      relToAbsolute((float*)Buff, totData);
    else
      relToAbsolute(Buff, totData);
    }

  double norm = ::getNorm(var.getNorm());

  if(needRefresh) {
    if(isReal)
      convToWaveNoCheck(target, (float*)Buff, IC.totData, norm);
    else
      convToWaveNoCheck(target, Buff, IC.totData, norm);
    }
  else {
    if(isReal)
      gChanged = convToWave(target, (float*)Buff, IC.totData, norm);
    else
      gChanged = convToWave(target, Buff, IC.totData, norm);
    }
  return gChanged;
}
//----------------------------------------------------------------------------
//#define OFFX 5
//#define OFFY 5
//#define OFFY (Wave_X[0])
#define OFFY 0
//----------------------------------------------------------------------------
void PVarPlotXY::performDraw(HDC hdc)
{
  if(Panel) {
    Panel->setRect(get_Rect());
    Panel->paint(hdc);
    }
  if(!IC.totData)
    return;

  Grid(hdc);
  draw_wave(hdc);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PRect PVarPlotXY::getInflated()
{
  PRect r = get_Rect();
  r.Inflate(-4, -4);
  return r;
}
//----------------------------------------------------------------------------
#define MID_WIDTH_ARROW 3
#define HEIGHT_ARROW 8
//----------------------------------------------------------------------------
static void arrowUp(HDC hdc, int x, int y)
{
  MoveToEx(hdc, x - MID_WIDTH_ARROW, y + HEIGHT_ARROW, 0);
  LineTo(hdc, x, y);
  LineTo(hdc, x + MID_WIDTH_ARROW, y + HEIGHT_ARROW);
  LineTo(hdc, x - MID_WIDTH_ARROW, y + HEIGHT_ARROW);
}
//----------------------------------------------------------------------------
/*
static void arrowDn(HDC hdc, int x, int y)
{
  MoveToEx(hdc, x - MID_WIDTH_ARROW, y - HEIGHT_ARROW, 0);
  LineTo(hdc, x, y);
  LineTo(hdc, x + MID_WIDTH_ARROW, y - HEIGHT_ARROW);
  LineTo(hdc, x - MID_WIDTH_ARROW, y - HEIGHT_ARROW);
}
//----------------------------------------------------------------------------
static void arrowLeft(HDC hdc, int x, int y)
{
  MoveToEx(hdc, x + HEIGHT_ARROW, y - MID_WIDTH_ARROW, 0);
  LineTo(hdc, x, y);
  LineTo(hdc, x + HEIGHT_ARROW, y + MID_WIDTH_ARROW);
  LineTo(hdc, x + HEIGHT_ARROW, y - MID_WIDTH_ARROW);
}
*/
//----------------------------------------------------------------------------
static void arrowRight(HDC hdc, int x, int y)
{
  MoveToEx(hdc, x - HEIGHT_ARROW, y - MID_WIDTH_ARROW, 0);
  LineTo(hdc, x, y);
  LineTo(hdc, x - HEIGHT_ARROW, y + MID_WIDTH_ARROW);
  LineTo(hdc, x - HEIGHT_ARROW, y - MID_WIDTH_ARROW);
}
//----------------------------------------------------------------------------
void PVarPlotXY::Grid(HDC hdc)
{
  PRect r = getInflated();
  COLORREF oldColor;
  if(!NO_COLOR(COL_BKG))
    oldColor = SetBkColor(hdc, COL_BKG);

  if(!NO_COLOR(COL_GRID)) {
    HPEN Pen2 = CreatePen(PS_DOT, 1, COL_GRID);
    HGDIOBJ oldPen = SelectObject(hdc, Pen2);

    if(IC.nCol && IC.nCol < IC.totData) {
      int dimCol = IC.totData / IC.nCol;
      PRect r = getInflated();
      double stepx = r.Width();
      stepx /= IC.totData;
      stepx *= dimCol;
      double OffsX = stepx;
//      Wave_X[0] = 0;
      for(int i = 0; i < (int)IC.nCol; ++i, OffsX += stepx) {
        int xx = (int)OffsX + r.left;
        MoveToEx(hdc, xx, r.top, 0);
        LineTo(hdc, xx, r.bottom);
        }
      }
    if(IC.nRow) {
      double stepy = r.Height() / (double)IC.nRow;
      double j = stepy;
      for(int i = 0; i < (int)IC.nRow - 1; ++i, j += stepy) {
        long y = (LONG)j + r.top;
        MoveToEx(hdc, r.left, y, 0);
        LineTo(hdc, r.right, y);
        }
      }
    SelectObject(hdc, oldPen);
    DeleteObject(Pen2);
    }

  if(!NO_COLOR(COL_AX)) {
    HPEN Pen = CreatePen(PS_SOLID, 2, COL_AX);

    HGDIOBJ oldPen = SelectObject(hdc, Pen);

    int y =  r.bottom;
    MoveToEx(hdc, r.left, y, 0);
    LineTo(hdc, r.right, y);

    MoveToEx(hdc, r.left, r.top, 0);
    LineTo(hdc, r.left, r.bottom);

    arrowUp(hdc, r.left, r.top);
    arrowRight(hdc, r.right, y);

    SelectObject(hdc, oldPen);
    DeleteObject(Pen);
    }
  if(!NO_COLOR(COL_BKG))
    SetBkColor(hdc, oldColor);
}
//----------------------------------------------------------------------------
void PVarPlotXY::restore_point(POINT& point, uint ix)
{
  point.y = (LONG)((Wave_Y[ix] - IC.minVal_Y) * IC.ratioY);
  point.y = IC.R.bottom - point.y - (LONG)OFFY;
  if(point.y < IC.R.top)
    point.y = IC.R.top;
  else if(point.y > IC.R.bottom)
    point.y = IC.R.bottom;

  point.x = (LONG)((Wave_X[ix]  - IC.minVal_X) * IC.ratioX) + (LONG)OFFY + IC.R.left;
  if(point.x < IC.R.left)
    point.x = IC.R.left;
  else if(point.x > IC.R.right)
    point.x = IC.R.right;
}
//----------------------------------------------------------------------------
void PVarPlotXY::draw_wave(HDC hdc)
{
  HPEN Pen1 = CreatePen(PS_SOLID, 1, COL_LINE1);
  HPEN Pen2 = CreatePen(PS_SOLID, 1, COL_LINE2);

  HGDIOBJ oldPen = SelectObject(hdc, Pen1);
  do {
    IC.R = getInflated();
    IC.ratioY = (IC.R.Height() - OFFY * 2) / MAX_Y_DEC;
    IC.ratioX = (IC.R.Width() - OFFY * 2) / MAX_X_DEC;
    } while(false);
  POINT p;
  restore_point(p, 0);
  MoveToEx(hdc, p.x, p.y, 0);

  int pos = CURR_POS(currPos);

  int i = 0;
  if(pos) {
    for(i = 1; i <= pos; ++i) {
      restore_point(p, i);
      LineTo(hdc, p.x, p.y);
      }
    SelectObject(hdc, Pen2);
    if(i < (int)MAX_NUM_POINT) {
      restore_point(p, i);
      MoveToEx(hdc, p.x, p.y, 0);
      }
    }
  else
    SelectObject(hdc, Pen2);

  for(++i; i < (int)MAX_NUM_POINT; ++i) {
    restore_point(p, i);
    LineTo(hdc, p.x, p.y);
    }
  SelectObject(hdc, oldPen);
  DeleteObject(Pen1);
  DeleteObject(Pen2);
}
//----------------------------------------------------------------------------
