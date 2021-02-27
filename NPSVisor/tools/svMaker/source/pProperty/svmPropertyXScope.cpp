//--------------- svmPropertyXScope.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyXScope.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "PStatic.h"
//-----------------------------------------------------------
xScopeColors::xScopeColors() : notUseBit(0)
{
  Color[cLineShow1]  = RGB(0xff, 0xff, 0x00);
  Color[cLineShow2]  = RGB(0xdf, 0xdf, 0xdf);
  Color[cGrid]  = RGB(0x3f, 0x50, 0x9f);
  Color[cAxe]   = RGB(0xff, 0x00, 0x00);
  Color[cBkg]    = RGB(0x9f, 0x9f, 0x9f);
}
//-----------------------------------------------------------
void xScopeColors::clone(const xScopeColors& other)
{
  for(uint i = 0; i < SIZE_A(Color); ++i)
    CLONE(Color[i]);
  CLONE(notUseBit);
}
//-----------------------------------------------------------
PropertyXScope::~PropertyXScope()
{
}
//-----------------------------------------------------------
inline
void chgBit(DWORD& bit, DWORD reset, DWORD set)
{
  bit &= ~set;
  bit |= reset;
}
//-----------------------------------------------------------
void sampleXScope::chgBit(DWORD reset, DWORD set)
{
  ::chgBit(RefColors.notUseBit, reset, set);
}
//-----------------------------------------------------------
LRESULT sampleXScope::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      do {
        PAINTSTRUCT Paint;
        HDC hdc = BeginPaint(hwnd, &Paint);

        evPaint(hdc);

        EndPaint(hwnd, &Paint);
        } while(false);
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
extern void arrowUp(HDC hdc, int x, int y);
extern void arrowRight(HDC hdc, int x, int y);
//-----------------------------------------------------------
void sampleXScope::evPaint(HDC hdc)
{
  drawAxis(hdc);
  drawLines(hdc);
}
//----------------------------------------------------------------------------
#define USE_COLOR_DW(dw,c) (!(dw & (1 << xScopeColors::c)))
//----------------------------------------------------------------------------
#define USE_COLOR(c) USE_COLOR_DW(RefColors.notUseBit, c)
//----------------------------------------------------------------------------
void sampleXScope::drawAxis(HDC hdc)
{
  PRect r;
  GetClientRect(*this, r);
  COLORREF oldColor;

  if(USE_COLOR(cBkg)) {
    HBRUSH br = CreateSolidBrush(RefColors.Color[xScopeColors::cBkg]);
    FillRect(hdc, r, br);
    DeleteObject(HGDIOBJ(br));
    oldColor = SetBkColor(hdc, RefColors.Color[xScopeColors::cBkg]);
    }

  if(USE_COLOR(cGrid)) {
    HPEN Pen2 = CreatePen(PS_DOT, 1, RefColors.Color[xScopeColors::cGrid]);
    HGDIOBJ oldPen = SelectObject(hdc, Pen2);

    const int numCol = 4;
    double stepx = r.Width() / (double)numCol;
    double j = stepx;
    for(int i = 0; i < numCol; ++i, j += stepx) {
      long x = (long)j + r.left;
      MoveToEx(hdc, x, r.top, 0);
      LineTo(hdc, x, r.bottom);
      }

    const int nRow = 4;
    double stepy = r.Height() / (double)nRow;
    j = stepy;
    for(int i = 0; i < nRow - 1; ++i, j += stepy) {
      long y = (LONG)j + r.top;
      MoveToEx(hdc, r.left, y, 0);
      LineTo(hdc, r.right, y);
      }
    SelectObject(hdc, oldPen);
    DeleteObject(Pen2);
    }

  if(USE_COLOR(cAxe)) {
    HPEN Pen = CreatePen(PS_SOLID, 2, RefColors.Color[xScopeColors::cAxe]);

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
  if(USE_COLOR(cBkg))
    SetBkColor(hdc, oldColor);
}
//-----------------------------------------------------------
void sampleXScope::drawLines(HDC hdc)
{
  PRect r;
  GetClientRect(*this, r);

  HPEN Pen1 = CreatePen(PS_SOLID, 1, RefColors.Color[xScopeColors::cLineShow1]);
  HPEN Pen2 = CreatePen(PS_SOLID, 1, RefColors.Color[xScopeColors::cLineShow2]);
  HGDIOBJ oldPen = SelectObject(hdc, Pen1);

  int width = r.Width() / 4;
  srand(width);
  MoveToEx(hdc, r.left, (r.bottom + r.top) / 2, 0);
  for(int i = 4, x = 4; i < width; ++i, x += 2) {
    int y = (rand() % r.Height()) + r.top;
    LineTo(hdc, x, y);
    }

  SelectObject(hdc, Pen2);
  int width2 = r.Width() / 2;
  for(int i = width, x = width * 2; i < width2; ++i, x += 2) {
    int y = (rand() % r.Height()) + r.top;
    LineTo(hdc, x, y);
    }

  SelectObject(hdc, oldPen);
  DeleteObject(Pen1);
  DeleteObject(Pen2);
}
//-----------------------------------------------------------
void PropertyXScope::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyXScope* po = dynamic_cast<const PropertyXScope*>(&other);
  if(po && po != this) {

    xScopeColor = po->getColors();

    for(uint i = 0; i < SIZE_A(DataPrf); ++i)
      DataPrf[i] = po->DataPrf[i];

    uniqueData = po->uniqueData;
    useVariableX = po->useVariableX;
    relativeBlock = po->relativeBlock;
    zeroOffset = po->zeroOffset;
    }
}
//-----------------------------------------------------------
void PropertyXScope::cloneMinusProperty(const Property& other)
{
  clone(other);
}
//-----------------------------------------------------------
static
ids Ids[] = {
  { IDC_COMBOBOX_PERIFS6, IDC_EDIT_ADDR6, IDC_COMBOBOX_TYPEVAL6, IDC_EDIT_NORMALIZ6 },

  { IDC_COMBOBOX_PERIF_INIT_X, IDC_EDIT_ADDR_INIT_X, IDC_COMBOBOX_TYPEVAL_INIT_X, IDC_EDIT_NORMALIZ_INIT_X },
  { IDC_COMBOBOX_PERIF_CURR_X, IDC_EDIT_ADDR_CURR_X_V, IDC_COMBOBOX_TYPEVAL_CURR_X_V, IDC_EDIT_NORMALIZ_CURR_X },
  };
