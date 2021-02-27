//--------------- svmPropertyDiam.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyDiam.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
//-----------------------------------------------------------
diamColors::diamColors()
{
  Color[cCROWN]  = RGB(0xdf, 0xdf, 0xff);
  Color[cPIVOT]  = RGB(0x3f, 0x50, 0x9f);
  Color[cAXIS]   = RGB(0xff, 0x00, 0x00);
  Color[cBKG]    = RGB(0x9f, 0x9f, 0x9f);
}
//-----------------------------------------------------------
void diamColors::clone(const diamColors& other)
{
  for(uint i = 0; i < SIZE_A(Color); ++i)
    CLONE(Color[i]);
}
//-----------------------------------------------------------
PropertyDiam::~PropertyDiam()
{
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PropertyDiam::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyDiam* po = dynamic_cast<const PropertyDiam*>(&other);
  if(po && po != this) {

    const setOfDiamColors& oSet = po->getSet();
    int nElem = oSet.getElem();
    DiamColor.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      DiamColor[i] = oSet[i];

    for(uint i = 0; i < SIZE_A(DataPrf); ++i)
      DataPrf[i] = po->DataPrf[i];

    useColor = po->usingColor();
    }
}
//-----------------------------------------------------------
void PropertyDiam::cloneMinusProperty(const Property& other)
{
#if 1
  clone(other);
#else
  baseClass::clone(other);
  const PropertyDiam* po = dynamic_cast<const PropertyDiam*>(&other);
  if(po && po != this) {
    }
#endif
}
//-----------------------------------------------------------
bool svmDialogDiam::create()
{
  if(!baseClass::create())
    return false;

//  if(!(Prop->style & Property::FILL) && !(Prop->style & Property::TRANSP))
//    SET_CHECK(IDC_RADIOBUTTON_NO_BKG_PANEL);
  if(!(Prop->style & Property::FILL))
    SET_CHECK(IDC_RADIOBUTTON_NO_BKG_PANEL);
  else
    SET_CHECK(IDC_RADIOBUTTON_FILL_PANEL);

  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);
  if(pt) {
    fillPrf2();
    fillTypeVal2();
    setConstValue(GetDlgItem(*this, IDC_EDIT_ADDR2), pt->DataPrf[0].addr, pt->DataPrf[0].perif, pt->DataPrf[0].typeVal);
    SET_INT(IDC_EDIT_NORMALIZ2, pt->DataPrf[0].normaliz);

    setConstValue(GetDlgItem(*this, IDC_EDIT_ADDR3), pt->DataPrf[1].addr, pt->DataPrf[1].perif, pt->DataPrf[1].typeVal);
    SET_INT(IDC_EDIT_NORMALIZ3, pt->DataPrf[1].normaliz);

    if(pt->usingColor())
      SET_CHECK(IDC_CHECKBOX_NEGATIVE);
    }
  if(Prop->type1 & PropertyDiam::AXIS)
    SET_CHECK(IDC_CHECKBOX_DRAW_AXES);

  if(Prop->type1 & PropertyDiam::TRANSP_PIVOT)
    SET_CHECK(IDC_CHECKBOX_TRANSP_PIVOT);

  if(Prop->type1 & PropertyDiam::DRAW_MAX)
    SET_CHECK(IDC_CHECKBOX_DRAW_MAX_DIAM);

  if(Prop->type1 & PropertyDiam::INVERT)
    SET_CHECK(IDC_CHECKBOX_REVERSE_DRAW);

  checkTransp();
  return true;
}
//-----------------------------------------------------------
void svmDialogDiam::fillPrf2()
{
  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);

  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS2);
  int prph = pt->DataPrf[0].perif;
  if(PRPH_4_CONST == prph)
    prph = PRPH_4_CONST_CB_SEL;
  fillCBPerifEx(hwnd, prph, true);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS3);
  prph = pt->DataPrf[1].perif;
  if(PRPH_4_CONST == prph)
    prph = PRPH_4_CONST_CB_SEL;
  fillCBPerifEx(hwnd, prph, true);

}
//-----------------------------------------------------------
void svmDialogDiam::fillTypeVal2()
{
  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(Prop);
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL2);
  fillCBTypeVal(hwnd, pt->DataPrf[0].typeVal);
  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL3);
  fillCBTypeVal(hwnd, pt->DataPrf[1].typeVal);
}
//-----------------------------------------------------------
LRESULT svmDialogDiam::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_TEXT_AND_COLOR:
          chooseColors();
          break;
