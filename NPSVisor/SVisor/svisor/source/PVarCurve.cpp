//------ PVarCurve.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarCurve.h"
#include "p_basebody.h"
#include "mainclient.h"
#include "p_util.h"
#include "newnormal.h"
//----------------------------------------------------------------------------
//#define MAX_Y_DEC 100.0
#define MAX_Y_DEC 10000.0
#define MAX_Y 100.0
#define STEP_Y IC.nRow
//#define STEP_Y (MAX_Y / 5.0)

#define REM_DEC(a) ((a) * MAX_Y / MAX_Y_DEC)
#define MUL_DEC(a) ((a) / MAX_Y * MAX_Y_DEC)
//----------------------------------------------------------------------------
#define IDC_STATIC_COORD 11111
#define ADD_H (IC.readOnly ? 0 : R__Y(16))
//----------------------------------------------------------------------------
PVarCurve::PVarCurve(P_BaseBody* owner, uint id) : baseActive(owner, id),
    baseWinClass(owner, id, PRect(0, 0, 10, 10), 0), Style(0), clipped(false),
    Wave(0), stepX(0), Hand(0), oldy(-1), oldx(-1), hBmpWork(0), mdcWork(0), oldObjWork(0),
    hBmpBkgWork(0), mdcBkgWork(0), oldObjBkgWork(0)
{
  ZeroMemory(&IC, sizeof(IC));
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool PVarCurve::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int nPoints  = 0;
  int perc1stSpace = 0;
  int nPoints1stSpace = 0;
  int readOnly = 1;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &Style, &nPoints,
                    &perc1stSpace, &nPoints1stSpace,
                    &readOnly);

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  Attr.x = rect.left;
  Attr.y = rect.top;
  Attr.w = rect.Width();
  Attr.h = rect.Height();
  setRect(rect);

  int nRow = 10;

  int RBl = 0x1f;
  int GBl = 0x1f;
  int BBl = 0;

  int RGr = 0xaf;
  int GGr = 0xaf;
  int BGr = 0xaf;

  int RAx = 0xff;
  int GAx = 0;
  int BAx = 0;

  int maxX = 0;
  int Rls = 0;
  int Gls = 0;
  int Bls = 0;
  TCHAR fileShow[_MAX_PATH] = _T("\0");

  p = getOwner()->getPageString(getThirdExtendId());
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s"),
                    &nRow,
                    &RBl, &GBl, &BBl,
                    &RGr, &GGr, &BGr,
                    &RAx, &GAx, &BAx,
                    &maxX,
                    &Rls, &Gls, &Bls,
                    fileShow, SIZE_A(fileShow)
                    );

  IC.Fg = RGB(Rfg, Gfg, Bfg);
  IC.Bkg = RGB(Rbk, Gbk, Bbk);
  IC.ball = RGB(RBl, GBl, BBl);
  IC.grid = RGB(RGr, GGr, BGr);
  IC.axe = RGB(RAx, GAx, BAx);

  IC.lineShow = RGB(Rls, Gls, Bls);
  IC.maxX = maxX;
  _tcscpy_s(IC.fileShow, SIZE_A(IC.fileShow), fileShow);

  IC.nRow = nRow;
  IC.nPoints = nPoints;
  IC.perc1stSpace = perc1stSpace;
  IC.nPoints1stSpace = nPoints1stSpace;
  IC.readOnly = readOnly;
  if(!makeStdVars())
    return false;

  enableCapture(true);

  switch(Style) {
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

  Brush = CreateSolidBrush(IC.Bkg);
  Wave = new double[IC.nPoints];
  stepX  = new double[IC.nPoints];
  ZeroMemory(Wave, IC.nPoints * sizeof(*Wave));
  ZeroMemory(stepX, IC.nPoints * sizeof(*stepX));
  if(!BaseVar.getPrph() && !IC.readOnly)
    IC.readOnly = 1;
  MK_I64(ftLastMod) = 0;

  if(!IC.readOnly) {
    PRect rC(0, 0, R__X(160), R__Y(20));
    rC.MoveTo(rect.Width() - R__X(170), 2);
    PStatic* st = new PStatic(this, IDC_STATIC_COORD, rC);
    HFONT f = D_FONT(R__Y(16), 0, 0, _T("arial"));
    st->setFont(f, true);
    }

  return create();
}
//----------------------------------------------------------------------------
PVarCurve::~PVarCurve()
{
  removeAllBmpWork(3);
  destroy();
  DeleteObject(Brush);
  delete []Wave;
  delete []stepX;
}
//----------------------------------------------------------------------------
bool PVarCurve::create()
{
  if(!baseWinClass::create())
    return false;

  calcGrid();
  reload(true);
  return true;
}
//-----------------------------------------------------------
void PVarCurve::setVisibility(bool set)
{
  addToClipped(set);
  baseActive::setVisibility(set);
}
//----------------------------------------------------------------------------
bool PVarCurve::isChangedFileShow()
{
  if(!*IC.fileShow)
    return false;
  P_File pf(IC.fileShow, P_READ_ONLY);
  if(!pf.P_open())
    return false;
  FILETIME curr;
  if(!pf.getData(curr))
    return false;
  if(!(ftLastMod == curr)) {
    ftLastMod = curr;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool PVarCurve::reload(bool force)
{
  if(!BaseVar.getPrph())
    return false;

  mainClient* mc = getMain();
  genericPerif* prph = mc->getGenPerif(BaseVar.getPrph());
  if(!prph)
    return false;
  prfData data;
  data.lAddr = BaseVar.getAddr();
  data.typeVar = BaseVar.getType();

  bool needRefresh = false;
  for(int i = 0; i < (int)IC.nPoints; ++i) {
    bool changed = prfData::isChanged == prph->get(data);
    if(changed)
      needRefresh = true;
    if(force || changed) {
      REALDATA t;
      getNormalizedData(t, data, getNorm(BaseVar.getNorm()));
      Wave[i] = MUL_DEC(t);
      }
    ++data.lAddr;
    }
  return needRefresh;
}
//-----------------------------------------------------------
void PVarCurve::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  int addr = BaseVar.getAddr() + 1;
  for(int i = 1; i < (int)IC.nPoints; ++i, ++addr)
    addReqVar2(allBits, BaseVar.getPrph(), addr);
}
//-----------------------------------------------------------
bool PVarCurve::addToClipped(bool add)
{
  PRect r;
  GetWindowRect(*this, r);
  MapWindowPoints(HWND_DESKTOP, *Owner, (LPPOINT)(LPRECT)r, 2);
  if(add)
    return Owner->addToClipped(r);
  Owner->removeFromClipped(r);
  return false;
}
//-----------------------------------------------------------
bool PVarCurve::update(bool force)
{
  bool upd = baseActive::update(force);
  if(!isVisible())
    return upd;
  if(!clipped)
    clipped = addToClipped(true);

  force |= upd;
  if(reload(force) || isChangedFileShow()) {
    invalidate();
//    InvalidateRect(*this, 0, 0);
    return true;
    }
  return upd;
}
//----------------------------------------------------------------------------
bool PVarCurve::evCtlColor(HBRUSH& result, HDC hdc, HWND hwnd, uint /*message*/)
{
  if(hwnd == getHandle()) {
    SetTextColor(hdc, IC.Fg);
    SetBkColor(hdc, IC.Bkg);
    result = Brush;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#ifndef GET_X_LPARAM
  #define GET_X_LPARAM(lp) LOWORD((DWORD)lp)
  #define GET_Y_LPARAM(lp) HIWORD((DWORD)lp)
#endif
//----------------------------------------------------------------------------
void PVarCurve::paint(HDC hdc, const PRect& r)
{
#define USE_MEMDC
#ifdef USE_MEMDC

          if(!hBmpWork) {
            allocBmpWork(hdc);
            }
          FillRect(mdcWork, r, Brush);
          evPaint(mdcWork);
          BitBlt(hdc, r.left, r.top, r.Width(), r.Height(), mdcWork, r.left, r.top, SRCCOPY);
/*

#define DEF_COLOR_BKG RGB(192, 192, 192)
        PRect r;
        GetClientRect(*this, r);
        HBITMAP hBmpWork = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
        HDC mdcWork = CreateCompatibleDC(hdc);
        HGDIOBJ oldObj = SelectObject(mdcWork, hBmpWork);
        FillRect(mdcWork, r, Brush);

        evPaint(mdcWork);

        BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdcWork, 0, 0, SRCCOPY);

        SelectObject(mdcWork, oldObj);

        DeleteDC(mdcWork);

        DeleteObject(hBmpWork);
*/

#else
        evPaint(hdc);
#endif

}
//---------------------------------------------------------------------
bool PVarCurve::allocBmpWork(HDC hdc)
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
//---------------------------------------------------------------------
bool PVarCurve::allocBmpBkgWork(HDC hdc)
{
  if(hBmpBkgWork)
    return true;

  PRect r;
  GetClientRect(*this, r);
  if(r.Width() && r.Height() && getHandle()) {
    HDC dc = hdc;
    if(!dc)
      dc = GetDC(*this);
    hBmpBkgWork = CreateCompatibleBitmap(dc, r.Width(), r.Height());
    if(hBmpBkgWork) {
      mdcBkgWork = CreateCompatibleDC(dc);
      oldObjBkgWork = SelectObject(mdcBkgWork, hBmpBkgWork);
      HBRUSH br = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
      FillRect(mdcBkgWork, r, br);
      }
    if(!hdc)
      ReleaseDC(*this, dc);
    }
  return toBool(hBmpBkgWork);
}
//---------------------------------------------------------------------
void PVarCurve::removeAllBmpWork(uint bits)
{
  if(hBmpWork && (bits & 1)) {
    SelectObject(mdcWork, oldObjWork);
    DeleteDC(mdcWork);
    DeleteObject(hBmpWork);
    hBmpWork = 0;
    }
  if(hBmpBkgWork && (bits & 2)) {
    SelectObject(mdcBkgWork, oldObjBkgWork);
    DeleteDC(mdcBkgWork);
    DeleteObject(hBmpBkgWork);
    hBmpBkgWork = 0;
    }
}
//----------------------------------------------------------------------------
LRESULT PVarCurve::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      do {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        evMouse(message, wParam, pt);
        } while(false);
      break;

    case WM_ERASEBKGND:
      if(wParam) {
        PRect r;
        GetClientRect(hwnd, r);
        if(!hBmpWork) {
          HDC dc = GetDC(*this);
          allocBmpWork(dc);
/*
          uint nElem = Edi.getElem();
          for(uint i = 0; i < nElem; ++i) {
            if(Edi[i]->isVisible()) {
              PRect r = Edi[i]->getRect();
              ExcludeClipRect(mdcWork, r.left, r.top, r.right, r.bottom);
              }
            }
          nElem = oTrend.getElem();
          for(uint i = 0; i < nElem; ++i) {
            if(oTrend[i]->isVisible()) {
              PRect r = oTrend[i]->getRect();
              ExcludeClipRect(mdcWork, r.left, r.top, r.right, r.bottom);
              }
            }
*/
          ReleaseDC(*this, dc);
          }
        HBRUSH br = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
        FillRect(mdcWork, r, br);
        }
/*
      do {
        HDC hdc = GetDC(hwnd);
        paint(hdc);
        ReleaseDC(hwnd, hdc);
        } while(false);
*/
      return 1;

    case WM_PAINT:
      if(GetUpdateRect(hwnd, 0, 0)) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        PRect r(ps.rcPaint);
        if(r.Width() && r.Height())
          paint(hdc, r);
/*
#define USE_MEMDC
#ifdef USE_MEMDC

#define DEF_COLOR_BKG RGB(192, 192, 192)
        PRect r;
        GetClientRect(*this, r);
        HBITMAP hBmpWork = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
        HDC mdcWork = CreateCompatibleDC(hdc);
        HGDIOBJ oldObj = SelectObject(mdcWork, hBmpWork);
        FillRect(mdcWork, r, Brush);

        evPaint(mdcWork);

        BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdcWork, 0, 0, SRCCOPY);

        SelectObject(mdcWork, oldObj);

        DeleteDC(mdcWork);

        DeleteObject(hBmpWork);

#else
        evPaint(hdc);
#endif
*/
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  return baseWinClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void PVarCurve::mouseLeave(const POINT& pt, uint flag)
{
  baseWinClass::mouseLeave(pt, flag);
  if(3 == Hand) {
    POINT pt = { -1, -1 };
    EvMouseRUp(0, pt);
    }
}
//-----------------------------------------------------------
void PVarCurve::updateCoord(POINT pt)
{
  trasf_point(pt);

  TCHAR buff[100];

  REALDATA v = REM_DEC(pt.y);
  REALDATA norm = getNorm(BaseVar.getNorm());
  if(norm != 1.0) {
    REALDATA n = v / getNorm(BaseVar.getNorm());
    _stprintf_s(buff, SIZE_A(buff), _T("%d - %0.2f [%0.4f]"), pt.x, v, n);
    }
  else
    _stprintf_s(buff, SIZE_A(buff), _T("%d - %0.2f"), pt.x, v);

  SetDlgItemText(*this, IDC_STATIC_COORD, buff);
}
//-----------------------------------------------------------
void PVarCurve::evMouse(uint message, uint key, POINT pt)
{
  if(IC.readOnly)
    return;
  switch(message) {
    case WM_LBUTTONUP:
      EvMouseLUp(key, pt);
      break;
    case WM_RBUTTONUP:
      EvMouseRUp(key, pt);
      break;
    case WM_LBUTTONDOWN:
      EvMouseLDn(key, pt);
      break;
    case WM_RBUTTONDOWN:
      EvMouseRDn(key, pt);
      break;
    case WM_MOUSEMOVE:
      MouseMove(key, pt, 1);
    }
  updateCoord(pt);
}
//----------------------------------------------------------------------------
HWND PVarCurve::evSetAroundFocus(UINT key)
{
  HWND hwnd = (HWND)-1;
  return hwnd;
}
//----------------------------------------------------------------------------
#define IS_DIFF(v1, v2) (fabs((v1) - (v2)) > dPRECISION)
//----------------------------------------------------------------------------
void PVarCurve::sendCurrData()
{
  if(IC.readOnly || !BaseVar.getPrph())
    return;
  mainClient* mc = getMain();
  genericPerif* prph = mc->getGenPerif(BaseVar.getPrph());
  prfData data;
  data.lAddr = BaseVar.getAddr();
  data.typeVar = BaseVar.getType();
  gestPerif::howSend oldHow;
  gestPerif* tp = dynamic_cast<gestPerif*>(prph);
  if(tp) {
    oldHow = tp->setAutoShow(gestPerif::CHECK_READY);

    prfData* mdata = new prfData[IC.nPoints];
    uint num = 0;
    for(int i = 0; i < (int)IC.nPoints; ++i) {
      prph->get(data);
      REALDATA t = 0;
      getNormalizedData(t, data, getNorm(BaseVar.getNorm()));
      REALDATA t2 = REM_DEC(Wave[i]);
      if(IS_DIFF(t, t2)) {
        mdata[num] = data;
        getDenormalizedData(mdata[num], t2, getNorm(BaseVar.getNorm()));
        ++num;
  //      prph->set(data, false);
        }
      ++data.lAddr;
      }
    if(num)
      tp->multiSet(mdata, num);
    delete []mdata;
    tp->setAutoShow(oldHow);
    }
  else {
    for(int i = 0; i < (int)IC.nPoints; ++i) {
      REALDATA t2 = REM_DEC(Wave[i]);
      getDenormalizedData(data, t2, getNorm(BaseVar.getNorm()));
      prph->set(data, false);
      ++data.lAddr;
      }
    }
}
//----------------------------------------------------------------------------
#define MAX_NUM_POINT IC.nPoints
#define COL_AX      IC.axe
#define COL_GRID    IC.grid
#define COL_LINE    IC.Fg
#define COL_CIRCLE  IC.ball
#define COL_BKG     IC.Bkg
//----------------------------------------------------------------------------
//#define OFFX 5
//#define OFFY 5
#define OFFY (stepX[0])
#define RADIUS 1.5
//----------------------------------------------------------------------------
void PVarCurve::calcGrid()
{
  PRect r;
  GetClientRect(*this, r);
  double stepxFirst;
  double stepxSecond;
  int OffsX;
  if(!IC.nPoints1stSpace || !IC.perc1stSpace || IC.nPoints1stSpace == MAX_NUM_POINT) {
    stepxFirst = 0;
    IC.nPoints1stSpace = 0;
    stepxSecond = (double)r.right / MAX_NUM_POINT;
    OffsX = (int)(stepxSecond / 4);
    }
  else {
    stepxFirst = r.right * (IC.perc1stSpace / 100.0) / IC.nPoints1stSpace;
    stepxSecond = (r.right * (1.0 - IC.perc1stSpace / 100.0) - stepxFirst) / (MAX_NUM_POINT - IC.nPoints1stSpace);
    OffsX = (int)(min(stepxFirst, stepxSecond) / 4);
    }
  stepX[0] = RADIUS + OffsX;
  int i;
  double offs = stepX[0] + stepxFirst;
  for(i = 1; i < (int)IC.nPoints1stSpace; ++i, offs += stepxFirst)
    stepX[i] = ROUND_POS_REAL(offs);

  for(; i < (int)MAX_NUM_POINT; ++i, offs += stepxSecond)
    stepX[i] = ROUND_POS_REAL(offs);
}
//----------------------------------------------------------------------------
void PVarCurve::evPaint(HDC hdc)
{
  PRect r;
  GetClientRect(*this, r);

  Grid(hdc, r);
  draw_wave(hdc);
  drawBorder(hdc, r);

  drawFileShow(hdc);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void PVarCurve::recToPoint(uShow& uds, double mulX, double mulY, int height)
{
  mulX *= uds.ds.x;
  uds.pt.x = (LONG)(mulX + 0.5 + stepX[0]);

  mulY *= MUL_DEC(uds.ds.v * getNorm(BaseVar.getNorm()));

  uds.pt.y = (LONG)(height - (mulY + 0.5) - OFFY);
}
//----------------------------------------------------------------------------
void PVarCurve::drawFileShow(HDC hdc)
{
  if(!*IC.fileShow)
    return;

  P_File pf(IC.fileShow, P_READ_ONLY);
  if(!pf.P_open())
    return;
  #define MAX_DATA_SHOW 500
  uShow uds[MAX_DATA_SHOW + 1];

  HPEN pen = CreatePen(PS_SOLID, 1, IC.lineShow);
  HGDIOBJ old = SelectObject(hdc, pen);

  double mw = stepX[IC.nPoints - 1];
  mw -= stepX[0];
  mw /= IC.maxX;

  PRect r;

  GetClientRect(*this, r);
  int height = r.Height();
  double mh = (height - OFFY * 2 - ADD_H) / MAX_Y_DEC;
  int len = (int)(pf.get_len() / sizeof(uds[0].ds));
  POINT oldP = { -1, -1 };
  while(len >= 2) {
    int j = oldP.x < 0 && oldP.y < 0 ? 0 : 1;
    if(j) {
      uds[0].pt.x = oldP.x;
      uds[0].pt.y = oldP.y;
      }
    int readed = pf.P_read(&uds[j], MAX_DATA_SHOW * sizeof(uds[0].ds)) / sizeof(uds[0].ds);
    // probabile errore
    if(!readed)
      break;
    for(int i = 0; i < readed; ++i)
      recToPoint(uds[ i + j], mw, mh, height);
    Polyline(hdc, (CONST POINT *)uds, readed + j);
    oldP.x = uds[readed + j - 1].pt.x;
    oldP.y = uds[readed + j - 1].pt.y;
    len -= readed;
    }
  // andrebbe considerato un eventuale ultimo valore, ma dovrebbe essere ininflluente
//  if(len)
//    legge da file ultimo record e visualizza tramite la MoveToEx(oldP) e LineTo(record)
  SelectObject(hdc, old);
  DeleteObject(pen);
}
//----------------------------------------------------------------------------
void PVarCurve::drawBorder(HDC hdc, PRect r)
{
  if(1 == Style) {
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
void PVarCurve::Grid(HDC hdc, PRect r)
{
  COLORREF oldColor = SetBkColor(hdc, COL_BKG);
  // non dovrebbe servire, lo sfondo è già cancellato
//  ExtTextOut(hdc, r.left, r.top, ETO_OPAQUE, r, 0, 0, 0);

  HPEN Pen2 = CreatePen(PS_DOT, 1, COL_GRID);
  HGDIOBJ oldPen = SelectObject(hdc, Pen2);

  int i;
  for(i = 1; i < (int)MAX_NUM_POINT; ++i) {
    MoveToEx(hdc, (int)stepX[i], 0, 0);
    LineTo(hdc, (int)stepX[i], r.bottom);
    }

  double stepy = MAX_Y_DEC / STEP_Y;
  double j = stepy;
  for(i = 0; i < (int)STEP_Y; ++i, j += stepy) {
    POINT pt = { 0, (LONG)j };
    restore_point(pt);
    MoveToEx(hdc, 0, pt.y, 0);
    LineTo(hdc, r.right, pt.y);
    }

  HPEN Pen = CreatePen(PS_SOLID, 2, COL_AX);

  SelectObject(hdc, Pen);

  int y =  r.bottom - (int)OFFY;
  MoveToEx(hdc, 0, y, 0);
  LineTo(hdc, r.right, y);

  MoveToEx(hdc, (int)stepX[0], 0, 0);
  LineTo(hdc, (int)stepX[0], r.bottom);

  arrowUp(hdc, (int)stepX[0], 0);
  arrowRight(hdc, r.right, y);
  SelectObject(hdc, oldPen);
  DeleteObject(Pen2);
  DeleteObject(Pen);

  SetBkColor(hdc, oldColor);
}
//----------------------------------------------------------------------------
void PVarCurve::trasf_point(POINT& point)
{
  PRect r;
  GetClientRect(*this, r);
  if(point.x <= stepX[0])
    point.x = 1;
  else {
    int x = (int)(point.x + RADIUS * 2);
     for(int i = 0; i < (int)(MAX_NUM_POINT - 1); ++i) {

        if(x >= stepX[i] && x <= stepX[i + 1]) {
          point.x = i + 1;
          break;
          }
        }
    }
  if(point.x > (int)MAX_NUM_POINT)
    point.x = MAX_NUM_POINT;

  point.y = r.Height() - point.y - (LONG)OFFY;

  if(point.y < 0)
    point.y = 0;

  double y = (r.Height() - OFFY * 2 - ADD_H) / MAX_Y_DEC;

  point.y = (LONG)(point.y / y);
  if(point.y > MAX_Y_DEC)
    point.y = (LONG)MAX_Y_DEC;
}
//----------------------------------------------------------------------------
void PVarCurve::restore_point(POINT& point, uint H, double y)
{
  point.y = (LONG)(point.y * y);
  point.y = H - point.y - (LONG)OFFY;

  point.x = (LONG)stepX[point.x - 1];
}
//----------------------------------------------------------------------------
void PVarCurve::restore_point(POINT& point)
{
  PRect r;
  GetClientRect(*this, r);

  double y = (r.Height() - OFFY * 2 - ADD_H) / MAX_Y_DEC;
  restore_point(point, r.Height(), y);
/*
  point.y = (LONG)(point.y * y);
  point.y = r.Height() - point.y - (LONG)OFFY;
  point.x = (LONG)stepX[point.x - 1];
*/
}
//----------------------------------------------------------------------------
void PVarCurve::draw_wave(HDC hdc)
{
  HPEN Pen = CreatePen(PS_SOLID, 1, COL_LINE);

  HGDIOBJ oldPen = SelectObject(hdc, Pen);

  PRect rect;
  GetClientRect(*this, rect);
  double y = (rect.Height() - OFFY * 2 - ADD_H) / MAX_Y_DEC;
  uint h = rect.Height();
  POINT p = { 1, (int)Wave[0] };
  restore_point(p, h, y);
  MoveToEx(hdc, p.x, p.y, 0);
  for(int i = 1; i < (int)MAX_NUM_POINT; ++i) {
    POINT p = { i + 1, (int)Wave[i] };
    restore_point(p, h, y);
    LineTo(hdc, p.x, p.y);
    }
  if(2 != IC.readOnly) {
    HPEN Pen2 = CreatePen(PS_SOLID, 1, COL_CIRCLE);
    SelectObject(hdc, Pen2);

    HBRUSH br = CreateSolidBrush(COL_CIRCLE);
    HGDIOBJ oldBrush = SelectObject(hdc, br);

    for(int i = 0; i < (int)MAX_NUM_POINT; ++i) {
      POINT p = { i + 1, (int)Wave[i] };
      restore_point(p, h, y);
      PRect r((int)(p.x - RADIUS), (int)(p.y - RADIUS), (int)(p.x + RADIUS), (int)(p.y + RADIUS));
      Ellipse(hdc, r.left, r.top, r.right, r.bottom);
      }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);

    DeleteObject(br);
    DeleteObject(Pen2);
    }
  else
    SelectObject(hdc, oldPen);
  DeleteObject(Pen);
}
//----------------------------------------------------------------------------
int PVarCurve::check_point(const POINT& point)
{
  POINT pt = point;
  trasf_point(pt);

  int y = pt.y;
  if(y >= Wave[pt.x - 1] - MUL_DEC(RADIUS) && y <= Wave[pt.x - 1] + MUL_DEC(RADIUS)) {
    int x = point.x;
    for(int i = 0; i < (int)MAX_NUM_POINT; ++i)
      if(x >= stepX[i] - RADIUS * 2 && x <= stepX[i] + RADIUS * 2)
        return 1;
    }
  return 0;
}
//----------------------------------------------------------------------------
void PVarCurve::MouseMove(uint modKeys, POINT point, int ok_point)
{
  static int old;
  HCURSOR curs = 0;

  if(ok_point) {
    if(old && Hand == 2)
      curs = LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_MY_HAND));
    EvMouseMove (modKeys, point);
    old = 0;
    }
  else if(Hand) {
    old = 1;
    curs = LoadCursor(0, IDC_ARROW);
    if(Hand == 1)
      Hand = 0;
    }
  if(curs)
    SetCursor(curs);
}
//----------------------------------------------------------------------------
void PVarCurve::new_ancor(const POINT& point, int rem_old)
{
  if(point.y != oldy) {
    HDC hdc = GetDC(*this);

    int old = SetROP2(hdc, R2_XORPEN);

    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0xff, 0xff, 0xff));
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    HBRUSH br = CreateSolidBrush(RGB(0xff, 0xff, 0xff));
    HGDIOBJ oldBrush = SelectObject(hdc, br);

    if(rem_old) {
      POINT p = { oldx, oldy };
      restore_point(p);
      Ellipse(hdc, (int)(p.x - RADIUS), (int)(p.y - RADIUS), (int)(p.x + RADIUS), (int)(p.y + RADIUS));
      }
    oldy = point.y;
    POINT p = { oldx, oldy };
    restore_point(p);
    Ellipse(hdc, (int)(p.x - RADIUS), (int)(p.y - RADIUS), (int)(p.x + RADIUS), (int)(p.y + RADIUS));

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);

    DeleteObject(br);
    DeleteObject(pen);

    SetROP2(hdc, old);
    ReleaseDC(*this, hdc);
   }
}
//----------------------------------------------------------------------------
void PVarCurve::draw_line(const POINT& point)
{
  HDC hdc = GetDC(*this);

  int old = SetROP2(hdc, R2_XORPEN);

  HPEN pen = CreatePen(PS_SOLID, 1, RGB(0xff, 0xff, 0xff));
  HGDIOBJ oldPen = SelectObject(hdc, pen);

  POINT p = { oldx, oldy };
  restore_point(p);
  MoveToEx(hdc, p.x, p.y, 0);
  p = point;
  restore_point(p);
  LineTo(hdc, p.x, p.y);

  SelectObject(hdc, oldPen);
  DeleteObject(pen);

  SetROP2(hdc, old);
  ReleaseDC(*this, hdc);
}
//----------------------------------------------------------------------------
void PVarCurve::EvMouseMove (uint modKeys, POINT point)
{
  if(2 == Hand || 3 == Hand)
    trasf_point(point);
  if(Hand == 2) {
    new_ancor(point);
    }
  else if(Hand == 3) {
    if(point.x != oldx || point.y != oldy) {
      draw_line(point);
      if(abs(oldx - point.x) > 1) {
        double step = point.y - oldy;
        step /= abs(oldx - point.x);
        int j = oldx < point.x ? 1 : -1;
        double ty = point.y;
        for(int i = oldx; i != point.x; i += j, ty += step)
          Wave[i - 1] = ty;
        }
      Wave[point.x - 1] = point.y;
      oldx = point.x;
      oldy = point.y;
      }
    }
  else if(check_point(point)) {
    if(!Hand) {
      HCURSOR curs = LoadCursor(0, IDC_CROSS);
      SetCursor(curs);
      Hand = 1;
      }
    }
  else if(Hand == 1) {
    HCURSOR curs = LoadCursor(0, IDC_ARROW);
    SetCursor(curs);
    Hand = 0;
    }
}
//----------------------------------------------------------------------------
void PVarCurve::EvMouseLDn (uint modKeys, POINT point)
{
  if(check_point(point)) {
    HCURSOR curs = LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_MY_HAND));
    SetCursor(curs);

    trasf_point(point);
    Hand = 2;
    oldy = -1;
    oldx = point.x;
    new_ancor(point, 0);
    }
}
//----------------------------------------------------------------------------
void PVarCurve::EvMouseLUp (uint modKeys, POINT point)
{
  HCURSOR curs;
  if(check_point(point)) {
    curs = LoadCursor(0, IDC_CROSS);
    Hand = 1;
    }
  else {
    curs = LoadCursor(0, IDC_ARROW);
    Hand = 0;
    }
  SetCursor(curs);
  if(oldy != -1) {
    Wave[oldx - 1] = oldy;
    InvalidateRect(*this, 0, 1);
    oldy = -1;
    sendCurrData();
    }
}
//----------------------------------------------------------------------------
void PVarCurve::EvMouseRDn (uint modKeys, POINT point)
{
  trasf_point(point);
  oldx = point.x;
  oldy = point.y;
  Wave[oldx - 1] = oldy;
  HCURSOR curs = LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_PENCIL));
  SetCursor(curs);
  Hand = 3;
}
//----------------------------------------------------------------------------
void PVarCurve::EvMouseRUp (uint modKeys, POINT point)
{
  HCURSOR curs;
  if(check_point(point)) {
    curs = LoadCursor(0, IDC_CROSS);
    Hand = 1;
    }
  else {
    curs = LoadCursor(0, IDC_ARROW);
    Hand = 0;
    }
  SetCursor(curs);
  oldy = -1;
  InvalidateRect(*this, 0, 1);
  sendCurrData();
}
//----------------------------------------------------------------------------

