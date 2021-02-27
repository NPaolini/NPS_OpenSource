//------------ baseAdvBtnDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "macro_utils.h"
#include "baseAdvBtnDlg.h"
//----------------------------------------------------------------------------
baseAdvBtnDlg::baseAdvBtnDlg(PWin* parent, PropertyBtn& prop, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  Prop(prop), lastSel4Mod(-1)
{
}
//----------------------------------------------------------------------------
baseAdvBtnDlg::~baseAdvBtnDlg()
{
  destroy();
}
//----------------------------------------------------------------------------
bool baseAdvBtnDlg::create()
{
  if(!baseClass::create())
    return false;
  const fullPrph& prph = getDataPrf();
  fillCBPerif(GetDlgItem(*this, IDC_COMBOBOX_PERIFS6), prph.prph);
  fillCBTypeVal(GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL7), prph.typeVal);
  uint check = getCheck();
  setPrph(prph);
  setCheck(check);
  enablePrph(check >= 1);
  ENABLE(IDC_EDIT_VAR_TEXT_VALUE, check > 1);
  return true;
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::enablePrph(bool enable)
{
  uint idc[] = {IDC_COMBOBOX_PERIFS6, IDC_EDIT_ADDR2, IDC_COMBOBOX_TYPEVAL7,
                //IDC_EDIT_NDEC,
                IDC_EDIT_NBITS2, IDC_EDIT_OFFSET2, IDC_EDIT_NORM };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::setPrph(const fullPrph& DataPrf)
{
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PERIFS6), CB_SETCURSEL, DataPrf.prph, 0);
  SET_INT(IDC_EDIT_ADDR2, DataPrf.addr);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL7), CB_SETCURSEL, DataPrf.typeVal, 0);
//  SET_INT(IDC_EDIT_NDEC, DataPrf.nDec);
  SET_INT(IDC_EDIT_NBITS2, DataPrf.nBits);
  SET_INT(IDC_EDIT_OFFSET2, DataPrf.offset);
  SET_INT(IDC_EDIT_NORM, DataPrf.normaliz);
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::retrievePrph(fullPrph& DataPrf)
{
  DataPrf.prph = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PERIFS6), CB_GETCURSEL, 0, 0);
  GET_INT(IDC_EDIT_ADDR2, DataPrf.addr);
  DataPrf.typeVal = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL7), CB_GETCURSEL, 0, 0);
