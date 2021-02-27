//------ PVarTrend.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
//----------------------------------------------------------------------------
#include "pVarTrend.h"
#include "p_util.h"
#include "newnormal.h"
#include "def_dir.h"
#include "1.h"
#include "Commonlgraph.h"
#include "P_BarProgr.h"
#include "PSliderWin.h"

#include "LimitsDlg.h"
#include "id_msg_common.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_Y_DEC 10000.0
#define MAX_Y_V 100.0
#define STEP_Y IC.nRow
//#define STEP_Y (MAX_Y_V / 5.0)

#define REM_DEC(a) ((a) * MAX_Y_V / MAX_Y_DEC)
#define MUL_DEC(a) ((a) / MAX_Y_V * MAX_Y_DEC)
//----------------------------------------------------------------------------
static freeListPacket FreeListPacket;
//----------------------------------------------------------------------------
freeListPacket& getFreePacket() { return FreeListPacket; }
//----------------------------------------------------------------------------

//#define IDC_STATIC_COORD 11111
//#define ADD_H (IC.readOnly ? 0 : R__Y(16))
//----------------------------------------------------------------------------
PVarTrend::PVarTrend(P_BaseBody* owner, uint id) : baseActive(owner, id),
    baseWinClass(owner, id, PRect(0, 0, 10, 10), 0),// Hand(0), oldy(-1), oldx(-1),
    Brush(0), fLastSeek(0), BuffStat(0), inExec(false), //onShow(false),
    ftFrom(I64_TO_FT(0)), ftTo(I64_TO_FT(0)), insideTime(false), pTrend(0),
    count4CheckTrend(0), hBmpWork(0), mdcWork(0), oldObjWork(0), manCoord(0),
    dontRedraw(0), Left(-1), Right(-1), onZoom(false), step24h(false), noAutoCenter(false),
    autoStep(false), autoRows(false)
{ }
//----------------------------------------------------------------------------
PVarTrend::~PVarTrend()
{
  removeZoom(true);
  removeBmpWork();
  destroy();
  if(Brush)
    DeleteObject(Brush);
  flushPV_toFree(FreeList, ListFile);
  delete []BuffStat;
}
//----------------------------------------------------------------------------
/*
id,x,y,w,h,type_sf,col_sf(rgb),type_grid,col_grid(rgb),num_col,num_row,id_font,
      [V1],[V2],[V3],[V4],trend_name
id+300,valori standard per addr inizio min-max [BaseVar]
id+600,valori standard (compresa visibilità)
id+900,serie di valori formati da S,R,G,B (spessore  e rgb)

type_sf -> 0 = down, 1 = up, 2 = black, 3 = none
type_grid -> 0 = solo tacche esterne, 1 = continua, 2 = lineette, 3 = punti

id,x,y,w,h,2,192,192,192,3,255,0,0,10,5,101,4012,4013,4014,4001,Trend01
id+300,1,165,5
id+900,1,255,255,0,1,0,255,255,255,1,255,0,255,1,255,200,200
*/
//-----------------------------------------------------------
#define AUTO_STEP_BIT (1 << 15)
//-----------------------------------------------------------
#define AUTO_ROWS_BIT (1 << 16)
//-----------------------------------------------------------
static bool extractAutoStep(int& v)
{
  bool set = toBool(v & AUTO_STEP_BIT);
  v &= ~AUTO_STEP_BIT;
  return set;
}
//-----------------------------------------------------------
static bool extractAutoRows(int& v)
{
  bool set = toBool(v & AUTO_ROWS_BIT);
  v &= ~AUTO_ROWS_BIT;
  return set;
}
//----------------------------------------------------------------------------
bool PVarTrend::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int typeBk = 0;
  int Rbk = 0;
  int Gbk = 0;
  int Bbk = 0;

  int typeGrid = 0;
  int Rgr = 220;
  int Ggr = 220;
  int Bgr = 220;

  int Rtxt = 0;
  int Gtxt = 0;
  int Btxt = 0;

  int Rline = 0;
  int Gline = 0;
  int Bline = 0;

  int nCol  = 0;
  int nRow  = 0;
  int idFont = 0;
  int vars[4];

  int nItem = _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,

                    &typeBk,
                    &Rbk, &Gbk, &Bbk,

                    &typeGrid,
                    &Rgr, &Ggr, &Bgr,

                    &Rtxt, &Gtxt, &Btxt,
                    &Rline, &Gline, &Bline,

                    &nCol, &nRow,
                    &idFont,
                    &vars[0], &vars[1], &vars[2], &vars[3]
                    );
  p = findNextParamTrim(p, nItem);
  if(!p)
    return false;

  idFont -= ID_INIT_FONT;

  iTrend.Bkg = RGB(Rbk, Gbk, Bbk);
  Brush = CreateSolidBrush(iTrend.Bkg);

  autoStep = extractAutoStep(typeBk);
  autoRows = extractAutoRows(typeBk);
  if(!autoStep)
    autoRows = false;
  iTrend.BorderType = typeBk;

  iTrend.Grid = RGB(Rgr, Ggr, Bgr);
  iTrend.Text = RGB(Rtxt, Gtxt, Btxt);
  iTrend.LineShow = RGB(Rline, Gline, Bline);
  iTrend.GridType = typeGrid;
  iTrend.nRow = nRow;
  iTrend.nCol = nCol;
  iTrend.hFont = getOwner()->getFont(idFont);
  _tcscpy_s(iTrend.trendName, p);

  if(!makeStdVars())
    return false;

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  Attr.x = rect.left;
  Attr.y = rect.top;
  Attr.w = rect.Width();
  Attr.h = rect.Height();
  setRect(rect);

  do {
    pVariable* varset[] = { &hideShowBit, &activeBit, &variousBit };
    for(uint i = 0; i < SIZE_A(varset); ++i)
      makeOtherVar(*varset[i], vars[i]);
    } while(false);

  do {
    uint idt = vars[SIZE_A(vars) - 1];
    makeOtherVar(dateStart, idt);
    hourStart = dateStart;
    hourStart.setAddr(dateStart.getAddr() + 2);
    dateHourLen = dateStart;
    dateHourLen.setAddr(dateStart.getAddr() + 4);
    } while(false);

  switch(typeBk) {
    case 1:
      // sembra che non venga disegnato
//      Attr.exStyle |= WS_EX_WINDOWEDGE;
      break;
    case 2:
      Attr.exStyle |= WS_EX_STATICEDGE;//WS_EX_CLIENTEDGE;
      break;
    case 3:
      Attr.style |= WS_BORDER;
      break;
    }

  if(!findTrendByName())
    return create();

  allocObj2();
  return create();
}
//----------------------------------------------------------------------------
enum eb_various {
  eb_back,
  eb_forward,
  eb_autocalc_min_max,
  eb_copy_min_max,
  eb_zoom,
  eb_show_val,
  eb_timer,
  eb_export,
  eb_print,
  eb_printerSetup,
  eb_show_range,

  eb_step24h,
  eb_noAutoCenter,

  eb_max,
  };
