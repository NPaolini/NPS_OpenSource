//------------ advBtnDlgImpl.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "macro_utils.h"
#include "advBtnDlgImpl.h"
#include "common.h"
#include "language_util.h"
//----------------------------------------------------------------------------
#define DIM_VAL 10
#define DIM_TEXT 2000
#define DIM_ALL_TEXT (DIM_VAL + DIM_TEXT + 2)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
baseAdvBtnDlg* getAdvBtn(PWin* parent, PropertyBtn& prop, eBtnPageType type)
{
  switch(type) {
    case eptText:
      return new advBtnDlgText(parent, prop, IDD_BUTTON_ADVANCED_TXT);
    case eptColor:
      return new advBtnDlgColor(parent, prop, IDD_BUTTON_ADVANCED_COLOR);
    case eptBmp:
      return new advBtnDlgBmp(parent, prop, IDD_BUTTON_ADVANCED_BMP);
    }
  return 0;
}
//----------------------------------------------------------------------------
bool advBtnDlgText::create()
{
  PListBox* lb = new PListBox(this, IDC_LISTBOX_VAR_TEXT);
  int tabs[] = { -DIM_VAL, DIM_TEXT };
  lb->SetTabStop(SIZE_A(tabs), tabs);
  new langEdit(this, IDC_EDIT_VAR_TEXT);
  if(!baseClass::create())
    return false;
  lb->setIntegralHeight();
  uint nElem = Prop.allText.getElem();
  if(Prop.Flags.textByVar > 1) {
    for(uint i = 0; i < nElem; ++i) {
      TCHAR t[DIM_ALL_TEXT];
      LPCTSTR p = Prop.allText[i];
      if(!p)
        p = _T("---");
      _stprintf_s(t, _T("%0.3f\t%s"), Prop.textVal[i], p);
      SendMessage(*lb, LB_ADDSTRING, 0, (LPARAM)t);
      }
    }
  else {
    for(uint i = 0; i < nElem; ++i) {
      TCHAR t[DIM_ALL_TEXT];
      LPCTSTR p = Prop.allText[i];
      if(!p)
        p = _T("---");
      _stprintf_s(t, _T("  \t%s"), p);
      SendMessage(*lb, LB_ADDSTRING, 0, (LPARAM)t);
      }
    }
  return true;
}
//----------------------------------------------------------------------------
static double valByRow(LPTSTR t, int ntab = 1)
{
  int ix = 0;
  for(; t[ix]; ++ix) {
    if(_T('\t') == t[ix]) {
      t[ix] = 0;
      if(--ntab <= 0) {
        ++ix;
        break;
        }
      }
    }
  double v = _tstof(t);
  int i = 0;
  for(; t[ix]; ++ix, ++i)
    t[i] = t[ix];
  t[i] = 0;
  return v;
}
//----------------------------------------------------------------------------
void advBtnDlgText::saveData()
{
  flushPAV(Prop.allText);
  Prop.allText[0] = 0;
  Prop.allText[1] = 0;
  Prop.textVal.reset();
  Prop.Flags.textByVar = retrieveCheck();
  retrievePrph(Prop.DataPrf[1]);

  HWND hwlb = GetDlgItem(*this, IDC_LISTBOX_VAR_TEXT);
  int count = SendMessage(hwlb, LB_GETCOUNT, 0, 0);
  if(count <= 0)
    return;
  TCHAR t[DIM_ALL_TEXT];
  for(int i = 0; i < count; ++i) {
    SendMessage(hwlb, LB_GETTEXT, i, LPARAM(t));
    double v = valByRow(t);
    if(Prop.Flags.textByVar > 1)
      Prop.textVal[i] = v;
    Prop.allText[i] = str_newdup(t);
    }
}
//----------------------------------------------------------------------------
LRESULT advBtnDlgText::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
/*
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        }

      switch(HIWORD(wParam)) {
        }
      break;
    }
*/
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
int advBtnDlgText::makeRow(LPTSTR target, uint sz)
{
  TCHAR t[DIM_ALL_TEXT];
  GET_TEXT(IDC_EDIT_VAR_TEXT, t);
  if(needVal()) {
    TCHAR v[64];
    GET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, v);
    uint len = _tcslen(t) + _tcslen(v) + 2;
    if(!target || sz < len)
      return len;
    wsprintf(target, _T("%s\t%s"), v, t);
    }
  else {
    uint len = _tcslen(t) + 4;
    if(!target || sz < len)
      return len;
    wsprintf(target, _T(" \t%s"), t);
    }
  return _tcslen(target);
}
//----------------------------------------------------------------------------
int advBtnDlgText::unmakedRow(LPTSTR target)
{
  double v = valByRow(target);
  TCHAR t[64];
  _stprintf_s(t, _T("%0.3f"), v);
  SET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, t);
  SET_TEXT(IDC_EDIT_VAR_TEXT, target);
  return 0;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool advBtnDlgBmp::create()
{
  PListBox* lb = new PListBox(this, IDC_LISTBOX_VAR_TEXT);
  int tabs[] = { -DIM_VAL, DIM_TEXT };
  lb->SetTabStop(SIZE_A(tabs), tabs);

  if(!baseClass::create())
    return false;
  lb->setIntegralHeight();

  const PVect<LPCTSTR>&bmp = Prop.getNames();
  uint nElem = bmp.getElem();
  if(Prop.Flags.bitmapByVar > 1) {
    for(uint i = 0; i < nElem; ++i) {
      TCHAR t[DIM_ALL_TEXT];
      LPCTSTR p = bmp[i];
      if(!p)
        p = _T("---");
      _stprintf_s(t, _T("%0.3f\t%s"), Prop.bmpVal[i], p);
      SendMessage(*lb, LB_ADDSTRING, 0, (LPARAM)t);
      }
    }
  else {
    for(uint i = 0; i < nElem; ++i) {
      TCHAR t[DIM_ALL_TEXT];
      LPCTSTR p = bmp[i];
      if(!p)
        p = _T("---");
      _stprintf_s(t, _T("  \t%s"), p);
      SendMessage(*lb, LB_ADDSTRING, 0, (LPARAM)t);
      }
    }
  switch(Prop.Flags.styleBmpDim) {
    case POwnBtnImageStd::eisScaled:
      SET_CHECK(IDC_CHECK_SCALED_BMP);
      break;
    case POwnBtnImageStd::eisStretched:
      SET_CHECK(IDC_CHECK_STRETCHED_BMP);
      break;
/*
    case POwnBtnImageStd::eisFixed:
      if(!Prop.Flags.fixedBmpDim)
        SET_CHECK(IDC_CHECK_SCALED_BMP);
      break;
*/
    }
  return true;
}
//----------------------------------------------------------------------------
void advBtnDlgBmp::saveData()
{
  PVect<LPCTSTR>&bmp = Prop.getNames();
  flushPAV(bmp);
  Prop.bmpVal.reset();
  Prop.Flags.bitmapByVar = retrieveCheck();
  retrievePrph(Prop.DataPrf[2]);

  Prop.Flags.fixedBmpDim = 1;
  Prop.Flags.styleBmpDim = IS_CHECKED(IDC_CHECK_SCALED_BMP) ? POwnBtnImageStd::eisScaled :
        IS_CHECKED(IDC_CHECK_STRETCHED_BMP) ? POwnBtnImageStd::eisStretched : 0;

  HWND hwlb = GetDlgItem(*this, IDC_LISTBOX_VAR_TEXT);
  int count = SendMessage(hwlb, LB_GETCOUNT, 0, 0);
  if(count <= 0)
    return;
  TCHAR t[DIM_ALL_TEXT];
  for(int i = 0; i < count; ++i) {
    SendMessage(hwlb, LB_GETTEXT, i, LPARAM(t));
    double v = valByRow(t);
    if(Prop.Flags.bitmapByVar > 1)
      Prop.bmpVal[i] = v;
    bmp[i] = str_newdup(t);
    }
}
//----------------------------------------------------------------------------
void advBtnDlgBmp::changedPosBmp(uint selected)
{
  bool center = POwnBtnImageStd::wCenter == selected;
  if(!center)
    SET_CHECK_SET(IDC_CHECK_STRETCHED_BMP, false);
  ENABLE(IDC_CHECK_STRETCHED_BMP, center);
}
//----------------------------------------------------------------------------
LRESULT advBtnDlgBmp::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECK_SCALED_BMP:
          if(IS_CHECKED(IDC_CHECK_SCALED_BMP))
            SET_CHECK_SET(IDC_CHECK_STRETCHED_BMP, false);
          break;
        case IDC_CHECK_STRETCHED_BMP:
          if(IS_CHECKED(IDC_CHECK_STRETCHED_BMP))
            SET_CHECK_SET(IDC_CHECK_SCALED_BMP, false);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
