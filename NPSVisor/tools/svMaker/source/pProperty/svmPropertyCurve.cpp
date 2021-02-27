//--------------- svmPropertyCurve.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyCurve.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
//-----------------------------------------------------------
curveColors::curveColors()
{
  Color[cBALL]  = RGB(0x1f, 0x1f, 0x00);
  Color[cGRID]  = RGB(0xaf, 0xaf, 0xaf);
  Color[cAXE]   = RGB(0xff, 0x00, 0x00);
}
//-----------------------------------------------------------
void curveColors::clone(const curveColors& other)
{
  for(uint i = 0; i < SIZE_A(Color); ++i)
    CLONE(Color[i]);
}
//-----------------------------------------------------------
PropertyCurve::~PropertyCurve()
{
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PropertyCurve::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyCurve* po = dynamic_cast<const PropertyCurve*>(&other);
  if(po && po != this) {
    CurveColor = po->getSet();
    nRow = po->nRow;
    maxX = po->maxX;
    _tcscpy_s(fileShow, po->fileShow);
    useFileShow = po->useFileShow;
    hideBall = po->hideBall;
    }
}
//-----------------------------------------------------------
void PropertyCurve::cloneMinusProperty(const Property& other)
{
#if 1
  clone(other);
#else
  baseClass::clone(other);
  const PropertyCurve* po = dynamic_cast<const PropertyCurve*>(&other);
  if(po && po != this) {
    }
#endif
}
//-----------------------------------------------------------
svmDialogCurve::svmDialogCurve(svmObject* owner, Property* prop, PWin* parent, uint id,
                            HINSTANCE hInst) :
      baseClass(owner, prop, parent, id, hInst)
{
  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(Prop);

  const curveColors& set = pt->getSet();
  for(uint i = 0; i < SIZE_A(otherBrush); ++i)
    otherBrush[i] = CreateSolidBrush(set.Color[i]);
}
//-----------------------------------------------------------
svmDialogCurve::~svmDialogCurve()
{
  destroy();
  for(uint i = 0; i < SIZE_A(otherBrush); ++i)
    DeleteObject(otherBrush[i]);
}
//-----------------------------------------------------------
bool svmDialogCurve::create()
{
  if(!baseClass::create())
    return false;

  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(Prop);
  if(pt) {

    SET_INT(IDC_EDIT_PERC1ST, pt->type1);
    SET_INT(IDC_EDIT_POINT1ST, pt->type2);

    SET_INT(IDC_EDIT_N_ROW, pt->nRow);

    SET_INT(IDC_EDIT_MAX_X_SHOW, pt->maxX);
    SET_TEXT(IDC_EDIT_FILENAME_SHOW, pt->fileShow);
    if(pt->useFileShow)
      SET_CHECK(IDC_CHECKBOX_USE_FILE_SHOW);
    if(pt->hideBall)
      SET_CHECK(IDC_CHECKBOX_HIDE_BALL);
    }
  checkEnable();
  checkEnableBall();
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogCurve::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_COLOR_BULLET:
          chooseCustomColor(0);
          break;
        case IDC_BUTTON_COLOR_GRID:
          chooseCustomColor(1);
          break;
        case IDC_BUTTON_COLOR_AXE:
          chooseCustomColor(2);
          break;
        case IDC_BUTTON_LINE_SHOW:
          chooseCustomColor(3);
          break;
        case IDC_CHECKBOX_NEGATIVE:
          checkEnableBall();
          break;
        case IDC_CHECKBOX_USE_FILE_SHOW:
          checkEnable();
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogCurve::checkEnableBall()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_NEGATIVE);
  ENABLE(IDC_CHECKBOX_HIDE_BALL, enable);
}
//-----------------------------------------------------------
void svmDialogCurve::checkEnable()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_USE_FILE_SHOW);
  ENABLE(IDC_EDIT_MAX_X_SHOW, enable);
  ENABLE(IDC_EDIT_FILENAME_SHOW, enable);
  ENABLE(IDC_BUTTON_LINE_SHOW, enable);
}
//-----------------------------------------------------------
HBRUSH svmDialogCurve::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  int type = 0;
  int ids[] = {
    IDC_STATICTEXT_BULLET, IDC_BUTTON_COLOR_BULLET,
    IDC_STATICTEXT_GRID, IDC_BUTTON_COLOR_GRID,
    IDC_STATICTEXT_AXE, IDC_BUTTON_COLOR_AXE,
    IDC_STATICTEXT_COLOR_LINE_SHOW, IDC_BUTTON_LINE_SHOW
    };
  for(uint i = 0; i < SIZE_A(ids); ++i)
    if(GetDlgItem(*this, ids[i]) == hWndChild) {
      type = i / 2 + 1;
      break;
      }

  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(tmpProp);
  if(pt && type) {

    const curveColors& set = pt->getSet();
    int i = type - 1;
    SetBkColor(dc, set.Color[i]);
    return otherBrush[i];
    }

  return baseClass::evCtlColor(dc, hWndChild, ctlType);
}
//-----------------------------------------------------------
void svmDialogCurve::CmOk()
{
  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(tmpProp);
  if(pt) {

    pt->style |= Property::FILL;
    pt->style &= ~Property::TRANSP;
    GET_INT(IDC_EDIT_PERC1ST, pt->type1);
    GET_INT(IDC_EDIT_POINT1ST, pt->type2);
    GET_INT(IDC_EDIT_N_ROW, pt->nRow);

    GET_INT(IDC_EDIT_MAX_X_SHOW, pt->maxX);
    GET_TEXT(IDC_EDIT_FILENAME_SHOW, pt->fileShow);
    pt->useFileShow = IS_CHECKED(IDC_CHECKBOX_USE_FILE_SHOW);
    pt->hideBall = IS_CHECKED(IDC_CHECKBOX_HIDE_BALL);
    }
  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmDialogCurve::chooseCustomColor(int ix)
{
  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(tmpProp);
  if(pt) {
    curveColors& set = pt->getSet();
    if(choose_Color(*this, set.Color[ix]))
      invalidateColor(ix);
    }
}
//-----------------------------------------------------------
void svmDialogCurve::invalidateColor(DWORD ix)
{
  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(tmpProp);
  if(pt) {
    const curveColors& set = pt->getSet();
    if(otherBrush[ix])
    DeleteObject(otherBrush[ix]);
    otherBrush[ix] = CreateSolidBrush(set.Color[ix]);

    const int ids[] = {
      IDC_STATICTEXT_BULLET,
      IDC_STATICTEXT_GRID,
      IDC_STATICTEXT_AXE,
      };
    if(getHandle())
      InvalidateRect(GetDlgItem(*this, ids[ix]), 0, 0);
    }
}
//-----------------------------------------------------------
/*
//-----------------------------------------------------------
class svmListBoxColor : public PListBox
{
  private:
    typedef PListBox baseClass;
  public:
    svmListBoxColor(PWin* parent, uint id = IDC_LISTBOX_FONT_ALLOCATED, int len = 255, HINSTANCE hinst = 0):
       PListBox(parent, id, len, hinst) { }

  protected:
    virtual void paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
//    SendMessage(getHandle(), LB_GETTEXT, drawInfo.itemID, (LPARAM)Buff);


//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual void paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo);
};
//------------------------------------------------------------------
void svmListBoxColor::paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(1 <= ix) {
    uint item = drawInfo.itemID;
    TCHAR buff[200];
    SendMessage(getHandle(), LB_GETTEXT, item, (LPARAM)buff);
    COLORREF cl[4];
    _stscanf(buff, _T("%d,%d,%d,%d"), &cl[0], &cl[1], &cl[2], &cl[3]);

    PRect r2(r);
    r2.Inflate(-1, -2);
    r2.Offset(1, 0);

    HBRUSH hbrush = CreateSolidBrush(cl[ix - 1]);
    FillRect(hdc, r2, hbrush);
    DeleteObject(hbrush);
    }
  PListBox::paintCustom(hdc, r, ix, drawInfo);
}
*/
//-----------------------------------------------------------
#define DIM_VALUE_COLORS 50
#define DIM_SHOW_COLORS 2
//#define DIM_SHOW_COLORS 10
//-----------------------------------------------------------
//-----------------------------------------------------------
/*
void svmDialogCurve::chooseColors()
{
  PropertyCurve* pt = dynamic_cast<PropertyCurve*>(tmpProp);
  if(pt) {
    pt->setUseColor(IS_CHECKED(IDC_CHECKBOX_NEGATIVE));
    svmChooseColor(pt, this).modal();
    }
}
*/
//-----------------------------------------------------------
/*
//-----------------------------------------------------------
svmChooseColor::svmChooseColor(PropertyCurve* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), Prop(prop), enlarged(false)
{
  LB = new svmListBoxColor(this);
  int tabs[] = {
    DIM_VALUE_COLORS,
    DIM_SHOW_COLORS,
    DIM_SHOW_COLORS,
    DIM_SHOW_COLORS,
    DIM_SHOW_COLORS };
  int show[] = { 0, 1, 1, 1, 1 };
  LB->SetTabStop(SIZE_A(tabs), tabs, show);
  const setOfCurveColors& set = Prop->getSet();
  if(set.getElem())
    currColors = set[0];
  for(int i = 0; i < SIZE_A(hb); ++i)
    hb[i] = CreateSolidBrush(currColors.Color[i]);
}
//-----------------------------------------------------------
svmChooseColor::~svmChooseColor()
{
  for(int i = 0; i < SIZE_A(hb); ++i)
    if(hb[i])
      DeleteObject(hb[i]);
  destroy();
}
//-----------------------------------------------------------
#define DISABLE(idc) ENABLE(idc, false)
//-----------------------------------------------------------
bool svmChooseColor::create()
{
  if(!baseClass::create())
    return false;
  SET_INT(IDC_EDIT_ADDR, Prop->DataPrf[2].addr);
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS);
  fillCBPerif(hwnd, Prop->DataPrf[2].perif);
  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  fillCBTypeVal(hwnd, Prop->DataPrf[2].typeVal);

  fill();
  if(!Prop->usingColor()) {
    DISABLE(IDC_COMBOBOX_PERIFS);
    DISABLE(IDC_EDIT_ADDR);
    DISABLE(IDC_COMBOBOX_TYPEVAL);
    DISABLE(IDC_LISTBOX1);
    DISABLE(IDC_BUTTON_UP_COLOR);
    DISABLE(IDC_BUTTON_DN_COLOR);
    DISABLE(IDC_BUTTON_ADD_COLOR);
    DISABLE(IDC_BUTTON_REM_COLOR);
    }
  PRect r;
  GetWindowRect(GetDlgItem(*this, IDC_STATIC_CROWN), r);
  double width = r.Width();
  width /= DIM_SHOW_COLORS;
  LB->setPixelWidthChar(width);
  LB->recalcWidth();
  LB->setIntegralHeight();
  return true;
}
//-----------------------------------------------------------
LRESULT svmChooseColor::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_FG:
          chooseColor(0);
          break;
        case IDC_BUTTON_CHOOSE_PIVOT:
          chooseColor(1);
          break;
        case IDC_BUTTON_CHOOSE_AXIS:
          chooseColor(2);
          break;
        case IDC_BUTTON_CHOOSE_BG:
          chooseColor(3);
          break;

        case IDC_BUTTON_ADD_COLOR:
          add();
          break;
        case IDC_BUTTON_REM_COLOR:
          rem();
          break;
        case IDC_BUTTON_UP_COLOR:
        case IDC_BUTTON_DN_COLOR:
          move(IDC_BUTTON_UP_COLOR == LOWORD(wParam));
          break;
        }
      break;

//    case WM_CTLCOLORBTN:
//    case WM_CTLCOLORDLG:
//    case WM_CTLCOLOREDIT:
//    case WM_CTLCOLORLISTBOX:
//    case WM_CTLCOLORMSGBOX:
//    case WM_CTLCOLORSCROLLBAR:
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
//-----------------------------------------------------------
curveColors svmChooseColor::unformat(int pos)
{
  TCHAR buff[500];
  SendMessage(*LB, LB_GETTEXT, pos, (LPARAM)buff);
  curveColors cl;
  _stscanf(buff, _T("%d,%d,%d,%d"), &cl.Color[0], &cl.Color[1], &cl.Color[2], &cl.Color[3]);

  return cl;
}
//-----------------------------------------------------------
void svmChooseColor::CmOk()
{
  setOfCurveColors& set = Prop->getSet();
  if(Prop->usingColor()) {
    set.reset();
    int count = SendMessage(*LB, LB_GETCOUNT, 0 ,0);
    if(count > 0) {
      set.setDim(count);
      for(int i = 0; i < count; ++i)
        set[i] = unformat(i);
      }
    }
  else
    set[0] = currColors;

  GET_INT(IDC_EDIT_ADDR, Prop->DataPrf[2].addr);
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS);
  Prop->DataPrf[2].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  Prop->DataPrf[2].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  baseClass::CmOk();
}
//-----------------------------------------------------------
HBRUSH svmChooseColor::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  const int ids[] = {
      IDC_STATIC_CROWN,
      IDC_STATIC_PIVOT,
      IDC_STATIC_AXES,
      IDC_STATIC_BKG
      };
  for(int i = 0; i < SIZE_A(ids); ++i) {
    if(GetDlgItem(*this, ids[i]) == hWndChild)
      return hb[i];
    }
  return 0;
}
//-----------------------------------------------------------
void svmChooseColor::chooseColor(int ix)
{
  if(choose_Color(*this, currColors.Color[ix])) {
    invalidateColor(ix);
    }
}
//-----------------------------------------------------------
void svmChooseColor::fill()
{
  SendMessage(*LB, LB_RESETCONTENT, 0, 0);
  const setOfCurveColors& set = Prop->getSet();
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i)
    add(set[i]);
  SendMessage(*LB, LB_SETCURSEL, 0, 0);
}
//-----------------------------------------------------------
void calc_WidthLB(bool& enlarged, bool add, PListBox* LB)
{
  if(!(add ^ enlarged))
    return;
  PRect r;
  SendMessage(*LB, LB_GETITEMRECT, 0, (LPARAM) (RECT FAR*) r);
  int count = SendMessage(*LB, LB_GETCOUNT, 0, 0);
  PRect rlb;
  GetClientRect(*LB, rlb);
  int maxShow = rlb.Height() / r.Height();
  if(add && maxShow < count) {
    GetWindowRect(*LB, rlb);
    int offs = GetSystemMetrics(SM_CXVSCROLL);
    rlb.right += offs;
    LB->setWindowPos(0, rlb, SWP_NOMOVE | SWP_NOZORDER);
    enlarged = true;
    }
  else if(!add) {
    if(maxShow >= count) {
      GetWindowRect(*LB, rlb);
      int offs = GetSystemMetrics(SM_CXVSCROLL);
      rlb.right -= offs;
      LB->setWindowPos(0, rlb, SWP_NOMOVE | SWP_NOZORDER);
      enlarged = false;
      }
    }
}
//-----------------------------------------------------------
void svmChooseColor::calcWidthLB(bool add)
{
#if 1
  calc_WidthLB(enlarged, add, LB);
#else
  if(!(add ^ enlarged))
    return;
  PRect r;
  SendMessage(*LB, LB_GETITEMRECT, 0, (LPARAM) (RECT FAR*) r);
  int count = SendMessage(*LB, LB_GETCOUNT, 0, 0);
  PRect rlb;
  GetClientRect(*LB, rlb);
  int maxShow = rlb.Height() / r.Height();
  if(add && maxShow < count) {
    GetWindowRect(*LB, rlb);
    int offs = GetSystemMetrics(SM_CXVSCROLL);
    rlb.right += offs;
    LB->setWindowPos(0, rlb, SWP_NOMOVE | SWP_NOZORDER);
    enlarged = true;
    }
  else if(!add) {
    if(maxShow >= count) {
      GetWindowRect(*LB, rlb);
      int offs = GetSystemMetrics(SM_CXVSCROLL);
      rlb.right -= offs;
      LB->setWindowPos(0, rlb, SWP_NOMOVE | SWP_NOZORDER);
      enlarged = false;
      }
    }
#endif
}
//-----------------------------------------------------------
void svmChooseColor::add()
{
  add(currColors);
}
//-----------------------------------------------------------
void svmChooseColor::rem()
{
  int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[500];
  SendMessage(*LB, LB_GETTEXT, sel, (LPARAM)buff);
  SendMessage(*LB, LB_DELETESTRING, sel, 0);
  curveColors cl;
  _stscanf(buff, _T("%d,%d,%d,%d"), &cl.Color[0], &cl.Color[1], &cl.Color[2], &cl.Color[3]);
  for(int i = 0; i < curveColors::MAX_COLOR_TYPE; ++i) {
    if(currColors.Color[i] != cl.Color[i]) {
      currColors.Color[i] = cl.Color[i];
      invalidateColor(i);
      }
    }
  calcWidthLB(false);
  int count = SendMessage(*LB, LB_GETCOUNT, 0, 0);
  if(sel >= count)
    sel = count - 1;
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
  SendMessage(*LB, LB_SETCARETINDEX, sel, 0);
  if(enlarged)
    SendMessage(*LB, WM_VSCROLL, 0, 0);
//    InvalidateRect(*LB, 0, 1);
}
//-----------------------------------------------------------
void svmChooseColor::move(bool up)
{
  int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
  if(!sel && up)
    return;
  int count = SendMessage(*LB, LB_GETCOUNT, 0, 0);
  if(sel == count - 1 && !up)
    return;

  TCHAR buff[500];
  SendMessage(*LB, LB_GETTEXT, sel, (LPARAM)buff);
  SendMessage(*LB, LB_DELETESTRING, sel, 0);
  sel += up ? -1 : 1;
  SendMessage(*LB, LB_INSERTSTRING, sel, (LPARAM)buff);
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
void svmChooseColor::add(const curveColors& info)
{
  TCHAR buff[500];
  fillStr(buff, _T(' '), SIZE_A(buff));
  LPTSTR p = buff;
  TCHAR t[150];
  wsprintf(t, _T("%d,%d,%d,%d"),
      info.Color[curveColors::cCROWN],
      info.Color[curveColors::cPIVOT],
      info.Color[curveColors::cAXIS],
      info.Color[curveColors::cBKG]
      );
  copyStr(p, t, _tcslen(t));
  p += DIM_VALUE_COLORS;
  *p++ = TAB;
  for(int i = 0; i < curveColors::MAX_COLOR_TYPE; ++i) {
    p += DIM_SHOW_COLORS;
    *p++ = TAB;
    }
  *p = 0;
  int sel = SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
  calcWidthLB(true);
}
//-----------------------------------------------------------
void svmChooseColor::invalidateColor(DWORD ix)
{
  if(hb[ix])
    DeleteObject(hb[ix]);
  hb[ix] = CreateSolidBrush(currColors.Color[ix]);

  const int ids[] = {
      IDC_STATIC_CROWN,
      IDC_STATIC_PIVOT,
      IDC_STATIC_AXES,
      IDC_STATIC_BKG
      };
  if(getHandle())
    InvalidateRect(GetDlgItem(*this, ids[ix]), 0, 1);
}
*/
