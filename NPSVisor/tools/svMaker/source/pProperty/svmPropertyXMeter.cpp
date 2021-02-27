//--------------- svmPropertyXMeter.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "svmPropertyXMeter.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "common.h"
//-----------------------------------------------------------
PropertyXMeter::PropertyXMeter() :  nameBmp(0), offsX(10), offsY(10), granularity(0), active(false), updateAlways(false),
    center(false), moveVert(true)
{
  type1 = 0;
  type2 = 90;
}
//-----------------------------------------------------------
PropertyXMeter::~PropertyXMeter()
{
  delete []nameBmp;
}
//-----------------------------------------------------------
#define INIT_ANGLE  type1
#define WIDTH_ANGLE type2
#define CLOCKWISE   style
//-----------------------------------------------------------
#define CLONE_XMETER(a) a = po->a
void PropertyXMeter::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyXMeter* po = dynamic_cast<const PropertyXMeter*>(&other);
  if(po && po != this) {
    delete []nameBmp;
    nameBmp = str_newdup(po->nameBmp);

    for(uint i = 0; i < SIZE_A(DataPrf); ++i)
      DataPrf[i] = po->DataPrf[i];
    CLONE_XMETER(granularity);
    CLONE_XMETER(offsX);
    CLONE_XMETER(offsY);
    CLONE_XMETER(active);
    CLONE_XMETER(updateAlways);
    CLONE_XMETER(center);
    CLONE_XMETER(moveVert);
  }
}
//-----------------------------------------------------------
void PropertyXMeter::cloneMinusProperty(const Property& other)
{
#if 1
  clone(other);
#else
  baseClass::clone(other);
  const PropertyXMeter* po = dynamic_cast<const PropertyXMeter*>(&other);
  if(po && po != this) {
    }
#endif
}
//-----------------------------------------------------------
bool svmDialogXMeter::create()
{
  if(!baseClass::create())
    return false;

  PropertyXMeter* pt = dynamic_cast<PropertyXMeter*>(Prop);
  if(pt) {
    fillPrf2();
    fillTypeVal2();
    setConstValue(GetDlgItem(*this, IDC_EDIT_ADDR2), pt->DataPrf[0].addr, pt->DataPrf[0].perif, pt->DataPrf[0].typeVal);
    SET_INT(IDC_EDIT_NORMALIZ2, pt->DataPrf[0].normaliz);

    setConstValue(GetDlgItem(*this, IDC_EDIT_ADDR3), pt->DataPrf[1].addr, pt->DataPrf[1].perif, pt->DataPrf[1].typeVal);
    SET_INT(IDC_EDIT_NORMALIZ3, pt->DataPrf[1].normaliz);

    SET_INT(IDC_EDIT_OFFSX, pt->offsX);
    SET_INT(IDC_EDIT_OFFSY, pt->offsY);
    SET_INT(IDC_EDIT_GRAN, pt->granularity);
    SET_TEXT(IDC_STATIC_IMG_XMETER, pt->getName());

    }
  SET_INT(IDC_EDIT_START_ANGLE, Prop->INIT_ANGLE);
  SET_INT(IDC_EDIT_AMP, Prop->WIDTH_ANGLE);

  if(Prop->CLOCKWISE & 1)
    SET_CHECK(IDC_RADIOBUTTON_SIMPLE_BORDER);
  if(pt->active)
    SET_CHECK(IDC_CHECK_ACTIVE);
  if(pt->updateAlways)
    SET_CHECK(IDC_CHECK_SEND_IMMEDDIATELY);
  if(pt->center)
    SET_CHECK(IDC_CHECK_CENTER_IMAGE);
  if(pt->moveVert)
    SET_CHECK(IDC_RADIO_MOVE_VERT);
  else
    SET_CHECK(IDC_RADIO_MOVE_HORZ);
  checkActive();
  checkCenter();
  return true;
}
//-----------------------------------------------------------
void svmDialogXMeter::fillPrf2()
{
  PropertyXMeter* pt = dynamic_cast<PropertyXMeter*>(Prop);
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
void svmDialogXMeter::fillTypeVal2()
{
  PropertyXMeter* pt = dynamic_cast<PropertyXMeter*>(Prop);
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL2);
  fillCBTypeVal(hwnd, pt->DataPrf[0].typeVal);
  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL3);
  fillCBTypeVal(hwnd, pt->DataPrf[1].typeVal);
}
//-----------------------------------------------------------
LRESULT svmDialogXMeter::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_BUTTON_NORMALIZ2:
          chooseNormaliz(IDC_EDIT_NORMALIZ2);
          break;
        case IDC_BUTTON_NORMALIZ3:
          chooseNormaliz(IDC_EDIT_NORMALIZ3);
          break;

        case IDC_BUTTON_CHOOSE_BMP:
          do {
            TCHAR buff[_MAX_PATH];
            GET_TEXT(IDC_STATIC_IMG_XMETER, buff);
            LPTSTR pBuff = buff;
            if(openFileImageWithCopy(*this, pBuff))
                SetDlgItemText(*this, IDC_STATIC_IMG_XMETER, buff);
            } while(false);
          break;
        case IDC_COMBOBOX_PERIFS2:
        case IDC_COMBOBOX_PERIFS3:
          switch(HIWORD(wParam)) {
            case CBN_SELCHANGE:
              check_const_prph(LOWORD(wParam));
              break;
            }
          break;
        case IDC_CHECK_CENTER_IMAGE:
          checkCenter();
          break;
        case IDC_CHECK_ACTIVE:
          checkActive();
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogXMeter::checkActive()
{
  bool enable = IS_CHECKED(IDC_CHECK_ACTIVE);
  ENABLE(IDC_CHECK_SEND_IMMEDDIATELY, enable);
  ENABLE(IDC_RADIO_MOVE_VERT, enable);
  ENABLE(IDC_RADIO_MOVE_HORZ, enable);
}
//-----------------------------------------------------------
void svmDialogXMeter::checkCenter()
{
  bool enabled = IS_CHECKED(IDC_CHECK_CENTER_IMAGE);
  ENABLE(IDC_EDIT_OFFSX, !enabled);
  ENABLE(IDC_EDIT_OFFSY, !enabled);
}
//-----------------------------------------------------------
void svmDialogXMeter::check_const_prph(uint ids)
{
  uint idcs[][4] = {
    { IDC_COMBOBOX_PERIFS2, /*IDC_COMBOBOX_TYPEVAL2, */IDC_EDIT_NORMALIZ2, IDC_BUTTON_NORMALIZ2 },
    { IDC_COMBOBOX_PERIFS3, /*IDC_COMBOBOX_TYPEVAL3, */IDC_EDIT_NORMALIZ3, IDC_BUTTON_NORMALIZ3 },
    };
  uint idlabel[] = { IDC_STATIC_XM_MIN, IDC_STATIC_XM_MAX };
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
void svmDialogXMeter::CmOk()
{
  int clockWise = IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_BORDER) ? 1 : 0;
  PropertyXMeter* pt = dynamic_cast<PropertyXMeter*>(tmpProp);
  if(pt) {

    HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS2);
    pt->DataPrf[0].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

    hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL2);
    if(PRPH_4_CONST_CB_SEL == pt->DataPrf[0].perif) {
      pt->DataPrf[0].perif = PRPH_4_CONST;
      TCHAR t[128];
      GET_TEXT(IDC_EDIT_ADDR2, t);
      zeroTrim(t);
      DWORD v;
      pt->DataPrf[0].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0); // 4;//prfData::tDWData;
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

    GET_INT(IDC_EDIT_OFFSX, pt->offsX);
    GET_INT(IDC_EDIT_OFFSY, pt->offsY);
    GET_INT(IDC_EDIT_GRAN, pt->granularity);
    TCHAR buff[_MAX_PATH];
    GET_TEXT(IDC_STATIC_IMG_XMETER, buff);
    pt->setName(buff);

    GET_INT(IDC_EDIT_START_ANGLE, pt->INIT_ANGLE);
    while(pt->INIT_ANGLE >= 360)
       pt->INIT_ANGLE -= 360;
    while(pt->INIT_ANGLE < 0)
       pt->INIT_ANGLE += 360;

    GET_INT(IDC_EDIT_AMP, pt->WIDTH_ANGLE);
    while(pt->WIDTH_ANGLE > 360)
       pt->WIDTH_ANGLE -= 360;
    while(pt->WIDTH_ANGLE <= 0)
       pt->WIDTH_ANGLE += 360;

    pt->active = IS_CHECKED(IDC_CHECK_ACTIVE);
    pt->updateAlways = IS_CHECKED(IDC_CHECK_SEND_IMMEDDIATELY);
    pt->center = IS_CHECKED(IDC_CHECK_CENTER_IMAGE);
    pt->moveVert = IS_CHECKED(IDC_RADIO_MOVE_VERT);
    }
  baseClass::CmOk();
  pt = dynamic_cast<PropertyXMeter*>(Prop);
  pt->CLOCKWISE = clockWise;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
