//---------- lgraph2-b.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
//----------------------------------------------------------------------------
#include "macro_utils.h"
#include "lgraph2-b.h"
#include "pOpensave.h"
#include "gestdata.h"
#include "svdialog.h"
#include "id_btn.h"
#include "d_choose.h"
#include "mainclient.h"
#include "lnk_body.h"
#include "pBitmap.h"
#include "sizer.h"
#include "def_dir.h"
#include "P_BarProgr.h"
#include "password.h"

#include "pvaredit.h"
#include "p_date.h"
#include "p_util.h"
#include "p_migl.h"
#include "gestprf.h"
#include "pVarListBox.h"
#include "dserchfl.h"
#include "LimitsDlg.h"

#include "1.h"
#include "id_msg_common.h"
//#include "1000000.h"
#include "dchoosnm.h"
#include "dlistExp.h"
//--------------------------------------------------------------------
#define DEF_TYPED_ROW         10
//--------------------------------------------------------------------
//#define USE_PRF_0
//----------------------------------------------------------------------------
#define DATE_FROM   0
#define DATE_TO     (DATE_FROM + 1)
#define TIME_FROM   (DATE_TO + 1)
#define TIME_TO     (TIME_FROM + 1)

#define VALUE_FROM DATE_FROM
#define VALUE_TO   (VALUE_FROM + 1)
//----------------------------------------------------------------------------
// Viene usato il tipo __int64 perché può contenere anche il tipo FILETIME.
// In caso di utilizzo per dati fREALDATA occorre effettuare i cast definiti
// sotto per evitare di perdere decimali. Il tipo __int64 non può essere usato
// senza cast, a meno che la base sia FILETIME
//----------------------------------------------------------------------------
#define CAST_fREAL_TO_I64(a) ((__int64)*(DWORD*)&a)
#define CAST_I64_TO_fREAL(a) (*(fREALDATA*)&a)
#define CAST_I64(a) MK_I64(*(FILETIME*)(a))
//-----------------------------------------------------------
typedef PVect<COLORREF> vTrendColors;
//----------------------------------------------------------------------------
void loadLBColors(COLORREF& col, LPCTSTR p);
//----------------------------------------------------------------------------
class trendListBox : public PVarListBox
{
  public:
    trendListBox(const vTrendColors& v, P_BaseBody* parent,
              uint style, uint id, const PRect& rect, int len = 255,
              HINSTANCE hinst = 0);
    bool hasFont() const { return toBool(getFont()); }
  protected:

    void paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
    vTrendColors colors;
  private:
    typedef PVarListBox baseClass;
};
//------------------------------------------------------------------
trendListBox::trendListBox(const vTrendColors& v, P_BaseBody* parent, uint style,
        uint id, const PRect& rect, int len, HINSTANCE hinst):
       PVarListBox(parent, style, id, rect, len, hinst), baseActive(parent, id)
{
  int nElem = v.getElem();
  colors.setDim(nElem);
  for(int i = 0; i < nElem; ++i)
    colors[i] = v[i];
}
//------------------------------------------------------------------
void trendListBox::paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(!ix) {
    uint item = drawInfo.itemID;
    if(item >= MAX_CHOOSE)
      return;
    PRect r2(r);
    r2.Inflate(-1, -2);
    r2.Offset(1, 0);
    HBRUSH hbrush = CreateSolidBrush(colors[item]);
    FillRect(hdc, r2, hbrush);
    DeleteObject(hbrush);
    }
  baseClass::paintCustom(hdc, r, ix, drawInfo);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define BKG_COLOR_N RGB(0xff, 0xc0, 0xc0)
