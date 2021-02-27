//-------- PVarListBox.h -----------------------------------------------------
#ifndef PVARLISTBOX_H_
#define PVARLISTBOX_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

#ifndef PLISTBOX_H_
  #include "pListBox.h"
#endif
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class PVarListBox : public PListBox, virtual public baseActive
{
  public:
    PVarListBox(P_BaseBody* owner, uint style, uint id, const PRect& rect,
              int textlen = 255, HINSTANCE hinst = 0);
    virtual ~PVarListBox();

    virtual bool allocObj(LPVOID param = 0)  { return true; }
    bool setFocus(pAround::around where);

    virtual void draw(HDC hdc, const PRect& rect) {}
    virtual void setVisibility(bool set);
    HWND setListBoxCallBack(infoCallBackSend* fzInfo, uint forWinProc);

    // necessaria per passare messaggi gestiti dalla parent window alla callback
    // per ora i messaggi inviati sono: LBN_SELCHANGE, LBN_DBLCLK
    virtual void executeWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
      windowProc(hwnd, message, wParam, lParam);
    }
  protected:
    bool preProcessMsg(MSG& msg);
    virtual void performDraw(HDC hdc) {};
    virtual uint getBaseId() const { return ID_INIT_VAR_LBOX; }
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HWND getHwnd() { return getHandle(); }
    virtual bool evKeyDown(WPARAM& key);
    bool evSetAroundFocus(UINT key);
    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
    virtual void paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo);
    virtual void paintCustom(HDC hDC, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);

    callBackLB FzCallBack;
    callBackLB FzCallBackWinProc;
    callBackLB FzCallBacPreProcess;

    LPVOID callBackOwner;

    bool preProcessMsgStd(MSG& msg);
    LRESULT windowProcStd(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

