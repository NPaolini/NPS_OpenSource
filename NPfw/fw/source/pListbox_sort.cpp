//-------------- pListboxSort.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pListbox_sort.h"
#include "p_Util.h"
//----------------------------------------------------------------------------
int lbComp::compare_(LPCTSTR p1, LPCTSTR p2, int dim)
{
  int i = 0;
  for(; i < dim; ++i)
    if(p1[i] > _T(' '))
      break;
  int j = 0;
  for(; j < dim; ++j)
    if(p2[j] > _T(' '))
      break;
  if(caseSens) {
    for(; i < dim && j < dim; ++i, ++j) {
      int diff = _totlower(p1[i]) - _totlower(p2[j]);
      if(diff > 0)
        return 1;
      if(diff < 0)
        return -1;
      }
    }
  else {
    for(; i < dim && j < dim; ++i, ++j) {
      if(p1[i] > p2[j])
        return 1;
      if(p1[i] < p2[j])
        return -1;
      }
    }
  if(i < dim)
    return 1;
  if(j < dim)
    return -1;
  return 0;
}
//-----------------------------------------------------------
pListboxSort::pListboxSort(PWin * parent, uint id, int x, int y, int w, int h, int /*len*/, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, 0, hinst), pfl(0), Tot(0)
{
    Attr.style = LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_SORT |
                 LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE |
//                 WS_BORDER |
                 WS_VSCROLL | WS_HSCROLL | WS_TABSTOP;
    Attr.exStyle |= WS_EX_CLIENTEDGE;
}
//-----------------------------------------------------------
pListboxSort::pListboxSort(PWin * parent, uint id, const PRect& r, int /*len*/, HINSTANCE hinst) :
      baseClass(parent, id, r, 0, hinst),pfl(0), Tot(0)
{
    Attr.style = LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_SORT |
                 LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE |
//                 WS_BORDER |
                 WS_VSCROLL | WS_HSCROLL | WS_TABSTOP;
    Attr.exStyle |= WS_EX_CLIENTEDGE;
}
//-----------------------------------------------------------
pListboxSort::~pListboxSort ()
{
  destroy();
  delete pfl;
}
//----------------------------------------------------------------------------
LPTSTR pListboxSort::getFree() { return pfl ? (LPTSTR)pfl->getFree() : 0; }
//----------------------------------------------------------------------------
void pListboxSort::releaseFree(LPTSTR p) { pfl ? pfl->addToFree(p) : 0; }
//----------------------------------------------------------------------------
bool pListboxSort::retrieveBuff(DRAWITEMSTRUCT FAR& drawInfo)
{
  if(!drawInfo.itemData)
    return baseClass::retrieveBuff(drawInfo);
  int len = _tcslen((LPCTSTR)drawInfo.itemData);
  if(len > MaxLen) {
    MaxLen = len + 1;
    delete []Buff;
    Buff = new TCHAR[MaxLen+2];
    }
  copyStrZ(Buff, (LPCTSTR)drawInfo.itemData);
  return true;
}
//----------------------------------------------------------------------------
void pListboxSort::SetTabStop(int ntabs, int *tabs, int *show)
{
  Tot = 0;
  for(int i = 0; i < ntabs; ++i)
    Tot += tabs[i] + 1;
  delete pfl;
  pfl = new P_freeList(Tot + 2);
  baseClass::SetTabStop(ntabs, tabs, show);
}
//----------------------------------------------------------------------------
LRESULT pListboxSort::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case LB_DELETESTRING:
      freeCurrData(hwnd, wParam);
      break;
    case LB_ADDSTRING:
      do {
        LPTSTR p = getFree();
        if(p) {
          copyStrZ(p, (LPCTSTR)lParam, Tot);
          lParam = (LPARAM)p;
          }
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void pListboxSort::freeCurrData(HWND hwnd, INT_PTR ix)
{
  if(ix < 0)
    return;
  LPTSTR p = (LPTSTR)SendMessage(hwnd, LB_GETITEMDATA, ix, 0);
  if(p)
    releaseFree(p);
}
//-----------------------------------------------------------
