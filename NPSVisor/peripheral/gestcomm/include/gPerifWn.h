//------------ gPerifWn.h ----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef GPERIFWN_H_
#define GPERIFWN_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "commwn.h"
//----------------------------------------------------------------------------
class TgPerif_Window : public gestCommWindow {
  public:
    TgPerif_Window(LPCTSTR title = 0, HINSTANCE hInst = 0);
    virtual ~TgPerif_Window();

  protected:
    virtual LRESULT processSVisorMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void processSVisorMsgEx(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, DWORD id);
};
//----------------------------------------------------------------------------
#endif

