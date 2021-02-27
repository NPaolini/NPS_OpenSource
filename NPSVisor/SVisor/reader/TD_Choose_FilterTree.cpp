//--------- TD_Choose_FilterTree.cpp -----------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "p_date.h"
#include "PListbox.h"
#include "svEdit.h"
#include "d_info.h"
#include "pCommonFilter.h"
//----------------------------------------------------------------------------
#include "TD_Choose_FilterTree.h"
#include "idMsgInfoTree.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
struct id_txt {
  uint idCtrl;
  uint idText;
  };
//----------------------------------------------------------------------------
#define IDC_T_BTN(a) { IDC_BUTTON_##a, ID_BTN_##a }
//----------------------------------------------------------------------------
id_txt idBtn[] = {
  IDC_T_BTN(ADD_DATE_TREE),
  IDC_T_BTN(REMALL_OPER_TREE),
  IDC_T_BTN(REM_DATE_TREE),
  IDC_T_BTN(REM_OPER_TREE),
  IDC_T_BTN(ADDALL_OPER_TREE),
  IDC_T_BTN(ADD_OPER_TREE),
  };
//----------------------------------------------------------------------------
#define IDC_T_RBN(a) { IDC_RADIOBUTTON_##a, ID_RBN_##a }
//----------------------------------------------------------------------------
id_txt idRBtn[] = {
  IDC_T_RBN(OPERATOR),
  IDC_T_RBN(CAUSE),
  };
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define IDC_T_TXT(a) { IDC_STATICTEXT_##a, ID_TXT_##a }
//----------------------------------------------------------------------------
id_txt idText[] = {
  IDC_T_TXT(TITLE_OPER_TREE),
  IDC_T_TXT(DATE_FROM_TREE),
  IDC_T_TXT(DATE_TO_TREE),
  };
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define IDC_T_GBOX(a) { IDC_GROUPBOX_##a, ID_GBOX_##a }
//----------------------------------------------------------------------------
id_txt idGBoxt[] = {
  IDC_T_GBOX(AVAILABLE_TREE),
  IDC_T_GBOX(CHOOSED_TREE),
  IDC_T_GBOX(DATE_TREE),
  IDC_T_GBOX(CHOOSE_REPORT),
  };
//----------------------------------------------------------------------------
static void setName(HWND HWindow);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT ListBoxKey::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SETFOCUS:
      EvSetFocus((HWND) wParam);
      break;
    case WM_KILLFOCUS:
      EvKillFocus((HWND) wParam);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void ListBoxKey::EvKillFocus(HWND hWndGetFocus)
{
  changeColor(false);
}
//----------------------------------------------------------------------------
void ListBoxKey::EvSetFocus(HWND hWndLostFocus)
{
  changeColor(true);
}
//----------------------------------------------------------------------------
void ListBoxKey::changeColor(bool hasfocus)
{
  COLORREF bkg[2] = { RGB(0xdf, 0xdf, 0xaf), RGB(0x7f, 0x7f, 0x7f) } ;
#if 0
  COLORREF text = RGB(0xff, 0xff, 0);
  SetColorSel(text, bkg[hasfocus]);
#else
  COLORREF text[2] = { RGB(0x7f, 0x7f, 0), RGB(0xff, 0xff, 0) };
  SetColorSel(text[hasfocus], bkg[hasfocus]);
#endif
  int sel = SendMessage(*this, LB_GETCURSEL, 0, 0);
  if(sel >= 0)
    SendMessage(*this, LB_SETCURSEL, sel, 0);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define DIM_ORDER 8
#define DIM_DATE  10

#define DIM_DEF_DATE  11
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
TD_Choose_FilterTree::TD_Choose_FilterTree(PWin* parent,
      vCodeType& oper, vRangeTime& date, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), Oper(oper), Date(date)
{
  svEdit* From = new svEdit(this, IDC_EDIT_DATE_FROM_TREE, DIM_DEF_DATE);
  svEdit* To = new svEdit(this, IDC_EDIT_DATE_TO_TREE, DIM_DEF_DATE);

  From->setFilter(new PDateFilter);
  To->setFilter(new PDateFilter);

  LB_Date = new ListBoxKey(this, IDC_LISTBOX_DATE_TREE);
  LB_Date->setFont(D_FONT(18, 0, 0, _T("arial")), true);

  int t[] = {  DIM_ORDER, DIM_DATE, DIM_DATE};
  int show[] = {  0, 1, 1};
  LB_Date->SetTabStop(sizeof(t)/sizeof(t[0]), t, show);
  LB_Date->setAlign(1, PListBox::aCenter);
  LB_Date->setAlign(2, PListBox::aCenter);

}
//--------------------------------------------------------------------------
TD_Choose_FilterTree::~TD_Choose_FilterTree()
{
  destroy();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define OFFS_INIT (DIM_ORDER + 1)
#define OFFS_DAY 0
#define OFFS_MONTH (OFFS_DAY + 3)
#define OFFS_YEAR (OFFS_MONTH + 3)

#define OFFS_DAY1 (OFFS_DAY + OFFS_INIT)
#define OFFS_MONTH1 (OFFS_MONTH + OFFS_INIT)
#define OFFS_YEAR1 (OFFS_YEAR + OFFS_INIT)

#define OFFS_INIT2 (OFFS_INIT + DIM_DATE + 1)
#define OFFS_DAY2 (OFFS_DAY + OFFS_INIT2)
#define OFFS_MONTH2 (OFFS_MONTH + OFFS_INIT2)
#define OFFS_YEAR2 (OFFS_YEAR + OFFS_INIT2)

#define OFFS_END_YEAR2 (OFFS_YEAR2 + 5)
//--------------------------------------------------------------------------
bool TD_Choose_FilterTree::create()
{
  if(!baseClass::create())
    return false;

  setName(*this);

  int nElem = Date.getElem();
  for(int i = 0; i < nElem; ++i)
    addToLBDate(Date[i]);
  fillLbOper();
  ::SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_OPERATOR), BM_SETCHECK, BST_CHECKED, 0);
  return true;
}
//----------------------------------------------------------------------------
LRESULT TD_Choose_FilterTree::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_DATE_TREE:
          BNClickedAddDate();
          break;
        case IDC_BUTTON_REM_DATE_TREE:
          BNClickedRemDate();
          break;

        case IDC_BUTTON_ADD_OPER_TREE:
          BNClickedAddOper();
          break;

        case IDC_BUTTON_ADDALL_OPER_TREE:
          BNClickedAddAllOper();
          break;

        case IDC_BUTTON_REM_OPER_TREE:
          BNClickedRemOper();
          break;

        case IDC_BUTTON_REMALL_OPER_TREE:
          BNClickedRemAllOper();
          break;
        }
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          switch(LOWORD(wParam)) {
            case IDC_LISTBOX_AVAILABLE_TREE:
              LBNDblclkAvailable();
              break;
            case IDC_LISTBOX_CHOOSED_TREE:
              LBNDblclkChoosed();
              break;
            case IDC_LISTBOX_DATE_TREE:
              LBNDblclkDate();
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::CmOk()
{
  saveTime();
  saveOper();
  int type = 1;
  if(BST_CHECKED ==::SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_OPERATOR), BM_GETCHECK, 0, 0))
    type = 0;
  runTree(type, this, Oper, Date);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::BNClickedAddDate()
{
  TCHAR buff[20];
  ::GetDlgItemText(*this, IDC_EDIT_DATE_FROM_TREE, buff, SIZE_A(buff));
  rangeTime range;
  if(!stringToTime(buff, range.from, true))
    return;

  ::GetDlgItemText(*this, IDC_EDIT_DATE_TO_TREE, buff, SIZE_A(buff));

  if(!stringToTime(buff, range.to, false))
    return;

  if(cMK_I64(range.from) > cMK_I64(range.to)) {
    MessageBox(*this, _T("Error in Date"), _T("Error"), MB_OK | MB_ICONSTOP);
    return;
    }
  addToLBDate(range);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::BNClickedRemDate()
{
  int ix = SendMessage(*LB_Date, LB_GETCURSEL, 0, 0);
  if(ix < 0)
    return;

  TCHAR t[200];
  SendMessage(*LB_Date, LB_GETTEXT, ix, (LPARAM)t);

  t[OFFS_YEAR1 + 4] = 0;
  ::SetDlgItemText(*this, IDC_EDIT_DATE_FROM_TREE, t + OFFS_DAY1);

  t[OFFS_YEAR2 + 4] = 0;
  ::SetDlgItemText(*this, IDC_EDIT_DATE_TO_TREE, t + OFFS_DAY2);

  SendMessage(*LB_Date, LB_DELETESTRING, ix, 0);

  int count =   SendMessage(*LB_Date, LB_GETCOUNT, 0, 0);
  if(count == ix)
    --ix;
  SendMessage(*LB_Date, LB_SETCURSEL, ix, 0);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::BNClickedMoveOper(HWND lbTarget, HWND lbSource)
{
  int nSel = ::SendMessage(lbSource, LB_GETSELCOUNT, 0, 0);
  if(nSel > 0) {
    int* buffIx = new int[nSel];
    ::SendMessage(lbSource, LB_GETSELITEMS, nSel, (LPARAM)buffIx);
    for(int i = 0; i < nSel; ++i) {
      TCHAR buff[500];
      ::SendMessage(lbSource, LB_GETTEXT, buffIx[i], (LPARAM)buff);
      ::SendMessage(lbTarget, LB_ADDSTRING, 0, (LPARAM)buff);
      }
    for(int i = nSel - 1; i >= 0; --i)
      ::SendMessage(lbSource, LB_DELETESTRING, buffIx[i], 0);
    delete []buffIx;
    }
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::SelectAll(HWND lb)
{
  int nSel = ::SendMessage(lb, LB_GETCOUNT, 0, 0);
  for(int i = 0; i < nSel; ++i)
    ::SendMessage(lb, LB_SETSEL, 1, i);
}
//--------------------------------------------------------------------------
#define LB_Available GetDlgItem(*this, IDC_LISTBOX_AVAILABLE_TREE)
#define LB_Choosed GetDlgItem(*this, IDC_LISTBOX_CHOOSED_TREE)
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::BNClickedAddOper()
{
  BNClickedMoveOper(LB_Choosed, LB_Available);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::BNClickedAddAllOper()
{
  SelectAll(LB_Available);
  BNClickedMoveOper(LB_Choosed, LB_Available);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::BNClickedRemOper()
{
  BNClickedMoveOper(LB_Available, LB_Choosed);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::BNClickedRemAllOper()
{
  SelectAll(LB_Choosed);
  BNClickedMoveOper(LB_Available, LB_Choosed);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::LBNDblclkAvailable()
{
  BNClickedMoveOper(LB_Choosed, LB_Available);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::LBNDblclkChoosed()
{
  BNClickedMoveOper(LB_Available, LB_Choosed);
}
//--------------------------------------------------------------------------
void TD_Choose_FilterTree::LBNDblclkDate()
{
  BNClickedRemDate();
}
//--------------------------------------------------------------------------
//---------------------------------------------------------
static void setName2(const id_txt* id, int size, HWND HWindow)
{
  for(int i = 0; i < size; ++i) {
    LPCTSTR p = getString(id[i].idText);
    if(p) {
      HWND h = ::GetDlgItem(HWindow, id[i].idCtrl);
      if(h)
        ::SetWindowText(h, p);
      }
    }
}
//----------------------------------------------------------------------------
#define SET_NAME(a) setName2((a), SIZE_A(a), HWindow)
//----------------------------------------------------------------------------
static void setName(HWND HWindow)
{
  SET_NAME(idText);
  SET_NAME(idGBoxt);
  SET_NAME(idBtn);
  SET_NAME(idRBtn);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void makeOrderedData(LPTSTR buff, const FILETIME& ft)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  wsprintf(buff, _T("%4d%2d%2d"), st.wYear, st.wMonth, st.wDay);
}
//----------------------------------------------------------------------------
#define SEP TAB
bool TD_Choose_FilterTree::addToLBDate(const rangeTime& range)
{
  TCHAR data1[30];
  set_format_data(data1, SIZE_A(data1), range.from, whichData(), _T("|"));
  data1[10] = 0;

  TCHAR data2[30];
  set_format_data(data2, SIZE_A(data2), range.to, whichData(), _T("|"));
  data2[10] = 0;

  TCHAR dataX[10];
  makeOrderedData(dataX, range.from);

  TCHAR buff[200];
  fillStr(buff, ' ', SIZE_A(buff));

  int offs = 0;
  copyStr(buff + offs, dataX, _tcslen(dataX));
  offs += DIM_ORDER;
  buff[offs++] = SEP;

  copyStr(buff + offs, data1, _tcslen(data1));
  offs += DIM_DATE;
  buff[offs++] = SEP;

  copyStr(buff + offs, data2, _tcslen(data2));
  offs += DIM_DATE;
  buff[offs++] = SEP;
  buff[offs] = 0;

  int pos = ::SendMessage(*LB_Date, LB_ADDSTRING, 0, (LPARAM)buff);
  if(pos >= 0) {
    SendMessage(*LB_Date, LB_SETCURSEL, pos, 0);
//    LB_Date->SetSelIndex(pos);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool TD_Choose_FilterTree::stringToTime(LPCTSTR buff, FILETIME& ft, bool from)
{
  SYSTEMTIME st;
  ZeroMemory(&st, sizeof(st));
  WORD day = (WORD)_ttoi(buff + OFFS_DAY);
  WORD month = (WORD)_ttoi(buff + OFFS_MONTH);
  if(European == whichData()) {
    st.wDay = day;
    st.wMonth = month;
    }
  else {
    st.wDay = month;
    st.wMonth = day;
    }
  st.wYear = (WORD)_ttoi(buff + OFFS_YEAR);
  if(!from) {
    st.wHour = 23;
    st.wMinute = 59;
    st.wSecond = 59;
    st.wMilliseconds = 999;
    }
  bool success = toBool(SystemTimeToFileTime(&st, &ft));
  if(!success)
    MessageBox(*this, _T("Error in Date"), _T("Error"), MB_OK | MB_ICONSTOP);
  return success;
}
//----------------------------------------------------------------------------
bool TD_Choose_FilterTree::stringToRange(LPTSTR buff, rangeTime& range)
{
  int offs[] = {
           OFFS_DAY1,
           OFFS_MONTH1,
           OFFS_YEAR1,
           OFFS_DAY2,
           OFFS_MONTH2,
           OFFS_YEAR2,
           OFFS_END_YEAR2
           };
  for(int i = 1; i < SIZE_A(offs); ++i)
    buff[offs[i] - 1] = 0;

  if(stringToTime(buff + OFFS_DAY1, range.from, true))
    return stringToTime(buff + OFFS_DAY2, range.to, false);
  return false;
}
//----------------------------------------------------------------------------
/*
inline bool operator <=(const FILETIME& ft1, const FILETIME& ft2)
{
  return cMK_I64(ft1) <= cMK_I64(ft2);
}
*/
//----------------------------------------------------------------------------
void TD_Choose_FilterTree::saveTime()
{
  int count = ::SendMessage(*LB_Date, LB_GETCOUNT, 0, 0);
  TCHAR t[500];
  Date.reset();
  FILETIME lastTo;
  int add = 0;
  for(int i = 0; i < count; ++i) {
    ::SendMessage(*LB_Date, LB_GETTEXT, i, (LPARAM)t);
//    LB_Date->GetString(t, i);
    rangeTime range;
    if(!stringToRange(t, range))
      return;
    // verifica se si intersecano le date e risolve i conflitti
    if(i && range.from <= lastTo) {
      if(range.to <= lastTo)
        continue;
      Date[add - 1].to = range.to;
      lastTo = range.to;
      continue;
      }
    Date[add++] = range;
    lastTo = range.to;
    }
}
//----------------------------------------------------------------------------
inline bool findCode(const vCodeType& oper, const codeType& code)
{
  int nElem = oper.getElem();
  for(int i = 0; i < nElem; ++i)
    if(code == oper[i])
      return true;
  return false;
}
//----------------------------------------------------------------------------
void TD_Choose_FilterTree::fillLbOper()
{
  codeOper allOper;
  int nAvail = allOper.getElem();

  HWND hLbChoosed = GetDlgItem(*this, IDC_LISTBOX_CHOOSED_TREE);
  HWND hLbAvail = GetDlgItem(*this, IDC_LISTBOX_AVAILABLE_TREE);
  HWND hTarget;
  for(int i = 0; i < nAvail; ++i) {
    if(findCode(Oper, allOper.getCode(i)))
      hTarget = hLbChoosed;
    else
      hTarget = hLbAvail;

    LPCTSTR name = allOper.getName(i);
    ::SendMessage(hTarget, LB_ADDSTRING, 0, (LPARAM)name);
    }
}
//----------------------------------------------------------------------------
void TD_Choose_FilterTree::saveOper()
{
  Oper.reset();
  HWND hLbChoosed = GetDlgItem(*this, IDC_LISTBOX_CHOOSED_TREE);

  int nCount = ::SendMessage(hLbChoosed, LB_GETCOUNT, 0, 0);
#if 0
  if(!nCount) {
    BNClickedAddAllOper();
    nCount = ::SendMessage(hLbChoosed, LB_GETCOUNT, 0, 0);
    }
#endif
  codeOper allOper;
  for(int i = 0; i < nCount; ++i) {
    TCHAR buff[500];
    ::SendMessage(hLbChoosed, LB_GETTEXT, i, (LPARAM)buff);
    Oper[i] = allOper.getCode(buff);
    }
}
//----------------------------------------------------------------------------
