//--------------- svmPropertyEdit.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyEdit.h"
#include "macro_utils.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
bool svmDialogEdit::create()
{
  if(!baseClass::create())
    return false;

  SET_INT(IDC_EDIT_LEN_EDIT, Prop->type1);
  CBChanged(IDC_COMBOBOX_TYPEVAL);
  UINT align = Prop->alignText;
  uint idCtrl = Property::aBottom == (align & Property::aBottom) ? IDC_RADIOBUTTON_A_BOTTOM :
         Property::aMid == (align & Property::aMid) ? IDC_RADIOBUTTON_A_VCENTER :
                                     IDC_RADIOBUTTON_A_TOP;
  SET_CHECK(idCtrl);
  checkOldStyle();
  checkBase();
  if(IsWindowEnabled(GetDlgItem(*this, IDC_RADIOBUTTON_BASE_10))) {
    switch(Prop->BaseNum) {
      case ebTen:
      default:
        SET_CHECK(IDC_RADIOBUTTON_BASE_10);
        break;
      case ebBinary:
        SET_CHECK(IDC_RADIOBUTTON_BASE_2);
        break;
      case ebHex:
        SET_CHECK(IDC_RADIOBUTTON_BASE_16);
        break;
      }
    }
  else
    SET_CHECK(IDC_RADIOBUTTON_BASE_10);
  return true;
}
//-----------------------------------------------------------
void svmDialogEdit::CBChanged(uint idCtrl)
{
  baseClass::CBChanged(idCtrl);

  int sel = SendMessage(GetDlgItem(*this, idCtrl), CB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  switch(idCtrl) {
    case IDC_COMBOBOX_TYPEVAL:
      if(14 == sel)
        SET_TEXT(IDC_STATIC_NDEC, _T("Len"));
      else
        SET_TEXT(IDC_STATIC_NDEC, _T("N Dec"));
      break;
    }
}
//-----------------------------------------------------------
void svmDialogEdit::checkOldStyle()
{
  bool enable = !IS_CHECKED(IDC_CHECKBOX_FILL_PANEL);
  ENABLE(IDC_RADIOBUTTON_STYLE_NONE, enable);
  ENABLE(IDC_RADIOBUTTON_SIMPLE_BORDER, enable);
  ENABLE(IDC_RADIOBUTTON_UP_BORDER, enable);
  ENABLE(IDC_RADIOBUTTON_DOWN_BORDER, enable);
}
//-----------------------------------------------------------
LRESULT svmDialogEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECKBOX_FILL_PANEL:
          checkOldStyle();
          break;
        case IDC_COMBOBOX_TYPEVAL:
          checkBase();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogEdit::checkBase()
{  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  int typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  bool enable = false;
  switch(typeVal) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
    case 11:
    case 12:
    case 13:
      enable = true;
      break;
    }
  uint idc[] = { IDC_RADIOBUTTON_BASE_10, IDC_RADIOBUTTON_BASE_2, IDC_RADIOBUTTON_BASE_16 };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
}
//-----------------------------------------------------------
void svmDialogEdit::CmOk()
{
  GET_INT(IDC_EDIT_LEN_EDIT, tmpProp->type1);

  UINT align = 0;
  if(IS_CHECKED(IDC_RADIOBUTTON_A_TOP))
    align = Property::aTop;
  else if(IS_CHECKED(IDC_RADIOBUTTON_A_VCENTER))
    align = Property::aMid;
  else
    align = Property::aBottom;

  if(IS_CHECKED(IDC_RADIOBUTTON_BASE_2))
    tmpProp->BaseNum = ebBinary;
  else if(IS_CHECKED(IDC_RADIOBUTTON_BASE_16))
    tmpProp->BaseNum = ebHex;
  else
    tmpProp->BaseNum = ebTen;

  baseClass::CmOk();
  Prop->alignText |= align;
}
