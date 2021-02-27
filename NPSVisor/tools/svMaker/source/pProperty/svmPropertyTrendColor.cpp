//----------- svmPropertyTrendColor.cpp --------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdio.h>
//----------------------------------------------------------------------------
#include "Macro_utils.h"
#include "svmPropertyTrendColor.h"
#include "p_Util.h"
#include "headerMsg.h"
#include "id_msg_common.h"
#include "svmProperty.h"
#include "plistbox.h"
//----------------------------------------------------------------------------
#define IDC_EDIT_GR_R_1 10000
#define FIRST_ID_ROW  IDC_EDIT_GR_R_1
#define TEXT_ID_ROW   (IDC_EDIT_GR_R_1 - 2)
//----------------------------------------------------------------------------
#define OFFSET_ID_ROW     40
#define OFFSET_ID_CLR     (OFFSET_ID_ROW * 3)
#define OFFSET_ID_PT_SCR  (OFFSET_ID_ROW * 4)
//----------------------------------------------------------------------------
#define INIT_X   7
#define INIT_Y  128
//#define INIT_Y  80
//----------------------------------------------------------------------------
#define WIDTH   28
#define HEIGHT  20
//----------------------------------------------------------------------------
#define OFFS_WIDTH   180
//#define OFFS_WIDTH   120
//----------------------------------------------------------------------------
#define D_X (WIDTH)
#define D_Y (HEIGHT + 1)
//----------------------------------------------------------------------------
//#define MAX_BLK 2
#define MAX_BLK 11
//----------------------------------------------------------------------------
PSimpleColorRowTrend::PSimpleColorRowTrend(PWin* parent, uint first_id, int x) :
    firstId(first_id), Focus(false)
{
  PRect r(0, 0, WIDTH, HEIGHT);
  r.MoveTo(x, INIT_Y + ((first_id - FIRST_ID_ROW) % MAX_BLK) * D_Y);
  for(uint i = 0; i < SIZE_A(rgb); ++i) {
    rgb[i] = new P_EditFocus(parent, firstId + i * OFFSET_ID_ROW, r, 0, 3);
    rgb[i]->setFilter(new PRGBFilter);
    r.Offset(D_X, 0);
    }
  r.Offset(2, 0);
  cbx = new PColorBox(parent, 0, firstId + OFFSET_ID_CLR, r);
}
//----------------------------------------------------------------------------
PSimpleColorRowTrend::checkFocus PSimpleColorRowTrend::setFocus(uint firstIdSB, HWND hwnd)
{
  bool focus = hwnd ? false : true;
  if(!focus)
    for(uint i = 0; i < SIZE_A(rgb); ++i)
      if(hwnd == *rgb[i]) {
        focus = true;
        break;
        }
  if(!focus && Focus) {
    Focus = false;
    return PSimpleColorRowTrend::LOSS;
    }

  if(!hwnd || focus && !Focus) {
    PWin* parent = rgb[0]->getParent();
    for(int i = 0; i < 3; ++i) {
      HWND hsb = GetDlgItem(*parent, firstIdSB + i);
      int val[3];
      if(hsb) {
        TCHAR buff[20];
        GetWindowText(*rgb[i], buff, SIZE_A(buff));
        val[i] = _ttoi(buff);
        SendMessage(hsb, SBM_SETPOS, val[i], true);
        }
      svmPropertyTrendColor* cg = dynamic_cast<svmPropertyTrendColor*>(parent);
      if(cg)
        cg->setCurrColor(RGB(val[0], val[1], val[2]));
      }
    Focus = true;
    return PSimpleColorRowTrend::GET;
    }
  return PSimpleColorRowTrend::NOCHANGE;
}
//----------------------------------------------------------------------------
COLORREF PSimpleColorRowTrend::getColor()
{
  int val[3];
  TCHAR buff[10];
  for(int i = 0; i < 3; ++i) {
    GetWindowText(*rgb[i], buff, SIZE_A(buff));
    val[i] = _ttoi(buff);
    }
  return RGB(val[0], val[1], val[2]);
}
//----------------------------------------------------------------------------
void PSimpleColorRowTrend::setColorBySelf(uint firstIdSB)
{
  int val[3];
  TCHAR buff[10];
  PWin* parent = rgb[0]->getParent();
  for(int i = 0; i < 3; ++i) {
    GetWindowText(*rgb[i], buff, SIZE_A(buff));
    val[i] = _ttoi(buff);

    HWND hsb = GetDlgItem(*parent, firstIdSB + i);
    if(hsb)
      SendMessage(hsb, SBM_SETPOS, val[i], true);
    }
  cbx->set(RGB(val[0], val[1], val[2]));
}
//----------------------------------------------------------------------------
void PSimpleColorRowTrend::setColor(COLORREF color)
{
  int val[3] = {
      GetRValue(color),
      GetGValue(color),
      GetBValue(color)
      };
  TCHAR buff[10];
  for(int i = 0; i < 3; ++i) {
    wsprintf(buff, _T("%d"), val[i]);
    SetWindowText(*rgb[i], buff);
    }
  cbx->set(color);
}
//----------------------------------------------------------------------------
void PSimpleColorRowTrend::setAll(LPCTSTR p)
{
  int r = 0;
  int g = 0;
  int b = 0;
  _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
  setColor(RGB(r, g, b));
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PColorRowTrend::PColorRowTrend(PWin* parent, uint first_id, int x) :
    baseClass(parent, first_id, x)
{
  PRect r(0, 0, WIDTH, HEIGHT);
  r.MoveTo(x, INIT_Y + ((first_id - FIRST_ID_ROW) % MAX_BLK) * D_Y);
  r.Offset(D_X * (SIZE_A(rgb) + 1) + 6, 0);

  ptScr = new P_EditFocus(parent, firstId + OFFSET_ID_PT_SCR, r, 0, 3);
}
//----------------------------------------------------------------------------
/*
PColorRowTrend::~PColorRowTrend()
{
  delete cbx;
}
*/
//----------------------------------------------------------------------------
PSimpleColorRowTrend::checkFocus PColorRowTrend::setFocus(uint firstIdSB, HWND hwnd)
{
  if(hwnd) {
    bool focus = toBool(hwnd == *ptScr);
    if(focus)
      return baseClass::setFocus(firstIdSB, *rgb[0]);
    }
  return baseClass::setFocus(firstIdSB, hwnd);
}
//----------------------------------------------------------------------------
WORD PColorRowTrend::getPtScr()
{
  TCHAR buff[10];
  GetWindowText(*ptScr, buff, SIZE_A(buff));
  return (WORD)_ttoi(buff);
}
//----------------------------------------------------------------------------
void PColorRowTrend::setPtScr(WORD val)
{
  TCHAR buff[10];
  wsprintf(buff, _T("%d"), val);
  SetWindowText(*ptScr, buff);
}
//----------------------------------------------------------------------------
void PColorRowTrend::setAll(LPCTSTR p)
{
  int r = 0;
  int g = 0;
  int b = 0;
  int scr = 2;
  _stscanf_s(p, _T("%d,%d,%d,%d"), &scr, &r, &g, &b);
  setColor(RGB(r, g, b));
  setPtScr(scr);
}
//----------------------------------------------------------------------------
svmPropertyTrendColor::svmPropertyTrendColor(PWin* parent, PVect<colorLine>& cl, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), ColorLine(cl)
{
  uint nElem = cl.getElem();

  int x = INIT_X;
  for(uint i = 0; i < nElem && i < MAX_BLK; ++i)
    Rows[i] = new PColorRowTrend(this, IDC_EDIT_GR_R_1 + i, x);

  x += OFFS_WIDTH;
  for(uint i = MAX_BLK; i < nElem && i < MAX_BLK * 2; ++i)
    Rows[i] = new PColorRowTrend(this, IDC_EDIT_GR_R_1 + i, x);

  x += OFFS_WIDTH;
  for(uint i = MAX_BLK * 2; i < nElem && i < MAX_BLK * 3; ++i)
    Rows[i] = new PColorRowTrend(this, IDC_EDIT_GR_R_1 + i, x);

  COLORREF base[] = { RGB(0xff, 0, 0), RGB(0, 0xff, 0), RGB(0, 0, 0xff) };
  for(uint i = 0; i < SIZE_A(BaseColor); ++i)
    BaseColor[i] = new PColorBox(this, base[i], IDC_STATIC_GR_BASE_R + i, getHInstance());
  CurrColor = new PColorBox(this, RGB(0,0,0), IDC_STATIC_GR_CURR_COLOR, getHInstance());
}
//----------------------------------------------------------------------------
svmPropertyTrendColor::~svmPropertyTrendColor()
{
  flushPV(Rows);
  destroy();
}
//----------------------------------------------------------------------------
bool svmPropertyTrendColor::create()
{
  if(!baseClass::create())
    return false;

  uint nElem = ColorLine.getElem();
  for(uint i = 0; i < nElem; ++i) {
    Rows[i]->setColor(ColorLine[i].Color);
    Rows[i]->setPtScr(ColorLine[i].tickness);
    }
  int needReduce = 0;
  if(nElem <= MAX_BLK)
    needReduce = 2;
  else if(nElem <= MAX_BLK * 2)
    needReduce = 1;

  PRect r;
  GetWindowRect(*this, r);
  if(needReduce)
    r.right -= OFFS_WIDTH * needReduce;
  if(nElem > MAX_BLK)
    nElem = MAX_BLK;
  r.bottom += nElem * D_Y;
  SetWindowPos(*this, 0, 0, 0, r.Width(), r.Height(), SWP_NOMOVE | SWP_NOZORDER);
  for(int i = 0; i < 3; ++i) {
    HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_GR_R + i);
    if(hsb)
      SendMessage(hsb, SBM_SETRANGE , 0, 255);
    }

  Rows[0]->setFocus(IDC_SCROLLBAR_GR_R);

  return true;
}
//----------------------------------------------------------------------------
HWND svmPropertyTrendColor::getFirstFocus()
{
  return GetDlgItem(*this, IDC_EDIT_GR_R_1);
}
//----------------------------------------------------------------------------
LRESULT svmPropertyTrendColor::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CUSTOM_SET_FOCUS:
      do {
        HWND focus = GetFocus();
        uint nElem = Rows.getElem();
        for(uint i = 0; i < nElem; ++i)
          Rows[i]->setFocus(IDC_SCROLLBAR_GR_R, focus);
        } while(false);
      break;


    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case EN_CHANGE:
          do {
            int id = LOWORD(wParam);
            if(IDC_EDIT_GR_R_1 <= id && id < IDC_EDIT_GR_R_1 + OFFSET_ID_CLR) {
              id -= IDC_EDIT_GR_R_1;
              id %= OFFSET_ID_ROW;
              Rows[id]->setColorBySelf(IDC_SCROLLBAR_GR_R);
              }
            } while(false);
          break;
        }
      break;
    case WM_HSCROLL:
      evHScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
      break;

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
//------------------------------------------------------------------------------
void svmPropertyTrendColor::evHScrollBar(HWND child, int flags, int pos)
{
  int curr = SendMessage(child, SBM_GETPOS, 0, 0);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = 255;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - 16;
      break;
    case SB_PAGERIGHT:
      pos = curr + 16;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if(pos > 255)
    pos = 255;
  SendMessage(child, SBM_SETPOS, pos, true);

  int val[3];
  for(int i = 0; i < 3; ++i) {
    HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_GR_R + i);
    val[i] = SendMessage(hsb, SBM_GETPOS, 0, 0);
    }
  PSimpleColorRowTrend* cr = 0;
  uint nElem = Rows.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(Rows[i]->hasFocus()) {
      cr = Rows[i];
      break;
      }
    }
  if(cr) {
    cr->setColor(RGB(val[0], val[1], val[2]));
    CurrColor->set(RGB(val[0], val[1], val[2]));
    }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void svmPropertyTrendColor::CmOk()
{
  int nElem = Rows.getElem();
  for(int i = 0; i < nElem; ++i) {
    ColorLine[i].Color = Rows[i]->getColor();
    ColorLine[i].tickness = Rows[i]->getPtScr();
    }
  baseClass::CmOk();
}
//------------------------------------------------------------------------------
HBRUSH svmPropertyTrendColor::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    int nElem = Rows.getElem();
    for(int i = 0; i < nElem; ++i) {
      PColorBox* box = Rows[i]->getBox();
      if(*box == hWndChild) {
        SetBkColor(hdc, box->getColor());
        return box->getBrush();
        }
      }
    for(uint i = 0; i < SIZE_A(BaseColor); ++i)
      if(*BaseColor[i] == hWndChild) {
        SetBkColor(hdc, BaseColor[i]->getColor());
        return BaseColor[i]->getBrush();
        }
    if(*CurrColor == hWndChild) {
      SetBkColor(hdc, CurrColor->getColor());
      return CurrColor->getBrush();
      }
    }
  return 0;
}
//------------------------------------------------------------------------------