#define FG_COLOR_N RGB(0x7f, 0, 0)
//----------------------------------------------------------------------------
#define BKG_COLOR_S RGB(0xc0, 0xff, 0xc0)
#define FG_COLOR_S RGB(0x7f, 0, 0)
//----------------------------------------------------------------------------
#define C__(a) RGB(r, g, b)
//--------------------------------------------------------------------
COLORREF getColor(LPCTSTR p, int *tick)
{
  if(!p) {
    *tick = 1;
    return RGB(0xff, 0xff, 0xff);
    }
  int red;
  int green;
  int blue;
  _stscanf_s(p, _T("%d,%d,%d,%d"), &red, &green, &blue, tick);
  return RGB(red, green, blue);
}
//----------------------------------------------------------------------------
void loadLBColors(COLORREF& col, LPCTSTR p)
{
  int dummy;
  col = getColor(p, &dummy);
}
//----------------------------------------------------------------------------
#define ID_INIT_CODE_COLORS 1000001
//----------------------------------------------------------------------------
PVarListBox* TD_LGraph2b::allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen, HINSTANCE hinst)
{
  vTrendColors cols;
  LPCTSTR p = getPageString(ID_INIT_CODE_COLORS);
  if(p) {
    cols.setDim(MAX_LINEAR_GRAPH);
    for(int i = 0; i < MAX_LINEAR_GRAPH; ++i) {
      LPCTSTR p2 = getPageString(ID_INIT_CODE_COLORS + i);
      if(p2)
        loadLBColors(cols[i], p2);
      else
        loadLBColors(cols[i], 0);
      }
    }
  return new trendListBox(cols, parent, style, id, rect, textlen, hinst);
}
//--------------------------------------------------------------------
linear::linear(PRect rect, COLORREF bkg, bStyle type, COLORREF frg, bool axis) :
  PLinearGraph(rect, bkg, type, frg, axis), BuffX(0), BuffY(0),
  Curr(0), Scale(rect.Height() / double(MAX_H_DATA))
{ }
//--------------------------------------------------------------------
PLinearGraph::sPoint linear::getNext()
{
  if(BuffY && BuffX && Curr < getNPoints()) {
    lgCoord val = (lgCoord)(BuffY[Curr] * Scale);
    if(val <= 0)
      val = 1;

    lgCoord x = static_cast<lgCoord>(BuffX[Curr] / MUL_W_DATA);
    if(x <= 0)
      x = 1;
    sPoint p(x, val);
    ++Curr;
    return p;
    }
  return sPoint(-1, -1);
}
//-----------------------------------------------------------
void linear::drawLines(HDC hdc)
{
  HPEN pen = CreatePen(PS_SOLID, getTick(), getLineColor());
  HGDIOBJ old = SelectObject(hdc, pen);

  int hMax = getRect().Height() - 2;

  int wMax = getNPoints();
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

  sPoint spt = init;
  int minDiff = 1000000;
  uint truePoint = 1;
  for(int i = 1; i < wMax; ++i) {
    init = getNext();
    if(init.y <= 0)
      break;
    else if(init.y >= hMax)
      init.y = lgCoord(hMax - 1);
    if(init.x <= 0)
      break;
    else if(init.x >= width)
      init.x = lgCoord(width - 1);

    if(spt.x != init.x || spt.y != init.y) {
      int t = init.x - spt.x;
      if(t > 0 && t < minDiff)
        minDiff = t;
      pp[truePoint].x = init.x + left;
      pp[truePoint].y = init.y + top;
      ++truePoint;
      spt = init;
      }
    }

  minDiff *= 20;

  POINT pt = pp[0];
  uint lastIx = 0;
  for(uint i = 1; i < truePoint; ++i) {
    if(pp[i].x - pt.x > minDiff) {
      Polyline(hdc, pp + lastIx, i - lastIx);
      lastIx = i;
      }
    pt = pp[i];
    }
  if(truePoint - lastIx > 1)
    Polyline(hdc, pp + lastIx, truePoint - lastIx);
  delete []pp;
  SelectObject(hdc, old);
  DeleteObject(pen);
}
//----------------------------------------------------------------------------
P_Body *getLGraph2(int idPar, PWin *parent)
{
  P_Body *bd = new TD_LGraph2b(idPar, parent);
  return bd;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static int IdParent;
bool TD_LGraph2b::showOver = true;
//----------------------------------------------------------------------------
#define NAME_STR getPageName()
#define DEF_WAIT_REFRESH 3000
//----------------------------------------------------------------------------
#define DEF_MAX_REC2SHOW 5000
//-------------------------------------------------------------------
#define MIN_REC_2_SHOW  500
#define MAX_REC_2_SHOW  100000
//----------------------------------------------------------------------------
TD_LGraph2b::TD_LGraph2b(int idPar, PWin* parent, int resId, HINSTANCE hinst)
:
    baseClass(idPar, parent, resId, hinst), RangeSet(new rangeSet), RangeSetOnUse(new rangeSet),
    lastTotVal(0), canUpdate(false), offsStat(sizeof(FILETIME)),
    sizeStat(0), sizeHeader(0), val4X(USE_FILETIME_4_X),
    BuffStat(0), fixedScale(0), firstEnter(true), fxType(fByHour),
    uniqueRange(false), count4refresh(0),
    maxWaitBeforeRefresh(0),
//    maxWaitBeforeRefresh(DEF_WAIT_REFRESH),
    onlyRefresh(false), byChooseFile(false), insideTime(false), usePrph_0(true),
    InfoFile(0), textColor(0)//RGB(0x7f, 0x7f, 0x7f))
    ,firstInvalidate(true)
    ,ManCoord(0)
    ,dontRedraw(false)
    ,maxRecord2Show(DEF_MAX_REC2SHOW)
    ,idTimerRefresh(0)
    ,currDay((WORD)-1)
    ,lastCurrVal(0)

{
  if(idPar)
    IdParent = idPar;
  for(int j = 0; j < MAX_LINEAR_GRAPH; ++j)
    lGraph[j] = 0;
  MK_I64(lastUpdate) = 0;
  fillStr(fileName, 0, SIZE_A(fileName));

#if 0
  TCHAR path[_MAX_PATH] = _T("\0");
  getFileStr(path);

  fillPageStr(path);
#endif
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
  memset(RangeSetOnUse, 0, sizeof(*RangeSetOnUse));

}
//----------------------------------------------------------------------------
TD_LGraph2b::~TD_LGraph2b()
{
  destroy();
  for(int i = 0; i < MAX_LINEAR_GRAPH; ++i)
    delete lGraph[i];

  delete RangeSet;
  delete RangeSetOnUse;
  delete []BuffStat;
  flushPV_toFree(FreeFile, List);
  flushPV(infoStat);
  delete InfoFile;
  delete ManCoord;
}
//----------------------------------------------------------------------------
bool TD_LGraph2b::create()
{
  if(!baseClass::create())
    return false;

  Attr.id = IDD_LGRAPH2;

  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_LGraph2b::checkInside()
{
  __int64 curr = MK_I64(getFileTimeCurr());
  insideTime = RangeSet->from < curr && curr < RangeSet->to;
}
//----------------------------------------------------------------------------
void TD_LGraph2b::makeInside()
{
  __int64 delta = RangeSet->to - RangeSet->from;
  RangeSet->from = MK_I64(getFileTimeCurr()) - (delta / 2);
  RangeSet->to = RangeSet->from + delta;
  insideTime = true;
  saveSet();
}
//----------------------------------------------------------------------------
#define MAX_LEN_DUMMY   4
#define MAX_LEN_MINMAX 12
//----------------------------------------------------------------------------
#define OFFS_TRENDTIMER 0
//----------------------------------------------------------------------------
void TD_LGraph2b::setReady(bool first)
{
  baseClass::setReady(true);

  if(LBox.getElem()) {

    int tabs[] = { MAX_LEN_DUMMY, MAX_DESCR, -MAX_LEN_MINMAX, -MAX_LEN_MINMAX };
    int show[] = { 1, 1, 1, 1 };
    LBox[0]->SetTabStop(SIZE_A(tabs), tabs, show);

    trendListBox* lb = dynamic_cast<trendListBox*>(LBox[0]);
    if(!lb || !lb->hasFont()) {
      LPCTSTR p = getPageString(ID_FONT_LISTBOX);
      if(p) {
        int h_font = R__Y(_ttoi(p));
        if(h_font) {
          p = findNextParam(p, 1);
          int style = _ttoi(p);
          HFONT Font = D_FONT(h_font, 0, style, p);
          LBox[0]->setFont(Font, true);
          }
        }
      }
    LBox[0]->setAlign(1, PListBox::aCenter);

    PRect r;
    GetClientRect(*LBox[0], r);
    double widthChar = r.Width() - 2 - GetSystemMetrics(SM_CXVSCROLL);

    widthChar /= MAX_DESCR + MAX_LEN_MINMAX * 2 + 4;
    LBox[0]->setPixelWidthChar(widthChar);

    LBox[0]->recalcWidth();
    LBox[0]->setIntegralHeight();
    }

  LPCTSTR p = getPageString(ID_OFFSET_INIT_STAT_FILE);
  if(p && *p)
    offsStat = _ttoi(p);

  p = getPageString(ID_SIZEOF_HEADER);
  if(p)
    sizeHeader = _ttoi(p);

  p = getPageString(ID_TREND_AX_COLOR);
  if(p) {
    int r = 0;
    int g = 0;
    int b = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
    textColor = RGB(r, g, b);
    }

  LPCTSTR pNameTrend = getPageString(ID_TREND_NAME);

  uint tot;
  uint id = getInitDataByNameTrend(pNameTrend, tot);
  if(id) {
    sizeStat = tot * sizeof(fREALDATA) + offsStat;
    infoStat.setDim(tot);

    for(uint i = 0; i < tot; ++i) {
      LPCTSTR pt = getString(i + id);
      if(!pt)
        break;
      rangeInFile* rIF = new rangeInFile;
      rIF->offsStatFile = i;

      pt = findNextParam(pt, 4);
      rIF->dec = _ttoi(pt);
      LPCTSTR p = findNextParam(pt, 1);
      p = checkNewMode(p);

      if(!p)
        p = pt;
      if(!p)
        p = _T("???");
      smartPointerConstString sp = getStringByLangSimple(p);

      int len = _tcslen(&sp);
      len = min(len, MAX_DESCR);
      copyStr(rIF->descr, &sp, len);
      rIF->descr[len] = 0;
      int nElem = infoStat.getElem();
      infoStat[nElem] = rIF;
      }
    }
  do {
    p = getPageString(DATE_FROM + ID_INIT_VAR_EDI + ADD_INIT_VAR);
    int idprf = 0;
    if(p)
      idprf = _ttoi(p);
    usePrph_0 = !idprf;
    } while(false);

  p = getPageString(ID_NUM_DATA_STAT_FILE);
  if(p) {
    LPCTSTR pt = findNextParam(p, 1);
    if(pt)
      uniqueRange = toBool(_ttoi(pt));
    else
      uniqueRange = toBool(_ttoi(p));
    }
  if(!sizeStat) {
    int nElem = infoStat.getElem();
    int offs = 0;
    for(int i = 0; i < nElem; ++i)
      if(offs < infoStat[i]->offsStatFile)
        offs = infoStat[i]->offsStatFile;
    sizeStat = (offs + 1) * sizeof(fREALDATA) + offsStat;
    }

  BuffStat = new BYTE[sizeStat + 4];

  p = getPageString(ID_TREND_X_BASED);
  val4X = USE_FILETIME_4_X;
  if(p) {
    val4X = _ttoi(p);
    p = findNextParam(p, 1);
    if(p) {
      fixedScale = _ttoi(p);
      p = findNextParam(p, 1);
      if(p)
      fxType = (TD_LGraph2b::fixType)_ttoi(p);
      }
    }
#if 1
  uint nElem = Simple.getElem();
  for(uint i = 0; i < nElem; ++i) {
    PSimpleText* st = dynamic_cast<PSimpleText*>(Simple[i]);
    if(st) {
      if(_T('?') == *st->get_Text()) {
        InfoFile = st;
        Simple.remove(i);
        break;
        }
      }
    }

#else
  p = getPageString(ID_INFO_FILENAME);
  while(!p) {
    p = getPageString(ID_SIMPLE_TXT);
    if(!p)
      break;
    int nTxt = _ttoi(p);
    for(int i = 0; i < nTxt; ++i) {
      p = getPageString(ID_INIT_SIMPLE_TXT + i);
      if(!p)
        continue;
      p = findNextParam(p, 13);
      if(!p)
        continue;
      int ix = _ttoi(p);
      p = getPageString(ix);
      if(!p)
        continue;
      if(_T('?') == *p) {
        TCHAR t[20];
        wsprintf(t, _T("%d"), ID_INIT_SIMPLE_TXT + i);
        sStr.replaceString(ID_INFO_FILENAME, str_newdup(t), true);
        break;
        }
      }
    break;
    }
#endif
  loadSet();

  for(maxChoose = 0; maxChoose < MAX_LINEAR_GRAPH; ++ maxChoose)
    if(!RangeSet->choose[maxChoose])
      break;

  if(fixedScale)
    makeInside();

  showSet();

  makeLinear();
  if(!canUpdate) {
    canUpdate = true;
//    makeAllGraph();
    }
  if(LBox.getElem())
    fillLB();
//  invalidateGraph();
  firstEnter = true;
  p = getPageString(ID_WAIT_FIXED_REFRESH);
  if(p) {
    maxWaitBeforeRefresh = _ttoi(p);
    mainClient* par = getMain();
    maxWaitBeforeRefresh += par->getTimeForTimer() / 2;
    maxWaitBeforeRefresh /= par->getTimeForTimer();
    if(maxWaitBeforeRefresh < 2 || maxWaitBeforeRefresh > 1000)
      maxWaitBeforeRefresh = 10;
    }
  if(USE_FILETIME_4_X == val4X) {
    PRect rect = getCoordGraph();
    if(rect.Width() && rect.Height())
      ManCoord = new manage_coord(rect);
    }
//  refresh();
//  click(VK_F9);
//  PostMessage(*this, WM_MYHOTKEY, IDC_BUTTON_F9, 0);
}
//----------------------------------------------------------------------------
static
bool hasFzText(setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  return p && *p;
}
//----------------------------------------------------------------------------
#define CHECK_FZ_ENABLED(btn) \
  if(!hasFzText(sStr, btn)) \
    return 0;
//----------------------------------------------------------------------------
P_Body* TD_LGraph2b::pushedBtn(int idBtn)
{
  switch(idBtn) {

    case ID_F2:
      CHECK_FZ_ENABLED(ID_F2)
      setTimeForTimer();
      return 0;

    case ID_F3:
      CHECK_FZ_ENABLED(ID_F3)
      showOver = !showOver;
      makeLinear();
      invalidateGraph();
      return 0;

    case ID_F4:
      CHECK_FZ_ENABLED(ID_F4)
      setLimits();
      loadSet();
      fillLB();
      makeAllGraph();
      return 0;

    case ID_F5:
      CHECK_FZ_ENABLED(ID_F5)
      addTime(true);
      return 0;

    case ID_F6:
      CHECK_FZ_ENABLED(ID_F6)
      addTime(false);
      return 0;

    case ID_F7:
      CHECK_FZ_ENABLED(ID_F7)
      if(chooseFile()) {
        canUpdate = false;
        makeAllGraph();
        }
      return 0;

    case ID_F8:
      CHECK_FZ_ENABLED(ID_F8)
      chooseGraph();
      fillLB();
      // fall through
    case ID_F9:
      CHECK_FZ_ENABLED(ID_F9)
      canUpdate = true;
      makeAllGraph();
      return 0;

    case ID_F10:
      CHECK_FZ_ENABLED(ID_F10)
      print_Graph();
      return 0;

    case ID_F11:
      CHECK_FZ_ENABLED(ID_F11)
      exportTrend();
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
void TD_LGraph2b::addTime(bool prev)
{
  if(!fixedScale)
    return;
  __int64 delta = RangeSet->to - RangeSet->from;
  if(prev)
    delta = -delta;
  RangeSet->from += delta;
  RangeSet->to += delta;
  showSet();
  firstEnter = true;
  checkInside();
  byChooseFile = false;
}
//----------------------------------------------------------------------------
void TD_LGraph2b::getText(int id, LPTSTR txt, int len)
{
  id += ID_INIT_VAR_EDI;

  HWND hwItem = ::GetDlgItem(*this, id);
  if(hwItem)
    ::SendMessage(hwItem, WM_GETTEXT, WPARAM(len), LPARAM(txt));
}
//----------------------------------------------------------------------------
void TD_LGraph2b::setText(int id, fREALDATA v)
{
//#ifdef USE_PRF_0
  if(usePrph_0) {
    int ndec = getDec(id);
    id += ID_INIT_VAR_EDI;

    HWND hwItem = ::GetDlgItem(*this, id);
    if(hwItem) {
      TCHAR txt[50];
      makeFixedString(v, txt, SIZE_A(txt), ndec);
      SendMessage(hwItem, WM_SETTEXT, 0, LPARAM(txt));
      }
    }
//#else
  else {
    LPCTSTR p = getPageString(id + ID_INIT_VAR_EDI + ADD_INIT_VAR);

    if(p) {
      int idprf;
      int addr;
      int type;
      int norm;
      int dec;
      _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &idprf, &addr, &type, &norm, &dec);

      genericPerif *prf = Par->getGenPerif(idprf);
      if(!prf)
        return;

      prfData data;
      data.lAddr = addr;
      data.typeVar = type;
      data.U.fw = v;
      prf->set(data);
      }
    }
//#endif
}
//----------------------------------------------------------------------------
int TD_LGraph2b::getDec(int id)
{
  LPCTSTR p = getPageString(id + ID_INIT_VAR_EDI + ADD_INIT_VAR);

  if(p) {
    int idprf;
    int addr;
    int type;
    int norm;
    int dec;
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &idprf, &addr, &type, &norm, &dec);

    return dec;
    }
  return 0;
}
//----------------------------------------------------------------------------
#define FILENAME_RANGESET _T("range.set")
//----------------------------------------------------------------------------
void TD_LGraph2b::getFileRange(LPTSTR path)
{
  path[0] = 0;
  LPCTSTR p = getPageString(ID_FILE_TREND_PATH);
  if(!p)
    return;
  TCHAR dir[_MAX_PATH];
  _tsplitpath_s(p, 0, 0, dir, SIZE_A(dir), 0, 0, 0, 0);

  p = getPageString(ID_FILE_RANGE_PATH);
  if(!p)
    p = FILENAME_RANGESET;

  _tcscpy_s(path, _MAX_PATH, getLocalBaseDir());
  if(*path)
    appendPath(path, dir);
  else
    _tcscpy_s(path, _MAX_PATH, dir);
  appendPath(path, p);
}
//----------------------------------------------------------------------------
void TD_LGraph2b::saveSet()
{
  TCHAR file[_MAX_PATH];
  getFileRange(file);
//  if(makePathTrend(file)) {
    P_File s(file, P_CREAT);
    if(s.P_open())
      s.P_write(RangeSet, sizeof(*RangeSet));
//    }
}
//----------------------------------------------------------------------------
void TD_LGraph2b::saveDefSet()
{
  saveSet();
  fillLB();
}
//----------------------------------------------------------------------------
//#define EPSILON ((fREALDATA)0.000001)
#define EPSILON FLT_EPSILON
//----------------------------------------------------------------------------
static bool isSameValue(fREALDATA v1, fREALDATA v2)
{
  return fabs(v1 - v2) < EPSILON;
}
//----------------------------------------------------------------------------
void TD_LGraph2b::fillLB()
{
  ::SendMessage(*LBox[0], LB_RESETCONTENT, 0 ,0);
  TCHAR buff[1000];
  for(int i = 0; i < maxChoose; ++i) {
    TCHAR tMin[50];
    TCHAR tMax[50];
    int type = RangeSet->choose[i];
    int ix = codeToType(type);
    if(ix < 0)
      break;
    int dec = infoStat[ix]->dec;
    const rangeSet* rs = RangeSet;
//    const rangeSet* rs = isSameValue(RangeSet->rMin[ix], RangeSet->rMax[ix]) ? RangeSetOnUse : RangeSet;
    makeFixedString(rs->rMin[ix], tMin, SIZE_A(tMin), dec);
    makeFixedString(rs->rMax[ix], tMax, SIZE_A(tMax), dec);
    fillStr(buff, _T(' '), SIZE_A(buff));
    LPTSTR p = buff + MAX_LEN_DUMMY;
    *p++ = _T('\t');

    copyStr(p, infoStat[ix]->descr, _tcslen(infoStat[ix]->descr));
    p += MAX_DESCR;
    *p++ = _T('\t');

    copyStr(p, tMin, _tcslen(tMin));
    p += MAX_LEN_MINMAX;
    *p++ = _T('\t');

    copyStr(p, tMax, _tcslen(tMax));
    p += MAX_LEN_MINMAX;
    *p++ = _T('\t');
    *p = 0;
    ::SendMessage(*LBox[0], LB_ADDSTRING, 0, (LPARAM)buff);
    }
}
//----------------------------------------------------------------------------
void TD_LGraph2b::calcFixTime()
{
  TCHAR time[20];
  getText(DATE_TO, time, SIZE_A(time));
  switch(fxType) {
    case fByHour:
    default:
      RangeSet->to = _ttoi(time);
      RangeSet->to *= HOUR_TO_I64;
      break;
    case fByMinute:
      RangeSet->to = _ttoi(time);
      RangeSet->to *= MINUTE_TO_I64;
      break;
    case fByFiletime:
      do {
        FILETIME ft;
        unformat_time(time, ft);
        RangeSet->to = MK_I64(ft);
        } while(false);
      break;
    }
  RangeSet->to += RangeSet->from;
}
//----------------------------------------------------------------------------
void TD_LGraph2b::saveSet(int id)
{
  int ix = id -= ID_INIT_VAR_EDI;
  if(USE_FILETIME_4_X == val4X) {

    if(DATE_FROM == ix || TIME_FROM == ix) {
      RangeSet->from = calcTime(DATE_FROM, TIME_FROM, false);
      if(fixedScale)
        calcFixTime();
      }
    else if(DATE_TO == ix || TIME_TO == ix)
      if(fixedScale)
        calcFixTime();
      else
        RangeSet->to = calcTime(DATE_TO, TIME_TO, false);
    }
  else {
    if(VALUE_FROM == ix)
      RangeSet->from = calcVal(VALUE_FROM);
    else if(VALUE_TO == ix)
      RangeSet->to = calcVal(VALUE_TO);
    }
  saveSet();
  canUpdate = false;
}
//----------------------------------------------------------------------------
LRESULT TD_LGraph2b::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      if(dontRedraw)
        return 0;
      break;
    case WM_DESTROY:
      if(idTimerRefresh) {
        KillTimer(*this, idTimerRefresh);
        idTimerRefresh = 0;
        }
      break;
    case WM_TIMER:
      if(idTimerRefresh == wParam) {
        KillTimer(*this, idTimerRefresh);
        idTimerRefresh = 0;
        makeAllGraph();
        }
      break;
    case WM_RBUTTONUP:
      do {
        POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
        showCurrValue(pt);
        } while(false);
      break;
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case EN_CHANGE:
          canUpdate = false;
          break;

//#ifdef USE_PRF_0
        case EN_KILLFOCUS:
          if(usePrph_0) {
            if(SendMessage((HWND)lParam, EM_GETMODIFY, 0, 0)) {
              saveSet(LOWORD(wParam));
              checkInside();
              showSet();
//            canUpdate = true;
              }
            }
          break;
//#endif
        case LBN_SELCHANGE:
          if(1 == maxChoose || showOver) {
            PRect rAll;
            GetClientRect(*this, rAll);
            PRect rect = getCoordGraph();
            rAll.right = rect.left;
            InvalidateRect(*this, rAll, 0);
            }
          break;
        }

      break;
    }
  if(ManCoord && showOver && fixedScale) {
    LRESULT res = ManCoord->windowProc(hwnd, message, wParam, lParam);
    switch(res) {
      case 0:
        dontRedraw = false;
        break;
      case -1:
        dontRedraw = true;
        break;
      case 1:
        if(idTimerRefresh)
          SendMessage(*this, WM_TIMER, idTimerRefresh, 0);
        PRect rect = ManCoord->getRect();
        rect.Normalize();
        showZoom(rect);
        dontRedraw = false;
        PostMessage(*getParent(), WM_MYHOTKEY, IDC_BUTTON_F9, 0);
        break;
      }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static double reverseVal(lgCoord vY, fREALDATA vMin, fREALDATA vMax)
{
  if(MAX_H_DATA <= vY)
    return vMin;
  if(1 >= vY)
    return vMax;

  double val = MAX_H_DATA - vY;
  double range = vMax - vMin;
  val *= range;
  val /= MAX_H_DATA;
  val += vMin;
  return val;
}
//----------------------------------------------------------------------------
void TD_LGraph2b::showCurrValue(POINT pt)
{
  PRect r = getCoordGraph();
//  MapWindowPoints(HWND_DESKTOP, *this, &pt, 1);
  if(!r.IsIn(pt))
    return;
  pt.x -= r.left;
  double px = pt.x;
  px /= r.Width() + 2;
  infoPointedValue ipv;

  pt.x *= MUL_W_DATA;
  ipv.ft = I64_TO_FT((__int64)((RangeSet->to - RangeSet->from) * px) + RangeSet->from);
  ipv.nValue = maxChoose;
  uint nElem = XCoord.getElem();
  bool found = false;
  uint ix = 0;
  for(; ix < nElem; ++ix) {
    if(pt.x == XCoord[ix]) {
      found = true;
      break;
      }
    if(pt.x < XCoord[ix])
      break;
    }
  if(ix >= nElem)
    return;
  for(int i = 0; i < maxChoose; ++i) {
    int code = RangeSet->choose[i];
    int type = codeToType(code);
    if(type < 0) {
      ipv.nValue = i;
      break;
      }
    LPCTSTR p = getPageString(ID_COLOR_1 + i);
    int dummy;
    ipv.color[i] = getColor(p, &dummy);
    rangeSet* rs = RangeSet;
    if(isSameValue(RangeSet->rMin[type], RangeSet->rMax[type]))
      rs = RangeSetOnUse;
    _tcscpy_s(ipv.name[i], infoStat[type]->descr);
    if(found)
      ipv.value[i] = reverseVal(yGraph[i][ix], rs->rMin[type], rs->rMax[type]);
    else {
      double v1 = reverseVal(yGraph[i][ix], rs->rMin[type], rs->rMax[type]);
      double v2 = v1;
      if(ix)
        v2 = reverseVal(yGraph[i][ix - 1], rs->rMin[type], rs->rMax[type]);
      ipv.value[i] = (v1 + v2) / 2;
      }
    }
  Par->enableDialogFunctionKey();
  lgraph2_ShowValue sv(this, ipv);
  startSimulDialog(sv, this);
  setDirty();
  Par->disableDialogFunctionKey();
}
//----------------------------------------------------------------------------
void TD_LGraph2b::showZoom(PRect& rect)
{
//  return;
  if(!lGraph[0])
    return;

  zoomInfo Z_I;
  Z_I.full = getCoordGraph();
  Z_I.sel = rect;

  Z_I.init = I64_TO_FT(RangeSet->from);
  Z_I.end = I64_TO_FT(RangeSet->to);

  int sel = SendMessage(*LBox[0], LB_GETCURSEL, 0 ,0);
  if(sel < 0)
    sel = 0;

  Z_I.ixActive = sel;
  uint nShow = SendMessage(*LBox[0], LB_GETCOUNT, 0 ,0);
  Z_I.infoVar.setDim(nShow);
  for(uint i = 0; i < nShow; ++i) {
    int ix_ = RangeSet->choose[i];
    ix_ = codeToType(ix_);
    if(ix_ < 0)
      ix_ = 0;
    const rangeSet* rs = isSameValue(RangeSet->rMin[ix_], RangeSet->rMax[ix_]) ? RangeSetOnUse : RangeSet;

    Z_I.infoVar[i].minVal = rs->rMin[ix_];
    Z_I.infoVar[i].maxVal = rs->rMax[ix_];
    LPCTSTR p = getPageString(ID_COLOR_1 + i);
    int tick;
    Z_I.infoVar[i].frg = getColor(p, &tick);
    Z_I.infoVar[i].sourceY = &yGraph[i];
    }
  do {
    LPCTSTR p = getPageString(ID_NUM_LABEL_ROW);
    Z_I.Dec = 0;
    Z_I.nLabel = 0;
    while(p) {
      Z_I.nLabel = _ttoi(p);
      if(!p)
        break;
      p = findNextParam(p, 1);
      if(p)
        Z_I.Dec = _ttoi(p);
      break;
      }

    Z_I.bkg = RGB(200, 200, 200);
    p = getPageString(ID_INIT_SIMPLE_PANEL);
    p = findNextParam(p, 5);
    if(p) {
      int r = 200;
      int g = 200;
      int b = 200;
      _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
      Z_I.bkg = RGB(r, g, b);
      }

    } while(false);

  Z_I.textColor = textColor;

//  Z_I.totPoint = lGraph[0]->getNPoints();
  Z_I.totPoint = XCoord.getElem();
//  Z_I.sourceY = &yGraph[sel];
  Z_I.sourceX = &XCoord;
  Z_I.hFont = Font.getElem() ? Font[0] : 0;

#if 1
    Par->enableDialogFunctionKey();
    lgraph2_zoom lZ(this, Z_I);
    startSimulDialog(lZ, this);
    setDirty();
    Par->disableDialogFunctionKey();
#else
  lgraph2_zoom(this, Z_I).modal();
#endif
}
//----------------------------------------------------------------------------
void TD_LGraph2b::postSendEdit(PVarEdit* edi)
{
/**/
  uint id = edi->getId();
  saveSet(id);
  if(fixedScale && id != DATE_TO + ID_INIT_VAR_EDI)
    saveSet(DATE_TO + ID_INIT_VAR_EDI);
  checkInside();
//  canUpdate = true;
/**/
 }
//----------------------------------------------------------------------------
void extractDatetime(FILETIME& date, FILETIME& time, __int64 ift)
{
  SYSTEMTIME st;
  FILETIME ft = I64_TO_FT(ift);
  FileTimeToSystemTime(&ft, &st);

  SYSTEMTIME stTime = st;
  stTime.wYear = 1601;
  stTime.wMonth = 1;
  stTime.wDay = 1;
  SystemTimeToFileTime(&stTime, &time);
  st.wHour = 0;
  st.wMinute = 0;
  st.wSecond = 0;
  st.wMilliseconds = 0;
  SystemTimeToFileTime(&st, &date);
}
//------------------------------------------------------------
void TD_LGraph2b::setDateTime(__int64 ift, int ctrlDate, int ctrlTime)
{
  if(DATE_TO == ctrlDate && canUpdate)
    ift = 0;

//#ifdef USE_PRF_0
  if(usePrph_0) {
    TCHAR buff[50];
    set_format_data(buff, SIZE_A(buff), I64_TO_FT(ift), whichData(), _T("|"));
    buff[10] = 0;
    int len = _tcslen(buff);
    SET_TEXT(ctrlDate + ID_INIT_VAR_EDI, buff);
    SET_TEXT(ctrlTime + ID_INIT_VAR_EDI, buff + len + 1);
    }
//#else
  else {
    FILETIME date;
    FILETIME time;
    extractDatetime(date, time, ift);

    LPCTSTR p = getPageString(ctrlDate + ID_INIT_VAR_EDI + ADD_INIT_VAR);

    if(p) {
      int idprf;
      int addr;
      int type;
      int norm;
      int dec;
      _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &idprf, &addr, &type, &norm, &dec);

      genericPerif *prf = Par->getGenPerif(idprf);
      if(!prf)
        return;

      prfData data;
      data.lAddr = addr;
      data.typeVar = type;
      data.U.ft = date;
      prf->set(data);
      }

    p = getPageString(ctrlTime + ID_INIT_VAR_EDI + ADD_INIT_VAR);

    if(p) {
      int idprf;
      int addr;
      int type;
      int norm;
      int dec;
      _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &idprf, &addr, &type, &norm, &dec);

      genericPerif *prf = Par->getGenPerif(idprf);
      if(!prf)
        return;

      prfData data;
      data.lAddr = addr;
      data.typeVar = type;
      data.U.ft = time;
      prf->set(data);
      }
    }
