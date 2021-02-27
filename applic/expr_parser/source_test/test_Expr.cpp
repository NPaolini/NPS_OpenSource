//----------- test_Expr.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commdlg.h>
#include <commctrl.h>
#include <stdio.h>
#include <shlwapi.h>
#include "test_Expr.h"
#include "p_txt.h"
#include "PEdit.h"
#include "pCommonFilter.h"
#include "POwnBtnImage.h"
#include "macro_utils.h"
#include "config.h"
#include "p_manage_ini.h"
//----------------------------------------------------------------------------
class myManageIni : public manageIni
{
  private:
    typedef manageIni baseClass;
  public:
    myManageIni(LPCTSTR filename);
};
//----------------------------------------------------------------------------
static void makeModulePath(LPTSTR target, size_t dim,  LPCTSTR filename)
{
  LPCTSTR pf = filename + _tcslen(filename) - 1;
  while(pf > filename) {
    if(_T('\\') == *pf || _T('/') == *pf) {
      ++pf;
      break;
      }
    --pf;
    }
  GetModuleDirName(dim, target);
  appendPath(target, pf);
}
//----------------------------------------------------------------------------
myManageIni::myManageIni(LPCTSTR filename) : baseClass(filename)
{
  TCHAR t[_MAX_PATH];
  makeModulePath(t, SIZE_A(t), filename);
  delete []Filename;
  Filename = str_newdup(t);
}
//----------------------------------------------------------------------------
#define SUB_BASE_KEY _T("Software\\NPS_Programs\\svTestExpression")
#define LAST_PATH   _T("path")
#define COORDS_PATH _T("coords")
#define SIZE_PATH   _T("size")
#define PRECISION_PATH   _T("precision")
#define SPLIT_1_2_PATH   _T("split_1_2")
#define SPLIT_3_4_PATH   _T("split_3_4")
#define SPLIT_5_PATH   _T("split_5")

#define COORDS_SEARCH_REPLACE _T("search_replace")

#define CALC_PATH _T("Calc")
#define INIT_PATH _T("Init")
#define VARS_PATH _T("Vars")
#define FUNCT_PATH _T("Funct")
#define TEST_PATH _T("Test")
#define RESULT_PATH _T("Result")
  #define REG_FG          _T("Foreground")
  #define REG_BG          _T("Background")
  #define REG_FNT_HEIGHT  _T("Fnt_Height")
  #define REG_FNT_STYLE   _T("Fnt_Style")
  #define REG_FNT_NAME    _T("Fnt_name")
#define MAIN_PATH _T("\\Main")
#define TEST_EXPR_INI_FILE _T("test_Expr.ini")
//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR path)
{
  myManageIni ini(TEST_EXPR_INI_FILE);
  ini.parse();
  ini_param param = { keyName, path };
  ini.addParam(param, MAIN_PATH);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPTSTR path)
{
  path[0] = 0;
  myManageIni ini(TEST_EXPR_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
  if(str)
    _tcscpy_s(path, _MAX_PATH, str);
}
//----------------------------------------------------------------------------
void getKeyPrec(LPTSTR path)
{
  getKeyPath(PRECISION_PATH, path);
}
//----------------------------------------------------------------------------
void setKeyPrec(LPTSTR path)
{
  setKeyPath(PRECISION_PATH, path);
}
//----------------------------------------------------------------------------
void setKeyParam(LPCTSTR keyName, DWORD value)
{
  myManageIni ini(TEST_EXPR_INI_FILE);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, MAIN_PATH);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyParam(LPCTSTR keyName, DWORD& value)
{
  myManageIni ini(TEST_EXPR_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    value = (DWORD)t;
    }
}
//----------------------------------------------------------------------------
template <typename T>
void getKeyParam(myManageIni& ini, LPCTSTR keyName, LPCTSTR keyBase, T& value)
{
  LPCTSTR str = ini.getValue(keyName, keyBase);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    value = (DWORD)t;
  }
}
//----------------------------------------------------------------------------
void getKeyInfoParam(LPCTSTR keyBase, infoEdit& ied)
{
  myManageIni ini(TEST_EXPR_INI_FILE);
  ini.parse();
  for(uint i = 0; i < MAX_INFO_COLOR; ++i) {
    TCHAR keyname[256];
    wsprintf(keyname, _T("%s-%d"), REG_FG, i + 1);
    getKeyParam(ini, keyname, keyBase, ied.fg[i]);
    }
  getKeyParam(ini, REG_BG, keyBase, ied.bg);
  getKeyParam(ini, REG_FNT_HEIGHT, keyBase, ied.fontHeight);
  getKeyParam(ini, REG_FNT_STYLE, keyBase, ied.fontStyle);
  LPCTSTR str = ini.getValue(REG_FNT_NAME, keyBase);
  if(str)
    _tcscpy_s(ied.fontName, str);
}
//----------------------------------------------------------------------------
void setKeyParam(myManageIni& ini, LPCTSTR keyName, LPCTSTR keyBase, DWORD value)
{
  TCHAR t[64];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, keyBase);
}
//----------------------------------------------------------------------------
void setKeyInfoParam(LPCTSTR keyBase, const infoEdit& ied)
{
  myManageIni ini(TEST_EXPR_INI_FILE);
  ini.parse();
  for(uint i = 0; i < MAX_INFO_COLOR; ++i) {
    TCHAR keyname[256];
    wsprintf(keyname, _T("%s-%d"), REG_FG, i + 1);
    setKeyParam(ini, keyname, keyBase, ied.fg[i]);
    }
  setKeyParam(ini, REG_BG, keyBase, ied.bg);
  setKeyParam(ini, REG_FNT_HEIGHT, keyBase, ied.fontHeight);
  setKeyParam(ini, REG_FNT_STYLE, keyBase, ied.fontStyle);

  ini_param param = { REG_FNT_NAME, ied.fontName };
  ini.addParam(param, keyBase);
  ini.save();
}
//----------------------------------------------------------------------------
bool needMaximized()
{
  DWORD coords = 0;
  getKeyParam(COORDS_PATH, coords);
  return (DWORD)-1 == coords;
}
//----------------------------------------------------------------------------
bool isMaximized(HWND hwnd)
{
  WINDOWPLACEMENT wp;
  ZeroMemory(&wp, sizeof(wp));
  wp.length = sizeof(wp);
  if(GetWindowPlacement(hwnd, &wp))
    return SW_SHOWMAXIMIZED == (SW_SHOWMAXIMIZED & wp.showCmd); // MaximizedWin;
  return false;
}
//----------------------------------------------------------------------------
#define TEST_TMP_FILE _T("~testSvx.Sv~")
//----------------------------------------------------------------------------
#define H_TOP 32

