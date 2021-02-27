//--------------- svmSendVarByPage.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmSendVarByPage.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "common.h"
#include "P_util.h"
//-----------------------------------------------------------
#define MIN_DIM 5
//-----------------------------------------------------------
#define DIM_PRPH    MIN_DIM
#define DIM_ADDR    5
#define DIM_TYPE    MIN_DIM
#define DIM_NBIT    MIN_DIM
#define DIM_OFFSET  MIN_DIM
#define DIM_NORM    5
#define DIM_VALUE   10

#define DIM_BUFF_TEXT (DIM_PRPH + DIM_ADDR + DIM_TYPE + DIM_NBIT + DIM_OFFSET + DIM_NORM + DIM_VALUE + 10)
//-----------------------------------------------------------
svmSendVarByPage::svmSendVarByPage(setOfString& set, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), Set(set)
{
  LB[0] = new PListBox(this, IDC_LIST_OPEN);
  LB[1] = new PListBox(this, IDC_LIST_CLOSE);
  int tabs[] = { -DIM_PRPH, -DIM_ADDR, -DIM_TYPE, -DIM_NBIT, -DIM_OFFSET, -DIM_NORM, -DIM_VALUE };
  LB[0]->SetTabStop(SIZE_A(tabs), tabs, 0);
  LB[1]->SetTabStop(SIZE_A(tabs), tabs, 0);
}
//-----------------------------------------------------------
svmSendVarByPage::~svmSendVarByPage()
{
  destroy();
}
//-----------------------------------------------------------
bool svmSendVarByPage::create()
{
  if(!baseClass::create())
    return false;
  fill();
  return true;
}
//-----------------------------------------------------------
LRESULT svmSendVarByPage::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_OPEN:
          add(esOpen);
          break;
        case IDC_BUTTON_ADD_CLOSE:
          add(esClose);
          break;

        case IDC_BUTTON_REM_OPEN:
          rem(esOpen);
          break;
        case IDC_BUTTON_REM_CLOSE:
          rem(esClose);
          break;


        case IDC_BUTTON_UP_OPEN:
          move(esOpen, true);
          break;
        case IDC_BUTTON_UP_CLOSE:
          move(esClose, true);
          break;
        case IDC_BUTTON_DN_OPEN:
          move(esOpen, false);
          break;
        case IDC_BUTTON_DN_CLOSE:
          move(esClose, false);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
