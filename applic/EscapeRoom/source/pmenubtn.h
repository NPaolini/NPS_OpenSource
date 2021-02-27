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
#include "PSliderWin.h"
#include "PComboBox.h"
#include "PEdit.h"
//-------------------------------------------------------------------
#define OFFS_BTN 5
#define SIZE_BTN_V 60
#define SIZE_BTN_H 76
#define SIZE_SLIDER_H 130
//-------------------------------------------------------------------
#define WM_CUSTOM_BY_BTN WM_FW_FIRST_FREE
    #define CM_BTN_SETUP   91
    #define CM_BTN_FULL_SCREEN      92
    #define CM_BTN_MOVE_SCREEN      93
    #define CM_BTN_AUDIO_BASE     94
    #define CM_BTN_AUDIO_ALERT    95
    #define CM_BTN_INFO      96
    #define CM_BTN_RESET_TIME     97
    #define CM_BTN_TIMER_PAUSE_RESUME   98

  #define CM_BTN_DONE     100
  #define ICD_SLIDER      102

  #define IDC_CBX_SOUND   110
  #define IDC_CBX_ALERT   111
  #define IDC_CBX_TIPS    112
  #define IDC_BTN_TIPS    113

#define OFFS_ROW 8
#define SIZE_ROW 24
#define SIZE_CBX_H 180
//-------------------------------------------------------------------
class pMenuBtn : public fullView
{
  private:
    typedef fullView baseClass;
  public:
    pMenuBtn(PWin* parent, LPCTSTR title = 0, HINSTANCE hInst = 0) :
        baseClass(parent, title, hInst), idTimer(0), Slider(0) { }

    pMenuBtn(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst), idTimer(0), Slider(0) { }

    pMenuBtn(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst), idTimer(0), Slider(0) { }
    ~pMenuBtn() { destroy(); }

    virtual bool create();

    PSliderWin* getSlider() { return Slider; }

    bool refreshCbx(uint type, LPTSTR audio);
    void getCurrSel(LPTSTR buffer, uint type); // 0 -> audio, 1 -> effect, 2 -> tips
    int getCurrSel(uint type); // 0 -> audio, 1 -> effect, 2 -> tips
    void setCurrSel(int ix, uint type);
    bool saveCurrTips();
    void reload(uint type);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    uint idTimer;
    PSliderWin* Slider;
    PComboBox* cbxSound;
    PComboBox* cbxAlert;
    PComboBox* cbxTips;
    void fillCbxSound();
    void fillCbxAlert();
    void fillCbxTips();
};
//-------------------------------------------------------------------
#endif