//        case IDC_RADIOBUTTON_NO_BKG_PANEL:
//        case IDC_RADIOBUTTON_FILL_PANEL:
//        case IDC_RADIOBUTTON_TRANSP_PANEL:
        case IDC_CHECKBOX_TRANSP_PIVOT:
          checkTransp();
          break;

        case IDC_BUTTON_NORMALIZ2:
          chooseNormaliz(IDC_EDIT_NORMALIZ2);
          break;
        case IDC_BUTTON_NORMALIZ3:
          chooseNormaliz(IDC_EDIT_NORMALIZ3);
          break;
        case IDC_COMBOBOX_PERIFS2:
        case IDC_COMBOBOX_PERIFS3:
          switch(HIWORD(wParam)) {
            case CBN_SELCHANGE:
              check_const_prph(LOWORD(wParam));
              break;
            }
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogDiam::check_const_prph(uint ids)
{
  uint idcs[][4] = {
    { IDC_COMBOBOX_PERIFS2, /*IDC_COMBOBOX_TYPEVAL2, */IDC_EDIT_NORMALIZ2, IDC_BUTTON_NORMALIZ2 },
    { IDC_COMBOBOX_PERIFS3, /*IDC_COMBOBOX_TYPEVAL3, */IDC_EDIT_NORMALIZ3, IDC_BUTTON_NORMALIZ3 },
    };
  uint idlabel[] = { IDC_STATIC_D_MAX, IDC_STATIC_D_MIN };
  HWND hwnd = GetDlgItem(*this, ids);
  int sel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  bool enableAll = PRPH_4_CONST_CB_SEL != sel;
  for(uint i = 0; i < SIZE_A(idcs); ++i) {
    if(idcs[i][0] == ids) {
      for(uint j = 1; j < SIZE_A(idcs[0]); ++j)
        ENABLE(idcs[i][j], enableAll);
      if(enableAll)
        SET_TEXT(idlabel[i], _T("Addr"));
      else
        SET_TEXT(idlabel[i], _T("Value"));
      break;
      }
    }
}
//-----------------------------------------------------------
void svmDialogDiam::checkTransp()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_TRANSP_PIVOT);
  ENABLE(IDC_CHECKBOX_REVERSE_DRAW, enable);
