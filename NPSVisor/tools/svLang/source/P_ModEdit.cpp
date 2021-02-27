//-------- P_ModEdit.cpp ---------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "mainwin.h"
#include "P_ModEdit.h"
#include "PEdit.h"
#include "P_ModListCodes.h"
#include "P_ModListFiles.h"
#include "p_Util.h"
#include "resource.h"
#include "P_file.h"
#include "macro_utils.h"
//-------------------------------------------------------------------
static freeList SetFree;
//-------------------------------------------------------------------
freeList& getSetFree() { return SetFree; }
//-------------------------------------------------------------------
template <typename T>
void releaseVect(PVect<T>& v) {
  uint nElem = v.getElem();
  for(uint i = 0; i < nElem; ++i)
    SetFree.release(v[i]);
  v.reset();
}
//-------------------------------------------------------------------
void releaseVect(PVect<LPCTSTR>& v) { releaseVect<LPCTSTR>(v); }
void releaseVect(PVect<LPTSTR>& v) { releaseVect<LPTSTR>(v); }
//-------------------------------------------------------------------
P_ModEdit::~P_ModEdit()
{
  destroy();
  if(font)
    DeleteObject(font);
}
//-------------------------------------------------------------------
void setLang(HWND hwnd, uint idText)
{
  TCHAR t[256];
  GetWindowText(hwnd, t, SIZE_A(t));
  SetWindowText(hwnd, getStringOrDef(idText, t));
}
//-------------------------------------------------------------------
void setLang(HWND hwnd, uint idc, uint idText)
{
  TCHAR t[256];
  HWND hwCtrl = GetDlgItem(hwnd, idc);
  GetWindowText(hwCtrl, t, SIZE_A(t));
  PWin* w = PWin::getWindowPtr(hwCtrl);
  if(w)
    w->setCaption(getStringOrDef(idText, t));
  else
    SetWindowText(hwCtrl, getStringOrDef(idText, t));
}
//-------------------------------------------------------------------
bool P_ModEdit::create()
{
  for(uint i = 0; i <  MAX_LANG; ++i)
    new PEdit(this, IDC_EDIT_N_1 + i);
  for(uint i = 0; i <  MAX_LANG; ++i)
    new PEdit(this, IDC_EDIT_T_1 + i);
  if(!baseClass::create())
    return false;
  SetWindowText(GetDlgItem(*this, IDC_EDIT_N_1), _T("Italiano"));
  SET_CHECK(IDC_RADIO1);
  struct idcText {
    uint idc;
    uint idText;
    };
  idcText it[] = {
    { ID_CM_F2, 9 },
    { ID_CM_F3, 10 },
    { ID_CM_F5, 11 },
    { IDC_RADIO0, 13 },

    { ID_CM_F4, 59 }
    };
  for(uint i = 0; i < SIZE_A(it); ++i)
    setLang(*this, it[i].idc, it[i].idText);

  for(uint i = 0; i < MAX_LANG; ++i) {
    HWND hwnd = GetDlgItem(*this, IDC_EDIT_T_1 + i);
    SendMessage(hwnd, EM_SETMODIFY, 0, 0);
    }
  return true;
}
//-------------------------------------------------------------------
LRESULT P_ModEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case ID_CM_F2:
        case ID_CM_F3:
        case ID_CM_F4:
        case ID_CM_F5:
          PostMessage(*getParentWin<mainWin>(this), message, wParam, lParam);
          break;
        case IDC_RADIO0:
        case IDC_RADIO1:
        case IDC_RADIO2:
        case IDC_RADIO3:
        case IDC_RADIO4:
        case IDC_RADIO5:
        case IDC_RADIO6:
        case IDC_RADIO7:
        case IDC_RADIO8:
        case IDC_RADIO9:
        case IDC_RADIO10:
          DirtyNameLang = true;
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
void P_ModEdit::setSelLang(uint ix)
{
  if(ix <= MAX_LANG) {
    for(uint i = 0; i <= MAX_LANG; ++i)
      SET_CHECK_SET(IDC_RADIO0 + i, ix == i);
    }
}
//-------------------------------------------------------------------
uint P_ModEdit::getSelLang() const
{
  for(uint i = 0; i <= MAX_LANG; ++i)
    if(IS_CHECKED(IDC_RADIO0 + i))
      return i;
  return 0;
}
//-------------------------------------------------------------------
void P_ModEdit::toggleNameLang()
{
  bool isEnabled = toBool(IsWindowEnabled(GetDlgItem(*this, IDC_RADIO1)));
  bool dirty = false;
  EnableWindow(GetDlgItem(*this, IDC_RADIO0), !isEnabled);
  for(uint i = 0; i < MAX_LANG; ++i) {
    EnableWindow(GetDlgItem(*this, IDC_RADIO1 + i), !isEnabled);
    if(isEnabled) {
      dirty |= toBool(SendMessage(GetDlgItem(*this, IDC_EDIT_N_1 + i), EM_GETMODIFY, 0, 0));
      SendMessage(GetDlgItem(*this, IDC_EDIT_N_1 + i), EM_SETMODIFY, 0, 0);
      }
    EnableWindow(GetDlgItem(*this, IDC_EDIT_N_1 + i), !isEnabled);
    }
  SetWindowText(GetDlgItem(*this, ID_CM_F5), isEnabled ?
          getStringOrDef(11, _T("[F5] - Edita Nome lingua")) : getStringOrDef(12, _T("[F5] - Blocca Nome lingua")));
  if(!dirty)
    return;
  DirtyNameLang = true;
  PVect<LPTSTR> buff;
  for(uint i = 0; i < MAX_LANG; ++i) {
    buff[i] = SetFree.get();
    wsprintf(buff[i], _T("[%d] "), i + 1);
    LPTSTR p = buff[i] + _tcslen(buff[i]);
    SendMessage(GetDlgItem(*this, IDC_EDIT_N_1 + i), WM_GETTEXT, SIZE_SINGLE_BUFF - 6, (LPARAM)p);
    }
  P_ModListCodes* mlc = getListCode(this);
  mlc->setTitle(buff);
}
//-------------------------------------------------------------------
void P_ModEdit::setTitle(PVect<LPTSTR>& buff)
{
  for(uint i = 0; i < MAX_LANG; ++i) {
    HWND hwnd = GetDlgItem(*this, IDC_EDIT_N_1 + i);
    SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)buff[i]);
    SendMessage(hwnd, EM_SETMODIFY, 0, 0);
    SetFree.release(buff[i]);
    }
}
//-------------------------------------------------------------------
void P_ModEdit::getTitle(PVect<LPTSTR>& buff)
{
  for(uint i = 0; i < MAX_LANG; ++i) {
    buff[i] = SetFree.get();
    buff[i][0] = 0;
    SendMessage(GetDlgItem(*this, IDC_EDIT_N_1 + i), WM_GETTEXT, SIZE_SINGLE_BUFF, (LPARAM)buff[i]);
    }
}
//-------------------------------------------------------------------
void P_ModEdit::setText(PVect<LPTSTR>& buff)
{
  for(uint i = 0; i < MAX_LANG; ++i) {
    HWND hwnd = GetDlgItem(*this, IDC_EDIT_T_1 + i);
    SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)buff[i]);
    SendMessage(hwnd, EM_SETMODIFY, 0, 0);
    SetFree.release(buff[i]);
    }
}
//-------------------------------------------------------------------
void P_ModEdit::changedSel()
{
  bool dirty = false;
  for(uint i = 0; i < MAX_LANG; ++i)
    if(SendMessage(GetDlgItem(*this, IDC_EDIT_T_1 + i), EM_GETMODIFY, 0, 0)) {
      dirty = true;
      break;
      }
  if(!dirty)
    return;
  PVect<LPTSTR> buff;
  for(uint i = 0; i < MAX_LANG; ++i) {
    buff[i] = SetFree.get();
    SendMessage(GetDlgItem(*this, IDC_EDIT_T_1 + i), WM_GETTEXT, SIZE_SINGLE_BUFF, (LPARAM)buff[i]);
    }
  P_ModListCodes* mlc = getListCode(this);
  mlc->setText(buff);
}
//-------------------------------------------------------------------
void P_ModEdit::fillField(const fillInfo& fInfo)
{
  LPTSTR buffS = SetFree.get();
  SendMessage(GetDlgItem(*this, IDC_EDIT_T_1 + fInfo.source), WM_GETTEXT, SIZE_SINGLE_BUFF, (LPARAM)buffS);
  bool needRefresh = false;
  if(*buffS) {
    LPTSTR buffT = SetFree.get();
    for(uint i = 0; i < MAX_LANG; ++i) {
      if(i != fInfo.source && (fInfo.target & (1 << i))) {
        buffT[0] = 0;
        HWND hwnd = GetDlgItem(*this, IDC_EDIT_T_1 + i);
        SendMessage(hwnd, WM_GETTEXT, SIZE_SINGLE_BUFF, (LPARAM)buffT);
        if(!*buffT || (fInfo.action & 2)) {
          SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)buffS);
          SendMessage(hwnd, EM_SETMODIFY, 0, 0);
          needRefresh = true;
          }
        }
      }
    SetFree.release(buffT);
    }
  SetFree.release(buffS);
  if(needRefresh)
    changedSel();
}
