//------------------ PFilter.cpp -----------------------------
//------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "pFilter.h"
//----------------------------------------------------------------------------
void myBinaryFilter::initCtrl(HWND ctrl)
{
  SendMessage(ctrl, EM_SETLIMITTEXT, 8, 0);
}
//----------------------------------------------------------------------------
void myHexFilter::initCtrl(HWND ctrl)
{
  SendMessage(ctrl, EM_SETLIMITTEXT, 2, 0);
}
//----------------------------------------------------------------------------
void myNumbFilter::initCtrl(HWND ctrl)
{
  SendMessage(ctrl, EM_SETLIMITTEXT, 3, 0);
}
//----------------------------------------------------------------------------
bool myNumbFilter::accept(WPARAM& key, HWND ctrl)
{
  if(!baseClass::accept(key, ctrl))
    return false;

  int len = SendMessage(ctrl, WM_GETTEXTLENGTH, 0, 0);
  if(len > 1) {
    TCHAR buff[5];
    GetWindowText(ctrl, buff, 5);
    int init;
    int end;
    if(!getSel(ctrl, init, end))
      return false;
    TCHAR t[5];
    memset(t, 0, sizeof(t));
    for(int i = 0; i < init; ++i)
      t[i] = buff[i];
    t[init] = key;

    for(int i = init + 1, j = end; buff[j]; ++i, ++j)
      t[i] = buff[j];
    if(_ttoi(t) > 255)
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
