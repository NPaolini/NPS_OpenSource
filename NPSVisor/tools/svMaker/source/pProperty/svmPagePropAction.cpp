//------- svmPagePropAction.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "svmPagePropAction.h"
#include "macro_utils.h"
#include "pOpensave.h"
#include "svmDefObj.h"
#include "svmProperty.h"
#include "p_file.h"
#include "dChoosePath.h"
//----------------------------------------------------------------------------
#define BKG RGB(0xff, 0xff, 0xef)
//----------------------------------------------------------------------------
svmPagePropAction::svmPagePropAction(PWin* parent, svmPageBase& data, uint resId, LPCTSTR title, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Data(data)
{
  setCaption(title);
}
//----------------------------------------------------------------------------
svmPagePropAction::~svmPagePropAction()
{
  destroy();
}
//----------------------------------------------------------------------------
bool svmPagePropAction::create()
{
  if(!baseClass::create())
    return false;
  fillPsw();

  return true;
}
//----------------------------------------------------------------------------
static LPCTSTR pswLevels[] = {
    _T("0: No Password"),
    _T("1: Operator Level"),
    _T("2: Autorized Level"),
    _T("3: Supervisor Level"),
    _T("-: Confirm only"),
};
//----------------------------------------------------------------------------
void svmPagePropAction::fillPsw()
{
  HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_PSWLEVEL);
  for(uint j = 0; j < SIZE_A(pswLevels); ++j)
    addStringToComboBox(hCBx, pswLevels[j]);
  int level = Data.pswLevel;
  if(ONLY_CONFIRM == level)
    level = 4;
//  else
//    ++level;
  SendMessage(hCBx, CB_SETCURSEL, level, 0);
}
//----------------------------------------------------------------------------
void svmPagePropAction::CmOk()
{
  HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_PSWLEVEL);
  int result = SendMessage(hCBx, CB_GETCURSEL, 0, 0);
  if(result >= 0 && (uint)result < SIZE_A(pswLevels)) {
    if(4 == result)
      result = ONLY_CONFIRM;
//    else
//      --result;
    Data.pswLevel = result;
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void svmPagePropAction::addEditToCB(HWND hcbx, LPCTSTR row)
{
  int count = SendMessage(hcbx, CB_GETCOUNT, 0, 0);
  for(int i = 0; i < count; ++i) {
    TCHAR t[200];
    SendMessage(hcbx, CB_GETLBTEXT, i, (LPARAM)t);
    if(!_tcscmp(row, t))
      return;
    }
  SendMessage(hcbx, CB_ADDSTRING, 0, (LPARAM)row);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
svmPagePropActionGetPrintScreen::svmPagePropActionGetPrintScreen(PWin* parent, svmPageBase& data, uint resId, LPCTSTR title, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Data(data)
{
}
//----------------------------------------------------------------------------
svmPagePropActionGetPrintScreen::~svmPagePropActionGetPrintScreen()
{
  destroy();
}
//----------------------------------------------------------------------------
bool svmPagePropActionGetPrintScreen::create()
{
  int preview = LOWORD(Data.pswLevel);
  int setup = HIWORD(Data.pswLevel);

  if(!baseClass::create())
    return false;
  if(preview)
    SET_CHECK(IDC_CHECKBOX_PREVIEW);
  if(setup)
    SET_CHECK(IDC_CHECKBOX_SETUP);

  return true;
}
//----------------------------------------------------------------------------
void svmPagePropActionGetPrintScreen::CmOk()
{
  bool preview = IS_CHECKED(IDC_CHECKBOX_PREVIEW);
  bool setup = IS_CHECKED(IDC_CHECKBOX_SETUP);
  Data.pswLevel = MAKELONG(preview, setup);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define PREDEFINITE_SET BASE_TEMP_NAME _T("predef.dat")
//----------------------------------------------------------------------------
struct predefInfo {
  int id;
  TCHAR info[60];
  predefInfo() : id(0) { fillStr(info, 0, SIZE_A(info)); }
};
//----------------------------------------------------------------------------
bool svmPagePropActionOpenPage::create()
{
  if(!baseClass::create())
    return false;
  P_File pf(PREDEFINITE_SET, P_READ_ONLY);
  if(pf.P_open()) {
    HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_CHOOSE_PREDEF);
    for(;;) {
      predefInfo Info;
      if(sizeof(Info) != pf.P_read(&Info, sizeof(Info)))
        break;
      TCHAR buff[100];
      wsprintf(buff, _T("%d,%s"), Info.id, Info.info);
      addStringToComboBox(hCBx, buff);
      if(Data.id == Info.id) {
        int count = SendMessage(hCBx, CB_GETCOUNT, 0, 0);
        SendMessage(hCBx, CB_SETCURSEL, count - 1, 0);
        }
      }
    }

  SET_TEXT(IDC_EDIT_CHOOSE_NAME, Data.name);
  if(Data.id)
    SET_CHECK(IDC_RADIOBUTTON_CHOOSE_ID);
  else
    SET_CHECK(IDC_RADIOBUTTON_CHOOSE_NAME);

  checkEnable();

  return true;
}
//----------------------------------------------------------------------------
void svmPagePropActionOpenPage::CmOk()
{
  HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_CHOOSE_PREDEF);
  int count = SendMessage(hCBx, CB_GETCOUNT, 0, 0);
  if(count) {
    P_File pf(PREDEFINITE_SET, P_CREAT);
    if(pf.P_open()) {
      for(int i = 0; i < count; ++i) {
        TCHAR buff[100];
        SendMessage(hCBx, CB_GETLBTEXT, i, (LPARAM)buff);
        predefInfo Info;
        Info.id = _ttoi(buff);
        LPCTSTR p = findNextParam(buff, 1);
        if(p)
          _tcscpy_s(Info.info, p);
        if(sizeof(Info) != pf.P_write(&Info, sizeof(Info)))
          break;
        }
      }
    }
  else
    DeleteFile(PREDEFINITE_SET);
  if(IS_CHECKED(IDC_RADIOBUTTON_CHOOSE_NAME)) {
    GET_TEXT(IDC_EDIT_CHOOSE_NAME, Data.name);
    Data.id = 0;
    }
  else {
    TCHAR buff[100];
    GET_TEXT(IDC_COMBOBOX_CHOOSE_PREDEF, buff);
    Data.id = _ttoi(buff);
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void svmPagePropActionOpenPage::checkEnable()
{
  bool enable = IS_CHECKED(IDC_RADIOBUTTON_CHOOSE_NAME);

  ENABLE(IDC_EDIT_CHOOSE_NAME, enable);
  ENABLE(IDC_BUTTON_CHOOSE_NAME, enable);

  ENABLE(IDC_COMBOBOX_CHOOSE_PREDEF, !enable);
}
//----------------------------------------------------------------------------
static void chooseName(PWin* owner, int idEdit)
{
  TCHAR buff[_MAX_PATH];
  HWND hw = GetDlgItem(*owner, idEdit);

  GetWindowText(hw, buff, SIZE_A(buff));

  TCHAR oldSub[_MAX_PATH];
  dataProject& Data = getDataProject();
  _tcscpy_s(oldSub, Data.getSubPath());

  if(IDOK == getPageFile(owner, buff, true).modal())
    SetWindowText(hw, buff);
  Data.setSubPath(oldSub);
}
//----------------------------------------------------------------------------
LRESULT svmPagePropActionOpenPage::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIOBUTTON_CHOOSE_NAME:
        case IDC_RADIOBUTTON_CHOOSE_ID:
          checkEnable();
          break;

        case IDC_BUTTON_CHOOSE_NAME:
          chooseName(this, IDC_EDIT_CHOOSE_NAME);
/*
          do {
            TCHAR buff[_MAX_PATH];
            GET_TEXT(IDC_EDIT_CHOOSE_NAME, buff);
            if(chooseFolder(buff))
                SetDlgItemText(*this, IDC_EDIT_PATH, buff);
            } while(false);
*/
          break;

        }
      switch(HIWORD(wParam)) {
//        case CBN_EDITCHANGE:
        case CBN_KILLFOCUS:
          addNameToCB((HWND)lParam);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void svmPagePropActionOpenPage::addNameToCB(HWND hcbx)
{
  TCHAR buff[200];
  GetWindowText(hcbx, buff, SIZE_A(buff));
  if(!*buff)
    return;
  addEditToCB(hcbx, buff);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool svmPagePropActionOpenName::create()
{
  if(!baseClass::create())
    return false;

  SET_TEXT(IDC_EDIT_CHOOSE_NAME, Data.name);
  return true;
}
//----------------------------------------------------------------------------
void svmPagePropActionOpenName::CmOk()
{
  GET_TEXT(IDC_EDIT_CHOOSE_NAME, Data.name);
  Data.id = 0;

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
LRESULT svmPagePropActionOpenName::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_NAME:
          chooseName(this, IDC_EDIT_CHOOSE_NAME);
/*
          do {
            TCHAR buff[_MAX_PATH];
            GET_TEXT(IDC_EDIT_CHOOSE_NAME, buff);
            if(chooseFolder(buff))
                SetDlgItemText(*this, IDC_EDIT_PATH, buff);
            } while(false);
*/
          break;

        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool svmPagePropActionGetPerif::create()
{
  if(!baseClass::create())
    return false;

  HWND hcbx = GetDlgItem(*this, IDC_COMBOBOX_CHOOSE_PERIF);
  fillCBPerif(hcbx, Data.id);

  return true;
}
//----------------------------------------------------------------------------
void svmPagePropActionGetPerif::CmOk()
{
  HWND hcbx = GetDlgItem(*this, IDC_COMBOBOX_CHOOSE_PERIF);
  Data.id = SendMessage(hcbx, CB_GETCURSEL, 0, 0);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
extern void fillCBCTRL_Action(HWND hwnd, int select);
//----------------------------------------------------------------------------
bool svmPagePropActionGetCTRL_Action::create()
{
  if(!baseClass::create())
    return false;

  HWND hcbx = GetDlgItem(*this, IDC_COMBOBOX_CHOOSE_PERIF);
  fillCBCTRL_Action(hcbx, Data.id);

  return true;
}
//----------------------------------------------------------------------------
void svmPagePropActionGetCTRL_Action::CmOk()
{
  HWND hcbx = GetDlgItem(*this, IDC_COMBOBOX_CHOOSE_PERIF);
  Data.id = SendMessage(hcbx, CB_GETCURSEL, 0, 0);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void perifForBtnAction::toString(LPTSTR target, const perifForBtnAction& data)
{
  switch(data.U.s.type) {
    case 5:
    case 7:
      _stprintf_s(target, _MAX_PATH, _T("?(%d,%d,%f)"), data.U.allBit, data.addr, *(double*)&data.mask);
      break;
    default:
      _stprintf_s(target, _MAX_PATH, _T("?(%d,%d,%I64d)"), data.U.allBit, data.addr, data.mask);
    }
}
//----------------------------------------------------------------------------
void perifForBtnAction::fromString(perifForBtnAction& data, LPCTSTR source)
{
  _stscanf_s(source, _T("?(%d,%d"), &data.U.allBit, &data.addr);
  source = findNextParam(source, 2);
  data.mask = 0;
  if(!source)
    return;
  switch(data.U.s.type) {
    case 5:
    case 7:
      *(double*)&data.mask = _tstof(source);
      break;
    default:
      data.mask = _tstoi64(source);
      break;
    }
}
//----------------------------------------------------------------------------
bool svmPagePropActionGetIdBtn::create()
{
  if(!baseClass::create())
    return false;

  perifForBtnAction data;
  perifForBtnAction::fromString(data, Data.name);

  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS);
  fillCBPerif(hwnd, data.U.s.prf);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  fillCBTypeVal(hwnd, data.U.s.type);

  SET_INT(IDC_EDIT_ADDR, data.addr);
  SET_INT(IDC_EDIT_NBITS, data.U.s.nbit);
  SET_INT(IDC_EDIT_OFFSET, data.U.s.offs);
  TCHAR buff[256];
  switch(data.U.s.type) {
    case 5:
      _stprintf_s(buff, _T("%f"), (float)*(double*)&data.mask);
      break;
    case 7:
      _stprintf_s(buff, _T("%f"), *(double*)&data.mask);
      break;
    default:
      _stprintf_s(buff, _T("%I64d"), data.mask);
      break;
    }
  SET_TEXT(IDC_EDIT_NORMALIZ, buff);
  if(data.U.s.neg)
    SET_CHECK(IDC_CHECKBOX_NEGATIVE);
  chgNameMask();
  return true;
}
//----------------------------------------------------------------------------
void svmPagePropActionGetIdBtn::CmOk()
{
  perifForBtnAction data;

  GET_INT(IDC_EDIT_ADDR, data.addr);
  GET_INT(IDC_EDIT_NBITS, data.U.s.nbit);
  GET_INT(IDC_EDIT_OFFSET, data.U.s.offs);
  data.U.s.neg = IS_CHECKED(IDC_CHECKBOX_NEGATIVE);

  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS);
  data.U.s.prf = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  data.U.s.type = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  TCHAR buff[256];
  GET_TEXT(IDC_EDIT_NORMALIZ, buff);
  switch(data.U.s.type) {
    case 5:
    case 7:
      *(double*)&data.mask = _tstof(buff);
      break;
    default:
      data.mask = _tstoi64(buff);
      break;
    }

  perifForBtnAction::toString(Data.name, data);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void svmPagePropActionGetIdBtn::chgNameMask()
{
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL), CB_GETCURSEL, 0, 0);
  if(sel == 1)
    SET_TEXT(IDC_STATIC_MASK, _T("mask"));
  else
    SET_TEXT(IDC_STATIC_MASK, _T("value"));
}
//----------------------------------------------------------------------------
LRESULT svmPagePropActionGetIdBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_COMBOBOX_TYPEVAL:
          chgNameMask();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool svmPagePropActionGetExport::create()
{
  if(!baseClass::create())
    return false;
  SET_CHECK(Data.id ? IDC_RB_ACTION_EXPORT_HIST : IDC_RB_ACTION_EXPORT_CURR);
  return true;
}
//----------------------------------------------------------------------------
void svmPagePropActionGetExport::CmOk()
{
  Data.id =  IS_CHECKED(IDC_RB_ACTION_EXPORT_HIST);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool svmPagePropActionSpin::create()
{
  if(!baseClass::create())
    return false;

  HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_PSWLEVEL);
  SendMessage(hCBx, CB_DELETESTRING, SIZE_A(pswLevels) - 1, 0);

  int idSpin = 0;
  int up = 0;
  _stscanf_s(Data.name, _T("%d,%d"), &up, &idSpin);

  if(up)
    SET_CHECK(IDC_RADIO_INCR);
  else
    SET_CHECK(IDC_RADIO_DECR);
  SET_INT(IDC_EDIT_ID_SPIN, idSpin);
  return true;
}
//----------------------------------------------------------------------------
void svmPagePropActionSpin::CmOk()
{
  int idSpin = 0;
  GET_INT(IDC_EDIT_ID_SPIN, idSpin);
  int up = IS_CHECKED(IDC_RADIO_INCR);
  wsprintf(Data.name, _T("%d,%d"), up, idSpin);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
