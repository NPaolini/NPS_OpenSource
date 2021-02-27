//------------------ svmMainClient.h ------------------------
//-----------------------------------------------------------
#ifndef SVMMAINCLIENT_H_
#define SVMMAINCLIENT_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <commctrl.h>
//-----------------------------------------------------------
#include "PscrollContainer.h"
#include "pDialog.h"
#include "pEdit.h"
#include "pStatic.h"
#include "resource.h"
#include "pBitmap.h"

#include "p_util.h"
#include "pRect.h"
#include "pWinTool.h"
#include "p_Txt.h"
#include "common.h"
#include "svmDataProject.h"
#include "svmObject.h"
//-----------------------------------------------------------
#define BASE_TEMP_NAME _T(".\\tmp\\")
#define PAGE_PROP_NAME _T("pageProperty") PAGE_EXT
#define CUSTOM_NAME _T("custom") PAGE_EXT
#define SHARP _T("#")
//-----------------------------------------------------------
#define FIRST_CUSTOM_ID 1000000
//-----------------------------------------------------------
struct menuPopup
{
  uint flag;
  uint id;
  LPCTSTR text;
};
//-----------------------------------------------------------
int popupMenu(HWND hwnd, menuPopup* item, int num, POINT* pt = 0);
//-----------------------------------------------------------
bool performInitData(PWin* parent, dataProject& data, bool first);
//-----------------------------------------------------------
void makeNameImage(LPTSTR target, LPCTSTR name);
void makeRelNameImage(LPTSTR target, LPCTSTR name, bool duplicateSlash = false);
//-----------------------------------------------------------
P_File* makeFileTmpCustom(bool crypt);
void loadTmpCustom(setOfString& set);
//-----------------------------------------------------------
#define WM_SEND_DIM   (WM_FW_FIRST_FREE + 4)
#define WM_SEND_PAN   (WM_SEND_DIM + 1)
#define WM_HAS_SCROLL (WM_SEND_PAN + 1)
//-----------------------------------------------------------
struct dimWin
{
  PRect mainRect;
  PRect clientRect;
};
//-----------------------------------------------------------
class createGroupObject;
//-----------------------------------------------------------
class svmMainClient : public PclientScrollWin
{
  private:
    typedef PclientScrollWin baseClass;
  public:
    svmMainClient(PWin * parent, HINSTANCE hInstance);
    virtual ~svmMainClient();

    virtual bool create();

    virtual bool idle(DWORD count);
    // resa pubblica perché viene richiamata dalla classe per i bitmap non
    // abbinati a variabile
    void drawBmp(HDC hdc, LPCTSTR p);

    bool isBkgSelected() const { return actionToBkg; }

    UINT getIncrementGlobalId();
    void decrementGlobalId();

    void addNewObject(class svmObject* obj);

    void closeProgr();
    void resizeByTitle(bool repaint);
    void invalidate();

    bool isUsedFontId(uint idfont);
    void getUsedFontId(PVect<uint>& set);
    void decreaseFontIfAbove(uint id_font);
    bool resetFontIfAbove(uint id_font);
    bool isFontAbove(uint id_font);
    bool needRecalcArea() { return RecalcArea; }
    void clearRecalcArea() { RecalcArea = false; }
  protected:

    class svmBase   *Base[MAX_PAGES];

    class PWinTools* Tools;
    class PWinTools* Actions;
    class PWinTools* Page;
    class PWinTools* chgPage;

    class PWinToolsCoord* Coord;

    PRect frame;

    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType) { return 0; }
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evPaint(HDC hdc, const PRect& rect);
  private:
    void resize();
    int currTools;
    bool multipleAddTools;
    bool RecalcArea;
    LPCTSTR baseCursor;

    HWND hwndTips;
    TCHAR tipsText[1024];
    void manageDispInfo(LPTOOLTIPTEXT info);

    void reloadBkg(LPCTSTR p);

    UINT currAction;

    UINT getCurrPage();
    void setCurrPage(UINT n);

    bool actionToBkg;
    void setActionToBkg();

    bool actionLock;
    void setActionLock();
    bool actionHide;
    void setActionHide();
    void actionGroup();
    void actionUngroup(bool all);
    void toggleEdTxt();
    void toggleChooseTxt();
    void centerVert();
    void centerHorz();
    void setActionMirror();

    setOfString pageProperty[MAX_PAGES];
    void setPageProperty();

    void setBtnStat();

    void Align();
    void Order();

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

    void flushData();

    void showAnchorType(const POINT& pt, int keyFlag);
    void setCursor(uint anchor, bool forceChange);
    void saveWinToolCoords();

    void openPage();
    bool savePage();

    void performNew();

    void loadCustomClear(setOfString& set);
    void loadCustomCrypt(setOfString& set);
    void loadSaveCustom(P_File& target, setOfString& source, bool forceUnicode);
    void saveCustom(P_File& pfCript, P_File& pfClear);

    bool loadPropertyPage(setOfString& set);
    int savePropertyPage(P_File& pfCript, P_File& pfClear);
    int savePropertyPageGen(setOfString& set, P_File& pfCript, P_File& pfClear, bool onUnicode);
    void resetPropertyPage();
    void resetCustom();

    void performNewPage();

    void setTitle();
    bool okToDone(bool all = false);

    POINT ptMoveByKey;
    // vale 1 se movimento, 2 se resize, 0 se nessuna azione
    int isMovingByKey;

    void performMoveByKey(int key);
    void performSizeByKey(int key);
    void performEndKey();

    PVect<PRect> linkedObj;
    void addRectObj(LPCTSTR p, const POINT& offset, bool useDim);
    void fillLinkedObj(setOfString& set, POINT offset);
    void loadLinkedPage(setOfString& set);

    void removeTemp();
    void performNewStd();
    void lockPage(bool lock);

    void performOpenPage(setOfString& set);
    void resetPage();

    void checkRMouseLinked();
    void lockObject(bool lock);

    void setAddLock();
    void setAddHide();

    void enableTools(bool set);
    template <typename OBJ, int offs>
    void alloc_objects(setOfString& set, uint num, uint idInit, HDC hdc, createGroupObject& createGrp);

    bool onExit;

    bool onPan;
    bool canPan();

    struct copyPasteCustom
    {
      setOfString set;
      setOfString setC;
    };
    copyPasteCustom* cpCustom;
    void copyCustom();
    void pasteCustom();

    bool performLoadPage(setOfString& set);
    void openPageFromOtherPrj();

    float moveKeyCount;
};
//-----------------------------------------------------------
svmMainClient* getMainClient(PWin* w);
//-----------------------------------------------------------
#endif
