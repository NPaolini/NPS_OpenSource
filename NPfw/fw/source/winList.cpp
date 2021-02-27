//----------- winList.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "winList.h"
//----------------------------------------------------------------------------
void win_info::clone(const win_info& other)
{
  if(&other != this) {
    delete []caption;
    caption = str_newdup(other.caption);
    hwnd = other.hwnd;
    }
}
//----------------------------------------------------------------------------
static bool existInList(HWND hwnd, const setOfInfo& set)
{
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i)
    if(set[i].hwnd == hwnd)
      return true;
  return false;
}
//----------------------------------------------------------------------------
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  winList* wl = reinterpret_cast<winList*>(lParam);
  if(hwnd != wl->owner && IsWindowVisible(hwnd)) {
    HWND parent = GetAncestor(hwnd, GA_ROOTOWNER);
    if(!parent)
      parent = hwnd;
    const setOfInfo& set = wl->set;
    if(!existInList(parent, set)) {
      TCHAR caption[500];
      GetWindowText(parent, caption, SIZE_A(caption));
      if(*caption) {
        if(_tcsicmp(_T("Program Manager"), caption) && _tcsicmp(_T("start"), caption)) {
          int nElem = wl->set.getElem();
          wl->set[nElem] = win_info(parent, caption);
          }
        }
      }
    }
  return TRUE;
}
//------------------------------------------------------------------------------
bool getListHwnd(winList& WList)
{
  bool success = toBool(EnumWindows(EnumWindowsProc, (LPARAM)&WList));
  if(success)
    WList.set.sort();

  return success;
}
//----------------------------------------------------------------------------