//#endif
}
//------------------------------------------------------------
void TD_LGraph2b::setRangeVal(__int64 iVal, int ctrl)
{
  if(VALUE_TO == ctrl && canUpdate)
    iVal = 0;
  fREALDATA v = CAST_I64_TO_fREAL(iVal);
  setText(ctrl, v);
}
//------------------------------------------------------------
void TD_LGraph2b::setFixedTime()
{
  __int64 iVal = RangeSet->to - RangeSet->from;
  switch(fxType) {
    case fByHour:
    default:
      iVal /= HOUR_TO_I64;
      if(iVal <= 0)
        iVal = 12;
      break;
    case fByMinute:
      iVal /= MINUTE_TO_I64;
      if(iVal <= 0)
        iVal = 12;
      break;
    case fByFiletime:
      break;
    }
//#ifdef USE_PRF_0
  if(usePrph_0) {
    TCHAR buff[50];
    if(fByFiletime == fxType)
      set_format_time(buff, SIZE_A(buff), I64_TO_FT(iVal));
    else
      _stprintf_s(buff, SIZE_A(buff), _T("%d"), (DWDATA)iVal);
    SET_TEXT(DATE_TO + ID_INIT_VAR_EDI, buff);
    }
//#else
  else {
    LPCTSTR p = getPageString(DATE_TO + ID_INIT_VAR_EDI + ADD_INIT_VAR);

    if(p) {
      int idprf;
      int addr;
      int type;
      int norm;
      int dec;
      _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &idprf, &addr, &type, &norm, &dec);

      genericPerif *prf = Par->getGenPerif(idprf);
      if(!prf)
        return;

      prfData data;
      data.lAddr = addr;
      data.typeVar = type;
      if(fByFiletime == fxType)
        data.U.ft = I64_TO_FT(iVal);
      else
        data.U.dw = (DWDATA)iVal;
      prf->set(data);
      }
    }
