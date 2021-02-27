//--------------- svmPropertyBarGraph.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyBarGraph.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "svmObjBarGraph.h"
#include "svmNormal.h"
#include "svmPropertyText.h"
#include "common.h"
//-----------------------------------------------------------
PropertyBarGraph::~PropertyBarGraph()
{
}
//-----------------------------------------------------------
void PropertyBarGraph::copyVarsToBase()
{
  perif    = vI[0].perif;
  addr     = vI[0].addr;
  typeVal  = vI[0].typeVal;
  normaliz = vI[0].normaliz;
  nBits    = vI[0].nBit;
  offset   = vI[0].offs;
}
//-----------------------------------------------------------
void PropertyBarGraph::copyVarsFromBase()
{
  vI[0].perif =    perif;
  vI[0].addr =     addr;
  vI[0].typeVal =  typeVal;
  vI[0].normaliz = normaliz;
  vI[0].nBit =     nBits;
  vI[0].offs =     offset;
}
//-----------------------------------------------------------
void PropertyBarGraph::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyBarGraph* po = dynamic_cast<const PropertyBarGraph*>(&other);
  if(po && po != this) {

    const setOfValueColor& oSet = po->getValueColors();
    int nElem = oSet.getElem();
    ValueColor.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      ValueColor[i] = oSet[i];
    for(uint i = 0; i < SIZE_A(vI); ++i)
      vI[i] = po->vI[i];
    negValue = po->negValue;
    for(uint i = 0; i < SIZE_A(iBmp); ++i)
      iBmp[i] = po->iBmp[i];
    centerZero = po->centerZero;
    valueTest[0] = po->valueTest[0];
    valueTest[1] = po->valueTest[1];
    }
}
//-----------------------------------------------------------
int PropertyBarGraph::infoBmp::getFlag() const
{
  int flag = 0;
  if(tiled)
    flag |= PGraphBmp::bmpTiled;
  if(rotate)
    flag |= PGraphBmp::bmpRotate;
  if(mirror)
    flag |= PGraphBmp::bmpMirror;
  return flag;
}
//-----------------------------------------------------------
void PropertyBarGraph::infoBmp::setFlag(int flag)
{
  tiled = flag & PGraphBmp::bmpTiled ? 1 : 0;
  rotate = flag & PGraphBmp::bmpRotate ? 1 : 0;
  mirror = flag & PGraphBmp::bmpMirror ? 1 : 0;
}
//-----------------------------------------------------------
void PropertyBarGraph::cloneMinusProperty(const Property& other)
{
  baseClass::clone(other);
}
//-----------------------------------------------------------
bool svmDialogBarGraph::create()
{
  PropertyBarGraph* pb = dynamic_cast<PropertyBarGraph*>(Prop);
  if(pb)
    pb->copyVarsToBase();

  if(!baseClass::create())
    return false;

  SET_CHECK_SET(IDC_CHECK_CENTER_ZERO, pb->centerZero & 1);
  if(pb->centerZero & 2)
    SET_CHECK(IDC_RADIO_ZERO_TO_EDGE);
  else
    SET_CHECK(IDC_RADIO_MIN_TO_MAX);

  SET_INT(IDC_EDIT_ZERO_MIN, pb->valueTest[0]);
  SET_INT(IDC_EDIT_ZERO_MAX, pb->valueTest[1]);

  SET_CHECK_SET(IDC_CHECKBOX_TRANSP_PANEL, Prop->type1 & 1);
  SET_CHECK_SET(IDC_CHECKBOX_FILL_PANEL, Prop->type1 & 2);
  if(!(Prop->style & Property::FILL)) {
    SET_CHECK(IDC_RADIOBUTTON_NO_ERASE_BKG);
    SET_CHECK_SET(IDC_RADIOBUTTON_DOWN_BORDER, false);
    }

  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_2);
  int prph = pb->vI[1].perif;
  if(PRPH_4_CONST == prph)
    prph = PRPH_4_CONST_CB_SEL;
  fillCBPerifEx(hwnd, prph, true);
  setConstValue(GetDlgItem(*this, IDC_EDIT_ADDR_2), pb->vI[1].addr, pb->vI[1].perif, pb->vI[1].typeVal);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL_2);
  fillCBTypeVal(hwnd, pb->vI[1].typeVal);
  SET_INT(IDC_EDIT_NBITS_2, pb->vI[1].nBit);
  SET_INT(IDC_EDIT_OFFSET_2, pb->vI[1].offs);
  SET_INT(IDC_EDIT_NORMALIZ_2, pb->vI[1].normaliz);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_MIN);
  prph = pb->vI[2].perif;
  if(PRPH_4_CONST == prph)
    prph = PRPH_4_CONST_CB_SEL;
  fillCBPerifEx(hwnd, prph, true);
  setConstValue(GetDlgItem(*this, IDC_EDIT_ADDR_MIN), pb->vI[2].addr, pb->vI[2].perif, pb->vI[2].typeVal);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL_MIN);
  fillCBTypeVal(hwnd, pb->vI[2].typeVal);
  SET_INT(IDC_EDIT_NBITS_MIN, pb->vI[2].nBit);
  SET_INT(IDC_EDIT_OFFSET_MIN, pb->vI[2].offs);
  SET_INT(IDC_EDIT_NORMALIZ_MIN, pb->vI[2].normaliz);

  SET_CHECK_SET(IDC_CHECKBOX_NEG_VALUE, pb->negValue);

  if(*pb->iBmp[0].img) {
    SET_CHECK(IDC_CHECK_USE_BMP_BKG);
    SET_CHECK_SET(IDC_CHECK_USE_BMP_BKG_ROTATE, pb->iBmp[0].rotate);
    SET_CHECK_SET(IDC_CHECK_USE_BMP_BKG_MIRROR, pb->iBmp[0].mirror);
    SET_CHECK_SET(IDC_CHECK_USE_BMP_BKG_TILED, pb->iBmp[0].tiled);
    SET_TEXT(IDC_STATICTEXT_BKG_BMP, pb->iBmp[0].img);
    }

  if(*pb->iBmp[1].img) {
    SET_CHECK(IDC_CHECK_USE_BMP_BAR);
    SET_CHECK_SET(IDC_CHECK_USE_BMP_BAR_ROTATE, pb->iBmp[1].rotate);
    SET_CHECK_SET(IDC_CHECK_USE_BMP_BAR_MIRROR, pb->iBmp[1].mirror);
    SET_CHECK_SET(IDC_CHECK_USE_BMP_BAR_TILED, pb->iBmp[1].tiled);
    SET_TEXT(IDC_STATICTEXT_BAR_BMP, pb->iBmp[1].img);
    }
  checkEnable(false);
  checkEnable(true);
  check_const_prph(IDC_COMBOBOX_PERIFS_2);
  check_const_prph(IDC_COMBOBOX_PERIFS_MIN);
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogBarGraph::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_BG:
          if(IS_CHECKED(IDC_CHECK_USE_BMP_BKG)) {
            chooseBmp(IDC_STATICTEXT_BKG_BMP);
            return 0;
            }
          break;

        case IDC_BUTTON_TEXT_AND_COLOR:
          if(IS_CHECKED(IDC_CHECK_USE_BMP_BAR))
            chooseBmp(IDC_STATICTEXT_BAR_BMP);
          else
            chooseValueAndColor();
          break;
        case IDC_BUTTON_NORMALIZ_2:
          chooseNormaliz(IDC_EDIT_NORMALIZ_2);
          break;
        case IDC_BUTTON_NORMALIZ_MIN:
          chooseNormaliz(IDC_EDIT_NORMALIZ_MIN);
          break;
        case IDC_CHECK_USE_BMP_BKG:
          checkEnable(false);
          break;
        case IDC_CHECK_USE_BMP_BAR:
          checkEnable(true);
          break;
        case IDC_COMBOBOX_PERIFS_2:
        case IDC_COMBOBOX_PERIFS_MIN:
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
void svmDialogBarGraph::check_const_prph(uint ids)
{
  uint idcs[][6] = {
    { IDC_COMBOBOX_PERIFS_2,  /*IDC_COMBOBOX_TYPEVAL_2, */IDC_EDIT_NBITS_2, IDC_EDIT_OFFSET_2, IDC_EDIT_NORMALIZ_2, IDC_BUTTON_NORMALIZ_2 },
    { IDC_COMBOBOX_PERIFS_MIN, /*IDC_COMBOBOX_TYPEVAL_MIN, */IDC_EDIT_NBITS_MIN, IDC_EDIT_OFFSET_MIN, IDC_EDIT_NORMALIZ_MIN, IDC_BUTTON_NORMALIZ_MIN },
    };
  HWND hwnd = GetDlgItem(*this, ids);
  int sel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  bool enableAll = PRPH_4_CONST_CB_SEL != sel;
  for(uint i = 0; i < SIZE_A(idcs); ++i) {
    if(idcs[i][0] == ids) {
      for(uint j = 1; j < SIZE_A(idcs[0]); ++j)
        ENABLE(idcs[i][j], enableAll);
      break;
      }
    }
}
//-----------------------------------------------------------
void svmDialogBarGraph::checkEnable(bool isbar)
{
  uint idc[][4] = {
    { IDC_CHECK_USE_BMP_BKG, IDC_CHECK_USE_BMP_BKG_ROTATE, IDC_CHECK_USE_BMP_BKG_MIRROR, IDC_CHECK_USE_BMP_BKG_TILED },
    { IDC_CHECK_USE_BMP_BAR, IDC_CHECK_USE_BMP_BAR_ROTATE, IDC_CHECK_USE_BMP_BAR_MIRROR, IDC_CHECK_USE_BMP_BAR_TILED }
    };
  bool enable = IS_CHECKED(idc[isbar][0]);
  for(uint i = 1; i < SIZE_A(idc[0]); ++i)
    ENABLE(idc[isbar][i], enable);

  LPCTSTR tit[][2] = {
    { _T("Sfondo"), _T("Bitmap") },
    { _T("Valore-Colore"), _T("Bitmap") }
    };
  uint idcB[] = { IDC_STATICTEXT_BACKGROUND, IDC_BUTTON_TEXT_AND_COLOR };
  SET_TEXT(idcB[isbar], tit[isbar][enable]);
}
//-----------------------------------------------------------
void svmDialogBarGraph::chooseBmp(uint idc)
{
  TCHAR buff[_MAX_PATH];
  GET_TEXT(idc, buff);
  LPTSTR pBuff = buff;
  if(openFileImageWithCopy(*this, pBuff))
    SetDlgItemText(*this, idc, buff);
}
//-----------------------------------------------------------
void svmDialogBarGraph::CmOk()
{
  PropertyBarGraph* pb = dynamic_cast<PropertyBarGraph*>(tmpProp);

  pb->type1 = 0;
  if(IS_CHECKED(IDC_CHECKBOX_TRANSP_PANEL))
    pb->type1 |= 1;

  if((Prop->type1 & 1) ^ (pb->type1 & 1)) {
    int w;
    int h;
    GET_INT(IDC_EDIT_W, w);
    GET_INT(IDC_EDIT_H, h);

    SET_INT(IDC_EDIT_W, h);
    SET_INT(IDC_EDIT_H, w);
    }

  if(IS_CHECKED(IDC_CHECKBOX_FILL_PANEL))
    pb->type1 |= 2;

  if(!IS_CHECKED(IDC_RADIOBUTTON_NO_ERASE_BKG)) {
    SET_CHECK(IDC_CHECKBOX_FILL_PANEL);
    }
  else {
    SET_CHECK_SET(IDC_CHECKBOX_FILL_PANEL, false);
    SET_CHECK(IDC_RADIOBUTTON_STYLE_NONE);
    }
  SET_CHECK_SET(IDC_CHECKBOX_TRANSP_PANEL, false);

  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_2);
  pb->vI[1].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);


  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL_2);
  if(PRPH_4_CONST_CB_SEL == pb->vI[1].perif) {
      pb->vI[1].perif = PRPH_4_CONST;
      TCHAR t[128];
      GET_TEXT(IDC_EDIT_ADDR_2, t);
      zeroTrim(t);
      DWORD v;
      pb->vI[1].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0); //4;//prfData::tDWData;
      bool isReal = getConstValue(t, v, pb->vI[1].typeVal);
      pb->vI[1].addr = v;
      pb->vI[1].normaliz = 0;
      pb->vI[1].nBit = 0;
      pb->vI[1].offs = 0;
      }
    else {
      pb->vI[1].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
      GET_INT(IDC_EDIT_ADDR_2, pb->vI[1].addr);
      GET_INT(IDC_EDIT_NBITS_2, pb->vI[1].nBit);
      GET_INT(IDC_EDIT_OFFSET_2, pb->vI[1].offs);
      GET_INT(IDC_EDIT_NORMALIZ_2, pb->vI[1].normaliz);
      }



  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_MIN);
  pb->vI[2].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);


  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL_MIN);
  if(PRPH_4_CONST_CB_SEL == pb->vI[2].perif) {
      pb->vI[2].perif = PRPH_4_CONST;
      TCHAR t[128];
      GET_TEXT(IDC_EDIT_ADDR_MIN, t);
      zeroTrim(t);
      DWORD v;
      pb->vI[2].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0); //4;//prfData::tDWData;
      bool isReal = getConstValue(t, v, pb->vI[2].typeVal);
      pb->vI[2].addr = v;
      pb->vI[2].normaliz = 0;
      pb->vI[2].nBit = 0;
      pb->vI[2].offs = 0;
      }
    else {
      pb->vI[2].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
      GET_INT(IDC_EDIT_ADDR_MIN, pb->vI[2].addr);
      GET_INT(IDC_EDIT_NBITS_MIN, pb->vI[2].nBit);
      GET_INT(IDC_EDIT_OFFSET_MIN, pb->vI[2].offs);
      GET_INT(IDC_EDIT_NORMALIZ_MIN, pb->vI[2].normaliz);
      }

  baseClass::CmOk();
  PropertyBarGraph* pbT = pb;
  pb = dynamic_cast<PropertyBarGraph*>(Prop);

  pb->centerZero = 0;
  if(IS_CHECKED(IDC_CHECK_CENTER_ZERO))
    pb->centerZero = 1;

  if(IS_CHECKED(IDC_RADIO_ZERO_TO_EDGE))
    pb->centerZero |= 2;

  GET_INT(IDC_EDIT_ZERO_MIN, pb->valueTest[0]);
  GET_INT(IDC_EDIT_ZERO_MAX, pb->valueTest[1]);

  setOfValueColor& c = pb->getValueColors();
  setOfValueColor& cT = pbT->getValueColors();
  int nElem = cT.getElem();
  c.reset();
  for(int i = 0; i < nElem; ++i)
    c[i] = valueAndColor(cT[i].Color, cT[i].value);
  pb->negValue = IS_CHECKED(IDC_CHECKBOX_NEG_VALUE);
  pb->copyVarsFromBase();

  pb->iBmp[0].img[0] = 0;
  if(IS_CHECKED(IDC_CHECK_USE_BMP_BKG)) {
    GET_TEXT(IDC_STATICTEXT_BKG_BMP, pb->iBmp[0].img);
    pb->iBmp[0].rotate = IS_CHECKED(IDC_CHECK_USE_BMP_BKG_ROTATE);
    pb->iBmp[0].mirror = IS_CHECKED(IDC_CHECK_USE_BMP_BKG_MIRROR);
    pb->iBmp[0].tiled = IS_CHECKED(IDC_CHECK_USE_BMP_BKG_TILED);
    }

  pb->iBmp[1].img[0] = 0;
  if(IS_CHECKED(IDC_CHECK_USE_BMP_BAR)) {
    GET_TEXT(IDC_STATICTEXT_BAR_BMP, pb->iBmp[1].img);
    pb->iBmp[1].rotate = IS_CHECKED(IDC_CHECK_USE_BMP_BAR_ROTATE);
    pb->iBmp[1].mirror = IS_CHECKED(IDC_CHECK_USE_BMP_BAR_MIRROR);
    pb->iBmp[1].tiled = IS_CHECKED(IDC_CHECK_USE_BMP_BAR_TILED);
    }
}
//-----------------------------------------------------------
void svmDialogBarGraph::chooseValueAndColor()
{
  PropertyBarGraph* pt = dynamic_cast<PropertyBarGraph*>(tmpProp);
  if(pt) {
    COLORREF based[2] = { RGB(0, 0, 0), RGB(0xcf, 0xcf, 0xcf) };
    uint useColor = 9;
    setOfPTextColor tmp;
    setOfValueColor& c = pt->getValueColors();
    int nElem = c.getElem();
    tmp.setDim(nElem);
    int j = nElem - 1;
    for(int i = 0; i < nElem; ++i, --j)
      tmp[i] = new textAndColor(0, 0, c[j].Color, c[j].value);
    if(IDOK == svmChooseTextAndColor(0, useColor, based, tmp, this).modal()) {
      c.reset();
      nElem = tmp.getElem();
      int j = nElem - 1;
      for(int i = 0; i < nElem; ++i, --j)
        c[i] = valueAndColor(tmp[j]->bgColor, tmp[j]->value);
      }
    flushPV(tmp);
    }
}
//-----------------------------------------------------------
