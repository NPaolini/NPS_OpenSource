//--------- TD_Choose_PrintTree.cpp -----------------------------------------
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
#include "idMsgInfoTree.h"
#include "TD_Choose_PrintTree.h"
//#include "TD_Choose_FilterTree.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
struct id_txt_2 {
  uint idCtrl;
  uint idText;
  };
//----------------------------------------------------------------------------
#define IDC_T_RBN(a) { IDC_RADIOBUTTON_##a, ID_RBN_##a }
//----------------------------------------------------------------------------
id_txt_2 idRBtn[] = {
  IDC_T_RBN(OPERATOR),
  IDC_T_RBN(CAUSE),
  };
//----------------------------------------------------------------------------
#define IDC_T_TXT(a) { IDC_STATICTEXT_##a, ID_TXT_##a }
//----------------------------------------------------------------------------
id_txt_2 idText[] = {
  IDC_T_TXT(TITLE_OPER_TREE),
  IDC_T_TXT(DATE_FROM_TREE),
  IDC_T_TXT(DATE_TO_TREE),
  };
//----------------------------------------------------------------------------
#define IDC_T_GBOX(a) { IDC_GROUPBOX_##a, ID_GBOX_##a }
//----------------------------------------------------------------------------
id_txt_2 idGBoxt[] = {
  IDC_T_GBOX(AVAILABLE_TREE),
  IDC_T_GBOX(DATE_TREE),
  IDC_T_GBOX(CHOOSE_REPORT),
  };
//----------------------------------------------------------------------------
static void setName(HWND HWindow);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
TCHAR TD_Choose_PrintTree::bDate[2][20];
TCHAR TD_Choose_PrintTree::bTime[2][20];
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define DIM_DEF_DATE  11
#define DIM_DEF_TIME  6
#define OFFS_DAY 0
#define OFFS_MONTH (OFFS_DAY + 3)
#define OFFS_YEAR (OFFS_MONTH + 3)
//-----------------------------------------------------------
#define MAX_MYTIME_LEN 5
//-----------------------------------------------------------
class myTimeFilter : public PTimeFilter
{
  protected:
    virtual int getLimitLen() { return MAX_MYTIME_LEN; }
};
//----------------------------------------------------------------------------
TD_Choose_PrintTree::TD_Choose_PrintTree(PWin* parent,
      vCodeType& oper, vRangeTime& date, uint resId, HINSTANCE module)
:
    baseClass(parent, resId, module), Oper(oper), Date(date)
{
  svEdit* From = new svEdit(this, IDC_EDIT_DATE_FROM_TREE, DIM_DEF_DATE);
  svEdit* To = new svEdit(this, IDC_EDIT_DATE_TO_TREE, DIM_DEF_DATE);

  From->setFilter(new PDateFilter);
  To->setFilter(new PDateFilter);

  From = new svEdit(this, IDC_EDIT_TIME_FROM_TREE, DIM_DEF_TIME);
  To = new svEdit(this, IDC_EDIT_TIME_TO_TREE, DIM_DEF_TIME);

  From->setFilter(new myTimeFilter);
  To->setFilter(new myTimeFilter);
}
//--------------------------------------------------------------------------
TD_Choose_PrintTree::~TD_Choose_PrintTree()
{
  destroy();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TD_Choose_PrintTree::create()
{
  if(!baseClass::create())
    return false;

  setName(*this);

  fillLbOper();
  ::SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_OPERATOR), BM_SETCHECK, BST_CHECKED, 0);
  ::SendMessage(GetDlgItem(*this, IDC_LISTBOX_AVAILABLE_TREE), LB_SETCURSEL, 0, 0);
  ::SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_PREVIEW), BM_SETCHECK, BST_CHECKED, 0);
  int ids[] = {
      IDC_EDIT_DATE_FROM_TREE,
      IDC_EDIT_TIME_FROM_TREE,
      IDC_EDIT_DATE_TO_TREE,
      IDC_EDIT_TIME_TO_TREE,
      };
  LPTSTR buff[] = {
      bDate[0], bTime[0], bDate[1], bTime[1] };
  for(int i = 0; i < SIZE_A(ids); ++i)
    SetDlgItemText(*this, ids[i], buff[i]);
  return true;
}
//--------------------------------------------------------------------------
void TD_Choose_PrintTree::CmOk()
{
  int type = 1;
  if(BST_CHECKED ==::SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_OPERATOR), BM_GETCHECK, 0, 0))
    type = 0;

  bool preview = BST_CHECKED ==::SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_PREVIEW), BM_GETCHECK, 0, 0);
  bool setup = BST_CHECKED ==::SendMessage(GetDlgItem(*this, IDC_REPORT_SETUP), BM_GETCHECK, 0, 0);
  if(!saveTime())
    return;
  saveOper(!type);
  runPrint(type, this, Oper, Date, preview, setup);
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
#define LB_Available GetDlgItem(IDC_LISTBOX_AVAILABLE_TREE)
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void TD_Choose_PrintTree::LBNDblclkAvailable()
{
  CmOk();
}
//--------------------------------------------------------------------------
//---------------------------------------------------------
static void setName2(const id_txt_2* id, int size, HWND HWindow)
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
  SET_NAME(idRBtn);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define SEP TAB