int advBtnDlgBmp::makeRow(LPTSTR target, uint sz)
{
  if(!target || sz < _MAX_PATH)
    return _MAX_PATH;
#if 0
  if(*nameMod) {
    if(needVal()) {
      TCHAR v[64];
      GET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, v);
      wsprintf(target, _T("%s\t%s"), v, nameMod);
      }
    else {
      wsprintf(target, _T(" \t%s"), nameMod);
      }
    nameMod[0] = 0;
    }
  else
#endif
    {
    TCHAR file[_MAX_PATH] = {0};
    if(*nameMod)
      _tcscpy_s(file, nameMod);
    LPTSTR pfile = file;
    if(openFileImageWithCopy(*this, pfile, false)) {
      LPCTSTR p = getOnlySubImagePath(file);
      if(needVal()) {
        TCHAR v[64];
        GET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, v);
        wsprintf(target, _T("%s\t%s"), v, file);
        }
      else {
        wsprintf(target, _T(" \t%s"), file);
        }
      }
    else
      return 0;
    }
  return _tcslen(target);
}
//----------------------------------------------------------------------------
int advBtnDlgBmp::unmakedRow(LPTSTR target)
{
  double v = valByRow(target);
  TCHAR t[128];
  _stprintf_s(t, _T("%0.3f"), v);
  SET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, t);
  _tcscpy_s(nameMod, target);
  return 0;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static