#define ID_BOTTOM 201
#define ID_BODY_LEFT 202
#define ID_BODY_RIGHT 203
#define ID_BODY_LEFT_TOP 204
#define ID_BODY_CENTER_TOP 205
//-------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xdf)
//#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor1 YELLOW_COLOR
//-------------------------------------------------------------------
#define CYAN_COLOR  RGB(0xdf,0xff,0xff)
#define bkgColor2 CYAN_COLOR
//----------------------------------------------------------------------------
class mySplitWin : public PSplitWin
{
  public:
    mySplitWin(PWin* parent, uint id, uint perc = 500, uint width = 3, uint style = PSplitter::PSW_HORZ, HINSTANCE hInst = 0) :
      PSplitWin(parent, id, perc, width, style, hInst) { Attr.style |= WS_CHILD ; }
    virtual ~mySplitWin() { destroy(); }
};
//----------------------------------------------------------------------------
#define ret_parFunct(w, f)  \
  test_Expr* te = getParentWin<test_Expr>(w);\
  if(te) \
    return te->f(); \
  return 0
//----------------------------------------------------------------------------
HWND getEditCtrlResult(PWin* owner)
{
  ret_parFunct(owner, getEditCtrlResult);
//  return ((test_Expr*)owner)->getEditCtrlResult();
}
//----------------------------------------------------------------------------
HWND getEditCtrlInit(PWin* owner)
{
  ret_parFunct(owner, getEditCtrlInit);
//  return ((test_Expr*)owner)->getEditCtrlInit();
}
//----------------------------------------------------------------------------
HWND getEditCtrlTest(PWin* owner)
{
  ret_parFunct(owner, getEditCtrlTest);
//  return ((test_Expr*)owner)->getEditCtrlTest();
}
//----------------------------------------------------------------------------
HWND getEditCtrlCalc(PWin* owner)
{
  ret_parFunct(owner, getEditCtrlCalc);
//  return ((test_Expr*)owner)->getEditCtrlCalc();
}
//----------------------------------------------------------------------------
HWND getEditCtrlFunct(PWin* owner)
{
  ret_parFunct(owner, getEditCtrlFunct);
//  return ((test_Expr*)owner)->getEditCtrlFunct();
}
//----------------------------------------------------------------------------
bool findFunct(PWin* owner, LPCTSTR functName, CHARRANGE& cr)
{
  test_Expr* te = getParentWin<test_Expr>(owner);
  if(te)
    return te->findFunct(functName, cr);
  return false;
}
//-----------------------------------------------------------
static void checkOut(int& x, int& y)
{
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);

  if(x < 0)
    x = 0;
  else if(x > width - 100)
    x = width - 100;
  if(y < 0)
    y = 0;
  else if(y > height - 100)
    y = height - 100;
}
//----------------------------------------------------------------------------
static HINSTANCE hinstRichEdit;
static LPCTSTR RichClass;
static bool onRichEdit2;
LPCTSTR getClassName_Rich() { return RichClass; }
bool useRichEdit2() { return onRichEdit2; }
//----------------------------------------------------------------------------
test_Expr::test_Expr(LPCTSTR title, HINSTANCE hinst) :
      baseClass(title, hinst), currState(sUndef), Split(0), objSearchReplace(0)
{

  hinstRichEdit = LoadLibrary(_T("Msftedit.dll"));
  if(hinstRichEdit)
    RichClass = MSFTEDIT_CLASS;
  else {
    hinstRichEdit = LoadLibrary(_T("Riched20.dll"));
    if(hinstRichEdit) {
      RichClass = RICHEDIT_CLASS;
      onRichEdit2 = true;
      }
    else {

      hinstRichEdit = LoadLibrary(_T("riched32.dll"));
      RichClass = _T("RichEdit");
      }
    }
  DWORD split_1_2 = 0;
  getKeyParam(SPLIT_1_2_PATH, split_1_2);
  DWORD split_3_4 = 0;
  getKeyParam(SPLIT_3_4_PATH, split_3_4);
  DWORD split_5 = 0;
  getKeyParam(SPLIT_5_PATH, split_5);

  topBtn = new PDialogBtn(this, IDD_BUTTON);
  int perc = 700;
  if(LOWORD(split_1_2))
    perc = LOWORD(split_1_2);

  PSplitWin* bottom = new mySplitWin(this, ID_BOTTOM, perc);
  Split = new PSplitter(this, topBtn, bottom, 100, 1, PSplitter::PSW_VERT);
  Split->setLock(PSplitter::PSL_FIRST);
  Split->setDrawStyle(PSplitter::PSD_NONE);
  Split->setDim1(H_TOP);

  perc = 300;
  if(HIWORD(split_1_2))
    perc = HIWORD(split_1_2);
  PSplitWin* bodyLeft = new mySplitWin(bottom, ID_BODY_LEFT, perc);
  perc = 500;
  if(LOWORD(split_3_4))
    perc = LOWORD(split_3_4);
  PSplitWin* bodyRight = new mySplitWin(bottom, ID_BODY_RIGHT, perc, 3, PSplitter::PSW_VERT);
  bottom->setWChild(bodyLeft, bodyRight);

  perc = 800;
  if(HIWORD(split_3_4))
    perc = HIWORD(split_3_4);
  PSplitWin* bodyLeftTop = new mySplitWin(bodyLeft, ID_BODY_LEFT_TOP, perc, 3, PSplitter::PSW_VERT);
  perc = 300;
  if(LOWORD(split_5))
    perc = LOWORD(split_5);
  PSplitWin* bodyCenterTop = new mySplitWin(bodyLeft, ID_BODY_CENTER_TOP, perc, 3, PSplitter::PSW_VERT);
  bodyLeft->setWChild(bodyLeftTop, bodyCenterTop);

  infoEdit ed;
  ed.fg[0] = RGB(0, 0, 0);
  ed.bg =  RGB(0xff,0xff,0xff);
  ed.fontHeight = 10;
  _tcscpy_s(ed.fontName, _T("courier new"));

//enum richEditStyle { eRichBase, eRichGoto, eRichLabel, eRichComment, eRichMax };
  ed.fg[1] = RGB(0, 0, 255);
  ed.fg[2] = RGB(196, 0, 0);
  ed.fg[3] = RGB(0, 128, 0);
//  ed[3].fg = RGB(92, 92, 92);

  getKeyInfoParam(FUNCT_PATH, ed);
  bodyFunct = new exp_BaseContainer(bodyCenterTop, IDC_EDIT_FUNCT, _T("sezione <FUNCT>"), &ed);
  getKeyInfoParam(CALC_PATH, ed);
  bodyCalc = new exp_BaseContainer(bodyCenterTop, IDC_EDIT_CALC, _T("sezione <CALC>"), &ed);
  bodyCenterTop->setWChild(bodyFunct, bodyCalc);

  getKeyInfoParam(VARS_PATH, ed);
  bodyVars = new exp_BaseContainer(bodyLeftTop, IDC_EDIT_VARS, _T("sezione <VARS>"), &ed);
  getKeyInfoParam(INIT_PATH, ed);
  bodyInit = new exp_BaseContainer(bodyLeftTop, IDC_EDIT_INIT, _T("sezione <INIT>"), &ed);

  bodyLeftTop->setWChild(bodyVars, bodyInit);

  ed.bg = bkgColor1;
  getKeyInfoParam(TEST_PATH, ed);
  bodyTest = new exp_BaseContainer(bodyRight, IDC_EDIT_TEST, _T("Immissione valori di test"), &ed);
  getKeyInfoParam(RESULT_PATH, ed);
  bodyResult = new exp_BaseContainer(bodyRight, IDC_EDIT_RESULT, _T("Risultati"), &ed, true);
  bodyRight->setWChild(bodyTest, bodyResult);

  forSearchRepace[0] = bodyVars;
  forSearchRepace[1] = bodyInit;
  forSearchRepace[2] = bodyCalc;
  forSearchRepace[3] = bodyFunct;
}
/*
gerarchia:
  Split
    topBtn
    bottom
      bodyLeft
        bodyLeftTop
          bodyVars
          bodyInit
        bodyCenterTop
          bodyFunct
          bodyCalc
      bodyRight
        bodyTest
        bodyResult
*/
//------------------------------------------------------------------
void test_Expr::savePerc()
{
  PSplitWin* bottom = (PSplitWin*)Split->getW2();
  int perc1 = bottom->getSplitter()->getPerc();
  PSplitWin* sp = (PSplitWin*)bottom->getSplitter()->getW1(); // bodyleft
  int perc2 = sp->getSplitter()->getPerc();
  setKeyParam(SPLIT_1_2_PATH, MAKELONG(perc1, perc2));

  PSplitWin* sp2  = (PSplitWin*)sp->getSplitter()->getW2(); // bodyCenterTop
  sp = (PSplitWin*)sp->getSplitter()->getW1(); // bodyLeftTop
  perc2 = sp->getSplitter()->getPerc();

  sp = (PSplitWin*)bottom->getSplitter()->getW2(); // bodyRight
  perc1 = sp->getSplitter()->getPerc();
  setKeyParam(SPLIT_3_4_PATH, MAKELONG(perc1, perc2));

  perc1 = sp2->getSplitter()->getPerc();
  setKeyParam(SPLIT_5_PATH, MAKELONG(perc1, 0));

  struct infoChild
  {
    exp_BaseContainer* child;
    LPCTSTR base;
  };
  infoChild childs[] = {
    { bodyCalc,   CALC_PATH },
    { bodyFunct,  FUNCT_PATH },
    { bodyInit,   INIT_PATH },
    { bodyVars,   VARS_PATH },
    { bodyTest,   TEST_PATH },
    { bodyResult, RESULT_PATH }
    };
  for(uint i = 0; i < SIZE_A(childs); ++i) {
    const infoEdit& ed = childs[i].child->getInfo();
    setKeyInfoParam(childs[i].base, ed);
    }
}
//----------------------------------------------------------------------------
test_Expr::~test_Expr()
{
  destroy();
  delete Split;
  if(hinstRichEdit) {
    FreeLibrary(hinstRichEdit);
    hinstRichEdit = 0;
    }
}
//-----------------------------------------------------------
bool test_Expr::create()
{
  Attr.style |= WS_CLIPCHILDREN;
//  Attr.style |= WS_CLIPCHILDREN | WS_THICKFRAME;
  Attr.exStyle |= WS_EX_WINDOWEDGE;
//  Attr.exStyle |= WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE;
  DWORD coords = 0;
  getKeyParam(COORDS_PATH, coords);
  if(coords && (DWORD)-1 != coords) {
    Attr.x = (short)LOWORD(coords);
    Attr.y = (short)HIWORD(coords);
    checkOut(Attr.x, Attr.y);
    getKeyParam(SIZE_PATH, coords);
    if(coords && (DWORD)-1 != coords) {
      Attr.w = (short)LOWORD(coords);
      Attr.h = (short)HIWORD(coords);
      }
    }

  if(!baseClass::create())
    return false;

  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));
  PostMessage(*this, WM_SIZE, 0, 0);
  return true;
}
//----------------------------------------------------------------------------
bool test_Expr::preProcessMsg(MSG& msg)
{
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
HWND test_Expr::getEditCtrlResult()
{
  return *bodyResult->getEdit();
}
//----------------------------------------------------------------------------
HWND test_Expr::getEditCtrlInit()
{
  return *bodyInit->getEdit();
}
//----------------------------------------------------------------------------
HWND test_Expr::getEditCtrlTest()
{
  return *bodyTest->getEdit();
}
//----------------------------------------------------------------------------
HWND test_Expr::getEditCtrlCalc()
{
  return *bodyCalc->getEdit();
}
//----------------------------------------------------------------------------
HWND test_Expr::getEditCtrlFunct()
{
  return *bodyFunct->getEdit();
}
//----------------------------------------------------------------------------
LRESULT test_Expr::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      if(isMaximized(hwnd)) {
        setKeyParam(COORDS_PATH, (DWORD)-1);
        }
      else {
        PRect r;
        GetWindowRect(hwnd, r);
        DWORD t = MAKELONG(r.left, r.top);
        setKeyParam(COORDS_PATH, t);
        t = MAKELONG(r.Width(), r.Height());
        setKeyParam(SIZE_PATH, t);
        }
      savePerc();
      PostQuitMessage(0);
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_RUN:
          run();
          break;
        case IDC_BUTTON_SAVE:
          save(false);
          break;
        case IDC_BUTTON_SAVEAS:
          save(true);
          break;
        case IDC_BUTTON_SEARCH_FILE:
          getFile();
          break;
        case IDC_BUTTON_SETUP:
          setup();
          break;
        case IDC_BUTTON_SEARCH:
          searchReplace();
          break;
        case IDC_BUTTON_DONE:
          if(!checkSave())
            break;
          deleteSearchReplace();
          PostQuitMessage(0);
          break;
        case IDC_CHECK_READ_ONLY:
          checkReadOnly();
          break;
        }
      break;
    case WM_SYSCOMMAND:
      switch(wParam & 0xFFF0) {
        case SC_CLOSE:
          if(!checkSave())
            return true;
          deleteSearchReplace();
          break;
        }
      break;

