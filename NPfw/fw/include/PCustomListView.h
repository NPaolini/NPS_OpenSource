//-------------- PCustomListView.h --------------------------
//-----------------------------------------------------------
#ifndef PCustomListView_H_
#define PCustomListView_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "PListView.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PCustomListView : public PListView
{
  private:
    typedef PListView baseClass;
  public:
    PCustomListView(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, text, hinst) {  }

    PCustomListView(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, text, hinst) {  }

    PCustomListView(PWin * parent, uint resid, HINSTANCE hinst = 0) :
        baseClass(parent, resid, hinst) {  }


    ~PCustomListView() { destroy(); }
    bool create();

    virtual bool ProcessNotify(LPARAM lParam, LRESULT& result) { return false; }

    virtual bool getColorBkg(COLORREF& c) { return false; }
    virtual bool getColorTextBkg(COLORREF& c) { return false; }
    virtual bool getColorText(COLORREF& c) { return false; }
    virtual bool getColorTextBkgSel(COLORREF& c) { return false; }
    virtual bool getColorTextSel(COLORREF& c) { return false; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