//#endif
}
//------------------------------------------------------------
void TD_LGraph2b::showSet()
{
  if(USE_FILETIME_4_X == val4X) {
    setDateTime(RangeSet->from, DATE_FROM, TIME_FROM);
    if(fixedScale)
      setFixedTime();
    else
      setDateTime(RangeSet->to, DATE_TO, TIME_TO);
    }
  else {
    setRangeVal(RangeSet->from, VALUE_FROM);
    setRangeVal(RangeSet->to, VALUE_TO);
    }
}
//----------------------------------------------------------------------------
bool TD_LGraph2b::makePathTrend(LPTSTR path, LPCTSTR file, bool history)
{
  path[0] = 0;
  LPCTSTR p = getPageString(ID_FILE_TREND_PATH);
  if(!p)
    return false;
  TCHAR dir[_MAX_PATH];
  _tsplitpath_s(p, 0, 0, dir, SIZE_A(dir), 0, 0, 0, 0);

  bool local = false;
  p = getPageString(ID_USE_LOCAL_PATH_FOR_TREND);
  if(p)
    local = toBool(_ttoi(p));
  if(history) {
    if(local)
      getRelPath(path, dHistory);
    else
      getPath(path, dHistory);
    }
  else {
    if(!local)
      _tcscpy_s(path, _MAX_PATH, getBaseDir());
    }

  appendPath(path, dir);
  appendPath(path, file);
  return true;
}
//----------------------------------------------------------------------------
bool TD_LGraph2b::makePathTrend(LPTSTR path, bool history)
{
  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, SIZE_A(file), path);
  return makePathTrend(path, file, history);
}
//----------------------------------------------------------------------------
void TD_LGraph2b::loadSet()
{
  memset(RangeSet, 0, sizeof(*RangeSet));
  TCHAR file[_MAX_PATH];
  getFileRange(file);

//  if(makePathTrend(file)) {
    P_File s(file, P_READ_ONLY);
    if(s.P_open())
      s.P_read(RangeSet, sizeof(*RangeSet));

    for(uint i = 0; i < SIZE_A(RangeSet->rMin); ++i) {
      if(!isSameValue(RangeSet->rMin[i], RangeSet->rMax[i])) {
        RangeSetOnUse->rMin[i] = 0;
        RangeSetOnUse->rMax[i] = 0;
        }
      }
//    }
}
//--------------------------------------------------------------------
PRect TD_LGraph2b::getCoordGraph()
{
  LPCTSTR p = getPageString(ID_INIT_SIMPLE_PANEL);

  if(!p)
    return PRect(0, 0, 0, 0);

  int x;
  int y;
  int w;
  int h;

  _stscanf_s(p, _T("%d,%d,%d,%d"), &x, &y, &w, &h);

  x += Offset.x;
  y += Offset.y;
#if 1
  PRect r(0, 0, R__X(w), R__Y(h));
  r.MoveTo(R__X(x), R__Y(y));
#else
  PRect r(R__X(x), R__Y(y), R__X(x + w), R__Y(y + h));
#endif
  r.Inflate(-2, -2);
  return r;
}
//--------------------------------------------------------------------
void TD_LGraph2b::makeLinear()
{
  PRect rect = getCoordGraph();

  if(!rect.Width() || !rect.Height())
    return;

  int tick[MAX_LINEAR_GRAPH];
  COLORREF col[MAX_LINEAR_GRAPH];
  for(int i = 0; i < MAX_LINEAR_GRAPH; ++i) {
    LPCTSTR p = getPageString(ID_COLOR_1 + i);
    col[i] = getColor(p, &tick[i]);
    }

  int hGraph = rect.Height();
  if(maxChoose) {
    hGraph /= maxChoose;
//    rect.bottom = rect.top + hGraph * maxChoose;
    }

  int n = 0;
  if(lGraph[0])
    n = lGraph[0]->getNPoints();
  for(int j = 0; j < MAX_LINEAR_GRAPH; ++j) {
    delete lGraph[j];
    lGraph[j] = 0;
    }

  PRect r(rect);
  if(!showOver)
    r.bottom = r.top + hGraph;

  for(int j = 0; j < maxChoose; ++j) {
    lGraph[j] = new linear(r, 0, PPanel::NO, col[j], !j || !showOver);
    lGraph[j]->setTick(tick[j]);
    if(!showOver)
      r.Offset(0, hGraph);
    }

  for(int j = 0; j < maxChoose; ++j)
    lGraph[j]->setBuff(&XCoord, &yGraph[j], n);
}
//----------------------------------------------------------------------------
void TD_LGraph2b::getFileStr(LPTSTR path)
{
  getFileStrCheckRemote(path, NAME_STR);
//  _tcscpy(path, NAME_STR);
//  getPath(path, dSystem);
}
//----------------------------------------------------------------------------
static bool checkDay(const FILETIME& ft, WORD& curr)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  bool changed = curr != st.wDay;
  curr = st.wDay;
  return changed;
}
//----------------------------------------------------------------------------
void TD_LGraph2b::refreshBody()
{
  baseClass::refreshBody();
  if(dontRedraw)
    return;
  if(firstEnter) {
    if(Edi.getElem()) {
      firstEnter = false;
      postSendEdit(Edi[0]);
      makeAllGraph();
      PostMessage(*getParent(), WM_MYHOTKEY, IDC_BUTTON_F9, 0);
//      if(firstInvalidate)
        canUpdate = toBool(fixedScale);
      }
    }

  if(fixedScale) {
    if(!insideTime)
      checkInside();
    else if(insideTime) {
      bool upd = canUpdate;
      if(mustResetEdit())
        canUpdate = true;
      __int64 curr = upd || !mustResetEdit() ? MK_I64(getFileTimeCurr()) : RangeSet->to + 1;
      if(curr >= RangeSet->to) {
        onlyRefresh = upd;
        if(upd) {
          makeInside();
          showSet();
          }
        makeAllGraph();
        onlyRefresh = false;
        return;
        }
      }
    }

  if(!canUpdate)// || fixedScale)
    return;

  if(!*fileName)
    return;

  WIN32_FIND_DATA fd;

  int nElem = List.getElem();
  HANDLE hf;
  if(!nElem)
    hf = FindFirstFile(fileName, &fd);
  else
    // verifica sull'ultimo file della lista
    hf = FindFirstFile(List[nElem - 1]->name, &fd);
  if(INVALID_HANDLE_VALUE == hf)
    return;

  FindClose(hf);
  if(lastUpdate < fd.ftLastWriteTime)
    lastUpdate = fd.ftLastWriteTime;
  // se è cambiato il file ed è più vecchio, inutile controllare
  else if(lastUpdate > fd.ftLastWriteTime) {
    canUpdate = false;
    return;
    }

  if(fixedScale) {
    __int64 ft = MK_I64(getFileTimeCurr());
    if(ft < RangeSet->from || ft > RangeSet->to) {
      canUpdate = false;
      return;
      }
#if 1
    bool changedDay = false;
    if((WORD)-1 == currDay) {
      checkDay(I64_TO_FT(ft), currDay);
      return;
      }
    else
      changedDay = checkDay(I64_TO_FT(ft), currDay);
    if(maxWaitBeforeRefresh) {
      if(++count4refresh >= maxWaitBeforeRefresh) {
        count4refresh = 0;
//        canUpdate = true;
        onlyRefresh = !changedDay;
        makeAllGraph();
        onlyRefresh = false;
        }
      }
    else {
      if(changedDay) {
        onlyRefresh = false;
        makeAllGraph();
        }
      else
        addLastToVect();
      }
#else
    if(++count4refresh >= maxWaitBeforeRefresh) {
      count4refresh = 0;
//      canUpdate = true;
      onlyRefresh = true;
      makeAllGraph();
      onlyRefresh = false;
      }
#endif
    }
  else if(XCoord.getElem() > 0)
    addLastToVect();
}
//----------------------------------------------------------------------------
__int64 TD_LGraph2b::calcVal(int ctrl)
{
  TCHAR value[20];
  getText(ctrl, value, SIZE_A(value));
  fREALDATA v = (fREALDATA)_tstof(value);
  __int64 t = CAST_fREAL_TO_I64(v);
  return t;
}
//----------------------------------------------------------------------------
__int64 TD_LGraph2b::calcTime(int ctrlDate, int ctrlTime, bool useCurrTime)
{
  TCHAR date[20];
  TCHAR time[20];
  getText(ctrlDate, date, SIZE_A(date));
  getText(ctrlTime, time, SIZE_A(time));
  if(!*date || *date == _T(' '))
    if(useCurrTime)
      return MK_I64(getFileTimeCurr());
    else {
//      FILETIME ft;
//      ZeroMemory(&ft, sizeof(ft));
//      return MK_I64(ft);
      return 0;
      }
  SYSTEMTIME st;
  ZeroMemory(&st, sizeof(st));
  int day;
  int month;
  int year;
  switch(whichData()) {
    case European:
      day = _ttoi(date);
      month = _ttoi(date + 3);
      year = _ttoi(date + 6);
      break;
    case American:
      month = _ttoi(date);
      day = _ttoi(date + 3);
      year = _ttoi(date + 6);
      break;
    case Other:
      year = _ttoi(date);
      month = _ttoi(date + 5);
      day = _ttoi(date + 8);
      break;
    }
  st.wYear = WORD(year);
  st.wMonth = WORD(month);
  st.wDay = WORD(day);
  if(*time && *time != _T(' ')) {
    st.wHour = WORD(_ttoi(time));
    LPTSTR p = time;
    while(*p && _istdigit((unsigned)*p))
      ++p;
    if(*p) {
      while(*p && !_istdigit((unsigned)*p))
        ++p;
      if(*p) {
        st.wMinute = WORD(_ttoi(p));
        while(*p && _istdigit((unsigned)*p))
          ++p;
        if(*p) {
          while(*p && !_istdigit((unsigned)*p))
            ++p;
          if(*p)
            st.wSecond = WORD(_ttoi(p));
          }
        }
      }
    }
  FILETIME ft;
  SystemTimeToFileTime(&st, &ft);
  return MK_I64(ft);
}
//--------------------------------------------------------------------
__int64 TD_LGraph2b::calcInitEnd(__int64& Init)
{
  __int64 End;
  Init = RangeSet->from;
  End = RangeSet->to;
  if(USE_FILETIME_4_X == val4X) {
//    Init = calcTime(DATE_FROM, TIME_FROM);
//    End = calcTime(DATE_TO, TIME_TO);
    return End - Init;
    }
  else {
//    Init = calcVal(VALUE_FROM);
//    End = calcVal(VALUE_TO);
    fREALDATA t1 = CAST_I64_TO_fREAL(End);
    fREALDATA t2 = CAST_I64_TO_fREAL(Init);
    t1 -= t2;
    return CAST_fREAL_TO_I64(t1);
    }

}
//--------------------------------------------------------------------
bool TD_LGraph2b::reCalcInit(LPCBYTE buffStat, __int64& Init)
{
  bool modified = false;
  if(USE_FILETIME_4_X == val4X) {
    if((!fixedScale || !Init) && CAST_I64(buffStat) != Init) {
      Init = CAST_I64(buffStat);
      modified = true;
      }
    }
  else {
    fREALDATA t = getXVal(buffStat);
    fREALDATA rInit = CAST_I64_TO_fREAL(Init);
    if(t != rInit) {
      Init = CAST_fREAL_TO_I64(t);
      modified = true;
      }
    }
  if(modified)
    RangeSet->from = Init;
  return modified;
}
//--------------------------------------------------------------------
bool TD_LGraph2b::reCalcEnd(LPCBYTE buffStat, __int64& End)
{
  if(fixedScale)
    return false;

  bool modified = false;
//  if(End)
//    modified = true;
//  else {
    if(USE_FILETIME_4_X == val4X) {
      if(!End || CAST_I64(buffStat) < End) {
        End = CAST_I64(buffStat);
        modified = true;
        }
      }
    else {
      fREALDATA t = getXVal(buffStat);
      fREALDATA rEnd = CAST_I64_TO_fREAL(End);
      if(!rEnd || t < rEnd) {
        End = CAST_fREAL_TO_I64(t);
        modified = true;
        }
      }
//    }
//  if(modified)
    RangeSet->to = End;
  return modified;
}
//--------------------------------------------------------------------
bool TD_LGraph2b::reCalcEnd(P_File& pf, __int64& End)
{
  LPBYTE buffStat = new BYTE[sizeStat];
  memset(buffStat, 0, sizeStat);
  DWORD pos = (DWORD)pf.get_pos();
  pf.P_seek(-long(sizeStat), SEEK_END_);
  pf.P_read(buffStat, sizeStat);
  pf.P_seek(pos);

  bool success = reCalcEnd(buffStat, End);
  delete []buffStat;
  return success;
}
//--------------------------------------------------------------------
void TD_LGraph2b::calcLastTot()
{
  if(USE_FILETIME_4_X == val4X) {
    if(!RangeSet->to)
      lastTotVal = (double)(MK_I64(getFileTimeCurr()) - RangeSet->from);
    else
      lastTotVal = (double)(RangeSet->to - RangeSet->from);
      }
  else {
    fREALDATA t =  CAST_I64_TO_fREAL(RangeSet->to);
    t -=  CAST_I64_TO_fREAL(RangeSet->from);
    lastTotVal = t; //CAST_fREAL_TO_I64(t);
    }
}
//--------------------------------------------------------------------
void TD_LGraph2b::reCalcInitEnd(P_File& pf, __int64& Init, __int64& End, LPCBYTE buffStat)
{
  bool modified = reCalcInit(buffStat, Init);

  modified |= reCalcEnd(pf, End);

  if(modified) {
    if(USE_FILETIME_4_X == val4X) {
      setDateTime(Init, DATE_FROM, TIME_FROM);
      // non dovrebbe essere necessario il controllo per fixedScale
      // init e end non dovrebbero cambiare in base al file
      setDateTime(End, DATE_TO, TIME_TO);
      }
    else {
      setRangeVal(Init, VALUE_FROM);
      setRangeVal(End, VALUE_TO);
      }
    saveSet();
    calcLastTot();
    }
}
//----------------------------------------------------------------------------