/*
    case WM_SIZE:
      if(SIZE_MAXIMIZED == wParam) {
        setKeyParam(COORDS_PATH, (DWORD)-1);
        setMaximized(true);
        }
//      else if(SIZE_RESTORED == wParam)
//        setMaximized(false);
      break;
*/
    }
  if(Split)
    Split->windowProcSplit(hwnd, message, wParam, lParam);
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//------------------------------------------------------------------
void test_Expr::checkReadOnly()
{
  bool disable = topBtn->isCheckedReadOnly();
  if(disable && !checkSave())
    return;
  topBtn->enableSave(!disable);
  exp_BaseContainer* body[] = { bodyCalc, bodyFunct, bodyInit, bodyVars, bodyTest };
  for(uint i = 0; i < SIZE_A(body); ++i)
    body[i]->setReadOnly(disable);
}
//------------------------------------------------------------------
void test_Expr::deleteSearchReplace()
{
  if(objSearchReplace) {
    PRect r;
    GetWindowRect(*objSearchReplace, r);
    setKeyParam(COORDS_SEARCH_REPLACE, MAKELONG(r.left, r.top));
    SetActiveWindow(*this);
    delete objSearchReplace;
    objSearchReplace = 0;
    }
}
//-----------------------------------------------------------
void test_Expr::searchReplace()
{
  bool isReadOnly = topBtn->isCheckedReadOnly();
  if(!objSearchReplace) {
    objSearchReplace = new PDialogSearchReplace(this, IDD_FIND_REPLACE, forSearchRepace, isReadOnly);
    objSearchReplace->create();
    DWORD coord = (DWORD)-1;
    getKeyParam(COORDS_SEARCH_REPLACE, coord);
    if((DWORD)-1 != coord) {
      int x = (short)LOWORD(coord);
      int y = (short)HIWORD(coord);
      checkOut(x, y);
      SetWindowPos(*objSearchReplace, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
      }
    }
  else
    objSearchReplace->setCurrSel(isReadOnly);
}
//-----------------------------------------------------------
#include "popensave.h"
//----------------------------------------------------------------------------
class myOpenSave : public POpenSave
{
  private:
    typedef POpenSave baseClass;
  public:
    myOpenSave(HWND owner) : baseClass(owner)   { checkPath(); }
  protected:
    virtual void setPathFromHistory();
    virtual void savePathToHistory();
    void setInitialFile(infoOpenSave& info);
    void checkPath();
    static bool firstTime;
};
//----------------------------------------------------------------------------
bool myOpenSave::firstTime = true;
//----------------------------------------------------------------------------
void myOpenSave::checkPath()
{
  if(firstTime) {
    firstTime = false;
    int successCount = 0;
    WIN32_FIND_DATA FindFileData;
    LPCTSTR toFind[] = {
      _T("nps_svisor*.exe"),
      _T("prph_*.exe"),
      _T("prph_*.ad?"),
      _T("prph_*.dat"),
      _T("job_base.dat"),
      _T("system\\main_page.npt"),
      _T("system\\base_data.npt"),
      };
    for(uint i = 0; i < SIZE_A(toFind); ++i) {
      HANDLE hf = FindFirstFile(toFind[i], &FindFileData);
      if(hf != INVALID_HANDLE_VALUE) {
        ++successCount;
        FindClose(hf);
        }
      }
    if(successCount >= 4) {
      TCHAR path[_MAX_PATH];
      GetCurrentDirectory(SIZE_A(path), path);
      setKeyPath(LAST_PATH, path);
      }
  }
}
//----------------------------------------------------------------------------
void myOpenSave::setInitialFile(infoOpenSave& info)
{
  if(info.getFile()) {
    LPCTSTR file = info.getFile();
    int len = _tcslen(file);
    for(int i = len - 1; i > 0; --i) {
      if(_T('\\') == file[i] || _T('/') == file[i]) {
        _tcscpy_s(filename, file + i + 1);
        return;
        }
      }
    }
  baseClass::setInitialFile(info);
}
//----------------------------------------------------------------------------
void myOpenSave::setPathFromHistory()
{
  TCHAR t[_MAX_PATH] = _T("");
  getKeyPath(LAST_PATH, t);
  if(*t)
    SetCurrentDirectory(t);
}
//----------------------------------------------------------------------------
void myOpenSave::savePathToHistory()
{
  TCHAR t[_MAX_PATH] = _T("");
  GetCurrentDirectory(_MAX_PATH, t);
  setKeyPath(LAST_PATH, t);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR filterExtSVX[] = { _T(".svx"), _T(".*"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterSVX =
  _T("expression file (svx)\0")
  _T("*.svx\0")
  _T("tutti i file (*.*)\0")
  _T("*.*\0");
//------------------------------------------------------------------
bool openFile(HWND owner, LPTSTR file)
{
  DWORD lastIx = 0;
  infoOpenSave Info(filterExtSVX, filterSVX, infoOpenSave::OPEN_F, lastIx, file);
  myOpenSave open(owner);

  bool success = open.run(Info);

  if(success) {
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
//------------------------------------------------------------------
bool openSaveFile(HWND owner, LPTSTR file)
{
  DWORD lastIx = 0;
  infoOpenSave Info(filterExtSVX, filterSVX, infoOpenSave::SAVE_AS, lastIx, file);
  myOpenSave open(owner);

  bool success = open.run(Info);

  if(success) {
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
//------------------------------------------------------------------
static LPCTSTR Marker[] = {
    _T("VARS>"), // zona delle variabili
    _T("FUNCT>"), // zona delle funzioni
    _T("CALC>"), // zona del calcolo
    _T("INIT>"), // zona delle inizializzazioni delle variabili
    };
//------------------------------------------------------------------
//extern int get_CRYPT_STEP();
//extern LPCSTR Header;
//extern LPCSTR Key;
//------------------------------------------------------------------
#define RESULT_TMP _T("~tmp~.~x~")
//---------------------------------------------------------------------
static bool openClear(LPCTSTR file, infoFileCr& result)
{
  P_File pf(file, P_READ_ONLY);
  if (!pf.P_open())
    return false;
  result.dim = (int)pf.get_len();
  if (!result.dim)
    return false;

  result.buff = new BYTE[result.dim];
  pf.P_read(result.buff, result.dim);
  return true;
}
//------------------------------------------------------------------
LPTSTR openAndDecrypt(LPCTSTR file, uint& dim)
{
  infoFileCr result;
  if (!openClear(file, result))
    return 0;
  LPBYTE t = new BYTE[result.dim + 2];
  CopyMemory(t, result.buff, result.dim);
  t[result.dim] = 0;
  t[result.dim + 1] = 0;
  delete[]result.buff;
  dim = result.dim + 2;
  LPTSTR text = autoConvert(t, dim);
  if (!text) {
    delete[]t;
    return 0;
  }
  return text;
}
//------------------------------------------------------------------
void test_Expr::findAndSetblock(LPTSTR buff, LPCTSTR marker, exp_BaseContainer* body)
{
  TCHAR mark[32] = _T("<");
  _tcscat_s(mark, marker);
  LPTSTR p = StrStrI(buff, mark);
  if(p) {
    p += _tcslen(mark);
    while(*p && (unsigned)*p <= _T(' '))
      ++p;
    mark[1] = _T('/');
    mark[2] = 0;
    _tcscat_s(mark, marker);
    LPTSTR p2 = StrStrI(p, mark);
    if(p2) {
      --p2;
      while(*p2 && (unsigned)*p2 <= _T(' ') && p2 > p)
        --p2;
      ++p2;
      if(p2 - p > 5) {
        TCHAR t[3];
        for(uint i = 0; i < 3; ++i)
          t[i] = p2[i];
        p2[0] = _T('\r');
        p2[1] = _T('\n');
        p2[2] = 0;
        body->setText(p);
        body->colorize();
        body->resetDirty();
        for(uint i = 0; i < 3; ++i)
          p2[i] = t[i];
        }
      else {
        body->setText(_T(""));
        body->colorize();
        body->resetDirty();
        }
      }
    }
}
//----------------------------------------------------------------------------
void test_Expr::loadPart(LPCTSTR file)
{
  uint len = 0;
  LPTSTR buff = openAndDecrypt(file, len);
  if(buff) {
    bodyCalc->setText(_T(""));
    bodyFunct->setText(_T(""));
    bodyInit->setText(_T(""));
    bodyVars->setText(_T(""));
    bodyTest->setText(_T(""));
    findAndSetblock(buff, Marker[0], bodyVars);
    findAndSetblock(buff, Marker[1], bodyFunct);
    findAndSetblock(buff, Marker[2], bodyCalc);
    findAndSetblock(buff, Marker[3], bodyInit);
    }
}
//---------------------------------------------------------------------
bool test_Expr::checkSave(bool reset)
{
  exp_BaseContainer* childs[] =  {
     bodyInit,
     bodyVars,
     bodyFunct,
     bodyCalc,
    };
  const uint mbStyle = MB_ICONSTOP | MB_YESNOCANCEL;
  for(uint i = 0; i < SIZE_A(childs); ++i) {
    if(childs[i]->isDirty()) {
      switch(MessageBox(*this, _T("Il file è stato modificato\r\nVuoi salvarlo?"), _T("Attenzione"), mbStyle)) {
        case IDYES:
          if(!save(true))
            return false;
        case IDNO:
          if(reset)
            resetDirty();
          return true;
        default:
          return false;
        }
      }
    }
  return true;
}
//---------------------------------------------------------------------
void test_Expr::setTitle(LPCTSTR tit)
{
  TCHAR t[_MAX_PATH + 64];
  wsprintf(t, _T("Express_Script Maker - %s"), tit);
  setCaption(t);
}
//---------------------------------------------------------------------
void test_Expr::getFile()
{
  if(!checkSave())
    return;

  TCHAR svx[_MAX_PATH];
  topBtn->getText(svx, SIZE_A(svx));
  if(openFile(*this, svx)) {
    topBtn->setText(svx);
    setTitle(svx);
    loadPart(svx);
    checkReadOnly();
    }
}
//---------------------------------------------------------------------
void test_Expr::setup()
{
  exp_BaseContainer* childs[] =  {
     bodyVars,
     bodyInit,
     bodyFunct,
     bodyCalc,
     bodyTest,
     bodyResult,
    };
  infoEdit ed[MAX_BODY];
  for(uint i = 0; i < MAX_BODY; ++i)
    ed[i] = childs[i]->getInfo();

  if(IDOK == PDialogColor(this, IDD_SETUP, ed).modal()) {
    for(uint i = 0; i < MAX_BODY; ++i) {
      infoEdit edOld;
      if(edOld != childs[i]->getInfo()) {
        bool dirty = childs[i]->isDirty();
        childs[i]->setInfo(ed[i]);
        childs[i]->colorize();
        if(!dirty)
          childs[i]->resetDirty();
        }
      }
    }
}
//---------------------------------------------------------------------
/*
static void saveCrypt(LPCTSTR target, LPCTSTR source)
{
  infoFileCrCopy ifcc = { (LPCBYTE)Key, strlen(Key), get_CRYPT_STEP(), (LPCBYTE)Header, strlen(Header) };
  cryptFileCopy(target, source, ifcc);
  DeleteFile(source);
}
*/
//----------------------------------------------------------------------------
void test_Expr::resetDirty()
{
  exp_BaseContainer* childs[] =  {
     bodyInit,
     bodyVars,
     bodyFunct,
     bodyCalc,
    };
  for(uint i = 0; i < SIZE_A(childs); ++i)
    childs[i]->resetDirty();
}
//----------------------------------------------------------------------------
bool test_Expr::save(bool dialog)
{
  TCHAR file[_MAX_PATH];
  topBtn->getText(file, SIZE_A(file));
  bool success = true;
  if(dialog || !*file)
    success = openSaveFile(*this, file);
  if(success) {
    do {
      P_File pf(file);
      pf.appendback();
      } while (false);
    if (saveFile(file)) {
      topBtn->setText(file);
      resetDirty();
      setTitle(file);
      return true;
      }
      /*
    if(saveFile(RESULT_TMP)) {
      saveCrypt(file, RESULT_TMP);
      topBtn->setText(file);
      resetDirty();
      setTitle(file);
      return true;
      }
*/
    }
  return false;
}
//----------------------------------------------------------------------------
bool test_Expr::saveFile(LPCTSTR file)
{
  uint dim = 0;
  LPTSTR buff = bodyVars->getText(dim);
  if(!*buff) {
    delete []buff;
    buff = topBtn->getText(dim);
    loadPart(buff);
    delete []buff;
    buff = bodyVars->getText(dim);
    if(!*buff) {
      delete []buff;
      return false;
      }
    }

  P_File pf(file, P_CREAT);
  if(!pf.P_open()) {
    delete []buff;
    return false;
    }

  pf.P_writeString(_T("<VARS>\r\n"));
  pf.P_writeString(buff);
  pf.P_writeString(_T("\r\n</VARS>\r\n"));
  delete []buff;
  pf.P_writeString(_T("<INIT>\r\n"));
  buff = bodyInit->getText(dim);
  if(dim)
    pf.P_writeString(buff);
  pf.P_writeString(_T("\r\n</INIT>\r\n"));
  delete []buff;

  pf.P_writeString(_T("<FUNCT>\r\n"));
  buff = bodyFunct->getText(dim);
  if(dim)
    pf.P_writeString(buff);
  pf.P_writeString(_T("\r\n</FUNCT>\r\n"));

  pf.P_writeString(_T("<CALC>\r\n"));
  buff = bodyCalc->getText(dim);
  if(dim)
    pf.P_writeString(buff);
  pf.P_writeString(_T("\r\n</CALC>\r\n"));
  delete []buff;
  return true;
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
static LPTSTR get_line(LPTSTR buff, size_t len)
{
  if ((int)len < 0)
    return 0;
  size_t i;
  LPTSTR p = buff;
  for (i = 0; i < len; ++i, ++p)
    if (U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if (i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if (++i == len)
    return 0;

  for (; i < len; ++i, ++p)
    if (U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
void checkOffs(LPTSTR p, ParserInfo& pi)
{
  LPTSTR p2 = p;
  while(*p2 && *p2 != _T('['))
    ++p2;
  if(*p2) {
    *p2++ = 0;
    pi.offset = _ttoi(p2);
    }
  pi.varName = p;
}
//----------------------------------------------------------------------------
void fillVar(LPTSTR p, ParserInfo& pi)
{
  pi.varName = 0;
  LPTSTR p2 = p;
  while(*p2 && *p2 != _T('='))
    ++p2;
  if(!*p2)
    return;
  *p2 = 0;
  if(_T('$') == *p)
    ++p;
  checkOffs(p, pi);
  p = ++p2;
  bool isReal = false;
  while(*p2 && *p2 > _T(' ')) {
    if(_T('.') == *p2)
      isReal = true;
    ++p2;
    }
  if (!*p2 && p == p2)
    return;
  *p2 = 0;
  if (isReal) {
    pi.type = itv_real;
    pi.d = _ttof(p);
    }
  else {
    pi.type = itv_Int;
    pi.i = _ttoi64(p);
    }
}
//----------------------------------------------------------------------------
void test_Expr::check_init(exp_BaseContainer* body, testManVars& tmv)
{
  uint dim = 0;
  LPTSTR buff = body->getText(dim);
  if (dim) {
    ParserInfo pi;
    uint offs = 0;
    LPTSTR p = buff;
    do {
      LPTSTR next = get_line(p, dim - offs);
      size_t len = next ? next - p : dim - offs;
      offs += len;
      ZeroMemory(&pi, sizeof(pi));
      fillVar(p, pi);
      if (!pi.varName)
        break;
      tmv.setValue(pi);
      p = next;
      } while (p);
    }
  delete []buff;
}
//----------------------------------------------------------------------------
  void test_Expr::run()
  {
    checkSave(false);
  bodyResult->setText(_T(""));
  saveFile(TEST_TMP_FILE);
  P_ExprMake mk;
  mk.make(TEST_TMP_FILE, false);
  testManVars test(mk, this);
  if(test.parse()) {
    check_init(bodyInit, test);
    check_init(bodyTest, test);
    test.perform();
    test.sendAllVars();
    bodyResult->colorize();
    }
/*
  svManExpression me;
  if(me.makeExpression(TEST_TMP_FILE)) {
    me.performExpression();
    bodyResult->colorize();
    }
*/
  DeleteFile(TEST_TMP_FILE);
}
//----------------------------------------------------------------------------
bool test_Expr::findFunct(LPCTSTR functName, CHARRANGE& cr)
{
  return bodyFunct->find(0, functName, FR_DOWN | FR_MATCHCASE | FR_WHOLEWORD, cr);
}
//----------------------------------------------------------------------------
