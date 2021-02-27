//----------- winList.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef winList_H_
#define winList_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_util.h"
//----------------------------------------------------------------------------
struct win_info
{
  HWND hwnd;
  LPCTSTR caption;
  win_info(HWND h, LPCTSTR c) : hwnd(h), caption(str_newdup(c)) {}
  win_info() : hwnd(0), caption(0) {}

  const win_info& operator =(const win_info& other) { clone(other); return *this; }
  win_info(const win_info& other) : caption(0) { clone(other); }

  ~win_info() { delete []caption; }
  bool operator < (const win_info& other) { return _tcsicmp(caption, other.caption) < 0; }
  private:
    void clone(const win_info& other);
};
//----------------------------------------------------------------------------
typedef PVect<win_info> setOfInfo;
//----------------------------------------------------------------------------
struct winList
{
  HWND owner;
  setOfInfo set;
};
//----------------------------------------------------------------------------
bool getListHwnd(winList& WList);
//----------------------------------------------------------------------------
#endif
