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
//#define SIZE_BTN_H 90
#define SIZE_BTN_V 60
#define SIZE_BTN_H 80
//#define SIZE_BTN_V 80
//-------------------------------------------------------------------
#define WM_CUSTOM_BY_BTN WM_FW_FIRST_FREE
  #define CM_BTN_FOLDER   91
  #define CM_BTN_ADD      92
  #define CM_BTN_REM      93
  #define CM_BTN_COPY     94
  #define CM_BTN_PASTE    95
  #define CM_BTN_UPD      96
  #define CM_BTN_SAVE     97
  #define CM_BTN_EXPORT   98
//  #define CM_BTN_IMPORT   99
  #define CM_BTN_DONE     100
  #define ICD_CHECK_DATE  102


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

    // 0 -> abilita tutto, 2 -> tutto meno salva, 4 -> tutto meno ricarica, 8 -> tutto meno copia
    // 16 -> tuuto meno incolla
    void enableDisable(uint flag);

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    uint idTimer;
};
//-------------------------------------------------------------------
#endif