//----------------------------------------------------------------------------
#define MAX_REPEAT_CHECK 200
//----------------------------------------------------------------------------
bool PVarTrend::allocObj2()
{
  if(count4CheckTrend > MAX_REPEAT_CHECK)
    return false;
  ++count4CheckTrend;

  if(!findTrendByName())
    return false;

  BuffStat = new BYTE[pTrend->getLenRec() + 2];
  iTrend.totVar = getTotVar();

  LPCTSTR p = getOwner()->getPageString(getThirdExtendId());
  if(p) {
    for(uint i = 0; i < iTrend.totVar; ++i)
      p = addColorInfo(p);
    }
  prfData data;
  variousBit.getData(getOwner(), data, getOffs());
  if(step24h ^ toBool(data.U.dw & (1 << eb_step24h)))
    step24h = !step24h;
  if(noAutoCenter ^ toBool(data.U.dw & (1 << eb_noAutoCenter)))
    noAutoCenter = !noAutoCenter;

  checkDate(data, false);
  makeInside();

  return true;
}
//----------------------------------------------------------------------------
DWORD PVarTrend::getTotVar()
{
  uint code = pTrend->getCode();
  LPCTSTR p = getString(code);
  p = findNextParam(p, 1);
  if(!p)
    return 0;

  uint dataCod = _ttoi(p);

  p = getString(dataCod);
  if(!p)
    return 0;
  DWORD totVar = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return totVar;
  uint initData = _ttoi(p);
  for(uint i = 0; i < totVar; ++i) {
    p = getString(initData + i);
    if(!p)
      break;
    uint ndec = 5;
    p = findNextParamTrim(p, 4);
    LPCTSTR p2 = findNextParamTrim(p);
    if(p2) {
      ndec = _ttoi(p);
      p = p2;
      }
    uint idtext = _ttoi(p);
    if(idtext >= 800000) {
      smartPointerConstString sp = getStringByLangGlob(idtext);
      iVarTrend.varName[i] = infoVar(sp, ndec);
      }
    else {
      smartPointerConstString sp(p, false);
      iVarTrend.varName[i] = infoVar(sp, ndec);
      }
    }
  return totVar;
}
//----------------------------------------------------------------------------
bool PVarTrend::findTrendByName()
{
  LPCTSTR p = getString(INIT_TREND_DATA);
  while(p) {
    uint code = _ttol(p);
    if(!code)
      break;
    LPCTSTR p2 = getString(code);
    p2 = findNextParamTrim(p2, 6);
    if(p2 && !_tcsicmp(p2, iTrend.trendName)) {
      mainClient* wMain = getMain();
      if(!wMain)
        return false;
      ImplJobData* job = wMain->getIData();
      pTrend = job->getSaveTrendByCode(code);
      return toBool(pTrend);
      }
    p = findNextParamTrim(p);
    }
  return false;
}
//----------------------------------------------------------------------------
static SIZE get_Rect(HWND hwnd, HFONT hfont, LPCTSTR str)
{
  HDC hDC = GetDC(hwnd);
  HGDIOBJ hfOld = 0;
  if(hfont)
    hfOld = SelectObject(hDC, hfont);
  SIZE size;
  int len =_tcslen(str);
  GetTextExtentPoint32(hDC, str, len, &size);
  if(hfont)
    SelectObject(hDC, hfOld);
  ReleaseDC(hwnd, hDC);
  size.cx += len;
  return size;
}
//----------------------------------------------------------------------------
#define TEST_STR _T("123456.12345")
//----------------------------------------------------------------------------
bool PVarTrend::create()
{
  if(!baseWinClass::create())
    return false;

  iTrend.defStr = ::get_Rect(*this, iTrend.hFont, TEST_STR);

  GetClientRect(*this, iTrend.Inside);
  iTrend.Inside.left += iTrend.defStr.cx;
  iTrend.Inside.bottom -= iTrend.defStr.cy * 3;

  Left = iTrend.Inside.Width() / 3;
  Right = Left * 2;

  if(pTrend)
    reload(true);
  return true;
}
//----------------------------------------------------------------------------
void PVarTrend::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, hideShowBit);
  addReqVar2(allBits, activeBit);
  addReqVar2(allBits, dateStart);
  addReqVar2(allBits, hourStart);
  addReqVar2(allBits, dateHourLen);
  addReqVar2(allBits, variousBit);
}
//----------------------------------------------------------------------------
void PVarTrend::setVisibility(bool set)
{
  if(set)
    Owner->addToClipped(getRect());
  else
    Owner->removeFromClipped(getRect());
  baseActive::setVisibility(set);
}
//----------------------------------------------------------------------------
bool PVarTrend::isInsideDate(LPCTSTR file, const FILETIME& ftStart, const FILETIME& ftEnd)
{
  P_File pf(file, P_READ_ONLY);
  if(!pf.P_open())
    return false;
  FILETIME ft;
  pf.P_read(&ft, sizeof(ft));
  if(ftEnd < ft)
    return false;
  long sizeStat = (long)pTrend->getLenRec();
  pf.P_seek(-sizeStat, SEEK_END_);
  pf.P_read(&ft, sizeof(ft));
  if(ftStart > ft)
    return false;
  return true;
}
//--------------------------------------------------------------------
void PVarTrend::insertInListFile(LPCTSTR filename)
{
  listFile* lf = FreeList.get();
  _tcscpy_s(lf->name, _MAX_PATH, filename);
  int nElem = ListFile.getElem();
  if(!nElem)
    ListFile[0] = lf;
  else {
    int i = 0;
    for(; i < nElem; ++i) {
      if(_tcsicmp(lf->name, ListFile[i]->name) <= 0)
        break;
      }
    ListFile.insert(lf, i);
    }
}
//----------------------------------------------------------------------------
static void removeFilename(LPTSTR file)
{
  int len = _tcslen(file);
  for(int i = len -1; i > 0; --i) {
    if(_T('\\') == file[i]) {
      file[i] = 0;
      break;
      }
    }
}
//----------------------------------------------------------------------------
void PVarTrend::checkInside()
{
  if(noAutoCenter)
    insideTime = false;
  else {
    FILETIME curr = getFileTimeCurr();
    insideTime = ftFrom < curr && curr < ftTo;
    }
}
//--------------------------------------------------------------------
DWORD PVarTrend::calcInitIx(DWORD& ixInit)
{
  DWORD nTotRec = 0;
  ixInit = 0;
  int nElem = ListFile.getElem();
  long sizeStat = (long)pTrend->getLenRec();
  for(int i = 0; i < nElem; ++i) {
    P_File f(ListFile[i]->name, P_READ_ONLY);
    if(!f.P_open())
      continue;
    DWORD nRec = (DWORD)f.get_len();
    nRec /= sizeStat;
    if(i == nElem - 1) {
      DWORD ixEnd = findFirstPos(f, ftTo, sizeStat, BuffStat, 0, false);
      nTotRec += ixEnd;
      }
    else
      nTotRec += nRec;
    if(!i) {
      ixInit = findFirstPos(f, ftFrom, sizeStat, BuffStat, 0, true);
      nTotRec -= ixInit;
      }
    }
  return nTotRec;
}
//----------------------------------------------------------------------------
#define DEF_MAX_REC2SHOW 5000
//-------------------------------------------------------------------
#define MIN_REC_2_SHOW  500
#define MAX_REC_2_SHOW  100000
//----------------------------------------------------------------------------
bool PVarTrend::reloadAll()
{
  if(inExec)
    return false;
  inExec = true;
  TCHAR path[_MAX_PATH];
  bool useHistory;
  if(!pTrend->makeFindableName(path, useHistory)) {
    inExec = false;
    return false;
    }
  flushPV_toFree(FreeList, ListFile);
  iVarTrend.resetXY();
  iVarTrend.resetRange();
  TCHAR file[_MAX_PATH];
  for(uint i = 0; i < 2; ++i) {
    makePath(file, path, dExRep, toBool(i), _T(""));
    WIN32_FIND_DATA FindFileData;
    HANDLE h = FindFirstFile(file, &FindFileData);
    if(h == INVALID_HANDLE_VALUE)
      continue;
    removeFilename(file);
    do {
      TCHAR t[_MAX_PATH];
      _tcscpy_s(t, file);
      appendPath(t, FindFileData.cFileName);
      if(isInsideDate(t, ftFrom, ftTo))
        insertInListFile(t);
      } while(FindNextFile(h, &FindFileData));
    FindClose(h);
    }
  if(!ListFile.getElem()) {
    inExec = false;
    return false;
    }
  bool result = makeVect();
  inExec = false;
  return result;
}
//--------------------------------------------------------------------
bool PVarTrend::makeVect()
{
  DWORD ixInit;
  DWORD nTotRec = calcInitIx(ixInit);
  DWORD maxRecord2Show = DEF_MAX_REC2SHOW;

  LPCTSTR p = ::getString(ID_GRAPH_MAX_REC_SHOW);
  if(p) {
    maxRecord2Show = _ttoi(p);
    if(maxRecord2Show > 0) {
      if(maxRecord2Show < MIN_REC_2_SHOW)
        maxRecord2Show = MIN_REC_2_SHOW;
      else if(maxRecord2Show > MAX_REC_2_SHOW)
        maxRecord2Show = MAX_REC_2_SHOW;
      }
    else
      maxRecord2Show = 0;
    }

  if(maxRecord2Show && nTotRec > (DWORD)maxRecord2Show)
    return makeVectStep(nTotRec, ixInit, maxRecord2Show);
  else
    return makeVectFull(nTotRec, ixInit);
}
//----------------------------------------------------------------------------
#define CAST_I64(a) MK_I64(*(FILETIME*)(a))
//----------------------------------------------------------------------------
//#define MIN_REC_4_SHOW_BAR 1000
#define MIN_REC_4_SHOW_BAR 10000
//----------------------------------------------------------------------------
bool PVarTrend::makeVectStep(DWORD nTotRec, DWORD ixInit, DWORD maxRecord2Show)
{
  int nElem = ListFile.getElem();

  HWND hwFocus = GetFocus();
  {
  BarProgr bar(this, nTotRec);
  if(nTotRec > MIN_REC_4_SHOW_BAR)
    bar.create();

  __int64 ftEnd = MK_I64(ftTo);
  if(!ftEnd)
    ftEnd = MK_I64(getFileTimeCurr());

  bool stopped = false;
  double step = nTotRec;
  step /= maxRecord2Show;
  long sizeStat = (long)pTrend->getLenRec();

  int nField = getNumField();

  double ixGlobal = 0;
  for(int i = 0; i < nElem && !stopped; ++i) {
    P_File f(ListFile[i]->name, P_READ_ONLY);
    if(!f.P_open())
      continue;

    memset(BuffStat, 0, sizeStat);

    DWORD nRec = (DWORD)f.get_len();
    nRec /= sizeStat;

    double ix = 0;
    if(!i)
      ix = ixInit;

    bool foundGreat = false;
    for(;; ixGlobal += step, ix += step) {
      if(ROUND_POS_REAL(ix) >= (long)nRec)
        break;
      f.P_seek(ROUND_POS_REAL(ix) * sizeStat);
      if(f.P_read(BuffStat, sizeStat) != sizeStat)
        break;

      if(!(ROUND_POS_REAL(ixGlobal) & 0xf))
        if(!bar.setPos((DWORD)ixGlobal)) {
          stopped = true;
          break;
          }

      if(CAST_I64(BuffStat) > ftEnd) {
        if(foundGreat)
          break;
        foundGreat = true;
        }
      addToVect(BuffStat, nField);
      }
    fLastSeek = f.get_len();
    }
  recalcNewRange(nField);
  }
  SetFocus(hwFocus);
  return true;
}
//----------------------------------------------------------------------------
void PVarTrend::recalcNewRange(int nField)
{
  setCurrRange(nField);
  for(int i = 0; i < nField; ++i)
    iVarTrend.recalcPix(i, iTrend.Inside);

  iVarTrend.recalcPix(MK_I64(ftFrom), MK_I64(ftTo) - MK_I64(ftFrom), iTrend.Inside);
}
//----------------------------------------------------------------------------
void PVarTrend::setCurrRange(int nField)
{
  if(!BaseVar.getPrph()) {
    LPCTSTR file = getFileMinMax();
    if(!file)
      return;
    TCHAR tmp[_MAX_PATH];
    makePath(tmp, file, dExRep, false, _T(""));
    rangeSet rSet;

    P_File f(tmp, P_READ_ONLY);
    if(f.P_open())
      f.P_read(&rSet, sizeof(rSet));
    else
      memset(&rSet, 0, sizeof(rSet));

    for(int i = 0; i < nField; ++i) {
      infoRangeTrend ir;
      if(rSet.rMax[i] - rSet.rMin[i] < 0.0000001)
        iVarTrend.getAutoRange(i, ir);
      else {
        ir.vMin = rSet.rMin[i];
        ir.vMax = rSet.rMax[i];
        }
      iVarTrend.setRange(i, ir);
      }
    return;
    }

  prfData data;
  BaseVar.fillAttr(data, getOffs());
  genericPerif* prph = getOwner()->getGenPerif(BaseVar.getPrph());
  if(!prph)
    return;
  int currNorm = BaseVar.getNorm();
  bool hasNorm = toBool(currNorm);
  for(int i = 0; i < nField; ++i, ++currNorm) {
    infoRangeTrend ir;
    prph->get(data);
    REALDATA nrm = hasNorm ? ::getNorm(currNorm) : 1.0;
    ir.vMin = (float)getNormalized(data.U.fw, nrm);
    ++data.lAddr;
    prph->get(data);
    ir.vMax = (float)getNormalized(data.U.fw, nrm);
    ++data.lAddr;
    if(ir.vMax > ir.vMin)
      iVarTrend.setRange(i, ir);
    }
}
//----------------------------------------------------------------------------
void PVarTrend::recalcCurrRange()
{
  int nField = getNumField();
  for(int i = 0; i < nField; ++i) {
    infoRangeTrend range;
    iVarTrend.getAutoRange(i, range);
    iVarTrend.setRange(i, range);
    }
//  saveCurrRange();
}
//----------------------------------------------------------------------------
void PVarTrend::saveCurrRange()
{
  int nField = getNumField();

  if(!BaseVar.getPrph()) {
    LPCTSTR file = getFileMinMax();
    if(!file)
      return;
    TCHAR tmp[_MAX_PATH];
    makePath(tmp, file, dExRep, false, _T(""));
    rangeSet rSet;
    do {
      P_File f(tmp, P_READ_ONLY);
      if(f.P_open())
        f.P_read(&rSet, sizeof(rSet));
      else
        memset(&rSet, 0, sizeof(rSet));
      f.appendback();
      } while(false);

    P_File f(tmp, P_CREAT);
    if(!f.P_open())
      return;

    for(int i = 0; i < nField; ++i) {
      infoRangeTrend ir;
      iVarTrend.getRange(i, ir);
      rSet.rMin[i] = ir.vMin;
      rSet.rMax[i] = ir.vMax;
      }
    f.P_write(&rSet, sizeof(rSet));
    return;
    }

  prfData data;
  BaseVar.fillAttr(data, getOffs());
  genericPerif* prph = getOwner()->getGenPerif(BaseVar.getPrph());
  if(!prph)
    return;
  int currNorm = BaseVar.getNorm();
  bool hasNorm = toBool(currNorm);
  for(int i = 0; i < nField; ++i, ++currNorm) {
    infoRangeTrend ir;
    if(!iVarTrend.getRange(i, ir))
      continue;
    REALDATA nrm = hasNorm ? ::getNorm(currNorm) : 0;
    getDenormalizedReal(data.U.fw, ir.vMin, nrm);
    prph->set(data, true);
    ++data.lAddr;
    getDenormalizedReal(data.U.fw, ir.vMax, nrm);
    prph->set(data, true);
    ++data.lAddr;
    }
}
//----------------------------------------------------------------------------
bool PVarTrend::appendToVect(LPBYTE buff, int nField)
{
  iVarTrend.append(CAST_I64(buff), MK_I64(ftFrom), MK_I64(ftTo) - MK_I64(ftFrom), iTrend.Inside);
  buff += sizeof(FILETIME);
  for(int i = 0; i < nField; ++i, buff += sizeof(float))
    iVarTrend.append(i, *(float*)buff, iTrend.Inside);
  return true;
}
//----------------------------------------------------------------------------
bool PVarTrend::addToVect(LPBYTE buff, int nField)
{
  iVarTrend.simpleAppend(CAST_I64(buff));
  buff += sizeof(FILETIME);
  for(int i = 0; i < nField; ++i, buff += sizeof(float))
    iVarTrend.simpleAppend(i, *(float*)buff);
  return true;
}
//----------------------------------------------------------------------------
bool PVarTrend::makeVectFull(DWORD nTotRec, DWORD ixInit)
{
  int nElem = ListFile.getElem();

  HWND hwFocus = GetFocus();
  {
  DWORD ixGlobal = 0;
  BarProgr bar(this, nTotRec);
  if(nTotRec > MIN_REC_4_SHOW_BAR)
    bar.create();

  __int64 ftEnd = MK_I64(ftTo);
  if(!ftEnd)
    ftEnd = MK_I64(getFileTimeCurr());

  long sizeStat = (long)pTrend->getLenRec();
  int nField = getNumField();

  bool stopped = false;
  for(int i = 0; i < nElem && !stopped; ++i) {
    P_File f(ListFile[i]->name, P_READ_ONLY);
    if(!f.P_open())
      continue;

    memset(BuffStat, 0, sizeStat);

    DWORD nRec = (DWORD)f.get_len();
    nRec /= sizeStat;

    DWORD ix = 0;
    if(!i)
      ix = ixInit;
    f.P_seek(ix * sizeStat);
    f.P_read(BuffStat, sizeStat);

    bool foundGreat = false;
    for(;;++ixGlobal) {
      if(!(ixGlobal & 0xf))
        if(!bar.setPos(ixGlobal)) {
          stopped = true;
          break;
          }

      if(CAST_I64(BuffStat) > ftEnd) {
        if(foundGreat)
          break;
        foundGreat = true;
        }
      addToVect(BuffStat, nField);
      if(f.P_read(BuffStat, sizeStat) != sizeStat)
        break;
      }
    fLastSeek = f.get_len();
    }
  recalcNewRange(nField);
  }
  SetFocus(hwFocus);
  return true;
}
//----------------------------------------------------------------------------
bool PVarTrend::reload(bool force)
{
  if(force)
    return reloadAll();
  if(!insideTime)
    return false;
  mainClient* wMain = getMain();
  if(!wMain)
    return false;

  TCHAR path[_MAX_PATH];
  if(!pTrend->makeCurrentName(wMain, path))
    return false;

  makePath(path, dExRep, false, _T(""));

  P_File pf(path, P_READ_ONLY);
  if(!pf.P_open())
    return false;

  if((UDimF)fLastSeek >= pf.get_len())
    return false;
  long sizeStat = (long)pTrend->getLenRec();
  int nField = getNumField();

  pf.P_seek(fLastSeek);
  for(;;) {
    if(pf.P_read(BuffStat, sizeStat) != sizeStat)
      break;
    appendToVect(BuffStat, nField);
    }
  fLastSeek = pf.get_len();
  return true;
}
//----------------------------------------------------------------------------
#define ONE_DAY (HOUR_TO_I64 * 24)
//----------------------------------------------------------------------------
static void clearTime(FILETIME& ft)
{
#if 1
  MK_I64(ft) /= ONE_DAY;
  MK_I64(ft) *= ONE_DAY;
#else
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  st.wHour = 0;
  st.wMinute = 0;
  st.wSecond = 0;
  st.wHundred = 0;
  SystemTimeToFileTime(&st, &ft);
#endif
}
//----------------------------------------------------------------------------
static void clearDate(FILETIME& ft)
{
  MK_I64(ft) %= ONE_DAY;
}
//----------------------------------------------------------------------------
void PVarTrend::saveTime()
{
  prfData data;
  dateStart.fillAttr(data, getOffs());
  genericPerif* prph = getOwner()->getGenPerif(dateStart.getPrph());
  if(!prph)
    return;

  data.U.ft = ftFrom;
  clearTime(data.U.ft);
  prfData data2;
  dateStart.getData(getOwner(), data2, getOffs());
  if(data2.U.ft != data.U.ft)
    prph->set(data, true);

  hourStart.fillAttr(data, getOffs());
  data.U.ft = ftFrom;
  clearDate(data.U.ft);
  hourStart.getData(getOwner(), data2, getOffs());
  if(data2.U.ft != data.U.ft)
    prph->set(data, true);

  dateHourLen.fillAttr(data, getOffs());
  data.U.ft = ftTo - ftFrom;
  dateHourLen.getData(getOwner(), data2, getOffs());
  if(data2.U.ft != data.U.ft)
    prph->set(data, true);
}
//----------------------------------------------------------------------------
void PVarTrend::makeInside()
{
  if(dontRedraw || noAutoCenter)
    return;
  FILETIME delta = ftTo - ftFrom;
  if(MK_I64(delta) < SECOND_TO_I64 || MK_I64(delta) > HOUR_TO_I64 * 24 * 3)
    MK_I64(delta) = MINUTE_TO_I64 * 10;
  ftFrom = getFileTimeCurr() - I64_TO_FT(MK_I64(delta) / 2);
  ftTo = ftFrom + delta;
  insideTime = true;
  saveTime();
}
//----------------------------------------------------------------------------
bool PVarTrend::changedRange()
{
  if(!BaseVar.getPrph())
    return false;
  prfData data;
  pVariable tV(BaseVar);
  uint addr = tV.getAddr();
  int nField = getNumField() * 2;
  for(int i = 0; i < nField; ++i) {
    if(prfData::isChanged == tV.getData(getOwner(), data, getOffs()))
      return true;
    tV.setAddr(++addr);
    }
  return false;
}
//----------------------------------------------------------------------------
void PVarTrend::setVar(pVariable& var, prfData& data, bool noCommit)
{
  genericPerif* prph = getOwner()->getGenPerif(var.getPrph());
  if(prph)
    prph->set(data, noCommit);
}
//----------------------------------------------------------------------------
int PVarTrend::checkDate(prfData& data, bool check)
{
  dateStart.getData(getOwner(), data, getOffs());

  if(!MK_I64(data.U.ft))
    return -1;
  FILETIME ft = data.U.ft;
  hourStart.getData(getOwner(), data, getOffs());
  ft += data.U.ft;
  int chgDate = MK_I64(ftFrom) != MK_I64(ft) ? 1 : 0;
  ftFrom = ft;
  dateHourLen.getData(getOwner(), data, getOffs());
  if(!MK_I64(data.U.ft))
    return -1;
  ft += data.U.ft;
  chgDate |= MK_I64(ftTo) != MK_I64(ft);
  ftTo = ft;

  if(check) {
    if(!chgDate) {
      bool old = insideTime;
      checkInside();
      if(old != insideTime) {
//        if(!insideTime)
          makeInside();
        chgDate = 2;
        }
      }
    else
      checkInside();
    }
  return chgDate;
}
//----------------------------------------------------------------------------
void PVarTrend::moveTime(int where)
{
  FILETIME ft = ftTo - ftFrom;
  if(step24h)
    ft = I64_TO_FT(ONE_DAY);
  if(where < 0) {
    ftTo -= ft;
    ftFrom -= ft;
    }
  else {
    ftTo += ft;
    ftFrom += ft;
    }
  insideTime = false;
  saveTime();
}
//----------------------------------------------------------------------------
#define PRESERVE_BITS ((1 << eb_zoom) | (1 << eb_noAutoCenter) | (1 << eb_step24h))
//----------------------------------------------------------------------------
bool PVarTrend::checkChangedVariousBit(bool force)
{
  prfData data;
  bool chg = prfData::isChanged == variousBit.getData(getOwner(), data, getOffs());

  if(step24h ^ toBool(data.U.dw & (1 << eb_step24h))) {
//    force = true;
    step24h = !step24h;
    }
  if(noAutoCenter ^ toBool(data.U.dw & (1 << eb_noAutoCenter))) {
//    force = true;
    noAutoCenter = !noAutoCenter;
    }

  if(!chg)
    return force;

  DWDATA t = data.U.dw;
  for(uint i = 0; i < eb_max && t; ++i, t >>= 1) {
    if(t & 1) {
      switch(i) {
        case eb_back:
          moveTime(-1);
          force = true;
          break;
        case eb_forward:
          moveTime(1);
          force = true;
          break;
        case eb_autocalc_min_max:
          recalcCurrRange();
          force = true;
          break;
        case eb_copy_min_max:
          saveCurrRange();
          force = true;
          break;
        case eb_zoom:
          if(!onZoom) {
            onZoom = true;
            manCoord = new manage_coord(iTrend.Inside);
            }
          break;
        case eb_show_val:
          data.U.dw &= ~(1 << eb_show_val);
          setVar(variousBit, data, true);
          runShowValue();
          break;
        case eb_timer:
          data.U.dw &= ~(1 << eb_timer);
          setVar(variousBit, data, true);
          setTimeForTimer();
          break;
        case eb_export:
          data.U.dw &= ~(1 << eb_export);
          setVar(variousBit, data, true);
          exportTrend();
          break;
        case eb_print:
          data.U.dw &= ~(1 << eb_print);
          setVar(variousBit, data, true);
          print_Graph();
          break;
        case eb_printerSetup:
          data.U.dw &= ~(1 << eb_printerSetup);
          setVar(variousBit, data, true);
          printerSetup();
          break;
        case eb_show_range:
          data.U.dw &= ~(1 << eb_show_range);
          setVar(variousBit, data, true);
          if(showRange())
            force = true;
          break;
        }
      }
    }
  if(onZoom && !(data.U.dw & (1 << eb_zoom)))
    removeZoom();
  if(data.U.dw & ~PRESERVE_BITS) {
    data.U.dw &= PRESERVE_BITS;
    setVar(variousBit, data, true);
    }
  return force;
}
//--------------------------------------------------------------------
LPCTSTR PVarTrend::getFileMinMax()
{
  return getOwner()->getPageString(BaseVar.getAddr());
}
//--------------------------------------------------------------------
bool PVarTrend::showRange()
{
  if(BaseVar.getPrph() || !pTrend)
    return false;
  LPCTSTR file = getFileMinMax();
  if(!file)
    return false;
  TCHAR tmp[_MAX_PATH];
  makePath(tmp, file, dExRep, false, _T(""));

  setOfString s;
  s.replaceString(ID_TREND_NAME, str_newdup(iTrend.trendName), true);
  PD_Limits(tmp, s, this).modal();
  recalcNewRange(getNumField());
  return true;
}
//--------------------------------------------------------------------
void PVarTrend::setTimeForTimer()
{
  if(!pTrend)
    return;
  uint currTimer = pTrend->getTime();
  smartPointerConstString tmsg = getStringOrIdByLangGlob(ID_TIMER_SET_TIME_G);
  smartPointerConstString ttit = getStringOrIdByLangGlob(ID_TITLE_SET_TIME_G);
  mainClient* Par = getParentWin<mainClient>(this);
  if(IDOK == TD_Timer(Par, &ttit, &tmsg, currTimer).modal()) {
    LPCTSTR p = getString(pTrend->getCode());
    if(p) {
      uint addrTimer = _ttoi(p);
      genericPerif *prf = Par->getGenPerif(PRF_MEMORY);
      prfData data;
      data.typeVar = prfData::tDWData;
      data.lAddr = addrTimer;
      data.U.dw = currTimer;
      prf->set(data);
      gestJobData* job = Par->getData();
      job->checkTimerTrend();
      }
    }
}
//----------------------------------------------------------------------------
//#define INVALIDATE InvalidateRect(*this, 0, 1)
#define INVALIDATE { InvalidateRect(*this, 0, 0); }
//----------------------------------------------------------------------------
#define SIZE_BIT(a) (sizeof(a) * 8)
//----------------------------------------------------------------------------
bool PVarTrend::update(bool force)
{
  bool upd = baseActive::update(force);
  if(!isVisible())
    return upd;
  if(!pTrend) {
    if(!allocObj2())
      return upd;
    }
  if(checkChangedVariousBit(force)) {
    reload(true);
#if 1
    // facciamogli controllare la visibilità
    force = true;
    INVALIDATE
#else
    return true;
#endif
    }

  prfData data;
  int chgDate = checkDate(data);
  if(-1 == chgDate && !force)
    return upd;

  bool chgActive = prfData::isChanged == activeBit.getData(getOwner(), data, getOffs());
  DWDATA dwActive = data.U.dw;
  if(prfData::isChanged == hideShowBit.getData(getOwner(), data, getOffs()) || force) {
    DWDATA& d = data.U.dw;
    uint nField = getNumField();
    PVect<infoY>& iY = iVarTrend.Y_Set;
    bool refr = false;
    for(uint i = 0; i < nField; ++i) {
      bool chg = toBool(d & (1 << i)) ^ iY[i].isVisible();
      if(chg) {
        if(iY[i].isVisible() && dwActive &  (1 << i)) {
          d |= 1 << i;
          setVar(hideShowBit, data, true);
          continue;
          }
        iY[i].toggleVisible();
        refr = true;
        }
      }
    if(refr && !force)
      INVALIDATE
    }
/*
  ricalcola left e se diverso dal precedente effettua il cambio delle dimensioni dell'area del grafico
  se si effettua il cambio
    occorre ricalcolare i dati
*/
  uint totScale = 0;
  DWDATA dw = dwActive;
  for(uint ix = 0; ix < SIZE_BIT(DWORD) && ix < (uint)getNumField(); ++ix, dw >>= 1)
    if(dw & 1)
      ++totScale;
  PRect r;
  GetClientRect(*this, r);
  r.left += iTrend.defStr.cx * totScale;
  if(r.left != iTrend.Inside.left) {
    iTrend.Inside.left = r.left;
    iTrend.gridPosX.reset();
    force = true;
    }
/*
  if(checkChangedVariousBit(force)) {
    reload(true);
    INVALIDATE
    return true;
    }
*/
  force |= toBool(chgDate);

  if(!force) {
    if(changedRange()) {
    // se ricarichiamo tutto è inutile fargli ricalcolare il range
//      recalcNewRange(getNumField());
      force = true;
      }
    }

  if(!force) {
    if(chgActive) {
      DWDATA& d = dwActive;
      uint nField = getNumField();
      PVect<infoY>& iY = iVarTrend.Y_Set;
      for(uint i = 0; i < nField; ++i) {
        if(d & (1 << i)) {
          if(!iY[i].isVisible()) {
            for(uint j = 0; j < nField; ++j) {
              if(iY[j].isVisible()) {
                activeBit.fillAttr(data, getOffs());
                data.U.dw = 1 << j;
                setVar(activeBit, data, true);
                break;
                }
              }
            }
          break;
          }
        }
      }
    if(chgActive)
      INVALIDATE
    }
  if(reload(force)) {
    INVALIDATE
    return true;
    }
  return upd | force;
}
//----------------------------------------------------------------------------
void PVarTrend::removeBmpWork()
{
  if(hBmpWork) {
    SelectObject(mdcWork, oldObjWork);
    DeleteDC(mdcWork);
    DeleteObject(hBmpWork);
    hBmpWork = 0;
    }
}
//---------------------------------------------------------------------
bool PVarTrend::allocBmpWork(HDC hdc)
{
  if(hBmpWork)
    return true;

  PRect r;
  GetClientRect(*this, r);
  if(r.Width() && r.Height() && getHandle()) {
    HDC dc = hdc;
    if(!dc)
      dc = GetDC(*this);
    hBmpWork = CreateCompatibleBitmap(dc, r.Width(), r.Height());
    if(hBmpWork) {
      mdcWork = CreateCompatibleDC(dc);
      oldObjWork = SelectObject(mdcWork, hBmpWork);
      }
    if(!hdc)
      ReleaseDC(*this, dc);
    }
  return toBool(hBmpWork);
}
//----------------------------------------------------------------------------
#ifndef GET_X_LPARAM
  #define GET_X_LPARAM(lp) LOWORD((DWORD)lp)
  #define GET_Y_LPARAM(lp) HIWORD((DWORD)lp)
