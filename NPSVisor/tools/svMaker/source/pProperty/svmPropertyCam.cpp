//--------------- svmPropertyCam.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "svmPropertyCam.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
//-----------------------------------------------------------
PropertyCam::~PropertyCam()
{
  delete []Text;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PropertyCam::setText(LPCTSTR txt)
{
  if(txt != Text) {
    delete []Text;
    Text = str_newdup(txt);
    }
}
//-----------------------------------------------------------
void PropertyCam::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyCam* po = dynamic_cast<const PropertyCam*>(&other);
  if(po && po != this) {
    setText(po->Text);
    }
}
//-----------------------------------------------------------
void PropertyCam::cloneMinusProperty(const Property& other)
{
#if 1
  clone(other);
#else
  baseClass::clone(other);
  const PropertyCam* po = dynamic_cast<const PropertyCam*>(&other);
  if(po && po != this) {
    setText(po->Text);
    }
#endif
}
//-----------------------------------------------------------
bool svmDialogCam::create()
{
  if(!baseClass::create())
    return false;

  PropertyCam* pt = dynamic_cast<PropertyCam*>(Prop);

  if(Bits) {
    uint ids[] = {
        IDC_CHECK_CAM_STOP,
        IDC_CHECK_CAM_SETUP,
        IDC_CHECK_CAM_PREVIEW,
        IDC_CHECK_CAM_CAPTURE_AUTO,
        IDC_CHECK_CAM_CAPTURE_FILE,
        IDC_CHECK_CAM_PLAY,
        IDC_CHECK_CAM_WIN_PLAY,
        IDC_CHECK_CAM_CONNECT,
        IDC_CHECK_CAM_PAUSE,
        IDC_CHECK_CAM_SET_TIME,
        IDC_CHECK_CAM_SHOW_TIME,
        };
      for(uint i = 0; i < SIZE_A(ids); ++i)
        if(*Bits & (1 << i))
          SET_CHECK(ids[i]);
    }
  else {
    ENABLE(IDC_RADIO_CAM, false);
    ENABLE(IDC_RADIO_PLAYER, false);
    PRect r;
    GetWindowRect(*this, r);

    PRect r2;
    GetWindowRect(GetDlgItem(*this, IDC_GROUPBOX_CAM_MAKE_BTN), r2);
    r.right = r2. left;
    setWindowPos(0, r, SWP_NOMOVE | SWP_NOZORDER);
    }
  if(pt) {
    if(!Prop->type1) {
      SET_CHECK(IDC_RADIO_CAM);
      SET_TEXT(IDC_EDIT_SIMPLE_TEXT, pt->getText());
      }
    else {
      SET_CHECK(IDC_RADIO_PLAYER);
      if(1 == Prop->type1)
        SET_CHECK(IDC_CHECK_FIXED_FILE);
      SET_TEXT(IDC_EDIT_FIXED_FILE, pt->getText());
      }
    }
  checkEnable();
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogCam::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIO_CAM:
        case IDC_RADIO_PLAYER:
        case IDC_CHECK_FIXED_FILE:
          checkEnable();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogCam::checkEnable()
{
  bool onCam = IS_CHECKED(IDC_RADIO_CAM);
  if(Bits) {
    uint idsCam[]= {
      IDC_EDIT_SIMPLE_TEXT,
      IDC_CHECK_CAM_STOP,
      IDC_CHECK_CAM_SETUP,
      IDC_CHECK_CAM_PREVIEW,
      IDC_CHECK_CAM_CAPTURE_AUTO,
      IDC_CHECK_CAM_CAPTURE_FILE,
      IDC_CHECK_CAM_CONNECT,
      IDC_CHECK_CAM_SET_TIME,
      IDC_CHECK_CAM_SHOW_TIME,
      };
    uint idsPlay[]= {
      IDC_CHECK_FIXED_FILE,
      IDC_EDIT_FIXED_FILE,
      };
    for(uint i = 0; i < SIZE_A(idsCam); ++i)
      ENABLE(idsCam[i], onCam);
    for(uint i = 0; i < SIZE_A(idsPlay); ++i)
      ENABLE(idsPlay[i], !onCam);
    if(!onCam && !IS_CHECKED(IDC_CHECK_FIXED_FILE))
      ENABLE(IDC_EDIT_FIXED_FILE, false);
    }
  else if(!onCam) {
//    ENABLE(IDC_EDIT_FIXED_FILE, IS_CHECKED(IDC_CHECK_FIXED_FILE));
    ENABLE(IDC_EDIT_SIMPLE_TEXT, false);
    }
}
//-----------------------------------------------------------
void svmDialogCam::CmOk()
{
  bool onCam = IS_CHECKED(IDC_RADIO_CAM);
  TCHAR buff[100];
  uint type = 0;
  if(onCam)
    GET_TEXT(IDC_EDIT_SIMPLE_TEXT, buff);
  else {
    if(IS_CHECKED(IDC_CHECK_FIXED_FILE))
      type = 1;
    else
      type = 2;
    GET_TEXT(IDC_EDIT_FIXED_FILE, buff);
    }
  PropertyCam* pt = dynamic_cast<PropertyCam*>(tmpProp);
  if(pt) {
    LPCTSTR old = pt->getText();
    if(!old || _tcscmp(old, buff))
      pt->setText(buff);
    }
  if(Bits) {
    *Bits = 0;
    uint idsCam[] = {
        IDC_CHECK_CAM_STOP,
        IDC_CHECK_CAM_SETUP,
        IDC_CHECK_CAM_PREVIEW,
        IDC_CHECK_CAM_CAPTURE_AUTO,
        IDC_CHECK_CAM_CAPTURE_FILE,
        IDC_CHECK_CAM_PLAY,
        IDC_CHECK_CAM_WIN_PLAY,
        IDC_CHECK_CAM_CONNECT,
        IDC_CHECK_CAM_PAUSE,
        IDC_CHECK_CAM_SET_TIME,
        IDC_CHECK_CAM_SHOW_TIME,
        };
    if(!onCam) {
      uint idno[] = { 0, 1, 2, 3, 4, 7, 9, 10 };
      for(uint i = 0; i < SIZE_A(idno); ++i)
        idsCam[idno[i]] = 0;
      }
    for(uint i = 0; i < SIZE_A(idsCam); ++i)
      if(idsCam[i] && IS_CHECKED(idsCam[i]))
        *Bits |= 1 << i;
    }
  baseClass::CmOk();
  Prop->type1 = type;
}
//-----------------------------------------------------------
