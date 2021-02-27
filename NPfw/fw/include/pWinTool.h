//-------------------- pWinTool.h ---------------------------
//-----------------------------------------------------------
#ifndef PWINTOOL_H_
#define PWINTOOL_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pWinToolBase.h"
//-----------------------------------------------------------
#include "headerMsg.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class PWinTools : public PWinToolBase
{
  private:
    typedef PWinToolBase baseClass;
  public:
    PWinTools(PWin * parent, struct PToolBarInfo *info, LPCTSTR title,
            UINT idBkg, HINSTANCE hInstance);
    virtual ~PWinTools();
    void enableBtn(int id, bool enable);
    void selectBtn(int id, bool sel = true);
    void setHotItem(int id);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual LPCTSTR getClassName() const;
    virtual LPTSTR getTips(uint id);
  private:
    void createToolbar();
    class PToolBar *tools;
//    PBitmap Bkg;
//    bool active;
};
//-----------------------------------------------------------
//inline LPCTSTR PWinTools::getClassName() const { return WC_DIALOG; }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
