//--------------- svmPropertyPlotXY.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyPlotXY.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "PStatic.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
inline
void chgBit(DWORD& bit, DWORD reset, DWORD set)
{
  bit &= ~set;
  bit |= reset;
}
//-----------------------------------------------------------
#define USE_COLOR_DW(dw,c) (!(dw & (1 << xScopeColors::c)))
//----------------------------------------------------------------------------
#define USE_COLOR(c) USE_COLOR_DW(RefColors.notUseBit, c)
//----------------------------------------------------------------------------
//-----------------------------------------------------------
PropertyPlotXY::~PropertyPlotXY()
{
}
//-----------------------------------------------------------
void PropertyPlotXY::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyPlotXY* po = dynamic_cast<const PropertyPlotXY*>(&other);
  if(po && po != this) {

    xScopeColor = po->getColors();

    for(uint i = 0; i < SIZE_A(DataPrf); ++i)
      DataPrf[i] = po->DataPrf[i];
    relativeBlock_Y = po->relativeBlock_Y;
    relativeBlock_X = po->relativeBlock_X;
    }
}
//-----------------------------------------------------------
void PropertyPlotXY::cloneMinusProperty(const Property& other)
{
  clone(other);
}
//-----------------------------------------------------------
static
ids Ids[] = {
 { IDC_COMBOBOX_PERIF_INIT_X, IDC_EDIT_ADDR_INIT_X, IDC_COMBOBOX_TYPEVAL_INIT_X, IDC_EDIT_NORMALIZ_INIT_X },
 { IDC_COMBOBOX_PERIF_CURR_X, IDC_EDIT_ADDR_CURR_X_V, IDC_COMBOBOX_TYPEVAL_CURR_X_V, IDC_EDIT_NORMALIZ_CURR_X },
 { IDC_COMBOBOX_PERIF_ENABLE_READ, IDC_EDIT_ADDR6, IDC_COMBOBOX_TYPEVAL6, IDC_EDIT_NORMALIZ6  },
 };
//-----------------------------------------------------------
static
ids IdsEx[] = {
 { IDC_COMBOBOX_PERIF_MIN_Y, IDC_EDIT_ADDR_PXY_MIN_Y, IDC_COMBOBOX_TYPEVAL_PXY_MIN_Y, IDC_EDIT_NORMALIZ_PXY_MIN_Y },
 { IDC_COMBOBOX_PERIF_MAX_Y, IDC_EDIT_ADDR_PXY_MAX_Y, IDC_COMBOBOX_TYPEVAL_PXY_MAX_Y, IDC_EDIT_NORMALIZ_PXY_MAX_Y },
 { IDC_COMBOBOX_PERIF_MIN_X, IDC_EDIT_ADDR_PXY_MIN_X, IDC_COMBOBOX_TYPEVAL_PXY_MIN_X, IDC_EDIT_NORMALIZ_PXY_MIN_X },
 { IDC_COMBOBOX_PERIF_MAX_X, IDC_EDIT_ADDR_PXY_MAX_X, IDC_COMBOBOX_TYPEVAL_PXY_MAX_X, IDC_EDIT_NORMALIZ_PXY_MAX_X },
 { IDC_COMBOBOX_PERIF_NUM_DATA, IDC_EDIT_ADDR3, IDC_COMBOBOX_TYPEVAL3, IDC_EDIT_NORMALIZ3 },
 };