void formatColor(LPTSTR t, const COLORREF& fg, const COLORREF& bg)
{
  wsprintf(t, _T("%d,%d,%d-%d,%d,%d"),
            GetRValue(fg), GetGValue(fg), GetBValue(fg),
            GetRValue(bg), GetGValue(bg), GetBValue(bg)
            );

}
//----------------------------------------------------------------------------
static
void unformatColor(LPCTSTR t, COLORREF& fg_, COLORREF& bg_)
{
  if(!t || !*t)
    return;
  int fr = 0;
  int fg = 0;
  int fb = 0;
  int br = 255;
  int bg = 255;
  int bb = 255;
  _stscanf_s(t, _T("%d,%d,%d-%d,%d,%d"), &fr, &fg, &fb, &br, &bg, &bb);
  fg_ = RGB(fr, fg, fb);
  bg_ = RGB(br, bg, bb);
}
//----------------------------------------------------------------------------
static void addColor(HWND hwnd, LPCTSTR prefix, COLORREF fg, COLORREF bg)
{
  TCHAR t[DIM_ALL_TEXT];
  _tcscpy_s(t, prefix);
  LPTSTR p = t + _tcslen(t);
  formatColor(p, fg, bg);
  SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)t);
}
//----------------------------------------------------------------------------
#define DIM_TEXT_COLOR 18
//----------------------------------------------------------------------------
bool advBtnDlgColor::create()
{
  PListBox* lb = new lbColor(this, IDC_LISTBOX_VAR_TEXT);
  int tabs[] = { 1, -DIM_VAL, DIM_TEXT_COLOR };
  lb->SetTabStop(SIZE_A(tabs), tabs);
  bgSample = CreateSolidBrush(Prop.background);

  if(!baseClass::create())
    return false;
  lb->setIntegralHeight();
  uint nElem = min(Prop.otherFg.getElem(), Prop.otherBg.getElem());
  if(Prop.Flags.colorByVar > 1) {
    TCHAR t[64];
    for(uint i = 0; i < nElem; ++i) {
      _stprintf_s(t, _T(" \t%0.3f\t"), Prop.colorVal[i]);
      addColor(*lb, t, Prop.otherFg[i], Prop.otherBg[i]);
      }
    }
  else {
    for(uint i = 0; i < nElem; ++i)
      addColor(*lb, _T(" \t \t"), Prop.otherFg[i], Prop.otherBg[i]);
    }
  return true;
}
//----------------------------------------------------------------------------
void advBtnDlgColor::saveData()
{
  Prop.colorVal.reset();
  Prop.otherFg.reset();
  Prop.otherBg.reset();
  Prop.Flags.colorByVar = retrieveCheck();
  retrievePrph(Prop.DataPrf[0]);

  HWND hwlb = GetDlgItem(*this, IDC_LISTBOX_VAR_TEXT);
  int count = SendMessage(hwlb, LB_GETCOUNT, 0, 0);
  if(count <= 0)
    return;
  TCHAR t[DIM_ALL_TEXT];
  for(int i = 0; i < count; ++i) {
    SendMessage(hwlb, LB_GETTEXT, i, LPARAM(t));
    double v = valByRow(t, 2);
    if(Prop.Flags.colorByVar > 1)
      Prop.colorVal[i] = v;

    COLORREF fg = 0;
    COLORREF bg = RGB(255, 255, 255);
    unformatColor(t, fg, bg);
    Prop.otherFg[i] = fg;
    Prop.otherBg[i] = bg;
    }
  Prop.foreground = Prop.otherFg[0];
  Prop.background = Prop.otherBg[0];
  Prop.fgPress = Prop.otherFg[1];
  Prop.bgPress = Prop.otherBg[1];
}
//----------------------------------------------------------------------------
LRESULT advBtnDlgColor::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_FG_VAR_TEXT:
          setColor(0);
          break;
        case IDC_BUTTON_BG_VAR_TEXT:
          setColor(1);
          break;
        }
