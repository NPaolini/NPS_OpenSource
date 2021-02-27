//------ PVarListView.cpp ----------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarListView.h"
#include "p_basebody.h"
#include "p_util.h"
//----------------------------------------------------------------------------
#define DEF_STYLE WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER | \
              LVS_NOSORTHEADER | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS
//----------------------------------------------------------------------------
PVarListView::PVarListView(P_BaseBody* owner, uint style, uint id, const PRect& rect,
              LPCTSTR text, HINSTANCE hinst) :
    PCustomListView(owner, id, rect, text, hinst), baseActive(owner, id)
{
  Attr.style = style;
  Attr.style |= DEF_STYLE;
  setRect(rect);
}
//----------------------------------------------------------------------------
PVarListView::~PVarListView()
{
  destroy();
}
//----------------------------------------------------------------------------
void PVarListView::setVisibility(bool set)
{
  baseActive::setVisibility(set);
//  if(!set)
    invalidateWithBorder();
}
//----------------------------------------------------------------------------
LRESULT PVarListView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_KEYDOWN:
      if(evKeyDown(wParam))
        return 0;
    }
  return PCustomListView::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static
bool isFirstRow(HWND hwnd)
{
  int currSel = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
  if(currSel < 0)
    currSel = ListView_GetItemCount(hwnd);
  return currSel <= 0;
}
//----------------------------------------------------------------------------
static
bool isLastRow(HWND hwnd)
{
  int currSel = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
  int count =  ListView_GetItemCount(hwnd);
  if(currSel < 0) {
    if(count)
      return false;
    return true;
    }

  return currSel == count - 1;
}
//----------------------------------------------------------------------------
bool PVarListView::evSetAroundFocus(UINT key)
{
  switch(key) {
    case VK_UP:
      if(isFirstRow(*this)) {
        Around.setFocus(pAround::aTop);
        return true;
        }
      break;
    case VK_DOWN:
      if(isLastRow(*this)) {
        Around.setFocus(pAround::aBottom);
        return true;
        }
      break;

    case VK_LEFT:
      Around.setFocus(pAround::aLeft);
      return true;

    case VK_RIGHT:
      Around.setFocus(pAround::aRight);
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool PVarListView::setFocus(pAround::around where)
{
  uint key[] = { VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN };
  return evSetAroundFocus(key[where]);
}
//----------------------------------------------------------------------------
bool PVarListView::evKeyDown(WPARAM& key)
{
  return evSetAroundFocus(key);
}
//----------------------------------------------------------------------------