//-----------------------------------------------------------
static
ids IdsEx[] = {
  { IDC_COMBOBOX_PERIFS2, IDC_EDIT_ADDR2, IDC_COMBOBOX_TYPEVAL2, IDC_EDIT_NORMALIZ2 },
  { IDC_COMBOBOX_PERIFS3, IDC_EDIT_ADDR3, IDC_COMBOBOX_TYPEVAL3, IDC_EDIT_NORMALIZ3 },
  { IDC_COMBOBOX_PERIFS4, IDC_EDIT_ADDR4, IDC_COMBOBOX_TYPEVAL4, IDC_EDIT_NORMALIZ4 },
  { IDC_COMBOBOX_PERIFS5, IDC_EDIT_ADDR5, IDC_COMBOBOX_TYPEVAL5, IDC_EDIT_NORMALIZ5 },
  };
//-----------------------------------------------------------
void svmDialogXScope::check_const_prph(uint ids)
{
  uint idcs[] = { IDC_BUTTON_NORMALIZ2, IDC_BUTTON_NORMALIZ3, IDC_BUTTON_NORMALIZ4, IDC_BUTTON_NORMALIZ5 };
  uint idlabel[] = { IDC_STATIC_XS_NBLK, IDC_STATIC_XS_NDATA, IDC_STATIC_XS_MIN, IDC_STATIC_XS_MAX };
  HWND hwnd = GetDlgItem(*this, ids);
  int sel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  bool enableAll = PRPH_4_CONST_CB_SEL != sel;
  for(uint i = 0; i < SIZE_A(IdsEx); ++i) {
    if(IdsEx[i].idPrph == ids) {
      ENABLE(IdsEx[i].idNorm, enableAll);
      ENABLE(IdsEx[i].idType, true /*enableAll*/);
      ENABLE(idcs[i], enableAll);
      if(enableAll)
        SET_TEXT(idlabel[i], _T("Addr"));
      else
        SET_TEXT(idlabel[i], _T("Value"));
      break;
      }
    }
}
//-----------------------------------------------------------
void invalidateSample(PWin* win)
{
  PWin* parent = win->getParent();
  PRect r;
  GetWindowRect(*win, r);
  r.Inflate(5, 5);
  MapWindowPoints(HWND_DESKTOP, *parent, (LPPOINT)(LPRECT)r, 2);
  InvalidateRect(*parent, r, 1);
}
//-----------------------------------------------------------
bool svmDialogXScope::create()
{
  sxs = new sampleXScope(this, IDC_SAMPLE_XSCOPE);

  if(!baseClass::create())
    return false;

  if(!(Prop->style & Property::FILL) && !(Prop->style & Property::TRANSP))
    SET_CHECK(IDC_RADIOBUTTON_NO_BKG_PANEL);

  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(tmpProp);

  sxs->setColors(pt->getColors());
  invalidateSample(sxs);

  for(uint i = 0; i < SIZE_A(Ids); ++i)
    fillData(Ids[i], pt->DataPrf[i]);

  for(uint i = 0, j = SIZE_A(Ids); i < SIZE_A(IdsEx); ++i, ++j)
    fillDataEx(IdsEx[i], pt->DataPrf[j]);

  SET_INT(IDC_EDIT_NUM_ROW_XSCOPE, Prop->type1);
  SET_INT(IDC_EDIT_ADDR_BITS_READY, Prop->type2);

  DWORD notUseBit = pt->getColors().notUseBit;
  if(USE_COLOR_DW(notUseBit, cBkg))
    SET_CHECK(IDC_CHECK_COL_BKG);

  if(USE_COLOR_DW(notUseBit, cGrid))
    SET_CHECK(IDC_CHECK_COL_GRID);

  if(USE_COLOR_DW(notUseBit, cAxe))
    SET_CHECK(IDC_CHECK_COL_AXES);
  checkEnableColor();

  if(pt->uniqueData)
    SET_CHECK(IDC_CHECK_UNIQUE_DATA);

  if(pt->useVariableX)
    SET_CHECK(IDC_CHECK_AXES_X);
  if(pt->relativeBlock)
    SET_CHECK(IDC_CHECK_AXES_X_REL);
  if(pt->zeroOffset)
    SET_CHECK(IDC_CHECK_AXES_X_ZERO);
  checkEnableAxesX();
  setInfoReadyOrOffset();

  for(uint i = 0; i < SIZE_A(IdsEx); ++i)
    check_const_prph(IdsEx[i].idPrph);
  return true;
}
//-----------------------------------------------------------
#define MAKE_COL_BIT(t) (1 << (DWORD)xScopeColors::t)
//-----------------------------------------------------------
void svmDialogXScope::chgBit(DWORD reset, DWORD set)
{
  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(tmpProp);
  xScopeColors& colors = pt->getColors();
  ::chgBit(colors.notUseBit, reset, set);
  sxs->chgBit(reset, set);
}
//-----------------------------------------------------------
void svmDialogXScope::setBitsColor(DWORD idc, bool alsoRadio)
{
  bool active = IS_CHECKED(idc);
  DWORD value = 0;
  switch(idc) {
    case IDC_CHECK_COL_AXES:
      value = MAKE_COL_BIT(cAxe);
      break;
    case IDC_CHECK_COL_GRID:
      value = MAKE_COL_BIT(cGrid);
      break;
    case IDC_CHECK_COL_BKG:
      value = MAKE_COL_BIT(cBkg);
      if(alsoRadio) {
        SET_CHECK_SET(IDC_RADIOBUTTON_FILL_PANEL, active);
        SET_CHECK_SET(IDC_RADIOBUTTON_NO_BKG_PANEL, !active);
        }
      break;
    }
  if(value) {
    if(active)
      chgBit(0, value);
    else
      chgBit(value, 0);
    checkEnableColor();
    invalidateSample(sxs);
    }
}
//-----------------------------------------------------------
void svmDialogXScope::checkEnableAxesX()
{
  bool enable = IS_CHECKED(IDC_CHECK_AXES_X);
  ENABLE(IDC_CHECK_AXES_X_REL, enable);
  ENABLE(IDC_CHECK_AXES_X_ZERO, enable);

  ENABLE(IDC_BUTTON_NORMALIZ_INIT_X, enable);
  ENABLE(IDC_BUTTON_NORMALIZ_CURR_X_V, enable);
  for(uint i = SIZE_A(Ids) - 2; i < SIZE_A(Ids); ++i) {
    ENABLE(Ids[i].idPrph, enable);
    ENABLE(Ids[i].idAddr, enable);
    ENABLE(Ids[i].idType, enable);
    ENABLE(Ids[i].idNorm, enable);
    }
  if(enable) {
    if(IS_CHECKED(IDC_CHECK_UNIQUE_DATA)) {
      SET_CHECK_SET(IDC_CHECK_UNIQUE_DATA, false);
      setInfoReadyOrOffset();
      }
    ENABLE(IDC_CHECK_UNIQUE_DATA, false);
    }
  else
    ENABLE(IDC_CHECK_UNIQUE_DATA, true);
}
//-----------------------------------------------------------
void svmDialogXScope::setPrphEqu()
{
  TCHAR t[256];
  if(IS_CHECKED(IDC_CHECK_UNIQUE_DATA))
    _tcscpy_s(t, _T("Perif 1"));
  else
    GET_TEXT(IDC_COMBOBOX_PERIFS, t);
  SET_TEXT(IDC_STATIC_PRPH_INFO, t);
}
//-----------------------------------------------------------
void svmDialogXScope::checkEnableColor()
{
  bool enable = IS_CHECKED(IDC_CHECK_COL_BKG);
  EnableWindow(GetDlgItem(*this, IDC_BUTTON_XSCOPE_BKG), enable);

  enable = IS_CHECKED(IDC_CHECK_COL_GRID);
  EnableWindow(GetDlgItem(*this, IDC_BUTTON_XSCOPE_GRID), enable);

  enable = IS_CHECKED(IDC_CHECK_COL_AXES);
  EnableWindow(GetDlgItem(*this, IDC_BUTTON_XSCOPE_AXES), enable);
}
//-----------------------------------------------------------
void svmDialogXScope::fillData(const ids& Ids, const dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  fillCBPerif(hwnd, DataPrf.perif);

  SET_INT(Ids.idAddr, DataPrf.addr);

  hwnd = ::GetDlgItem(*this, Ids.idType);
  fillCBTypeVal(hwnd, DataPrf.typeVal);

  SET_INT(Ids.idNorm, DataPrf.normaliz);
}
//-----------------------------------------------------------
void svmDialogXScope::fillDataEx(const ids& Ids, const dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  int prph = DataPrf.perif;
  if(PRPH_4_CONST == prph)
    prph = PRPH_4_CONST_CB_SEL;
  fillCBPerifEx(hwnd, prph, true);

  setConstValue(GetDlgItem(*this, Ids.idAddr), DataPrf.addr, DataPrf.perif, DataPrf.typeVal);

  hwnd = ::GetDlgItem(*this, Ids.idType);
  fillCBTypeVal(hwnd, DataPrf.typeVal);

  SET_INT(Ids.idNorm, DataPrf.normaliz);
}
//-----------------------------------------------------------
LRESULT svmDialogXScope::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_XSCOPE_BKG:
        case IDC_BUTTON_XSCOPE_GRID:
        case IDC_BUTTON_XSCOPE_AXES:
        case IDC_BUTTON_XSCOPE_LINE1:
        case IDC_BUTTON_XSCOPE_LINE2:
          if(chooseColor(LOWORD(wParam)))
            drawColors();
          break;

        case IDC_CHECK_COL_BKG:
        case IDC_CHECK_COL_GRID:
        case IDC_CHECK_COL_AXES:
          setBitsColor(LOWORD(wParam));
          break;

        case IDC_CHECK_AXES_X:
          checkEnableAxesX();
          break;
        case IDC_RADIOBUTTON_FILL_PANEL:
          SET_CHECK(IDC_CHECK_COL_BKG);
          setBitsColor(IDC_CHECK_COL_BKG, false);
          break;

        case IDC_RADIOBUTTON_NO_BKG_PANEL:
          SET_CHECK_SET(IDC_CHECK_COL_BKG, false);
          setBitsColor(IDC_CHECK_COL_BKG, false);
          break;

        case IDC_BUTTON_NORMALIZ2:
          chooseNormaliz(IDC_EDIT_NORMALIZ2);
          break;
        case IDC_BUTTON_NORMALIZ3:
          chooseNormaliz(IDC_EDIT_NORMALIZ3);
          break;
        case IDC_BUTTON_NORMALIZ4:
          chooseNormaliz(IDC_EDIT_NORMALIZ4);
          break;
        case IDC_BUTTON_NORMALIZ5:
          chooseNormaliz(IDC_EDIT_NORMALIZ5);
          break;
        case IDC_BUTTON_NORMALIZ_INIT_X:
          chooseNormaliz(IDC_EDIT_NORMALIZ_INIT_X);
          break;
        case IDC_BUTTON_NORMALIZ_CURR_X_V:
          chooseNormaliz(IDC_EDIT_NORMALIZ_CURR_X);
          break;

        case IDC_CHECK_UNIQUE_DATA:
          setInfoReadyOrOffset();
          break;
        case IDC_COMBOBOX_PERIFS:
          setPrphEqu();
          break;
        case IDC_COMBOBOX_PERIFS2:
        case IDC_COMBOBOX_PERIFS3:
        case IDC_COMBOBOX_PERIFS4:
        case IDC_COMBOBOX_PERIFS5:
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
bool svmDialogXScope::chooseColor(uint idc)
{
  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(tmpProp);
  xScopeColors& colors = pt->getColors();
  uint ix = 0;
  switch(idc) {
    case IDC_BUTTON_XSCOPE_BKG:
      break;
    case IDC_BUTTON_XSCOPE_GRID:
      ix = 1;
      break;
    case IDC_BUTTON_XSCOPE_AXES:
      ix = 2;
      break;
    case IDC_BUTTON_XSCOPE_LINE1:
      ix = 3;
      break;
    case IDC_BUTTON_XSCOPE_LINE2:
      ix = 4;
      break;
    }
  return choose_Color(*this, colors.Color[ix]);
}
//-----------------------------------------------------------
void svmDialogXScope::setInfoReadyOrOffset()
{
  LPCTSTR info = IS_CHECKED(IDC_CHECK_UNIQUE_DATA) ? _T("Indirizzo Offset (Prph 1)") : _T("Punto di elaborazione");
  SET_TEXT(IDC_GROUPBOX_PT_ELAB, info);
  setPrphEqu();
}
//-----------------------------------------------------------
void svmDialogXScope::drawColors()
{
  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(tmpProp);
  sxs->setColors(pt->getColors());
  invalidateSample(sxs);
//  InvalidateRect(*sxs, 0, 0);
}
//-----------------------------------------------------------
void svmDialogXScope::unfillData(const ids& Ids, dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  DataPrf.perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  GET_INT(Ids.idAddr, DataPrf.addr);

  hwnd = ::GetDlgItem(*this, Ids.idType);
  DataPrf.typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  GET_INT(Ids.idNorm, DataPrf.normaliz);
}
//-----------------------------------------------------------
void svmDialogXScope::unfillDataEx(const ids& Ids, dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  DataPrf.perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  hwnd = ::GetDlgItem(*this, Ids.idType);
  if(PRPH_4_CONST_CB_SEL == DataPrf.perif) {
    DataPrf.perif = PRPH_4_CONST;
    TCHAR t[128];
    GET_TEXT(Ids.idAddr, t);
    zeroTrim(t);
    DWORD v;
    DataPrf.typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0); //4;//prfData::tDWData;
    bool isReal = getConstValue(t, v, DataPrf.typeVal);
    DataPrf.addr = v;
    DataPrf.normaliz = 0;
    }
  else {
    GET_INT(Ids.idAddr, DataPrf.addr);
    DataPrf.typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    GET_INT(Ids.idNorm, DataPrf.normaliz);
    }
}
//-----------------------------------------------------------
void svmDialogXScope::CmOk()
{
  PropertyXScope* pt = dynamic_cast<PropertyXScope*>(tmpProp);
  if(pt) {
    for(uint i = 0; i < SIZE_A(Ids); ++i)
      unfillData(Ids[i], pt->DataPrf[i]);
    for(uint i = 0, j = SIZE_A(Ids); i < SIZE_A(IdsEx); ++i, ++j)
      unfillDataEx(IdsEx[i], pt->DataPrf[j]);
    }
  GET_INT(IDC_EDIT_NUM_ROW_XSCOPE, tmpProp->type1);
  GET_INT(IDC_EDIT_ADDR_BITS_READY, tmpProp->type2);

  pt->uniqueData = IS_CHECKED(IDC_CHECK_UNIQUE_DATA);
  pt->useVariableX = IS_CHECKED(IDC_CHECK_AXES_X);
  pt->relativeBlock = IS_CHECKED(IDC_CHECK_AXES_X_REL);
  pt->zeroOffset = IS_CHECKED(IDC_CHECK_AXES_X_ZERO);

  baseClass::CmOk();
}
//-----------------------------------------------------------
