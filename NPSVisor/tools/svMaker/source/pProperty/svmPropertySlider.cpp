//--------------- svmPropertySlider.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertySlider.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "svmObjSlider.h"
#include "svmNormal.h"
//#include "svmPropertyText.h"
#include "common.h"
//-----------------------------------------------------------
PropertySlider::PropertySlider() : fullImage(false), H_W(0), vert(false), mirror(false), reverse(false), readOnly(false), updateAlways(false)
{
  ZeroMemory(img, sizeof(img));
}
//-----------------------------------------------------------
PropertySlider::~PropertySlider()
{
}
//-----------------------------------------------------------
#define CLONE_V(a) a = po->a
//-----------------------------------------------------------
void PropertySlider::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertySlider* po = dynamic_cast<const PropertySlider*>(&other);
  if(po && po != this) {

    for(uint i = 0; i < SIZE_A(vI); ++i)
      CLONE_V(vI[i]);

    _tcscpy_s(img, po->img);

    CLONE_V(H_W);
    CLONE_V(fullImage);
    CLONE_V(vert);
    CLONE_V(mirror);
    CLONE_V(reverse);

    CLONE_V(readOnly);
    CLONE_V(updateAlways);
    }
}
//-----------------------------------------------------------
void PropertySlider::cloneMinusProperty(const Property& other)
{
  baseClass::clone(other);
}
//-----------------------------------------------------------
bool svmDialogSlider::create()
{
  if(!baseClass::create())
    return false;

  PropertySlider* pb = dynamic_cast<PropertySlider*>(Prop);

  SET_INT(IDC_EDIT_H_W, pb->H_W);
  SET_CHECK_SET(IDC_CHECK_FULLIMAGE, pb->fullImage);
  SET_CHECK_SET(IDC_CHECK_VERT, pb->vert);
//  SET_CHECK_SET(IDC_CHECK_MIRROR, pb->mirror);
  SET_CHECK_SET(IDC_CHECK_REVERSE, pb->reverse);
  SET_CHECK_SET(IDC_CHECK_READONLY, pb->readOnly);
  SET_CHECK_SET(IDC_CHECK_UPDATEALWAYS, pb->updateAlways);

  SET_TEXT(IDC_STATICTEXT_BMP, pb->img);


  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_MIN);
  int prph = pb->vI[0].perif;
  if(PRPH_4_CONST == prph)
    prph = PRPH_4_CONST_CB_SEL;
  fillCBPerifEx(hwnd, prph, true);
  setConstValue(GetDlgItem(*this, IDC_EDIT_ADDR_MIN), pb->vI[0].addr, pb->vI[0].perif, pb->vI[0].typeVal);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL_MIN);
  fillCBTypeVal(hwnd, pb->vI[0].typeVal);
  SET_INT(IDC_EDIT_NBITS_MIN, pb->vI[0].nBit);
  SET_INT(IDC_EDIT_OFFSET_MIN, pb->vI[0].offs);
  SET_INT(IDC_EDIT_NORMALIZ_MIN, pb->vI[0].normaliz);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_MAX);
  prph = pb->vI[1].perif;
  if(PRPH_4_CONST == prph)
    prph = PRPH_4_CONST_CB_SEL;
  fillCBPerifEx(hwnd, prph, true);
  setConstValue(GetDlgItem(*this, IDC_EDIT_ADDR_MAX), pb->vI[1].addr, pb->vI[1].perif, pb->vI[1].typeVal);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL_MAX);
  fillCBTypeVal(hwnd, pb->vI[1].typeVal);
  SET_INT(IDC_EDIT_NBITS_MAX, pb->vI[1].nBit);
  SET_INT(IDC_EDIT_OFFSET_MAX, pb->vI[1].offs);
  SET_INT(IDC_EDIT_NORMALIZ_MAX, pb->vI[1].normaliz);

  check_const_prph(IDC_COMBOBOX_PERIFS_MAX);
  check_const_prph(IDC_COMBOBOX_PERIFS_MIN);
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogSlider::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_BMP:
          chooseBmp(IDC_STATICTEXT_BMP);
          break;

        case IDC_BUTTON_NORMALIZ_MAX:
          chooseNormaliz(IDC_EDIT_NORMALIZ_MAX);
          break;
        case IDC_BUTTON_NORMALIZ_MIN:
          chooseNormaliz(IDC_EDIT_NORMALIZ_MIN);
          break;
        case IDC_COMBOBOX_PERIFS_MAX:
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
void svmDialogSlider::check_const_prph(uint ids)
{
  uint idcs[][6] = {
    { IDC_COMBOBOX_PERIFS_MAX,  /*IDC_COMBOBOX_TYPEVAL_MAX, */IDC_EDIT_NBITS_MAX, IDC_EDIT_OFFSET_MAX, IDC_EDIT_NORMALIZ_MAX, IDC_BUTTON_NORMALIZ_MAX },
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
void svmDialogSlider::chooseBmp(uint idc)
{
  TCHAR buff[_MAX_PATH];
  GET_TEXT(idc, buff);
  LPTSTR pBuff = buff;
  if(openFileImageWithCopy(*this, pBuff))
    SetDlgItemText(*this, idc, buff);
}
//-----------------------------------------------------------
void svmDialogSlider::CmOk()
{
  PropertySlider* pb = dynamic_cast<PropertySlider*>(tmpProp);
  PropertySlider* pbOld = dynamic_cast<PropertySlider*>(Prop);

  if(pbOld->vert ^ pb->vert) {
    int w;
    int h;
    GET_INT(IDC_EDIT_W, w);
    GET_INT(IDC_EDIT_H, h);

    SET_INT(IDC_EDIT_W, h);
    SET_INT(IDC_EDIT_H, w);
    }


  GET_INT(IDC_EDIT_H_W, pb->H_W);
  pb->fullImage = IS_CHECKED(IDC_CHECK_FULLIMAGE);
  pb->vert = IS_CHECKED(IDC_CHECK_VERT);
//  pb->mirror = IS_CHECKED(IDC_CHECK_MIRROR);
  pb->reverse = IS_CHECKED(IDC_CHECK_REVERSE);
  pb->readOnly = IS_CHECKED(IDC_CHECK_READONLY);
  pb->updateAlways = IS_CHECKED(IDC_CHECK_UPDATEALWAYS);
  GET_TEXT(IDC_STATICTEXT_BMP, pb->img);

  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_MAX);
  pb->vI[1].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);


  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL_MAX);
  if(PRPH_4_CONST_CB_SEL == pb->vI[1].perif) {
      pb->vI[1].perif = PRPH_4_CONST;
      TCHAR t[128];
      GET_TEXT(IDC_EDIT_ADDR_MAX, t);
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
      GET_INT(IDC_EDIT_ADDR_MAX, pb->vI[1].addr);
      GET_INT(IDC_EDIT_NBITS_MAX, pb->vI[1].nBit);
      GET_INT(IDC_EDIT_OFFSET_MAX, pb->vI[1].offs);
      GET_INT(IDC_EDIT_NORMALIZ_MAX, pb->vI[1].normaliz);
      }

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_MIN);
  pb->vI[0].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL_MIN);
  if(PRPH_4_CONST_CB_SEL == pb->vI[0].perif) {
      pb->vI[0].perif = PRPH_4_CONST;
      TCHAR t[128];
      GET_TEXT(IDC_EDIT_ADDR_MIN, t);
      zeroTrim(t);
      DWORD v;
      pb->vI[0].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0); //4;//prfData::tDWData;
      bool isReal = getConstValue(t, v, pb->vI[0].typeVal);
      pb->vI[0].addr = v;
      pb->vI[0].normaliz = 0;
      pb->vI[0].nBit = 0;
      pb->vI[0].offs = 0;
      }
    else {
      pb->vI[0].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
      GET_INT(IDC_EDIT_ADDR_MIN, pb->vI[0].addr);
      GET_INT(IDC_EDIT_NBITS_MIN, pb->vI[0].nBit);
      GET_INT(IDC_EDIT_OFFSET_MIN, pb->vI[0].offs);
      GET_INT(IDC_EDIT_NORMALIZ_MIN, pb->vI[0].normaliz);
      }

  baseClass::CmOk();
}
//-----------------------------------------------------------
