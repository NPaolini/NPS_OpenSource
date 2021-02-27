//-------------------- pManTimer.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pManTimer.h"
#include "mainwin.h"
#include "p_util.h"
//-------------------------------------------------------------------
static int getTimeTimer(int type)
{
  TCHAR time[64];
  getKeyPath(MAX_TIME_TIMER, time);
  int t[3] = { 0 };
  LPCTSTR p = time;
  for (uint i = 0; i < SIZE_A(t); ++i) {
    if (!p)
      break;
    t[i] = _ttoi(p);
    p = findNextParamTrim(p);
  }
  switch (type) {
    case -1:
      return t[0] * 10000 + t[1] * 100 + t[2];
    case 0:
      return t[0];
    case 1:
      return t[1];
    case 2:
      return t[2];
    }
  return 0;
}
//-----------------------------------------------------------
void manageTimer::calcNow()
{
  int h = currVal / 10000;
  int m = (currVal / 100) % 100;
  int s = currVal % 100;
  if(--s < 0) {
    s = 59;
    if(--m < 0) {
      m = 60;
      s = 0;
      if(--h < 0) {
        h = 0;
        m = 0;
        }
      }
    }
  currVal = h * 10000 + m * 100 + s;
}
//-----------------------------------------------------------
void manageTimer::reset()
{
  currVal = getTimeTimer(-1);
}
//-----------------------------------------------------------
void manageTimer::addRefresh(PWin* w)
{
  setRefresh[setRefresh.getElem()] = w;
}
//-----------------------------------------------------------
void manageTimer::refreshAll()
{
  uint nElem = setRefresh.getElem();
  for(uint i = 0; i < nElem; ++i)
    refresh(i);
}
//-----------------------------------------------------------
void manageTimer::refresh(uint ix)
{
  InvalidateRect(*(setRefresh[ix]), 0, 0);
}