/*
      switch(HIWORD(wParam)) {
        }
*/
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
HBRUSH advBtnDlgColor::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  if(IDC_STATIC_TEST_COLOR == id) {
    COLORREF fg = 0;
    COLORREF bg = RGB(255, 255, 255);
    TCHAR t[64];
    GET_TEXT(IDC_STATIC_TEST_COLOR, t);
    unformatColor(t, fg, bg);
    SetTextColor(dc, fg);
    SetBkColor(dc, bg);
    return bgSample;
    }
  return 0;
}
//----------------------------------------------------------------------------
void advBtnDlgColor::setColor(uint ix)
{
  COLORREF fg = 0;
  COLORREF bg = RGB(255, 255, 255);
  TCHAR t[64];
  GET_TEXT(IDC_STATIC_TEST_COLOR, t);
  unformatColor(t, fg, bg);
  bool success;
  if(ix)
    success = choose_Color(*this, bg);
  else
    success = choose_Color(*this, fg);
  if(success) {
    formatColor(t, fg, bg);
    SET_TEXT(IDC_STATIC_TEST_COLOR, t);
    if(ix) {
      if(bgSample)
        DeleteObject(bgSample);
      bgSample = CreateSolidBrush(bg);
      }
    InvalidateRect(GetDlgItem(*this, IDC_STATIC_TEST_COLOR), 0, 1);
    }
}
//----------------------------------------------------------------------------
int advBtnDlgColor::makeRow(LPTSTR target, uint sz)
{
  TCHAR t[64];
  GET_TEXT(IDC_STATIC_TEST_COLOR, t);
  if(needVal()) {
    TCHAR v[64];
    GET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, v);
    uint len = _tcslen(t) + _tcslen(v) + 5;
    if(!target || sz < len)
      return len;
    wsprintf(target, _T(" \t%s\t%s"), v, t);
    }
  else {
    uint len = _tcslen(t) + 5;
    if(!target || sz < len)
      return len;
    wsprintf(target, _T(" \t \t%s"), t);
    }
  return _tcslen(target);
}
//----------------------------------------------------------------------------
int advBtnDlgColor::unmakedRow(LPTSTR target)
{
  double v = valByRow(target, 2);
  TCHAR t[128];
  _stprintf_s(t, _T("%0.3f"), v);
  SET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, t);
  SET_TEXT(IDC_STATIC_TEST_COLOR, target);
  COLORREF fg;
  COLORREF bkg;
  unformatColor(target, fg, bkg);
  if(bgSample)
    DeleteObject(bgSample);
  bgSample = CreateSolidBrush(bkg);
  InvalidateRect(GetDlgItem(*this, IDC_STATIC_TEST_COLOR), 0, 1);
  return 0;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
HPEN lbColor::setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo)
{
  advBtnDlgColor* par = dynamic_cast<advBtnDlgColor*>(getParent());
  if(drawInfo.itemID < 0)
    return baseClass::setColor(hDC, drawInfo);

  COLORREF bkg;
  if (drawInfo.itemState & ODS_SELECTED)
    bkg = C_BkgSel;
  else
    bkg = C_Bkg;

  PRect Rect(drawInfo.rcItem);
  Rect.right = Rect.left + pxTabs[0];

  SetBkColor(hDC, bkg);
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, Rect, 0, 0, 0);
  Rect.left = Rect.right;
  Rect.right = drawInfo.rcItem.right;

  COLORREF fg;
  LPCTSTR p = findNextParamTrim(Buff, 2, _T('\t'));
  unformatColor(p, fg, bkg);

  SetBkColor(hDC, bkg);
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, Rect, 0, 0, 0);

  SetTextColor(hDC, fg);
  HPEN pen = CreatePen(PS_SOLID, 1, fg);
  HPEN oldPen = (HPEN) SelectObject(hDC, pen);
  return oldPen;
}
//----------------------------------------------------------------------------
void lbColor::paintCustom(HDC hDC, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  HPEN pen = CreatePen(PS_SOLID, 1, C_Text);
  HPEN oldPen = (HPEN) SelectObject(hDC, pen);
  baseClass::paintCustom(hDC, r, ix, drawInfo);
  DeleteObject(SelectObject(hDC, oldPen));
}
//----------------------------------------------------------------------------
