//-------------- PCustomListView.cpp ------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "PCustomListView.h"
//-----------------------------------------------------------
bool PCustomListView::create()
{
  if(!baseClass::create())
    return false;
  COLORREF c;
  if(getColorBkg(c))
    ListView_SetBkColor(*this, c);
  if(getColorTextBkg(c))
    ListView_SetTextBkColor(*this, c);
  if(getColorText(c))
    ListView_SetTextColor(*this, c);
  return true;
}
//----------------------------------------------------------------------------
LRESULT PCustomListView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_VSCROLL:
      switch(LOWORD(wParam)) {
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
          break;
        default:
          if(GetKeyState(VK_LBUTTON) & 0x8000)
            InvalidateRect(*this, 0, 1);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
/*
const int LVP_LISTITEM = 1;
const int LIS_NORMAL = 1;
const int LIS_HOT = 2;
const int LIS_SELECTED = 3;
const int LIS_DISABLED = 4;
const int LIS_SELECTEDNOTFOCUS = 5;
const int LVP_LISTGROUP = 2;
const int LVP_LISTDETAIL = 3;
const int LVP_LISTSORTEDDETAIL = 4;
const int LVP_EMPTYTEXT = 5;
*/
//-----------------------------------------------------------
/*
bool PCustomListView::ProcessNotify(LPARAM lParam, LRESULT& result)
{
  switch(((LPNMHDR)lParam)->code) {
    case NM_CUSTOMDRAW:
      do {
        LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW) lParam;
        if(!lplvcd)
          return -1;
        LRESULT result = CDRF_DODEFAULT;
#if 0 // sembra che uItemState sia sempre CDIS_SELECTED, finché non trovo dove non va meglio togliere il codice
        switch (lplvcd->nmcd.dwDrawStage) {
          case CDDS_PREPAINT:
            result = CDRF_NOTIFYITEMDRAW;
            break;
          case CDDS_ITEM | CDDS_ITEMPREPAINT:
//            result = CDRF_NOTIFYSUBITEMDRAW;
//            break;

//         case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
#if 0
            do {
              LVITEM lv;
              lv.iItem = lplvcd->nmcd.dwItemSpec;
              lv.mask = LVIF_STATE;
              if(ListView_GetItem(*this, &lv)) {
                if((LVIS_SELECTED | LVIS_FOCUSED) & lv.state) {
//              if(CDIS_CHECKED & lplvcd->nmcd.uItemState) {
//              if((CDIS_SELECTED  | CDIS_FOCUS) & lplvcd->nmcd.uItemState) {
//              if(lplvcd-> iStateId & (LIS_SELECTED | LIS_SELECTEDNOTFOCUS)) {
                  getColorTextSel(lplvcd->clrText);
                  getColorTextBkgSel(lplvcd->clrTextBk);
                  result = CDRF_NEWFONT;
                  }
                }
              } while(false);
#else
//          if(CDIS_CHECKED & lplvcd->nmcd.uItemState) {
//          if((CDIS_SELECTED  | CDIS_FOCUS) & lplvcd->nmcd.uItemState) {
          if(CDIS_SELECTED & lplvcd->nmcd.uItemState) {
            getColorTextSel(lplvcd->clrText);
            getColorTextBkgSel(lplvcd->clrTextBk);
            result = CDRF_NEWFONT;
            }
#endif
          break;
       }
#endif
      return result;
}
*/
/*
   case CDDS_PREPAINT:
      return CDRF_NOTIFYITEMDRAW;

   case CDDS_ITEMPREPAINT:
      return CDRF_NOTIFYSUBITEMDRAW;

   case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
*/
