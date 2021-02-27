//-------- pMenuBtn.h ----------------------------------------------
//-------------------------------------------------------------------
#ifndef pMenuBtn_H
#define pMenuBtn_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
extern LPCTSTR getStringOrDef(uint code, LPCTSTR def);
//-------------------------------------------------------------------
#include "fullView.h"
#include "headerMsg.h"
//-------------------------------------------------------------------
#define OFFS_BTN 5
#define SIZE_BTN 72
//-------------------------------------------------------------------
#define WM_CUSTOM_BY_BTN WM_FW_FIRST_FREE
  #define CM_BTN_DONE     1
  #define CM_BTN_UPD      2
  #define CM_BTN_FOLDER   3
  #define CM_BTN_SAVE     4
  #define CM_BTN_EXPORT   5
  #define CM_BTN_IMPORT   6

  #define CM_BTN_FULL_SCREEN 1000
//-------------------------------------------------------------------
class pMenuBtn : public fullView
{
  private:
    typedef fullView baseClass;
  public:
    pMenuBtn(PWin* parent, LPCTSTR title = 0, HINSTANCE hInst = 0) :
        baseClass(parent, title, hInst), idTimer(0) { }

    pMenuBtn(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst), idTimer(0) { }

    pMenuBtn(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst), idTimer(0) { }
    ~pMenuBtn() { destroy(); }

    virtual bool create();
    bool saveOnUnicode();
    void setUnicode();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    uint idTimer;
};
//-------------------------------------------------------------------
#endif
