//-------- d_choosedate.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "p_date.h"
#include "p_util.h"
#include "pcommonfilter.h"
#include "pedit.h"
#include "p_body.h"
#include "d_chooseDate.h"
#include "id_btn.h"
#include "5500.h"

#include "1.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class ListBoxKeyFocus : public PListBox
{
  public:
    ListBoxKeyFocus (PWin* parent, int id, int hFont = 15, int len = 255):
      PListBox(parent, id, len), limite(upper), heightFont(hFont) {}

    virtual bool create();
    virtual void ClearList() { SendMessage(*this, LB_RESETCONTENT, 0, 0); limite = upper; }
    void changeColor(bool focus);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void ev_KeyDown(uint key);
    void evKillFocus();
    void evSetFocus();
    virtual bool preProcessMsg(MSG& msg);
  private:
    enum where { upper = -1, inside, lower };
    where limite;
    int heightFont;
};
//----------------------------------------------------------------------------
bool ListBoxKeyFocus::create()
{
  if(!PListBox::create())
    return false;
  setFont(DEF_FONT(heightFont), true);
  return true;
}
//----------------------------------------------------------------------------
LRESULT ListBoxKeyFocus::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_KEYDOWN:
      ev_KeyDown(wParam);
      break;
    case WM_KILLFOCUS:
      evKillFocus();
      break;
    case WM_SETFOCUS:
      evSetFocus();
      break;
    }
  return PListBox::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void ListBoxKeyFocus::ev_KeyDown(uint key)
{
  switch(key) {
    case VK_UP:
    case VK_DOWN:
      break;
    case VK_LEFT:
      key = VK_UP;
      break;
    case VK_RIGHT:
      key = VK_DOWN;
      break;
    default:
      return;
    }
  int sel = SendMessage(*this, LB_GETCURSEL, 0, 0);
  if(VK_UP == key) {
    if(sel <= 0) {
      if(upper == limite)
        shift_click(VK_TAB);
      else
        limite = upper;
      }
    else
      limite = inside;
    }
  else /*if(VK_DOWN == key)*/ {
    int count = SendMessage(*this, LB_GETCOUNT ,0 ,0);
    if(count - 1 == sel) {
        if(lower == limite)
        click(VK_TAB);
      else
        limite = lower;
      }
    else
      limite = inside;
    }
}
//----------------------------------------------------------------------------
bool ListBoxKeyFocus::preProcessMsg(MSG& msg)
{
  if(WM_KEYDOWN == msg.message)
    if(VK_RETURN == msg.wParam)
        click(VK_TAB);
  return PListBox::preProcessMsg(msg);
}
//----------------------------------------------------------------------------
void ListBoxKeyFocus::evKillFocus()
{
  changeColor(false);
}
//----------------------------------------------------------------------------
void ListBoxKeyFocus::evSetFocus()
{
  changeColor(true);
  int sel = SendMessage(*this, LB_GETCURSEL, 0, 0);
  limite = inside;
  if(!sel)
    limite = upper;
  else {
    int count = SendMessage(*this, LB_GETCOUNT, 0, 0);
    if(count - 1 == sel)
      limite = lower;
    }
}
//----------------------------------------------------------------------------
void ListBoxKeyFocus::changeColor(bool hasfocus)
{
  COLORREF bkg[2] = { RGB(0xdf, 0xdf, 0xaf), RGB(0x7f, 0x7f, 0x7f) } ;
//  TColor bkg[2] = { TColor(RGB(0xaf, 0xaf, 0xaf)), TColor(RGB(0x7f, 0x7f, 0x7f)) } ;
  COLORREF text[2] = { RGB(0x7f, 0x7f, 0), RGB(0xff, 0xff, 0) };
  SetColorSel(text[hasfocus], bkg[hasfocus]);

  int ix = SendMessage(*this, LB_GETCURSEL, 0, 0);
  SendMessage(*this, LB_SETCURSEL, ix, 0);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define DIM_ORDER 8
#define DIM_DATE  10

#define DIM_DEF_DATE  11
//----------------------------------------------------------------------------
TD_ChooseDate::TD_ChooseDate(PWin* parent, PVect<rangeTime>& range, uint resId)
:
    baseClass(parent, resId), Range(range)
{
  LB_Choose = new ListBoxKeyFocus(this, IDC_LISTBOX_CHOOSE_LG);

  int t[] = {  DIM_ORDER, DIM_DATE, DIM_DATE};
  int show[] = {  0, 1, 1};
  LB_Choose->SetTabStop(sizeof(t)/sizeof(t[0]), t, show);
  LB_Choose->setAlign(1, PListBox::aCenter);
  LB_Choose->setAlign(2, PListBox::aCenter);

  From = new svEdit(this, IDC_EDIT_FROM_DT_EXP, DIM_DEF_DATE);
  To = new svEdit(this, IDC_EDIT_TO_DT_EXP, DIM_DEF_DATE);
  From->setFilter(new PDateFilter);
  To->setFilter(new PDateFilter);
/*
  From = new my_Edit(this, IDC_EDIT_FROM_DT_EXP, DIM_DEF_DATE);
  To = new my_Edit(this, IDC_EDIT_TO_DT_EXP, DIM_DEF_DATE);
  From->SetValidator(new TPXPictureValidator("[##/##/####]",true));
  To->SetValidator(new TPXPictureValidator("[##/##/####]",true));

  new ButtonKey(this, IDC_BUTTON_F4);
*/
}
//----------------------------------------------------------------------------
TD_ChooseDate::~TD_ChooseDate()
{
  destroy();
}
//----------------------------------------------------------------------------
static void makeOrderedData(LPTSTR buff, size_t lenBuff, const FILETIME& ft)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  _stprintf_s(buff, lenBuff, _T("%4d%2d%2d"), st.wYear, st.wMonth, st.wDay);
}
//----------------------------------------------------------------------------
#define SEP TAB
bool TD_ChooseDate::addToLB(const rangeTime& range)
{
  TCHAR data1[30];
  set_format_data(data1, SIZE_A(data1), range.from, whichData(), _T("|"));
  data1[10] = 0;

  TCHAR data2[30];
  set_format_data(data2, SIZE_A(data2), range.to, whichData(), _T("|"));
  data2[10] = 0;

  TCHAR dataX[10];
  makeOrderedData(dataX, SIZE_A(dataX), range.from);

  TCHAR buff[200];
  fillStr(buff, _T(' '), SIZE_A(buff));

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

  int pos = SendMessage(*LB_Choose, LB_ADDSTRING, 0, (LPARAM)buff);
  if(pos >= 0) {
    SendMessage(*LB_Choose, LB_SETCURSEL, pos, 0);
    LB_SelChange();
    SetFocus(*LB_Choose);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void TD_ChooseDate::enableBtn(int id, bool enable)
{
  HWND hw = ::GetDlgItem(*this, id);
  if(hw)
    ::EnableWindow(hw, enable);
}
//----------------------------------------------------------------------------
bool TD_ChooseDate::isEnableBtn(int id)
{
  HWND hw = ::GetDlgItem(*this, id);
  if(hw)
    return toBool(::IsWindowEnabled(hw));
  return false;
}
//----------------------------------------------------------------------------
void TD_ChooseDate::LB_SelChange()
{
  int ix = SendMessage(*LB_Choose, LB_GETCURSEL, 0, 0);
  int count = SendMessage(*LB_Choose, LB_GETCOUNT, 0, 0);
  bool enabled = ix >= 0 && count;

  enableBtn(IDC_BUTTON_F4, enabled);
}
//----------------------------------------------------------------------------
#define SET_(id, txt)   ::SetDlgItemText(*this, id, txt)
//----------------------------------------------------------------------------
bool TD_ChooseDate::create()
{
  if(!baseClass::create())
    return false;

  // INSERT>> Your code here.
  setCaption(getStringOrId(ID_TEXT_CHOOSE_TITLE));
  for(int i = 0; i < MAX_BTN_CHOOSE; ++i)
#if 1
    SET_(IDC_STATICTEXT_G_F1 + i, getStringOrIdByLangGlob(ID_TEXT_CHOOSE_BTN + i));

  SET_(IDC_STATICTEXT_G_FROM, getStringOrIdByLangGlob(ID_TEXT_CHOOSE_FROM));//1970));
  SET_(IDC_STATICTEXT_G_TO, getStringOrIdByLangGlob(ID_TEXT_CHOOSE_TO));//1971));

#else
    SET_(IDC_STATICTEXT_G_F1 + i, getStringOrId(ID_TEXT_CHOOSE_BTN + i));

  SET_(IDC_STATICTEXT_G_FROM, getStringOrId(ID_TEXT_CHOOSE_FROM));//1970));
  SET_(IDC_STATICTEXT_G_TO, getStringOrId(ID_TEXT_CHOOSE_TO));//1971));
#endif
  PRect r;
  GetWindowRect(*LB_Choose, r);
  int h = LB_Choose->getHItem();
  int hScroll = GetSystemMetrics(SM_CYHSCROLL);
  int horzExtend = SendMessage(*LB_Choose, LB_GETHORIZONTALEXTENT, 0, 0);
  if(horzExtend < r.Width())
    hScroll = 0;
  int hBorder = GetSystemMetrics(SM_CYEDGE);
  int diff = (r.Height() - hScroll - hBorder * 2) % h;

  r.bottom -= diff;

  LB_Choose->setWindowPos(0, r, SWP_NOMOVE | SWP_NOZORDER);

  int nElem = Range.getElem();
  for(int i = 0; i < nElem; ++i)
    addToLB(Range[i]);

  return true;
}
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
//----------------------------------------------------------------------------
bool TD_ChooseDate::stringToTime(LPCTSTR buff, FILETIME& ft, bool from)
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
bool TD_ChooseDate::stringToRange(LPTSTR buff, rangeTime& range)
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
void TD_ChooseDate::BNClicked_F1()
{
  // INSERT>> Your code here.
  int count = SendMessage(*LB_Choose, LB_GETCOUNT, 0, 0);
  TCHAR t[500];
  Range.reset();
  FILETIME lastTo;
  int add = 0;
  for(int i = 0; i < count; ++i) {
    SendMessage(*LB_Choose, LB_GETTEXT, i, (LPARAM)t);
    rangeTime range;
    if(!stringToRange(t, range))
      return;
    // verifica se si intersecano le date e risolve i conflitti
    if(i && range.from <= lastTo) {
      if(range.to <= lastTo)
        continue;
      Range[add - 1].to = range.to;
      lastTo = range.to;
      continue;
      }
    Range[add++] = range;
    lastTo = range.to;
    }
   EndDialog(*this, IDOK);
//  TDialog::CmOk();
}
//----------------------------------------------------------------------------
void TD_ChooseDate::BNClicked_F2()
{
  // INSERT>> Your code here.
   EndDialog(*this, IDCANCEL);
}
//----------------------------------------------------------------------------
void TD_ChooseDate::BNClicked_F3()
{
  // INSERT>> Your code here.
  if(!isEnableBtn(IDC_BUTTON_F3))
    return;

  TCHAR buff[20];
  SendMessage(*From, WM_GETTEXT, (WPARAM)SIZE_A(buff), (LPARAM)buff);
  rangeTime range;
  if(!stringToTime(buff, range.from, true))
    return;

  SendMessage(*To, WM_GETTEXT, (WPARAM)SIZE_A(buff), (LPARAM)buff);
  if(!stringToTime(buff, range.to, false))
    return;

  if(cMK_I64(range.from) > cMK_I64(range.to)) {
    MessageBox(*this, _T("Error in Date"), _T("Error"), MB_OK | MB_ICONSTOP);
    return;
    }
  addToLB(range);
  SetFocus(*From);
}
//---------------------------------------------------------------------
void TD_ChooseDate::BNClicked_F4()
{
  if(!isEnableBtn(IDC_BUTTON_F4))
    return;
  int ix = SendMessage(*LB_Choose, LB_GETCURSEL, 0, 0);
  if(ix < 0)
    return;

  TCHAR t[200];
  SendMessage(*LB_Choose, LB_GETTEXT, ix, (LPARAM)t);

  t[OFFS_YEAR1 + 4] = 0;
  From->setText(t + OFFS_DAY1);

  t[OFFS_YEAR2 + 4] = 0;
  To->setText(t + OFFS_DAY2);

  SendMessage(*LB_Choose, LB_DELETESTRING, ix, 0);

  if(SendMessage(*LB_Choose, LB_GETCOUNT, 0, 0) == ix)
    --ix;
  SendMessage(*LB_Choose, LB_SETCURSEL, ix, 0);
  if(!isEnableBtn(IDC_BUTTON_F3))
    enableBtn(IDC_BUTTON_F3, true);

  LB_SelChange();
  SetFocus(*From);
}
//---------------------------------------------------------------------
//----------------------------------------------------------------------------
#define EV_BN_CLICKED(id, fz) \
        case id:  \
          fz();   \
          break;
//----------------------------------------------------------------------------
LRESULT TD_ChooseDate::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        EV_BN_CLICKED(IDC_BUTTON_F1, BNClicked_F1)
        EV_BN_CLICKED(IDC_BUTTON_F2, BNClicked_F2)
        EV_BN_CLICKED(IDC_BUTTON_F3, BNClicked_F3)
        EV_BN_CLICKED(IDC_BUTTON_F4, BNClicked_F4)

        EV_BN_CLICKED(IDOK, BNClicked_F1)
        EV_BN_CLICKED(IDCANCEL, BNClicked_F2)
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);

}