//-----------------------------------------------------------
void svmDialogPlotXY::check_const_prph(uint ids)
{
  uint idcs[] = { IDC_BUTTON_NORMALIZ_PXY_MIN_Y, IDC_BUTTON_NORMALIZ_PXY_MAX_Y, IDC_BUTTON_NORMALIZ_PXY_MIN_X, IDC_BUTTON_NORMALIZ_PXY_MAX_X, IDC_BUTTON_NORMALIZ3 };
  uint idlabel[] = { IDC_STATIC_XY_MINY, IDC_STATIC_XY_MAXY, IDC_STATIC_XY_MINX, IDC_STATIC_XY_MAXX, IDC_STATIC_XY_NDATA };
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
extern void invalidateSample(PWin* win);
//-----------------------------------------------------------
bool svmDialogPlotXY::create()
{
  sxs = new sampleXScope(this, IDC_SAMPLE_XSCOPE);

  if(!baseClass::create())
    return false;

  if(!(Prop->style & Property::FILL) && !(Prop->style & Property::TRANSP))
    SET_CHECK(IDC_RADIOBUTTON_NO_BKG_PANEL);

  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(tmpProp);

  sxs->setColors(pt->getColors());
  invalidateSample(sxs);

  for(uint i = 0; i < SIZE_A(Ids); ++i)
    fillData(Ids[i], pt->DataPrf[i]);
  for(uint i = 0, j = SIZE_A(Ids); i < SIZE_A(IdsEx); ++i, ++j)
    fillDataEx(IdsEx[i], pt->DataPrf[j]);

  SET_INT(IDC_EDIT_NUM_ROW_XSCOPE, Prop->type1);
  SET_INT(IDC_EDIT_NUM_COL_XSCOPE, Prop->type2);

  DWORD notUseBit = pt->getColors().notUseBit;
  if(USE_COLOR_DW(notUseBit, cBkg))
    SET_CHECK(IDC_CHECK_COL_BKG);

  if(USE_COLOR_DW(notUseBit, cGrid))
    SET_CHECK(IDC_CHECK_COL_GRID);

  if(USE_COLOR_DW(notUseBit, cAxe))
    SET_CHECK(IDC_CHECK_COL_AXES);
  checkEnableColor();

  if(pt->relativeBlock_X)
    SET_CHECK(IDC_CHECK_AXES_X_REL);
  if(pt->relativeBlock_Y)
    SET_CHECK(IDC_CHECK_AXES_Y_REL);
  for(uint i = 0; i < SIZE_A(IdsEx); ++i)
    check_const_prph(IdsEx[i].idPrph);
  return true;
}
//-----------------------------------------------------------
#define MAKE_COL_BIT(t) (1 << (DWORD)xScopeColors::t)
//-----------------------------------------------------------
void svmDialogPlotXY::chgBit(DWORD reset, DWORD set)
{
  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(tmpProp);
  xScopeColors& colors = pt->getColors();
  ::chgBit(colors.notUseBit, reset, set);
  sxs->chgBit(reset, set);
}
//-----------------------------------------------------------
void svmDialogPlotXY::setBitsColor(DWORD idc, bool alsoRadio)
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
void svmDialogPlotXY::checkEnableColor()
{
  bool enable = IS_CHECKED(IDC_CHECK_COL_BKG);
  EnableWindow(GetDlgItem(*this, IDC_BUTTON_XSCOPE_BKG), enable);

  enable = IS_CHECKED(IDC_CHECK_COL_GRID);
  EnableWindow(GetDlgItem(*this, IDC_BUTTON_XSCOPE_GRID), enable);

  enable = IS_CHECKED(IDC_CHECK_COL_AXES);
  EnableWindow(GetDlgItem(*this, IDC_BUTTON_XSCOPE_AXES), enable);
}
//-----------------------------------------------------------
void svmDialogPlotXY::fillData(const ids& Ids, const dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  fillCBPerif(hwnd, DataPrf.perif);

  SET_INT(Ids.idAddr, DataPrf.addr);

  hwnd = ::GetDlgItem(*this, Ids.idType);
  fillCBTypeVal(hwnd, DataPrf.typeVal);

  SET_INT(Ids.idNorm, DataPrf.normaliz);
}
//-----------------------------------------------------------
void svmDialogPlotXY::fillDataEx(const ids& Ids, const dataPrf& DataPrf)
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
#define CHECK_NORM(a) \
  case IDC_BUTTON_NORMALIZ##a: \
    chooseNormaliz(IDC_EDIT_NORMALIZ##a); \
    break
//-----------------------------------------------------------
LRESULT svmDialogPlotXY::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

        case IDC_RADIOBUTTON_FILL_PANEL:
          SET_CHECK(IDC_CHECK_COL_BKG);
          setBitsColor(IDC_CHECK_COL_BKG, false);
          break;

        case IDC_RADIOBUTTON_NO_BKG_PANEL:
          SET_CHECK_SET(IDC_CHECK_COL_BKG, false);
          setBitsColor(IDC_CHECK_COL_BKG, false);
          break;

        CHECK_NORM(_PXY_MIN_Y);
        CHECK_NORM(_PXY_MAX_Y);
        CHECK_NORM(_PXY_MIN_X);
        CHECK_NORM(_PXY_MAX_X);
        CHECK_NORM(_INIT_X);

//        CHECK_NORM(2);
        CHECK_NORM(3);

        case IDC_BUTTON_NORMALIZ_CURR_X_V:
          chooseNormaliz(IDC_EDIT_NORMALIZ_CURR_X);
          break;
        case IDC_COMBOBOX_PERIF_MIN_Y:
        case IDC_COMBOBOX_PERIF_MAX_Y:
        case IDC_COMBOBOX_PERIF_MIN_X:
        case IDC_COMBOBOX_PERIF_MAX_X:
        case IDC_COMBOBOX_PERIF_NUM_DATA:
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
bool svmDialogPlotXY::chooseColor(uint idc)
{
  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(tmpProp);
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
void svmDialogPlotXY::drawColors()
{
  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(tmpProp);
  sxs->setColors(pt->getColors());
  invalidateSample(sxs);
}
//-----------------------------------------------------------
void svmDialogPlotXY::unfillData(const ids& Ids, dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  DataPrf.perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  GET_INT(Ids.idAddr, DataPrf.addr);

  hwnd = ::GetDlgItem(*this, Ids.idType);
  DataPrf.typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  GET_INT(Ids.idNorm, DataPrf.normaliz);
}
//-----------------------------------------------------------
void svmDialogPlotXY::unfillDataEx(const ids& Ids, dataPrf& DataPrf)
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
void svmDialogPlotXY::CmOk()
{
  PropertyPlotXY* pt = dynamic_cast<PropertyPlotXY*>(tmpProp);
  if(pt) {
    for(uint i = 0; i < SIZE_A(Ids); ++i)
      unfillData(Ids[i], pt->DataPrf[i]);
    for(uint i = 0, j = SIZE_A(Ids); i < SIZE_A(IdsEx); ++i, ++j)
      unfillDataEx(IdsEx[i], pt->DataPrf[j]);
    }
  GET_INT(IDC_EDIT_NUM_ROW_XSCOPE, tmpProp->type1);
  GET_INT(IDC_EDIT_NUM_COL_XSCOPE, tmpProp->type2);
  pt->relativeBlock_X = IS_CHECKED(IDC_CHECK_AXES_X_REL);
  pt->relativeBlock_Y = IS_CHECKED(IDC_CHECK_AXES_Y_REL);
  baseClass::CmOk();
}
//-----------------------------------------------------------
