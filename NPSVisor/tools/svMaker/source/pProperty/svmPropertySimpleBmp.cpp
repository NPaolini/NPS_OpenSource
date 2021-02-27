//--------------- svmPropertySimpleBmp.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertySimpleBmp.h"
#include "macro_utils.h"
#include "common.h"
#include "svmNormal.h"
//-----------------------------------------------------------
// N.B. type1 è usato per il time del blink
//-----------------------------------------------------------
PropertySimpleBmp::PropertySimpleBmp() : filename(0)
{
  szBmp.cx = 0;
  szBmp.cy = 0;
}
//-----------------------------------------------------------
PropertySimpleBmp::~PropertySimpleBmp()
{
  delete []filename;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PropertySimpleBmp::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertySimpleBmp* po = dynamic_cast<const PropertySimpleBmp*>(&other);
  if(po && po != this) {
    delete []filename;
    if(po->filename && *po->filename)
      filename = str_newdup(po->filename);
    else
      filename = 0;
    szBmp = po->szBmp;
    }
}
//-----------------------------------------------------------
void PropertySimpleBmp::cloneMinusProperty(const Property& other)
{
  clone(other);
/*
  baseClass::clone(other);
  const PropertySimpleBmp* po = dynamic_cast<const PropertySimpleBmp*>(&other);
  if(po && po != this) {
    setText(po->Text);
    useColor = po->usingColor();
    }
*/
}
//-----------------------------------------------------------
void svmDialogSimpleBmp::allocBmp()
{
  delete Bmp;
  PropertySimpleBmp* po = dynamic_cast<PropertySimpleBmp*>(tmpProp);
  if(po) {
    if(po->filename && *po->filename) {
      TCHAR name[_MAX_PATH];
      makeNameImage(name, po->filename);
      Bmp = new PBitmap(name);
      po->szBmp = Bmp->getSize();
      wsprintf(name, _T("Dim reali Immagine = %dx%d"), po->szBmp.cx, po->szBmp.cy);
      SET_TEXT(IDC_STATICTEXT_REAL_SIZE, name);
      SET_TEXT(IDC_EDIT_NAME_BMP, po->filename);
      }
    else {
      po->szBmp.cx = 0;
      po->szBmp.cy = 0;
      SET_TEXT(IDC_STATICTEXT_REAL_SIZE, _T(""));
      SET_TEXT(IDC_EDIT_NAME_BMP, _T(""));
      }
    }
}
//-----------------------------------------------------------
bool svmDialogSimpleBmp::create()
{
  if(!baseClass::create())
    return false;
  allocBmp();
  if(Prop->type1) {
    SET_CHECK(IDC_CHECK_BLINK);
    SET_INT(IDC_EDIT_BLINK, Prop->type1);
    }
  checkBlink();
  return true;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
LRESULT svmDialogSimpleBmp::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_BMP:
          chooseBmp();
          break;
        case IDC_COMBOBOX_VIS_PERIFS:
        case IDC_CHECK_BLINK:
          checkBlink();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmDialogSimpleBmp::checkBlink()
{
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_VIS_PERIFS), CB_GETCURSEL, 0, 0);
  if(sel <= 0) {
    SET_CHECK_SET(IDC_CHECK_BLINK, false);
    ENABLE(IDC_CHECK_BLINK, false);
    }
  else
    ENABLE(IDC_CHECK_BLINK, true);
  bool enable = IS_CHECKED(IDC_CHECK_BLINK);
  ENABLE(IDC_EDIT_BLINK, enable);
}
//-----------------------------------------------------------
void svmDialogSimpleBmp::CmOk()
{
  TCHAR buff[_MAX_PATH];
  GET_TEXT(IDC_EDIT_NAME_BMP, buff);
  PropertySimpleBmp* po = dynamic_cast<PropertySimpleBmp*>(tmpProp);
  if(po) {
    if(po->filename && _tcsicmp(po->filename, buff)) {
      delete []po->filename;
      po->filename = str_newdup(buff);
      }
    }
  if(IS_CHECKED(IDC_CHECK_BLINK)) {
    int tick;
    GET_INT(IDC_EDIT_BLINK, tick);
    tmpProp->type1 = tick;
    }
  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmDialogSimpleBmp::chooseBmp()
{
  PropertySimpleBmp* pt = dynamic_cast<PropertySimpleBmp*>(tmpProp);
  if(pt) {
    TCHAR old[_MAX_PATH] = _T("\0");
    if(pt->filename)
      _tcscpy_s(old, pt->filename);
    TCHAR buff[_MAX_PATH];
    GET_TEXT(IDC_EDIT_NAME_BMP, buff);
    LPTSTR pBuff = buff;
    if(openFileImageWithCopy(*this, pBuff)) {
      SetDlgItemText(*this, IDC_EDIT_NAME_BMP, buff);
      if(_tcsicmp(old, buff)) {
        delete []pt->filename;
        pt->filename = str_newdup(buff);
        allocBmp();
        }
      else if(*old) {
        delete Bmp;
        Bmp = 0;
        }
      }
    }
}
//-----------------------------------------------------------