#define DEF_GROUP_DATE 4
#define MAX_GROUP_DATE 7
#define N_INFO_DATE 12
#define N_TOT_INFO_DATE (N_INFO_DATE * nGroupDate)
//#define H_LINE1 10
//#define H_LINE2 25
//#define H_LINE_TXT 30
#define H_LINE1 5
#define H_LINE2 8
#define H_LINE_TXT (H_LINE2 + 0)
#define DIM_F 15
//----------------------------------------------------------------------------
void TD_LGraph2b::evPaint(HDC hdc, const PRect& Rect)
{
  baseClass::evPaint(hdc, Rect);
  InfoFile->draw(hdc, Rect);
/**/
  if(lGraph[0]) {

    PRect rect = getCoordGraph();

    int y_base = rect.bottom + 2;
    int y_line1 = rect.bottom + R__Y(H_LINE1);
    int y_line2 = rect.bottom + R__Y(H_LINE2);
    int y_txt = rect.bottom + R__Y(H_LINE_TXT);
    int y_txt2 = y_txt + R__Y(DIM_F - 2);

    int nGroupDate = DEF_GROUP_DATE;
    if(fixedScale) {
      __int64 iVal = RangeSet->to - RangeSet->from;
      switch(fxType) {
        case fByHour:
        default:
          iVal /= HOUR_TO_I64;
          break;
        case fByMinute:
          iVal /= MINUTE_TO_I64;
          break;
        case fByFiletime:
          do {
            iVal /= SECOND_TO_I64;
            while(iVal > 60)
              iVal /= 60;
            } while(false);
          break;
        }

      if(iVal < DEF_GROUP_DATE)
        iVal = DEF_GROUP_DATE;
      else if(iVal > MAX_GROUP_DATE)
        iVal = MAX_GROUP_DATE;
      nGroupDate = (int)iVal;
/*
      nGroupDate += (N_INFO_DATE - 1);
      nGroupDate /= N_INFO_DATE;
      nGroupDate *= N_INFO_DATE;
*/
      }

    HPEN p1 = CreatePen(PS_SOLID, 1, textColor);
    HPEN p2 = CreatePen(PS_SOLID, 1, RGB(160, 160, 160));

    HPEN pBlack = CreatePen(PS_SOLID, 1, 0);

    HGDIOBJ oldPen = SelectObject(hdc, p1);
    int x1 = rect.left;
    __int64 ftInit;
    __int64 tmp = calcInitEnd(ftInit);
    double step;

    if(USE_FILETIME_4_X == val4X)
      step = (double)tmp;
    else
      step = CAST_I64_TO_fREAL(tmp);

    if(step < 0)
      step = 0;
    step /= N_TOT_INFO_DATE / N_INFO_DATE;
    REALDATA fixX = (REALDATA)rect.Width() / (REALDATA)(N_TOT_INFO_DATE - 1);
    HGDIOBJ oldFont = 0;
    if(Font.getElem())
      oldFont = SelectObject(hdc, Font[0]);

    int oldMode = SetBkMode(hdc, TRANSPARENT);

    UINT oldAlign = SetTextAlign(hdc, TA_LEFT | TA_TOP);

    TCHAR t[50];
    LPCTSTR sep = _T("|");

    int dec = getDec(VALUE_FROM);

    SetTextColor(hdc, textColor);
    SetTextAlign(hdc, TA_CENTER | TA_TOP);
    for(int i = 0; i < N_TOT_INFO_DATE; ++i) {
      int x = (int)(fixX * i + x1);
      if(!(i % N_INFO_DATE)) {
//        if(!i)
//          SetTextAlign(hdc, TA_LEFT | TA_TOP);
//        else
          {
          SelectObject(hdc, p2);
          MoveToEx(hdc, x, rect.top + 1, 0);
          LineTo(hdc, x, rect.bottom - 1);
          SelectObject(hdc, p1);
//          SetTextAlign(hdc, TA_CENTER | TA_TOP);
          }
        if(USE_FILETIME_4_X == val4X) {
          bool drawDate = true;
          if(nGroupDate > DEF_GROUP_DATE)
            drawDate = !i || !((i / N_INFO_DATE) % (nGroupDate / DEF_GROUP_DATE));
          if(drawDate) {
            set_format_data(t, SIZE_A(t), I64_TO_FT(ftInit), whichData(), sep);
            t[10] = 0;
            int len = _tcslen(t);
            TextOut(hdc, x, y_txt, t, len);
            LPTSTR t2 = t + len + 1;
            TextOut(hdc, x, y_txt2, t2, _tcslen(t2));
            }
          ftInit += (__int64)step;
          }
        else {
          makeFixedString(CAST_I64_TO_fREAL(ftInit), t, SIZE_A(t), dec);
          int len = _tcslen(t);
          TextOut(hdc, x, y_txt, t, len);
          CAST_I64_TO_fREAL(ftInit) += (fREALDATA)step;
          }
        SelectObject(hdc, pBlack);
        MoveToEx(hdc, x, y_base, 0);
        LineTo(hdc, x, y_line2);
        SelectObject(hdc, p1);
        }
      else {
        MoveToEx(hdc, x, y_base, 0);
        LineTo(hdc, x, y_line1);
        }
      }
    int x = rect.right;
    SelectObject(hdc, pBlack);
    MoveToEx(hdc, x, y_base, 0);
    LineTo(hdc, x, y_line2);
    SetTextAlign(hdc, TA_RIGHT | TA_TOP);
    SelectObject(hdc, p1);

    GetClientRect(*this, rect);
    x = rect.right - 6;

    if(USE_FILETIME_4_X == val4X) {
      set_format_data(t, SIZE_A(t), I64_TO_FT(ftInit), whichData(), sep);
      t[10] = 0;
      int len = _tcslen(t);
      TextOut(hdc, x, y_txt, t, len);
      LPTSTR t2 = t + len + 1;
      TextOut(hdc, x, y_txt2, t2, _tcslen(t2));
      }
    else {
      makeFixedString(CAST_I64_TO_fREAL(ftInit), t, SIZE_A(t), dec);
      int len = _tcslen(t);
      TextOut(hdc, x, y_txt, t, len);
      }
    SelectObject(hdc, oldPen);
    SetTextAlign(hdc, oldAlign);
    DeleteObject(pBlack);
    DeleteObject(p2);
    DeleteObject(p1);
    SetBkMode(hdc, oldMode);

    if(1 == maxChoose || showOver)
      drawYInfo(hdc);
    for(int i = 0; i < maxChoose; ++i) {
      lGraph[i]->rewind();
      lGraph[i]->draw(hdc);
      }
    if(oldFont)
      SelectObject(hdc, oldFont);
    }
/**/
}
//--------------------------------------------------------------------
void TD_LGraph2b::drawYInfo(HDC hdc)
{
#if 1
  int sel = SendMessage(*LBox[0], LB_GETCURSEL, 0 ,0);
  if(sel < 0)
    sel = 0;
  sel = RangeSet->choose[sel];
  int ix_ = codeToType(sel);
  if(ix_ < 0)
     ix_ = 0;

#else
  const uint ix_ = 0;
#endif
  const rangeSet* rs = isSameValue(RangeSet->rMin[ix_], RangeSet->rMax[ix_]) ? RangeSetOnUse : RangeSet;
  fREALDATA range = rs->rMax[ix_] - rs->rMin[ix_];
  if(range <= 0)
    return;

  int type = 0;
  fREALDATA nRow = DEF_TYPED_ROW;
  fREALDATA round = 0;
  LPCTSTR p = getPageString(ID_SET_TYPE_ROW);
  if(p) {
    type = _ttoi(p);
    p = findNextParam(p, 1);
    if(p) {
      nRow = (fREALDATA)_tstof(p);
      p = findNextParam(p, 1);
      if(p)
        round = (fREALDATA)_tstof(p);
      }
    }
  if(nRow <= 0)
    return;
  PRect rect = getCoordGraph();
  fREALDATA deltaY;
  fREALDATA dyVal = range / nRow;
  int hGraph = rect.Height();
  switch(type) {
    case 0:
      nRow = dyVal;
      if(round > 0) {
        if(dyVal > round) {
          fREALDATA t = round;
          while(round < dyVal)
            round += t;
          }
        nRow = round;
        }
//      deltaY = nRow * (float)hGraph / range;
//      break;
    case 1:
      deltaY = nRow * (float)hGraph / range;
      break;
    default:
      return;
    }
  if(!deltaY || !nRow)
    return;
  fREALDATA offsY = 0;
  if(rs->rMin[0] < 0)
    offsY = -rs->rMin[ix_] * (float)hGraph / range;
  else {
    fREALDATA t = nRow - rs->rMin[ix_];
    offsY = deltaY * t / nRow;
    }
  if(offsY < 0)
    offsY = 0;
//  HPEN pen = CreatePen(PS_DASH, 1, RGB(0xff, 0xff, 0xff));
  HPEN pen = CreatePen(PS_DASH, 1, RGB(160, 160, 160));
  HGDIOBJ oldPen = SelectObject(hdc, pen);
  int oldMode = SetBkMode(hdc, TRANSPARENT);
  int x1 = rect.left;
  int x2 = rect.right;
  for(fREALDATA y = offsY + rect.top; y < rect.bottom; y += deltaY) {
    MoveToEx(hdc, x1, (int)y, 0);
    LineTo(hdc, x2, (int)y);
    }

  for(fREALDATA y = offsY + rect.top - deltaY; y > rect.top; y -= deltaY) {
    MoveToEx(hdc, x1, (int)y, 0);
    LineTo(hdc, x2, (int)y);
    }
  p = getPageString(ID_NUM_LABEL_ROW);
  while(p) {
    int nLabel = _ttoi(p);
    if(!p)
      break;
    p = findNextParam(p, 1);
    int dec = 0;
    if(p)
      dec = _ttoi(p);
    HPEN pBlack = CreatePen(PS_SOLID, 1, textColor);
//    HPEN pBlack = CreatePen(PS_SOLID, 1, 0);
    SelectObject(hdc, pBlack);
    SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
    fREALDATA stepVal = rs->rMax[ix_] - rs->rMin[ix_];
    stepVal /= nLabel;
    fREALDATA step = (fREALDATA)rect.Height();
    step /= nLabel;
    fREALDATA val = rs->rMin[ix_];
    fREALDATA y = (fREALDATA)rect.bottom;
    x1 = 1;
    x2 = rect.left - R__X(5);

    for(int i = 0; i < nLabel; ++i) {
      MoveToEx(hdc, x1, (int)y, 0);
      LineTo(hdc, x2, (int)y);
      TCHAR buff[50];
      makeFixedString(val, buff, SIZE_A(buff), dec);
      TextOut(hdc, x2, (int)y, buff, _tcslen(buff));
      y -= step;
      val += stepVal;
      }
    DeleteObject(pBlack);
    break;
    }

  SelectObject(hdc, oldPen);
  DeleteObject(pen);
  SetBkMode(hdc, oldMode);
}
//--------------------------------------------------------------------
int TD_LGraph2b::codeToType(int code)
{
  int nElem = infoStat.getElem();
  if(code > 0 && code <= nElem)
    return code - 1;
  return -1;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
void TD_LGraph2b::chooseGraph()
{
  bool ok = TD_Choose(this, sStr, RangeSet->choose).modal() == IDOK;

  if(ok) {
    for(maxChoose = 0; maxChoose < MAX_LINEAR_GRAPH; ++ maxChoose)
      if(!RangeSet->choose[maxChoose])
        break;
#if 1
    for(int i= 0; i < MAX_LINEAR_GRAPH; ++i)
      if(lGraph[i])
        lGraph[i]->reset();
      else
        break;
#endif
    saveSet();
    makeLinear();
    }
}
//--------------------------------------------------------------------
#define MAX_fREALDATA 10e10f
static void getMinMax(fREALDATA *v, int n, fREALDATA& min, fREALDATA& max)
{
  fREALDATA vMin = MAX_fREALDATA;
  fREALDATA vMax = 0.0;
  statist<fREALDATA> stat(vMin, vMax);
  for(int i = 0; i < n; ++i)
    stat.calc(v[i]);
  min = stat.getMin();
  max = stat.getMax();
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
#define MAX_RECORD_TO_LOAD 10000
//--------------------------------------------------------------------
/*
void TD_LGraph2b::reload()
{
  if(!*fileName)
    return;

  P_File f(fileName, P_READ_ONLY);

  if(f.P_open()) {

//    LPBYTE buffStat = new BYTE[sizeStat];
    memset(BuffStat, 0, sizeStat);

    DWORD nRec = (DWORD)f.get_len() - sizeHeader;
    nRec /= sizeStat;

    DWORD ix = 0;
//    if(nRec > MAX_RECORD_TO_LOAD)
//      ix = nRec - MAX_RECORD_TO_LOAD;

    f.P_seek(ix * sizeStat + sizeHeader);

//    DWORD ix = findFirstPos(f, RangeSet->from);

    if(f.P_read(BuffStat, sizeStat) != sizeStat) {
//      delete []buffStat;
      return;
      }

    fREALDATA minV = 0.0;
    fREALDATA maxV = 0.0;

    int nElem = infoStat.getElem();
    const int TOT_TYPE = nElem;//6;

    typedef statist<fREALDATA> vStat;
    PVect<vStat*> allStat;

    // alloca già quello che serve
    allStat.setDim(TOT_TYPE);

    fREALDATA* pData = (fREALDATA*)(BuffStat + offsStat);
    for(int i = 0; i < nElem; ++i) {
      int offs = infoStat[i]->offsStatFile;
      getMinMax(pData + offs, 1, minV, maxV);
      allStat[i] = new vStat(minV, maxV);
      }

    BarProgr bar(this, nRec);
    bar.create();
    for(;; ++ix) {
    if(!(ix & 0xf))
      if(!bar.setPos(ix))
        break;
      if(f.P_read(BuffStat, sizeStat) != sizeStat)
        break;

      for(int i = 0; i < nElem; ++i) {
        int offs = infoStat[i]->offsStatFile;
        getMinMax(pData + offs, 1, minV, maxV);
        allStat[i]->calc(minV);
        allStat[i]->calc(maxV);
        }
      }
    for(int i = 0; i < nElem; ++i) {
      RangeSet->rMin[i] = allStat[i]->getMin();
      RangeSet->rMax[i] = allStat[i]->getMax();
      }
    flushPV(allStat);
    saveDefSet();
//    delete []buffStat;
    }
}
*/
//--------------------------------------------------------------------
void TD_LGraph2b::invalidateGraph()
{
  static PRect rAll;
  if(firstInvalidate || !rAll.Height()) {
    firstInvalidate = false;
    GetClientRect(*this, rAll);

    PRect rect = getCoordGraph();
    rAll.top = rect.top;
    rAll.bottom = rect.bottom + R__Y(H_LINE_TXT + DIM_F - 2);

    HGDIOBJ oldFont = 0;
    HDC hdc = GetDC(*this);
    if(Font.getElem())
      oldFont = SelectObject(hdc, Font[0]);

    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    if(oldFont)
      SelectObject(hdc, oldFont);

    ReleaseDC(*this, hdc);
    rAll.bottom += tm.tmHeight;
    }
  InvalidateRect(*this, rAll, 0);
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
bool TD_LGraph2b::findLastTrend(LPTSTR file)
{
  WIN32_FIND_DATA FindFileData;
  HANDLE h = FindFirstFile(file, &FindFileData);
  if(h == INVALID_HANDLE_VALUE)
    return false;
  FILETIME ft;
  TCHAR name[_MAX_PATH] = _T("\0");
  MK_I64(ft) = 0;
  do {
    if(FindFileData.ftLastWriteTime > ft) {
      ft = FindFileData.ftLastWriteTime;
      _tcscpy_s(name, SIZE_A(name), FindFileData.cFileName);
      }
    } while(FindNextFile(h, &FindFileData));

  FindClose(h);

  if(*name)
    return makePathTrend(file, name);
  return false;
}
//--------------------------------------------------------------------
long extractCodeFromText(LPCTSTR buff)
{
  if(!buff)
    return 0;
  int len = _tcslen(buff);
  if(!len)
    return 0;
  LPCTSTR p = buff + len - 1;
  while(*p && *p != _T(','))
    --p;
  if(*p) {
    ++p;
    long idCode = _ttoi(p);
    return idCode;
    }
  return 0;
}
//--------------------------------------------------------------------
bool TD_LGraph2b::isInsideDate(LPCTSTR filename, const FILETIME& ftInit, const FILETIME& ftEnd, bool history)
{
  TCHAR path[_MAX_PATH];
  if(!makePathTrend(path, filename, history))
    return false;
  P_File pf(path, P_READ_ONLY);
  if(!pf.P_open())
    return false;
  if(sizeHeader) {
    BYTE tmp[500];
    pf.P_read(tmp, sizeHeader);
    }
  FILETIME ft;
  pf.P_read(&ft, sizeof(ft));
  if(ftEnd < ft)
    return false;
  pf.P_seek(-(long)sizeStat, SEEK_END_);
  pf.P_read(&ft, sizeof(ft));
  if(ftInit > ft)
    return false;
  return true;
}
//--------------------------------------------------------------------
LPCTSTR findLastSlash(LPCTSTR filename)
{
  int len = _tcslen(filename);
  for(int i = len - 1; i > 0; --i)
    if(_T('\\') == filename[i])
      return filename + i + 1;
  return filename;
}
//--------------------------------------------------------------------
void TD_LGraph2b::insertInListFile(LPCTSTR filename, bool history)
{
  listFile* lf = FreeFile.get();
  makePathTrend(lf->name, filename, history);
  LPCTSTR p = findLastSlash(lf->name);
  int nElem = List.getElem();
  if(!nElem)
    List[0] = lf;
  else {
    int i = 0;
    for(; i < nElem; ++i) {
      LPCTSTR t = findLastSlash(List[i]->name);
      if(_tcsicmp(p, t) <= 0)
        break;
      }
    List.insert(lf, i);
    }
}
//--------------------------------------------------------------------
bool TD_LGraph2b::findMatchTrend()
{
  LPCTSTR p = getPageString(ID_FILE_TREND_PATH);
  if(!p)
    return false;
  TCHAR path[_MAX_PATH] = _T("\0");
  TCHAR ext[_MAX_PATH] = _T("\0");
  _tsplitpath_s(p, 0, 0, 0, 0, path, SIZE_A(path), ext, SIZE_A(ext));
  // si presuppone che il nome sia terminato da '*'
  TCHAR name[_MAX_PATH];
  _tcscpy_s(name, path);
  int len = _tcslen(name);
  if(len) {
    --len;
    name[len] = 0;
    }
#if 1
  _tcscat_s(path, SIZE_A(path), ext);
#endif
  FILETIME ftFrom = I64_TO_FT(RangeSet->from);
  FILETIME ftTo;
  if(RangeSet->to)
    ftTo = I64_TO_FT(RangeSet->to);
  else
    ftTo = getFileTimeCurr();
  for(int i = 0; i < 2; ++i) {
    if(!makePathTrend(path, toBool(i)))
      break;
//    _tcscat(path, p);
    WIN32_FIND_DATA FindFileData;
    HANDLE h = FindFirstFile(path, &FindFileData);
    if(h == INVALID_HANDLE_VALUE)
      continue;
    do {
      if(_T('.') == FindFileData.cFileName[0])
        continue;
      if(_tcsncicmp(name, FindFileData.cFileName, len))
        continue;

/*
      TCHAR ext2[_MAX_PATH] = _T("\0");
      _tsplitpath_s(FindFileData.cFileName, 0, 0, 0, 0, 0, 0, ext2, SIZE_A(ext2));
      if(_tcsicmp(ext, ext2))
        continue;
*/
      if(isInsideDate(FindFileData.cFileName, ftFrom, ftTo, toBool(i)))
        insertInListFile(FindFileData.cFileName, toBool(i));
      } while(FindNextFile(h, &FindFileData));

    FindClose(h);
    path[0] = 0;
    getRelPath(path, dHistory);
    }
  return List.getElem() > 0;
}
//--------------------------------------------------------------------
void TD_LGraph2b::replaceNameInfo()
{
  if(!InfoFile)
    return;
  TCHAR tmp[_MAX_FNAME];
  _tsplitpath_s(fileName, 0, 0, 0, 0, tmp, SIZE_A(tmp), 0, 0);
  InfoFile->set_Text(tmp);
}
//--------------------------------------------------------------------
void TD_LGraph2b::makeAllGraph()
{
  if(!maxChoose)
    return;

  if(!onlyRefresh && !byChooseFile) {
    flushPV_toFree(FreeFile, List);
    if(RangeSet->from) {
      if(findMatchTrend())
        _tcscpy_s(fileName, SIZE_A(fileName), List[0]->name);
      }
    else {
      LPCTSTR p = getPageString(ID_FILE_TREND_PATH);
      if(!p)
        return;
//    if(!makePathTrend(file, p))
//      return;
      _tcscpy_s(fileName, SIZE_A(fileName), p);
      if(!findLastTrend(fileName)) {
        *fileName = 0;
        return;
        }
      listFile* lf = FreeFile.get();
      _tcscpy_s(lf->name, SIZE_A(lf->name), fileName);
      List[0] = lf;
      if(USE_FILETIME_4_X == val4X)
        calcInputDatefromFile();
      }
    replaceNameInfo();
    calcLastTot();
    }
  if(byChooseFile)
    replaceNameInfo();

  XCoord.reset();
  for(int j = 0; j < maxChoose; ++j)
    yGraph[j].reset();

//  calcLastTot();

  int dim = makeVect();

  for(int j = 0; j < maxChoose; ++j)
    lGraph[j]->setBuff(&XCoord, &yGraph[j], dim);

  invalidateGraph();
}
//--------------------------------------------------------------------------
__int64 TD_LGraph2b::getToValue()
{
  if(USE_FILETIME_4_X == val4X)
    if(fixedScale) {
      calcFixTime();
      return RangeSet->to;
      }
    else
      return calcTime(DATE_TO, TIME_TO, false);

  return calcVal(VALUE_TO);
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------
fREALDATA TD_LGraph2b::getXVal(LPCBYTE buff)
{
  buff += offsStat;
  fREALDATA* pBuff = (fREALDATA*)buff;
  return pBuff[val4X];
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
DWORD TD_LGraph2b::calcInitIx(DWORD& ixInit)
{
  DWORD nTotRec = 0;
  int nElem = List.getElem();
  for(int i = 0; i < nElem; ++i) {
    P_File f(List[i]->name, P_READ_ONLY);
    if(!f.P_open())
      continue;
    DWORD nRec = (DWORD)f.get_len() - sizeHeader;
    nRec /= sizeStat;
    if(i == nElem - 1) {
      FILETIME ft;
      DWORD ixEnd;
      if(RangeSet->to) {
        ft = I64_TO_FT(RangeSet->to);
        ixEnd = findFirstPos(f, ft, sizeStat, BuffStat, sizeHeader, false);
        }
      else
        ixEnd = nRec;
      nTotRec += ixEnd;
      }
    else
      nTotRec += nRec;
    if(!i) {
      FILETIME ft = I64_TO_FT(RangeSet->from);
      ixInit = findFirstPos(f, ft, sizeStat, BuffStat, sizeHeader, true);
      nTotRec -= ixInit;
      }
    }
  return nTotRec;
}
//--------------------------------------------------------------------
int TD_LGraph2b::makeVectFull(DWORD nTotRec, DWORD ixInit)
{
  int curr = 0;
  int nElem = List.getElem();

  DWORD ixGlobal = 0;
  BarProgr bar(this, nTotRec);
  if(!onlyRefresh) {
    bar.create();
    }
  __int64 ftEnd = getToValue();
  if(ftEnd && !fixedScale)
    canUpdate = false;
  if(!ftEnd)
    ftEnd = MK_I64(getFileTimeCurr());

  bool stopped = false;
  for(int i = 0; i < nElem && !stopped; ++i) {
    P_File f(List[i]->name, P_READ_ONLY);
    if(!f.P_open())
      continue;

    memset(BuffStat, 0, sizeStat);

//    DWORD nRec = (DWORD)f.get_len() - sizeHeader;
//    nRec /= sizeStat;

//    DWORD ix = 0;
    if(!i)
//      ix = ixInit;
    f.P_seek(ixInit * sizeStat + sizeHeader);
//    f.P_seek(ix * sizeStat + sizeHeader);
    f.P_read(BuffStat, sizeStat);

    bool foundGreat = false;
    for(;;++ixGlobal) {
      if(!onlyRefresh && !(ixGlobal & 0xf))
        if(!bar.setPos(ixGlobal)) {
          stopped = true;
          break;
          }

      if(USE_FILETIME_4_X == val4X) {
        if(CAST_I64(BuffStat) > ftEnd) {
          if(foundGreat)
            break;
          foundGreat = true;
          }
        }
      else {
        if(getXVal(BuffStat) > CAST_I64_TO_fREAL(ftEnd)) {
          if(foundGreat)
            break;
          foundGreat = true;
          }
        }
      curr = addToVect(BuffStat, curr);
      if(f.P_read(BuffStat, sizeStat) != sizeStat)
        break;
      }
    }
  return curr;
}
//--------------------------------------------------------------------
int TD_LGraph2b::makeVectStep(DWORD nTotRec, DWORD ixInit)
{
  int curr = 0;
  int nElem = List.getElem();

  BarProgr bar(this, nTotRec);
  if(!onlyRefresh) {
    bar.create();
    }
  __int64 ftEnd = getToValue();
  if(ftEnd && !fixedScale)
    canUpdate = false;
  if(!ftEnd)
    ftEnd = MK_I64(getFileTimeCurr());

  bool stopped = false;
  double step = nTotRec;
  step /= maxRecord2Show;

  double ixGlobal = 0;
  for(int i = 0; i < nElem && !stopped; ++i) {
    P_File f(List[i]->name, P_READ_ONLY);
    if(!f.P_open())
      continue;

    memset(BuffStat, 0, sizeStat);

    DWORD nRec = (DWORD)f.get_len() - sizeHeader;
    nRec /= sizeStat;

    double ix = 0;
    if(!i)
      ix = ixInit;

    bool foundGreat = false;
    for(;; ixGlobal += step, ix += step) {
      if(ROUND_POS_REAL(ix) >= (long)nRec)
        break;
      f.P_seek(ROUND_POS_REAL(ix) * sizeStat + sizeHeader);
      if(f.P_read(BuffStat, sizeStat) != sizeStat)
        break;

      if(!onlyRefresh && !(ROUND_POS_REAL(ixGlobal) & 0xf))
        if(!bar.setPos((DWORD)ixGlobal)) {
          stopped = true;
          break;
          }

      if(USE_FILETIME_4_X == val4X) {
        if(CAST_I64(BuffStat) > ftEnd) {
          if(foundGreat)
            break;
          foundGreat = true;
          }
        }
      else {
        if(getXVal(BuffStat) > CAST_I64_TO_fREAL(ftEnd)) {
          if(foundGreat)
            break;
          foundGreat = true;
          }
        }
      curr = addToVect(BuffStat, curr);
      }
    }
  return curr;
}
//--------------------------------------------------------------------
int TD_LGraph2b::makeVect()
{
  static bool inExec;
  if(inExec)
    return 0;
  inExec = true;
  DWORD ixInit;
  DWORD nTotRec = calcInitIx(ixInit);

  int result;
  if(maxRecord2Show && nTotRec > (DWORD)maxRecord2Show)
    result = makeVectStep(nTotRec, ixInit);
  else
    result = makeVectFull(nTotRec, ixInit);
  inExec = false;
  return result;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
lgCoord TD_LGraph2b::calcY(fREALDATA val, int ix)
{
  rangeSet* rs = RangeSet;
  if(isSameValue(RangeSet->rMin[ix], RangeSet->rMax[ix])) {
    rs = RangeSetOnUse;
    bool needTimer = false;
     if(!rs->rMin[ix] && isSameValue(rs->rMin[ix], rs->rMax[ix])) {
      rs->rMin[ix] = val - 1;
      rs->rMax[ix] = val + 1;
      needTimer = true;
      }
    else {
      if(val < rs->rMin[ix]) {
        rs->rMin[ix] = val;
        needTimer = true;
        }
      if(val > rs->rMax[ix]) {
        rs->rMax[ix] = val;
        needTimer = true;
        }
      }
    if(needTimer && !idTimerRefresh)
      SetTimer(*this, idTimerRefresh = 4334, 1000, 0);
    }
  fREALDATA range = rs->rMax[ix] - rs->rMin[ix];
  fREALDATA curr =  val - rs->rMin[ix];

  if(curr <= 0.0 || range <= 0.0)
    return MAX_H_DATA;
  double tval = MAX_H_DATA - ((double)curr * MAX_H_DATA / range);
  if(tval < 1.0)
    return 1;
  return lgCoord(tval);
}
//--------------------------------------------------------------------
lgCoord TD_LGraph2b::calcX(const LPBYTE stf)
{
  if(USE_FILETIME_4_X == val4X)
    lastCurrVal = (double)(CAST_I64(stf) - RangeSet->from);
  else {
    fREALDATA* pData = (fREALDATA*)(stf + offsStat);
    lastCurrVal = *(pData + val4X) - CAST_I64_TO_fREAL(RangeSet->from);
    }

  if(lastCurrVal < 0)
    return lgCoord(-1);

  double maxX = lGraph[0]->getRect().Width() * MUL_W_DATA;
  lgCoord result = lgCoord(lastCurrVal * maxX / lastTotVal);
#if 1
  return result;
#else
  return result < 1 ? lgCoord(1) : result;
#endif
}
//--------------------------------------------------------------------
int TD_LGraph2b::addToVect(const LPBYTE stf, int curr)
{
  double currVal = lastCurrVal;
  lgCoord x = calcX(stf);
  // se ha caricato il record precedente, inutile aggiungerlo
  if(curr && fabs(currVal - lastCurrVal) <= DBL_EPSILON)
    return curr;

#if 1
  if(x < 0) {
    if(curr > 0)
      x = XCoord[curr - 1];
    else
      return curr;
    }
#else
  if(x < 0)
    return curr;
#endif

  int type;
  fREALDATA val;
  for(int i = 0; i < maxChoose; ++i) {
    int code = RangeSet->choose[i];
    type = codeToType(code);

    if(type < 0)
      return curr;

    fREALDATA* pData = (fREALDATA*)(stf + offsStat);
    int offs = infoStat[type]->offsStatFile;
    val = *(pData + offs);
    yGraph[i][curr] = calcY(val, type);
    yGraph[i].setIncr();
    }

  XCoord[curr] = x;
  XCoord.setIncr();
  return curr + 1;
}
//--------------------------------------------------------------------
void TD_LGraph2b::recalcXCoord(LPCBYTE buffStat)
{
  int totElem = XCoord.getElem();
  double oldTotVal = lastTotVal;
  __int64 dummy = 0;

  reCalcEnd(buffStat, dummy);
  saveSet();
  calcLastTot();
  if(fabs(lastTotVal - oldTotVal) < 0.5)
    return;
  double ratio = oldTotVal / lastTotVal;
  for(int i = 1; i < totElem - 1; ++i) {
    XCoord[i] = (lgCoord)(XCoord[i] * ratio);
    }
}
//--------------------------------------------------------------------
void TD_LGraph2b::addLastToVect()
{
  if(!*fileName)
    return;
  bool ok = false;

//  LPBYTE buffStat = new BYTE[sizeStat];
  memset(BuffStat, 0, sizeStat);

  do {
    int nElem = List.getElem();
    if(!nElem)
      return;
    P_File f(List[nElem - 1]->name, P_READ_ONLY);
    if(f.P_open()) {
      f.P_seek(-long(sizeStat), SEEK_END_);
      f.P_read(BuffStat, sizeStat);
      ok = true;
      }
    } while(false);
  if(ok) {
    int old = XCoord.getElem();
    int curr = addToVect(BuffStat, old);
    if(curr > old) {
      if(!fixedScale)
        recalcXCoord(BuffStat);
      for(int i = 0; i < maxChoose; ++i)
        lGraph[i]->setBuff(&XCoord, &yGraph[i], curr);
      invalidateGraph();
      }
    }
//   delete []buffStat;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
void TD_LGraph2b::setTimeForTimer()
{
  LPCTSTR p = getPageString(ID_TREND_TIME_PSW);
  if(p) {
    int pswLevel = _ttoi(p);
    bool dummy;
    if(!hasPermission(pswLevel, dummy))
      return;
    PassWord().restartTime(0);
    }

  p = getPageString(ID_TREND_NAME);
  uint id = getIdByNameTrend(p);
  p = getString(id);
  if(!p)
    return;
  int addr = _ttoi(p);
  genericPerif *prf = Par->getGenPerif(PRF_MEMORY);

  prfData data;
  data.typeVar = prfData::tDWData;
  data.lAddr = addr;
  data.U.dw = 0;
  prf->get(data);

// il tempo è in millisecondi
  uint currTimer = data.U.dw;

  smartPointerConstString tmsg = getStringOrIdByLang(ID_TIMER_SET_TIME);

  smartPointerConstString ttit = getStringOrIdByLang(ID_TITLE_SET_TIME);

  if(IDOK == TD_Timer(this, &ttit, &tmsg, currTimer).modal()) {
//  if(IDOK == TD_Timer(this, sStr.getStringOrId(ID_TITLE_SET_TIME),
//        sStr.getStringOrId(ID_TIMER_SET_TIME), currTimer).modal()) {

    data.U.dw = currTimer;
    prf->set(data);
//    prf->set(data, true);
    gestJobData* job = Par->getData();
    job->checkTimerTrend();
    }

}
//--------------------------------------------------------------------------
#define KEY_LGRAPH_FILE _T("lGraphFile")
//--------------------------------------------------------------------
bool TD_LGraph2b::chooseFile()
{
#if 0 // pronta per rimuovere le informazioni ridondanti
  LPCTSTR pNameTrend = getPageString(ID_TREND_NAME);
  uint idInitCode = getIdByNameTrend(pNameTrend);
  mainClient* wMain = getMain();
  if(!wMain)
    return false;
  TCHAR path[_MAX_PATH];
  ImplJobData* job = wMain->getIData();
  const ImplJobMemBase::save_trend* pTrend = job->getSaveTrendByCode(idInitCode);
  bool history = false;
  if(!pTrend->makeFindableName(path, history))
    return false;

#else
  LPCTSTR p = getPageString(ID_FILE_TREND_PATH);
  if(!p)
    return false;
  TCHAR path[_MAX_PATH];
  TCHAR ext[_MAX_EXT];
  TCHAR file[_MAX_FNAME];
  _tsplitpath_s(p, 0, 0, 0, 0, file, SIZE_A(file), ext, SIZE_A(ext));
  _tmakepath_s(path, 0, 0, file, ext);

  bool history = false;
  p =  getPageString(ID_FILE_USE_HISTORY);
  if(p)
    history = toBool(_ttoi(p));
  if(!makePathTrend(path, history))
    return false;
#endif
  smartPointerConstString sp = getStringOrIdByLang(ID_TITLE_FIND_FILE);

  infoSearchParam isp;
  getKeyInfoSearch(KEY_LGRAPH_FILE, isp);
  infoSearch info(sp, path, isp);
  if(!info.chooseParam(this, true))
    return false;

  info.copyDataTo(isp);
  setKeyInfoSearch(KEY_LGRAPH_FILE, isp);

  if(gSearchFile(info, this, fileName)) {
#if 0
    TCHAR ext[_MAX_EXT] = _T("");
    LPCTSTR p = getPageString(ID_FILE_TREND_PATH);
    if(p)
      _tsplitpath_s(p, 0, 0, 0, 0, 0, 0, ext, SIZE_A(ext));
#endif
    _tcscpy_s(path, fileName);
    _tcscat_s(fileName, ext);
    flushPV_toFree(FreeFile, List);
    insertInListFile(fileName, history);
    makePathTrend(fileName, path, history);
    _tcscat_s(fileName, ext);
    if(USE_FILETIME_4_X == val4X)
      calcInputDatefromFile();
    byChooseFile = true;
    return true;
    }
  return false;
}
//--------------------------------------------------------------------
void TD_LGraph2b::calcInputDatefromFile()
{
  P_File pf(fileName, P_READ_ONLY);
  if(!pf.P_open())
    return;
  pf.P_seek(sizeHeader);
  FILETIME ft;
  pf.P_read(&ft, sizeof(ft));
  RangeSet->from = MK_I64(ft);
  pf.P_seek(-(long)sizeStat, SEEK_END_);
  pf.P_read(&ft, sizeof(ft));
  RangeSet->to = MK_I64(ft);
  showSet();
  setDirty();
}
//--------------------------------------------------------------------
void TD_LGraph2b::setLimits()
{
  TCHAR file[_MAX_PATH];
  getFileRange(file);

//  if(makePathTrend(file)) {
    if(uniqueRange) {
      int count = infoStat.getElem();

      double init = RangeSet->rMin[0];
      double end = RangeSet->rMax[0];
      if(IDOK == TD_ChooseNum(this, init, end, false).modal()) {
        for(int i = 0; i < count; ++i) {
          RangeSet->rMin[i] = (fREALDATA)init;
          RangeSet->rMax[i] = (fREALDATA)end;
          }
        P_File f(file, P_CREAT);
        if(f.P_open())
          f.P_write(RangeSet, sizeof(rangeSet));
        }
      }
    else
      PD_Limits(file, sStr, this).modal();
//    }
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
#include "printGraph.h"
//--------------------------------------------------------------------
extern svPrinter* getPrinter();
//--------------------------------------------------------------------
void TD_LGraph2b::print_Graph()
{
  if(!XCoord.getElem())
    return;
  infoPrintGraph ipg;
  ipg.numPage = 4;
  ipg.trendType = val4X;
  ipg.numGraph = maxChoose;
  ipg.nElem = XCoord.getElem();
  ipg.XCoord = new lgCoord[ipg.nElem];
#if 1
  ipg.XCoord[0] = XCoord[0];
  for(int i = 1; i < ipg.nElem; ++i) {
    ipg.XCoord[i] = XCoord[i];
    if(ipg.XCoord[i] < ipg.XCoord[i - 1])
      ipg.XCoord[i] = ipg.XCoord[i - 1];
    }
#else
  memcpy(ipg.XCoord, &XCoord, ipg.nElem * sizeof(lgCoord));
#endif
  ipg.pixWidth = lGraph[0]->getRect().Width() * MUL_W_DATA;
  ipg.from = RangeSet->from;
  ipg.to   = RangeSet->to;
  ipg.numRow = 10;
  ipg.dimMinRow = 50;
  ipg.U.f.graphOver = showOver;
//  ipg.U.f.preview = 1;
//  ipg.U.f.setup = 1;

  {
    TCHAR tmp[_MAX_FNAME];
    _tsplitpath_s(fileName, 0, 0, 0, 0, tmp, SIZE_A(tmp), 0, 0);
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

  ipg.nDec = getDec(VALUE_FROM);

  ipg.FieldSet.setDim(maxChoose);
  ipg.U.f.activeLabel = 0;
  for(int i = 0; i < maxChoose; ++i) {
    ipg.yGraph[i] = new lgCoord[ipg.nElem];
    memcpy(ipg.yGraph[i], &yGraph[i], ipg.nElem * sizeof(lgCoord));

    int type = RangeSet->choose[i];
    int ix = codeToType(type);
    if(ix < 0)
      break;
    int dec = infoStat[ix]->dec;

    infoPrintGraph::field fld;
    copyStr(fld.info, infoStat[ix]->descr, _tcslen(infoStat[ix]->descr) + 1);
    fld.vMin = RangeSet->rMin[ix];
    fld.vMax = RangeSet->rMax[ix];
    fld.color = lGraph[i]->getColor();
    LPCTSTR p = getPageString(ID_COLOR_1 + i);
    p = findNextParam(p, 4);
    fld.tickness = 1;
    if(p)
      fld.tickness = _ttoi(p);
    fld.nDec = dec;
    ipg.FieldSet[i] = fld;
    }

  smartPointerConstString tmsg = getStringOrIdByLang(ID_GET_PAGE);
  smartPointerConstString ttit = getStringOrIdByLang(ID_TITLE_GET_PAGE);

  static uint nPage = 1;
  if(IDOK == TD_Timer(this, &ttit, &tmsg, nPage).modal()) {
//  if(IDOK == TD_Timer(this, sStr.getStringOrId(ID_TITLE_GET_PAGE),
//          sStr.getStringOrId(ID_GET_PAGE), nPage).modal()) {
    if(nPage > MAX_PAGE)
      nPage = MAX_PAGE;
    ipg.numPage = nPage;
    LPCTSTR p = getPageString(ID_PRINT_TREND_PREVIEW);
    bool setup = true;
    bool preview = false;
    if(p) {
      preview = toBool(_ttoi(p));
      p = findNextParam(p, 1);
      if(p)
        setup = toBool(_ttoi(p));
      }
    if(preview)
      printGraph(this, getPrinter(), ipg).Preview(setup);
    else
      printGraph(this, getPrinter(), ipg).Print(setup);
    }

  for(int i = maxChoose - 1; i >= 0; --i)
    delete []ipg.yGraph[i];
  delete []ipg.XCoord;
};
//----------------------------------------------------------------------------
static LPCTSTR filterExt[] = { _T(".tab"), 0 };
//----------------------------------------------------------------------------
static LPCTSTR filter =
  _T("Export (tab)\0")
  _T("*.tab\0");
//----------------------------------------------------------------------------
class expOpenSave : public POpenSave
{
  private:
    typedef POpenSave baseClass;
  public:
    expOpenSave(HWND owner) : baseClass(owner)   {  }
  protected:
    static TCHAR Path[_MAX_PATH];
    virtual void setPathFromHistory();
    virtual void savePathToHistory();
    virtual void setInitialFile(infoOpenSave& info);
};
//----------------------------------------------------------------------------
void expOpenSave::setInitialFile(infoOpenSave& info)
{
  if(*Path && info.getFile()) {
    LPCTSTR file = info.getFile();
    int len = _tcslen(file);
    for(int i = len - 1; i > 0; --i) {
      if(_T('\\') == file[i] || _T('/') == file[i]) {
        _tcscpy_s(filename, file + i + 1);
        return;
        }
      }
    }
  baseClass::setInitialFile(info);
}
//----------------------------------------------------------------------------
TCHAR expOpenSave::Path[_MAX_PATH];
//----------------------------------------------------------------------------
void expOpenSave::setPathFromHistory()
{
  if(*Path)
    SetCurrentDirectory(Path);
}
//----------------------------------------------------------------------------
void expOpenSave::savePathToHistory()
{
  GetCurrentDirectory(_MAX_PATH, Path);
}
//--------------------------------------------------------------------
/*
static void add_suffix(LPTSTR target, LPCTSTR source)
{
  TCHAR disk[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];
  TCHAR file[_MAX_FNAME];
  TCHAR ext[_MAX_EXT];
  _tsplitpath_s(source, disk, SIZE_A(disk), dir, SIZE_A(dir), file, SIZE_A(file), ext, SIZE_A(ext));
  _tcscat_s(file, _T("_aa"));
  _tmakepath_s(target, _MAX_PATH, disk, dir, file, ext);
}
*/
//--------------------------------------------------------------------
bool queryExportPath(PWin* parent, LPTSTR fileExp)
{
  infoOpenSave Info(filterExt, filter, infoOpenSave::SAVE_AS, 0, fileExp);
  expOpenSave open(*parent);

  if(open.run(Info)) {
    _tcscpy_s(fileExp, _MAX_PATH, open.getFile());
//    add_suffix(fileExp, open.getFile());
    return true;
    }
  return false;

}
//--------------------------------------------------------------------
void TD_LGraph2b::exportTrend()
{
  bool onShow = toBool(*fileName);

  setOfString set;

  LPCTSTR pNameTrend = getPageString(ID_TREND_NAME);
  uint idInitCode = getIdByNameTrend(pNameTrend);

  if(!onShow) {
    mainClient* wMain = getMain();
    if(!wMain)
      return;
    ImplJobData* job = wMain->getIData();
    const ImplJobMemBase::save_trend* pTrend = job->getSaveTrendByCode(idInitCode);
    pTrend->makeCurrentName(wMain, fileName);
    if(!*fileName)
      return;
    }

  TCHAR fileExp[_MAX_PATH];
  do {
    TCHAR tf[_MAX_PATH];
    _tsplitpath_s(fileName, 0, 0, 0, 0, tf, SIZE_A(tf), 0, 0);

    makeExportPath(fileExp, _T(""), true, dHistory);
    createDirectoryRecurse(fileExp);

    makeExportPath(fileExp, tf, true, dHistory);
#if 1
#if 0
    int len = _tcslen(fileExp);
    do {
      if(_T('_') == fileExp[len - 3]) {
        _tcscat_s(fileExp, _T("-"));
        ++len;
        }
      else
        break;
      } while(true);
#endif
    _tcscat_s(fileExp, _T(".tab"));
    if(!queryExportPath(this, fileExp)) {
      if(!onShow)
        fileName[0] = 0;
      return;
      }
#if 0
    len = _tcslen(fileExp);
    bool ok = true;
    do {
      for(int i = len - 1; i > 0; --i) {
        if(_T('.') == fileExp[i]) {
          if(_T('_') == fileExp[i - 3]) {
            for(int j = len; j >= i; --j)
              fileExp[j + 1] = fileExp[j];
            fileExp[i] = _T('-');
            ok = false;
            }
          break;
          }
        }
      } while(!ok);
#endif
#else
//    _tcscat_s(fileExp, _T("_aa"));
    _tcscat_s(fileExp, _T(".tab"));
#endif
    } while(false);
  P_File pfS(fileName, P_READ_ONLY);
  if(!pfS.P_open()) {
    if(!onShow)
      fileName[0] = 0;
    return;
    }
  P_File pfT(fileExp, P_CREAT);
  if(!pfT.P_open()) {
    if(!onShow)
      fileName[0] = 0;
    return;
    }
  returnExp result = exportExByGraph(this, pfT, pfS, idInitCode);

  if(!onShow)
    fileName[0] = 0;
  if(rAbort != result)
    msgBoxByLangGlob(this, ID_EXP_FAILED + result, ID_EXP_SUCCESS_TITLE, MB_ICONINFORMATION);

}