/*
  bool enable = IS_CHECKED(IDC_RADIOBUTTON_NO_BKG_PANEL);
  if(!enable) {
    ENABLE(IDC_CHECKBOX_TRANSP_PIVOT, false);
    ENABLE(IDC_CHECKBOX_REVERSE_DRAW, false);
    }
  else {
    enable = IS_CHECKED(IDC_CHECKBOX_TRANSP_PIVOT);
    ENABLE(IDC_CHECKBOX_TRANSP_PIVOT, true);
    ENABLE(IDC_CHECKBOX_REVERSE_DRAW, enable);
    }
*/
}
//-----------------------------------------------------------
void svmDialogDiam::CmOk()
{
  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(tmpProp);
  if(pt) {
    pt->setUseColor(IS_CHECKED(IDC_CHECKBOX_NEGATIVE));

    HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS2);
    pt->DataPrf[0].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

    hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL2);
    if(PRPH_4_CONST_CB_SEL == pt->DataPrf[0].perif) {
      pt->DataPrf[0].perif = PRPH_4_CONST;
      TCHAR t[128];
      GET_TEXT(IDC_EDIT_ADDR2, t);
      zeroTrim(t);
      DWORD v;
      pt->DataPrf[0].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0); //4;//prfData::tDWData;
      bool isReal = getConstValue(t, v, pt->DataPrf[0].typeVal);
      pt->DataPrf[0].addr = v;
      pt->DataPrf[0].normaliz = 0;
      }
    else {
      pt->DataPrf[0].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
      GET_INT(IDC_EDIT_ADDR2, pt->DataPrf[0].addr);
      GET_INT(IDC_EDIT_NORMALIZ2, pt->DataPrf[0].normaliz);
      }

    hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS3);
    pt->DataPrf[1].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

    hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL3);
    if(PRPH_4_CONST_CB_SEL == pt->DataPrf[1].perif) {
      pt->DataPrf[1].perif = PRPH_4_CONST;
      TCHAR t[128];
      GET_TEXT(IDC_EDIT_ADDR3, t);
      zeroTrim(t);
      DWORD v;
      pt->DataPrf[1].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0); //4;//prfData::tDWData;
      bool isReal = getConstValue(t, v, pt->DataPrf[1].typeVal);
      pt->DataPrf[1].addr = v;
      pt->DataPrf[1].normaliz = 0;
      }
    else {
      pt->DataPrf[1].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
      GET_INT(IDC_EDIT_ADDR3, pt->DataPrf[1].addr);
      GET_INT(IDC_EDIT_NORMALIZ3, pt->DataPrf[1].normaliz);
      }

    pt->type1 = 0;
    if(IS_CHECKED(IDC_CHECKBOX_DRAW_AXES))
      pt->type1 |= PropertyDiam::AXIS;
    if(IS_CHECKED(IDC_CHECKBOX_TRANSP_PIVOT))
      pt->type1 |= PropertyDiam::TRANSP_PIVOT;
    if(IS_CHECKED(IDC_CHECKBOX_DRAW_MAX_DIAM))
      pt->type1 |= PropertyDiam::DRAW_MAX;
    if(IS_CHECKED(IDC_CHECKBOX_REVERSE_DRAW))
      pt->type1 |= PropertyDiam::INVERT;
    }
  baseClass::CmOk();
}
//-----------------------------------------------------------
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
    _stscanf_s(buff, _T("%d,%d,%d,%d"), &cl[0], &cl[1], &cl[2], &cl[3]);

    PRect r2(r);
    r2.Inflate(-1, -2);
    r2.Offset(1, 0);

    HBRUSH hbrush = CreateSolidBrush(cl[ix - 1]);
    FillRect(hdc, r2, hbrush);
    DeleteObject(hbrush);
    }
  PListBox::paintCustom(hdc, r, ix, drawInfo);
}
//-----------------------------------------------------------
#define DIM_VALUE_COLORS 50
#define DIM_SHOW_COLORS 2
//#define DIM_SHOW_COLORS 10
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmDialogDiam::chooseColors()
{
  PropertyDiam* pt = dynamic_cast<PropertyDiam*>(tmpProp);
  if(pt) {
    pt->setUseColor(IS_CHECKED(IDC_CHECKBOX_NEGATIVE));
    svmChooseColor(pt, this).modal();
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmChooseColor::svmChooseColor(PropertyDiam* prop, PWin* parent, uint id, HINSTANCE hInst) :
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
  const setOfDiamColors& set = Prop->getSet();
  if(set.getElem())
    currColors = set[0];
  for(uint i = 0; i < SIZE_A(hb); ++i)
    hb[i] = CreateSolidBrush(currColors.Color[i]);
}
//-----------------------------------------------------------
svmChooseColor::~svmChooseColor()
{
  for(uint i = 0; i < SIZE_A(hb); ++i)
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
    DISABLE(IDC_LISTBOX_VAR_TEXT);
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
diamColors svmChooseColor::unformat(int pos)
{
  TCHAR buff[500];
  SendMessage(*LB, LB_GETTEXT, pos, (LPARAM)buff);
  diamColors cl;
  _stscanf_s(buff, _T("%d,%d,%d,%d"), &cl.Color[0], &cl.Color[1], &cl.Color[2], &cl.Color[3]);

  return cl;
}
//-----------------------------------------------------------
void svmChooseColor::CmOk()
{
  setOfDiamColors& set = Prop->getSet();
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
  for(uint i = 0; i < SIZE_A(ids); ++i) {
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
  const setOfDiamColors& set = Prop->getSet();
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
  diamColors cl;
  _stscanf_s(buff, _T("%d,%d,%d,%d"), &cl.Color[0], &cl.Color[1], &cl.Color[2], &cl.Color[3]);
  for(int i = 0; i < diamColors::MAX_COLOR_TYPE; ++i) {
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
void svmChooseColor::add(const diamColors& info)
{
  TCHAR buff[500];
  fillStr(buff, _T(' '), SIZE_A(buff));
  LPTSTR p = buff;
  TCHAR t[150];
  wsprintf(t, _T("%d,%d,%d,%d"),
      info.Color[diamColors::cCROWN],
      info.Color[diamColors::cPIVOT],
      info.Color[diamColors::cAXIS],
      info.Color[diamColors::cBKG]
      );
  copyStr(p, t, _tcslen(t));
  p += DIM_VALUE_COLORS;
  *p++ = TAB;
  for(int i = 0; i < diamColors::MAX_COLOR_TYPE; ++i) {
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
    InvalidateRect(GetDlgItem(*this, ids[ix]), 0, 0);
}
