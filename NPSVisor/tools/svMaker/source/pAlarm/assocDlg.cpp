//------------ assocDlg.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "assocDlg.h"
#include "alarmEditorDlg.h"
#include "fillAlarmAssoc.h"
#include "alarm_file_Dlg.h"
//----------------------------------------------------------------------------
PD_Assoc::PD_Assoc(PWin* parent, uint idPrph, uint resId, HINSTANCE hinstance) :
    baseClass(parent, idPrph, resId, hinstance)
{
  for(int i = 0; i < MAX_GROUP; ++i)
    Rows[i] = 0;
  ClearAll();
}
//----------------------------------------------------------------------------
PD_Assoc::~PD_Assoc()
{
  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
}
//----------------------------------------------------------------------------
void PD_Assoc::Clear(int from, int to)
{
  dirty = true;
  from = min((uint)from, SIZE_A(Cfg) - 1);
  to = min((uint)to, SIZE_A(Cfg) - 1);
  for(int i = from; i <= to; ++i)
    Cfg[i].clear();
}
//----------------------------------------------------------------------------
bool PD_Assoc::create()
{
  POINT pt = { X_INIT, Y_INIT };
  Rows[0] = new PRow_Assoc(this, IDC_EDIT_FIRST_FIELD, pt, -1);

  pt.y += H_EDIT - 1;
  for(int i = 1; i < MAX_GROUP - 1; ++i) {
    Rows[i] = new PRow_Assoc(this, IDC_EDIT_FIRST_FIELD + i, pt, 0);
    pt.y += H_EDIT - 1;
    }

  Rows[MAX_GROUP - 1] = new PRow_Assoc(this, IDC_EDIT_FIRST_FIELD + MAX_GROUP - 1, pt, 1);

  if(!baseClass::create())
    return false;
  PRect r;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1), r);
  int y = r.top;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_2), r);
  int h = r.top - y;
  PVect<infoPosCtrl> ipc;

  uint idc[] = { IDC_STATIC_PRPH, IDC_STATIC_ASS_ADDR, IDC_STATIC_ASS_TYPE, IDC_STATIC_ASS_NBIT,
      IDC_STATIC_ASS_OFFS, IDC_STATIC_ASS_BTN, IDC_STATIC_DESCR, IDC_BUTTON_CHOOSE_FILE  };
  for(uint i = 0; i < SIZE_A(idc); ++i) {
    GetWindowRect(GetDlgItem(*this, idc[i]), r);
    ipc[i].x = r.left;
    ipc[i].y = y;
    ipc[i].w = r.Width();
    ipc[i].h = h - 1;
    }
  ipc[SIZE_A(idc) - 1].x += ipc[SIZE_A(idc) - 1].w / 3;
  ipc[SIZE_A(idc) - 1].w -= ipc[SIZE_A(idc) - 1].w / 3;

  Rows[0]->offset(ipc);
  Rows[0]->setSelected();
  for(uint j = 1; j < MAX_GROUP; ++j) {
    GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1 + j), r);
    for(uint i = 0; i < SIZE_A(idc); ++i)
      ipc[i].y = r.top;
    Rows[j]->offset(ipc);
    }
  loadData();
  setScrollRange();
  return true;
}
//----------------------------------------------------------------------------
void PD_Assoc::manageText(uint idcBtn)
{

}
//----------------------------------------------------------------------------
#define F_BTN (IDC_EDIT_FIRST_FIELD + OFFSET_TEXT_ASSOC)
#define L_BTN (F_BTN + MAX_GROUP)
//----------------------------------------------------------------------------
static bool isChanged(HWND hed)
{
  return toBool(SendMessage(hed, EM_GETMODIFY, 0, 0));
}
//----------------------------------------------------------------------------
LRESULT PD_Assoc::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_FILE:
          openAssocFile();
          break;
        }
      if(LOWORD(wParam) >= F_BTN && LOWORD(wParam) < L_BTN)
        if(MY_EN_CHANGE == HIWORD(wParam) || EN_KILLFOCUS ==  HIWORD(wParam) && isChanged(HWND(lParam))) {
          saveCurrData();
          loadData();
          }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_Assoc::openAssocFile()
{
  int i = 0;
  TCHAR file[_MAX_PATH] = { 0 };
  for(; i < MAX_GROUP; ++i) {
    if(Rows[i]->isSelected()) {
      Rows[i]->getFile(file);
      break;
      }
    }

  PD_alarm_file_Dlg* dlg = new PD_alarm_file_Dlg(getParent(), file);
  if(IDOK == dlg->modal() && i < MAX_GROUP) {
    TCHAR old[_MAX_PATH];
    Rows[i]->getFile(old);
    if(_tcscmp(old, file)) {
      Rows[i]->setFile(file);
      setDirty();
      }
    }
  delete dlg;
}
//----------------------------------------------------------------------------
void PD_Assoc::Copy() {}
//----------------------------------------------------------------------------
extern bool myIcmp(LPCTSTR str1, LPCTSTR str2, uint len);
extern bool foundText(LPCTSTR text, LPCTSTR toFind, uint len);
//----------------------------------------------------------------------------
static int gSearchText(HWND edit, const baseRowAssoc* cfg, int currPos)
{
  static int last = -1;
  TCHAR t[512];
  GetWindowText(edit, t, SIZE_A(t));
  trim(lTrim(t));
  if(!*t)
    return -1;
  if(currPos == last)
    ++currPos;
  last = currPos;
  uint len = 0;
  for(; t[len]; ++len)
    t[len] = _totlower(t[len]);

  for(uint i = 0; i < MAX_ADDRESSES; ++i) {
    int pos = (currPos + i) % MAX_ADDRESSES;
    if(foundText(cfg[pos].assocFile, t, len)) {
      last = pos;
      return pos;
      }
    }
  return -1;
}
//----------------------------------------------------------------------------
void PD_Assoc::searchText()
{
  int pos = gSearchText(GetDlgItem(*this, IDC_EDIT_SEARCH_TEXT), Cfg, currPos);
  if(pos >= 0)
    evVScrollBar(GetDlgItem(*this, IDC_SCROLLBAR_COUNT), SB_THUMBTRACK, pos);
}
//----------------------------------------------------------------------------
void PD_Assoc::fill()
{
  baseRowAssoc* tmpCfg = new baseRowAssoc[MAX_ADDRESSES];
  for(int i = 0; i < MAX_ADDRESSES; ++i)
    tmpCfg[i] = Cfg[i];

  if(IDOK == PD_FillAssoc(tmpCfg, this).modal())  {
    for(int i = 0; i < MAX_ADDRESSES; ++i)
      Cfg[i] = tmpCfg[i];

    loadData();
    dirty = true;
    }
  delete []tmpCfg;
}
//------------------------------------------------------------------------------
bool PD_Assoc::saveData()
{
  PD_alarmEditorDlg* aed = getParentWin<PD_alarmEditorDlg>(this);
  PD_Alarm* pda = aed->getAlarm(this);
  PostMessage(*pda, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_SAVE_ALARM, 0), 0);
  return true;
}
//------------------------------------------------------------------------------
void PD_Assoc::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    dirty |= Rows[i]->isDirty();
    Cfg[pos].prph = Rows[i]->getPrph();
    Cfg[pos].addr = Rows[i]->getAddr();
    Cfg[pos].type = Rows[i]->getType();
    Cfg[pos].nBit = Rows[i]->getNBit();
    Cfg[pos].offset = Rows[i]->getOffs();
    Cfg[pos].assocType = Rows[i]->getAssoc();
    Rows[i]->getFile(Cfg[pos].assocFile);
    }
}
//------------------------------------------------------------------------------
void PD_Assoc::loadData()
{
  int pos = currPos;

  bool oldDirty = dirty;
  for(int i = 0; i < MAX_GROUP && pos < MAX_ADDRESSES; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos + 1);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);

    Rows[i]->setAll(Cfg[pos].prph, Cfg[pos].addr, Cfg[pos].type, Cfg[pos].nBit, Cfg[pos].offset, Cfg[pos].assocType, Cfg[pos].assocFile);
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + i), 0, 0);
    Rows[i]->resetDirty();
    }
  dirty = oldDirty;
}
//----------------------------------------------------------------------------
bool PD_Assoc::isRowEmpty(uint pos)
{
  pos += IDC_EDIT_FIRST_FIELD;
  for(int i = 0; i < 1; ++i, pos += DEF_OFFSET_CTRL)
    if(GetDlgItemInt(*this, pos, 0, true))
      return false;
  return true;
}
//-----------------------------------------------------------------------------
