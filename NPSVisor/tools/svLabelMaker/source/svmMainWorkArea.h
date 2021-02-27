//------------------ svmMainWorkArea.h ------------------------
//-----------------------------------------------------------
#ifndef svmMainWorkArea_H_
#define svmMainWorkArea_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "headerMsg.h"
#include <stdlib.h>
#include <commctrl.h>
#include "svmManZoom.h"
#include "pregistry.h"
#include "pWinTool.h"
#include "pToolBar.h"
#include "pStatic.h"
#include "POwnBtn.h"
#include "macro_utils.h"
#include "p_Txt.h"
#include "svmRule.h"
//-----------------------------------------------------------
#include "PscrollContainer.h"
#define ID_CLIENT_SCROLL 110
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//-----------------------------------------------------------
extern void g_setZoom(PWin* child, svmManZoom::zoomX zoom, bool force = false);
extern void g_setStartX(PWin* child, long v);
extern void g_setStartY(PWin* child, long v);
//-----------------------------------------------------------
class childScrollWorkArea : public PclientScrollWin
{
  private:
    typedef PclientScrollWin baseClass;
  public:
    childScrollWorkArea(PWin * parent, uint id, const PRect& rect, HINSTANCE hInstance = 0);
    virtual ~childScrollWorkArea();

    virtual bool create();

    virtual void setHPos(uint first);
    virtual void setVPos(uint first);
    virtual void setScrollPos(uint x, uint y);
    virtual void resetScroll();
    void setZoom(svmManZoom::zoomX zoom, bool force = false);
    svmManZoom::zoomX getZoom() const { return Zoom; }
    bool idle(DWORD count);
    UINT getIncrementGlobalId();

    bool isUsedFontId(uint idfont);
    void getUsedFontId(PVect<uint>& set);
    void decreaseFontIfAbove(uint id_font);
    bool resetFontIfAbove(uint id_font);
    bool isFontAbove(uint id_font);


  protected:
    virtual LPCTSTR getClassName() const { return _T("svmWorkArea_ClassName"); }
    virtual void getWindowClass(WNDCLASS& wcl);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType) { return 0; }
    svmManZoom::zoomX Zoom;

    class PWinTools* Tools;
    class PWinTools* Actions;
    class PWinTools* Page;
    class PWinTools* chgPage;
    class PWinToolsCoord* Coord;
    void enableTools(bool set);
    PRect frame;

    HWND hwndTips;
    TCHAR tipsText[1024];
    void createTips();
    void manageDispInfo(LPTOOLTIPTEXT info);

    int currTools;
    bool multipleAddTools;

    LPCTSTR baseCursor;
    UINT currAction;
    bool actionLock;
    bool actionHide;
    POINT ptMoveByKey;
    // vale 1 se movimento, 2 se resize, 0 se nessuna azione
    int isMovingByKey;
    bool onExit;

    bool onPan;
    mdcCustom Mdc;
    void evPaint(HDC hdc, const PRect& rect);
  private:
    UINT getCurrPage();
    void setCurrPage(UINT n);
    void setPageTitle();
    void setBtnStat();
    HDC getDC();
    void releaseDC(HDC hdc);
    class svmBase* Base[MAX_PAGES];


    void evMouse(UINT message, WPARAM wParam, LPARAM lParam);
    void evMouseMove(UINT message, WPARAM wParam, LPARAM lParam);
    void evMouseLDown(UINT message, WPARAM wParam, LPARAM lParam);
    void evMouseLUp(UINT message, WPARAM wParam, LPARAM lParam);
    void evMouseRUp(UINT message, WPARAM wParam, LPARAM lParam);

    void beginXor(const POINT& pt);
    void drawXorBlock(HDC hdc, const POINT& pt);
    void drawXorBlockFinal(HDC hdc, const POINT& pt);
    void drawXorObject(HDC hdc, const POINT& pt);
    void drawXorObjectFinal(HDC hdc, const POINT& pt);

    void beginXorSizeByKey();
    void drawResizeAllObject(HDC hdc, const POINT& pt);
    void drawResizeAllObjectFinal(HDC hdc);

    void findObjects(const PRect& frame, int keyFlag);
    void showSelected(HDC hdc);
    void unselectAll(HDC hdc);

    class svmObject* addObject(HDC hdc, const POINT& pt, UINT currTools);
    int findPoint(HDC hdc, const POINT&pt, int keyFlag);

    void showAnchorType(const POINT& pt, int keyFlag);
    void setCursor(uint anchor, bool forceChange);

    bool okToDone(bool all = false);

    void performMoveByKey(int key);
    void performSizeByKey(int key);
    void performEndKey();

    void addRectObj(LPCTSTR p, const POINT& offset, bool useDim);
    void lockObject(bool lock);

    void setAddLock();
    void setAddHide();

    template <typename OBJ, int offs>
    void alloc_objects(setOfString& set, uint num, uint idInit, HDC hdc);

    bool canPan();

    void clearPage(bool all);

    bool openPage();
    void addLinked(LPCTSTR file);
    void open_Linked();
    bool savePage();
    bool performNew();
    void clear_Page();
    void setTransform(HDC hdc);
    void resetTransform(HDC hdc);

    void Align();
    void Order();
    void setActionHide();
    void setActionLock();

    bool loadFileTemplate(LPCTSTR file, LPTSTR dataPath, uint* idVars);
    bool saveFileTemplate(LPCTSTR fileTempl, LPCTSTR fileVars);

};
//-----------------------------------------------------------
#endif
