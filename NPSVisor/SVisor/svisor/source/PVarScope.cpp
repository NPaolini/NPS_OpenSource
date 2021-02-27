//------ PVarScope.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//----------------------------------------------------------------------------
#include "PVarScope.h"
#include "p_basebody.h"
#include "mainclient.h"
#include "p_util.h"
#include "newnormal.h"
//----------------------------------------------------------------------------
#define NO_COLOR(c) (((c) & 0xff000000) == 0xff000000)
#define MAKE_NO_COLOR(c) ((c) | 0xff000000)
#define MAKE_NO_COLORS(r,g,b) (MAKE_NO_COLOR(RGB(abs(r), abs(g), abs(b))))
//----------------------------------------------------------------------------
#define MAX_Y_DEC (IC.maxVal - IC.minVal)
#define MAX_X_DEC (stepX[IC.totData - 1])
//#define MAX_Y 100.0

//#define REM_DEC(a) ((a) * MAX_Y / MAX_Y_DEC)
//#define MUL_DEC(a) ((a) / MAX_Y * MAX_Y_DEC)
#define MUL_DEC(a) (a)
//----------------------------------------------------------------------------
PVarScope::PVarScope(P_BaseBody* owner, uint id) :  baseVar(owner, id),
    Panel(0), currPos(0), Wave(0), stepX(0), Buff(0)
{}
//----------------------------------------------------------------------------
PVarScope::~PVarScope()
{
  delete Panel;
  delete []Wave;
  delete []stepX;
  delete []Buff;
}
//----------------------------------------------------------------------------
bool PVarScope::allocObj(LPVOID param)
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
  int idNumBlock = 0;
  int idMaxNumData = 0;
  int idEnableRead = 0;
  int idMinVal = 0;
  int idMaxVal = 0;
  int onlyOne = 0;
  int variableX = 0;
  int idAxesX = 0;
  int addrReadyPos2 = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &flag, &idColors, &IC.nRow,
                    &idNumBlock, &idMaxNumData,
                    &idMinVal, &idMaxVal,
                    &idEnableRead,
                    &IC.addrReadyPos,
                    &onlyOne,
                    &variableX, &idAxesX, &addrReadyPos2
                    );

  IC.onlyOneData = toBool(onlyOne);
  if(!IC.onlyOneData) {
    IC.useVariableX = toBool(variableX & 1);
    IC.relativeBlock = toBool(variableX & 2);
    IC.zeroOffset = !toBool(variableX & 4);
    }
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
  if(IC.useVariableX) {
    makeOtherVar(vAxesX, idAxesX);
    makeOtherVar(vCurrX, addrReadyPos2);
    if(!vAxesX.getPrph())
      IC.useVariableX = false;
    else if(!vCurrX.getPrph())
      IC.useVariableX = false;
    }
  makeOtherVar(vNBlock, idNumBlock);
  makeOtherVar(vTotData, idMaxNumData);
  makeOtherVar(vEnableRead, idEnableRead);
  makeOtherVar(vMinVal, idMinVal);
  makeOtherVar(vMaxVal, idMaxVal);

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
void PVarScope::loadColors(int idColors)
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
bool PVarScope::canCacheReq(const PVect<bool>& sPrph)
{
  int prph = BaseVar.getPrph() - WM_PLC;
  if(prph < 0)
    return true;
  // se la periferica invia il flag che non ha variabili a richiesta, inutile
  // far ricaricare
  if(!sPrph[prph])
    return true;
  return IC.onlyOneData;
}
//----------------------------------------------------------------------------
void PVarScope::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, vTotData);
  addReqVar2(allBits, vNBlock);
  addReqVar2(allBits, vEnableRead);
  addReqVar2(allBits, vMinVal);
  addReqVar2(allBits, vMaxVal);
  addReqVar2(allBits, vAxesX);
  addReqVar2(allBits, vCurrX);
  if(!IC.onlyOneData) {
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
}
//----------------------------------------------------------------------------
#define MAX_NUM_POINT IC.totData
#define NUM_BLOCK   (IC.nBlock ? IC.nBlock : 1)
#define CURR_POS_DIFF(a) (IC.onlyOneData ? ((a) - 1) : (a))
#define CURR_POS(a) ((uint)(a) < MAX_NUM_POINT ? CURR_POS_DIFF(a) : MAX_NUM_POINT - 1)
//#define CURR_POS(a) (IC.onlyOneData ? (a) : ((a) + 1) * (MAX_NUM_POINT / NUM_BLOCK))
//#define CURR_POS(a) (IC.onlyOneData ? (a) : ((a) + 1))
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
bool PVarScope::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible())
    return upd;


  prfData data;
  prfData::tResultData result = vMinVal.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vMinVal.getPrph(), data, result);
    return upd;
    }
  force |= upd;

  double v = vMinVal.getNormalizedResult(data);
  if(IS_DIFF(IC.minVal, v))
    force = true;

  IC.minVal = v;

  result = vMaxVal.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vMaxVal.getPrph(), data, result);
    return upd;
    }

  v = vMaxVal.getNormalizedResult(data);
  if(IS_DIFF(IC.maxVal, v))
    force = true;

  IC.maxVal = v;
  if(IC.maxVal - IC.minVal <= 0)
    IC.maxVal = IC.minVal + 1;

  result = vEnableRead.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vEnableRead.getPrph(), data, result);
    return upd;
    }

  uint bit = vEnableRead.getNorm();
  // non è abilitata
  if(!(data.U.dw & (1 << bit))/*&& !IC.onlyOneData*/) {
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
  // presuppone che il valore sia una dword
  // aggiornato, il valore può essere anche real e/o con normalizzatore. Lo trasformiamo poi in intero
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

  result = vNBlock.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vNBlock.getPrph(), data, result);
    return upd;
    }

  // idem come il totdata
  v = vNBlock.getNormalizedResult(data);
  data.U.dw = ROUND_POS_REAL(v);

  if(data.U.dw > MAX_BLOCK_TO_SHOW)
    data.U.dw = MAX_BLOCK_TO_SHOW;
  else if(!data.U.dw)
    data.U.dw = 1;
  if(data.U.dw != IC.nBlock) {
    IC.nBlock = data.U.dw;
    redim = true;
    }

  if(!IC.onlyOneData) {
    pVariable t = BaseVar;
    t.setAddr(IC.addrReadyPos);
    t.setType(prfData::tDWData);

    result = t.getData(getOwner(), data, getOffs());
    // di default dovrebbe essere dword, diamo la possibilità di usarlo anche come float
    // se è un valore oltre il consentito probabilmente è un float
    if(data.U.dw > IC.totData)
      data.U.dw = ROUND_POS_REAL(data.U.fw);

    if(prfData::failed >= result) {
      getOwner()->ShowErrorData(t.getPrph(), data, result);
      return upd;
      }
    force |= prfData::isChanged == result;
    currPos = data.U.dw;
    }

  if(redim) {
    delete []Wave;
    delete []stepX;
    delete []Buff;
    Wave = new double[IC.totData];
    stepX = new double[IC.totData];
    Buff = new DWORD[IC.totData];
    if(IC.useVariableX)
      readWaveX(true);
    calcGrid();
//    currPos = 0;
    readWave(true);
    invalidate();
    return true;
    }

  // fin qui tutto comune, ora si differenzia per il caso di un solo dato

  if(IC.onlyOneData)
    return readOnlyOne(false);

  if(IC.useVariableX) {
    force |= readWaveX(force);
    result = vCurrX.getData(getOwner(), data, getOffs());

    if(prfData::failed >= result) {
      getOwner()->ShowErrorData(vCurrX.getPrph(), data, result);
      return upd;
      }
    v = vCurrX.getNormalizedResult(data);
    if(IS_DIFF(IC.currVal, v))
      force = true;

    IC.currVal = v;

    }
  // esegue la lettura di tutti i blocchi
  force |= readWave(force);

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
bool PVarScope::readOnlyOne(bool clear)
{
  genericPerif* prph = getOwner()->getGenPerif(BaseVar.getPrph());
  if(!prph)
    return false;

  if(clear) {
    currPos = 0;
    ZeroMemory(Wave, IC.totData * sizeof(*Wave));
    }
  prfData data;
  int offs = 0;
  if(IC.addrReadyPos) {
    pVariable t(1, IC.addrReadyPos, prfData::tDWsData);

    prfData::tResultData result = t.getData(getOwner(), data, getOffs());
    if(prfData::failed < result)
      offs = data.U.sdw;
    }
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs() + offs);
  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(vTotData.getPrph(), data, result);
    return false;
    }

  double v = BaseVar.getNormalizedResult(data);
  convToWaveSingle(Wave + currPos, v);
  ++currPos;
  currPos %= IC.totData;

  invalidate();
  return true;
}
//----------------------------------------------------------------------------
bool PVarScope::readWave(bool needRefresh)
{
  if(IC.onlyOneData)
    return readOnlyOne(true);

  genericPerif* prph = getOwner()->getGenPerif(BaseVar.getPrph());
  if(!prph)
    return false;

  bool isReal = prfData::tFRData == BaseVar.getType() || prfData::tRData == BaseVar.getType();

  bool gChanged = false;
  prph->getBlockData(Buff, IC.totData, BaseVar.getAddr());
  double norm = ::getNorm(BaseVar.getNorm());

  if(needRefresh) {
    if(isReal)
      convToWaveNoCheck(Wave, (float*)Buff, IC.totData, norm);
    else
      convToWaveNoCheck(Wave, Buff, IC.totData, norm);
    }
  else {
    if(isReal)
      gChanged = convToWave(Wave, (float*)Buff, IC.totData, norm);
    else
      gChanged = convToWave(Wave, Buff, IC.totData, norm);
    }
  return gChanged;
}
//----------------------------------------------------------------------------
bool PVarScope::readWaveX(bool needRefresh)
{
  if(IC.onlyOneData)
    return readOnlyOne(true);

  genericPerif* prph = getOwner()->getGenPerif(vAxesX.getPrph());
  if(!prph)
    return false;

  bool isReal = prfData::tFRData == vAxesX.getType() || prfData::tRData == vAxesX.getType();

  bool gChanged = false;
  int totData = IC.totData;
  LPDWORD pBuffX = Buff;
  if(!IC.zeroOffset) {
    --totData;
    Buff[0] = 0;
    ++pBuffX;
    }
  prph->getBlockData(pBuffX, totData, vAxesX.getAddr());

  if(IC.relativeBlock) {
    if(isReal)
      relToAbsolute((float*)pBuffX, totData);
    else
      relToAbsolute(pBuffX, totData);
    }

  double norm = ::getNorm(vAxesX.getNorm());

  if(needRefresh) {
    if(isReal)
      convToWaveNoCheck(stepX, (float*)Buff, IC.totData, norm);
    else
      convToWaveNoCheck(stepX, Buff, IC.totData, norm);
    }
  else {
    if(isReal)
      gChanged = convToWave(stepX, (float*)Buff, IC.totData, norm);
    else
      gChanged = convToWave(stepX, Buff, IC.totData, norm);
    }
  return gChanged;
}
//----------------------------------------------------------------------------
//#define OFFX 5
//#define OFFY 5
#define OFFY (stepX[0])
//----------------------------------------------------------------------------
void PVarScope::calcGrid()
{
  PRect r = getInflated();
  double stepx = r.Width();
  stepx /= IC.totData;
#if 1
  stepX[0] = 0;
#else
  int OffsX = (int)(stepx / 4);
  stepX[0] = OffsX;
#endif
  // viene calcolata durante il disegno
  if(IC.useVariableX)
    return;

  double curr = stepX[0];
  for(int i = 1; i < (int)IC.totData; ++i, curr += stepx)
    stepX[i] = curr;
}
//----------------------------------------------------------------------------
void PVarScope::performDraw(HDC hdc)
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
PRect PVarScope::getInflated()
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
void PVarScope::Grid(HDC hdc)
{
  PRect r = getInflated();
  COLORREF oldColor;
  if(!NO_COLOR(COL_BKG))
    oldColor = SetBkColor(hdc, COL_BKG);

  if(!NO_COLOR(COL_GRID)) {
    HPEN Pen2 = CreatePen(PS_DOT, 1, COL_GRID);
    HGDIOBJ oldPen = SelectObject(hdc, Pen2);

    if(IC.nBlock && IC.nBlock < IC.totData) {
      double dimCol = (double)IC.totData / IC.nBlock;
      PRect r = getInflated();
      double stepx = r.Width();
      stepx /= IC.totData;
      stepx *= dimCol;
      double OffsX = stepx;
      stepX[0] = 0;
      for(int i = 0; i < (int)IC.nBlock; ++i, OffsX += stepx) {
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
void PVarScope::restore_point(POINT& point, uint ix)
{
  PRect r = getInflated();

  double y = (r.Height() - OFFY * 2) / MAX_Y_DEC;

  point.y = (LONG)((Wave[ix] - IC.minVal) * y);
  point.y = r.bottom - point.y - (LONG)OFFY;

  if(point.y < r.top)
    point.y = r.top;
  else if(point.y > r.bottom)
    point.y = r.bottom;

  // il primo punto della X è già stato memorizzato nella grid() e/o nella calcGrid()
  if(IC.useVariableX && ix) {
    double x = (r.Width() - OFFY * 2) / MAX_X_DEC;

    point.x = (LONG)(stepX[ix] * x) + (LONG)OFFY + r.left;

    if(point.x < r.left)
      point.x = r.left;
    else if(point.x > r.right)
      point.x = r.right;
    }
  else
    point.x = (LONG)stepX[ix] + r.left;

}
//----------------------------------------------------------------------------
void PVarScope::drawInterp(HDC hdc, int ix)
{
  double stepx = ix ? stepX[ix] : 0;
  double dx = stepX[ix + 1] - stepx;
  if(dx  > dPRECISION) {
    POINT pt;
    restore_point(pt, ix);
    POINT pt2;
    restore_point(pt2, ix + 1);
    double offsx = IC.currVal - stepx;
    double scalex = offsx / dx;
    pt.x += (long)((pt2.x - pt.x) * scalex);
    pt.y += (long)((pt2.y - pt.y) * scalex);
    LineTo(hdc, pt.x, pt.y);
    }
}
//----------------------------------------------------------------------------
void PVarScope::draw_wave(HDC hdc)
{
  HPEN Pen1 = CreatePen(PS_SOLID, 1, COL_LINE1);
  HPEN Pen2 = CreatePen(PS_SOLID, 1, COL_LINE2);

  HGDIOBJ oldPen = SelectObject(hdc, Pen1);

  POINT p;
  restore_point(p, 0);
  MoveToEx(hdc, p.x, p.y, 0);
  if(IC.useVariableX) {
    int i = 0;
    for(i = 1; i < (int)MAX_NUM_POINT; ++i) {
      if(IC.currVal < stepX[i])
        break;
      restore_point(p, i);
      LineTo(hdc, p.x, p.y);
      }
    if(i < (int)MAX_NUM_POINT) {
      drawInterp(hdc, i - 1);
      SelectObject(hdc, Pen2);
      restore_point(p, i);
      LineTo(hdc, p.x, p.y);
      }

    for(++i; i < (int)MAX_NUM_POINT; ++i) {
      restore_point(p, i);
      LineTo(hdc, p.x, p.y);
      }
    }
  else {
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
    }
  SelectObject(hdc, oldPen);
  DeleteObject(Pen1);
  DeleteObject(Pen2);
}
//----------------------------------------------------------------------------
