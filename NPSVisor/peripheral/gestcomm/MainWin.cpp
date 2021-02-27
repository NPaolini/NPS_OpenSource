//--------------- MainWin.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "commidt.h"
#include "gperifwn.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class custom_Window : public TgPerif_Window
{
  public:
    custom_Window(LPCTSTR title = 0, HINSTANCE hInst = 0) : TgPerif_Window(title, hInst)
    {    }
};
//----------------------------------------------------------------------------
gestCommWindow* commIdentity::getMainWindow(LPCTSTR title, HINSTANCE hInst)
{
  return new custom_Window(title, hInst);
}


