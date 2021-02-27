//---------- lgraph2-zoom.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "macro_utils.h"
#include "lgraph2-zoom.h"
#include "pbutton.h"
#include "1.h"
//----------------------------------------------------------------------------
LRESULT manage_coord::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_MOUSEMOVE:
      if(evMouseMove(hwnd, wParam, lParam))
        return -1;
      return 0;

    case WM_LBUTTONDOWN:
      if(evMouseLDown(hwnd, wParam, lParam))
        return -1;
      return 0;

    case WM_LBUTTONUP:
      return evMouseLUp(hwnd, wParam, lParam);
    }
  return 0;
}
//-----------------------------------------------------------
static bool makeInside(POINT& pt, const PRect& rect)
{
  bool isExt = false;
  if(pt.x < rect.left) {
    pt.x = rect.left;
    isExt = true;
    }
  else if(pt.x > rect.right) {
    pt.x = rect.right;
    isExt = true;
    }
  if(pt.y < rect.top) {
    pt.y = rect.top;
    isExt = true;
    }
  else if(pt.y > rect.bottom) {
    pt.y = rect.bottom;
    isExt = true;
    }
  return isExt;
}
//-----------------------------------------------------------
static void drawFocusRect(HDC hdc, const PRect& norm)
{
#if 1
  HPEN pen = CreatePen(PS_DOT, 1, RGB(127, 127, 127));
  HGDIOBJ oldPen = SelectObject(hdc, pen);
  int old = SetROP2(hdc, R2_NOTXORPEN);
  PRect r(norm);
  MoveToEx(hdc, r.left, r.top, 0);
  LineTo(hdc, r.left, r.bottom);
  LineTo(hdc, r.right, r.bottom);
  LineTo(hdc, r.right, r.top);
  LineTo(hdc, r.left, r.top);
  SelectObject(hdc, oldPen);
  SetROP2(hdc, old);
  DeleteObject(pen);
#else
  DrawFocusRect(hdc, norm);
#endif
}
//-----------------------------------------------------------
bool manage_coord::evMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };

  if(!makeInside(pt, inside))
    SetCursor(LoadCursor(0, IDC_SIZEALL));
  switch(Stat) {
    case eClicked:
      Stat = eOnDrag;
      // fall through
    case eOnDrag:
      break;
//    case eReleased:
//      Stat = eNoStat;
//      break;
    default:
      return false;
    }
  HDC hdc = GetDC(hwnd);
  // se il rettangolo non è vuoto occorre cancellare il disegno precedente
  if(frame.Width() || frame.Height()) {
    PRect norm(frame);
    norm.Normalize();
    drawFocusRect(hdc, norm);
    }

  // assegna nuova posizione
  frame.right = pt.x;
  frame.bottom = pt.y;

  if(frame.Width() || frame.Height()) {
    PRect norm(frame);
    norm.Normalize();
    drawFocusRect(hdc, norm);
    }
  ReleaseDC(hwnd, hdc);
  return true;
}
//-----------------------------------------------------------
bool manage_coord::evMouseLDown(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  if(eClicked == Stat)
    return true;

  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  if(makeInside(pt, inside))
    return false;

  Stat = eClicked;
  frame.left = pt.x;
  frame.top = pt.y;
  frame.right = pt.x;
  frame.bottom = pt.y;
  return true;
}
//-----------------------------------------------------------
#define MIN_DIM_4_ZOOM 10
//-----------------------------------------------------------
bool manage_coord::evMouseLUp(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  if(eClicked == Stat) {
    Stat = eNoStat;
    return false;
    }
  if(eOnDrag != Stat) {
    Stat = eNoStat;
    return false;
    }

  HDC hdc = GetDC(hwnd);
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  makeInside(pt, inside);

  PRect norm(frame);
  norm.Normalize();
  drawFocusRect(hdc, norm);
  ReleaseDC(hwnd, hdc);
  Stat = eNoStat;
  return norm.Width() >= MIN_DIM_4_ZOOM && norm.Height() >= MIN_DIM_4_ZOOM;
}
//----------------------------------------------------------------------------
static uint calcInfoValue(HFONT hf);
static uint calcInfoDate();
//----------------------------------------------------------------------------
inline lgCoord rescale(lgCoord val, double scale)
{
  return (lgCoord)(val * scale);
}
//----------------------------------------------------------------------------
/*
  lungo la X i valori rappresentano il pixel (inizio zero) moltiplicato per MUL_W_DATA

  lungo la Y i valori vanno da zero (valore massimo) a MAX_H_DATA (valore minimo)
*/
//----------------------------------------------------------------------------
lgraph2_zoom::lgraph2_zoom(PWin* parent, const zoomInfo& sourceInfo)
:
    baseClass(parent, IDD_BASE_MODAL_NO_CAPTION), XCoord(0),
    totPoint(0), bkgColor(sourceInfo.bkg)
{
  hFont = sourceInfo.hFont;
  nLabel = sourceInfo.nLabel;
  Dec = sourceInfo.Dec;
  textColor = sourceInfo.textColor;

  double diffTime = (double)(cMK_I64(sourceInfo.end) - cMK_I64(sourceInfo.init));
  double ratioPix = diffTime / sourceInfo.full.Width();

  ftInit = I64_TO_FT((_int64)((sourceInfo.sel.left - sourceInfo.full.left) * ratioPix + cMK_I64(sourceInfo.init)));
  ftEnd = I64_TO_FT((_int64)((sourceInfo.sel.right - sourceInfo.full.left) * ratioPix + cMK_I64(sourceInfo.init)));


  lgCoord result = (lgCoord)((sourceInfo.sel.left - sourceInfo.full.left) * MUL_W_DATA);
  lgCoord offs = result;
  DWORD start = 0;
  while(start < sourceInfo.totPoint && result > sourceInfo.sourceX[start])
    ++start;

  result = (lgCoord)((sourceInfo.sel.right - sourceInfo.full.left) * MUL_W_DATA);
  DWORD stop = start;
  while(stop < sourceInfo.totPoint && result >= sourceInfo.sourceX[stop])
    ++stop;

  totPoint = stop - start;

  XCoord = new lgCoord[totPoint];

  int cx = GetSystemMetrics(SM_CXSCREEN);
  int cy = GetSystemMetrics(SM_CYSCREEN);

  gfxRect.left = sourceInfo.nLabel ? calcInfoValue(hFont) : R__X(5);
  gfxRect.right = cx - R__X(10);

  gfxRect.top = R__Y(5);
  gfxRect.bottom = cy - calcInfoDate();
  double scaleX = gfxRect.Width();
  scaleX /= sourceInfo.sel.Width();

  uint nGrfx = sourceInfo.infoVar.getElem();
  VarInfo.setDim(nGrfx);

  DWORD tstart = start;

  for(uint i = 0; i < totPoint; ++i, ++tstart)
    XCoord[i] = rescale(sourceInfo.sourceX[tstart] - offs, scaleX);

  for(uint g = 0; g < nGrfx; ++g) {
    tstart = start;
    const zoomInfoVar& ziv = sourceInfo.infoVar[g];

    double range = ziv.maxVal - ziv.minVal;

    double min_val = sourceInfo.full.bottom - sourceInfo.sel.bottom;
    min_val /= sourceInfo.full.Height();
    min_val *= range;
    double tminVal = min_val + ziv.minVal;

    double tmaxVal = sourceInfo.sel.top - sourceInfo.full.top;
    tmaxVal /= sourceInfo.full.Height();
    tmaxVal *= range;
    tmaxVal = ziv.maxVal - tmaxVal;

    double newRange = tmaxVal - tminVal;
    if(sourceInfo.ixActive == g) {
      minVal = tminVal;
      maxVal = tmaxVal;
      }
    VarInfo[g].yGraph = new lgCoord[totPoint];

    for(uint i = 0; i < totPoint; ++i, ++tstart) {
      double t = MAX_H_DATA - ziv.sourceY[tstart];
      t = t / MAX_H_DATA * range - min_val;
      if(t < 0)
        VarInfo[g].yGraph[i] = MAX_H_DATA;
      else {
        t = MAX_H_DATA - (t * MAX_H_DATA / newRange);
        if(t < 1)
          t = 1;
        else if( t > MAX_H_DATA)
          t = MAX_H_DATA;
        VarInfo[g].yGraph[i] = (lgCoord)t;
        }
      }

    VarInfo[g].lGraph = new linear(gfxRect, 0, PPanel::NO, ziv.frg, false);

    VarInfo[g].lGraph->setBuff(XCoord, VarInfo[g].yGraph, totPoint);
    }
}
//----------------------------------------------------------------------------
lgraph2_zoom::~lgraph2_zoom()
{
  destroy();
  uint nElem = VarInfo.getElem();
  for(uint i = 0; i < nElem; ++i) {
    delete VarInfo[i].lGraph;
    delete []VarInfo[i].yGraph;
    }
  delete []XCoord;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool lgraph2_zoom::create()
{
  int cx = GetSystemMetrics(SM_CXSCREEN);
  int cy = GetSystemMetrics(SM_CYSCREEN);

  PButton* ok = new PButton(this, IDC_BUTTON_F1, 1, 1, 80, 22, _T("F1"));
  ok->Attr.style |= BS_DEFPUSHBUTTON;
  if(!baseClass::create())
    return false;
  setWindowTextByLangGlob(*ok, ID_DIALOG_OK, false);

  SetWindowPos(*this, 0, 0, 0, cx, cy, SWP_NOZORDER | SWP_SHOWWINDOW);
  return true;
}
//----------------------------------------------------------------------------
bool lgraph2_zoom::preProcessMsg(MSG& msg)
{
  return toBool(IsDialogMessage(getHandle(), &msg));
}
//----------------------------------------------------------------------------
LRESULT lgraph2_zoom::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
        case IDC_BUTTON_F2:
          CmCancel();
          break;
        }
      break;

    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if(!hdc) {
          EndPaint(hwnd, &ps);
          break;
          }

