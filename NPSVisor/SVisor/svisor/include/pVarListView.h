//-------- PVarListView.h -----------------------------------------------------
#ifndef PVarListView_H_
#define PVarListView_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "PCustomListView.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class PVarListView : public PCustomListView, virtual public baseActive
{
  public:
    PVarListView(P_BaseBody* owner, uint style, uint id, const PRect& rect,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    virtual ~PVarListView();

    virtual bool allocObj(LPVOID param = 0)  { return true; }
    bool setFocus(pAround::around where);

    virtual void draw(HDC hdc, const PRect& rect) {}
    virtual void setVisibility(bool set);
  protected:
    virtual void performDraw(HDC hdc) {};
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HWND getHwnd() { return getHandle(); }
    virtual bool evKeyDown(WPARAM& key);
    bool evSetAroundFocus(UINT key);

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

