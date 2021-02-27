//-------------------- pWinToolBase.h -----------------------
//-----------------------------------------------------------
#ifndef PWINTOOLBASE_H_
#define PWINTOOLBASE_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pBitmap.h"
//-----------------------------------------------------------
#include "headerMsg.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class PWinToolBase : public PWin
{
    typedef PWin baseClass;
  public:
    PWinToolBase(PWin * parent, LPCTSTR title, UINT idBkg, HINSTANCE hInstance);
    virtual ~PWinToolBase();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR getClassName() const;
  private:
    PBitmap Bkg;
    bool active;
};
//-----------------------------------------------------------
inline LPCTSTR PWinToolBase::getClassName() const { return WC_DIALOG; }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
