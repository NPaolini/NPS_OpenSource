//--------------------- svmMainWorkArea.cpp ------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include "svmMainWorkArea.h"
#include "svmMainClient.h"
#include "svmBase.h"
#include "common.h"
#include "resource.h"
#include "svmCalibrate.h"
#include "svmDialogVariable.h"
#include "POwnBtnImageStd.h"
#include "svm_AllObj.h"
#include "pPrintPage.h"
//----------------------------------------------------------------------------
static UINT globalId[MAX_PAGES] = { 0 };
static UINT globalIdLinked[MAX_PAGES] = { 0 };
//----------------------------------------------------------------------------
static bool ToggleUnicode = true;
bool saveAsUnicode() { return ToggleUnicode; }
//----------------------------------------------------------------------------
static bool ToggleXor;
bool isColoredXor() { return ToggleXor; }
//----------------------------------------------------------------------------
#define TOOLS_PATH   _T("Tools_coords")
#define ACTION_PATH  _T("Actions_coords")
#define FILE_PATH    _T("Files_coords")
#define CHG_PAGE_PATH _T("chgPage_coords")
#define MOUSE_PATH _T("mousePos_coords")
//----------------------------------------------------------------------------
#ifndef TTS_BALLOON
  #define TTS_BALLOON         0x0040
  #define TTF_TRANSPARENT     0x0100
  #define TTM_SETMAXTIPWIDTH  (WM_USER + 24)

