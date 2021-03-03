//--------------------- svmMainClient.cpp ------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include "svmMainClient.h"
#include "pToolBar.h"
#include "svmBase.h"
#include "svm_AllObj.h"
#include "sizer.h"
#include "svmPageProperty.h"
#include "p_file.h"
#include "dChoosePath.h"
#include "dManageStdMsg.h"
#include "id_btn.h"
#include "alarmEditorDlg.h"
#include "groupObject.h"
#include "svMkVersion.h"
#include "PDiagMirror.h"
//----------------------------------------------------------------------------
static UINT globalId[MAX_PAGES] = { 0 };
//----------------------------------------------------------------------------
static UINT globalIdLinked[MAX_PAGES] = { 0 };
//----------------------------------------------------------------------------
static bool ToggleUnicode = true;
bool saveAsUnicode() { return ToggleUnicode; }
//----------------------------------------------------------------------------
static bool ToggleXor;
bool isColoredXor() { return ToggleXor; }
//----------------------------------------------------------------------------
extern void gRunGlobalAlarmTExt(PWin* parent);
//----------------------------------------------------------------------------
svmMainClient* getMainClient(PWin* w)
{
  while(w) {
    svmMainClient* mc = dynamic_cast<svmMainClient*>(w);
    if(mc)
      return mc;
    w = w->getParent();
    }
  return 0;
}
//----------------------------------------------------------------------------
bool getCoord(LPCTSTR sub, DWORD& target)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  LPCTSTR value = ini.getValue(WINTOOLS_COORD, sub);
  if(value) {
    __int64 t = _tcstoi64(value, 0, 10);
    target = (DWORD)t;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool setCoord(LPCTSTR sub, DWORD source)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), source);
  ini_param param = { WINTOOLS_COORD, t };
  ini.addParam(param, sub);
  return ini.save();
}
//----------------------------------------------------------------------------
bool getBorderOffset(DWORD& target)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  LPCTSTR value = ini.getValue(ADD_BORDER, MAIN_PATH);
  if(value) {
    __int64 t = _tcstoi64(value, 0, 10);
    target = (DWORD)t;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool setBorderOffset(DWORD source)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), source);
  ini_param param = { ADD_BORDER, t };
  ini.addParam(param, MAIN_PATH);
  return ini.save();
}
//----------------------------------------------------------------------------
bool getRegText(LPCTSTR sub, LPCTSTR keyVal, LPTSTR target, size_t dim)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  LPCTSTR value = ini.getValue(keyVal, sub);
  if(value) {
    _tcscpy_s(target, dim, value);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool setRegText(LPCTSTR sub, LPCTSTR keyVal, LPCTSTR source)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  ini_param param = { keyVal, source };
  ini.addParam(param, sub);
  return ini.save();
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
    tBUTTON,
    tTEXT,
    tEDIT,
    tPANEL,
    tPICTURE,
    tBARGRAPH,
    tLED,
    tLISTBOX,
    tDIAMETER,
    tCHOOSE,
    tCURVE,
    tCAM,
    tXMETER,
    tXSCOPE,
    tOBJ_ALARM,
    tOBJ_TREND,
    tOBJ_SPIN,
    tOBJ_TABLE_INFO,
    tOBJ_PLOT_XY,
    tOBJ_SLIDER,

    tTOOL_LAST
    };
//-----------------------------------------------------------
enum typeActions {
    tUNDO,
    tREDO,
    tALIGN,
    tORDER,
    tPROPERTY,
    tGROUP_LOCK,
    tGROUP_HIDE,
    tMIRROR,
    tGROUP,
    tUNGROUP,
    tUNGROUPALL,
    tBACKGROUND,
    tFONT,
    tCUSTOM,
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
    tOPEN_PAGE_FROM_OTHER_PRJ,
    tSTD_MSG,
    tALARM,
    tG_TXT_ALARM,
    tG_TXT,
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

enum idCmd {  IDM_ARROW = 100,
              IDM_PAN,
              IDM_BUTTON,
              IDM_TEXT,
              IDM_EDIT,
              IDM_RECT,
              IDM_PICTURE,
              IDM_BARGRAPH,
              IDM_LED,
              IDM_LISTBOX,
              IDM_DIAMTER,
              IDM_CHOOSE,
              IDM_CURVE,
              IDM_CAM,
              IDM_XMETER,
              IDM_XSCOPE,
              IDM_OBJ_ALARM,
              IDM_OBJ_TREND,
              IDM_OBJ_SPIN,
              IDM_OBJ_TABLE_INFO,
              IDM_OBJ_PLOT_XY,
              IDM_OBJ_SLIDER,

              IDM_UNDO,
              IDM_REDO,
              IDM_ALIGN,
              IDM_ORDER,
              IDM_PROPERTY,
              IDM_GROUP_LOCK,
              IDM_GROUP_HIDE,
              IDM_MIRROR,
              IDM_GROUP,
              IDM_UNGROUP,
              IDM_UNGROUPALL,
              IDM_BKGND,
              IDM_FONT,
              IDM_CUSTOM,
              IDM_TOGGLE_XOR,
              IDM_RESTORE_INSIDE,