#ifdef USE_MEMDC

        if(isReady) {
          PRect r;
          GetClientRect(hwnd, r);
          if(!hBmpWork) {
            allocBmpWork(hdc);
            }
          HBRUSH br = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
          FillRect(mdcWork, r, br);
          evPaint(mdcWork);
          BitBlt(hdc, r.left, r.top, r.Width(), r.Height(), mdcWork, r.left, r.top, SRCCOPY);
          }
#else
        evPaint(hdc);
#endif
        EndPaint(hwnd, &ps);

        } while(false);

      return 0;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
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
static
uint calcInfoValue(HFONT hf)
{
  HDC hdc = GetDC(0);
  HGDIOBJ old = hf ? SelectObject(hdc, hf) : 0;
  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);
  if(old)
    SelectObject(hdc, old);
  ReleaseDC(0, hdc);

  return (uint)(tm.tmAveCharWidth * 10);
/*
  double cx = GetSystemMetrics(SM_CXSCREEN);
  cx /= 800;
  return (uint)(cx * 80);
*/
}
//----------------------------------------------------------------------------
static
uint calcInfoDate()
{
  return R__Y(H_LINE_TXT) + R__Y(DIM_F * 3);
}
//----------------------------------------------------------------------------
void lgraph2_zoom::evPaint(HDC hdc)
{
  if(!VarInfo.getElem())
    return;

  PRect rect(gfxRect);

  PPanel(rect, bkgColor, PPanel::DN_FILL).paint(hdc);

  int y_base = rect.bottom + 2;
  int y_line1 = rect.bottom + R__Y(H_LINE1);
  int y_line2 = rect.bottom + R__Y(H_LINE2);
  int y_txt = rect.bottom + R__Y(H_LINE_TXT);
  int y_txt2 = y_txt + R__Y(DIM_F - 2);

  int nGroupDate = DEF_GROUP_DATE;

  HPEN p1 = CreatePen(PS_SOLID, 1, textColor);
  HPEN p2 = CreatePen(PS_SOLID, 1, RGB(160, 160, 160));

  HPEN pBlack = CreatePen(PS_SOLID, 1, 0);

  HGDIOBJ oldPen = SelectObject(hdc, p1);
  int x1 = rect.left;
  __int64 ft_init = MK_I64(ftInit);
  __int64 ft_end = MK_I64(ftEnd);

  double step = (double)(ft_end - ft_init);

  if(step < 0)
    step = 0;

  step /= N_TOT_INFO_DATE / N_INFO_DATE;
  double fixX = (double)rect.Width() / (double)(N_TOT_INFO_DATE - 1);
  HGDIOBJ oldFont = hFont ? SelectObject(hdc, hFont) : 0;

  int oldMode = SetBkMode(hdc, TRANSPARENT);

  UINT oldAlign = SetTextAlign(hdc, TA_LEFT | TA_TOP);

  TCHAR t[50];
  LPCTSTR sep = _T("|");

  SetTextColor(hdc, textColor);

  for(int i = 0; i < N_TOT_INFO_DATE; ++i) {
    int x = (int)(fixX * i + x1);
    if(!(i % N_INFO_DATE)) {
      if(!i)
        SetTextAlign(hdc, TA_LEFT | TA_TOP);
      else {
        SelectObject(hdc, p2);
        MoveToEx(hdc, x, rect.top + 1, 0);
        LineTo(hdc, x, rect.bottom - 1);
        SelectObject(hdc, p1);
        SetTextAlign(hdc, TA_CENTER | TA_TOP);
        }
      bool drawDate = true;
      if(nGroupDate > DEF_GROUP_DATE)
        drawDate = !i || !((i / N_INFO_DATE) % (nGroupDate / DEF_GROUP_DATE));
      if(drawDate) {
        set_format_data(t, SIZE_A(t), I64_TO_FT(ft_init), whichData(), sep);
        t[10] = 0;
        int len = _tcslen(t);
        TextOut(hdc, x, y_txt, t, len);
        LPTSTR t2 = t + len + 1;
        TextOut(hdc, x, y_txt2, t2, _tcslen(t2));
        }
      ft_init += (__int64)step;
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

  set_format_data(t, SIZE_A(t), I64_TO_FT(ft_init), whichData(), sep);
  t[10] = 0;
  int len = _tcslen(t);
  TextOut(hdc, x - 4, y_txt, t, len);
  LPTSTR t2 = t + len + 1;
  TextOut(hdc, x - 4, y_txt2, t2, _tcslen(t2));

  if(oldFont)
    SelectObject(hdc, oldFont);

  SelectObject(hdc, oldPen);
  SetTextAlign(hdc, oldAlign);
  DeleteObject(pBlack);
  DeleteObject(p2);
  DeleteObject(p1);
  SetBkMode(hdc, oldMode);

  drawYInfo(hdc);
  uint nElem = VarInfo.getElem();
  for(uint i = 0; i < nElem; ++i) {
    VarInfo[i].lGraph->rewind();
    VarInfo[i].lGraph->draw(hdc);
    }
}
//--------------------------------------------------------------------
void lgraph2_zoom::drawYInfo(HDC hdc)
{
  double range = maxVal - minVal;
  if(range <= 0)
    return;

  PRect rect(gfxRect);
  int oldMode = SetBkMode(hdc, TRANSPARENT);
/*
  int hGraph = rect.Height();
  static fREALDATA deltaY;
  if(!deltaY) {
    fREALDATA dyVal = range / nRow;
    switch(typeInfo) {
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
      case 1:
        deltaY = nRow * (float)hGraph / range;
        break;
      default:
        return;
      }
    }
  if(!deltaY || !nRow)
    return;

  fREALDATA offsY = 0;
  if(minVal < 0)
    offsY = -minVal * (float)hGraph / range;
  else {
    fREALDATA t = nRow - minVal;
    offsY = deltaY * t / nRow;
    }
  if(offsY < 0)
    offsY = 0;

  HPEN pen = CreatePen(PS_DASH, 1, RGB(160, 160, 160));
  HGDIOBJ oldPen = SelectObject(hdc, pen);
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
*/
  if(nLabel) {
    HPEN pBlack = CreatePen(PS_SOLID, 1, textColor);
    HPEN pen = CreatePen(PS_DASH, 1, RGB(160, 160, 160));

    HGDIOBJ old_Pen = SelectObject(hdc, pBlack);
    SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
    fREALDATA stepVal = (fREALDATA)(maxVal - minVal);
    stepVal /= nLabel;
    fREALDATA step = (fREALDATA)rect.Height();
    step /= nLabel;
    fREALDATA val = (fREALDATA)minVal;
    fREALDATA y = (fREALDATA)rect.bottom;
    int x1 = 1;
    int x2 = rect.left - R__X(5);
    int x2a = rect.left + R__X(5);
    int x3 = rect.right - R__X(5);
    for(int i = 0; i < nLabel; ++i) {
      MoveToEx(hdc, x1, (int)y, 0);
      LineTo(hdc, x2, (int)y);
      SelectObject(hdc, pen);
      MoveToEx(hdc, x2a, (int)y, 0);
      LineTo(hdc, x3, (int)y);
      SelectObject(hdc, pBlack);
      TCHAR buff[50];
      makeFixedString(val, buff, SIZE_A(buff), Dec);
      TextOut(hdc, x2, (int)y, buff, _tcslen(buff));
      y -= step;
      val += stepVal;
      }
    SelectObject(hdc, old_Pen);
    DeleteObject(pBlack);
    DeleteObject(pen);
    }
//  SelectObject(hdc, oldPen);
//  DeleteObject(pen);
  SetBkMode(hdc, oldMode);
}
//----------------------------------------------------------------------------
lgraph2_ShowValue::lgraph2_ShowValue(PWin* parent, const infoPointedValue& sourceInfo) :
  baseClass(parent, IDD_LGRAPH_SHOWVALUE), sourceInfo(sourceInfo)
{
  for(uint i = 0; i < sourceInfo.nValue; ++i)
    hBrush[i] = CreateSolidBrush(sourceInfo.color[i]);
}
//----------------------------------------------------------------------------
lgraph2_ShowValue::~lgraph2_ShowValue()
{
  destroy();
  for(uint i = 0; i < sourceInfo.nValue; ++i)
    DeleteObject(hBrush[i]);
}
//----------------------------------------------------------------------------
bool lgraph2_ShowValue::create()
{
  if(!baseClass::create())
    return false;
  uint idcN[] = {
    IDC_STATIC_SV_NAME1,
    IDC_STATIC_SV_NAME2,
    IDC_STATIC_SV_NAME3,
    IDC_STATIC_SV_NAME4,
    IDC_STATIC_SV_NAME5,
    IDC_STATIC_SV_NAME6,
    IDC_STATIC_SV_NAME7,
    IDC_STATIC_SV_NAME8,
    IDC_STATIC_SV_NAME9,
    IDC_STATIC_SV_NAME10,
    };
  uint idcV[] = {
    IDC_STATIC_SV_VALUE1,
    IDC_STATIC_SV_VALUE2,
    IDC_STATIC_SV_VALUE3,
    IDC_STATIC_SV_VALUE4,
    IDC_STATIC_SV_VALUE5,
    IDC_STATIC_SV_VALUE6,
    IDC_STATIC_SV_VALUE7,
    IDC_STATIC_SV_VALUE8,
    IDC_STATIC_SV_VALUE9,
    IDC_STATIC_SV_VALUE10,
    };
  for(uint i = 0; i < sourceInfo.nValue; ++i) {
    SetDlgItemText(*this, idcN[i], sourceInfo.name[i]);
    TCHAR t[64];
    _stprintf_s(t, _T("%0.5f"), sourceInfo.value[i]);
    zeroTrim(t);
    SetDlgItemText(*this, idcV[i], t);
    }
  TCHAR buff[64];
  set_format_data(buff, SIZE_A(buff), sourceInfo.ft, whichData(), _T(" - "));
  SetDlgItemText(*this, IDC_STATIC_SV_TIME, buff);
  return true;
}
//----------------------------------------------------------------------------
bool lgraph2_ShowValue::preProcessMsg(MSG& msg)
{
  return toBool(IsDialogMessage(getHandle(), &msg));
}
//----------------------------------------------------------------------------
LRESULT lgraph2_ShowValue::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
        case IDC_BUTTON_F2:
          CmCancel();
          break;
        }
      break;
/*
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
*/
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HBRUSH lgraph2_ShowValue::evCtlColor(HDC hdc, HWND hwnd, uint /*message*/)
{
  const uint idcColor[] = {
    IDC_STATIC_SV_VALUE_COLOR1,
    IDC_STATIC_SV_VALUE_COLOR2,
    IDC_STATIC_SV_VALUE_COLOR3,
    IDC_STATIC_SV_VALUE_COLOR4,
    IDC_STATIC_SV_VALUE_COLOR5,
    IDC_STATIC_SV_VALUE_COLOR6,
    IDC_STATIC_SV_VALUE_COLOR7,
    IDC_STATIC_SV_VALUE_COLOR8,
    IDC_STATIC_SV_VALUE_COLOR9,
    IDC_STATIC_SV_VALUE_COLOR10,
    };
  int id = ::GetDlgCtrlID(hwnd);
  for(uint i = 0; i < sourceInfo.nValue; ++i) {
    if(idcColor[i] == id) {
      SetBkColor(hdc, sourceInfo.color[i]);
      return hBrush[i];
      }
    }
  return 0;
}