#endif
//----------------------------------------------------------------------------
LRESULT PVarTrend::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      removeBmpWork();
      break;
    case WM_ERASEBKGND:
      return 1;

    case WM_PAINT:
      if(GetUpdateRect(hwnd, 0, 0)) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if(!hdc) {
          EndPaint(hwnd, &ps);
          break;
          }
#define USE_MEMDC
#ifdef USE_MEMDC

#define DEF_COLOR_BKG RGB(192, 192, 192)
        if(!dontRedraw) {
          PRect r;
          GetClientRect(*this, r);
          if(allocBmpWork(hdc)) {
            FillRect(mdcWork, r, Brush);
            evPaint(mdcWork);
            BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdcWork, 0, 0, SRCCOPY);
            }
          }
        else if(1 == dontRedraw) {
          PRect r;
          GetClientRect(*this, r);
          BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdcWork, 0, 0, SRCCOPY);
          }
#else
        evPaint(hdc);
#endif
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  if(manCoord) {
    LRESULT res = manCoord->windowProc(hwnd, message, wParam, lParam);
    switch(res) {
      case 0:
        dontRedraw = 0;
        break;
      case -1:
        dontRedraw = 2;
        break;
      case 1:
        PRect rect = manCoord->getRect();
        removeZoom(true);
        rect.Normalize();
        showZoom(rect);
        dontRedraw = 0;
        break;
      }
    }
  return baseWinClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HWND PVarTrend::evSetAroundFocus(UINT key)
{
  HWND hwnd = (HWND)-1;
  return hwnd;
}
//----------------------------------------------------------------------------
void PVarTrend::evPaint(HDC hdc)
{
  PRect r;
  GetClientRect(*this, r);
  COLORREF oldColor = SetBkColor(hdc, iTrend.Bkg);
//  ExtTextOut(hdc, r.left, r.top, ETO_OPAQUE, r, 0, 0, 0);

  drawGrid(hdc);
  int width = iTrend.Inside.Width();
  prfData data;
  dateHourLen.getData(getOwner(), data, getOffs());
  if(!MK_I64(data.U.ft))
    return;
  double rangeTime = (double)MK_I64(data.U.ft) / (SECOND_TO_I64 / 1000);
  uint currTimer = pTrend->getTime();
  // sembra che qualche volta salti il tick del salvataggio ... allora poniamo ad almeno 2.5 volte la dimensione minima
  // avrà valore superiore a zero solo con ampiezza temporale molto bassa
  int minPix = int((double)(width * currTimer * 2.5) / rangeTime);

  iVarTrend.paint(hdc, minPix);
  uint oldC = SetTextColor(hdc, iTrend.Text);
  drawText(hdc);
  SetTextColor(hdc, oldC);
  drawBorder(hdc, r);

//  drawFileShow(hdc);
  SetBkColor(hdc, oldColor);
}
//----------------------------------------------------------------------------
#define OFFS_GRID 4
//----------------------------------------------------------------------------
#define OFFS_AXIS 2
//----------------------------------------------------------------------------
void PVarTrend::drawLeftScaleText(HDC hdc, uint ix, PRect r)
{
  uint oldC = SetTextColor(hdc, iVarTrend.Y_Set[ix].color);
  LPCTSTR name = iVarTrend.varName[ix].name;
  DrawText(hdc, name, _tcslen(name), r, DT_CENTER | DT_PATH_ELLIPSIS | DT_WORD_ELLIPSIS | DT_TOP | DT_WORDBREAK);
  r.bottom = r.top + iTrend.defStr.cy + OFFS_GRID;

  infoRangeTrend range;
  iVarTrend.getRange(ix, range);
  {
    infoRangeTrend rg;
    if(rg.vMax == range.vMax && rg.vMin == range.vMin) {
      range.vMax = 0;
      range.vMin = 0;
      }
  }

  TCHAR buff[64];
  uint nDec = iVarTrend.varName[ix].nDec;
  makeFixedString(range.vMin, buff, SIZE_A(buff), nDec);

  r.MoveTo(r.left, iTrend.Inside.bottom - iTrend.defStr.cy - 2);
  DrawText(hdc, buff, _tcslen(buff), r, DT_TOP | DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_RIGHT);

  double step = range.vMax - range.vMin;
  if(autoStep) {
    int muldiv = 0;
    if(step > 10) {
      while(step > 10) {
        step /= 10;
        ++muldiv;
        }
      step = 1;
      while(muldiv-- > 0)
        step *= 10;
      }
    else {
      while(step < 10) {
        step *= 10;
        ++muldiv;
        }
      step = 1;
      while(muldiv-- > 0)
        step /= 10;
      }
    // step ora è la base -> 0.001, 0.10, 0.1, 1, 10, 100, 1000, ecc.
    // diventa il passo
    if((range.vMax - range.vMin) / step < 2)
      step /= 10;

    double v = 0;
    int addH = r.Height() - r.Height() / 3;
    double ratio = iTrend.Inside.Height() / (range.vMax - range.vMin);
    int bottom = iTrend.Inside.bottom;
    for(uint i = 0; v < range.vMax; ++i, v += step) {
      int y = bottom - ROUND_REAL(ratio * (v + range.vMin)) - iTrend.defStr.cy / 2;
      if(y + addH <= r.top) {
        r.MoveTo(r.left, y);
        makeFixedString(v, buff, SIZE_A(buff), nDec);
        DrawText(hdc, buff, _tcslen(buff), r, DT_TOP | DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_RIGHT);
        }
      }
    int y = ROUND_REAL(ratio * range.vMin) - iTrend.defStr.cy / 2;
    r.MoveTo(r.left, y);
    v = -step;
    addH = r.Height() / 3;
    for(uint i = 0; v > range.vMin; ++i, v -= step) {
      int y = bottom - ROUND_REAL(ratio * (v + range.vMin)) - iTrend.defStr.cy / 2;
      if(y + addH >= r.bottom) {
        r.MoveTo(r.left, y);
        makeFixedString(v, buff, SIZE_A(buff), nDec);
        DrawText(hdc, buff, _tcslen(buff), r, DT_TOP | DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_RIGHT);
        }
      }
    }
  else {
    step /= iTrend.nRow;
    double v = range.vMax - step;
    r.MoveTo(r.left, 0);
    int addH = r.Height() / 3;
    for(uint i = 1; i < iTrend.nRow; ++i, v -= step) {
      int y = iTrend.gridPosY[i - 1] - iTrend.defStr.cy / 2;
      if(i <= 1 || y + addH >= r.bottom) {
        r.MoveTo(r.left, y);
        makeFixedString(v, buff, SIZE_A(buff), nDec);
        DrawText(hdc, buff, _tcslen(buff), r, DT_TOP | DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_RIGHT);
        }
      }
    }
  r.MoveTo(r.left, iTrend.Inside.top);
  makeFixedString(range.vMax, buff, SIZE_A(buff), nDec);
  DrawText(hdc, buff, _tcslen(buff), r, DT_TOP | DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_RIGHT);
  SetTextColor(hdc, oldC);
}
//----------------------------------------------------------------------------
void PVarTrend::drawText(HDC hdc)
{
  if(!pTrend)
    return;
  prfData data;
  activeBit.getData(getOwner(), data, getOffs());
  DWORD bits = data.U.dw;
  if(!bits) {
    data.U.dw = 1;
    genericPerif* prph = getOwner()->getGenPerif(activeBit.getPrph());
    if(prph)
      prph->set(data);
    bits = 1;
    }

  HGDIOBJ oldFont = SelectObject(hdc, iTrend.hFont);
  PRect r;
  GetClientRect(*this, r);
  PRect r2(r);
  r2.right = r2.left + iTrend.defStr.cx;
  r2.top = iTrend.Inside.bottom + OFFS_GRID + OFFS_AXIS;

  for(uint ix = 0; ix < SIZE_BIT(DWORD) && ix < (uint)getNumField(); ++ix, bits >>= 1) {
    if(bits & 1) {
      PRect r3(r2);
      r2.Offset(iTrend.defStr.cx, 0);
      r3.Inflate(-OFFS_GRID - OFFS_AXIS - 2, 0);
      drawLeftScaleText(hdc, ix, r3);
      }
    }

  TCHAR buff[64];
  set_format_data(buff, SIZE_A(buff), ftFrom,  whichData(), _T("\r\n"));
  DrawText(hdc, buff, _tcslen(buff), r, DT_LEFT | DT_TOP | DT_CALCRECT);

  PRect r3(r);
  r3.MoveTo(iTrend.Inside.left + 2, iTrend.Inside.bottom + OFFS_GRID + OFFS_AXIS);
  DrawText(hdc, buff, _tcslen(buff), r3, DT_LEFT | DT_TOP);

  set_format_data(buff, SIZE_A(buff), ftTo,  whichData(), _T("\r\n"));
  DrawText(hdc, buff, _tcslen(buff), r2, DT_RIGHT | DT_TOP | DT_CALCRECT);
  r2.MoveTo(iTrend.Inside.right - r2.Width() - 2, iTrend.Inside.bottom + OFFS_GRID + OFFS_AXIS);
  DrawText(hdc, buff, _tcslen(buff), r2, DT_RIGHT | DT_TOP);

  double step = (double)(MK_I64(ftTo) - MK_I64(ftFrom));
  step /= iTrend.nCol;
  double v = (double)MK_I64(ftFrom) + step;

  for(uint i = 1; i < iTrend.nCol; ++i, v += step) {
    int x = iTrend.gridPosX[i - 1];
    FILETIME ft = I64_TO_FT((__int64)v);
    set_format_data(buff, SIZE_A(buff), ft,  whichData(), _T("\r\n"));
    DrawText(hdc, buff, _tcslen(buff), r, DT_CENTER | DT_TOP | DT_CALCRECT);
    r.MoveTo(x - r.Width() / 2, r2.top);
    if(r.Intersec(r3) || r.Intersec(r2))
      continue;
    r3 = r;
    DrawText(hdc, buff, _tcslen(buff), r, DT_CENTER | DT_TOP);
    }
  SelectObject(hdc, oldFont);
}
//----------------------------------------------------------------------------
void PVarTrend::drawBorder(HDC hdc, PRect r)
{
  if(1 == iTrend.BorderType) {
    HPEN Pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    --r.right;
    --r.bottom;
    HGDIOBJ oldPen = SelectObject(hdc, Pen);
    MoveToEx(hdc, r.left, r.bottom, 0);
    LineTo(hdc, r.right, r.bottom);
    LineTo(hdc, r.right, r.top);
    HPEN Pen2 = CreatePen(PS_SOLID, 1, RGB(0xff, 0xff, 0xff));

    SelectObject(hdc, Pen2);
    MoveToEx(hdc, r.right, r.top, 0);
    LineTo(hdc, r.left, r.top);
    LineTo(hdc, r.left, r.bottom);
    SelectObject(hdc, oldPen);

    DeleteObject(Pen2);
    DeleteObject(Pen);
    }
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
static void arrowRight(HDC hdc, int x, int y)
{
  MoveToEx(hdc, x - HEIGHT_ARROW, y - MID_WIDTH_ARROW, 0);
  LineTo(hdc, x, y);
  LineTo(hdc, x - HEIGHT_ARROW, y + MID_WIDTH_ARROW);
  LineTo(hdc, x - HEIGHT_ARROW, y - MID_WIDTH_ARROW);
}
//----------------------------------------------------------------------------
void PVarTrend::drawAutoRows(HDC hdc, uint ix, int x1, int x2)
{
  infoRangeTrend range;
  iVarTrend.getRange(ix, range);
  {
    infoRangeTrend rg;
    if(rg.vMax == range.vMax && rg.vMin == range.vMin) {
      range.vMax = 0;
      range.vMin = 0;
      }
  }
  double step = range.vMax - range.vMin;
  int muldiv = 0;
  if(step > 10) {
    while(step > 10) {
      step /= 10;
      ++muldiv;
      }
    step = 1;
    while(muldiv-- > 0)
      step *= 10;
    }
  else {
    while(step < 10) {
      step *= 10;
      ++muldiv;
      }
    step = 1;
    while(muldiv-- > 0)
      step /= 10;
    }
  // step ora è la base -> 0.001, 0.10, 0.1, 1, 10, 100, 1000, ecc.
  // diventa il passo, due righe per ogni label
  double tmp = range.vMax - range.vMin;
  if(tmp / step < 2)
    step /= 20;
  else if(tmp / step < 4)
    step /= 10;
  else if(tmp / step < 7)
    step /= 5;
  else
    step /= 2;
  double v = 0;
  double ratio = iTrend.Inside.Height() / (range.vMax - range.vMin);
  int bottom = iTrend.Inside.bottom;
  for(uint i = 0; v < range.vMax; ++i, v += step) {
    int y = bottom - ROUND_REAL(ratio * (v + range.vMin));
    MoveToEx(hdc, x1, y, 0);
    LineTo(hdc, x2, y);
    }

  v = -step;
  for(uint i = 0; v > range.vMin; ++i, v -= step) {
    int y = bottom - ROUND_REAL(ratio * (v + range.vMin));
    MoveToEx(hdc, x1, y, 0);
    LineTo(hdc, x2, y);
    }
}
//----------------------------------------------------------------------------
void PVarTrend::drawGrid(HDC hdc)
{
  uint tpen = PS_SOLID;
  switch(iTrend.GridType) {
    case infoTrend::eDot:
      tpen = PS_DOT;
      break;
    case  infoTrend::eDashed:
      tpen = PS_DASH;
      break;
    default:
      break;
    }

  HPEN Pen2 = CreatePen(tpen, 1, iTrend.Grid);
  HGDIOBJ oldPen = SelectObject(hdc, Pen2);

  PRect r(iTrend.Inside);
  r.left -= OFFS_GRID;
  r.bottom += OFFS_GRID;
  int right = r.right;
  int top = r.top;
  if(infoTrend::eNoGrid == iTrend.GridType) {
    right = r.left + OFFS_GRID;
    top = r.bottom - OFFS_GRID;
    }
  if(!iTrend.gridPosX.getElem()) {
    if(!iTrend.nCol)
      iTrend.nCol = 10;
    double stepX = iTrend.Inside.Width();
    stepX /= iTrend.nCol;
    iTrend.gridPosX.setDim(iTrend.nCol - 1);
    for(uint i = 1; i < iTrend.nCol; ++i)
      iTrend.gridPosX[i - 1] = (int)(i * stepX) + iTrend.Inside.left;
    }
  if(!iTrend.gridPosY.getElem()) {
    if(!autoRows) {
      if(!iTrend.nRow)
        iTrend.nRow = 4;
      double stepY = iTrend.Inside.Height();
      stepY /= iTrend.nRow;
      iTrend.gridPosY.setDim(iTrend.nRow - 1);
      for(uint i = 1; i < iTrend.nRow; ++i)
        iTrend.gridPosY[i - 1] = (int)(i * stepY) + iTrend.Inside.top;
      }
    else {
      prfData data;
      activeBit.getData(getOwner(), data, getOffs());
      DWORD bits = data.U.dw;
      if(!bits)
        bits = 1;
      for(uint ix = 0; ix < SIZE_BIT(DWORD) && ix < (uint)getNumField(); ++ix, bits >>= 1) {
        if(bits & 1) {
          drawAutoRows(hdc, ix, r.left, right);
          break;
          }
        }
      }
    }
  for(uint i = 1; i < iTrend.nCol; ++i) {
    int x = iTrend.gridPosX[i - 1];
    MoveToEx(hdc, x, top, 0);
    LineTo(hdc, x, r.bottom);
    }

  if(!autoRows) {
    for(uint i = 1; i < iTrend.nRow; ++i) {
      int y = iTrend.gridPosY[i - 1];
      MoveToEx(hdc, r.left, y, 0);
      LineTo(hdc, right, y);
      }
    }
  HPEN Pen = CreatePen(PS_SOLID, 2, iTrend.Grid);

  SelectObject(hdc, Pen);
  GetClientRect(*this, r);

  MoveToEx(hdc, 0, iTrend.Inside.bottom + OFFS_AXIS, 0);
  LineTo(hdc, r.right, iTrend.Inside.bottom + OFFS_AXIS);


  MoveToEx(hdc, iTrend.Inside.left - OFFS_AXIS, 0, 0);
  LineTo(hdc, iTrend.Inside.left - OFFS_AXIS, r.bottom);

  arrowUp(hdc, iTrend.Inside.left - OFFS_AXIS, 0);
  arrowRight(hdc, r.right, iTrend.Inside.bottom + OFFS_AXIS);
  SelectObject(hdc, oldPen);
  DeleteObject(Pen2);
  DeleteObject(Pen);
}
//----------------------------------------------------------------------------
void PVarTrend::fillPacket(PacketInfo* packet)
{
  DWORD x = *(LPDWORD)(packet->Value) + iTrend.Inside.left - 1;
  packet->Value[0] = 0;
  iVarTrend.fillPacket(packet, x);
}
//----------------------------------------------------------------------------
LPCTSTR PVarTrend::addColorInfo(LPCTSTR p)
{
  int tick = 0;
  int r = 0;
  int g = 0;
  int b = 0;
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d"), &tick, &r, &g, &b);
  uint nElem = iVarTrend.Y_Set.getElem();
  iVarTrend.Y_Set[nElem] = infoY(RGB(r, g, b), tick);
  return findNextParamTrim(p, 4);
}
//----------------------------------------------------------------------------
void PVarTrend::removeZoom(bool send)
{
  onZoom = false;
  delete manCoord;
  manCoord = 0;
  if(send) {
    prfData data;
    variousBit.getData(getOwner(), data, getOffs());
    data.U.dw &= ~(1 << eb_zoom);
    setVar(variousBit, data, true);
    }
}
//----------------------------------------------------------------------------
void PVarTrend::showZoom(PRect& rect)
{
  zoomInfo Z_I;
  Z_I.full = iTrend.Inside;
  Z_I.sel = rect;

  Z_I.init = ftFrom;
  Z_I.end = ftTo;

  uint maxShow = getTotVar();

  prfData data;
  activeBit.getData(getOwner(), data, getOffs());
  DWDATA dwActive = data.U.dw;

  hideShowBit.getData(getOwner(), data, getOffs());
  DWDATA dwOnShow = data.U.dw;
  Z_I.ixActive = 0;
  for(uint i = 0; i < maxShow; ++i) {
    if(dwActive & (1 << i)) {
      Z_I.ixActive = i;
      break;
      }
    }

  Z_I.infoVar.setDim(maxShow);
  uint nShow = 0;
  typedef PVect<lgCoord> lgCoordSet;

  PVect<lgCoordSet> Set;
  uint nElem = iVarTrend.X_Set.getElem();
  double Scale = iTrend.Inside.Height() / double(MAX_H_DATA);
  int dy = iTrend.Inside.top;
  for(uint i = 0; i < maxShow; ++i) {
    if(dwOnShow & (1 << i)) {
      infoRangeTrend r;
      iVarTrend.getRange(i, r);
      Z_I.infoVar[nShow].minVal = r.vMin;
      Z_I.infoVar[nShow].maxVal = r.vMax;
      Z_I.infoVar[nShow].frg = iVarTrend.Y_Set[i].color;
      Set[nShow].setDim(nElem);
      ySet& yS = iVarTrend.Y_Set[i].Set;
      lgCoordSet& cSet = Set[nShow];
      for(uint j = 0; j < nElem; ++j)
        cSet[j] = (lgCoord)((yS[j].Y - dy) / Scale);
      Z_I.infoVar[nShow].sourceY = &cSet;
      ++nShow;
      }
    }
  Z_I.nLabel = iTrend.nRow;
  Z_I.Dec = iVarTrend.varName[Z_I.ixActive].nDec;
  Z_I.bkg = iTrend.Bkg;
  Z_I.textColor = iTrend.Text;
  Z_I.totPoint = nElem;

  lgCoordSet XCoord;
  XCoord.setDim(nElem);
  int dx = iTrend.Inside.left;
  for(uint j = 0; j < nElem; ++j)
    XCoord[j] = (iVarTrend.X_Set[j].X - dx) * MUL_W_DATA;
  Z_I.sourceX = &XCoord;
  Z_I.hFont = iTrend.hFont;

  mainClient* Par = getParentWin<mainClient>(this);
  Par->enableDialogFunctionKey();
  lgraph2_zoom lZ(this, Z_I);
  startSimulDialog(lZ, this);
  Par->disableDialogFunctionKey();
}
//----------------------------------------------------------------------------
void PVarTrend::runShowValue()
{
  PRect r(iTrend.Inside);
  POINT pt = { r.left, r.top };
  SIZE sz = { r.Width(), r.Height() };
  PBitmap bmp(mdcWork, pt, sz);
  MapWindowPoints(*this, HWND_DESKTOP, (LPPOINT)(LPRECT)r, 2);
  DWORD bitShow = 0;
  uint nElem = getNumField();
  for(uint i = 0; i < nElem; ++i)
    if(iVarTrend.Y_Set[i].isVisible())
      bitShow |= 1 << i;

  PSliderWin slider(this, 111, r, &bmp, iVarTrend.varName, iTrend, bitShow);
  slider.setLimits(Left, Right);

  mainClient* Par = getParentWin<mainClient>(this);
  Par->enableDialogFunctionKey();
  dontRedraw = 1;

  startSimulDialog(slider, this);

  Par->disableDialogFunctionKey();
  slider.getLimits(Left, Right);
  dontRedraw = 0;
  update(true);
}
//--------------------------------------------------------------------
#include "dlistExp.h"
extern bool queryExportPath(PWin* parent, LPTSTR fileExp);
//--------------------------------------------------------------------
void PVarTrend::exportTrend()
{
  if(!pTrend)
    return;

  mainClient* wMain = getMain();
  if(!wMain)
    return;

  TCHAR source[_MAX_PATH];
  if(!pTrend->makeName(source, ftFrom))
    return;
  makePath(source, dExRep, false, _T(""));

  TCHAR target[_MAX_PATH];
  do {
    TCHAR tf[_MAX_PATH];
    _tsplitpath_s(source, 0, 0, 0, 0, tf, SIZE_A(tf), 0, 0);

    makeExportPath(target, _T(""), true, dHistory);
    createDirectoryRecurse(target);

    makeExportPath(target, tf, true, dHistory);
    _tcscat_s(target, _T(".tab"));
    if(!queryExportPath(this, target))
      return;
    } while(false);
  P_File pfS(source, P_READ_ONLY);
  if(!pfS.P_open())
    return;
  P_File pfT(target, P_CREAT);
  if(!pfT.P_open())
    return;

  returnExp result = exportExByGraph(this, pfT, pfS, pTrend->getCode());
  if(rAbort != result)
    msgBoxByLangGlob(this, ID_EXP_FAILED + result, ID_EXP_SUCCESS_TITLE, MB_ICONINFORMATION);
}
//----------------------------------------------------------------------------
#include "printGraph.h"
//--------------------------------------------------------------------
extern svPrinter* getPrinter();
//--------------------------------------------------------------------
lgCoord PVarTrend::trasform4Comp(uint ix)
{
  return (lgCoord)((iVarTrend.X_Set[ix].X - iTrend.Inside.left) * MUL_W_DATA);
}
//--------------------------------------------------------------------
void PVarTrend::print_Graph()
{
  if(!iVarTrend.X_Set.getElem())
    return;
  infoPrintGraph ipg;
  ipg.numPage = 4;
  ipg.trendType = -1;

  uint maxShow = getTotVar();

  prfData data;
  hideShowBit.getData(getOwner(), data, getOffs());
  DWDATA dwOnShow = data.U.dw;

  activeBit.getData(getOwner(), data, getOffs());
  DWDATA dwActive = data.U.dw;

  uint ixActive = 0;
  for(uint i = 0; i < maxShow; ++i) {
    if(dwActive & (1 << i)) {
      ixActive = i;
      break;
      }
    }
  ipg.nElem = iVarTrend.X_Set.getElem();
  ipg.XCoord = new lgCoord[ipg.nElem];

  ipg.XCoord[0] = trasform4Comp(0);
  for(int i = 1; i < ipg.nElem; ++i) {
    ipg.XCoord[i] = trasform4Comp(i);
    if(ipg.XCoord[i] < ipg.XCoord[i - 1])
      ipg.XCoord[i] = ipg.XCoord[i - 1];
    }

  ipg.pixWidth = iTrend.Inside.Width() * MUL_W_DATA;
  ipg.from = MK_I64(ftFrom);
  ipg.to   = MK_I64(ftTo);
  ipg.numRow = iTrend.nRow;
  ipg.dimMinRow = 50;
  ipg.U.f.graphOver = 1;

  {
    TCHAR tmp[_MAX_FNAME];
    TCHAR source[_MAX_PATH];
    if(!pTrend->makeName(source, ftFrom))
      return;
    _tsplitpath_s(source, 0, 0, 0, 0, tmp, SIZE_A(tmp), 0, 0);
    int len = _tcslen(tmp);

    LPCTSTR pt = getString(ID_MAIN_TITLE);
    if(!pt)
      pt = _T("?");
    int lenT = _tcslen(pt);
    if(lenT + len > MAX_TITLE_PRINT)
      lenT = MAX_TITLE_PRINT - (len + 1 + 4);

    _tcsncpy_s(ipg.title, SIZE_A(ipg.title), pt, lenT);

    ipg.title[lenT] = 0;
    _tcscat_s(ipg.title, SIZE_A(ipg.title), _T(" - "));
    _tcscat_s(ipg.title, SIZE_A(ipg.title), tmp);
  }

  ipg.nDec = iVarTrend.varName[ixActive].nDec;

  ipg.FieldSet.setDim(maxShow);

  uint nShow = 0;

  double Scale = iTrend.Inside.Height() / double(MAX_H_DATA);
  int dy = iTrend.Inside.top;
  for(uint i = 0; i < maxShow; ++i) {
    if(dwOnShow & (1 << i)) {
      infoRangeTrend r;
      iVarTrend.getRange(i, r);

      ipg.yGraph[nShow] = new lgCoord[ipg.nElem];
      ySet& yS = iVarTrend.Y_Set[i].Set;
      for(int j = 0; j < ipg.nElem; ++j)
        ipg.yGraph[nShow][j] = (lgCoord)((yS[j].Y - dy) / Scale);

      int dec = iVarTrend.varName[i].nDec;

      infoPrintGraph::field fld;
      copyStr(fld.info, (LPCTSTR)iVarTrend.varName[i].name, _tcslen(iVarTrend.varName[i].name) + 1);
      fld.vMin = r.vMin;
      fld.vMax = r.vMax;
      fld.color = iVarTrend.Y_Set[i].color;
      fld.tickness = iVarTrend.Y_Set[i].tickness;
      fld.nDec = iVarTrend.varName[i].nDec;
      ipg.FieldSet[nShow] = fld;
      ++nShow;
      }
    }
  ipg.numGraph = nShow;
  ipg.U.f.forceLabel = 1;
  ipg.U.f.activeLabel = ixActive;

//  ipg.IIP.U.f.drawLabels = true;
  smartPointerConstString tmsg = getStringOrIdByLangGlob(ID_GET_PAGE_G);
  smartPointerConstString ttit = getStringOrIdByLangGlob(ID_TITLE_GET_PAGE_G);

  static uint nPage = 1;
  if(IDOK == TD_Timer(this, &ttit, &tmsg, nPage).modal()) {
    if(nPage > MAX_PAGE)
      nPage = MAX_PAGE;
    ipg.numPage = nPage;
    printGraph(this, getPrinter(), ipg).Preview(false);
    }

  for(int i = nShow - 1; i >= 0; --i)
    delete []ipg.yGraph[i];
  delete []ipg.XCoord;
};
//----------------------------------------------------------------------------
void PVarTrend::printerSetup()
{
  svPrinter* Printer = getPrinter();
  if(Printer) {
    Printer->setResetPageSetupFlag(0, PSD_RETURNDEFAULT);
    Printer->setup(PPrinter::DlgPrintProperty);
    }
}
//----------------------------------------------------------------------------
#if 0
  #define constrainX(x, rect)
  #define constrainY(y, rect)
#else
static void constrainX(int& x, const PRect& rect)
{
  if(x < rect.left)
    x = rect.left;
  if(x > rect.right)
    x = rect.right;
}
//----------------------------------------------------------------------------
static void constrainY(int& y, const PRect& rect)
{
  if(y < rect.top)
    y = rect.top;
  if(y > rect.bottom)
    y = rect.bottom;
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
//#define EPSILON 0.000001
#define EPSILON FLT_EPSILON
//----------------------------------------------------------------------------
void infoY::recalcPix(const PRect& rect)
{
  float d = Range.vMax - Range.vMin;
  if(fabs(d) < EPSILON)
    return;
  int h = rect.Height();
  float r = (float)h / d;
  uint nElem = Set.getElem();
  for(uint i = 0; i < nElem; ++i) {
    Set[i].Y = rect.bottom - (int)((Set[i].Value - Range.vMin) * r);
    constrainY(Set[i].Y, rect);
    }

}
//----------------------------------------------------------------------------
void infoY::getAutoRange(infoRangeTrend& range)
{
  uint nElem = Set.getElem();
  range.vMin = 0;
  range.vMax = 0;
  if(!nElem)
    return;
  infoRangeTrend rg;
  for(uint i = 0; i < nElem; ++i)
    rg.calc(Set[i].Value);
  range = rg;
}
//----------------------------------------------------------------------------
void infoY::simpleAppend(float val)
{
  Range.calc(val);
  Set[Set.getElem()] = infoY_Coord(val);
  Set.setIncr();
}
//----------------------------------------------------------------------------
void infoY::append(float val, const PRect& rect)
{
  append(infoY_Coord(val), rect);
}
//----------------------------------------------------------------------------
void infoY::append(const infoY_Coord& val, const PRect& rect)
{
  uint nElem = Set.getElem();
  float d = Range.vMax - Range.vMin;
  int h = rect.Height();
  if(fabs(d) < EPSILON || !h) {
    Set[nElem] = infoY_Coord(0, val.Value);
    return;
    }
  float r = (float)h / d;
  int y = rect.bottom - (int)((val.Value - Range.vMin) * r);
  constrainY(y, rect);

  Set[nElem] = infoY_Coord(y, val.Value);
  Set.setIncr();
}
//----------------------------------------------------------------------------
class manAvr
{
  public:
    manAvr() { }

    void addVal(int v);
    int getResult();
  private:
    struct infoAvr
    {
      int n;
      double v;
      void add(int val);
      infoAvr() : n(0), v(0) {}
    };
  infoAvr info[3]; // 0 -> minore, 1 -> media corrente, 2 -> massima
};
//----------------------------------------------------------------------------
void manAvr::infoAvr::add(int val)
{
  v *= n;
  v += val;
  ++n;
  v /= n;
}
//----------------------------------------------------------------------------
void manAvr::addVal(int v)
{
#if 1
  if(v > info[1].v)
    info[2].add(v);
//  else if(v < info[1].v)
//    info[0].add(v);
  info[1].add(v);
#else
  info[1].add(v);
  #if 0
// non usiamo il valore minimo
  if(v < info[0].v || v < info[1].v)
    info[0].add(v);
  else
  #endif
  if(v > info[2].v || v > info[1].v)
    info[2].add(v);
#endif
}
//----------------------------------------------------------------------------
int manAvr::getResult()
{
  double diff = info[2].v - info[1].v;
  double v = diff > 10 ? info[2].v + diff : info[2].v * 1.2;
  return ROUND_POS_REAL(v);
//  return ROUND_POS_REAL(info[2].v * 1.5);
}
//----------------------------------------------------------------------------
void infoY::paint(HDC hdc, const PVect<infoX_Coord>& XSet, int minPix)
{
  if(!isVisible())
    return;
  uint nElem = min(Set.getElem(), XSet.getElem());
  if(nElem < 2)
    return;
  HPEN pen = CreatePen(PS_SOLID, tickness, color);
  HGDIOBJ old = SelectObject(hdc, pen);
#if 0
  POINT pt = { XSet[0].X, Set[0].Y  };
  MoveToEx(hdc, pt.x, pt.y, 0);

  for(uint i = 1; i < nElem; ++i) {
    if(pt.x != XSet[i].X || pt.y != Set[i].Y) {
      pt.x = XSet[i].X;
      pt.y = Set[i].Y;
      LineTo(hdc, pt.x, pt.y);
      }
    }
#else
  if(nElem > nPoint) {
    delete []pPT;
    pPT = new POINT[nElem];
    nPoint = nElem;
    }
  uint truePoint = 1;
  POINT pt = { XSet[0].X, Set[0].Y  };
  pPT[0] = pt;
  manAvr mAvr;
  int maxDiff = (XSet[nElem - 1].X - pt.x);
  for(uint i = 1; i < nElem; ++i) {
    if(pt.x != XSet[i].X || pt.y != Set[i].Y) {
      int t = XSet[i].X - pt.x;
      if(t > 0)
        mAvr.addVal(t);
      pPT[truePoint].x = XSet[i].X;
      pPT[truePoint].y = Set[i].Y;
      pt = pPT[truePoint];
      ++truePoint;
      }
    }
  maxDiff = min(mAvr.getResult(), maxDiff);
  maxDiff = max(minPix, maxDiff);
  pt = pPT[0];
  uint lastIx = 0;
  for(uint i = 1; i < truePoint; ++i) {
    if((pPT[i].x - pt.x > maxDiff) && (i - lastIx > 1)) {
      Polyline(hdc, pPT + lastIx, i - lastIx);
      lastIx = i;
      }
    pt = pPT[i];
    }
  if(truePoint - lastIx > 1)
    Polyline(hdc, pPT + lastIx, truePoint - lastIx);
#endif
  DeleteObject(SelectObject(hdc, old));
}
//----------------------------------------------------------------------------
void infoVarTrend::append(uint ix, float val, const PRect& rect)
{
  uint nElem = Y_Set.getElem();
  if(ix >= nElem)
    return;
  Y_Set[ix].append(val, rect);
}
//----------------------------------------------------------------------------
bool infoVarTrend::append(__int64 curr, __int64 start, __int64 width, const PRect& rect)
{
  if(!width || !rect.Width())
    return false;

  int w = rect.Width();

  double r = (double)w / (double)width;
  int x = rect.left + (int)((curr - start) * r);

  constrainX(x, rect);
  uint nElem = X_Set.getElem();

  X_Set[nElem] = infoX_Coord(x, curr);
  X_Set.setIncr();
  return true;
}
//----------------------------------------------------------------------------
void infoVarTrend::simpleAppend(uint ix, float val)
{
  uint nElem = Y_Set.getElem();
  if(ix >= nElem)
    return;
  Y_Set[ix].simpleAppend(val);
}
//----------------------------------------------------------------------------
void infoVarTrend::getAutoRange(uint ix, infoRangeTrend& range)
{
  uint nElem = Y_Set.getElem();
  if(ix >= nElem)
    return;
  Y_Set[ix].getAutoRange(range);
}
//----------------------------------------------------------------------------
void infoVarTrend::recalcPix(uint ix, const PRect& rect)
{
  uint nElem = Y_Set.getElem();
  if(ix >= nElem)
    return;
  Y_Set[ix].recalcPix(rect);
}
//----------------------------------------------------------------------------
void infoVarTrend::simpleAppend(__int64 curr)
{
  X_Set[X_Set.getElem()] = infoX_Coord(curr);
  X_Set.setIncr();
}
//----------------------------------------------------------------------------
void infoVarTrend::recalcPix(__int64 start, __int64 width, const PRect& rect)
{
  if(!width || !rect.Width())
    return;

  int w = rect.Width();

  double r = (double)w / (double)width;

  uint nElem = X_Set.getElem();

  for(uint i = 0; i < nElem; ++i) {
    __int64 t = X_Set[i].Time - start;
    if(t < 0)
      t = 0;
    X_Set[i].X = rect.left + (int)(t * r);
    constrainX(X_Set[i].X, rect);
    }
}
//----------------------------------------------------------------------------
bool infoVarTrend::getRange(uint ix, infoRangeTrend& r) const
{
  uint nElem = Y_Set.getElem();
  if(ix >= nElem)
    return false;
  r = Y_Set[ix].getRange();
  return true;
}
//----------------------------------------------------------------------------
void infoVarTrend::setRange(uint ix, const infoRangeTrend& r)
{
  uint nElem = Y_Set.getElem();
  if(ix >= nElem)
    return;
  Y_Set[ix].setRange(r);
}
//----------------------------------------------------------------------------
void infoVarTrend::paint(HDC hdc, int minPix)
{
  uint nElem = Y_Set.getElem();
  for(uint i = 0; i < nElem; ++i)
    Y_Set[i].paint(hdc, X_Set, minPix);
}
//----------------------------------------------------------------------------
void infoVarTrend::resetRange()
{
  uint nElem = Y_Set.getElem();
  infoRangeTrend r;
  for(uint i = 0; i < nElem; ++i)
    Y_Set[i].setRange(r);
}
//----------------------------------------------------------------------------
void infoVarTrend::resetXY()
{
  X_Set.reset();
  uint nElem = Y_Set.getElem();
  for(uint i = 0; i < nElem; ++i)
    Y_Set[i].resetY();
}
//----------------------------------------------------------------------------
static int cmp_X_Value(const infoX_Coord& v, const infoX_Coord& ins)
{
  return v.X - ins.X;
}
//----------------------------------------------------------------------------
bool infoVarTrend::fillPacket(PacketInfo* packet, int x)
{
  if(!X_Set.getElem())
    return false;

  infoX_Coord t(x);
  uint pos = 0;
  bool found = X_Set.find(t, pos, cmp_X_Value);
// verificare se fare la media o prendere semplicemente il valore della posizione trovata
  uint nElem = Y_Set.getElem();
  if(nElem > SIZE_A(packet->Value))
    nElem = SIZE_A(packet->Value);
  for(uint i = 0; i < nElem; ++i)
    packet->Value[i] = Y_Set[i].Set[pos].Value;
  packet->Ft = I64_TO_FT(X_Set[pos].Time);
  return true;
}
//--------------------------------------------------------------------