              IDM_NEW,
              IDM_OPEN,
              IDM_SAVE,
              IDM_CLEAR,
              IDM_OPEN_PAGE_FROM_OTHER_PRJ,
              IDM_STD_MSG,
              IDM_ALARM,
              IDM_G_TXT_ALARM,
              IDM_G_TXT,
              IDM_DONE,

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
#define TIP(a) (IDM_ARROW + (a))
//-----------------------------------------------------------
#define MAKE_BTN_ROW(a, id)  { (a), id, TBSTATE_ENABLED, MY_TBSTYLE1, 0L, TIP(a) }
//-----------------------------------------------------------
TBBUTTON tbButtons[] = {
  MAKE_BTN_ROW( 0, IDM_ARROW),
  MAKE_BTN_ROW( 1, IDM_PAN),
  MAKE_BTN_ROW( 2, IDM_BUTTON),
  MAKE_BTN_ROW( 3, IDM_TEXT),
  MAKE_BTN_ROW( 4, IDM_EDIT),
  MAKE_BTN_ROW( 5, IDM_RECT),
  MAKE_BTN_ROW( 6, IDM_PICTURE),
  MAKE_BTN_ROW( 7, IDM_BARGRAPH),
  MAKE_BTN_ROW( 8, IDM_LED),
  MAKE_BTN_ROW( 9, IDM_LISTBOX),
  MAKE_BTN_ROW(10, IDM_DIAMTER),
  MAKE_BTN_ROW(11, IDM_CHOOSE),
  MAKE_BTN_ROW(12, IDM_CURVE),
  MAKE_BTN_ROW(13, IDM_CAM),
  MAKE_BTN_ROW(14, IDM_XMETER),
  MAKE_BTN_ROW(15, IDM_XSCOPE),
  MAKE_BTN_ROW(16, IDM_OBJ_ALARM),
  MAKE_BTN_ROW(17, IDM_OBJ_TREND),
  MAKE_BTN_ROW(18, IDM_OBJ_SPIN),
  MAKE_BTN_ROW(19, IDM_OBJ_TABLE_INFO),
  MAKE_BTN_ROW(20, IDM_OBJ_PLOT_XY),
  MAKE_BTN_ROW(21, IDM_OBJ_SLIDER)
};
//-----------------------------------------------------------
#undef MAKE_BTN_ROW
//-----------------------------------------------------------
#define TIP2(a) (IDM_UNDO + (a))
//-----------------------------------------------------------
#define MAKE_BTN_ROW(a, id, st)  { (a), id, TBSTATE_ENABLED, MY_TBSTYLE##st, 0L, TIP2(a) }
//-----------------------------------------------------------
TBBUTTON tbButtonsActions[] = {
  MAKE_BTN_ROW( 0, IDM_UNDO,        2),
  MAKE_BTN_ROW( 1, IDM_REDO,        2),
  MAKE_BTN_ROW( 2, IDM_ALIGN,       2),
  MAKE_BTN_ROW( 3, IDM_ORDER,       2),
  MAKE_BTN_ROW( 4, IDM_PROPERTY,    2),
  MAKE_BTN_ROW( 5, IDM_GROUP_LOCK,  3),
  MAKE_BTN_ROW( 6, IDM_GROUP_HIDE,  3),
  MAKE_BTN_ROW( 7, IDM_MIRROR,      2),
  MAKE_BTN_ROW( 8, IDM_GROUP,       2),
  MAKE_BTN_ROW( 9, IDM_UNGROUP,     2),
  MAKE_BTN_ROW(10, IDM_UNGROUPALL,  2),
  MAKE_BTN_ROW(11, IDM_BKGND,       3),
  MAKE_BTN_ROW(12, IDM_FONT,        2),
  MAKE_BTN_ROW(13, IDM_CUSTOM,      2),
  MAKE_BTN_ROW(14, IDM_TOGGLE_XOR,  3),
  MAKE_BTN_ROW(15, IDM_RESTORE_INSIDE, 2)
};
//-----------------------------------------------------------
#undef MAKE_BTN_ROW
//-----------------------------------------------------------
#define TIP3(a) (IDM_NEW + (a))
//-----------------------------------------------------------
#define MAKE_BTN_ROW(a, id)  { (a), id, TBSTATE_ENABLED, MY_TBSTYLE2, 0L, TIP3(a) }
//-----------------------------------------------------------
TBBUTTON tbButtonsPage[] = {
  MAKE_BTN_ROW(0, IDM_NEW),
  MAKE_BTN_ROW(1, IDM_OPEN),
  MAKE_BTN_ROW(2, IDM_SAVE),
  MAKE_BTN_ROW(3, IDM_CLEAR),
  MAKE_BTN_ROW(4, IDM_OPEN_PAGE_FROM_OTHER_PRJ),
  MAKE_BTN_ROW(5, IDM_STD_MSG),
  MAKE_BTN_ROW(6, IDM_ALARM),
  MAKE_BTN_ROW(7, IDM_G_TXT_ALARM),
  MAKE_BTN_ROW(8, IDM_G_TXT),
  MAKE_BTN_ROW(9, IDM_DONE),
};
//-----------------------------------------------------------
#undef MAKE_BTN_ROW
//-----------------------------------------------------------
#define TIP4(a) (IDM_ONE_P + (a))
//-----------------------------------------------------------
#define MAKE_BTN_ROW(a, id)  { (a), id, TBSTATE_ENABLED, MY_TBSTYLE1, 0L, TIP4(a) }
//-----------------------------------------------------------
TBBUTTON tbButtonsChgPage[] = {
  MAKE_BTN_ROW(0, IDM_ONE_P),
  MAKE_BTN_ROW(1, IDM_TWO_P),
  MAKE_BTN_ROW(2, IDM_THREE_P),
  MAKE_BTN_ROW(3, IDM_FOUR_P),
  MAKE_BTN_ROW(4, IDM_FIVE_P),
  MAKE_BTN_ROW(5, IDM_SIX_P),
  MAKE_BTN_ROW(6, IDM_SEVEN_P),
  MAKE_BTN_ROW(7, IDM_EIGHT_P),
};
//-----------------------------------------------------------
#undef MAKE_BTN_ROW
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
  _T("Pulsante"),
  _T("Testo fisso o variabile"),
  _T("Edit"),
  _T("Pannello"),
  _T("Immagine"),
  _T("Bar Graph"),
  _T("Display a Led"),
  _T("ListBox"),
  _T("Diametri"),
  _T("Scelta Lista"),
  _T("Curve"),
  _T("Cam"),
  _T("xMeter"),
  _T("xScope"),
  _T("Allarmi"),
  _T("Trend"),
  _T("Spin"),
  _T("Tabella Info"),
  _T("Plot XY"),
  _T("Slider"),
  };
//-----------------------------------------------------------
static LPCTSTR tipsActions[] = {
  _T("Annulla"),
  _T("Ripristina"),
  _T("Allineamento"),
  _T("Ordine"),
  _T("Proprietà"),
  _T("Blocca oggetti"),
  _T("Nascondi oggetti"),
  _T("Ribalta a specchio"),
  _T("Raggruppa"),
  _T("Separa"),
  _T("Separa tutto"),
  _T("Attiva azioni su\r\nimmagini di sfondo"),
  _T("Modifica Font"),
  _T("Gestione codici\r\npersonalizzati e/o\r\nper pagine predefinite"),
  _T("Selezioni Colorate "),
  _T("Ripristina fuori campo"),
  };
//-----------------------------------------------------------
static LPCTSTR tipsPage[] = {
  _T("Nuovo ..."),
  _T("Apri Pagina"),
  _T("Salva Pagina Corrente"),
  _T("Pulisci Pagina Corrente"),
  _T("Apre pagina da un altro progetto"),
  _T("Gestione Base_Data"),
  _T("Gestione pagine Allarmi"),
  _T("Modifica Testi Allarmi associati"),
  _T("Modifica Testi globale"),
  _T("Fine"),
  };
//-----------------------------------------------------------
static LPCTSTR tipsChgPage[] = {
  _T("Vai a Pagina 1"),
  _T("Vai a Pagina 2"),
  _T("Vai a Pagina 3"),
  _T("Vai a Pagina 4"),
  _T("Vai a Pagina 5"),
  _T("Vai a Pagina 6"),
  _T("Vai a Pagina 7"),
  _T("Vai a Pagina 8"),
  };
//-----------------------------------------------------------
PToolBarInfo iTools = {
    ID_TOOLBAR1,
    tbButtons,
    SIZE_A(tbButtons),
    IDB_TOOLBAR,
//    1, // 2, 5,
    2,

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
    2,
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
    2,
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
UINT getGlobalId()
{
  return ++globalId[DataP.currPage];
}
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
  if(*DataP.getPageName(ix))
    return (LPTSTR)DataP.getPageName(ix);
  return baseClass::getTips(id);
}
//-----------------------------------------------------------
#define BKG_COORD RGB(0xd0, 0xff, 0xff)
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
bool PWinToolsCoord::create()
{
  HFONT hf = D_FONT(20, 0, fBOLD, _T("arial"));
  PStatic* st = new PStatic(this, IDC_ST_COORD_X, 2, 1, 50, 22);
  // N.B. il primo controllo elimina il font, tutti i successivi devono
  // impostare l'autodelete a false
  st->setFont(hf, true);
  st = new PStatic(this, IDC_ST_COORD_Y, 54, 1, 50, 22);
  st->setFont(hf, false);
  st = new PStatic(this, IDC_ST_COORD_X2, 2, 26, 50, 22);
  st->setFont(hf, false);
  st = new PStatic(this, IDC_ST_COORD_Y2, 54, 26, 50, 22);
  st->setFont(hf, false);
#if 1
  Btn = new POwnBtn(this, IDC_BTN_ZOOM, 108, 1, 50, 24, _T("x 1"));
  POwnBtn::colorRect cr = Btn->getColorRect();
  cr.bkg = RGB(178, 202, 210);
  cr.focus = RGB(208, 255, 255);
  cr.txt = RGB(0, 0, 128);
  Btn->setColorRect(cr);
#else
  // dà problemi col focus, occorre cliccare due volte, allora meglio usare il pulsante standard
  Btn = new POwnBtnImageStd(this, IDC_BTN_ZOOM, 108, 1, 50, 24, 0,  POwnBtnImageStd::wLeft, false, _T("x 1"));
#endif
  Btn->setFont(hf, false);
  Attr.style |= WS_VISIBLE;
  PRect r(0, 0, 158, 18);
  AdjustWindowRect(r, Attr.style, FALSE);
  Attr.w = r.Width();
  Attr.h = r.Height();
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
void PWinToolsCoord::changeZoom()
{
  enum idMenu { id1 = 1000, id2, id3, id4, id5, id6, id7 };
  menuPopup menu[] = {
    { MF_STRING, id1, _T("Zoom x 1/2") },
    { MF_STRING, id2, _T("Zoom x 2/3") },
    { MF_STRING, id3, _T("Zoom x 1") },
    { MF_STRING, id4, _T("Zoom x 3/2") },
    { MF_STRING, id5, _T("Zoom x 2") },
    { MF_STRING, id6, _T("Zoom x 3") },
    { MF_STRING, id7, _T("Zoom x 4") },
    };
  int curr = getDataProject().getCurrZoom();
  menu[curr].flag |= MF_GRAYED;

  int res = popupMenu(*this, menu, SIZE_A(menu)) - id1;
  if((uint)res >= svmManZoom::z_Max)
    return;
  LPCTSTR tit[] = { _T("x 1/2"), _T("x 2/3"), _T("x 1"), _T("x 3/2"), _T("x 2"), _T("x 3"), _T("x 4") };
  Btn->setCaption(tit[res]);

  bool oldZoom = onZoom;
  onZoom = res != svmManZoom::zOne;
  if(onZoom && !oldZoom) {
    PRect r;
    GetWindowRect(*this, r);
    r.bottom += 24;
    setWindowPos(0, r, SWP_NOMOVE | SWP_NOZORDER);
    }
  else if(!onZoom && oldZoom) {
    PRect r;
    GetWindowRect(*this, r);
    r.bottom -= 24;
    setWindowPos(0, r, SWP_NOMOVE | SWP_NOZORDER);
    }
  getDataProject().setCurrZoom((svmManZoom::zoomX)res);

  svmMainClient* mc = getMainClient(this);
  if(mc) {
    InvalidateRect(*mc, 0, 0);
    mc->resetScroll();
    mc->resizeByTitle(true);
    }
}
//-----------------------------------------------------------
void PWinToolsCoord::setCoord(int x, int y)
{
  if(X != x) {
    X = x;
    SET_INT(IDC_ST_COORD_X, x);
    }
  if(Y != y) {
    Y = y;
    SET_INT(IDC_ST_COORD_Y, y);
    }
  if(onZoom) {
    POINT pt = { x, y };
    getDataProject().calcFromScreen(pt);
    SET_INT(IDC_ST_COORD_X2, pt.x);
    SET_INT(IDC_ST_COORD_Y2, pt.y);
    }
}
//-----------------------------------------------------------
#define SET_BMP_AND_SIZE(a, b) \
    a.idBmp = b;\
    a.szBtn.cx = BMP_CX_640;\
    a.szBtn.cy = BMP_CY_640;\
    a.szBmp = a.szBtn;
//-----------------------------------------------------------
extern void showLogo(PWin* parent);
//-----------------------------------------------------------
static
void createTmpDirs(uint n)
{
  TCHAR path[_MAX_PATH];
  wsprintf(path, _T("%s%d"), BASE_TEMP_NAME, n + 1);
  createDirectoryRecurse(path);
}
//-----------------------------------------------------------
static
void makeTmpDirName(LPTSTR target, uint n)
{
  wsprintf(target, _T("%s%d\\"), BASE_TEMP_NAME, n + 1);
}
//-----------------------------------------------------------
static
void set_Coord(PWin* t, DWORD coords)
{
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);

  t->Attr.x = (short)LOWORD(coords);
  t->Attr.y = (short)HIWORD(coords);
#ifdef CHECK_OUT
  if(t->Attr.x < 0)
    t->Attr.x = 0;
  else if(t->Attr.x > width - 100)
    t->Attr.x = width - 100;
  if(t->Attr.y < 0)
    t->Attr.y = 0;
  else if(t->Attr.y > height - 100)
    t->Attr.y = height - 100;
#endif
}
//-----------------------------------------------------------
svmMainClient::svmMainClient(PWin * parent, HINSTANCE hInstance) :
      baseClass(parent, IDD_MAIN_DIALOG, PRect(0, 0, 10, 10), 0, hInstance), currTools(tSELECT),
      multipleAddTools(false), baseCursor(IDC_ARROW), cpCustom(0),
      actionToBkg(false), hwndTips(0), isMovingByKey(0), Tools(0),
      actionLock(false), actionHide(false), onExit(false), onPan(false), RecalcArea(true), moveKeyCount(1.0f)
{
  fillStr(tipsText, 0, SIZE_A(tipsText));
  resetPropertyPage();
  resetCustom();

//  Attr.style |= WS_DLGFRAME;

  for(uint i = 0; i < SIZE_A(Base); ++i) {
    Base[i] = new svmBase(this);
    createTmpDirs(i);
    }

  DWORD coords;

  Tools = new saveCoordWinTools(this, &iTools, _T("Oggetti"), IDB_BKG, hInstance, TOOLS_PATH);
  Tools->Attr.x = 10;
  if(getCoord(TOOLS_PATH, coords))
    set_Coord(Tools, coords);

  Actions = new saveCoordWinTools(this, &iActions, _T("Azioni"), IDB_BKG,  hInstance, ACTION_PATH);
  Actions->Attr.x = 10;
  Actions->Attr.y = 100;
  if(getCoord(ACTION_PATH, coords))
    set_Coord(Actions, coords);

  Page = new saveCoordWinTools(this, &iPage, _T("File"), IDB_BKG,  hInstance, FILE_PATH);
  Page->Attr.x = 175;
  Page->Attr.y = 100;

  if(getCoord(FILE_PATH, coords))
    set_Coord(Page, coords);

  chgPage = new chgPageWinTools(this, &iChgPage, _T("Scelta Pagine"), IDB_BKG,  hInstance);
  chgPage->Attr.x = 275;
  chgPage->Attr.y = 100;

  if(getCoord(CHG_PAGE_PATH, coords))
    set_Coord(chgPage, coords);

  Coord = new PWinToolsCoord(this, _T("Coord"), IDB_BKG,  hInstance);
  Coord->Attr.x = 375;
  Coord->Attr.y = 100;

  if(getCoord(MOUSE_PATH, coords))
    set_Coord(Coord, coords);

  setFlag(pfWantIdle);

  ptMoveByKey.x = 0;
  ptMoveByKey.y = 0;
}
//-----------------------------------------------------------
svmMainClient::~svmMainClient()
{
  onExit = true;
  if(hwndTips)
    DestroyWindow(hwndTips);
  for(uint i = 0; i < SIZE_A(Base); ++i)
    delete Base[i];
  removeTemp();
  delete cpCustom;
  destroy();
}
//-----------------------------------------------------------
void svmMainClient::enableTools(bool set)
{
  EnableWindow(*Tools, set);
  EnableWindow(*Actions, set);
  EnableWindow(*Page, set);
  EnableWindow(*chgPage, set);
}
//-----------------------------------------------------------
UINT svmMainClient::getCurrPage()
{
  return DataP.currPage;
}
//-----------------------------------------------------------
void svmMainClient::setCurrPage(UINT n)
{
  DataP.currPage = n;
}
//-----------------------------------------------------------
void svmMainClient::removeTemp()
{
  TCHAR path[_MAX_PATH];
  for(int i = 0; i < MAX_PAGES; ++i) {
    setCurrPage(i);
    resetCustom();
    resetPropertyPage();
    makeTmpDirName(path, i);
    RemoveDirectory(path);
    }
  RemoveDirectory(BASE_TEMP_NAME);
}
//-----------------------------------------------------------
extern void startTimerForSplash(DWORD time);
//-----------------------------------------------------------
#define WAIT_TIME_LOGO 2500
//-----------------------------------------------------------
struct svmParam
{
  union {
    struct {
      DWORD xor : 1;
      DWORD uni : 1;
      };
    DWORD v;
    } U;
  svmParam() { U.v = 0; }
  svmParam(bool x, bool u) { U.v = 0; U.xor = x; U.uni = u; }
  operator DWORD() { return U.v; }
  operator LPDWORD() { return &U.v; }
};
//----------------------------------------------------------------------------
bool getMainCoord(DWORD& target)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  LPCTSTR value = ini.getValue(OLD_MAIN_POS, MAIN_PATH);
  if(value) {
    __int64 t = _tcstoi64(value, 0, 10);
    target = (DWORD)t;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool setMainCoord(DWORD source)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), source);
  ini_param param = { OLD_MAIN_POS, t };
  ini.addParam(param, MAIN_PATH);
  return ini.save();
}
//-----------------------------------------------------------
bool svmMainClient::create()
{
  ShowWindow(*getParent(), SW_HIDE);

  startTimerForSplash(WAIT_TIME_LOGO);
  performInitData(this, DataP, true);

  if(!baseClass::create())
    return false;

  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));

  resize();
  svmParam param;
  getKeyParam(KEY_PARAM, param);
  if(param.U.xor) {
    ToggleXor = true;
    Actions->selectBtn(tXOR_COLOR);
    }
  ToggleUnicode = true;
  setTitle();
  DWORD pos = 0;
  getMainCoord(pos);
  SetWindowPos(*getParent(), 0, (short)LOWORD(pos), (short)HIWORD(pos), 0, 0, SWP_NOZORDER|SWP_NOSIZE);

  return true;
}
//----------------------------------------------------------------------------
void saveRegAd4(LPCTSTR buff)
{
  setRegText(AD4_PATH, AD4_COORD, buff);
}
//-----------------------------------------------------------
void loadRegAd4(LPTSTR buff, size_t dim)
{
  buff[0] = 0;
  getRegText(AD4_PATH, AD4_COORD, buff, dim);
}
//----------------------------------------------------------------------------
void saveRegCheckAddr(LPCTSTR buff)
{
  setRegText(CHECKADDR_PATH, AD4_COORD, buff);
}
//-----------------------------------------------------------
void loadRegCheckAddr(LPTSTR buff, size_t dim)
{
  buff[0] = 0;
  getRegText(CHECKADDR_PATH, AD4_COORD, buff, dim);
}
//-----------------------------------------------------------
void svmMainClient::saveWinToolCoords()
{
  svmParam param(ToggleXor, true);
  setKeyParam(KEY_PARAM, param);
}
//-----------------------------------------------------------
void svmMainClient::evMouse(UINT message, WPARAM wParam, LPARAM lParam)
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
    case WM_RBUTTONUP:
      evMouseRUp(message, wParam, lParam);
      break;
    }
}
//-----------------------------------------------------------
// spazio libero per header
#define HEADER_WIDTH     585
#define HEADER_HEIGHT     72
#define OFFS_HEADER_LEFT 102
//-----------------------------------------------------------
// dal programma svisor
//#define H_HEADER 137
#define H_HEADER 125
#define H_FOOTER 120
//-----------------------------------------------------------
void svmMainClient::resize()
{
  static bool onResize;
  if(onResize)
    return;
  onResize = true;
  PRect rc;
  GetClientRect(*getParent(), rc);
  PRect r;
  GetWindowRect(*getParent(), r);
  r.MoveTo(0, 0);

  int left = 0;
  int top = 0;
  int width;
  int height;
  SIZE sz;
  sizer::eSizer currSize = sizer::getDefault(sz);
  sizer::setDefault(sizer::s800x600);

  if(DataP.getWhatPage() & 1) { // header
    left = R__X(OFFS_HEADER_LEFT);
    top = R__Y(H_HEADER - HEADER_HEIGHT);
    width = R__X(HEADER_WIDTH);
    height = R__Y(H_HEADER) - top;

    r.bottom = r.top + top + height + r.Height() - rc.Height() + 2;
    }
  else {
    width = sizer::getWidth();
    height = sizer::getHeight();
    do {
      PWin test(0, 0, 0, 0, width, height);
      test.Attr.style = WS_OVERLAPPEDWINDOW;
      test.Attr.exStyle = WS_EX_APPWINDOW;
      test.create();
      PRect rt;
      GetClientRect(test, rt);
      PRect rt2;
      GetWindowRect(test, rt2);
      width -= rt2.Width() - rt.Width();
      height -= rt2.Height() - rt.Height();
      } while(false);
    if(!(DataP.getWhatPage() & 2)) { // se visualizza l'header
      top = R__Y(H_HEADER);
      height -= R__Y(H_HEADER);
      }
    if(!(DataP.getWhatPage() & 4)) { // se visualizza il footer
#if 1
  // nel sVisor viene calcolato in questo modo e si perdono
  // alcuni valori troncando i decimali, per cui occorre usare
  // lo stesso calcono
  #define Y_TXT 5
  #define DIM_H_FONT 16
  #define DEF_H_BTN 20
  #define REAL_H_BTN (ratio * DEF_H_BTN)
      double ratio = sizer::getHeight();
      ratio /= DEF_Y_SCREEN;

      int H_Row = (int)(DIM_H_FONT * ratio);
      int H_Txt = (int)(H_Row * 3.5);
      int H_Btn = (int)REAL_H_BTN;
      int H_Stat = (int)(H_Row * 1.5);

      int h = H_Txt + H_Btn + H_Stat;
      h = (int)( h + ratio * Y_TXT * 4);
      height -= h;//  + GetSystemMetrics(SM_CYCAPTION);
#else
      height -= R__Y(H_FOOTER) + !DataP.hideTitle ? GetSystemMetrics(SM_CYCAPTION) : 0;
#endif
      }
    if(DataP.hideTitle)
      height += GetSystemMetrics(SM_CYCAPTION);

    r.bottom = r.top + top + height + r.Height() - rc.Height() + 2;
    }

  PRect rz(left, top, left + width, top + height);

  DWORD borderOffest = 0;
  getBorderOffset(borderOffest);
  int offsX = (short)LOWORD(borderOffest);
  int offsY = (short)HIWORD(borderOffest);
  rz.right += offsX;
  rz.bottom += offsY;

  getDataProject().calcToScreen(rz);
  setWindowPos(0, rz, SWP_NOZORDER| SWP_FRAMECHANGED);
  sizer::setDefault(currSize, &sz);
  dimWin dW = { r, rz };
  SendMessage(*getParent(), WM_SEND_DIM, 0, (LPARAM)&dW);

#define TTS_STYLE (TTS_ALWAYSTIP | TTS_BALLOON | WS_POPUP | TTS_NOPREFIX)

  if(hwndTips)
    DestroyWindow(hwndTips);

  hwndTips = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, 0, TTS_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,0, 0, getHInstance(), 0);
  if(hwndTips) {
    SetWindowPos(hwndTips, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    TOOLINFO ti;
    memset(&ti, sizeof(ti), 0);
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_TRANSPARENT;
    ti.hwnd = getHandle();
    ti.hinst = getHInstance();
    ti.lpszText = LPSTR_TEXTCALLBACK;
    ti.uId = 1;
    GetClientRect(getHandle(), rz);
    PRect r(rz);
    getDataProject().calcFromScreen(rz);
    rz |= r;
    ti.rect = *rz;
    SendMessage(hwndTips, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
    }

  InvalidateRect(*this, 0, true);
  bool canPan = toBool(SendMessage(*getParent(), WM_HAS_SCROLL, 0, 0));
  Tools->enableBtn(1, canPan);
  onResize = false;
}
//-----------------------------------------------------------
void svmMainClient::resizeByTitle(bool repaint)
{
  resize();
  uint old = getCurrPage();
  for(uint i = 0; i < SIZE_A(Base); ++i) {
    setCurrPage(i);
    TCHAR path[_MAX_PATH];
    makeTmpDirName(path, getCurrPage());
    appendPath(path, PAGE_PROP_NAME);

    setOfString set(path);
    LPCTSTR p = set.getString(ID_INIT_BMP);
    if(!repaint && !p)
      continue;
    reloadBkg(p);
    }
  setCurrPage(old);
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
void svmMainClient::manageDispInfo(LPTOOLTIPTEXT info)
{
  info->hinst = 0;
  info->lpszText = tipsText;
  SendMessage(hwndTips, TTM_SETMAXTIPWIDTH, 0, 300);
}
//-----------------------------------------------------------
static void myRect(HDC hdc, PRect& r)
{
  MoveToEx(hdc, r.left, r.top, 0);
  LineTo(hdc, r.right, r.top);
  LineTo(hdc, r.right, r.bottom);
  LineTo(hdc, r.left, r.bottom);
  LineTo(hdc, r.left, r.top);
}
//-----------------------------------------------------------
void svmMainClient::lockObject(bool lock)
{
  for(uint i = tBUTTON; i < tTOOL_LAST; ++i) {
    bool t = tPICTURE == i ? true : !lock;
    Tools->enableBtn(i, t);
    }
}
//-----------------------------------------------------------
void svmMainClient::lockPage(bool lock)
{
  Actions->enableBtn(tFONT, !lock);
  Actions->enableBtn(tCUSTOM, !lock);
  Page->enableBtn(tNEW, !lock);
  Page->enableBtn(tOPEN, !lock);
  Page->enableBtn(tSAVE, !lock);
  Page->enableBtn(tDONE, !lock);
  for(int i = 0; i < tCHGPAGE_LAST; ++i)
    chgPage->enableBtn(i, !lock);
}
//----------------------------------------------------------------------------
void closeWithAnim(HWND hwnd, bool check)
{
  return;
/*
  if(!check || isWinXP_orLater()) {
    uint playTime = 500;
    AnimateWindow(hwnd, playTime, AW_HOR_NEGATIVE | AW_VER_POSITIVE | AW_SLIDE | AW_CENTER | AW_HIDE);
    }
*/
}
//-----------------------------------------------------------
static void saveCoord(HWND hwnd)
{
  PRect r;
  GetWindowRect(hwnd, r);
  DWORD coords = MAKELONG(r.left, r.top);
  setMainCoord(coords);
}
//-----------------------------------------------------------
void svmMainClient::closeProgr()
{
  if(okToDone(true)) {
    saveCoord(*getParent());
    closeWithAnim(*getParent(), false);
    PostQuitMessage(0);
    }
}
//-----------------------------------------------------------
extern bool runCustomGraph(PWin* parent);
extern bool runCustomGen(PWin* parent);
extern bool runCustomRecipe(PWin* parent, bool hasListbox);
extern bool runCustomRecipeRow(PWin* parent);
extern void modifyPageFont(PWin* parent);
//-----------------------------------------------------------
LRESULT svmMainClient::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      saveWinToolCoords();
      break;

    case WM_SIZE:
      resize();
      return true;

    case WM_NOTIFY:
      switch(((LPNMHDR)lParam)->code) {
        case TTN_NEEDTEXT:
          manageDispInfo((LPTOOLTIPTEXT)lParam);
          break;
        }
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
#ifdef FIXED_MULTIPLE_ADD
          multipleAddTools = needMultipleAdd();
#endif
          }
        else if(curr < LAST_ACTION) {
          curr -= LAST_TOOL;
          switch(curr) {
            case tUNDO:
              Base[getCurrPage()]->Undo();
              break;
            case tREDO:
              Base[getCurrPage()]->Redo();
              break;
            case tALIGN:
              Align();
              break;
            case tORDER:
              Order();
              break;
            case tPROPERTY:
              if(!Base[getCurrPage()]->canProperty())
                setPageProperty();
              else
                Base[getCurrPage()]->setProperty(needSingleProperty());
              break;
            case tGROUP_LOCK:
              setActionLock();
              break;
            case tGROUP_HIDE:
              setActionHide();
              break;
            case tMIRROR:
              setActionMirror();
              break;
            case tBACKGROUND:
              setActionToBkg();
              break;
            case tFONT:
              lockPage(true);
              modifyPageFont(this);
              lockPage(false);
              break;
            case tGROUP:
              actionGroup();
              break;
            case tUNGROUP:
              actionUngroup(false);
              break;
            case tUNGROUPALL:
              actionUngroup(true);
              break;

            case tRESTORE_INSIDE:
              Base[getCurrPage()]->ReInside();
              break;

            case tCUSTOM:
              lockPage(true);
              switch(DataP.getPageType()) {
                case ptcStandard:
//                default:
                  if(runCustomGen(this))
                    getDataProject().setDirty();
                  break;
                case ptcRecipeRow:
                  if(runCustomRecipeRow(this))
                    getDataProject().setDirty();
                  break;
                case ptcRecipe:
                  if(runCustomRecipe(this, false))
                    getDataProject().setDirty();
                  break;
                case ptcRecipeListbox:
                  if(runCustomRecipe(this, true))
                    getDataProject().setDirty();
                  break;
                case ptcGraphTrend:
                  if(runCustomGraph(this))
                    getDataProject().setDirty();
                  break;
                }
              lockPage(false);
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
                saveCoord(*getParent());
                closeWithAnim(*getParent(), false);
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

            case tSTD_MSG:
              dManageStdMsg(this).modal();
              break;
            case tALARM:
              PD_alarmEditorDlg(this).modal();
              break;

            case tSAVE:
              savePage();
              break;
            case tCLEAR:
              if(!okToDone())
                break;
              performNewPage();
              break;
            case tOPEN_PAGE_FROM_OTHER_PRJ:
              if(okToDone())
                openPageFromOtherPrj();
              break;
            case tG_TXT:
              gRunGlobalPageTExt(this);
              invalidate();
              break;
            case tG_TXT_ALARM:
              gRunGlobalAlarmTExt(this);
              break;
            default:
              MessageBox(*this, tipsPage[curr], _T("Choose"), MB_OK);
              break;
            }
          }
        else {
          curr -= LAST_PAGE;
          setCurrPage(curr);
          setTitle();
          resize();
          setBtnStat();
          lockObject(actionToBkg);
          InvalidateRect(*this, 0, 0);
          setPos();
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

    case WM_MOUSEMOVE:
      PostMessage(*Coord, WM_CUSTOM_COORD, MAKEWPARAM(CM_SET_COORD, 0), lParam);

    case WM_RBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
      evMouse(message, wParam, lParam);
      break;

    case WM_LBUTTONDBLCLK:
      PostMessage(*this, WM_C_CHANGED_SELECTION, FIRST_ID_COMMAND + LAST_TOOL + tPROPERTY, 0);
      break;

    case WM_PRINT:
    case WM_PRINTCLIENT:
      do {
        PRect r;
        GetClientRect(*this, r);
        evPaint((HDC)wParam, r);
        lParam &= ~PRF_CHILDREN;
        } while(false);
      break;

    case WM_PAINT:
      do {
        PAINTSTRUCT Paint;
        HDC hdc = BeginPaint(hwnd, &Paint);
        if(!hdc) {
          EndPaint(*this, &Paint);
          return 0;
          }
        PRect r;
        GetClientRect(*this, r);
        evPaint(hdc, r);
        EndPaint(hwnd, &Paint);
        } while(false);

      return 0;

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
            bool fromBkg = Base[getCurrPage()]->pasteFromBkg();
            if(fromBkg ^ actionToBkg)
              setActionToBkg();

            HDC hdc = GetWindowDC(*this);
            Base[getCurrPage()]->pasteCopied(hdc);
            ReleaseDC(*this, hdc);
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
        case IDCM_AD4:
          Base[getCurrPage()]->manageAd4();
          break;
        case IDCM_CHECKADDR:
          Base[getCurrPage()]->manageCheckAddr(false);
          break;
        case IDCM_CHECKADDR_REFRESH:
          Base[getCurrPage()]->manageCheckAddr(true);
          break;
        case IDCM_ADDR_OFFSET:
          if(Base[getCurrPage()]->setActionOffset())
            DataP.setDirty();
          break;
        case IDCM_GROUP:
          if(Base[getCurrPage()]->canGroup())
            actionGroup();
          break;
        case IDCM_UNGROUP:
          if(Base[getCurrPage()]->canUngroup())
            actionUngroup(false);
          break;
        case IDCM_UNGROUP_ALL:
          if(Base[getCurrPage()]->canUngroup())
            actionUngroup(true);
          break;

        case IDCM_COPY_CUSTOM:
          copyCustom();
          break;
        case IDCM_PASTE_CUSTOM:
          pasteCustom();
          break;
        case IDCM_TOGGLE_ED_TXT:
          if(Base[getCurrPage()]->canToggleEdTxt())
            toggleEdTxt();
          break;
        case IDCM_TOGGLE_CHOOSE_TXT:
          if(Base[getCurrPage()]->canToggleChooseTxt())
            toggleChooseTxt();
          break;
        case IDCM_CENTER_VERT:
          if(Base[getCurrPage()]->canCenter())
            centerVert();
          break;
        case IDCM_CENTER_HORZ:
          if(Base[getCurrPage()]->canCenter())
            centerHorz();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmMainClient::centerVert()
{
  Base[getCurrPage()]->centerVert();
}
//-----------------------------------------------------------
void svmMainClient::centerHorz()
{
  Base[getCurrPage()]->centerHorz();
}
//-----------------------------------------------------------
void svmMainClient::toggleEdTxt()
{
  Base[getCurrPage()]->toggleEdTxt();
}
//-----------------------------------------------------------
void svmMainClient::toggleChooseTxt()
{
  Base[getCurrPage()]->toggleChooseTxt();
}
//-----------------------------------------------------------
void svmMainClient::actionGroup()
{
  Base[getCurrPage()]->actionGroup();
}
//-----------------------------------------------------------
void svmMainClient::actionUngroup(bool all)
{
  Base[getCurrPage()]->actionUngroup(all);
}
//-----------------------------------------------------------
void svmMainClient::invalidate()
{
  Base[getCurrPage()]->invalidate();
}
//-----------------------------------------------------------
#define DEF_COLOR_BKG GetSysColor(COLOR_BTNFACE)
//-----------------------------------------------------------
void svmMainClient::evPaint(HDC hdc, const PRect& rect2)
{
  PRect rect(rect2);
  bool onZoom = getDataProject().calcFromScreen(rect);
  PRect r;
  GetClientRect(*this, r);
  getDataProject().calcFromScreen(r);
  HBITMAP hBmpTmp = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
  HDC mdc = CreateCompatibleDC(hdc);
  HGDIOBJ oldObj = SelectObject(mdc, hBmpTmp);
  Base[getCurrPage()]->paint(mdc, rect, true);
  if(onZoom) {
    int old = SetStretchBltMode(hdc, HALFTONE);
    StretchBlt(hdc, rect2.left, rect2.top, rect2.Width(), rect2.Height(),
               mdc, rect.left, rect.top, rect.Width(), rect.Height(), SRCCOPY);
    SetStretchBltMode(hdc, old);
    }
  else
    BitBlt(hdc, rect2.left, rect2.top, rect2.Width(), rect2.Height(), mdc, rect2.left, rect2.top, SRCCOPY);

  Base[getCurrPage()]->paint(hdc, rect, false);

  SelectObject(mdc, oldObj);
  DeleteDC(mdc);
  DeleteObject(hBmpTmp);
}
//-----------------------------------------------------------
void svmMainClient::performMoveByKey(int key)
{
  SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  if(!isMovingByKey) {
    isMovingByKey = 1;
    ptMoveByKey.x = 0;
    ptMoveByKey.y = 0;
    beginXor(ptMoveByKey);
    moveKeyCount = 1.0f;
    }
  switch(key) {
    case VK_LEFT:
      ptMoveByKey.x -= (int)moveKeyCount;
      break;
    case VK_RIGHT:
      ptMoveByKey.x += (int)moveKeyCount;
      break;
    case VK_UP:
      ptMoveByKey.y -= (int)moveKeyCount;
      break;
    case VK_DOWN:
      ptMoveByKey.y += (int)moveKeyCount;
      break;
    }
  if(moveKeyCount > 3.0f)
    moveKeyCount += 0.8f;
  else if(moveKeyCount > 2.0f)
    moveKeyCount += 0.3f;
  else
    moveKeyCount += 0.08f;
  HDC hdc = GetDC(*this);
  drawXorBlock(hdc, ptMoveByKey);
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------
void svmMainClient::performSizeByKey(int key)
{
  SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  ptMoveByKey.x = 0;
  ptMoveByKey.y = 0;
  if(!isMovingByKey) {
    isMovingByKey = 2;
    beginXorSizeByKey();
    moveKeyCount = 1.0f;
    }
  switch(key) {
    case VK_LEFT:
      ptMoveByKey.x -= (int)moveKeyCount;
      break;
    case VK_RIGHT:
      ptMoveByKey.x += (int)moveKeyCount;
      break;
    case VK_UP:
      ptMoveByKey.y -= (int)moveKeyCount;
      break;
    case VK_DOWN:
      ptMoveByKey.y += (int)moveKeyCount;
      break;
    }
  if(moveKeyCount > 3.0f)
    moveKeyCount += 0.8f;
  else if(moveKeyCount > 2.0f)
    moveKeyCount += 0.3f;
  else
    moveKeyCount += 0.08f;

  HDC hdc = GetDC(*this);
  drawResizeAllObject(hdc, ptMoveByKey);
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------
void svmMainClient::performEndKey()
{
  if(!isMovingByKey)
    return;
  SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  HDC hdc = GetDC(*this);
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
  ReleaseDC(*this, hdc);
  moveKeyCount = 1.0f;
}
//-----------------------------------------------------------
void svmMainClient::setActionHide()
{
  actionHide = !actionHide;

  if(actionHide)
    Base[getCurrPage()]->addBtnStat(svmBase::eBS_Hide);
  else
    Base[getCurrPage()]->remBtnStat(svmBase::eBS_Hide);
  if(!Base[getCurrPage()]->setActionHide(actionHide, actionToBkg)) {
    if(actionHide) {
      Base[getCurrPage()]->remBtnStat(svmBase::eBS_Hide);
      setBtnStat();
      }
    }
}
//-----------------------------------------------------------
void svmMainClient::setActionLock()
{
  actionLock = !actionLock;

  if(actionLock)
    Base[getCurrPage()]->addBtnStat(svmBase::eBS_Lock);
  else
    Base[getCurrPage()]->remBtnStat(svmBase::eBS_Lock);

  if(!Base[getCurrPage()]->setActionLock(actionLock, actionToBkg)) {
    if(actionLock) {
      Base[getCurrPage()]->remBtnStat(svmBase::eBS_Lock);
      setBtnStat();
      }
    }
}
//-----------------------------------------------------------
void svmMainClient::setActionMirror()
{
  static bool all;
  static uint choose;
  if(IDOK == PDiagMirror(this, all, choose, Base[getCurrPage()]->canProperty()).modal())
    Base[getCurrPage()]->setActionMirror(all, choose);
}
//-----------------------------------------------------------
void svmMainClient::setActionToBkg()
{
  actionToBkg = !actionToBkg;
  allowSelect& aObj = getAllow();
  if(actionToBkg)
    aObj.set(allowSelect::asOnlyBkg);
  else
    aObj.set(allowSelect::asOtherThenBkg);
  lockObject(actionToBkg);

  Base[getCurrPage()]->setBtnStat(actionToBkg ? svmBase::eBS_Bkg : 0);
  Base[getCurrPage()]->setActionToBkg(actionToBkg);
  Base[getCurrPage()]->findBtnStat(actionToBkg);
  setBtnStat();
}
//-----------------------------------------------------------
bool svmMainClient::okToDone(bool all)
{
  if(all) {
    for(uint i = 0; i < SIZE_A(Base); ++i) {
      if(!Base[i]->canUndo() && !DataP.isDirty(i))
        continue;
      TCHAR buff[_MAX_PATH];
      if(*DataP.getPageName(i))
        _tcscpy_s(buff, DataP.getPageName(i));
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
          if(!result)
            break;

        case IDCANCEL:
          return false;
        }
      }
    return true;
  }



  if(!Base[getCurrPage()]->canUndo() && !DataP.isDirty())
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
void svmMainClient::reloadBkg(LPCTSTR p)
{
  PRect r;
  GetClientRect(*this, r);
  getDataProject().calcFromScreen(r);
  HDC hdc = GetDC(*this);
  HBITMAP hBmpTmp = CreateCompatibleBitmap(hdc, r.Width(), r.Height());

  HDC mdc = CreateCompatibleDC(hdc);
  HGDIOBJ oldObj = SelectObject(mdc, hBmpTmp);
  HBRUSH br = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  FillRect(mdc, r, br);
  drawBmp(mdc, p);

  POINT pt = { 0, 0 };
  SIZE sz = { r.Width(), r.Height() };
  Base[getCurrPage()]->setBkg(new PBitmap(mdc, pt, sz));

  SelectObject(mdc, oldObj);
  DeleteDC(mdc);
  DeleteObject(hBmpTmp);
  ReleaseDC(*this, hdc);

  InvalidateRect(*this, 0, 0);
}
//-----------------------------------------------------------
#define ALLOC_OBJECTS_LINKED(OBJ, num, idInit, offset) \
{ \
  for(int i = offset; i < num; ++i) {\
    LPCTSTR p = set.getString(idInit + i);\
    if(!p)  continue;\
    svmObject *t = new OBJ(this, ++globalIdLinked[getCurrPage()], 0, 0);\
    t->setLinked();\
    if(!t->load(idInit + i, set)) { --globalIdLinked[getCurrPage()]; delete t; } \
    else Base[getCurrPage()]->addObjectLinked(t);\
    }\
}
//-----------------------------------------------------------
#define CHECK_ALLOC_OBJ_LINKED(OBJ, type) \
  p = set.getString(ID_##type);\
  if(p)\
    ALLOC_OBJECTS_LINKED(OBJ, _ttoi(p), ID_INIT_##type, 0)
//-----------------------------------------------------------
#define CHECK_ALLOC_OBJ_OFFSET_LINKED(OBJ, type, offs) \
  p = set.getString(ID_##type);\
  if(p)\
    ALLOC_OBJECTS_LINKED(OBJ, _ttoi(p), ID_INIT_##type, offs)


//-----------------------------------------------------------
void svmMainClient::fillLinkedObj(setOfString& set, POINT pt)
{
  dual* dualBmp = getDualBmp4BtnLinked();
  dualBmp->reset();
  dualBmp->load(set);

  getFontObjLinked().allocFont(set);
  LPCTSTR p;
  CHECK_ALLOC_OBJ_OFFSET_LINKED(svmObjSimpleBmp, BMP, 1)
  CHECK_ALLOC_OBJ_LINKED(svmObjBarGraph, BARGRAPH)
  CHECK_ALLOC_OBJ_LINKED(svmObjPanel, SIMPLE_PANEL)
  CHECK_ALLOC_OBJ_LINKED(svmObjText, SIMPLE_TXT)
  CHECK_ALLOC_OBJ_LINKED(svmObjText, VAR_TXT)
  CHECK_ALLOC_OBJ_LINKED(svmObjEdit, VAR_EDI)
  CHECK_ALLOC_OBJ_LINKED(svmObjButton, VAR_BTN)
  CHECK_ALLOC_OBJ_LINKED(svmObjBmp, VAR_BMP)
  CHECK_ALLOC_OBJ_LINKED(svmObjLed, VAR_LED)
  CHECK_ALLOC_OBJ_LINKED(svmObjDiam, VAR_DIAM)
  CHECK_ALLOC_OBJ_LINKED(svmObjListBox, VAR_LBOX)
  CHECK_ALLOC_OBJ_LINKED(svmObjChoose, VAR_CHOOSE)
  CHECK_ALLOC_OBJ_LINKED(svmObjCurve, VAR_CURVE)

  CHECK_ALLOC_OBJ_LINKED(svmObjXMeter, VAR_XMETER)

  CHECK_ALLOC_OBJ_LINKED(svmObjXScope, VAR_SCOPE)

  CHECK_ALLOC_OBJ_LINKED(svmObjAlarm, VAR_ALARM)

  CHECK_ALLOC_OBJ_LINKED(svmObjTrend, VAR_TREND)
  CHECK_ALLOC_OBJ_LINKED(svmObjSpin, VAR_SPIN)
  CHECK_ALLOC_OBJ_LINKED(svmObjTableInfo, VAR_TABLE_INFO)
  CHECK_ALLOC_OBJ_LINKED(svmObjPlotXY, VAR_PLOT_XY)

  p = set.getString(ID_OFFSET_X);
  if(p)
    pt.x += R__X(_ttoi(p));
  p = set.getString(ID_OFFSET_Y);
  if(p)
    pt.y += R__Y(_ttoi(p));
  if(pt.x || pt.y)
    Base[getCurrPage()]->setOffsetLinked(pt);

}
//-----------------------------------------------------------
static void appendWithSharp(LPTSTR path, LPCTSTR file)
{
  LPCTSTR slash = _tcschr(file, _T('\\'));
  if(!slash)
    slash = _tcschr(file, _T('/'));
  if(slash) {
    TCHAR t[_MAX_PATH];
    _tcscpy_s(t, file);
    t[slash - file] = 0;
    appendPath(path, t);
    appendPath(path, _T("#"));
    _tcscat_s(path, _MAX_PATH, slash + 1);
    }
  else {
    appendPath(path, _T("#"));
    _tcscat_s(path, _MAX_PATH, file);
  }
}
//-----------------------------------------------------------
void svmMainClient::loadLinkedPage(setOfString& set)
{
  linkedObj.reset();
  LPCTSTR p = set.getString(ID_LINK_PAGE_DYN);
  while(p) {
    int offsX = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    int offsY = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    TCHAR path[_MAX_PATH];
     _tcscpy_s(path, DataP.newPath);
    appendPath(path, DataP.systemPath);
    appendPath(path, p);

    setOfString tmpset(path);

    _tcscpy_s(path, DataP.newPath);
    appendPath(path, DataP.systemPath);
    appendWithSharp(path, p);

    manageCryptPage mcp;
    bool crypted = mcp.isCrypted(path);
    if(crypted)
      mcp.makePathAndDecrypt(path);
    tmpset.add(path);
    if(crypted)
      mcp.releaseFileDecrypt(path);

    if(tmpset.setFirst()) {
      POINT pt = { offsX, offsY };
      fillLinkedObj(tmpset, pt);
      }
    break;
    }
  InvalidateRect(*this, 0, 0);
}
//-----------------------------------------------------------
void svmMainClient::setPageProperty()
{
  if(DataP.getWhatPage() & 1)
    return;

  TCHAR path[_MAX_PATH];
  makeTmpDirName(path, getCurrPage());
  appendPath(path, PAGE_PROP_NAME);

  setOfString set(path);
  TCHAR old[500] = _T("\0");
  LPCTSTR p = set.getString(ID_INIT_BMP);
  if(p)
    _tcscpy_s(old, p);

  DWORD oldShow = DataP.getWhatPage() >> 1;
  if(IDOK != svmPageProperty(this, set).modal())
    return;
  DataP.setDirty();
  if(set.setFirst()) {
    P_File pf(path, P_CREAT);
    if(pf.P_open()) {
      // non sapendo com'è va salvata in unicode, pena la perdita di lingue
      // che necessitano dell'unicode
      bool unicode = true;
      do {
        if(!set.writeCurrent(pf, unicode))
          break;
        } while(set.setNext());
      }
    p = set.getString(ID_INIT_BMP);
    if(p && _tcsicmp(p, old))
      reloadBkg(p);
    else if(!p && *old) {
      Base[getCurrPage()]->setBkg(0);
      InvalidateRect(*this, 0, 0);
      }
    DWORD val = (DWORD)-1;
    p = set.getString(ID_CODE_PAGE_TYPE);
    p = findNextParam(p, 1);
    if(p)
      val = _ttoi(p);
    if(val != (DWORD)-1 && val != oldShow) {
      DataP.setWhatPage(val << 1);
      resize();
      }
    }
}
//-----------------------------------------------------------
void svmMainClient::flushData()
{
  Base[getCurrPage()]->flushSel();
  Base[getCurrPage()]->flushObject();
  Base[getCurrPage()]->flushHistory();

  Base[getCurrPage()]->setBkg(0);
  DataP.resetDirty();

  getFontObj().flush();
  getFontObjLinked().flush();

  InvalidateRect(*this, 0, 0);
}
//-----------------------------------------------------------
static void drawFocusRect(HDC hdc, const PRect& norm)
{
#if 1
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
#else
  DrawFocusRect(hdc, norm);
#endif
}
//-----------------------------------------------------------
static bool keyForPan()
{
  return (GetKeyState(VK_MENU) & 0x8000) && (GetKeyState(VK_CONTROL) & 0x8000);
}
//-----------------------------------------------------------
bool svmMainClient::canPan()
{
  if(tPAN == currTools)
    return true;
  if(keyForPan())
    return toBool(SendMessage(*getParent(), WM_HAS_SCROLL, 0, 0));
  return false;
}
//-----------------------------------------------------------
void svmMainClient::evMouseMove(UINT /*message*/, WPARAM wParam, LPARAM lParam)
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
  HDC hdc = GetDC(*this);
  POINT pt2 = pt;
  getDataProject().calcFromScreen(pt2);

  switch(currAction) {
    case svmBase::select:
    // se il rettangolo non è vuoto occorre cancellare il disegno precedente
      if(frame.Width() || frame.Height()) {
        PRect norm(frame);
        norm.Normalize();
        drawFocusRect(hdc, norm);
        }
      break;
    case svmBase::moving:
      drawXorBlock(hdc, pt2);
      break;
    case svmBase::sizing:
      drawXorObject(hdc, pt2);
      break;
    }

  // assegna nuova posizione
  frame.right = pt.x;
  frame.bottom = pt.y;

  if(frame.Width() || frame.Height()) {
    PRect norm(frame);
    norm.Normalize();

    switch(currAction) {
      case svmBase::select:
        drawFocusRect(hdc, norm);
        break;
      }
    }
  if(svmBase::none == currAction)
    showAnchorType(pt2, wParam);
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------
void svmMainClient::evMouseLDown(UINT /*message*/, WPARAM wParam, LPARAM lParam)
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

  HDC hdc = GetDC(*this);
  POINT pt2 = pt;
  getDataProject().calcFromScreen(pt2);

  // se currTools è zero, si attiva la selezione, a meno che non si sia sopra
  // un oggetto, allora occorre controllare se si vuole spostare o ridimensionare
  // e se premuto lo shift aggiungere l'oggetto alla selezione attuale
  if(tSELECT == currTools) {
    SetCapture(*this);
    currAction = findPoint(hdc, pt2, wParam);
    frame.left = pt.x;
    frame.top = pt.y;
    frame.right = pt.x;
    frame.bottom = pt.y;
    if(svmBase::none == currAction)
      ReleaseCapture();
    else if(svmBase::select != currAction)
      beginXor(pt2);
    }
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------
void svmMainClient::evMouseLUp(UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  if(onPan) {
    onPan = false;
    SetCursor(LoadCursor(0, IDC_ARROW));
    ReleaseCapture();
    return;
    }
  HDC hdc = GetDC(*this);
  POINT pt2 = pt;
  getDataProject().calcFromScreen(pt2);

  if(tSELECT != currTools) {
    svmObject* t = addObject(hdc, pt2, currTools);
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
      norm.Normalize();
      switch(currAction) {
        case svmBase::select:
          drawFocusRect(hdc, norm);
          getDataProject().calcFromScreen(norm);
          findObjects(norm, wParam);
          break;
        case svmBase::moving:
          drawXorBlockFinal(hdc, pt2);
          break;
        case svmBase::sizing:
          drawXorObjectFinal(hdc, pt2);
          break;
        }

      showSelected(hdc);
      currAction = svmBase::none;
      }
    }
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------
void svmMainClient::checkRMouseLinked()
{
  int stat = Base[getCurrPage()]->hasLinked();
  enum idMenu { idLinkedShow = 1000, idLinkedHide, idCopyCustom, idPasteCustom };
  menuPopup menu[] = {
    { MF_STRING, idLinkedShow, _T("Visualizza Pagina Linkata") },
    { MF_STRING, idLinkedHide, _T("Nascondi Pagina Linkata") },
    { MF_STRING, idCopyCustom, _T("Copia codici personalizzati") },
    { MF_STRING, idPasteCustom, _T("Incolla codici personalizzati") },
    };
  if(stat > 0)
    menu[0].flag |= MF_GRAYED;
  else
    menu[1].flag |= MF_GRAYED;
  if(!cpCustom)
    menu[idPasteCustom - idLinkedShow].flag |= MF_GRAYED;

  menuPopup* pmenu = menu;
  int nmenu = SIZE_A(menu);
  if(!stat) {
    pmenu += 2;
    nmenu -= 2;
    }
  int msg = 0;
  switch(popupMenu(*this, pmenu, nmenu)) {
    case idLinkedShow:
      stat = 1;
      break;
    case idLinkedHide:
      stat = 0;
      break;
    case idCopyCustom:
      msg = IDCM_COPY_CUSTOM;
      break;
    case idPasteCustom:
      msg = IDCM_PASTE_CUSTOM;
      break;
    default:
      return;
    }
  if(msg)
    PostMessage(*this, WM_COMMAND, MAKEWPARAM(msg, 0), 0);
  else {
    Base[getCurrPage()]->hideShowLinked(toBool(stat));
    InvalidateRect(*this, 0, 0);
    }
}
//-----------------------------------------------------------
void svmMainClient::setAddLock()
{
  Base[getCurrPage()]->addBtnStat(1);
  actionLock = true;
  Base[getCurrPage()]->setActionLock(actionLock, actionToBkg);
  setBtnStat();
}
//-----------------------------------------------------------
void svmMainClient::setAddHide()
{
  Base[getCurrPage()]->addBtnStat(2);
  actionHide = true;
  Base[getCurrPage()]->setActionHide(actionHide, actionToBkg);
  setBtnStat();
}
//-----------------------------------------------------------
extern bool getListHwnd(HWND* phwnd);
//-----------------------------------------------------------
void svmMainClient::evMouseRUp(UINT message, WPARAM wParam, LPARAM lParam)
{
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  bool changed;
  bool onSel;
  svmObject::typeOfAnchor anchor = Base[getCurrPage()]->getAnchor(pt, tipsText, SIZE_A(tipsText), changed, onSel);

  enum idMenu { idFirst = 1000, idCopy = idFirst, idPaste, idFlushPaste, idSep1, idDup, idSep2, idLock, idHide,
        idSep3, idAd4, idCheckAddr, idCheckAddrRefresh, idAddrOffset, idSep4, idToggleEdTxt, idToggleChooseTxt, idSep5, idCenterVert, idCenterHorz, idSep6, idGrp, idUngrp, idUngrpAll };
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
    { MF_STRING, idAd4, _T("CTRL+A -> Apri file di Periph_Addr") },
    { MF_STRING, idCheckAddr, _T("CTRL+K -> Apri CheckAddr") },
    { MF_STRING, idCheckAddrRefresh, _T("CTRL+ALT+K -> Apri CheckAddr con refresh") },
    { MF_STRING, idAddrOffset, _T("CTRL+O -> Aggiungi Offset Addr") },
    { MF_SEPARATOR, 0, 0 },
    { MF_STRING, idToggleEdTxt, _T("CTRL+T -> Scambia Edit con Text") },
    { MF_STRING, idToggleChooseTxt, _T("CTRL+W -> Scambia Scelta con Text") },
    { MF_SEPARATOR, 0, 0 },

    { MF_STRING, idCenterVert, _T("CTRL+SHIFT+V -> Centra Verticalmente") },
    { MF_STRING, idCenterHorz, _T("CTRL+SHIFT+H -> Centra Orizzontalmente") },
    { MF_SEPARATOR, 0, 0 },

    { MF_STRING, idGrp, _T("CTRL+R -> Raggruppa") },
    { MF_STRING, idUngrp, _T("CTRL+S -> Separa") },
    { MF_STRING, idUngrpAll, _T("CTRL+X -> Separa tutto") },
    };

  bool hasCopied = Base[getCurrPage()]->canPaste();
  if(!Base[getCurrPage()]->canProperty() && svmObject::Extern == anchor) {
    if(!hasCopied)
      return checkRMouseLinked();

    menu[idCopy - idFirst].flag |= MF_GRAYED;
    menu[idDup - idFirst].flag |= MF_GRAYED;
    }

  if(!hasCopied) {
    menu[idPaste - idFirst].flag |= MF_GRAYED;
    menu[idFlushPaste - idFirst].flag |= MF_GRAYED;
    }
  // se il click dx avviene su un oggetto non selezionato, simula il click sx
  if(!onSel) {
    uint old = currAction;
    currAction = svmBase::select;
    evMouseLDown(message, wParam, lParam);
    evMouseLUp(message, wParam, lParam);
    currAction = old;
    }

  int nItem = SIZE_A(menu);
  if(!Base[getCurrPage()]->canGroup() && !Base[getCurrPage()]->canUngroup())
    nItem -= 4;
  else {
    if(!Base[getCurrPage()]->canGroup())
      menu[idGrp - idFirst].flag |= MF_GRAYED;
    if(!Base[getCurrPage()]->canUngroup()) {
      menu[idUngrp - idFirst].flag |= MF_GRAYED;
      menu[idUngrpAll - idFirst].flag |= MF_GRAYED;
      }
    }
  if(!Base[getCurrPage()]->canToggleEdTxt())
    menu[idToggleEdTxt - idFirst].flag |= MF_GRAYED;

  if(!Base[getCurrPage()]->canToggleChooseTxt())
    menu[idToggleChooseTxt - idFirst].flag |= MF_GRAYED;

  if(!Base[getCurrPage()]->canCenter()) {
    menu[idCenterVert - idFirst].flag |= MF_GRAYED;
    menu[idCenterHorz - idFirst].flag |= MF_GRAYED;
    }
  uint msg = 0;
  HWND find = 0;
  getListHwnd(&find);
  if(!find) {
    menu[idCheckAddr - idFirst].flag |= MF_GRAYED;
    menu[idCheckAddrRefresh - idFirst].flag |= MF_GRAYED;
    }
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
    case idAd4:
      msg = IDCM_AD4;
      break;
    case idCheckAddr:
      msg = IDCM_CHECKADDR;
      break;
    case idCheckAddrRefresh:
      msg = IDCM_CHECKADDR_REFRESH;
      break;
    case idAddrOffset:
      msg =IDCM_ADDR_OFFSET;
      break;
    case idToggleEdTxt:
      msg =IDCM_TOGGLE_ED_TXT;
      break;
    case idToggleChooseTxt:
      msg =IDCM_TOGGLE_CHOOSE_TXT;
      break;
    case idCenterVert:
      msg =IDCM_CENTER_VERT;
      break;
    case idCenterHorz:
      msg =IDCM_CENTER_HORZ;
      break;

    case idGrp:
      msg =IDCM_GROUP;
      break;
    case idUngrp:
      msg =IDCM_UNGROUP;
      break;
    case idUngrpAll:
      msg =IDCM_UNGROUP_ALL;
      break;
    }
  if(msg)
    PostMessage(*this, WM_COMMAND, MAKEWPARAM(msg, 0), 0);
}
//-----------------------------------------------------------
void svmMainClient::showAnchorType(const POINT& pt, int keyFlag)
{
  allowSelect& aObj = getAllow();
  allowSelect::allow allowedObj = aObj.get();
  if(isBkgSelected()) {
    if(allowSelect::asOtherThenBkg == allowedObj)
      aObj.set(allowSelect::asOnlyBkg);
    }
  else if(allowSelect::asOnlyBkg == allowedObj) {
    aObj.set(allowSelect::asOtherThenBkg);
    }
  bool changed;
  bool dummy;
  svmObject::typeOfAnchor anchor = Base[getCurrPage()]->getAnchor(pt, tipsText, SIZE_A(tipsText), changed, dummy);
  if(svmObject::Extern != anchor) {
    if(changed)
      SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
    SendMessage(hwndTips, TTM_ACTIVATE, TRUE, 0);
    MSG Msg = { *this, WM_MOUSEMOVE, keyFlag, MAKELPARAM(pt.x, pt.y), GetTickCount() };
    Msg.pt = pt;
    SendMessage(hwndTips, TTM_RELAYEVENT, 0, (LPARAM)&Msg);
    }
  else
    SendMessage(hwndTips, TTM_ACTIVATE, FALSE, 0);
  setCursor(anchor, false);
}
//-----------------------------------------------------------
void svmMainClient::beginXor(const POINT& pt)
{
  svmObject::typeOfAnchor anchor = Base[getCurrPage()]->beginXor(pt);
  setCursor(anchor, true);
}
//-----------------------------------------------------------
void svmMainClient::beginXorSizeByKey()
{
  Base[getCurrPage()]->beginXorSizeByKey();
}
//-----------------------------------------------------------
void svmMainClient::setCursor(uint anchor, bool forceChange)
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
void svmMainClient::drawXorBlock(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawXorBlock(hdc, pt);
}
//-----------------------------------------------------------
void svmMainClient::drawXorBlockFinal(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawXorBlockFinal(hdc, pt);
}
//-----------------------------------------------------------
void svmMainClient::drawXorObject(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawXorObject(hdc, pt);
}
//-----------------------------------------------------------
void svmMainClient::drawXorObjectFinal(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawXorObjectFinal(hdc, pt);
}
//-----------------------------------------------------------
void svmMainClient::drawResizeAllObjectFinal(HDC hdc)
{
  Base[getCurrPage()]->drawResizeAllObjectFinal(hdc);
}
//-----------------------------------------------------------
void svmMainClient::drawResizeAllObject(HDC hdc, const POINT& pt)
{
  Base[getCurrPage()]->drawResizeAllObject(hdc, pt);
}
//-----------------------------------------------------------
void svmMainClient::findObjects(const PRect& frame, int keyFlag)
{
  Base[getCurrPage()]->findObjects(frame, keyFlag);
}
//-----------------------------------------------------------
void svmMainClient::showSelected(HDC hdc)
{
  Base[getCurrPage()]->showSelected(hdc);
}
//-----------------------------------------------------------
void svmMainClient::unselectAll(HDC hdc)
{
  Base[getCurrPage()]->unselectAll(hdc);
}
//-----------------------------------------------------------
int svmMainClient::findPoint(HDC hdc, const POINT&pt, int keyFlag)
{
  return Base[getCurrPage()]->findPoint(hdc, pt, keyFlag);
}
//-----------------------------------------------------------
svmObject* svmMainClient::addObject(HDC hdc, const POINT& pt, UINT currTools)
{
  svmObject *t = 0;
  switch(currTools) {
    case tBUTTON:
      t = new svmObjButton(this, getGlobalId(), pt.x, pt.y);
      break;
    case tTEXT:
      t = new svmObjText(this, getGlobalId(), pt.x, pt.y);
      break;
    case tEDIT:
      t = new svmObjEdit(this, getGlobalId(), pt.x, pt.y);
      break;
    case tPANEL:
      t = new svmObjPanel(this, getGlobalId(), pt.x, pt.y);
      break;
    case tPICTURE:
      if(!isBkgSelected())
        t = new svmObjBmp(this, getGlobalId(), pt.x, pt.y);
      else
        t = new svmObjSimpleBmp(this, getGlobalId(), pt.x, pt.y);
      break;
    case tBARGRAPH:
      t = new svmObjBarGraph(this, getGlobalId(), pt.x, pt.y);
      break;
    case tLED:
      t = new svmObjLed(this, getGlobalId(), pt.x, pt.y);
      break;
    case tLISTBOX:
      t = new svmObjListBox(this, getGlobalId(), pt.x, pt.y);
      break;
    case tDIAMETER:
      t = new svmObjDiam(this, getGlobalId(), pt.x, pt.y);
      break;
    case tCHOOSE:
      t = new svmObjChoose(this, getGlobalId(), pt.x, pt.y);
      break;
    case tCURVE:
      t = new svmObjCurve(this, getGlobalId(), pt.x, pt.y);
      break;

    case tCAM:
      t = new svmObjCam(this, getGlobalId(), pt.x, pt.y);
      break;
    case tXMETER:
      t = new svmObjXMeter(this, getGlobalId(), pt.x, pt.y);
      break;

    case tXSCOPE:
      t = new svmObjXScope(this, getGlobalId(), pt.x, pt.y);
      break;
    case tOBJ_ALARM:
      t = new svmObjAlarm(this, getGlobalId(), pt.x, pt.y);
      break;
    case tOBJ_TREND:
      t = new svmObjTrend(this, getGlobalId(), pt.x, pt.y);
      break;
    case tOBJ_SPIN:
      t = new svmObjSpin(this, getGlobalId(), pt.x, pt.y);
      break;
    case tOBJ_TABLE_INFO:
      t = new svmObjTableInfo(this, getGlobalId(), pt.x, pt.y);
      break;
    case tOBJ_PLOT_XY:
      t = new svmObjPlotXY(this, getGlobalId(), pt.x, pt.y);
      break;
    case tOBJ_SLIDER:
      t = new svmObjSlider(this, getGlobalId(), pt.x, pt.y);
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
bool svmMainClient::idle(DWORD count)
{
  bool enable = Base[getCurrPage()]->canUndo();
  Actions->enableBtn(tUNDO, enable);

  enable = Base[getCurrPage()]->canRedo();
  Actions->enableBtn(tREDO, enable);

  enable = Base[getCurrPage()]->canAlign();
  Actions->enableBtn(tALIGN, enable);

  enable = Base[getCurrPage()]->canOrder();
  Actions->enableBtn(tORDER, enable);

  enable = Base[getCurrPage()]->canReInside();
  Actions->enableBtn(tRESTORE_INSIDE, enable);

  enable = Base[getCurrPage()]->canGroup();
  Actions->enableBtn(tGROUP, enable);

  enable = Base[getCurrPage()]->canUngroup();
  Actions->enableBtn(tUNGROUP, enable);
  Actions->enableBtn(tUNGROUPALL, enable);

  return baseClass::idle(count);
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
//-----------------------------------------------------------
void svmMainClient::Align()
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
      Base[getCurrPage()]->Align(svmBase::aTop);
      break;
    case idRight:
      Base[getCurrPage()]->Align(svmBase::aRight);
      break;
    case idBottom:
      Base[getCurrPage()]->Align(svmBase::aBottom);
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
void svmMainClient::Order()
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
  for(uint i = 0; i < SIZE_A(menu); ++i)
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
void svmMainClient::setBtnStat()
{
  DWORD stat = Base[getCurrPage()]->getBtnStat();
  actionLock = toBool(stat & 1);
  Actions->selectBtn(tGROUP_LOCK, actionLock);

  actionHide = toBool(stat & 2);
  Actions->selectBtn(tGROUP_HIDE, actionHide);

  actionToBkg = toBool(stat & 4);
  Actions->selectBtn(tBACKGROUND, actionToBkg);
}
//-----------------------------------------------------------
void svmMainClient::performNewPage()
{
  flushData();
  resetPropertyPage();
  resetCustom();
  DataP.reset4New();
  setTitle();
  manageGroupObject& groupMan = getManageGroupObject();
  groupMan.flushAll();
}
//-----------------------------------------------------------
void svmMainClient::performNew()
{
  enum idMenu { idNProj = 1000, idNPage, idNHeader, idNStandard };
  menuPopup menu[] = {
    { MF_STRING, idNProj, _T("Nuovo Progetto") },
    { MF_STRING, idNPage, _T("Nuova Pagina") },
    { MF_STRING, idNHeader, _T("Nuovo Header") },
    { MF_STRING, idNStandard, _T("Nuova Pagina Predefinita") },
    };

  switch(popupMenu(*this, menu, SIZE_A(menu))) {
    case idNProj:
      if(performInitData(this, DataP, false)) {
        RecalcArea = true;
        for(int i = 0; i < MAX_PAGES; ++i) {
          setCurrPage(i);
          performNewPage();
          }
        setCurrPage(0);
        chgPage->selectBtn(tONE);
        resize();
        ToggleUnicode = true;
        Page->selectBtn(3);
        resetScroll();
        }
      break;
    case idNHeader:
      performNewPage();
      DataP.setWhatPage(1);
      resize();
      resetScroll();
      break;
    case idNPage:
      performNewPage();
      do {
        uint v = DataP.getWhatPage();
        DataP.setWhatPage(v & ~1);
        } while(false);
      resize();
      resetScroll();
      break;
    case idNStandard:
      performNewStd();
      resetScroll();
      break;
    default:
      return;
    }
  Base[getCurrPage()]->setBtnStat(0);
  setBtnStat();
}
//-----------------------------------------------------------
#include "wizardDlg.h"
void svmMainClient::performNewStd()
{
  lockPage(true);
  setOfString set;
  if(IDOK == wizardDlg(this, set).modal()) {
    if(set.setFirst()) {
      performOpenPage(set);
      DataP.setDirty();
      }
    }
  lockPage(false);
}
//-----------------------------------------------------------
void svmMainClient::setTitle()
{
  TCHAR buff[_MAX_PATH * 2];

  if(*DataP.getPageName()) {
    TCHAR t[_MAX_PATH];
    if(*DataP.getSubPath())
      wsprintf(t, _T("%s\\%s"),  DataP.getSubPath(), DataP.getPageName());
    else
      _tcscpy_s(t, DataP.getPageName());
    if(*DataP.newPath)
      wsprintf(buff, _T("SVisor Maker - Ver. %d.%d.%d.%d - %s - %s"), svMkFILEVERSION, DataP.newPath, t);
    else
      wsprintf(buff, _T("SVisor Maker - Ver. %d.%d.%d.%d - %s"), svMkFILEVERSION, t);
    }
  else {
    if(*DataP.newPath)
      wsprintf(buff, _T("SVisor Maker - Ver. %d.%d.%d.%d - %s"), svMkFILEVERSION, DataP.newPath);
    else
      wsprintf(buff, _T("SVisor Maker - Ver. %d.%d.%d.%d"), svMkFILEVERSION);
    }
  getParent()->setCaption(buff);
}
//-----------------------------------------------------------
UINT svmMainClient::getIncrementGlobalId()
{
  return getGlobalId();
}
//-----------------------------------------------------------
void svmMainClient::decrementGlobalId() { --globalId[getCurrPage()]; }
//-----------------------------------------------------------
void svmMainClient::addNewObject(svmObject* obj)
{
  HDC hdc = GetDC(*this);
  Base[getCurrPage()]->addObject(obj, hdc);
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------
template <typename OBJ, int offs>
void svmMainClient::alloc_objects(setOfString& set, uint num, uint idInit, HDC hdc, createGroupObject& createGrp)
{
  for(uint i = offs; i < num; ++i) {
    manageObjId moi(0, idInit);
    uint id = moi.calcBaseExtendId(i);
    LPCTSTR p = set.getString(id);
    if(!p)
      continue;
    svmObject *t = new OBJ(this, getGlobalId(), 0, 0);
    if(!t->load(id, set)) {
      --globalId[getCurrPage()];
      delete t;
      }
    else {
      Base[getCurrPage()]->addObject(t, hdc);
      createGrp.setGroup(t, id);
      }
    }
}
//-----------------------------------------------------------
#define ALLOC_OBJECTS(OBJ, num, idInit, offs) alloc_objects<OBJ,offs>(set, num, idInit, hdc, createGrp);
//-----------------------------------------------------------
//-----------------------------------------------------------
#define CHECK_ALLOC_OBJ(OBJ, type) \
  p = set.getString(ID_##type);\
  if(p)\
    ALLOC_OBJECTS(OBJ, _ttoi(p), ID_INIT_##type, 0)
//-----------------------------------------------------------
#define CHECK_ALLOC_OBJ_OFFSET(OBJ, type, offs) \
  p = set.getString(ID_##type);\
  if(p)\
    ALLOC_OBJECTS(OBJ, _ttoi(p), ID_INIT_##type, offs)
//-----------------------------------------------------------
P_File* makeFileTmpCustom(bool crypt)
{
  TCHAR path[_MAX_PATH];
  makeTmpDirName(path, DataP.currPage);
  if(crypt) {
    appendPath(path, SHARP);
    _tcscat_s(path, CUSTOM_NAME);
    }
  else
    appendPath(path, CUSTOM_NAME);
  P_File* pf = new P_File(path, P_CREAT);
  if(pf->P_open())
    return pf;
  delete pf;
  return 0;
}
//-----------------------------------------------------------
void loadTmpCustom(setOfString& set)
{
  set.reset();
  TCHAR path[_MAX_PATH];
  do {
    makeTmpDirName(path, DataP.currPage);
    appendPath(path, SHARP);
    _tcscat_s(path, CUSTOM_NAME);

    set.add(path);
    } while(false);
  do {
    makeTmpDirName(path, DataP.currPage);
    appendPath(path, CUSTOM_NAME);
    set.add(path);
    } while(false);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmMainClient::resetPage()
{
  resetCustom();
}
//-----------------------------------------------------------
bool operator !=(const SIZE& sz1, const SIZE& sz2)
{
  return sz1.cx != sz2.cx || sz1.cy != sz2.cy;
}
//-----------------------------------------------------------
void svmMainClient::performOpenPage(setOfString& set)
{
  manageGroupObject& groupMan = getManageGroupObject();
  groupMan.flushAll();
  createGroupObject createGrp(groupMan, set);

  globalId[getCurrPage()] = 0;
  globalIdLinked[getCurrPage()] = 0;
  flushData();
  resize();
  SIZE sz;
  sizer::eSizer oldSz = sizer::getDefault(sz);
  LPCTSTR p = set.getString(ID_RESOLUTION);
  if(p) {
    switch(_ttoi(p)) {
      case -1:
        sizer::setDefault(sizer::s640x480);
        break;
      case 0:
        sizer::setDefault(sizer::s800x600);
        break;
      case 1:
        sizer::setDefault(sizer::s1024x768);
        break;
      case 2:
        sizer::setDefault(sizer::s1280x1024);
        break;
      case 3:
        sizer::setDefault(sizer::s1440x900);
        break;
      case 4:
        sizer::setDefault(sizer::s1600x1200);
        break;
      case 5:
        sizer::setDefault(sizer::s1680x1050);
        break;
      case 6:
        sizer::setDefault(sizer::s1920x1440);
        break;
      case 100:
        p = findNextParamTrim(p);
        if(p) {
          uint w = _ttoi(p);
          uint h = w / 2;
          p = findNextParamTrim(p);
          if(p)
            h = _ttoi(p);
          SIZE sz2 = { w, h };
          sizer::setDefault(sizer::sPersonalized, &sz2);
          }
        break;
      }
    }
  else
    sizer::setCurrAsDefault();
  do {
    SIZE newSz;
    sizer::eSizer newRes = sizer::getDefault(newSz);
    if(newRes != oldSz || sizer::sPersonalized == newRes && newSz != sz) {
      LPCTSTR msg = _T("La risoluzione della pagina è diversa da quella corrente, vuoi adattarla?");
      if(IDYES != MessageBox(*this, msg, _T("Attenzione"), MB_YESNO | MB_ICONINFORMATION))
        sizer::setDefault(oldSz, &sz);
      }
    } while(false);
  dual* dualBmp = getDualBmp4Btn();
  dualBmp->reset();
  dualBmp->load(set);

  if(!loadPropertyPage(set))
    return;

  getFontObj().allocFont(set);

  HDC hdc = GetDC(*this);

  CHECK_ALLOC_OBJ_OFFSET(svmObjSimpleBmp, BMP, 1)
  CHECK_ALLOC_OBJ(svmObjBarGraph, BARGRAPH)
  CHECK_ALLOC_OBJ(svmObjPanel, SIMPLE_PANEL)
  CHECK_ALLOC_OBJ(svmObjText, SIMPLE_TXT)
  CHECK_ALLOC_OBJ(svmObjText, VAR_TXT)
  CHECK_ALLOC_OBJ(svmObjEdit, VAR_EDI)
  CHECK_ALLOC_OBJ(svmObjButton, VAR_BTN)
  CHECK_ALLOC_OBJ(svmObjBmp, VAR_BMP)
  CHECK_ALLOC_OBJ(svmObjLed, VAR_LED)
  CHECK_ALLOC_OBJ(svmObjDiam, VAR_DIAM)
  CHECK_ALLOC_OBJ(svmObjListBox, VAR_LBOX)
  CHECK_ALLOC_OBJ(svmObjChoose, VAR_CHOOSE)
  CHECK_ALLOC_OBJ(svmObjCurve, VAR_CURVE)

  CHECK_ALLOC_OBJ(svmObjCam, VAR_CAM)
  CHECK_ALLOC_OBJ(svmObjXMeter, VAR_XMETER)
  CHECK_ALLOC_OBJ(svmObjXScope, VAR_SCOPE)
  CHECK_ALLOC_OBJ(svmObjAlarm, VAR_ALARM)
  CHECK_ALLOC_OBJ(svmObjTrend, VAR_TREND)
  CHECK_ALLOC_OBJ(svmObjSpin, VAR_SPIN)
  CHECK_ALLOC_OBJ(svmObjTableInfo, VAR_TABLE_INFO)
  CHECK_ALLOC_OBJ(svmObjPlotXY, VAR_PLOT_XY)
  CHECK_ALLOC_OBJ(svmObjSlider, VAR_SLIDER)

  // se è l'header non deve caricare l'offset;
  if(!(DataP.getWhatPage() & 1)) {
    POINT pt = { 0, 0 };
    p = set.getString(ID_OFFSET_X);
    if(p)
      pt.x = R__X(_ttoi(p));
    p = set.getString(ID_OFFSET_Y);
    if(p)
      pt.y = R__Y(_ttoi(p));
    if(pt.x || pt.y)
      Base[getCurrPage()]->setOffset(pt);
    }

  sizer::setCurrAsDefault();

  Base[getCurrPage()]->flushHistory();
  showSelected(hdc);
  unselectAll(hdc);
  ReleaseDC(*this, hdc);
  SetFocus(*this);
  setTitle();

  loadLinkedPage(set);
  p = set.getString(ID_INIT_BMP);
  if(p)
    reloadBkg(p);
}
//-----------------------------------------------------------
bool svmMainClient::performLoadPage(setOfString& set)
{
  resetPage();
  resetScroll();
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, DataP.newPath);
  appendPath(path, DataP.systemPath);
  appendPath(path, DataP.getSubPath());
  appendPath(path, DataP.getPageName());

  set.add(path);

  loadCustomClear(set);

  _tcscpy_s(path, DataP.newPath);
  appendPath(path, DataP.systemPath);
  appendPath(path, DataP.getSubPath());
  appendPath(path, _T("#"));
  _tcscat_s(path, DataP.getPageName());

  manageCryptPage mcp;
  bool crypted = mcp.isCrypted(path);
  if(crypted)
    mcp.makePathAndDecrypt(path);

  do {
    setOfString setC(path);
    loadCustomCrypt(setC);
    } while(false);

  set.add(path);
  if(crypted)
    mcp.releaseFileDecrypt(path);

  if(!crypted && mcp.isCrypted()) {
    MessageBox(*this, _T("Pagina non abilitata"), _T("Errore"), MB_OK | MB_ICONSTOP);
    return false;
    }

  performOpenPage(set);
  Base[getCurrPage()]->setBtnStat(0);
  setBtnStat();
  return true;
}
//-----------------------------------------------------------
void svmMainClient::openPage()
{
  enableTools(false);
  int result = openPageFile(this, DataP).modal();
  enableTools(true);
  if(IDOK != result)
    return;
  setOfString set;
  performLoadPage(set);
}
//-----------------------------------------------------------
static bool copyFile(PWin* owner, LPCTSTR target_file, LPCTSTR path_source)
{
  dataProject& dp = getDataProject();
  TCHAR target[_MAX_PATH];
  _tcscpy_s(target, dp.newPath);
  appendPath(target, dp.systemPath);
  appendPath(target, dp.getSubPath());
  createDirectoryRecurse(target);

  LPTSTR t = target + _tcslen(target);
  appendPath(target, target_file);

  if(P_File::P_exist(target)) {
    if(IDYES != MessageBox(*owner, _T("Il file esiste,\r\nvuoi sovrascriverlo?"),
        _T("Warning"), MB_YESNO | MB_ICONINFORMATION))
      return false;
    }
  TCHAR source[_MAX_PATH];
  _tcscpy_s(source, path_source);
  appendPath(source, dp.systemPath);
  appendPath(source, dp.getSubPath());
  LPTSTR s = source + _tcslen(source);
  appendPath(source, dp.getPageName());
  if(CopyFile(source, target, false)) {
    *t = 0;
    *s = 0;
    appendPath(target, _T("#"));
    appendPath(source, _T("#"));
    _tcscat_s(target, target_file);
    _tcscat_s(source, dp.getPageName());
    return CopyFile(source, target, false) != 0;
    }
  return false;
}
//-----------------------------------------------------------
static void deleteFile()
{
  dataProject& dp = getDataProject();
  TCHAR target[_MAX_PATH];
  _tcscpy_s(target, dp.newPath);
  appendPath(target, dp.systemPath);
  appendPath(target, dp.getSubPath());
  LPTSTR t = target + _tcslen(target);
  appendPath(target, dp.getPageName());
  DeleteFile(target);
  *t = 0;
  appendPath(target, _T("#"));
  _tcscat_s(target, dp.getPageName());
  DeleteFile(target);
}
//-----------------------------------------------------------
static void check4Bmp(PWin* owner, setOfString& pageSet, LPCTSTR source_)
{
  PVect<LPCTSTR> set;
  for(uint i = 0; i < 100; ++i) {
    LPCTSTR p = pageSet.getString(ID_INIT_BMP_4_BTN + i);
    if(!p)
      break;
    if(_tcsstr(p, _T("image")))
      set[set.getElem()] = str_newdup(p);
    }
  for(uint i = 0; i < 100; ++i) {
    LPCTSTR p = pageSet.getString(ID_INIT_BMP + i);
    p = findNextParamTrim(p, 5);
    if(!p)
      break;
    if(_tcsstr(p, _T("image")))
      set[set.getElem()] = str_newdup(p);
    }

  dataProject& dp = getDataProject();
  TCHAR target[_MAX_PATH];
  _tcscpy_s(target, dp.newPath);
  LPTSTR t = target + _tcslen(target);
  int nElem = set.getElem();
  for(int i = nElem - 1; i >= 0; --i) {
    appendPath(target, set[i]);
    if(P_File::P_exist(target))
      delete[]set.remove(i);
    *t = 0;
    }
  nElem = set.getElem();
  if(!nElem)
    return;

  actionChoice action = acNo;
  TCHAR source[_MAX_PATH];
  _tcscpy_s(source, source_);
  LPTSTR s = source + _tcslen(source);
  TCHAR info[_MAX_PATH];
  _tcscpy_s(info, _T("Copiare il file bitmap -> "));
  LPTSTR p = info + _tcslen(info);
  for(int i = nElem - 1; i >= 0; --i) {
    if(action != acYesAll) {
      *p = 0;
      _tcscat_s(info, set[i]);
      action = getAction(owner, action, info);
      }
    if(acNoAll == action)
      break;
    if(action != acNo) {
      appendPath(target, set[i]);
      appendPath(source, set[i]);
      CopyFile(source, target, false);
      *t = 0;
      *s = 0;
      }
    }
  flushPAV(set);
}
//-----------------------------------------------------------
static LPCTSTR baseName(LPCTSTR path)
{
  int len = _tcslen(path);
  for(int i = len - 1; i > 0; --i)
    if(_T('\\') == path[i])
      return path + i + 1;
  return path;
}
//-----------------------------------------------------------
void svmMainClient::openPageFromOtherPrj()
{
  TCHAR oldPath[_MAX_PATH];
   _tcscpy_s(oldPath, DataP.newPath);
  EnableWindow(*this, false);
  static TCHAR path[_MAX_PATH];
  if(PChooseFolder(path, _T("Scelta progetto esterno"), false, 0)) {
    _tcscpy_s(DataP.newPath, path);
    TCHAR page[_MAX_PATH] = _T("");
    if(IDOK == getPageFile(this, page).modal()) {
      DataP.setPageName(baseName(page));
      _tcscpy_s(DataP.newPath, oldPath);
      if(copyFile(this, _T("~tmp~.~x~"), path)) {
        DataP.setPageName( _T("~tmp~.~x~"));
        setOfString set;
        if(performLoadPage(set)) {
          deleteFile();
          DataP.setDirty();
          check4Bmp(this, set, path);
          invalidate();
          }
        DataP.setPageName( _T(""));
        setTitle();
        }
      }
    _tcscpy_s(DataP.newPath, oldPath);
    }
  EnableWindow(*this, true);
}
  // salvataggio DataP
  // scelta cartella progetto e impostazione nel DataP
  // richiamo di getPageFile-simpleOpen per scelta file
  // copia dei file nel progetto corrente
  // ripristino DataP
  // caricamento set
  // eliminazione file pagina
  // verifica esistenza bmp e richiesta di copia
  // caricamento pagina ed impostazione dirty
//-----------------------------------------------------------
static LPCTSTR msgFontOver = _T("Ci sono oggetti con Font fuori range!\r\n")
    _T("Scegliere se impostarli al primo font,\r\nproseguire comunque\r\no annullare il salvataggio");
//-----------------------------------------------------------
bool svmMainClient::savePage()
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
  enableTools(false);
  int result = savePageFile(this, DataP).modal();
  enableTools(true);
  if(IDOK != result)
    return false;

  TCHAR crypt[_MAX_PATH];
  _tcscpy_s(crypt, DataP.newPath);
  appendPath(crypt, DataP.systemPath);
  appendPath(crypt, DataP.getSubPath());

  TCHAR clear[_MAX_PATH];
  _tcscpy_s(clear, crypt);

  appendPath(crypt, _T("#"));
  _tcscat_s(crypt, DataP.getPageName());

  appendPath(clear, DataP.getPageName());

  do {
    P_File pfCrypt(crypt);
    pfCrypt.appendback();

    P_File pfClear(clear);
    pfClear.appendback();
    } while(false);

  manageCryptPage mcp;
  bool crypted = mcp.isCrypted();
  if(crypted)
    mcp.makePathCrypt(crypt);

  P_File pfCrypt(crypt, P_CREAT);
  P_File pfClear(clear, P_CREAT);

  if(pfCrypt.P_open()) {
    if(pfClear.P_open()) {
      dual* dualBmp = getDualBmp4Btn();
      dualBmp->reset();
      SIZE sz;
      int type = sizer::getCurr(sz);
      if(sizer::sPersonalized == type)
        wsprintf(crypt, _T("99,%d,%d,%d\r\n"), type, sz.cx, sz.cy);
      else
        wsprintf(crypt, _T("99,%d\r\n"), type - 1);
      writeStringChkUnicode(pfCrypt, crypt);
      int what = DataP.getWhatPage();
      if(what & 1)
        what = -1;
      else
        what >>= 1;

      wsprintf(crypt, _T("999999,%d,%d\r\n"), DataP.getPageType(), what);
      writeStringChkUnicode(pfCrypt, crypt);

      int nBtn = savePropertyPage(pfCrypt, pfClear);
      Base[getCurrPage()]->savePage(pfCrypt, pfClear, nBtn);
      saveCustom(pfCrypt, pfClear);
      dualBmp->save(pfCrypt);
      // se è l'header deve aggiungere l'offset
      if(DataP.getWhatPage() & 1) {
        SIZE sz;
        sizer::eSizer currSize = sizer::getCurr(sz);
        sizer::setDefault(sizer::s800x600);
        int x = R__X(100);
        int y = R__Y(45);
        sizer::setDefault(currSize, &sz);
        wsprintf(crypt, _T("30,%d\r\n31,%d\r\n"), x, y);
        writeStringChkUnicode(pfCrypt, crypt);
        }
      Base[getCurrPage()]->flushHistory();
      setTitle();
      DataP.resetDirty();
      pfCrypt.P_close();
      if(crypted) {
        _tcscpy_s(crypt, DataP.newPath);
        appendPath(crypt, DataP.systemPath);
        appendPath(crypt, DataP.getSubPath());
        appendPath(crypt, _T("#"));
        _tcscat_s(crypt, DataP.getPageName());
        mcp.releaseFileCrypt(crypt);
        }
      return true;
      }
    }
  return false;
}
//-----------------------------------------------------------
void svmMainClient::resetCustom()
{
  TCHAR path[_MAX_PATH];
  makeTmpDirName(path, getCurrPage());
  appendPath(path, CUSTOM_NAME);
  DeleteFile(path);

  makeTmpDirName(path, getCurrPage());
  appendPath(path, SHARP);
  _tcscat_s(path, CUSTOM_NAME);
  DeleteFile(path);
  if(actionToBkg && !onExit)
    setActionToBkg();
}
//-----------------------------------------------------------
void svmMainClient::loadCustomClear(setOfString& set)
{
  TCHAR path[_MAX_PATH];
  makeTmpDirName(path, getCurrPage());
  appendPath(path, CUSTOM_NAME);

  P_File set2(path, P_CREAT);
  if(set2.P_open())
    loadSaveCustom(set2, set, true);
}
//-----------------------------------------------------------
void svmMainClient::loadCustomCrypt(setOfString& set)
{
  TCHAR path[_MAX_PATH];
  makeTmpDirName(path, getCurrPage());
  appendPath(path, SHARP);
  _tcscat_s(path, CUSTOM_NAME);
  P_File set2(path, P_CREAT);
  if(set2.P_open())
    loadSaveCustom(set2, set, true);
}
//-----------------------------------------------------------
void svmMainClient::loadSaveCustom(P_File& target, setOfString& source, bool forceUnicode)
{
  if(source.setLast()) {
    bool unicode = forceUnicode ? true : saveAsUnicode();
    do {
      long id = source.getCurrId();
      if(id < FIRST_CUSTOM_ID)
        break;
      source.writeCurrent(target, unicode);
      } while(source.setPrev());
    }
}
//-----------------------------------------------------------
void svmMainClient::saveCustom(P_File& pfCrypt, P_File& pfClear)
{
  TCHAR path[_MAX_PATH];
  do {
    makeTmpDirName(path, getCurrPage());
    appendPath(path, SHARP);
    _tcscat_s(path, CUSTOM_NAME);

    setOfString set(path);
    loadSaveCustom(pfCrypt, set, false);
    } while(false);
  do {
    makeTmpDirName(path, getCurrPage());
    appendPath(path, CUSTOM_NAME);
    setOfString set(path);
    loadSaveCustom(pfClear, set, false);
    } while(false);
}
//-----------------------------------------------------------
void svmMainClient::copyCustom()
{
  copyPasteCustom* tmpCustom = new copyPasteCustom;

  TCHAR path[_MAX_PATH];
  do {
    makeTmpDirName(path, getCurrPage());
    appendPath(path, SHARP);
    _tcscat_s(path, CUSTOM_NAME);

    tmpCustom->setC.add(path);
    } while(false);
  do {
    makeTmpDirName(path, getCurrPage());
    appendPath(path, CUSTOM_NAME);
    tmpCustom->set.add(path);
    } while(false);

  if(!(tmpCustom->set.getNumElem() + tmpCustom->setC.getNumElem())) {
    delete tmpCustom;
    MessageBox(*this, _T("Nessun codice personalizzato presente!"), _T("Attenzione"), MB_OK | MB_ICONSTOP);
    }
  else {
    if(cpCustom)
      delete cpCustom;
    cpCustom = tmpCustom;
    }
}
//-----------------------------------------------------------
static void msgPasteCustom(HWND hwnd, bool success)
{
  LPCTSTR tit = _T("Scrittura Codici Personalizzati");
  struct infoMsg {
    LPCTSTR msg;
    uint id;
    } iMsg[] = {
    { _T("Fallita!"), MB_ICONSTOP },
    { _T("Successo!"), MB_ICONINFORMATION },
    };
  MessageBox(hwnd, iMsg[success].msg, tit, MB_OK | iMsg[success].id);
}
//-----------------------------------------------------------
static bool saveSet(P_File& pf, setOfString& set)
{
  if(!set.setFirst())
    return true;
  bool success = true;
  do {
    success &= set.writeCurrent(pf, true);
    } while(set.setNext());
  return success;
}
//-----------------------------------------------------------
void svmMainClient::pasteCustom()
{
  if(!cpCustom)
    return;
  if(IDYES != MessageBox(*this, _T("Incollare i codici personalizzati?"), _T("Copia&Incolla Codici"), MB_YESNO | MB_ICONINFORMATION))
    return;
  TCHAR path[_MAX_PATH];
  bool success = false;
  do {
    makeTmpDirName(path, getCurrPage());
    appendPath(path, SHARP);
    _tcscat_s(path, CUSTOM_NAME);
    P_File pf(path, P_CREAT);
    if(pf.P_open())
      success = saveSet(pf, cpCustom->setC);
    } while(false);
  do {
    makeTmpDirName(path, getCurrPage());
    appendPath(path, CUSTOM_NAME);
    P_File pf(path, P_CREAT);
    if(pf.P_open())
      success &= saveSet(pf, cpCustom->set);
    } while(false);
  msgPasteCustom(*this, success);
  getDataProject().setDirty();
}
//-----------------------------------------------------------
void svmMainClient::resetPropertyPage()
{
  TCHAR path[_MAX_PATH];
  makeTmpDirName(path, getCurrPage());
  appendPath(path, PAGE_PROP_NAME);
  DeleteFile(path);
}
//-----------------------------------------------------------
#ifdef _UNICODE
  #define M_VER 7
  #define m_VER 0
#else
  #define M_VER 4
  #define m_VER 6
#endif
//-----------------------------------------------------------
#define DEF_VERSION MAKEWORD(m_VER, M_VER)
//-----------------------------------------------------------
#define DEF_OLD_VERSION MAKEWORD(0, 4)
//-----------------------------------------------------------
static WORD getVersion(LPCTSTR p)
{
  if(!p)
    return 0;
  DWORD high = 0;
  DWORD low = 0;
  _stscanf_s(p, _T("%d,%d"), &high, &low);
  return MAKEWORD(low, high);
}
//-----------------------------------------------------------
void makeVersion(LPTSTR p)
{
  wsprintf(p, _T("%d,%d"), M_VER, m_VER);
}
//-----------------------------------------------------------
static
bool acceptVersion(PWin* owner, LPCTSTR p, WORD version)
{
  TCHAR title[200];
  TCHAR msg[200];
  wsprintf(msg, _T("Versione corrente %d.%d\r\nVuoi continuare lo stesso?"), M_VER, m_VER);

  if(!p || !version)
    wsprintf(title, _T("Questa pagina non ha il codice di Versione"));
  else
    wsprintf(title, _T("Questa pagina ha la versione %d.%d"), HIBYTE(version), LOBYTE(version));
  return IDYES == MessageBox(*owner, msg, title, MB_YESNO | MB_ICONINFORMATION);
}
//-----------------------------------------------------------
bool svmMainClient::loadPropertyPage(setOfString& set)
{
  TCHAR path[_MAX_PATH];
  makeTmpDirName(path, getCurrPage());
  appendPath(path, PAGE_PROP_NAME);

  P_File pf(path, P_CREAT);
  if(!pf.P_open())
    return false;
  LPCTSTR p = set.getString(ID_VERSION);
  WORD version = getVersion(p);
  // è compatibile con DEF_OLD_VERSION, quindi se la trova la dà per buona
  if(!p || version != DEF_VERSION && version != DEF_OLD_VERSION)
    if(!acceptVersion(this, p, version))
      return false;
  savePropertyPageGen(set, pf, pf, true);
  return true;
}
//-----------------------------------------------------------
int svmMainClient::savePropertyPage(P_File& pfCrypt, P_File& pfClear)
{
  TCHAR path[_MAX_PATH];
  makeTmpDirName(path, getCurrPage());
  appendPath(path, PAGE_PROP_NAME);

  setOfString set(path);
  do {
    TCHAR buff[20];
    wsprintf(buff, _T("%d,%d"), M_VER, m_VER);
    set.replaceString(ID_VERSION, str_newdup(buff), true);
    } while(false);
  return savePropertyPageGen(set, pfCrypt, pfClear, saveAsUnicode());
}
//-----------------------------------------------------------
static
int idStandard[] = {
          ID_DEF_SEND_EDIT,
          ID_SHOW_SIMPLE_SEND,
          ID_MODAL_RECT,
          ID_NO_CAPTION_MODAL,
          ID_INIT_BMP,
          ID_VERSION,
          ID_MODELESS_FIX,
          ID_LINK_PAGE,
          ID_LINK_PAGE_DYN,
          ID_CODE_BTN_FZ_BEHA,
          ID_SEND_VALUE_ON_OPEN,
          ID_SEND_VALUE_ON_CLOSE,
          };
//-----------------------------------------------------------
void resetSavePredef(setOfString& Set)
{
  setOfString tmp;
  for(uint i = 0; i < SIZE_A(idStandard); ++i) {
    int id = idStandard[i];
    LPCTSTR p = Set.getString(id);
    if(p) {
      Set.replaceString(id, 0, false, false);
      tmp.replaceString(id, (LPTSTR)p, true);
      }
    }
  Set.reset();
  while(tmp.setFirst()) {
    int id = tmp.getCurrId();
    LPCTSTR p = tmp.getString(id);
    tmp.replaceString(id, 0, false, false);
    Set.replaceString(id, (LPTSTR)p, true);
    }
}
//-----------------------------------------------------------
int svmMainClient::savePropertyPageGen(setOfString& set, P_File& pfCrypt, P_File& pfClear, bool onUnicode)
{
  if(set.getString(ID_TITLE))
    set.writeCurrent(pfClear, onUnicode);
  int nBtnSendOne = 0;

  uint ids[] =  { ID_INIT_MENU, ID_F13, ID_S5 };

  for(uint k = 0; k < 3; ++k) {
    for(int i = 0; i < MAX_BTN; ++i) {
      if(set.getString(ids[k] + i))
        set.writeCurrent(pfClear, onUnicode);
      LPCTSTR p = set.getString(ids[k] + MAX_BTN + i);
      if(p) {
        set.writeCurrent(pfCrypt, onUnicode);
        uint code = _ttoi(p);
        if(aSendOne == code || aSendToggle == code) {
          ++nBtnSendOne;
          p = findNextParam(p, 2);
          if(p) {
            int idBtn = _ttoi(p);
            if(set.getString(idBtn))
              set.writeCurrent(pfCrypt, onUnicode);
            idBtn += ADD_INIT_VAR;
            if(set.getString(idBtn))
              set.writeCurrent(pfCrypt, onUnicode);
            idBtn += ADD_INIT_BITFIELD;
            if(set.getString(idBtn))
              set.writeCurrent(pfCrypt, onUnicode);
            }
          }
        }
      }
    }
  for(uint i = 0; i < SIZE_A(idStandard); ++i) {
    int id = idStandard[i];
    if(set.getString(id))
      set.writeCurrent(pfCrypt, onUnicode);
    }

  return nBtnSendOne;
}
//-----------------------------------------------------------
static void get_Coord(PWin* win, int flag, PBitmap* bmp, POINT& pt, SIZE& scale)
{
  switch(flag) {
    case bUSE_COORD:
    case bTILED:
    // questo è valido solo per non trasparente, ed è banale il calcolo
    case bFILLED:
      break;

    case bCENTERED:
      do {
        PRect r;
        GetClientRect(*win, r);
        getDataProject().calcFromScreen(r);
        SIZE sz = bmp->getSize();

        sz.cx *= scale.cx;
        sz.cx /= 1000;
        sz.cy *= scale.cy;
        sz.cy /= 1000;

        pt.x = (r.Width() - R__X(sz.cx)) / 2;
        pt.y = (r.Height() - R__Y(sz.cy)) / 2;
        } while(false);
      break;

    case bSCALED:
      do {
        PRect r;
        GetClientRect(*win, r);
        getDataProject().calcFromScreen(r);
        SIZE sz = bmp->getSize();
        double rX = r.Width();
        rX /= sz.cx;
        double rY = r.Height();
        rY /= sz.cy;
        if(rX > rY) {
          scale.cx = sz.cy * 1000 / r.Height();
          scale.cy = scale.cx;
          pt.x = (LONG)((r.Width() - sz.cx * 1000.0 / scale.cx) / 2);
          pt.y = 0;
          }
        else {
          scale.cx = sz.cx * 1000 / r.Width();
          scale.cy = scale.cx;
          pt.y = (LONG)((r.Height() - sz.cy * 1000.0 / scale.cx) / 2);
          pt.x = 0;
          }
        } while(false);
      break;
    }

}
//-----------------------------------------------------------
void svmMainClient::drawBmp(HDC hdc, LPCTSTR p)
{
  if(!p)
    return;
  int x = 0;
  int y = 0;
  int scaleX = 0;
  int scaleY = 0;
  int flag = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &x, &y, &scaleX, &scaleY, &flag);

  bool transp = false;
  if(flag >= 1000) {
    transp = true;
    flag -= 1000;
    }

  LPCTSTR name = findNextParam(p, 5);

  if(!name)
    return;

  TCHAR fullName[_MAX_PATH];
  _tcscpy_s(fullName, DataP.newPath);
  appendPath(fullName, name);
  PBitmap* bmp = new PBitmap(fullName);

  if(!bmp->isValid()) {
    delete bmp;
    return;
    }

  // annulla il flag di trasparenza se non permesso
  switch(flag) {
    case bTILED:
    case bFILLED:
      transp = false;
      break;
    case bCENTERED:
    case bSCALED:
    case bUSE_COORD:
      break;
    }
  POINT pt = { R__X(x), R__Y(y) };
  SIZE scale = { scaleX, scaleY };
  if(!scale.cx)
    scale.cx = 1000;
  if(!scale.cy)
    scale.cy = 1000;
  if(transp) {
    switch(flag) {
      case bUSE_COORD:
        break;

      case bCENTERED:
      case bSCALED:
        get_Coord(this, flag, bmp, pt, scale);
        break;
      }

    PTraspBitmap tbmp(this, bmp, pt, false);
    if(bSCALED != flag)
      tbmp.setScale(R__Xf(scale.cx / 1000.0), R__Yf(scale.cy / 1000.0));
    else
      tbmp.setScale(1000.0 / scale.cx, 1000.0 / scale.cy);
    tbmp.Draw(hdc);
    }
  else {
    SIZE sz = bmp->getSize();
    switch(flag) {
      case bUSE_COORD:
      case bCENTERED:
        get_Coord(this, flag, bmp, pt, scale);
        sz.cx = R__X(sz.cx / 1000.0 * scale.cx);
        sz.cy = R__Y(sz.cy / 1000.0 * scale.cy);
        break;
      case bSCALED:
        get_Coord(this, flag, bmp, pt, scale);
        sz.cx = (long)(sz.cx * 1000.0 / scale.cx);
        sz.cy = (long)(sz.cy * 1000.0 / scale.cy);
        break;

      case bFILLED:
        do {
          pt.x = 0;
          pt.y = 0;
          PRect r;
          GetClientRect(*this, r);
          getDataProject().calcFromScreen(r);
          sz.cx = r.Width();
          sz.cy = r.Height();
          } while(false);
        break;
      }
    if(bTILED != flag)
      bmp->draw(hdc, pt, sz);

    else {
      pt.x = 0;
      pt.y = 0;
      PRect r;
      GetClientRect(*this, r);
      getDataProject().calcFromScreen(r);

      if(scale.cx && scale.cx > 10 && scale.cy && scale.cy > 10) {
        int width = (int)(sz.cx / 1000.0 * scale.cx);
        int height = (int)(sz.cy / 1000.0 * scale.cy);
        HDC memDC1 = CreateCompatibleDC(hdc);
        HBITMAP hBmp1 = CreateCompatibleBitmap(hdc, width , height);
        HGDIOBJ old1 = SelectObject(memDC1, (HGDIOBJ)hBmp1);

        HDC memDC2 = CreateCompatibleDC(hdc);
        HGDIOBJ old2 = SelectObject(memDC2, *bmp);

        int stretchMode = HALFTONE;
        if(isWin95() || isWin98orLater())
          stretchMode = COLORONCOLOR;

        int oldMode = SetStretchBltMode(memDC1, stretchMode);
        StretchBlt(memDC1, 0, 0, width, height,
                           memDC2, 0, 0, sz.cx, sz.cy, SRCCOPY);

        SetStretchBltMode(memDC1, oldMode);

        SelectObject(memDC2, (HGDIOBJ)old2);

        delete bmp;
        bmp = new PBitmap(hBmp1, true);

        SelectObject(memDC1, (HGDIOBJ)old1);

        DeleteDC(memDC1);
        DeleteDC(memDC2);
        sz.cx = width;
        sz.cy = height;
        }
      bmp->setQualityOnStretch(true);
      bmp->drawTiled(hdc, r);
      }
    }
  delete bmp;
}
//-----------------------------------------------------------
bool svmMainClient::isUsedFontId(uint idfont)
{
  return Base[getCurrPage()]->isUsedFontId(idfont);
}
//-----------------------------------------------------------
void svmMainClient::getUsedFontId(PVect<uint>& set)
{
   Base[getCurrPage()]->getUsedFontId(set);
}
//-----------------------------------------------------------
void svmMainClient::decreaseFontIfAbove(uint id_font)
{
   if(Base[getCurrPage()]->decreaseFontIfAbove(id_font))
      getDataProject().setDirty();
}
//-----------------------------------------------------------
bool svmMainClient::resetFontIfAbove(uint id_font)
{
  return Base[getCurrPage()]->resetFontIfAbove(id_font);
}
//-----------------------------------------------------------
bool svmMainClient::isFontAbove(uint id_font)
{
  return Base[getCurrPage()]->isFontAbove(id_font);
}
//---------------------------------------------------------------------
void makeNameImage(LPTSTR target, LPCTSTR name)
{
  _tcscpy_s(target, _MAX_PATH, DataP.newPath);
  appendPath(target, DataP.imagePath);
  appendPath(target, name);
}
//---------------------------------------------------------------------
void makeRelNameImage(LPTSTR target, LPCTSTR name, bool duplicateSlash)
{
  _tcscpy_s(target, _MAX_PATH, DataP.imagePath);
  if(duplicateSlash) {
    appendPath(target, _T("\\"));
    TCHAR t[_MAX_PATH];
    translateFromCRNL(t, name);
    appendPath(target, t);
    }
  else
    appendPath(target, name);
}
