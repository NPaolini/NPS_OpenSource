//-------- P_ModListCodes.cpp ---------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "mainwin.h"
#include "P_ModListCodes.h"
#include "P_ModListFiles.h"
#include "P_ModEdit.h"
#include "p_Util.h"
#include "resource.h"
#include "P_file.h"
//-------------------------------------------------------------------
#define IDC_LIST1 4321
//-------------------------------------------------------------------
P_ModListCodes::~P_ModListCodes()
{
  destroy();
}
//-------------------------------------------------------------------
struct infoColumn
{
  LPCTSTR title;
  int sz;
};
//-------------------------------------------------------------------
static
infoColumn titleList[] = {
  { _T("Codice"), 80 },
  { _T("[1] Italiano"), 150 },
  { _T("[%d] "), 150 },
};
//-------------------------------------------------------------------
#define TOT_COLUMN SIZE_A(titleList)
#define LV_TXT_COLOR RGB(0, 0, 0xb3)
#define LV_BKG_COLOR RGB(0xf8, 0xe4, 0xb3)
//-------------------------------------------------------------------
bool P_ModListCodes::create()
{
  List = new PListView(this, IDC_LIST1, PRect(0, 0, 100, 100));
  List->Attr.style |= LVS_SINGLESEL | LVS_SHOWSELALWAYS;
  if(!baseClass::create())
    return false;
  List->setFont(D_FONT(20, 0, 0, _T("Arial Unicode MS")), true);

  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

  for(uint i = 0; i < MAX_LANG + 1; ++i) {
    lvc.iSubItem = i;
    if(!i) {
      lvc.pszText = (LPTSTR)getStringOrDef(8, titleList[i].title);
      lvc.cx = titleList[i].sz;
      }
    else if(i < SIZE_A(titleList) - 1) {
      lvc.pszText = (LPTSTR)titleList[i].title;
      lvc.cx = titleList[i].sz;
      }
    else {
      uint ix = SIZE_A(titleList) - 1;
      TCHAR buff[64];
      wsprintf(buff, titleList[ix].title, i);
      lvc.pszText = buff;
      lvc.cx = titleList[ix].sz;
      }
    lvc.fmt = LVCFMT_LEFT;
    if(-1 == ListView_InsertColumn(*List, i, &lvc))
      return false;
    }
  DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  ListView_SetExtendedListViewStyle(*List, dwExStyle);


  ListView_SetBkColor(*List, LV_BKG_COLOR);
  ListView_SetTextBkColor(*List, LV_BKG_COLOR);

  ListView_SetTextColor(*List, LV_TXT_COLOR);
  return true;
}
//-------------------------------------------------------------------
void P_ModListCodes::setNext()
{
  int nEl = ListView_GetItemCount(*List);
  if(!nEl)
    return;

  if(currSel >= nEl - 1) {
    chgSel(true);
    return;
    }

  ListView_SetItemState(*List, currSel + 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
  ListView_EnsureVisible(*List, currSel + 1, TRUE);
  chgSel();
}
//-------------------------------------------------------------------
void P_ModListCodes::setPrev()
{
  int nEl = ListView_GetItemCount(*List);
  if(!nEl)
    return;
  if(currSel <= 0) {
    chgSel(true);
    return;
    }

  ListView_SetItemState(*List, currSel - 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
  ListView_EnsureVisible(*List, currSel - 1, TRUE);
  chgSel();
}
//-------------------------------------------------------------------
void P_ModListCodes::setTitle(PVect<LPTSTR>& allTitle)
{
  LVCOLUMN lvc;
  ZeroMemory(&lvc, sizeof(lvc));
  lvc.mask = LVCF_TEXT;
  freeList& SetFree = getSetFree();
  for(int i = 0; i < MAX_LANG; ++i) {
    lvc.pszText = allTitle[i];
    ListView_SetColumn(*List, i + 1, &lvc);
    SetFree.release(allTitle[i]);
    }
}
//-------------------------------------------------------------------
void P_ModListCodes::setText(PVect<LPTSTR>& allText)
{
  if(currSel < 0)
    return;
  LVITEM lvi;
  ZeroMemory(&lvi, sizeof(lvi));
  lvi.mask = LVIF_TEXT;
  lvi.iItem = currSel;
  freeList& SetFree = getSetFree();

  for(uint i = 0; i < MAX_LANG; ++i) {
    lvi.iSubItem = i + 1;
    lvi.pszText = (LPTSTR)allText[i];

    ListView_SetItem(*List, &lvi);
    SetFree.release(allText[i]);
    }
  Dirty = true;
}
//-------------------------------------------------------------------
void P_ModListCodes::getText(PVect<LPTSTR>& allText)
{
  if(currSel < 0)
    currSel = 0;
  LVITEM item;
  item.mask = LVIF_TEXT;
  item.cchTextMax = SIZE_SINGLE_BUFF;
  item.iItem = currSel;
  freeList& SetFree = getSetFree();
  for(int i = 0; i < MAX_LANG; ++i) {
    item.iSubItem = i + 1;
    allText[i] = SetFree.get();
    item.pszText = allText[i];
    ListView_GetItem(*List, &item);
    }
}
//-------------------------------------------------------------------
void P_ModListCodes::refresh()
{
  currSel = -1;
  chgSel(true);
  SetFocus(*List);
  ListView_SetItemState(*List, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x00ff);
}
//-------------------------------------------------------------------
void P_ModListCodes::chgSel(bool force)
{
  int curr = ListView_GetNextItem(*List, -1, LVNI_SELECTED);

  if(!force && (curr < 0 || curr == currSel))
    return;
  P_ModEdit* med = getEditCode(this);
  med->changedSel();

  currSel = curr;
  PVect<LPTSTR> buff;
  getText(buff);

  med->setText(buff);
}
//-------------------------------------------------------------------
LRESULT P_ModListCodes::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_NOTIFY:
      switch(((NMHDR*)lParam)->code) {
        case LVN_ITEMACTIVATE:
        case LVN_ITEMCHANGED:
          chgSel();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
void P_ModListCodes::performFillText(PVect<LPTSTR>& allText, const fillInfo& fInfo)
{
  LPTSTR buffS = allText[fInfo.source];
  for(uint i = 0; i < MAX_LANG; ++i) {
    if(i != fInfo.source && (fInfo.target & (1 << i))) {
      if(!allText[i][0] || (fInfo.action & 2)) {
        _tcscpy_s(allText[i], SIZE_SINGLE_BUFF, buffS);
        Dirty = true;
        }
      }
    }
}
//-------------------------------------------------------------------
void P_ModListCodes::fillField(const fillInfo& fInfo)
{
  int nEl = ListView_GetItemCount(*List);
  if(!nEl)
    return;
  int oldCurr = currSel;
  PVect<LPTSTR> allText;
  for(currSel = 0; currSel < nEl; ++currSel) {
    getText(allText);
    performFillText(allText, fInfo);
    setText(allText);
    }
  currSel = oldCurr;
  ListView_SetItemState(*List, currSel, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
  ListView_EnsureVisible(*List, currSel, TRUE);
  chgSel(true);
}
//-------------------------------------------------------------------
