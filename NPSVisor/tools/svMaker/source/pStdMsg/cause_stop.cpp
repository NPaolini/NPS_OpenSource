//--------- cause_stop.cpp --------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <stdio.h>
#include "infoText.h"
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
//-------------------------------------------------------------------
#include "commonSimpleRow.h"
//----------------------------------------------------------------------------
#define MAX_ITEM  300
#define PAGE_SCROLL_LEN  MAX_GROUP_SR
#define MAX_V_SCROLL (MAX_ITEM - MAX_GROUP_SR)
//----------------------------------------------------------------------------
#define WM_CUSTOM_VSCROLL (WM_APP + 101)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class dManageStdCauseStop : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdCauseStop(const setOfString& set, PWin* parent, HINSTANCE hInst = 0);
    ~dManageStdCauseStop();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    TCHAR Filename[_MAX_PATH];
    baseRow Saved[MAX_ITEM];
    class PEditRow* Rows[MAX_GROUP_SR];
    int currPos;

    void saveCurrData();
    bool saveData();
    void loadData();
    void loadAll();
    bool writeSimple(P_File& pf, uint idc, uint id);
    bool writeSimple(P_File& pfClear, P_File& pfCrypt, uint idc, uint id);
    void evVScrollBar(HWND child, int flags, int pos);
};
//-------------------------------------------------------------------
basePage* allocStdCauseStop( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdCauseStop(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdCauseStop::dManageStdCauseStop(const setOfString& set, PWin* parent, HINSTANCE hInst) :
        baseClass(set, parent, IDD_STD_MSG_CAUSE_STOP, hInst), currPos(0)
{
  Rows[0] = new PEditFirstRow(this, IDC_STATIC_STD_M1);

  int i;
  for(i = 1; i < MAX_GROUP_SR - 1; ++i)
    Rows[i] = new PEditRow(this, IDC_STATIC_STD_M1 + i);

  Rows[i] = new PEditLastRow(this, IDC_STATIC_STD_M1 + i);
  uint idc[] = {
    IDC_EDIT_STD_GET_WAFER,
    IDC_EDIT_STD_TIT_NOT_OPER,
    IDC_EDIT_STD_MSG_NOT_OPER,
    IDC_EDIT_STD_CAUSE_ALARM,
    IDC_EDIT_STD_CAUSE_END,
/*
    IDC_EDIT_STD_M1,
    IDC_EDIT_STD_M2,
    IDC_EDIT_STD_M3,
    IDC_EDIT_STD_M4,
    IDC_EDIT_STD_M5,
    IDC_EDIT_STD_M6,
    IDC_EDIT_STD_M7,
    IDC_EDIT_STD_M8,
*/
    };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    new langEdit(this, idc[i]);

}
//-------------------------------------------------------------------
dManageStdCauseStop::~dManageStdCauseStop()
{
  for(int i = 0; i < MAX_GROUP_SR; ++i)
    delete Rows[i];
  destroy();
}
//-------------------------------------------------------------------
bool dManageStdCauseStop::create()
{
  if(!baseClass::create())
    return false;

  loadAll();
  loadData();

  HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_STD_M);
  if(hsb) {
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nPage = PAGE_SCROLL_LEN;
    si.nMin = 0;
    si.nMax = MAX_ITEM - 1;
    SendMessage(hsb, SBM_SETSCROLLINFO, 1, (LPARAM)&si);
    }

  return true;
}
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
bool dManageStdCauseStop::preProcessMsg(MSG& msg)
{
  switch(msg.message) {
    case WM_KEYDOWN:
      switch(msg.wParam) {
        case VK_NEXT:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
          break;
        case VK_PRIOR:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
          break;
        case VK_HOME:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_LEFT, 0);
          break;
        case VK_END:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_RIGHT, 0);
          break;
        }
      break;
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
LRESULT dManageStdCauseStop::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CUSTOM_VSCROLL:
      lParam = (LPARAM)GetDlgItem(*this, IDC_SCROLLBAR_STD_M);
      // passa sotto
    case WM_VSCROLL:
      evVScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//------------------------------------------------------------------------------
void dManageStdCauseStop::evVScrollBar(HWND child, int flags, int pos)
{
  int curr = SendMessage(child, SBM_GETPOS, 0, 0);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = MAX_V_SCROLL;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - PAGE_SCROLL_LEN;
      break;
    case SB_PAGERIGHT:
      pos = curr + PAGE_SCROLL_LEN;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if(pos > MAX_V_SCROLL)
    pos = MAX_V_SCROLL;
  SendMessage(child, SBM_SETPOS, pos, true);

  saveCurrData();
  currPos = pos;
  loadData();
}
//------------------------------------------------------------------------------
#define SET_TXT(ctrl, id) p = Set.getString(id); if(p) SET_TEXT(ctrl, p)
//------------------------------------------------------------------------------
void dManageStdCauseStop::loadAll()
{
  for(uint i = 0; i < SIZE_A(Saved); ++i) {
    Saved[i].id = ID_CAUSE_TITLE_INIT + i;
    LPCTSTR p = Set.getString(ID_CAUSE_TITLE_INIT + i);
    if(!p)
      continue;
    _tcscpy_s(Saved[i].text, p);
    }
  LPCTSTR p;
  SET_TXT(IDC_EDIT_STD_GET_WAFER, ID_MSG_NEED_CODE);
  SET_TXT(IDC_EDIT_STD_CAUSE_ALARM, ID_CAUSE_ALARM);
  SET_TXT(IDC_EDIT_STD_CAUSE_END, ID_CAUSE_NORMAL_STOP);
  SET_TXT(IDC_EDIT_PRF_WAFER, ID_PRF_READER);
  SET_TXT(IDC_EDIT_STD_MSG_NOT_OPER, ID_NOT_OPER_DEFINED);
  SET_TXT(IDC_EDIT_STD_TIT_NOT_OPER, ID_NOT_OPER_DEFINED_TITLE);

  p = Set.getString(ID_BIT_CARD);
  if(p) {
    int addr;
    int offs;
    int type;
    _stscanf_s(p, _T("%d,%d,%d"), &addr, &type, &offs);
    SET_INT(IDC_EDIT_STD_BIT_SS_ADDR, addr);
    SET_INT(IDC_EDIT_STD_BIT_SS_OFFS, offs);
    SET_INT(IDC_EDIT_STD_BIT_SS_TYPE, type);
    }

  p = Set.getString(ID_ADDR_CODE_OPER);
  if(p) {
    int prph;
    int addr;
    _stscanf_s(p, _T("%d,%d"), &prph, &addr);

    SET_INT(IDC_EDIT_STD_PRPH_OPER, prph);
    SET_INT(IDC_EDIT_STD_ADDR_OPER, addr);
    }

  p = Set.getString(ID_ADDR_ALTERNATE_INPUT_CAUSE);
  if(p) {
    int addr;
    _stscanf_s(p, _T("%d"), &addr);
    SET_INT(IDC_EDIT_ADDR_ALTERNATE_CHOOSE_CAUSE, addr);
    }
}
//------------------------------------------------------------------------------
void dManageStdCauseStop::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP_SR; ++i, ++pos) {
    Saved[pos].id = Rows[i]->getID();
    Rows[i]->getDescr(Saved[pos].text);
    }
}
//------------------------------------------------------------------------------
void dManageStdCauseStop::loadData()
{
  int pos = currPos;

  for(int i = 0; i < MAX_GROUP_SR; ++i, ++pos) {

    Rows[i]->setID(Saved[pos].id);
    Rows[i]->setDescr(Saved[pos].text);
    }
}
//----------------------------------------------------------------------------
HBRUSH dManageStdCauseStop::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    uint groupBox[] = {
      IDC_GROUPBOX18,
      IDC_GROUPBOX15,
      IDC_GROUPBOX22,
      IDC_GROUPBOX24,
      };
    for(uint i = 0; i < SIZE_A(groupBox); ++i)
      if(GetDlgItem(*this, groupBox[i]) == hWndChild) {
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
    SetBkColor(hdc, bkgColor3);
    return (Brush3);
    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, bkgColor);
    return (Brush);
    }
  return 0;
}
//-------------------------------------------------------------------
bool dManageStdCauseStop::writeSimple(P_File& pf, uint idc, uint id)
{
  TCHAR buff[500];
  TCHAR t[20];
  GET_TEXT(idc, buff);
  if(!*buff)
    return true;
  wsprintf(t, _T("%d,"), id);
  if(!writeStringChkUnicode(pf, t))
    return false;
  if(!writeStringChkUnicode(pf, buff))
    return false;
  if(!writeStringChkUnicode(pf, _T("\r\n")))
    return false;
  return true;
}
//-------------------------------------------------------------------
bool dManageStdCauseStop::writeSimple(P_File& pfClear, P_File& pfCrypt, uint idc, uint id)
{
  TCHAR buff[500 * 10];
  TCHAR t[20];
  GET_TEXT(idc, buff);
  if(!*buff)
    return true;
  P_File* pf;
  if(isGlobalPageString(buff))
    pf = &pfCrypt;
  else
    pf = &pfClear;
  wsprintf(t, _T("%d,"), id);
  if(!writeStringChkUnicode(*pf, t))
    return false;
  if(!writeStringChkUnicode(*pf, buff))
    return false;
  if(!writeStringChkUnicode(*pf, _T("\r\n")))
    return false;
  return true;
}
//-------------------------------------------------------------------
#define SAVE_TXT(idc, id)  if(!writeSimple(pfCrypt, idc, id)) return false
//-------------------------------------------------------------------
#define SAVE_TXT_S(idc, id)  if(!writeSimple(pfClear, pfCrypt, idc, id)) return false
//-------------------------------------------------------------------
bool dManageStdCauseStop::save(P_File& pfCrypt, P_File& pfClear)
{
  saveCurrData();
  for(uint i = 0; i < SIZE_A(Saved); ++i) {
    if(*Saved[i].text) {
      TCHAR buff[20];
      P_File* pf;
      if(isGlobalPageString(Saved[i].text))
        pf = &pfCrypt;
      else
        pf = &pfClear;

      wsprintf(buff, _T("%d,"), Saved[i].id);
      if(!writeStringChkUnicode(*pf, buff))
        return false;
      if(!writeStringChkUnicode(*pf, Saved[i].text))
        return false;
      if(!writeStringChkUnicode(*pf, _T("\r\n")))
        return false;
      }
    else
      break;
    }
  SAVE_TXT_S(IDC_EDIT_STD_GET_WAFER, ID_MSG_NEED_CODE);
  SAVE_TXT_S(IDC_EDIT_STD_CAUSE_ALARM, ID_CAUSE_ALARM);
  SAVE_TXT_S(IDC_EDIT_STD_CAUSE_END, ID_CAUSE_NORMAL_STOP);
  SAVE_TXT(IDC_EDIT_PRF_WAFER, ID_PRF_READER);
  SAVE_TXT_S(IDC_EDIT_STD_MSG_NOT_OPER, ID_NOT_OPER_DEFINED);
  SAVE_TXT_S(IDC_EDIT_STD_TIT_NOT_OPER, ID_NOT_OPER_DEFINED_TITLE);

  int prph = 0;
  GET_INT(ID_PRF_READER, prph);
  int addr = 0;
  GET_INT(IDC_EDIT_STD_BIT_SS_ADDR, addr);
  if(addr || prph > 1) {
    int offs;
    int type;
    GET_INT(IDC_EDIT_STD_BIT_SS_OFFS, offs);
    GET_INT(IDC_EDIT_STD_BIT_SS_TYPE, type);
    TCHAR buff[50];
    wsprintf(buff, _T("%d,%d,%d,%d\r\n"), ID_BIT_CARD, addr, type, offs);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }
  prph = 0;
  addr = 0;
  GET_INT(IDC_EDIT_STD_PRPH_OPER, prph);
  if(prph) {
    GET_INT(IDC_EDIT_STD_ADDR_OPER, addr);
    TCHAR buff[50];
    wsprintf(buff, _T("%d,%d,%d\r\n"), ID_ADDR_CODE_OPER, prph, addr);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  addr = 0;
  GET_INT(IDC_EDIT_ADDR_ALTERNATE_CHOOSE_CAUSE, addr);
  if(addr) {
    TCHAR buff[50];
    wsprintf(buff, _T("%d,%d\r\n"), ID_ADDR_ALTERNATE_INPUT_CAUSE, addr);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  return true;
}
//----------------------------------------------------------------------------