#define MAX_DIM_ROW 4096
//-----------------------------------------------------------
void svmSendVarByPage::appendVars(LPTSTR target, LPCTSTR source)
{
  PVect<wrapItem> t;
  unfillRow(t, source);
  if(*target)
    _tcscat_s(target, MAX_DIM_ROW, _T(","));
  uint nElem = t.getElem();
  for(uint i = 0; i < nElem - 1; ++i) {
    _tcscat_s(target, MAX_DIM_ROW, t[i].V);
    _tcscat_s(target, MAX_DIM_ROW, _T(","));
    }
  _tcscat_s(target, MAX_DIM_ROW, t[nElem - 1].V);
}
//-----------------------------------------------------------
void svmSendVarByPage::replaceVars(eLb which)
{
  uint id = esOpen == which ? ID_SEND_VALUE_ON_OPEN : ID_SEND_VALUE_ON_CLOSE;
  Set.replaceString(id, 0);
  int count = SendMessage(*LB[which], LB_GETCOUNT, 0 ,0);
  if(count > 0) {
    TCHAR buff[MAX_DIM_ROW] = { 0 };
    TCHAR tmp[DIM_BUFF_TEXT];
    for(int i = 0; i < count; ++i) {
      SendMessage(*LB[which], LB_GETTEXT, i, (LPARAM)tmp);
      appendVars(buff, tmp);
      }
    Set.replaceString(id, str_newdup(buff), true);
    }
}
//-----------------------------------------------------------
void svmSendVarByPage::CmOk()
{
  replaceVars(esOpen);
  replaceVars(esClose);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
template <class S, class T>
void copyStrNoZ(T* target, const S* source, uint size)
{
  for(uint i = 0; i < size && source[i]; ++i)
    target[i] = (T)source[i];
}
//-----------------------------------------------------------
bool svmSendVarByPage::fillRowByEdit(LPTSTR target)
{
  fillStr(target, _T(' '), DIM_BUFF_TEXT);
  LPTSTR p = target;
  TCHAR buff[MAX_DIM_TXT_WRAP];
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PERIFS), CB_GETCURSEL, 0, 0);
  if(sel <= 0)
    return false;
  wsprintf(buff, _T("%d"), sel);
  copyStrNoZ(p, buff, DIM_PRPH);
  p += DIM_PRPH;
  *p++ = _T('\t');
  GET_TEXT(IDC_EDIT_ADDR, buff);
  copyStrNoZ(p, buff, DIM_ADDR);
  p += DIM_ADDR;
  *p++ = _T('\t');

  sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL), CB_GETCURSEL, 0, 0);
  if(sel <= 0)
    return false;
  wsprintf(buff, _T("%d"), sel);
  copyStrNoZ(p, buff, DIM_TYPE);
  p += DIM_TYPE;
  *p++ = _T('\t');

  GET_TEXT(IDC_EDIT_NBITS, buff);
  copyStrNoZ(p, buff, DIM_NBIT);
  p += DIM_NBIT;
  *p++ = _T('\t');

  GET_TEXT(IDC_EDIT_OFFSET, buff);
  copyStrNoZ(p, buff, DIM_OFFSET);
  p += DIM_OFFSET;
  *p++ = _T('\t');

  GET_TEXT(IDC_EDIT_NORMALIZ, buff);
  copyStrNoZ(p, buff, DIM_NORM);
  p += DIM_NORM;
  *p++ = _T('\t');

  GET_TEXT(IDC_EDIT_VALUE, buff);
  copyStrNoZ(p, buff, DIM_VALUE);
  p += DIM_VALUE;
  *p = 0;
  return true;
}
//-----------------------------------------------------------
LPCTSTR svmSendVarByPage::fillRowByStr(LPTSTR target, LPCTSTR source)
{
  fillStr(target, _T(' '), DIM_BUFF_TEXT);
  LPTSTR p = target;
  uint dim[] = { DIM_PRPH, DIM_ADDR, DIM_TYPE, DIM_NBIT, DIM_OFFSET, DIM_NORM, DIM_VALUE };
  TCHAR buff[MAX_DIM_TXT_WRAP];
  for(uint i = 0; i < SIZE_A(dim); ++i) {
    extractParam(buff, dim[i] + 1, source, 0);
    source = findNextParam(source);
    copyStrNoZ(p, buff, dim[i]);
    p += dim[i];
    *p++ = _T('\t');
    }
  --p;
  *p = 0;
  return source;
}
//-----------------------------------------------------------
void svmSendVarByPage::unfillRow(PVect<wrapItem>& target, LPCTSTR p)
{
  pvvChar t;
  uint nElem = splitParam(t, p, _T('\t'));
//  if(nElem)
//    --nElem;
  target.setDim(nElem);
  for(uint i = 0; i < nElem; ++i) {
    _tcscpy_s(target[i].V, &t[i]);
    trim(target[i].V);
    if(!target[i].V[0]) {
      target[i].V[0] = _T('0');
      target[i].V[1] = 0;
      }
    }
}
//-----------------------------------------------------------
void svmSendVarByPage::fillLb(LPCTSTR p, HWND hwlb)
{
  TCHAR buff[DIM_BUFF_TEXT];
  while(p && *p) {
    p = fillRowByStr(buff, p);
    SendMessage(hwlb, LB_ADDSTRING, 0, (LPARAM)buff);
    }
}
//-----------------------------------------------------------
void svmSendVarByPage::fill()
{
  LPCTSTR p = Set.getString(ID_SEND_VALUE_ON_OPEN);
  if(p)
    fillLb(p, *LB[esOpen]);
  p = Set.getString(ID_SEND_VALUE_ON_CLOSE);
  if(p)
    fillLb(p, *LB[esClose]);
  LB[0]->setIntegralHeight();
  LB[1]->setIntegralHeight();
  fillCBPerif(GetDlgItem(*this, IDC_COMBOBOX_PERIFS), 0);
  fillCBTypeVal(GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL), 0);
}
//-----------------------------------------------------------
void svmSendVarByPage::add(eLb which)
{
  TCHAR buff[DIM_BUFF_TEXT];
  if(fillRowByEdit(buff)) {
    int sel = SendMessage(*LB[which], LB_GETCURSEL, 0, 0);
    SendMessage(*LB[which], LB_INSERTSTRING, sel, (LPARAM)buff);
    }
}
//-----------------------------------------------------------
void svmSendVarByPage::rem(eLb which)
{
  int sel = SendMessage(*LB[which], LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[DIM_BUFF_TEXT];
  SendMessage(*LB[which], LB_GETTEXT, sel, (LPARAM)buff);
  PVect<wrapItem> target;
  unfillRow(target, buff);

  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PERIFS), CB_SETCURSEL, _ttoi(target[0].V), 0);
  SET_TEXT(IDC_EDIT_ADDR, target[1].V);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL), CB_SETCURSEL, _ttoi(target[2].V), 0);
  SET_TEXT(IDC_EDIT_NBITS, target[3].V);
  SET_TEXT(IDC_EDIT_OFFSET, target[4].V);
  SET_TEXT(IDC_EDIT_NORMALIZ, target[5].V);
  SET_TEXT(IDC_EDIT_VALUE, target[6].V);

  SendMessage(*LB[which], LB_DELETESTRING, sel, 0);

  int count = SendMessage(*LB[which], LB_GETCOUNT, 0, 0);
  if(sel >= count)
    sel = count - 1;
  SendMessage(*LB[which], LB_SETCURSEL, sel, 0);
  SendMessage(*LB[which], LB_SETCARETINDEX, sel, 0);
}
//-----------------------------------------------------------
void svmSendVarByPage::move(eLb which, bool up)
{
  int sel = SendMessage(*LB[which], LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  SendMessage(*LB[which], LB_SETCURSEL, sel, 0);
  if(!sel && up)
    return;
  int count = SendMessage(*LB[which], LB_GETCOUNT, 0, 0);
  if(sel == count - 1 && !up)
    return;

  TCHAR buff[DIM_BUFF_TEXT];
  SendMessage(*LB[which], LB_GETTEXT, sel, (LPARAM)buff);
  SendMessage(*LB[which], LB_DELETESTRING, sel, 0);
  sel += up ? -1 : 1;
  SendMessage(*LB[which], LB_INSERTSTRING, sel, (LPARAM)buff);
  SendMessage(*LB[which], LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