#endif
#define TTS_STYLE (TTS_ALWAYSTIP | TTS_BALLOON)
//----------------------------------------------------------------------------
bool getCoord(LPCTSTR sub, DWORD& target)
{
  return getKeyParam(sub, MAIN_PATH, target);
}
//----------------------------------------------------------------------------
bool setCoord(LPCTSTR sub, DWORD source)
{
  return setKeyParam(sub, MAIN_PATH, source);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define LAST_TOOL tTOOL_LAST
#define LAST_ACTION (LAST_TOOL + tACTION_LAST)
#define LAST_PAGE (LAST_ACTION + tPAGE_LAST)
//-----------------------------------------------------------
enum typeTools {
    tSELECT,
    tPAN,
    tTEXT,
    tLINE,
    tRECT,
    tELLIPSE,
    tPICTURE,

    tTOOL_LAST
    };
//-----------------------------------------------------------
enum typeActions {
    tUNDO,
    tALIGN,
    tORDER,
    tPROPERTY,
    tGROUP_LOCK,
    tGROUP_HIDE,
    tFONT,
    tXOR_COLOR,
    tRESTORE_INSIDE,

    tACTION_LAST
    };
//-----------------------------------------------------------
enum typePage {
    tNEW,
    tOPEN,
    tSAVE,
    tCLEAR,
    tVARS,
    tLINKED,
    tDONE,
    tPAGE_LAST
    };
//-----------------------------------------------------------
enum typeChgPage {
    tONE,
    tTWO,
    tTHREE,
    tFOUR,
    tFIVE,
    tSIX,
    tSEVEN,
    tEIGHT,

    tCHGPAGE_LAST
    };
//-----------------------------------------------------------
#define FIRST_ID_COMMAND IDM_ARROW

enum id_Cmd {  IDM_ARROW = 100,
              IDM_PAN,
              IDM_TEXT,
              IDM_LINE,
              IDM_RECT,
              IDM_ELLIPSE,
              IDM_PICTURE,

              IDM_UNDO,
              IDM_ALIGN,
              IDM_ORDER,
              IDM_PROPERTY,
              IDM_GROUP_LOCK,
              IDM_GROUP_HIDE,
              IDM_FONT,
              IDM_TOGGLE_XOR,
              IDM_RESTORE_INSIDE,

              IDM_NEW,
              IDM_OPEN,
              IDM_SAVE,
              IDM_CLEAR,
              IDM_VARS,
              IDM_LINKED,
              IDM_DONE_,

              IDM_ONE_P,
              IDM_TWO_P,
              IDM_THREE_P,
              IDM_FOUR_P,
              IDM_FIVE_P,
              IDM_SIX_P,
              IDM_SEVEN_P,
              IDM_EIGHT_P,
            };
//-----------------------------------------------------------
#ifndef TBSTYLE_FLAT
  // Borland nella struttura dichiara un BYTE reserved[2] ????
  #define MY_TBSTYLE1 TBSTYLE_CHECKGROUP, { 0 }
  #define MY_TBSTYLE2 TBSTYLE_BUTTON, { 0 }
  #define MY_TBSTYLE3 TBSTYLE_CHECK, { 0 }
#else
  #define MY_TBSTYLE1 TBSTYLE_CHECKGROUP
  #define MY_TBSTYLE2 TBSTYLE_BUTTON
  #define MY_TBSTYLE3 TBSTYLE_CHECK
#endif
//-----------------------------------------------------------
#define TB_ROW(n, id, style) { (n), (id) + (n), TBSTATE_ENABLED, (style), 0L, (id) + (n) }
//-----------------------------------------------------------
#define TB_ROW_ST1(n, id) TB_ROW(n, id, MY_TBSTYLE1)
#define TB_ROW_ST2(n, id) TB_ROW(n, id, MY_TBSTYLE2)
//-----------------------------------------------------------
TBBUTTON tbButtons[] = {
  TB_ROW_ST1(0, IDM_ARROW),
  TB_ROW_ST1(1, IDM_ARROW),
  TB_ROW_ST1(2, IDM_ARROW),
  TB_ROW_ST1(3, IDM_ARROW),
  TB_ROW_ST1(4, IDM_ARROW),
  TB_ROW_ST1(5, IDM_ARROW),
  TB_ROW_ST1(6, IDM_ARROW),
};
//-----------------------------------------------------------
TBBUTTON tbButtonsActions[] = {
  TB_ROW_ST2(0, IDM_UNDO),
  TB_ROW_ST2(1, IDM_UNDO),
  TB_ROW_ST2(2, IDM_UNDO),
  TB_ROW_ST2(3, IDM_UNDO),
  TB_ROW(4, IDM_UNDO, MY_TBSTYLE3),
  TB_ROW(5, IDM_UNDO, MY_TBSTYLE3),
  TB_ROW_ST2(6, IDM_UNDO),
  TB_ROW(7, IDM_UNDO, MY_TBSTYLE3),
  TB_ROW_ST2(8, IDM_UNDO),
};
//-----------------------------------------------------------
TBBUTTON tbButtonsPage[] = {
  TB_ROW_ST2(0, IDM_NEW),
  TB_ROW_ST2(1, IDM_NEW),
  TB_ROW_ST2(2, IDM_NEW),
  TB_ROW_ST2(3, IDM_NEW),
  TB_ROW_ST2(4, IDM_NEW),
  TB_ROW_ST2(5, IDM_NEW),
  TB_ROW_ST2(6, IDM_NEW),
};
//-----------------------------------------------------------
TBBUTTON tbButtonsChgPage[] = {
  TB_ROW_ST1(0, IDM_ONE_P),
  TB_ROW_ST1(1, IDM_ONE_P),
  TB_ROW_ST1(2, IDM_ONE_P),
  TB_ROW_ST1(3, IDM_ONE_P),
  TB_ROW_ST1(4, IDM_ONE_P),
  TB_ROW_ST1(5, IDM_ONE_P),
  TB_ROW_ST1(6, IDM_ONE_P),
  TB_ROW_ST1(7, IDM_ONE_P),
};
//-----------------------------------------------------------
// identifiers
#define ID_TOOLBAR1         1000
#define ID_TOOLBAR2         1001
#define ID_TOOLBAR3         1002
#define ID_TOOLBAR4         1003

// other constants
// for 640x480
#define BMP_CX_640        20
#define BMP_CY_640        20

#define BMP_CX        32
#define BMP_CY        32
//-----------------------------------------------------------
static LPCTSTR tipsTools[] = {
  _T("Selezione"),
  _T("Muovi Vista"),
  _T("Testo fisso o variabile"),
  _T("Linea"),
  _T("Rettangolo"),
  _T("Ellisse"),
  _T("Immagine"),
  };
//-----------------------------------------------------------
static LPCTSTR tipsActions[] = {
  _T("Annulla"),
  _T("Allineamento"),
  _T("Ordine"),
  _T("Proprietà"),
  _T("Blocca oggetti"),
  _T("Nascondi oggetti"),
  _T("Modifica Font"),
  _T("Selezioni Colorate "),
  _T("Ripristina fuori campo"),
  };
//-----------------------------------------------------------
static LPCTSTR tipsPage[] = {
  _T("Nuovo Template"),
  _T("Apri Template"),
  _T("Salva Template Corrente"),
  _T("Pulisci Template Corrente"),
  _T("Gestione Variabili"),
  _T("Collega a Template"),
  _T("Fine"),
  };
//-----------------------------------------------------------
static LPCTSTR tipsChgPage[] = {
  _T("Vai a Template 1"),
  _T("Vai a Template 2"),
  _T("Vai a Template 3"),
  _T("Vai a Template 4"),
  _T("Vai a Template 5"),
  _T("Vai a Template 6"),
  _T("Vai a Template 7"),
  _T("Vai a Template 8"),
  };
//-----------------------------------------------------------
PToolBarInfo iTools = {
    ID_TOOLBAR1,
    tbButtons,
    SIZE_A(tbButtons),
    IDB_TOOLBAR,
    1, // 2, 5,

    { BMP_CX, BMP_CY },
    { BMP_CX, BMP_CY },
    0,
    tipsTools,
    };
//-----------------------------------------------------------
PToolBarInfo iActions = {
    ID_TOOLBAR2,
    tbButtonsActions,
    SIZE_A(tbButtonsActions),
    IDB_TOOLBAR_ACTION,
    1,
    { BMP_CX, BMP_CY },
    { BMP_CX, BMP_CY },
    -1,
    tipsActions
    };
//-----------------------------------------------------------
PToolBarInfo iPage = {
    ID_TOOLBAR3,
    tbButtonsPage,
    SIZE_A(tbButtonsPage),
    IDB_TOOLBAR_PROJECT,
    1,
    { BMP_CX, BMP_CY },
    { BMP_CX, BMP_CY },
    -1,
    tipsPage
    };
//-----------------------------------------------------------
PToolBarInfo iChgPage = {
    ID_TOOLBAR4,
    tbButtonsChgPage,
    SIZE_A(tbButtonsChgPage),
    IDB_TOOLBAR_CHG_PAGE,
    2,
    { BMP_CX, BMP_CY },
    { BMP_CX, BMP_CY },
    0,
    tipsChgPage
    };
//-----------------------------------------------------------
static dataProject DataP;
//-----------------------------------------------------------
dataProject& getDataProject()
{
  return DataP;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void saveCoordGen(HWND hwnd, LPCTSTR tool)
{
  PRect r;
  GetWindowRect(hwnd, r);
  DWORD coords = MAKELONG(r.left, r.top);
  setCoord(tool, coords);
}
//----------------------------------------------------------------------------
class saveCoordWinTools : public PWinTools
{
  private:
    typedef PWinTools baseClass;
  public:
    saveCoordWinTools(PWin * parent, struct PToolBarInfo *info, LPCTSTR title,
            UINT idBkg, HINSTANCE hInstance, LPCTSTR path) :
            baseClass(parent, info, title, idBkg, hInstance), Path(path) {}
    virtual ~saveCoordWinTools() { destroy(); }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LPCTSTR Path;
};
//----------------------------------------------------------------------------
LRESULT saveCoordWinTools::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      saveCoordGen(hwnd, Path);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
class chgPageWinTools : public saveCoordWinTools
{
  private:
    typedef saveCoordWinTools baseClass;
  public:
    chgPageWinTools(PWin * parent, struct PToolBarInfo *info, LPCTSTR title,
            UINT idBkg, HINSTANCE hInstance) :
            baseClass(parent, info, title, idBkg, hInstance, CHG_PAGE_PATH) {}
    virtual ~chgPageWinTools() { destroy(); }
  protected:
    virtual LPTSTR getTips(uint id);
};
//----------------------------------------------------------------------------
LPTSTR chgPageWinTools::getTips(uint id)
{
  uint ix = id - IDM_ONE_P;
  if(ix >= MAX_PAGES)
    return baseClass::getTips(id);
  if(*DataP.getcurrPath(ix) && *DataP.gettemplateName(ix)) {
    static TCHAR tit[_MAX_PATH * 2];
    wsprintf(tit, _T("%s\r\ntemplate -> %s"), DataP.getcurrPath(ix), DataP.gettemplateName(ix));
    int len = _tcslen(tit);
    for(int i = len -1; i > 0 && tit[i] > _T(' '); --i) {
      if(_T('.') == tit[i]) {
        tit[i] = 0;
        break;
        }
      }
    return tit;
    }
  return baseClass::getTips(id);
}
//-----------------------------------------------------------
//#define BKG_COORD RGB(0xd0, 0xff, 0xff)
#define BKG_COORD RGB(200,180,160)
//-----------------------------------------------------------
class PWinToolsCoord : public PWinToolBase
{
    typedef PWinToolBase baseClass;
  public:
    PWinToolsCoord(PWin * parent, LPCTSTR title, UINT idBkg, HINSTANCE hInstance) :
      PWinToolBase(parent, title, idBkg, hInstance), X(0), Y(0), hbkg(CreateSolidBrush(BKG_COORD)), onZoom(false), Btn(0) { }
    ~PWinToolsCoord() { destroy(); DeleteObject(hbkg); }

    bool create();
    virtual bool preProcessMsg(MSG& msg)
    {
      return toBool(IsDialogMessage(getHandle(), &msg));
    }
    void restoreZoom(svmManZoom::zoomX zoom);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC hdc, HWND hwnd);
    void setCoord(int x, int y);
    int X;
    int Y;
    HBRUSH hbkg;
    bool onZoom;
    POwnBtn* Btn;
    void changeZoom();
};
//-----------------------------------------------------------
#define WM_CUSTOM_COORD (WM_FW_FIRST_FREE + 2)
#define IDC_ST_COORD_X 100
#define IDC_ST_COORD_Y 101
#define IDC_BTN_ZOOM   102

#define IDC_ST_COORD_X2 103
#define IDC_ST_COORD_Y2 104
//-----------------------------------------------------------
#define CM_SET_COORD   1
#define CM_SET_COORD_X 2
#define CM_SET_COORD_Y 3
//-----------------------------------------------------------
#define WIDTH_COORD 60
#define X_COORD_2 (WIDTH_COORD + 4)
#define X_COORD_BTN (WIDTH_COORD * 2 + 7)
#define ALL_WIDTH_COORD (WIDTH_COORD * 3 + 8)
//-----------------------------------------------------------
#define HEIGHT_COORD 22
//-----------------------------------------------------------
class myBtnZoom : public POwnBtnImageStd
{
  private:
    typedef POwnBtnImageStd baseClass;
  public:

    enum where { wLeft, wTop, wRight, wBottom, wCenter };

    myBtnZoom(PWin * parent, uint id, int x, int y, int w, int h,
          PBitmap* image, uint fPos = wLeft, bool autoDelete = false,
          LPCTSTR text = 0, HINSTANCE hinst = 0)  :
      baseClass(parent, id, x, y, w, h, image, fPos, autoDelete, text, hinst) {}

    virtual ~myBtnZoom() { destroy(); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//-----------------------------------------------------------
LRESULT myBtnZoom::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static bool inExec;
  switch(message) {
    case WM_LBUTTONUP:
//    case WM_LBUTTONDOWN:
      if(!inExec) {
        inExec = true;
        SendMessage(*this, BM_CLICK, 0, 0);
        inExec = false;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
bool PWinToolsCoord::create()
{
  HFONT hf = D_FONT(20, 0, fBOLD, _T("Comic Sans MS"));
//  HFONT hf = D_FONT(20, 0, fBOLD, _T("arial"));
  PStatic* st = new PStatic(this, IDC_ST_COORD_X, 2, 1, WIDTH_COORD, HEIGHT_COORD);
  // N.B. il primo controllo elimina il font, tutti i successivi devono
  // impostare l'autodelete a false
  st->setFont(hf, true);
  st = new PStatic(this, IDC_ST_COORD_Y, X_COORD_2, 1, WIDTH_COORD, HEIGHT_COORD);
  st->setFont(hf, false);
  st = new PStatic(this, IDC_ST_COORD_X2, 2, 26, WIDTH_COORD, HEIGHT_COORD);
  st->setFont(hf, false);
  st = new PStatic(this, IDC_ST_COORD_Y2, X_COORD_2, 26, WIDTH_COORD, HEIGHT_COORD);
  st->setFont(hf, false);
  Attr.style |= WS_VISIBLE;
  PRect r(0, 0, ALL_WIDTH_COORD, HEIGHT_COORD);
  AdjustWindowRectEx(r, Attr.style, FALSE, Attr.exStyle);
  Attr.w = r.Width();
  Attr.h = r.Height();
  if(isWinNT_Based()) {
    Btn = new myBtnZoom(this, IDC_BTN_ZOOM, X_COORD_BTN, 0, WIDTH_COORD, HEIGHT_COORD, 0,  POwnBtnImageStd::wLeft, false, _T("x 1"));
    Btn->setFont(hf, false);
    }
  else
    Attr.w -= WIDTH_COORD;
  if(!baseClass::create())
    return false;

  return true;
}
//-----------------------------------------------------------
LRESULT PWinToolsCoord::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      saveCoordGen(hwnd, MOUSE_PATH);
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BTN_ZOOM:
          changeZoom();
          break;
        }
      break;
    case WM_CUSTOM_COORD:
      switch(LOWORD(wParam)) {
        case CM_SET_COORD:
          setCoord((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
          break;
        case CM_SET_COORD_X:
          setCoord((int)(short)LOWORD(lParam), Y);
          break;
        case CM_SET_COORD_Y:
          setCoord(X, (int)(short)HIWORD(lParam));
          break;
      }
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
HBRUSH PWinToolsCoord::evCtlColor(HDC hdc, HWND hwnd)
{
  SetTextColor(hdc, RGB(0, 0, 128));
  SetBkColor(hdc, BKG_COORD);
  return hbkg;
}
//-----------------------------------------------------------
void PWinToolsCoord::restoreZoom(svmManZoom::zoomX zoom)
{
  LPCTSTR tit[] = { _T("x 1/2"), _T("x 2/3"), _T("x 1"), _T("x 3/2"), _T("x 2"), _T("x 3"), _T("x 4") };
  Btn->setCaption(tit[zoom]);

  bool oldZoom = onZoom;
  onZoom = zoom != svmManZoom::zOne;
  if(onZoom ^ oldZoom) {
    PRect r;
    GetWindowRect(*this, r);
    if(onZoom)
      r.bottom += 24;
    else
      r.bottom -= 24;
    setWindowPos(0, r, SWP_NOMOVE | SWP_NOZORDER);
    InvalidateRect(*this, 0 ,TRUE);
    }
}
//-----------------------------------------------------------
void PWinToolsCoord::changeZoom()
{
  enum idMenu { id1 = 1000, id2, id3, id4, id5, id6, id7, id8 };
  menuPopup menu[] = {
    { MF_STRING, id1, _T("Zoom x 1/2") },
    { MF_STRING, id2, _T("Zoom x 2/3") },
    { MF_STRING, id3, _T("Zoom x 1") },
    { MF_STRING, id4, _T("Zoom x 3/2") },
    { MF_STRING, id5, _T("Zoom x 2") },
    { MF_STRING, id6, _T("Zoom x 3") },
    { MF_STRING, id7, _T("Zoom x 4") },
    { MF_STRING, id8, _T("Calibra schermo") },
    };
  childScrollWorkArea* mc = getParentWin<childScrollWorkArea>(this);
  if(mc) {

    int curr = mc->getZoom();
    menu[curr].flag |= MF_GRAYED;
    if(curr != svmManZoom::zOne)
      menu[7].flag |= MF_GRAYED;

    int res = popupMenu(*this, menu, SIZE_A(menu)) - id1;
    if(7 == res) {
      PCalibrate(this).modal();
      return;
      }
    if((uint)res >= svmManZoom::z_Max)
      return;
    restoreZoom((svmManZoom::zoomX)res);
    mc->resetScroll();
    mc->setZoom((svmManZoom::zoomX)res);
    InvalidateRect(*mc, 0, 0);
    }
}
//-----------------------------------------------------------
void set_int_dec(PWin* owner, uint idc, int v)
{
  TCHAR buff[64];
  wsprintf(buff, _T("%d.%02d"), v / 100, v % 100);
  SetWindowText(GetDlgItem(*owner, idc), buff);
}
//-----------------------------------------------------------
#define SET_INT_DEC(idc, v) set_int_dec(this, idc, v)
//#define SET_INT_DEC SET_INT
//-----------------------------------------------------------
void PWinToolsCoord::setCoord(int x, int y)
{
  HDC hdc = GetDC(*this);
  SetMapMode(hdc, MM_LOMETRIC);
  POINT pt = { x, -y };
  DPtoLP(hdc, &pt, 1);
  SetMapMode(hdc, MM_TEXT);
  ReleaseDC(*this, hdc);

  svmManZoom mZ;
  mZ.setCurrZoom(svmManZoom::zOne);
  POINT pt2 = pt;
  mZ.calcFromScreen(pt);

  if(X != pt.x) {
    X = pt.x;
    SET_INT_DEC(IDC_ST_COORD_X, pt.x);
    }
  if(Y != pt.y) {
    Y = pt.y;
    SET_INT_DEC(IDC_ST_COORD_Y, pt.y);
    }
  if(onZoom) {
    childScrollWorkArea* par = getParentWin<childScrollWorkArea>(this);
    if(par) {
      mZ.setCurrZoom(par->getZoom());
      mZ.calcFromScreen(pt2);
//      getDataProject().calcFromScreen(pt);
      SET_INT_DEC(IDC_ST_COORD_X2, pt2.x);
      SET_INT_DEC(IDC_ST_COORD_Y2, pt2.y);
      }
    }
}
//-----------------------------------------------------------
static
void set_Coord(PWinToolBase* t, DWORD coords)
{
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);

  t->Attr.x = (short)LOWORD(coords);
  t->Attr.y = (short)HIWORD(coords);
  if(t->Attr.x < 0)
    t->Attr.x = 0;
  else if(t->Attr.x > width - 100)
    t->Attr.x = width - 100;
  if(t->Attr.y < 0)
    t->Attr.y = 0;
  else if(t->Attr.y > height - 100)
    t->Attr.y = height - 100;
}
//-----------------------------------------------------------
void newProject(HWND owner)
{
  PWin* w = PWin::getWindowPtr(owner);
  childScrollWorkArea* par = getParentWin<childScrollWorkArea>(w);
  if(par)
    SendMessage(*par, WM_C_CHANGED_SELECTION, LAST_ACTION + tNEW + FIRST_ID_COMMAND, 0);
}
//-----------------------------------------------------------
void invalidateWorkArea(PWin* child)
{
  childScrollWorkArea* par = getParentWin<childScrollWorkArea>(child);
  if(par)
    InvalidateRect(*par, 0, 0);
}
//-----------------------------------------------------------
childScrollWorkArea::childScrollWorkArea(PWin* parent, uint id, const PRect& rect, HINSTANCE hInstance) :
      baseClass(parent, id, rect, 0, hInstance), Zoom(svmManZoom::zOne), hwndTips(0), Tools(0), currTools(tSELECT),
      multipleAddTools(false), baseCursor(IDC_ARROW), isMovingByKey(0),
      actionLock(false), actionHide(false), onExit(false), onPan(false)
{
  fillStr(tipsText, 0, SIZE_A(tipsText));

  for(int i = 0; i < SIZE_A(Base); ++i) {
    Base[i] = new svmBase(this);
    }

  DWORD coords;
  Mdc.setBkg(RGB(255,255,255));

  Tools = new saveCoordWinTools(this, &iTools, _T("Oggetti"), IDB_BKG, getHInstance(), TOOLS_PATH);
  Tools->Attr.x = 10;
  if(getCoord(TOOLS_PATH, coords))
    set_Coord(Tools, coords);

  Actions = new saveCoordWinTools(this, &iActions, _T("Azioni"), IDB_BKG,  getHInstance(), ACTION_PATH);
  Actions->Attr.x = 10;
  Actions->Attr.y = 100;
  if(getCoord(ACTION_PATH, coords))
    set_Coord(Actions, coords);

  Page = new saveCoordWinTools(this, &iPage, _T("File"), IDB_BKG,  getHInstance(), FILE_PATH);
  Page->Attr.x = 175;
  Page->Attr.y = 100;

  if(getCoord(FILE_PATH, coords))
    set_Coord(Page, coords);

  chgPage = new chgPageWinTools(this, &iChgPage, _T("File Template"), IDB_BKG,  getHInstance());
  chgPage->Attr.x = 275;
  chgPage->Attr.y = 100;

  if(getCoord(CHG_PAGE_PATH, coords))
    set_Coord(chgPage, coords);

  Coord = new PWinToolsCoord(this, _T("Coord"), IDB_BKG,  getHInstance());
  Coord->Attr.x = 375;
  Coord->Attr.y = 100;

  if(getCoord(MOUSE_PATH, coords))
    set_Coord(Coord, coords);

  setFlag(pfWantIdle);

  ptMoveByKey.x = 0;
  ptMoveByKey.y = 0;
}
//-----------------------------------------------------------
childScrollWorkArea::~childScrollWorkArea()
{
  onExit = true;
  if(hwndTips)
    DestroyWindow(hwndTips);
  for(int i = 0; i < SIZE_A(Base); ++i)
    delete Base[i];
  destroy();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
UINT childScrollWorkArea::getIncrementGlobalId()
{
  return ++globalId[getCurrPage()];
}
//-----------------------------------------------------------
#ifndef XBUTTON1
  #define XBUTTON1      0x0001
  #define XBUTTON2      0x0002
#endif
//-----------------------------------------------------------
bool childScrollWorkArea::create()
{
  if(!baseClass::create())
    return false;
  getDataProject().initPath();
  setBtnStat();

  // sembra che finché non ci si clicca non cambia il cursore del mouse, allora lo forziamo
  POINT pt = { 20, 20 };
  MapWindowPoints(*this, HWND_DESKTOP, &pt, 1);
  mouse_event(MOUSEEVENTF_LEFTDOWN, pt.x, pt.y,  XBUTTON1, 0);
  Sleep(0);
  mouse_event(MOUSEEVENTF_LEFTUP, pt.x, pt.y,  XBUTTON1, 0);

  return true;
}
//-----------------------------------------------------------
#define DEF_STEP 100
#define VERT_ANGLE 2700
//-----------------------------------------------------------
#define ADD_POINT(buff, v) \
    wsprintf(buff, _T("%d.%02d"), (v) / 100, (v) % 100)
//-----------------------------------------------------------
void childScrollWorkArea::evPaint(HDC hdc, const PRect& rect)
{
#if 0
  PRect r;
  GetClientRect(*this, r);
  DPtoLP(hdc, (LPPOINT)(LPRECT)r, 2);
  int h = -200;
  LONG step = 100;
  int Start = 0;
  int start = Start % step;

  int end = r.Width() + start;
  for(int i = -start; i < end; i += step) {
    MoveToEx(hdc, i, 0, 0);
    LineTo(hdc, i, h);
    }
  h /= 2;
  for(int i = step / 2 - start; i < end; i += step) {
    MoveToEx(hdc, i, 0, 0);
    LineTo(hdc, i, h);
    }
  TCHAR buff[64];
  HFONT fnt = D_FONT_ORIENT(30, 0, 0, 0, _T("arial"));
  HGDIOBJ oldF = SelectObject(hdc, fnt);
  int start2 = Start - start;
  h *= 2;

  for(int i = -start; i < end; i += step, start2 += DEF_STEP) {
    ADD_POINT(buff, start2);
    ExtTextOut(hdc, i, h, 0 , 0, buff, _tcslen(buff), 0);
    }
  DeleteObject(SelectObject(hdc, oldF));
#endif
  Base[getCurrPage()]->paint(hdc, rect, true);
  Base[getCurrPage()]->paint(hdc, rect, false);
}
//-----------------------------------------------------------
void childScrollWorkArea::enableTools(bool set)
{
  EnableWindow(*Tools, set);
  EnableWindow(*Actions, set);
  EnableWindow(*Page, set);
  EnableWindow(*chgPage, set);
}
//-----------------------------------------------------------
UINT childScrollWorkArea::getCurrPage()
{
  return DataP.currPage;
}
//-----------------------------------------------------------
void childScrollWorkArea::setPageTitle()
{
  if(*DataP.getcurrPath()) {
    Base[getCurrPage()]->addBtnStat(svmBase::eBS_hasProject);
    PMainWin* m = getParentWin<PMainWin>(this);
    if(m) {
      TCHAR tit[_MAX_PATH * 2];
      wsprintf(tit, _T("SvLabel - %s - %s"), DataP.getcurrPath(), DataP.gettemplateName());
      m->setCaption(tit);
      }
    }
  else {
    Base[getCurrPage()]->remBtnStat(svmBase::eBS_hasProject);
    PMainWin* m = getParentWin<PMainWin>(this);
    if(m)
      m->setCaption(_T("SvLabel"));
    }
}
//-----------------------------------------------------------
void childScrollWorkArea::setCurrPage(UINT n)
{
  DataP.setStartX(FirstH);
  DataP.setStartY(FirstV);
  DataP.setCurrZoom(Zoom);
  DataP.currPage = n;

  setPageTitle();

  setBtnStat();
  svmManZoom::zoomX z = DataP.getCurrZoom();
  setZoom(z, true);
  POINT pt = { DataP.getStartX(), DataP.getStartY() };
  SendMessage(*getParent(), WM_SEND_SCROLL_POS, 0, (LPARAM)&pt);
  setScrollPos(pt.x, pt.y);
  InvalidateRect(*this, 0, 0);
  Coord->restoreZoom(z);
}
//-----------------------------------------------------------
PRect getLogRectBase();
//-----------------------------------------------------------
void childScrollWorkArea::setZoom(svmManZoom::zoomX zoom, bool force)
{
  Zoom = zoom;
  PRect base = lgToDp(getLogRectBase());
  svmManZoom& mz = DataP.getZoom();
  mz.setCurrZoom(Zoom);
  mz.calcToScreen(base);
  setWindowPos(0, base, SWP_NOZORDER);
  Mdc.clear();
  SendMessage(*getParent(), WM_SEND_DIM, 0, 0);
//  PostMessage(*getParent(), WM_SEND_DIM, 0, 0);

  g_setZoom(this, zoom, force);
}
//-----------------------------------------------------------
void childScrollWorkArea::getWindowClass(WNDCLASS& wcl)
{
  baseClass::getWindowClass(wcl);
  wcl.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
}
//-----------------------------------------------------------
void childScrollWorkArea::setScrollPos(uint x, uint y)
{
  baseClass::setScrollPos(x, y);
  svmManZoom mZ;
  mZ.setCurrZoom(Zoom);
  POINT pt = { x, y };
  mZ.calcFromScreen(pt);
  g_setStartX(this, pt.x);
  g_setStartY(this, pt.y);
}
//-----------------------------------------------------------
void childScrollWorkArea::setHPos(uint first)
{
  baseClass::setHPos(first);
  svmManZoom mZ;
  mZ.setCurrZoom(Zoom);
  LONG v = first;
  mZ.calcFromScreenH(v);
  g_setStartX(this, v);
}
//-----------------------------------------------------------
void childScrollWorkArea::setVPos(uint first)
{
  baseClass::setVPos(first);
  svmManZoom mZ;
  mZ.setCurrZoom(Zoom);
  LONG v = first;
  mZ.calcFromScreenV(v);
  g_setStartY(this, v);
}
//-----------------------------------------------------------
void childScrollWorkArea::resetScroll()
{
  baseClass::resetScroll();
  g_setStartX(this, 0);
  g_setStartY(this, 0);
}
//-----------------------------------------------------------
void childScrollWorkArea::manageDispInfo(LPTOOLTIPTEXT info)
{
  info->hinst = 0;
  info->lpszText = tipsText;
  SendMessage(hwndTips, TTM_SETMAXTIPWIDTH, 0, 300);
}
//-----------------------------------------------------------
void childScrollWorkArea::createTips()
{
  if(hwndTips)
    DestroyWindow(hwndTips);
  hwndTips = CreateWindow(TOOLTIPS_CLASS, 0, TTS_STYLE,
    0, 0, 0, 0, 0, 0, getHInstance(), 0);
  if(hwndTips) {
    TOOLINFO ti;
    memset(&ti, sizeof(ti), 0);
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = /*TTF_IDISHWND |*/ TTF_TRANSPARENT;
    ti.hwnd = getHandle();
    ti.hinst = getHInstance();
    ti.lpszText = LPSTR_TEXTCALLBACK;
    ti.uId = 1;
    GetClientRect(*this, &ti.rect);
    SendMessage(hwndTips, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
    }
}
//-----------------------------------------------------------
void childScrollWorkArea::setTransform(HDC hdc)
{
  svmManZoom& mZ = getDataProject().getZoom();
  XFORM xForm;
  FLOAT z = 1;
  mZ.calcToScreenH(z);
  xForm.eM11 = z;
  xForm.eM12 = (FLOAT) 0.0;
  xForm.eM21 = (FLOAT) 0.0;
  z = 1;
  mZ.calcToScreenV(z);
  xForm.eM22 = z;
  xForm.eDx  = (FLOAT) 0.0;
  xForm.eDy  = (FLOAT) 0.0;
  SetWorldTransform(hdc, &xForm);
}
//-----------------------------------------------------------
void childScrollWorkArea::resetTransform(HDC hdc)
{
  svmManZoom& mZ = getDataProject().getZoom();
  XFORM xForm;
  FLOAT z = 1;
  xForm.eM11 = z;
  xForm.eM12 = (FLOAT) 0.0;
  xForm.eM21 = (FLOAT) 0.0;
  xForm.eM22 = z;
  xForm.eDx  = (FLOAT) 0.0;
  xForm.eDy  = (FLOAT) 0.0;
  SetWorldTransform(hdc, &xForm);
}
//-----------------------------------------------------------
LRESULT childScrollWorkArea::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_NOTIFY:
      switch(((LPNMHDR)lParam)->code) {
        case TTN_NEEDTEXT:
//        case TTN_GETDISPINFO:
          manageDispInfo((LPTOOLTIPTEXT)lParam);
          break;
        }
      break;
    case WM_HAS_SCROLL:
      if(!wParam && tPAN == currTools) {
        Tools->selectBtn(0);
        PostMessage(*this, WM_C_CHANGED_SELECTION, FIRST_ID_COMMAND + tSELECT, 0);
        }
      Tools->enableBtn(1, toBool(wParam));
      break;
    case WM_PAINT:
      do {
        PAINTSTRUCT Paint;
        HDC hdc = BeginPaint(hwnd, &Paint);
        if(!hdc) {
          EndPaint(*this, &Paint);
          return 0;
          }

        HDC mdc = Mdc.getMdc(this, hdc);
        if(mdc) {
          SetMapMode(mdc, MM_LOMETRIC);
          setTransform(mdc);
          PRect rect(Paint.rcPaint);
          DPtoLP(hdc, (LPPOINT)(LPRECT)rect, 2);
          evPaint(mdc, rect);
          resetTransform(mdc);
          SetMapMode(mdc, MM_TEXT);
          PRect rect2(Paint.rcPaint);
          BitBlt(hdc, rect2.left, rect2.top, rect2.Width(), rect2.Height(), mdc, rect2.left, rect2.top, SRCCOPY);
          }
        EndPaint(hwnd, &Paint);
        } while(false);
      return 0;
    case WM_SIZE:
      createTips();
      break;

    case WM_C_CHANGED_SELECTION:
      do {
        int curr = wParam - FIRST_ID_COMMAND;
        if(curr < LAST_TOOL) {
          currTools = curr;
          switch(currTools) {
            case tPAN:
              baseCursor = MAKEINTRESOURCE(IDC_CURSOR_PAN);
              break;
            case tSELECT:
              baseCursor = IDC_ARROW;
              break;
            default:
              baseCursor = IDC_CROSS;
              break;
            }
          }
        else if(curr < LAST_ACTION) {
          curr -= LAST_TOOL;
          switch(curr) {
            case tUNDO:
              Base[getCurrPage()]->Undo();
              break;
            case tALIGN:
              Align();
              break;
            case tORDER:
              Order();
              break;
            case tPROPERTY:
/*
              if(!Base[getCurrPage()]->canProperty())
                setPageProperty();
              else
*/
                Base[getCurrPage()]->setProperty(needSingleProperty());
              break;
            case tGROUP_LOCK:
              setActionLock();
              break;
            case tGROUP_HIDE:
              setActionHide();
              break;
/**/
            case tFONT:
//              lockPage(true);
              modifyPageFont(this);
//              lockPage(false);
              break;
/**/
            case tRESTORE_INSIDE:
              Base[getCurrPage()]->ReInside();
              break;

            case tXOR_COLOR:
              ToggleXor = !ToggleXor;
              InvalidateRect(*this, 0, 0);
              break;
            default:
              MessageBox(*this, tipsActions[curr], _T("Choose"), MB_OK);
              break;
            }
          }
        else if(curr < LAST_PAGE) {
          curr -= LAST_ACTION;
          switch(curr) {
            case tDONE:
              if(okToDone(true)) {
                PostQuitMessage(0);
                }
              break;
            case tNEW:
              if(!okToDone())
                break;
              performNew();
              break;

            case tOPEN:
              if(okToDone())
                openPage();
              break;

            case tSAVE:
              savePage();
              break;

            case tVARS:
              do {
                int dummy = 0;
                svmDialogVariable(this, IDD_DIALOG_VARS, dummy).modal();
                } while(false);
              break;

            case tLINKED:
              open_Linked();
              break;
            case tCLEAR:
              if(!okToDone())
                break;
              clear_Page();
              break;

            default:
              MessageBox(*this, tipsPage[curr], _T("Choose"), MB_OK);
              break;
            }
          }
/**/
        else {
          curr -= LAST_PAGE;
          setCurrPage(curr);
//          setBtnStat();
          }
        SetFocus(*this);
        } while(false);
      break;

    case WM_C_DELETE_SELECTION:
      Base[getCurrPage()]->deleteSelected();
      break;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;

    case WM_KEYDOWN:
      switch(wParam) {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
          if((GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_CONTROL) & 0x8000))
            performMoveByKey(wParam);
          // da modificare
          else if((GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000))
            performSizeByKey(wParam);
          break;

        case VK_CONTROL:
          if(!onPan && GetKeyState(VK_MENU) & 0x8000)
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
          break;
        case VK_MENU:
          if(!onPan && GetKeyState(VK_CONTROL) & 0x8000)
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
          break;
        }
      break;

    case WM_KEYUP:
      switch(wParam) {
        case VK_BACK:
        case VK_DELETE:
          PostMessage(*this, WM_C_DELETE_SELECTION, 0, 0);
          break;

        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_SHIFT:
        case VK_CONTROL:
          performEndKey();
        case VK_MENU:
          SetCursor(LoadCursor(0,  IDC_ARROW));
          break;
          break;
        }
      break;

    case WM_MOUSEWHEEL:
      PostMessage(*getParent(), WM_MOUSEWHEEL, wParam, lParam);
      return 0;

    case WM_MOUSEMOVE:
      PostMessage(*Coord, WM_CUSTOM_COORD, MAKEWPARAM(CM_SET_COORD, 0), lParam);

    case WM_RBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
//    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
      evMouse(message, wParam, lParam);
      break;

    case WM_LBUTTONDBLCLK:
      PostMessage(*this, WM_C_CHANGED_SELECTION, FIRST_ID_COMMAND + LAST_TOOL + tPROPERTY, 0);
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {

        case IDCM_PROP:
          PostMessage(*this, WM_C_CHANGED_SELECTION, LAST_TOOL + tPROPERTY + FIRST_ID_COMMAND, 0);
          break;

        case IDCM_DUP:
        case IDCM_COPY:
          Base[getCurrPage()]->copySelected();
          if(IDCM_COPY == LOWORD(wParam))
            break;
          // fall through
        case IDCM_PASTE:
          do {
            HDC hdc = getDC();
            Base[getCurrPage()]->pasteCopied(hdc);
            releaseDC(hdc);
            } while(false);
          break;
        case IDCM_FLUSH:
          Base[getCurrPage()]->flushSelected4Paste();
          break;
        case IDCM_UNDO:
          PostMessage(*this, WM_C_CHANGED_SELECTION, LAST_TOOL + tUNDO + FIRST_ID_COMMAND, 0);
          break;
        case IDCM_LOCK:
          setAddLock();
          break;
        case IDCM_HIDE:
          setAddHide();
          break;
        case IDCM_OFFSET:
          if(Base[getCurrPage()]->setActionOffset())
            DataP.setDirty(true);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
extern bool openLinked(PWin* parent);
//-----------------------------------------------------------
void childScrollWorkArea::open_Linked()
{
  if(openLinked(this)) {
    Base[getCurrPage()]->flushLinked();
    dataProject& dp = getDataProject();
    if(*dp.getlinkedFileName())
      addLinked(dp.getlinkedFileName());
    InvalidateRect(*this, 0, 0);
    }
}
//-----------------------------------------------------------
void childScrollWorkArea::clear_Page()
{
  clearPage(true);
  FirstH = 0;
  FirstV = 0;
  Zoom = svmManZoom::zOne;
  setCurrPage(DataP.currPage);
}
//-----------------------------------------------------------
bool childScrollWorkArea::performNew()
{
  clearPage(true);
  if(IDOK == PDataProject(this, PDataProject::useForNew, IDD_DIALOG_PROJECT).modal()) {
    FirstH = 0;
    FirstV = 0;
    Zoom = svmManZoom::zOne;
    setCurrPage(DataP.currPage);
//    clearPage(true);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
class my_ManageRowOfBuff : public manageRowOfBuff
{
  private:
    typedef manageRowOfBuff baseClass;
  public:
    my_ManageRowOfBuff(LPCTSTR filename, PVect<LPCTSTR>& rows) :
      baseClass(filename), Rows(rows) {}
  protected:
    virtual bool manageLine(LPCTSTR row)
    {
      if(_ttoi(row) > 0)
        Rows[Rows.getElem()] = str_newdup(row);
      return true;
    }
  private:
    PVect<LPCTSTR>& Rows;
};
//-----------------------------------------------------------
// presuppone che nel file template ci siano le info per caricare il file delle variabili
//-----------------------------------------------------------
bool childScrollWorkArea::loadFileTemplate(LPCTSTR file, LPTSTR dataPath, uint* idVars)
{
  PVect<LPCTSTR> Rows;
  do {
    my_ManageRowOfBuff(file, Rows).run();
    } while(false);
  uint nElem = Rows.getElem();
  if(!nElem)
    return false;

  dataProject& dp = getDataProject();
  infoState& iStat = dp.getCurrState();
  iStat.reset();

  for(uint i = 0; i < nElem; ++i) {
    LPCTSTR p = Rows[i];
    svmObject* t = 0;
    int id = _ttoi(p);
    switch(id) {
      case PRN_TXT:
      case PRN_TXT_ON_BOX:
      case PRN_TXT_VAR:
      case PRN_TXT_ON_BOX_VAR:
        t = new svmObjText(this, ++globalId[getCurrPage()], 0, 0);
        break;
      case PRN_LINE:
        t = new svmObjLine(this, ++globalId[getCurrPage()], 0, 0);
        break;
      case PRN_BMP:
        t = new svmObjBitmap(this, ++globalId[getCurrPage()], 0, 0);
        break;
      case PRN_RECT:
      case PRN_ROUND_RECT:
        t = new svmObjRect(this, ++globalId[getCurrPage()], 0, 0);
        break;
      case PRN_OVAL:
        t = new svmObjEllipse(this, ++globalId[getCurrPage()], 0, 0);
        break;
      case PRN_SET_FONT:
      case PRN_SET_FONT2:
        iStat.loadFont(p);
        break;
      case PRN_SET_COLOR_FG:
        iStat.loadForeground(p);
        break;
      case PRN_SET_COLOR_BKG:
        iStat.loadBackground(p);
        break;
      case PRN_SET_ALIGN:
        iStat.loadAlign(p);
        break;
      case PRN_SET_BKMODE:
        iStat.loadBkMode(p);
        break;
      case PRN_SET_PEN:
        iStat.loadPen(p);
        break;
      case PRN_SET_BRUSH1:
      case PRN_SET_BRUSH2:
      case PRN_SET_NULL_BRUSH:
        iStat.loadBrush(p);
        break;
      case ID_FILE_DATA:
        // se esiste è una versione aggiornata, sovrascriviamo il buffer
        // passato come argomento
        p = findNextParamTrim(p);
        if(p) {
          _tcscpy_s(dataPath, _MAX_PATH, dp.getcurrPath());
          appendPath(dataPath, p);
          }
        break;
      case ID_FILE_DATA_VARS:
        // idem come sopra
        p = findNextParamTrim(p);
        for(uint j = 0; j < MAX_ID_DATA_VARS; ++j) {
          if(p) {
            uint ix = _ttoi(p);
            idVars[j] = ix;
            p = findNextParamTrim(p);
            }
          }
        break;
      case ID_FILE_LINKED:
        do {
          infoState old = iStat;
          addLinked(findNextParamTrim(p));
          iStat = old;
          } while(false);
        break;
/*    viene gestito nel dialogo di apertura pagina
      case ID_FILE_PAGE_DIM:
        p = findNextParamTrim(p);
        if(p) {
          dp.setPageWidth(_ttoi(p));
          p = findNextParamTrim(p);
          if(p)
            dp.setPageHeight(_ttoi(p));
          }
        break;
*/
      }
    if(t) {
      if(!t->load(p)) {
        --globalId[getCurrPage()];
        delete t;
        }
      else
        Base[getCurrPage()]->addObject(t);
      }
    }

  if(*dataPath) {
    manageVariable& dv = getManVariables();
    dv.load(dataPath, idVars);
    }
  Base[getCurrPage()]->flushHistory();
  flushPAV(Rows);
  return true;
}
//-----------------------------------------------------------
void childScrollWorkArea::addLinked(LPCTSTR file)
{
  dataProject& dp = getDataProject();
  dp.setlinkedFileName(file);
  PVect<LPCTSTR> Rows;
  do {
    TCHAR path[_MAX_PATH];
    _tcscpy_s(path, dp.getcurrPath());
    appendPath(path, file);
    my_ManageRowOfBuff(path, Rows).run();
    } while(false);
  uint nElem = Rows.getElem();
  if(!nElem) {
    dp.setlinkedFileName(_T(""));
    return;
    }
  infoState& iStat = dp.getCurrState();
  iStat.reset();

  for(uint i = 0; i < nElem; ++i) {
    LPCTSTR p = Rows[i];
    svmObject* t = 0;
    int id = _ttoi(p);
    switch(id) {
      case PRN_TXT:
      case PRN_TXT_ON_BOX:
      case PRN_TXT_VAR:
      case PRN_TXT_ON_BOX_VAR:
        t = new svmObjText(this, ++globalIdLinked[getCurrPage()], 0, 0);
        break;
      case PRN_LINE:
        t = new svmObjLine(this, ++globalIdLinked[getCurrPage()], 0, 0);
        break;
      case PRN_BMP:
        t = new svmObjBitmap(this, ++globalIdLinked[getCurrPage()], 0, 0);
        break;
      case PRN_RECT:
      case PRN_ROUND_RECT:
        t = new svmObjRect(this, ++globalIdLinked[getCurrPage()], 0, 0);
        break;
      case PRN_OVAL:
        t = new svmObjEllipse(this, ++globalIdLinked[getCurrPage()], 0, 0);
        break;
      case PRN_SET_FONT:
      case PRN_SET_FONT2:
        iStat.loadFont(p, true);
        break;
      case PRN_SET_COLOR_FG:
        iStat.loadForeground(p);
        break;
      case PRN_SET_COLOR_BKG:
        iStat.loadBackground(p);
        break;
      case PRN_SET_ALIGN:
        iStat.loadAlign(p);
        break;
      case PRN_SET_BKMODE:
        iStat.loadBkMode(p);
        break;
      case PRN_SET_PEN:
        iStat.loadPen(p);
        break;
      case PRN_SET_BRUSH1:
      case PRN_SET_BRUSH2:
      case PRN_SET_NULL_BRUSH:
        iStat.loadBrush(p);
        break;
      }
    if(t) {
      t->setLinked();
      if(!t->load(p)) {
        --globalIdLinked[getCurrPage()];
        delete t;
        }
      else
        Base[getCurrPage()]->addObjectLinked(t);
      }
    }
}
//-----------------------------------------------------------
bool childScrollWorkArea::openPage()
{
  uint idVars[MAX_ID_DATA_VARS] = { 0 };
  if(IDOK == PDataProject(this, PDataProject::useForOPen, IDD_DIALOG_PROJECT, idVars).modal()) {
    FirstH = 0;
    FirstV = 0;
    Zoom = svmManZoom::zOne;
    setCurrPage(DataP.currPage);
    clearPage(false);
    TCHAR path[_MAX_PATH];
    dataProject& dp = getDataProject();
    _tcscpy_s(path, dp.getcurrPath());
    appendPath(path, dp.gettemplateName());
    TCHAR datapath[_MAX_PATH];
    _tcscpy_s(datapath, dp.getcurrPath());
    appendPath(datapath, dp.getvarFileName());
    return loadFileTemplate(path, datapath, idVars);
    }
  return false;
}
//-----------------------------------------------------------
void childScrollWorkArea::clearPage(bool all)
{
  manageVariable& mv = getManVariables();
  mv.reset();
  dataProject& dp = getDataProject();
  if(all)
    dp.clearAll();
  else
    dp.resetZoom();
  Base[getCurrPage()]->flushHistory();
  Base[getCurrPage()]->flushObject();
  clearDual();
  InvalidateRect(*this, 0, 0);
}
//-----------------------------------------------------------
static void appendExt(LPTSTR path, LPCTSTR ext)
{
  TCHAR e[_MAX_EXT];
  _tsplitpath_s(path, 0, 0, 0, 0, 0, 0, e, SIZE_A(e));
  if(*e)
    return;
  _tcscat_s(path, _MAX_PATH, ext);
}
//-----------------------------------------------------------
bool childScrollWorkArea::saveFileTemplate(LPCTSTR fileTempl, LPCTSTR fileVars)
{
  dataProject& dp = getDataProject();
  uint ids[MAX_ID_DATA_VARS] = { 0 };
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, dp.getcurrPath());
  appendPath(path, fileVars);
  manageVariable& dv = getManVariables();
  if(!dv.save(path, ids))
    return false;

  infoState& iStat = dp.getCurrState();
  iStat.reset4save();
  do {
    P_File pf(fileTempl);
    pf.appendback();
    } while(false);
  P_File pf(fileTempl, P_CREAT);
  if(!pf.P_open()) {
    pf.unappendback();
    return false;
    }
  bool success = Base[getCurrPage()]->save(pf, fileVars, ids);
  if(!success)
    pf.unappendback();
  return success;
}
//-----------------------------------------------------------
static LPCTSTR msgFontOver = _T("Ci sono oggetti con Font fuori range!\r\n")
    _T("Scegliere se impostarli al primo font,\r\nproseguire comunque\r\no annullare il salvataggio");
//-----------------------------------------------------------
bool childScrollWorkArea::savePage()
{
  uint nFont = getFontObj().getElem();
  if(isFontAbove(nFont - 1)) {
    switch(MessageBox(*this, msgFontOver, _T("Attenzione!"), MB_YESNOCANCEL | MB_ICONSTOP)) {
      case IDYES:
        resetFontIfAbove(nFont - 1);
        break;
      case IDCANCEL:
        return false;
      }
    }
  if(IDOK == PDataProject(this, PDataProject::useForSave, IDD_DIALOG_PROJECT).modal()) {
    TCHAR path[_MAX_PATH];
    dataProject& dp = getDataProject();
    _tcscpy_s(path, dp.getcurrPath());
    appendPath(path, dp.gettemplateName());
    appendExt(path, _T(".ptp"));
    TCHAR fileVars[_MAX_PATH];
    _tcscpy_s(fileVars, dp.getvarFileName());
    appendExt(fileVars, _T(".ptd"));

    if(saveFileTemplate(path, fileVars)) {
      Base[getCurrPage()]->flushHistory();
      setPageTitle();
      DataP.setDirty(false);
      return true;
      }
    MessageBox(*this, _T("Errore nel salvataggio!"), _T("Attenzione"), MB_OK | MB_ICONSTOP);
    }
  return false;
}
//-----------------------------------------------------------
void childScrollWorkArea::Align()
{
  enum idMenu { idLeft = 1000, idTop, idRight, idBottom, idSpaceLeft, idSpaceTop };
  menuPopup menu[] = {
    { MF_STRING, idLeft, _T("Sinistra") },
    { MF_STRING, idTop, _T("Alto") },
    { MF_STRING, idRight, _T("Destra") },
    { MF_STRING, idBottom, _T("Basso") },
    { MF_SEPARATOR, 0, 0 },
    { MF_STRING, idSpaceLeft, _T("Spaziatura Orizzontale") },
    { MF_STRING, idSpaceTop, _T("Spaziatura Verticale") },
    };
  int nItem = 4;
  if(Base[getCurrPage()]->canAlignSpace())
    nItem = SIZE_A(menu);

  switch(popupMenu(*this, menu, nItem)) {
    case idLeft:
      Base[getCurrPage()]->Align(svmBase::aLeft);
      break;
    case idTop:
      Base[getCurrPage()]->Align(svmBase::aBottom);
      break;
    case idRight:
      Base[getCurrPage()]->Align(svmBase::aRight);
      break;
    case idBottom:
      Base[getCurrPage()]->Align(svmBase::aTop);
      break;

    case idSpaceLeft:
      Base[getCurrPage()]->AlignSpace(svmBase::aLeft);
      break;
    case idSpaceTop:
      Base[getCurrPage()]->AlignSpace(svmBase::aTop);
      break;
    }
}
//-----------------------------------------------------------
void childScrollWorkArea::Order()
{
  enum idMenu { idTop = 1000, idPrevious, idNext, idBottom };
  menuPopup menu[] = {
    { MF_STRING, idTop, _T("Primo") },
    { MF_STRING, idPrevious, _T("Precedente") },
    { MF_STRING, idNext, _T("Successivo") },
    { MF_STRING, idBottom, _T("Ultimo") },
    };
  bool buff[svmBase::oMax];
  Base[getCurrPage()]->enableWichOrder(buff);
  for(int i = 0; i < SIZE_A(menu); ++i)
    if(!buff[i])
      menu[i].flag |= MF_GRAYED;

  switch(popupMenu(*this, menu, SIZE_A(menu))) {
    case idTop:
      Base[getCurrPage()]->Order(svmBase::oTop);
      break;
    case idPrevious:
      Base[getCurrPage()]->Order(svmBase::oPrevious);
      break;
    case idNext:
      Base[getCurrPage()]->Order(svmBase::oNext);
      break;
    case idBottom:
      Base[getCurrPage()]->Order(svmBase::oBottom);
      break;
    }
}
//-----------------------------------------------------------
void childScrollWorkArea::setActionHide()
{
  actionHide = !actionHide;

  if(actionHide)
    Base[getCurrPage()]->addBtnStat(svmBase::eBS_Hide);
  else
    Base[getCurrPage()]->remBtnStat(svmBase::eBS_Hide);
  if(!Base[getCurrPage()]->setActionHide(actionHide)) {
    if(actionHide) {
      Base[getCurrPage()]->remBtnStat(svmBase::eBS_Hide);
      setBtnStat();
      }
    }
}
//-----------------------------------------------------------
void childScrollWorkArea::setActionLock()
{
  actionLock = !actionLock;

  if(actionLock)
    Base[getCurrPage()]->addBtnStat(svmBase::eBS_Lock);
  else
    Base[getCurrPage()]->remBtnStat(svmBase::eBS_Lock);

  if(!Base[getCurrPage()]->setActionLock(actionLock)) {
    if(actionLock) {
      Base[getCurrPage()]->remBtnStat(svmBase::eBS_Lock);
      setBtnStat();
      }
    }
}
//-----------------------------------------------------------
HDC childScrollWorkArea::getDC()
{
  HDC hdc = GetDC(*this);
  SetMapMode(hdc, MM_LOMETRIC);
  SetGraphicsMode(hdc, GM_ADVANCED);
  setTransform(hdc);
  return hdc;
}
//-----------------------------------------------------------
void childScrollWorkArea::releaseDC(HDC hdc)
{
  resetTransform(hdc);
  SetGraphicsMode(hdc, GM_COMPATIBLE);
  SetMapMode(hdc, MM_TEXT);
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------
int popupMenu(HWND hwnd, menuPopup* item, int num, POINT* pt)
{
  HMENU hmenu = CreatePopupMenu();
  if(!hmenu)
    return 0;
  for(int i = 0; i < num; ++i)
    AppendMenu(hmenu, item[i].flag, item[i].id, item[i].text);
  const uint style = TPM_RETURNCMD | TPM_RIGHTALIGN | TPM_TOPALIGN;
  POINT Pt;
  if(pt)
    Pt = *pt;
  else
    GetCursorPos(&Pt);
  int result = TrackPopupMenu(hmenu, style, Pt.x, Pt.y, 0, hwnd, 0);
  DestroyMenu(hmenu);
  return result;
}
//-----------------------------------------------------------
void childScrollWorkArea::evMouse(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_MOUSEMOVE:
      evMouseMove(message, wParam, lParam);
      SetFocus(*this);
      break;

    case WM_LBUTTONDOWN:
      evMouseLDown(message, wParam, lParam);
      break;

    case WM_LBUTTONUP:
      evMouseLUp(message, wParam, lParam);
      break;

    case WM_RBUTTONDOWN:
      SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
      break;

    case WM_RBUTTONUP:
      evMouseRUp(message, wParam, lParam);
      break;
    }
}
//-----------------------------------------------------------
static void drawFocusRect(HDC hdc, const PRect& norm)
{
  HPEN pen = CreatePen(PS_DOT, 1, RGB(127, 127, 127));
  HGDIOBJ oldPen = SelectObject(hdc, pen);
  int old = SetROP2(hdc, R2_XORPEN);
  PRect r(norm);
  MoveToEx(hdc, r.left, r.top, 0);
  LineTo(hdc, r.left, r.bottom);
  LineTo(hdc, r.right, r.bottom);
  LineTo(hdc, r.right, r.top);
  LineTo(hdc, r.left, r.top);
  SelectObject(hdc, oldPen);
  SetROP2(hdc, old);
  DeleteObject(pen);
}
//-----------------------------------------------------------
static bool keyForPan()
{
  return (GetKeyState(VK_MENU) & 0x8000) && (GetKeyState(VK_CONTROL) & 0x8000);
}
//-----------------------------------------------------------
bool childScrollWorkArea::canPan()
{
  if(tPAN == currTools)
    return true;
  if(keyForPan())
    return toBool(SendMessage(*getParent(), WM_HAS_SCROLL, 0, 0));
  return false;
}
//-----------------------------------------------------------
//#define FIXED_MULTIPLE_ADD
static inline
bool continueAdd(bool flag)
{
#ifdef FIXED_MULTIPLE_ADD
  return flag;
#else
  return needMultipleAdd();
#endif
}
//-----------------------------------------------------------
void childScrollWorkArea::setBtnStat()
{
  DWORD stat = Base[getCurrPage()]->getBtnStat();
  actionLock = toBool(stat & svmBase::eBS_Lock);
  Actions->selectBtn(tGROUP_LOCK, actionLock);

  actionHide = toBool(stat & svmBase::eBS_Hide);
  Actions->selectBtn(tGROUP_HIDE, actionHide);
  bool hasPrj = toBool(stat & svmBase::eBS_hasProject);

  Tools->enableBtn(tTEXT, hasPrj);
  Tools->enableBtn(tLINE, hasPrj);
  Tools->enableBtn(tRECT, hasPrj);
  Tools->enableBtn(tELLIPSE, hasPrj);
  Tools->enableBtn(tPICTURE, hasPrj);

  Page->enableBtn(tVARS, hasPrj);
  Page->enableBtn(tLINKED, hasPrj);

}
//-----------------------------------------------------------
void childScrollWorkArea::evMouseMove(UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  if(onPan) {
    SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN_MOVE)));
    static bool inExec;
    if(inExec)
      return;
    inExec = true;
    POINT pt2 = pt;
    pt2.x = frame.left - pt.x;
    pt2.y = frame.top - pt.y;
    if(pt2.x || pt2.y) {
      SendMessage(*getParent(), WM_SEND_PAN, 0, (LPARAM)&pt2);
      }
    SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN_MOVE)));
    // svuota la coda dei messaggi dovuta al movimento della window
    MSG msg;
    while(PeekMessage(&msg, *this, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
      if(WM_LBUTTONUP == msg.message) {
        evMouseLUp(msg.message, msg.wParam, msg.lParam);
        break;
        }
      }
    SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN_MOVE)));
    GetCursorPos(&pt);
    MapWindowPoints(HWND_DESKTOP, *this, &pt, 1);
    frame.left = pt.x;
    frame.top = pt.y;
    inExec = false;
    return;
    }
  if(canPan()) {
    SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
    return;
    }
  HDC hdc = getDC();
  DPtoLP(hdc, &pt, 1);

  switch(currAction) {
    case svmBase::select:
    // se il rettangolo non è vuoto occorre cancellare il disegno precedente
      if(frame.Width() || frame.Height()) {
        PRect norm(frame);
        normalizeYNeg(norm);
        drawFocusRect(hdc, norm);
        }
      break;
    case svmBase::moving:
      drawXorBlock(hdc, pt);
      break;
    case svmBase::sizing:
      drawXorObject(hdc, pt);
      break;
    }

  // assegna nuova posizione
  frame.right = pt.x;
  frame.bottom = pt.y;

  if(frame.Width() || frame.Height()) {
    PRect norm(frame);
    normalizeYNeg(norm);

    switch(currAction) {
      case svmBase::select:
        drawFocusRect(hdc, norm);
        break;
      }
    }
  if(svmBase::none == currAction)
    showAnchorType(pt, wParam);
  releaseDC(hdc);
}
//-----------------------------------------------------------
void childScrollWorkArea::evMouseLDown(UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
  SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  if(canPan()) {
    onPan = true;
    SetCapture(*this);
    frame.left = pt.x;
    frame.top = pt.y;
    SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN_MOVE)));
    return;
    }

  HDC hdc = getDC();
  DPtoLP(hdc, &pt, 1);

  // se currTools è zero, si attiva la selezione, a meno che non si sia sopra
  // un oggetto, allora occorre controllare se si vuole spostare o ridimensionare
  // e se premuto lo shift aggiungere l'oggetto alla selezione attuale
  if(tSELECT == currTools) {
    SetCapture(*this);
    currAction = findPoint(hdc, pt, wParam);
    frame.left = pt.x;
    frame.top = pt.y;
    frame.right = pt.x;
    frame.bottom = pt.y;
    if(svmBase::none == currAction)
      ReleaseCapture();
    else if(svmBase::select != currAction)
      beginXor(pt);
    }
  releaseDC(hdc);
}
//-----------------------------------------------------------
void childScrollWorkArea::evMouseLUp(UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  if(onPan) {
    onPan = false;
    SetCursor(LoadCursor(0, IDC_ARROW));
    ReleaseCapture();
    return;
    }
  HDC hdc = getDC();
  DPtoLP(hdc, &pt, 1);

  if(tSELECT != currTools) {
    svmObject* t = addObject(hdc, pt, currTools);
    if(!continueAdd(multipleAddTools)) {
      Tools->selectBtn(tSELECT);
      currTools = tSELECT;
      baseCursor = IDC_ARROW;
      }
    }
  else {
    if(svmBase::none != currAction)  {
      ReleaseCapture();
      SetCursor(LoadCursor(0, IDC_ARROW));

      PRect norm(frame);
      normalizeYNeg(norm);
      switch(currAction) {
        case svmBase::select:
          drawFocusRect(hdc, norm);
//          getDataProject().calcFromScreen(norm);
          findObjects(norm, wParam);
          break;
        case svmBase::moving:
          drawXorBlockFinal(hdc, pt);
          break;
        case svmBase::sizing:
          drawXorObjectFinal(hdc, pt);
          break;
        }
      showSelected(hdc);
      currAction = svmBase::none;
      }
    }
  releaseDC(hdc);
}
//-----------------------------------------------------------
void childScrollWorkArea::performMoveByKey(int key)
{
  SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  if(!isMovingByKey) {
    isMovingByKey = 1;
    ptMoveByKey.x = 0;
    ptMoveByKey.y = 0;
    beginXor(ptMoveByKey);
    }
  switch(key) {
    case VK_LEFT:
      --ptMoveByKey.x;
      break;
    case VK_RIGHT:
      ++ptMoveByKey.x;
      break;
    case VK_UP:
      ++ptMoveByKey.y;
      break;
    case VK_DOWN:
      --ptMoveByKey.y;
      break;
    }

  HDC hdc = getDC();
  drawXorBlock(hdc, ptMoveByKey);
  releaseDC(hdc);
}
//-----------------------------------------------------------
void childScrollWorkArea::performSizeByKey(int key)
{
  SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  ptMoveByKey.x = 0;
  ptMoveByKey.y = 0;
  if(!isMovingByKey) {
    isMovingByKey = 2;
    beginXorSizeByKey();
    }
  switch(key) {
    case VK_LEFT:
      --ptMoveByKey.x;
      break;
    case VK_RIGHT:
      ++ptMoveByKey.x;
      break;
    case VK_UP:
      ++ptMoveByKey.y;
      break;
    case VK_DOWN:
      --ptMoveByKey.y;
      break;
    }

  HDC hdc = getDC();
  drawResizeAllObject(hdc, ptMoveByKey);
  releaseDC(hdc);
}
//-----------------------------------------------------------
void childScrollWorkArea::performEndKey()
{
  if(!isMovingByKey)
    return;
  SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  HDC hdc = getDC();
  if(1 == isMovingByKey) {
    if(!(GetKeyState(VK_SHIFT) & 0x8000)) {
      isMovingByKey = 0;
      drawXorBlockFinal(hdc, ptMoveByKey);
      }
    }
  else if(2 == isMovingByKey) {
    if(!(GetKeyState(VK_CONTROL) & 0x8000)) {
      isMovingByKey = 0;
      drawResizeAllObjectFinal(hdc);
      }
    }
  releaseDC(hdc);
}
//-----------------------------------------------------------
void childScrollWorkArea::setAddLock()
{
  Base[getCurrPage()]->addBtnStat(1);
  actionLock = true;
  Base[getCurrPage()]->setActionLock(actionLock);
  setBtnStat();
}
//-----------------------------------------------------------
void childScrollWorkArea::setAddHide()
{
  Base[getCurrPage()]->addBtnStat(2);
  actionHide = true;
  Base[getCurrPage()]->setActionHide(actionHide);
  setBtnStat();
}
//-----------------------------------------------------------
void childScrollWorkArea::evMouseRUp(UINT message, WPARAM wParam, LPARAM lParam)
{
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  bool changed;
  bool onSel;
  svmObject::typeOfAnchor anchor = Base[getCurrPage()]->getAnchor(pt, tipsText, SIZE_A(tipsText), changed, onSel);

  enum idMenu { idCopy = 1000, idPaste, idDup, idLock, idHide, idFlushPaste, idOffset };
  menuPopup menu[] = {
    { MF_STRING, idCopy, _T("CTRL+C -> Copia") },
    { MF_STRING, idPaste, _T("CTRL+V -> Incolla") },
    { MF_STRING, idFlushPaste, _T("CTRL+E -> Scarica copiati") },
    { MF_SEPARATOR, 0, 0 },
    { MF_STRING, idDup, _T("CTRL+D -> Duplica") },
    { MF_SEPARATOR, 0, 0 },
    { MF_STRING, idLock, _T("CTRL+L -> Blocca") },
    { MF_STRING, idHide, _T("CTRL+H -> Nascondi") },
    { MF_SEPARATOR, 0, 0 },
    { MF_STRING, idOffset, _T("CTRL+O -> Offset Variabili") },
    };

  bool hasCopied = Base[getCurrPage()]->canPaste();
  if(!Base[getCurrPage()]->canProperty() && svmObject::Extern == anchor) {
    if(!hasCopied)
      return;

    menu[0].flag |= MF_GRAYED;
    menu[4].flag |= MF_GRAYED;
    }

  if(!hasCopied) {
    menu[1].flag |= MF_GRAYED;
    menu[2].flag |= MF_GRAYED;
    }
  // se il click dx avviene su un oggetto non selezionato, simula il click sx
  if(!onSel) {
    uint old = currAction;
    currAction = svmBase::select;
    evMouseLDown(message, wParam, lParam);
    evMouseLUp(message, wParam, lParam);
    currAction = old;
//    menu[9].flag |= MF_GRAYED;
    }

  const int nItem = SIZE_A(menu);

  uint msg = 0;
  switch(popupMenu(*this, menu, nItem)) {
    case idCopy:
      msg = IDCM_COPY;
      break;
    case idPaste:
      msg = IDCM_PASTE;
      break;
    case idDup:
      msg = IDCM_DUP;
      break;
    case idLock:
      msg = IDCM_LOCK;
      break;
    case idHide:
      msg = IDCM_HIDE;
      break;
    case idFlushPaste:
      msg = IDCM_FLUSH;
      break;
    case idOffset:
      msg = IDCM_OFFSET;
      break;
    }
  if(msg)
    PostMessage(*this, WM_COMMAND, MAKEWPARAM(msg, 0), 0);
}
//-----------------------------------------------------------
void childScrollWorkArea::showAnchorType(const POINT& pt, int keyFlag)
{
  bool changed;
  bool dummy;
  svmObject::typeOfAnchor anchor = Base[getCurrPage()]->getAnchor(pt, tipsText, SIZE_A(tipsText), changed, dummy);
  if(svmObject::Extern != anchor) {
    if(changed)
      SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
    SendMessage(hwndTips, TTM_ACTIVATE, TRUE, 0);
    POINT pt2 = pt;
    {
    HDC hdc = getDC();
    LPtoDP(hdc, &pt2, 1);
    releaseDC(hdc);
    }
    MSG Msg = { *this, WM_MOUSEMOVE, keyFlag, MAKELPARAM(pt2.x, pt2.y), GetTickCount() };
    Msg.pt = pt2;
    SendMessage(hwndTips, TTM_RELAYEVENT, 0, (LPARAM)&Msg);
    }
  else
    SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  setCursor(anchor, false);
}
//-----------------------------------------------------------
void childScrollWorkArea::beginXor(const POINT& pt)
{
  svmObject::typeOfAnchor anchor = Base[getCurrPage()]->beginXor(pt);
  setCursor(anchor, true);
}
//-----------------------------------------------------------
void childScrollWorkArea::beginXorSizeByKey()
{
  Base[getCurrPage()]->beginXorSizeByKey();
}
//-----------------------------------------------------------
void childScrollWorkArea::setCursor(uint anchor, bool forceChange)
{
  LPCTSTR cursor = baseCursor;
  if(IDC_CROSS != cursor) {
    switch(anchor) {
      case svmObject::Extern:
        if(!forceChange)
          break;
      case svmObject::Inside:
        cursor = IDC_SIZEALL;
        break;

      case svmObject::LeftTop:
      case svmObject::RightBottom:
        cursor = IDC_SIZENWSE;
        break;

      case svmObject::Top:
      case svmObject::Bottom:
        cursor = IDC_SIZENS;
        break;

      case svmObject::RightTop:
      case svmObject::LeftBottom:
        cursor = IDC_SIZENESW;
        break;

      case svmObject::Right:
      case svmObject::Left:
        cursor = IDC_SIZEWE;
        break;
      }
    }

  SetCursor(LoadCursor(0, cursor));
}
//-----------------------------------------------------------
void childScrollWorkArea::drawXorBlock(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawXorBlock(hdc, pt);
}
//-----------------------------------------------------------
void childScrollWorkArea::drawXorBlockFinal(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawXorBlockFinal(hdc, pt);
}
//-----------------------------------------------------------
void childScrollWorkArea::drawXorObject(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawXorObject(hdc, pt);
}
//-----------------------------------------------------------
void childScrollWorkArea::drawXorObjectFinal(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawXorObjectFinal(hdc, pt);
}
//-----------------------------------------------------------
void childScrollWorkArea::drawResizeAllObjectFinal(HDC hdc)
{
  Base[getCurrPage()]->drawResizeAllObjectFinal(hdc);
}
//-----------------------------------------------------------
void childScrollWorkArea::drawResizeAllObject(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawResizeAllObject(hdc, pt);
}
//-----------------------------------------------------------
void childScrollWorkArea::findObjects(const PRect& frame, int keyFlag)
{
  Base[getCurrPage()]->findObjects(frame, keyFlag);
}
//-----------------------------------------------------------
void childScrollWorkArea::showSelected(HDC hdc)
{
  Base[getCurrPage()]->showSelected(hdc);
}
//-----------------------------------------------------------
void childScrollWorkArea::unselectAll(HDC hdc)
{
  Base[getCurrPage()]->unselectAll(hdc);
}
//-----------------------------------------------------------
int childScrollWorkArea::findPoint(HDC hdc, const POINT&pt, int keyFlag)
{
  return Base[getCurrPage()]->findPoint(hdc, pt, keyFlag);
}
//-----------------------------------------------------------
svmObject* childScrollWorkArea::addObject(HDC hdc, const POINT& pt, UINT currTools)
{
  svmObject *t = 0;
  switch(currTools) {
    case tTEXT:
      t = new svmObjText(this, ++globalId[getCurrPage()], pt.x, pt.y);
      break;
    case tLINE:
      t = new svmObjLine(this, ++globalId[getCurrPage()], pt.x, pt.y);
      break;
    case tRECT:
      t = new svmObjRect(this, ++globalId[getCurrPage()], pt.x, pt.y);
      break;
    case tELLIPSE:
      t = new svmObjEllipse(this, ++globalId[getCurrPage()], pt.x, pt.y);
      break;
    case tPICTURE:
      t = new svmObjBitmap(this, ++globalId[getCurrPage()], pt.x, pt.y);
      break;
    }
  if(t) {
    svmObjHistory::typeOfAction result = t->dialogProperty();
    if(svmObjHistory::Abort == result) {
      delete t;
      return 0;
      }
    Base[getCurrPage()]->addObject(t, hdc);
    }
  return t;
}
//-----------------------------------------------------------
bool childScrollWorkArea::idle(DWORD count)
{
  bool enable = Base[getCurrPage()]->canUndo();
  Actions->enableBtn(0, enable);

  enable = Base[getCurrPage()]->canAlign();
  Actions->enableBtn(1, enable);

  enable = Base[getCurrPage()]->canOrder();
  Actions->enableBtn(2, enable);

  enable = Base[getCurrPage()]->canReInside();
  Actions->enableBtn(8, enable);

  return baseClass::idle(count);
}
//-----------------------------------------------------------
bool childScrollWorkArea::okToDone(bool all)
{
//  return true;
  if(all) {
    for(int i = 0; i < SIZE_A(Base); ++i) {
      if(!Base[i]->canUndo() && !DataP.getDirty(i))
        continue;
      TCHAR buff[_MAX_PATH];
      if(*DataP.gettemplateName(i))
        _tcscpy_s(buff, DataP.gettemplateName(i));
      else
        wsprintf(buff, _T("Nuova pagina %d"), i + 1);

      int result = MessageBox(*this,
            _T("I Dati sono cambiati\r\nVuoi salvare la pagina?"),
            buff, MB_YESNOCANCEL | MB_ICONINFORMATION);
      int old = DataP.currPage;
      switch(result) {
        case IDYES:
          DataP.currPage = i;
          result = savePage();
          DataP.currPage = old;
          if(result)
            break;

        case IDCANCEL:
          return false;
        }
      }
    return true;
  }



  if(!Base[getCurrPage()]->canUndo() && !DataP.getDirty())
    return true;
  int result = MessageBox(*this,
            _T("Vuoi salvare la pagina?"),
            _T("I Dati sono cambiati"), MB_YESNOCANCEL | MB_ICONINFORMATION);
  switch(result) {
    case IDYES:
      if(!savePage())
        break;

    case IDNO:
      return true;
    }
  return false;
}
//-----------------------------------------------------------
bool childScrollWorkArea::isUsedFontId(uint idfont)
{
  return Base[getCurrPage()]->isUsedFontId(idfont);
}
//-----------------------------------------------------------
void childScrollWorkArea::getUsedFontId(PVect<uint>& set)
{
   Base[getCurrPage()]->getUsedFontId(set);
}
//-----------------------------------------------------------
void childScrollWorkArea::decreaseFontIfAbove(uint id_font)
{
   if(Base[getCurrPage()]->decreaseFontIfAbove(id_font))
      getDataProject().setDirty(true);
}
//-----------------------------------------------------------
bool childScrollWorkArea::resetFontIfAbove(uint id_font)
{
  return Base[getCurrPage()]->resetFontIfAbove(id_font);
}
//-----------------------------------------------------------
bool childScrollWorkArea::isFontAbove(uint id_font)
{
  return Base[getCurrPage()]->isFontAbove(id_font);
}

