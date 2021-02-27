//--------- std_mntnce.cpp --------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "infoText.h"
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
//-------------------------------------------------------------------
#include "commonSimpleRow.h"
//----------------------------------------------------------------------------
#define MAX_ITEM  80
#define PAGE_SCROLL_LEN  MAX_GROUP_SR
#define MAX_V_SCROLL (MAX_ITEM - MAX_GROUP_SR)
//----------------------------------------------------------------------------
#define WM_CUSTOM_VSCROLL (WM_APP + 101)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PEditRow::PEditRow(PWin* parent, uint first_id) :
    firstId(first_id)
{
  ID = new PStatic(parent, firstId);
  Descr = new PageEdit(parent, firstId + OFFSET_DESCR_SR);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class dManageStdMaint : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdMaint(const setOfString& set, PWin* parent, HINSTANCE hInst = 0);
    ~dManageStdMaint();

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
    bool writeSimple(P_File& pfClear, P_File& pfCrypt, uint idc, uint id);

    void evVScrollBar(HWND child, int flags, int pos);
};
//-------------------------------------------------------------------
basePage* allocStdMaint( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdMaint(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdMaint::dManageStdMaint(const setOfString& set, PWin* parent, HINSTANCE hInst) :
        baseClass(set, parent, IDD_STD_MSG_MAINT, hInst), currPos(0)
{
  Rows[0] = new PEditFirstRow(this, IDC_STATIC_STD_M1);

  int i;
  for(i = 1; i < MAX_GROUP_SR - 1; ++i)
    Rows[i] = new PEditRow(this, IDC_STATIC_STD_M1 + i);

  Rows[i] = new PEditLastRow(this, IDC_STATIC_STD_M1 + i);
  uint idc[] = {
    IDC_EDIT_STD_M_TIT_RESET,
    IDC_EDIT_STD_M_MSG_RESET,
    IDC_EDIT_STD_M_TIT_WARN,
    IDC_EDIT_STD_M_TIT_PRESET,
    IDC_EDIT_STD_M_HOUR_PRESET,
    IDC_EDIT_STD_M_MINUTE_PRESET,
    };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    new langEdit(this, idc[i]);
}
//-------------------------------------------------------------------
dManageStdMaint::~dManageStdMaint()
{
  for(int i = 0; i < MAX_GROUP_SR; ++i)
    delete Rows[i];
  destroy();
}
//-------------------------------------------------------------------
bool dManageStdMaint::create()
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
bool dManageStdMaint::preProcessMsg(MSG& msg)
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
LRESULT dManageStdMaint::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
void dManageStdMaint::evVScrollBar(HWND child, int flags, int pos)
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
void dManageStdMaint::loadAll()
{

  for(uint i = 0; i < SIZE_A(Saved); ++i) {
    Saved[i].id = ID_FIRST_MAINT + i;
    LPCTSTR p = Set.getString(ID_FIRST_MAINT + i);
    if(!p)
      continue;
    _tcscpy_s(Saved[i].text, p);
    }
  LPCTSTR p = Set.getString(ID_TITLE_LIST_MANUT);
  if(p)
    SET_TEXT(IDC_EDIT_STD_M_TIT_WARN, p);

  p = Set.getString(ID_WARNING_HOURS);
  if(p)
    SET_TEXT(IDC_EDIT_WARN_HOUR, p);

  p = Set.getString(ID_ALERT_HOURS);
  if(p)
    SET_TEXT(IDC_EDIT_ALERT_HOUR, p);

  p = Set.getString(ID_TITLE_CONFIRM_RESET);
  if(p)
    SET_TEXT(IDC_EDIT_STD_M_TIT_RESET, p);

  p = Set.getString(ID_CONFIRM_RESET);
  if(p)
    SET_TEXT(IDC_EDIT_STD_M_MSG_RESET, p);

  p = Set.getString(ID_TITLE_MAINT_SET_TIME);
  if(p)
    SET_TEXT(IDC_EDIT_STD_M_TIT_PRESET, p);

  p = Set.getString(ID_MAINT_SET_HOUR);
  if(p)
    SET_TEXT(IDC_EDIT_STD_M_HOUR_PRESET, p);

  p = Set.getString(ID_MAINT_SET_MINUTE);
  if(p)
    SET_TEXT(IDC_EDIT_STD_M_MINUTE_PRESET, p);
}
//------------------------------------------------------------------------------
void dManageStdMaint::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP_SR; ++i, ++pos) {
    Saved[pos].id = Rows[i]->getID();
    Rows[i]->getDescr(Saved[pos].text);
    }
}
//------------------------------------------------------------------------------
void dManageStdMaint::loadData()
{
  int pos = currPos;

  for(int i = 0; i < MAX_GROUP_SR; ++i, ++pos) {

    Rows[i]->setID(Saved[pos].id);
    Rows[i]->setDescr(Saved[pos].text);
    }
}
//----------------------------------------------------------------------------
HBRUSH dManageStdMaint::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    uint groupBox[] = {
      IDC_GROUPBOX18,
      IDC_GROUPBOX15,
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
bool dManageStdMaint::writeSimple(P_File& pfClear, P_File& pfCrypt, uint idc, uint id)
{
  TCHAR buff[500];
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
bool dManageStdMaint::save(P_File& pfCrypt, P_File& pfClear)
{
  saveCurrData();
  for(uint i = 0; i < SIZE_A(Saved); ++i) {
    if(*Saved[i].text) {
      P_File* pf;
      if(isGlobalPageString(Saved[i].text))
        pf = &pfCrypt;
      else
        pf = &pfClear;
      TCHAR buff[20];
      wsprintf(buff, _T("%d,"), Saved[i].id);
      if(!writeStringChkUnicode(*pf, buff))
        return false;
      if(!writeStringChkUnicode(*pf, Saved[i].text))
        return false;
      if(!writeStringChkUnicode(*pf, _T("\r\n")))
        return false;
      }
    }
  if(!writeSimple(pfClear, pfCrypt, IDC_EDIT_STD_M_TIT_WARN, ID_TITLE_LIST_MANUT))
    return false;

  if(!writeSimple(pfCrypt, pfCrypt, IDC_EDIT_WARN_HOUR, ID_WARNING_HOURS))
    return false;

  if(!writeSimple(pfCrypt, pfCrypt, IDC_EDIT_ALERT_HOUR, ID_ALERT_HOURS))
    return false;

  if(!writeSimple(pfClear, pfCrypt, IDC_EDIT_STD_M_TIT_RESET, ID_TITLE_CONFIRM_RESET))
    return false;

  if(!writeSimple(pfClear, pfCrypt, IDC_EDIT_STD_M_MSG_RESET, ID_CONFIRM_RESET))
    return false;

  if(!writeSimple(pfClear, pfCrypt, IDC_EDIT_STD_M_TIT_PRESET, ID_TITLE_MAINT_SET_TIME))
    return false;

  if(!writeSimple(pfClear, pfCrypt, IDC_EDIT_STD_M_HOUR_PRESET, ID_MAINT_SET_HOUR))
    return false;

  if(!writeSimple(pfClear, pfCrypt, IDC_EDIT_STD_M_MINUTE_PRESET, ID_MAINT_SET_MINUTE))
    return false;

  return true;
}
//----------------------------------------------------------------------------