//  GET_INT(IDC_EDIT_NDEC, DataPrf.nDec);
  GET_INT(IDC_EDIT_NBITS2, DataPrf.nBits);
  GET_INT(IDC_EDIT_OFFSET2, DataPrf.offset);
  GET_INT(IDC_EDIT_NORM, DataPrf.normaliz);
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::setCheck(uint ix)
{
  switch(ix) {
    case 0:
      SET_CHECK(IDC_RADIO1);
      break;
    case 1:
      SET_CHECK(IDC_RADIO2);
      break;
    case 2:
      SET_CHECK(IDC_RADIO3);
      break;
    case 3:
      SET_CHECK(IDC_RADIO4);
      break;
    }
}
//----------------------------------------------------------------------------
uint baseAdvBtnDlg::retrieveCheck()
{
  if(IS_CHECKED(IDC_RADIO2))
    return 1;
  if(IS_CHECKED(IDC_RADIO3))
    return 2;
  if(IS_CHECKED(IDC_RADIO4))
    return 3;
  return 0;
}
//----------------------------------------------------------------------------
bool baseAdvBtnDlg::needVal()
{
  return retrieveCheck() > 1;
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::enableOther(uint ix)
{
  bool enable = ix >  1;
  ENABLE(IDC_EDIT_VAR_TEXT_VALUE, enable);
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::move(bool up)
{
  HWND hwLb = GetDlgItem(*this, IDC_LISTBOX_VAR_TEXT);
  int sel = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  if(!sel && up)
    return;
  int count = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  if(sel == count - 1 && !up)
    return;

  int len = SendMessage(hwLb, LB_GETTEXTLEN, sel, 0);
  LPTSTR buff = new TCHAR[len + 1];
  SendMessage(hwLb, LB_GETTEXT, sel, (LPARAM)buff);
  SendMessage(hwLb, LB_DELETESTRING, sel, 0);
  sel += up ? -1 : 1;
  SendMessage(hwLb, LB_INSERTSTRING, sel, (LPARAM)buff);
  SendMessage(hwLb, LB_SETCURSEL, sel, 0);
  SendMessage(hwLb, LB_SETCARETINDEX, sel, 0);
  delete []buff;
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::add()
{
  HWND hwLb = GetDlgItem(*this, IDC_LISTBOX_VAR_TEXT);
  int sel = lastSel4Mod;
  if(sel < 0) {
    sel = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
    if(sel < 0) {
      SendMessage(hwLb, LB_SETCURSEL, 0, 0);
      sel = 0;
      }
    else
      ++sel;
    }
  int result = makeRow(0, 0);
  LPTSTR buff = new TCHAR[result + 2];
  if(makeRow(buff, result)) {
    SendMessage(hwLb, LB_INSERTSTRING, sel, (LPARAM)buff);
    SendMessage(hwLb, LB_SETCURSEL, sel, 0);
    SendMessage(hwLb, LB_SETCARETINDEX, sel, 0);
    }
  enableMod(true);
  advancedBtnDlg* par = dynamic_cast<advancedBtnDlg*>(getParent());
  par->setOnMod(false);
  lastSel4Mod = -1;
}
//----------------------------------------------------------------------------
int baseAdvBtnDlg::rem()
{
  HWND hwLb = GetDlgItem(*this, IDC_LISTBOX_VAR_TEXT);
  int sel = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return -1;
  int len = SendMessage(hwLb, LB_GETTEXTLEN, sel, 0);
  LPTSTR buff = new TCHAR[len + 1];
  SendMessage(hwLb, LB_GETTEXT, sel, (LPARAM)buff);
  unmakedRow(buff);
  delete []buff;

  SendMessage(hwLb, LB_DELETESTRING, sel, 0);

  int count = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  int prevSel = sel;
  if(sel >= count)
    sel = count - 1;
  else if(sel > 0)
    --sel;
  SendMessage(hwLb, LB_SETCURSEL, sel, 0);
  SendMessage(hwLb, LB_SETCARETINDEX, sel, 0);

  return prevSel;
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::enableMod(bool set)
{
  uint idc[] = { IDC_LISTBOX_VAR_TEXT, IDC_BUTTON_MOD_VAR_TEXT, IDC_BUTTON_DEL_VAR_TEXT,
                  IDC_BUTTON_UP_VAR_TEXT, IDC_BUTTON_DOWN_VAR_TEXT };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], set);
}
//----------------------------------------------------------------------------
void baseAdvBtnDlg::mod()
{
  int sel = rem();
  if(sel >= 0) {
    advancedBtnDlg* par = dynamic_cast<advancedBtnDlg*>(getParent());
    par->setOnMod(true);
    enableMod(false);
    lastSel4Mod = sel;
    }
}
//----------------------------------------------------------------------------
LRESULT baseAdvBtnDlg::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIO1:
          enablePrph(false);
          enableOther(0);
          break;
        case IDC_RADIO2:
          enablePrph(true);
          enableOther(1);
          break;
        case IDC_RADIO3:
          enablePrph(true);
          enableOther(2);
          break;
        case IDC_RADIO4:
          enablePrph(true);
          enableOther(3);
          break;
        case IDC_BUTTON_UP_VAR_TEXT:
          move(true);
          break;
        case IDC_BUTTON_DOWN_VAR_TEXT:
          move(false);
          break;
        case IDC_BUTTON_ADD_VAR_TEXT:
          add();
          break;
        case IDC_BUTTON_DEL_VAR_TEXT:
          rem();
          break;
        case IDC_BUTTON_MOD_VAR_TEXT:
          mod();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
extern baseAdvBtnDlg* getAdvBtn(PWin* parent, PropertyBtn& prop, eBtnPageType type);
//----------------------------------------------------------------------------
advancedBtnDlg::advancedBtnDlg(PWin* parent, PropertyBtn& prop, uint resId, HINSTANCE hinstance)
  : Prop(prop), baseClass(parent, resId, hinstance), currPage(0), onMod(false)
{
  for(uint i = 0; i < SIZE_A(advBtn); ++i)
    advBtn[i] = getAdvBtn(this, Prop, (eBtnPageType)i);
}
//----------------------------------------------------------------------------
advancedBtnDlg::~advancedBtnDlg()
{
  destroy();
}
//----------------------------------------------------------------------------
bool advancedBtnDlg::create()
{
  if(!baseClass::create())
    return false;

  LPTSTR tabTitle[] = {
    _T("Testi"),
    _T("Colori"),
    _T("Bitmap"),
    };

  HWND hChild = GetDlgItem(*this, IDC_TAB1);

  for(uint i = 0; i < SIZE_A(tabTitle); ++i) {
    TC_ITEM ti;
    memset(&ti, 0, sizeof(ti));
    ti.mask = TCIF_TEXT;
    ti.pszText = tabTitle[i];
    ti.cchTextMax = _tcslen(tabTitle[i]);
    TabCtrl_InsertItem(hChild, i, &ti);
    }
  PRect r;
  TabCtrl_GetItemRect(hChild, 0, r);

  PRect r2;
  GetWindowRect(hChild, r2);
  for(uint i = 0; i < SIZE_A(tabTitle); ++i)
    SetWindowPos(*advBtn[i], 0, r.left, r.Height() + r2.Height(), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);

  ShowWindow(*advBtn[0], SW_SHOWNORMAL);
  if(Prop.Flags.theme)
    SET_CHECK(IDC_CHECK_THEME);
  if(Prop.Flags.flat)
    SET_CHECK(IDC_CHECK_FLAT);
  if(Prop.Flags.noBorder)
    SET_CHECK(IDC_CHECK_NO_BORDER);

  LPCTSTR cbPos[] = {
    _T("Bmp Sinistra - Testo Destra"),
    _T("Bmp Sopra - Testo Sotto"),
    _T("Bmp Destra - Testo Sinistra"),
    _T("Bmp Sotto - Testo Sopra"),
    _T("Centrati - Testo Sovrapposto"),
    };
  hChild = GetDlgItem(*this, IDC_COMBO_POS_BMP);
  for(uint i = 0; i < SIZE_A(cbPos); ++i)
    addStringToComboBox(hChild, cbPos[i]);
  SendMessage(hChild, CB_SETCURSEL, Prop.Flags.pos, 0);
  for(uint i = 0; i < SIZE_A(advBtn); ++i)
    advBtn[i]->changedPosBmp(Prop.Flags.pos);
  enableStyle();
  return true;
}
//----------------------------------------------------------------------------
void advancedBtnDlg::setOnMod(bool set)
{
  onMod = set;
  ENABLE(IDC_TAB1, !set);
  ENABLE(IDOK, !set);
  ENABLE(IDCANCEL, !set);
}
//----------------------------------------------------------------------------
LRESULT advancedBtnDlg::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_NOTIFY:
      if(((NMHDR*)lParam)->code == TCN_SELCHANGE)
        chgPage(TabCtrl_GetCurSel(((NMHDR*)lParam)->hwndFrom));
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECK_THEME:
        case IDC_CHECK_FLAT:
        case IDC_CHECK_NO_BORDER:
          enableStyle();
          break;
        }
      switch(HIWORD(wParam)) {
        case CBN_SELCHANGE:
          if(IDC_COMBO_POS_BMP == LOWORD(wParam)) {
            int sel = SendMessage(GetDlgItem(*this, IDC_COMBO_POS_BMP), CB_GETCURSEL, 0, 0);
            for(uint i = 0; i < SIZE_A(advBtn); ++i)
              advBtn[i]->changedPosBmp(sel);
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void advancedBtnDlg::enableStyle()
{
  bool set = IS_CHECKED(IDC_CHECK_NO_BORDER);
  ENABLE(IDC_CHECK_THEME, !set);
  ENABLE(IDC_CHECK_FLAT, !set);
  set = IS_CHECKED(IDC_CHECK_THEME) || IS_CHECKED(IDC_CHECK_FLAT);
  ENABLE(IDC_CHECK_NO_BORDER, !set);
}
//----------------------------------------------------------------------------
void advancedBtnDlg::chgPage(int page)
{
  ShowWindow(*advBtn[currPage], SW_HIDE);
  currPage = page;
  ShowWindow(*advBtn[currPage], SW_SHOWNORMAL);
}
//----------------------------------------------------------------------------
void advancedBtnDlg::CmOk()
{
  if(onMod)
    return;
  for(uint i = 0; i < SIZE_A(advBtn); ++i)
    advBtn[i]->saveData();
  Prop.Flags.theme = IS_CHECKED(IDC_CHECK_THEME);
  Prop.Flags.flat = IS_CHECKED(IDC_CHECK_FLAT);
  Prop.Flags.noBorder = IS_CHECKED(IDC_CHECK_NO_BORDER);

  Prop.Flags.pos = SendMessage(GetDlgItem(*this, IDC_COMBO_POS_BMP), CB_GETCURSEL, 0, 0);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void advancedBtnDlg::CmCancel()
{
  if(onMod)
    return;
  baseClass::CmCancel();
}
//----------------------------------------------------------------------------