bool TD_Choose_PrintTree::stringToTime(LPCTSTR date, LPCTSTR time, FILETIME& ft, bool from)
{
  SYSTEMTIME st;
  ZeroMemory(&st, sizeof(st));
  WORD day = (WORD)_ttoi(date + OFFS_DAY);
  WORD month = (WORD)_ttoi(date + OFFS_MONTH);
  if(European == whichData()) {
    st.wDay = day;
    st.wMonth = month;
    }
  else {
    st.wDay = month;
    st.wMonth = day;
    }
  st.wYear = (WORD)_ttoi(date + OFFS_YEAR);
  WORD hour = _ttoi(time);
  WORD minute = _ttoi(time + 3);
  st.wHour = hour;
  st.wMinute = minute;
#if 0
  if(from)
    st.wSecond = 59;
    st.wMilliseconds = 999;
    }
#endif
  bool success = toBool(SystemTimeToFileTime(&st, &ft));
  if(!success)
    MessageBox(*this, _T("Error in Date"), _T("Error"), MB_OK | MB_ICONSTOP);
  return success;
}
//----------------------------------------------------------------------------
/*
inline bool operator <=(const FILETIME& ft1, const FILETIME& ft2)
{
  return cMK_I64(ft1) <= cMK_I64(ft2);
}
*/
//----------------------------------------------------------------------------
bool TD_Choose_PrintTree::saveTime()
{
  int ids[] = {
      IDC_EDIT_DATE_FROM_TREE,
      IDC_EDIT_TIME_FROM_TREE,
      IDC_EDIT_DATE_TO_TREE,
      IDC_EDIT_TIME_TO_TREE,
      };
  LPTSTR buff[] = {
      bDate[0], bTime[0], bDate[1], bTime[1] };
  for(int i = 0; i < SIZE_A(ids); ++i)
    GetDlgItemText(*this, ids[i], buff[i], sizeof(bDate[0]));
  rangeTime range;
  if(!stringToTime(bDate[0], bTime[0], range.from, true))
    return false;
  if(!stringToTime(bDate[1], bTime[1], range.to, true))
    return false;

  if(range.from > range.to) {
    MessageBox(*this, _T("Error in Date"), _T("Error"), MB_OK | MB_ICONSTOP);
    return false;
    }
  Date.reset();
  Date[0] = range;
  return true;
}
//----------------------------------------------------------------------------
void TD_Choose_PrintTree::fillLbOper()
{
  codeOper allOper;
  int nAvail = allOper.getElem();

  HWND hTarget = GetDlgItem(*this, IDC_LISTBOX_AVAILABLE_TREE);
  for(int i = 0; i < nAvail; ++i) {
    LPCTSTR name = allOper.getName(i);
    ::SendMessage(hTarget, LB_ADDSTRING, 0, (LPARAM)name);
    }
}
//----------------------------------------------------------------------------
void TD_Choose_PrintTree::saveOper(bool filter)
{
  Oper.reset();
  codeOper allOper;

  HWND hLb = GetDlgItem(*this, IDC_LISTBOX_AVAILABLE_TREE);
  if(!filter) {
    int nCount = ::SendMessage(hLb, LB_GETCOUNT, 0, 0);
    for(int i = 0; i < nCount; ++i) {
      TCHAR buff[500];
      ::SendMessage(hLb, LB_GETTEXT, i, (LPARAM)buff);
      Oper[i] = allOper.getCode(buff);
      }
    return;
    }
#if 1
  int nSel = ::SendMessage(hLb, LB_GETSELCOUNT, 0, 0);
  if(nSel > 0) {
    int* buffIx = new int[nSel];
    ::SendMessage(hLb, LB_GETSELITEMS, nSel, (LPARAM)buffIx);
    for(int i = 0; i < nSel; ++i) {
      TCHAR buff[500];
      ::SendMessage(hLb, LB_GETTEXT, buffIx[i], (LPARAM)buff);
      Oper[i] = allOper.getCode(buff);
      }
    delete []buffIx;
    }
#else
  int sel = ::SendMessage(hLb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[500];
  ::SendMessage(hLb, LB_GETTEXT, sel, (LPARAM)buff);
  Oper[0] = allOper.getCode(buff);
#endif
}
//----------------------------------------------------------------------------
void TD_Choose_PrintTree::checkEnable()
{
  bool enable = BST_CHECKED ==::SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_OPERATOR), BM_GETCHECK, 0, 0);
  ::EnableWindow(GetDlgItem(*this, IDC_LISTBOX_AVAILABLE_TREE), enable);
}
//----------------------------------------------------------------------------
