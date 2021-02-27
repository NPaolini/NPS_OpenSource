//-------------- PCustomListViewFullEdit.cpp --------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "PCustomListViewFullEdit.h"
#include "PEdit.h"
#include "P_util.h"
//-----------------------------------------------------------
int PCustomListViewFullEdit::iItem = -1;    // Currently selected item index
int PCustomListViewFullEdit::iSubItem = 0;  // Currently selected subitem index
PRect PCustomListViewFullEdit::rcSubItem;   // RECT  of selected subitem
#if 1
//-----------------------------------------------------------
PWin* PCustomListViewFullEdit::wEdit;
//-----------------------------------------------------------
class winEdit : public PEdit
{
  private:
    typedef PEdit baseClass;
  public:
    winEdit(HWND hwnd, HINSTANCE hInst) :  baseClass(hwnd, hInst) {}
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

};
//-----------------------------------------------------------
LRESULT winEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_WINDOWPOSCHANGING:
      do {
        WINDOWPOS * pos = (WINDOWPOS*)lParam;
        PRect r = PCustomListViewFullEdit::getrcSubItem();
        // vanno ridefiniti tutti i valori, altrimenti, se il primo subitem è nascosto, assumono valori errati
        pos->x = r.left;
        pos->cx = r.right;
        pos->y = r.top;
        pos->cy = r.bottom;

        return TRUE;
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
PWin* PCustomListViewFullEdit::makeWinEdit(HWND hwnd)
{
  PWin* w = new winEdit(hwnd, getHInstance());
  w->setParent(this);
  w->create();
  return w;
}
//-----------------------------------------------------------
#else
//-----------------------------------------------------------
WNDPROC PCustomListViewFullEdit::wpOrigEditProc;
HWND PCustomListViewFullEdit::hwndEdit;     // handle of hacked edit control
//-----------------------------------------------------------
LRESULT APIENTRY EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
    case WM_WINDOWPOSCHANGING:
      do {
        WINDOWPOS * pos = (WINDOWPOS*)lParam;
        // vanno ridefiniti tutti i valori, altrimenti, se il primo subitem è nascosto, assumono valori errati
        pos->x = PCustomListViewFullEdit::rcSubItem.left;
        pos->cx = PCustomListViewFullEdit::rcSubItem.right;
        pos->y = PCustomListViewFullEdit::rcSubItem.top;
        pos->cy = PCustomListViewFullEdit::rcSubItem.bottom;

        return TRUE;
        } while(false);
      break;
    }
  return CallWindowProc(PCustomListViewFullEdit::wpOrigEditProc, hwnd, uMsg, wParam, lParam);
}
//-----------------------------------------------------------
#endif
//-----------------------------------------------------------
#ifndef  LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER     0x00010000
#endif
bool PCustomListViewFullEdit::create()
{
  if(!baseClass::create())
    return false;
  if(isFlagSet(pfFromResource)) {
    PRect r;
    GetWindowRect(*this, r);
    MapWindowPoints(HWND_DESKTOP, *getParent(), (LPPOINT)(RECT*)r, 2);
    HWND prev = GetNextWindow(*this, GW_HWNDPREV);
    // forza per sicurezza, se non è stato impostato nel file di risorse
    LONG_PTR val = GetWindowLongPtr(*this, GWL_STYLE) | LVS_EDITLABELS | LVS_SHOWSELALWAYS;
    LONG_PTR exVal = GetWindowLongPtr(*this, GWL_EXSTYLE);
    destroy();
    Attr.x = r.left;
    Attr.y = r.top;
    Attr.w = r.Width();
    Attr.h = r.Height();
    clearFlag(pfFromResource);
    Attr.style |= val;
    Attr.exStyle |= exVal;
    if(baseClass::create()) {
      if(prev)
        SetWindowPos(*this, prev, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      ListView_SetExtendedListViewStyleEx(*this, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
      return true;
      }
    return false;
    }
  ListView_SetExtendedListViewStyleEx(*this, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
  return true;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
LRESULT PCustomListViewFullEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      if(bEditing && iSubItem > 0) {
        PRect rcItem;
        if(ListView_GetItemRect(hwnd, iItem, rcItem, LVIR_LABEL))
          ValidateRect(hwnd, rcItem);
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void PCustomListViewFullEdit::onNm_Click(NMITEMACTIVATE* pia)
{
  if(canEditItem(pia->iItem, pia->iSubItem)) {
    iItem = pia->iItem;
    iSubItem = pia->iSubItem;
    }
}
//-----------------------------------------------------------
bool PCustomListViewFullEdit::ProcessNotify(LPARAM lParam, LRESULT& result)
{
  LPNMHDR lpnmHdr = (LPNMHDR)lParam;
  switch(lpnmHdr->code) {
    case NM_CLICK:
      onNm_Click((NMITEMACTIVATE*)lParam);
      break;

    case LVN_BEGINLABELEDIT:
      result = true;
      if(canEditItem(iItem, iSubItem)) {
        NMLVDISPINFO *pdi = (NMLVDISPINFO*) lParam;
        PRect rc;

        result = false;
        if (iSubItem >= 0) {
          bEditing = true;

          ListView_GetItemRect(*this, iItem, rc, LVIR_LABEL);
          ValidateRect(*this, rc);

          HWND hwndEdit = ListView_GetEditControl(*this);
          delete wEdit;
          wEdit = makeWinEdit(hwndEdit);
          ListView_GetSubItemRect(*this, iItem, iSubItem, LVIR_LABEL, (LPARAM)(LPRECT)rc);
          rcSubItem = rc;
          rcSubItem.right = rc.Width();
          rcSubItem.bottom = rc.Height() + GetSystemMetrics(SM_CYBORDER) * 3;
          TCHAR szText[1024] = _T("");
          ListView_GetItemText(*this, iItem, iSubItem, szText, SIZE_A(szText) - 1);
          personalizeEdit(iItem, iSubItem, szText, SIZE_A(szText));
          SetWindowText(hwndEdit, szText);
          }
        }
      return true;

    case LVN_ENDLABELEDIT:
      result = false;
      do {
        NMLVDISPINFO * lvdisp = (NMLVDISPINFO*)lParam;
        LVITEM * lvItem = &lvdisp->item;
        if(iSubItem >= 0) {
          if(lvItem->pszText) {
            TCHAR szText[1024] = _T("");
            copyStrZ(szText, lvItem->pszText);
            if(personalizeEditEnd(iItem, iSubItem, szText, SIZE_A(szText)))
              Dirty = true;
            ListView_SetItemText(*this, iItem, iSubItem, szText);
            }
          if(SendMessage(*wEdit, EM_GETMODIFY, 0, 0))
            Dirty = true;
          delete wEdit;
          wEdit = 0;
          iSubItem = 0;
          bEditing = false;
          }
        else
          result = true;
        return true;  // set result to true to indicate that the text can be set, to false otherwise
        } while(false);
      break;
    }
  return baseClass::ProcessNotify(lParam, result);
}
