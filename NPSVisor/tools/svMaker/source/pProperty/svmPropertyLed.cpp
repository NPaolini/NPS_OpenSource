//--------------- svmPropertyLed.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyLed.h"
#include "macro_utils.h"
#include "common.h"
#include "svmNormal.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PropertyLed::PropertyLed() {  }
//-----------------------------------------------------------
PropertyLed::~PropertyLed()
{
  flushPAV(nameBmp);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PropertyLed::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyLed* po = dynamic_cast<const PropertyLed*>(&other);
  if(po && po != this) {
    flushPAV(nameBmp);

    int nElem = po->nameBmp.getElem();
    nameBmp.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      nameBmp[i] = str_newdup(po->nameBmp[i]);
    showZero = po->showZero;
//    nDigit = po->nDigit;
//    nDec = po->nDec;
    }
}
//-----------------------------------------------------------
void PropertyLed::cloneMinusProperty(const Property& other)
{
  clone(other);
}
//-----------------------------------------------------------
LRESULT svmDialogLed::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_BMP:
          chooseBmp();
          break;
        }

      switch(HIWORD(wParam)) {
//        case EN_CHANGE:
        case EN_KILLFOCUS:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_NBITS:
            case IDC_EDIT_NDEC:
              checkDigit();
              break;
            }
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogLed::checkDigit()
{
  int dec;
  int digit;
  GET_INT(IDC_EDIT_NBITS, digit);
  GET_INT(IDC_EDIT_NDEC, dec);
  if(dec > digit)
    SET_INT(IDC_EDIT_NBITS, dec);
}
//-----------------------------------------------------------
bool svmDialogLed::create()
{
  if(!baseClass::create())
    return false;
  PropertyLed* pt = dynamic_cast<PropertyLed*>(tmpProp);
  if(pt) {
    if(pt->showZero)
      SET_CHECK(IDC_RADIO_SHOW_ZERO);
    else
      SET_CHECK(IDC_RADIO_SHOW_DASHES);
    }
  return true;
}
//-----------------------------------------------------------
void svmDialogLed::CmOk()
{
  baseClass::CmOk();
  PropertyLed* pt = dynamic_cast<PropertyLed*>(Prop);
  if(pt)
    pt->showZero = IS_CHECKED(IDC_RADIO_SHOW_ZERO);

}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmDialogLed::chooseBmp()
{
  PropertyLed* pt = dynamic_cast<PropertyLed*>(tmpProp);
  if(pt)
    svmChooseBitmap(pt->getNames(), this).modal();
}
//-----------------------------------------------------------
