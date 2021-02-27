//------ systray.h -----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef systray_H_
#define systray_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
#include <objbase.h>
#include <Shellapi.h>
#include <Shlwapi.h>
//----------------------------------------------------------------------------
//#include <stdio.h>
#include "1.h"
#include "p_txt.h"
//----------------------------------------------------------------------------
#define ID_TIMER_CHECK_TRAY 999
//----------------------------------------------------------------------------
struct textInfo;
class mainClient;
class mainWin;
//----------------------------------------------------------------------------
#define MAX_TIPS_LEN 80
//----------------------------------------------------------------------------
class tipsObj
{
  public:
    tipsObj(LPCTSTR row = 0) : Level(0), prph(0), addr(0), type(0), nbit(0), offs(0), oldVal(0)
          { ZeroMemory(Text, sizeof(Text)); load(row); }
    ~tipsObj() {}

    LPCTSTR getText() { return Text; }
    int getLevel() { return Level; }

    enum infoActive { none, noChangedActive, changedRemoved, changedActive };
    infoActive needAdd(mainClient* mc, bool force);

  private:
    TCHAR Text[MAX_TIPS_LEN];
    uint Level;

    uint prph;
    uint addr;
    uint type;
    uint nbit;
    uint offs;

    uint oldVal;

    void load(LPCTSTR row);
};
//----------------------------------------------------------------------------
class manTipsObj
{
  public:
    manTipsObj(mainClient* owner);
    ~manTipsObj() {}

    LPCTSTR getNormalString()  { return normalString; }
    LPCTSTR getTitle()  { return title; }
//    PVect<tipsObj>& getObject() { return tObj; }

    uint getElem() { return tObj.getElem(); }
    LPCTSTR getText(uint ix) { return tObj[ix].getText(); }
    int getLevel(uint ix) { return tObj[ix].getLevel(); }

    // torna il level max
    int makeString(LPTSTR text, uint dim, bool force);
    bool checkChanged(bool force);
  private:
    mainClient* Owner;
    PVect<tipsObj> tObj;
    TCHAR title[MAX_TIPS_LEN];
    TCHAR normalString[MAX_TIPS_LEN];
};
//----------------------------------------------------------------------------
class sysTray
{
  public:
    sysTray(mainWin* owner, manTipsObj& m_obj);
    ~sysTray() { removeSysTray(); }

    bool windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result);

  private:
    mainWin* Owner;
    NOTIFYICONDATA niData;
    manTipsObj& mObj;

    void gotoSysTray(int level); // -1 per refresh da system
    void removeSysTray();
    void setBaloon();
    void setTips();
    void check();
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
