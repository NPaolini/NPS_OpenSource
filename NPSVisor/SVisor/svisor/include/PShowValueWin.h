//-------- PShowValueWin.h ----------------------------------------------
//-------------------------------------------------------------------
#ifndef PShowValueWin_H
#define PShowValueWin_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pFullView.h"
#include "pListview.h"
#include "pVarTrend.h"
//-------------------------------------------------------------------
class PShowValueWin : public pFullView
{
  private:
    typedef pFullView baseClass;
  public:
    PShowValueWin(PWin* parent, const PVect<infoVar>& varName, const infoTrend& iTrend,
        DWORD bitShow, LPCTSTR title = 0, HINSTANCE hInst = 0) :
        baseClass(parent, title, hInst), Left(0), Right(0),
      varName(varName), iTrend(iTrend), bitShow(bitShow) { }

    PShowValueWin(PWin * parent, uint id, const PVect<infoVar>& varName, const infoTrend& iTrend,
        DWORD bitShow, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst), Left(0), Right(0),
      varName(varName), iTrend(iTrend), bitShow(bitShow) { }

    PShowValueWin(PWin * parent, uint id, const PVect<infoVar>& varName, const infoTrend& iTrend,
        DWORD bitShow, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst), Left(0), Right(0),
      varName(varName), iTrend(iTrend), bitShow(bitShow) { }

      ~PShowValueWin();
    bool create();

    void setInfo(PacketInfo* packet);
  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HWND getHWClient() { return *List; }

    PListView* List;
  private:
    const PVect<infoVar>& varName;
    const infoTrend& iTrend;
    DWORD bitShow;
    PacketInfo* Left;
    PacketInfo* Right;

    void release(PacketInfo** packet);
};
//-------------------------------------------------------------------
#endif
