//---------- mainWin.cpp -----------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "mainwin.h"
#include "PModDialog.h"
#include "P_ModListFiles.h"
#include "pMenuBtn.h"
#include "pSplitwin.h"
#include "popensave.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "p_manage_ini.h"
#include "macro_utils.h"
//----------------------------------------------------------------------------
#define SUB_BASE_KEY _T("Software\\NPS_Programs\\svTrendEditor")
#define REL_SYSTEM_PATH _T("LastPath")
#define FILL_INFO _T("fillInfo")
//----------------------------------------------------------------------------
#define COORDS_PATH _T("coords")
#define SIZE_PATH   _T("size")
//----------------------------------------------------------------------------
#define INI_FILENAME _T("svTrendEditor.ini")
#define MAIN_PATH _T("\\Main")
//----------------------------------------------------------------------------
class myManageIni : public manageIni
{
  private:
    typedef manageIni baseClass;
  public:
    myManageIni(LPCTSTR filename);
};
//----------------------------------------------------------------------------
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
bool setKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPCTSTR path)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  ini_param param = { keyName, path };
  ini.addParam(param, keyBlock);
  return ini.save();
}
//----------------------------------------------------------------------------
bool getKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPTSTR path)
{
  path[0] = 0;
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str) {
    _tcscpy_s(path, _MAX_PATH, str);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool setKeyParam(LPCTSTR keyName, LPCTSTR keyBlock, DWORD value)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, keyBlock);
  return ini.save();
}
//----------------------------------------------------------------------------
bool getKeyParam(LPCTSTR keyName, LPCTSTR keyBlock, DWORD& value)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    value = (DWORD)t;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void setKeyParam(LPCTSTR keyName, DWORD value)
{
  setKeyParam(keyName, MAIN_PATH, value);
}
//----------------------------------------------------------------------------
void getKeyParam(LPCTSTR keyName, LPDWORD value)
{
  getKeyParam(keyName, MAIN_PATH, *value);
}
//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR path)
{
  setKeyPath(keyName, MAIN_PATH, path);
}
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPTSTR path)
{
  path[0] = 0;
  getKeyPath(keyName, MAIN_PATH, path);
}
//----------------------------------------------------------------------------
void setPath(LPCTSTR path)
{
  setKeyPath(REL_SYSTEM_PATH, path);
}
//----------------------------------------------------------------------------
void getPath(LPTSTR path)
{
  getKeyPath(REL_SYSTEM_PATH, path);
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
bool needMaximized()
{
  DWORD coords = 0;
  getKeyParam(COORDS_PATH, &coords);
  return (DWORD)-1 == coords;
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
struct idc_id
{
  uint idObj;
  uint idText;
};
//----------------------------------------------------------------------------
static setOfString gSet;
//----------------------------------------------------------------------------
LPCTSTR getStringOrDef(uint code, LPCTSTR def)
{
  LPCTSTR p = gSet.getString(code);
  return p ? p : def;
}
//-------------------------------------------------------------------
void setLang(HWND hwnd, uint idText)
{
  LPCTSTR txt = gSet.getString(idText);
  if(txt)
    SetWindowText(hwnd, txt);
}
//-------------------------------------------------------------------
void setLang(HWND hwnd, uint idc, uint idText)
{
  LPCTSTR txt = gSet.getString(idText);
  if(txt)
    SetDlgItemText(hwnd, idc, txt);
}
//--------------------------------------------------------------------
mainWin::mainWin(HINSTANCE hInstance) :
      PMainWin(_T("NpSvTrendEditor"), hInstance), mainSplit(0)
{
  GetModuleDirName(SIZE_A(CurrPath), CurrPath);
  appendPath(CurrPath, _T("npsvTrendEditor.lng"));
  gSet.add(CurrPath);
  ZeroMemory(CurrPath, sizeof(CurrPath));
  getPath(CurrPath);
  if(*CurrPath)
    SetCurrentDirectory(CurrPath);
}
//----------------------------------------------------------------------------
mainWin::~mainWin()
{
  destroy();
  delete mainSplit;
}
//------------------------------------------------------------------------------
P_ModListFiles* getListFile(PWin* child)
{
  mainWin* mv = getParentWin<mainWin>(child);
  return mv->getListFile();
}
//-----------------------------------------------------------
#define IDD_MENU_BTN 1
#define IDD_MAIN_SPLIT 2
#define IDD_CLIENT_SPLIT 3
#define IDD_CODE 4
#define IDD_FILE 5
//---------------------------------------------------------
#define MIN_WIDTH  500
#define MIN_HEIGHT 530
//------------------------------------------------------------------------------
bool mainWin::create()
{
  menuBtn = new pMenuBtn(this, IDD_MENU_BTN);
  clientFile = new P_ModListFiles(this, IDD_FILE);

  mainSplit = new PSplitter(this, menuBtn, clientFile);

  mainSplit->setDim1(OFFS_BTN * 2 + SIZE_BTN_H, false);
  mainSplit->setLock(PSplitter::PSL_FIRST);
  mainSplit->setDrawStyle(PSplitter::PSD_LEFT);

  DWORD coords = 0;
  getKeyParam(COORDS_PATH, &coords);
  if(coords && (DWORD)-1 != coords) {
    Attr.x = (short)LOWORD(coords);
    Attr.y = (short)HIWORD(coords);
    checkOut(Attr.x, Attr.y);
    getKeyParam(SIZE_PATH, &coords);
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

  bool success = mainSplit->create();

  return success;
}
//----------------------------------------------------------------------------
bool mainWin::preProcessMsg(MSG& msg)
{
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
// #define getStringOrDef(n,t) t
//----------------------------------------------------------------------------
#define SAVE_QUERY_MSG getStringOrDef(IDT_SAVE_QUERY, _T("Il file è stato modificato\r\nVuoi salvare prima di proseguire?"))
#define SAVE_QUERY_TIT getStringOrDef(IDT_GENERIC_WARNING, _T("Attenzione"))
#define SAVE_QUERY_FLAG (MB_YESNOCANCEL | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
#define UPD_QUERY_MSG getStringOrDef(IDT_UPD_QUERY, _T("Il file è stato modificato\r\nProseguendo perderai tutte le modifiche\r\nVuoi proseguire?"))
#define UPD_QUERY_FLAG (MB_YESNO | MB_ICONINFORMATION)
//-----------------------------------------------------------
LRESULT mainWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  mainSplit->windowProcSplit(hwnd, message, wParam, lParam);
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
      break;
    case WM_GETMINMAXINFO:
      do {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
        lpmmi->ptMinTrackSize.x = MIN_WIDTH;
        lpmmi->ptMinTrackSize.y = MIN_HEIGHT;
        } while(false);
      break;
    case WM_CUSTOM_BY_BTN:
      switch(LOWORD(wParam)) {
        case CM_BTN_DONE:
          if(clientFile->isDirty()) {
            switch(MessageBox(*this, SAVE_QUERY_MSG, SAVE_QUERY_TIT, SAVE_QUERY_FLAG)) {
              case IDYES:
                save();
              case IDNO:
                break;
              case IDCANCEL:
                return baseClass::windowProc(hwnd, message, wParam, lParam);
              }
            }
          PostQuitMessage(0);
          break;
        case CM_BTN_FOLDER:
          setFolder();
          break;
        case CM_BTN_SAVE:
          save();
          break;
        case CM_BTN_EXPORT:
          menuExport();
          break;
/*
        case CM_BTN_IMPORT:
          menuImport();
          break;
*/
        case ICD_CHECK_DATE:
          clientFile->setModifyDate(toBool(lParam));
          break;
        case CM_BTN_ADD:
          menuAdd();
          break;
        case CM_BTN_REM:
          menuRem();
          break;
        case CM_BTN_COPY:
          copy();
          break;
        case CM_BTN_PASTE:
          paste();
          break;
        case CM_BTN_UPD:
          if(clientFile->isDirty()) {
            switch(MessageBox(*this, UPD_QUERY_MSG, SAVE_QUERY_TIT, UPD_QUERY_FLAG)) {
              case IDYES:
                fill();
                break;
              }
            }
          break;
        }
      break;
    case WM_COMMAND:
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void mainWin::setPathAndTitle(LPCTSTR path)
{
  TCHAR oldPath[_MAX_PATH];
  _tcscpy_s(oldPath, path);
  dirName(oldPath);
  SetCurrentDirectory(oldPath);
  setPath(oldPath);
  _tcscpy_s(CurrPath, path);
  _tcscpy_s(oldPath, _T("svTrendEditor - "));
  _tcscat_s(oldPath, CurrPath);
  setCaption(oldPath);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR filterExtTrend[] = { _T(".trd"), _T(".*"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterTrend =
  _T("%s (trd)\0*.trd\0")
  _T("%s (*.*)\0*.*\0");
//----------------------------------------------------------------------------
static DWORD lastIx;
//----------------------------------------------------------------------------
static void makeFilterTrend(LPTSTR flt, size_t dim)
{
  _tcscpy_s(flt, dim, getStringOrDef(IDT_FILE_TRD, _T("Trend File")));
  _tcscat_s(flt, dim, _T(" (trd)"));
  LPTSTR p = flt + _tcslen(flt) + 1;
  _tcscpy_s(p, dim - (p - flt), _T("*.trd"));
  p += _tcslen(p) + 1;
  _tcscpy_s(p, dim - (p - flt), getStringOrDef(IDT_FILE_ALL, _T("Tutti i File")));
  p += _tcslen(p);
  _tcscpy_s(p, dim - (p - flt), _T(" (*.*)"));
  p += _tcslen(p) + 1;
  _tcscpy_s(p, dim - (p - flt), _T("*.*"));
  p += _tcslen(p) + 1;
  *p = 0;
}
//----------------------------------------------------------------------------
bool openFileTrend(HWND owner, LPTSTR file, LPCTSTR CurrPath)
{
  TCHAR flt[_MAX_PATH];
  makeFilterTrend(flt, SIZE_A(flt));

  infoOpenSave Info(filterExtTrend, flt, infoOpenSave::OPEN_F, lastIx, file);
  POpenSave open(owner);

  SetCurrentDirectory(CurrPath);
  if(open.run(Info)) {
    lastIx = Info.ixFilter;
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool saveFileTrend(HWND owner, LPTSTR file, LPCTSTR CurrPath)
{
  TCHAR flt[_MAX_PATH];
  makeFilterTrend(flt, SIZE_A(flt));

  infoOpenSave Info(filterExtTrend, flt, infoOpenSave::SAVE_AS, lastIx, file);
  POpenSave save(owner);

  SetCurrentDirectory(CurrPath);

  if(save.run(Info)) {
    lastIx = Info.ixFilter;
    _tcscpy_s(file, _MAX_PATH, save.getFile());
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void mainWin::setFolder()
{
  if(clientFile->isDirty()) {
    switch(MessageBox(*this, SAVE_QUERY_MSG, SAVE_QUERY_TIT, SAVE_QUERY_FLAG)) {
      case IDYES:
        save();
      case IDNO:
        break;
      case IDCANCEL:
        return;
      }
    }
  TCHAR oldPath[_MAX_PATH];
  getPath(oldPath);

  TCHAR file[_MAX_PATH] = _T("");
  if(openFileTrend(*this, file, oldPath)) {
    setPathAndTitle(file);
    fill();
    }
}
//----------------------------------------------------------------------------
void mainWin::save()
{
  TCHAR oldPath[_MAX_PATH];
  _tcscpy_s(oldPath, CurrPath);

  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, CurrPath);
  if(saveFileTrend(*this, file, dirName(oldPath))) {
    if(!_tcsicmp(file, CurrPath) && !clientFile->isDirty()) {
      MessageBox(*this, getStringOrDef(IDT_SAVE_NO_MOD, _T("Nessuna modifica apportata")),
          getStringOrDef(IDT_SAVE_NO_MOD_TIT, _T("Inutile salvare")), MB_OK | MB_ICONINFORMATION);
      return;
      }
    if(!clientFile->save(file))
      MessageBox(*this, getStringOrDef(IDT_SAVE_ERR, _T("Errore nel salvataggio!")),
          getStringOrDef(IDT_GENERIC_WARNING, _T("Attenzione")), MB_OK | MB_ICONSTOP);
    else
      setPathAndTitle(file);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class pasteDialog : public PModDialog
{
    typedef PModDialog baseClass;
  public:
    pasteDialog(mainWin* parent, pasteInfo& pi) :
        baseClass(parent, IDD_DIALOG_PASTE), pInfo(pi) {}
    ~pasteDialog() { destroy(); }

    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    pasteInfo& pInfo;
    void enableEdit();
};
//----------------------------------------------------------------------------
bool pasteDialog::create()
{
  if(!baseClass::create())
    return false;

  setLang(*this, IDT_DLG_PASTE);

  idc_id idc[] = {
     { IDOK, IDT_OK },
     { IDCANCEL, IDT_CANC },
     { IDC_RADIO_AFTER_, IDT_PASTE_AFTER_W },
     { IDC_RADIO_FIRST, IDT_PASTE_FIRST },
     { IDC_RADIO_LAST, IDT_PASTE_LAST },
     { IDC_RADIO_BY_ROWS, IDT_PASTE_BY_ROW },
     { IDC_RADIO_BY_COL, IDT_PASTE_BY_COL },
     { IDC_CHECK_OVERWRITE, IDT_PASTE_OVERWRITE },
     { IDC_STATIC_PASTE_AFTER, IDT_PASTE_AFTER },
  };

  for(uint i = 0; i < SIZE_A(idc); ++i)
    setLang(*this, idc[i].idObj, idc[i].idText);


  SET_INT(IDC_EDIT_AFTER, pInfo.after);
  SET_INT(IDC_EDIT_AFTER_ROW, pInfo.fromRow);
  if(pInfo.type & 1)
    SET_CHECK(IDC_RADIO_BY_ROWS);
  else
    SET_CHECK(IDC_RADIO_BY_COL);
  if(pInfo.type & 2)
    SET_CHECK(IDC_CHECK_OVERWRITE);

  if(pInfo.type & 4)
    SET_CHECK(IDC_RADIO_AFTER_);
  else if(pInfo.type & 8)
    SET_CHECK(IDC_RADIO_LAST);
  else
    SET_CHECK(IDC_RADIO_FIRST);
  enableEdit();
  return true;
}
//-----------------------------------------------------------
LRESULT pasteDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIO_FIRST:
        case IDC_RADIO_LAST:
        case IDC_RADIO_AFTER_:
        case IDC_RADIO_BY_COL:
        case IDC_RADIO_BY_ROWS:
        case IDC_EDIT_AFTER_ROW:
          enableEdit();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void pasteDialog::enableEdit()
{
  bool enable = IS_CHECKED(IDC_RADIO_AFTER_);
  ENABLE(IDC_EDIT_AFTER, enable);
  if(IS_CHECKED(IDC_RADIO_BY_COL)) {
    ENABLE(IDC_EDIT_AFTER_ROW, true);
    int v;
    GET_INT(IDC_EDIT_AFTER_ROW, v);
    if(v) {
      SET_CHECK(IDC_CHECK_OVERWRITE);
      ENABLE(IDC_CHECK_OVERWRITE, false);
      if(IS_CHECKED(IDC_RADIO_LAST))
        SET_CHECK_UNCHECK(IDC_RADIO_FIRST, IDC_RADIO_LAST);
      ENABLE(IDC_RADIO_LAST, false);
      }
    else {
      ENABLE(IDC_CHECK_OVERWRITE, true);
      ENABLE(IDC_RADIO_LAST, true);
      }
    }
  else
    ENABLE(IDC_EDIT_AFTER_ROW, false);
}
//----------------------------------------------------------------------------
void pasteDialog::CmOk()
{
  GET_INT(IDC_EDIT_AFTER, pInfo.after);
  GET_INT(IDC_EDIT_AFTER_ROW, pInfo.fromRow);
  pInfo.type = 0;
  if(IS_CHECKED(IDC_RADIO_AFTER_))
    pInfo.type = 4;
  else if(IS_CHECKED(IDC_RADIO_LAST))
    pInfo.type = 8;

  if(IS_CHECKED(IDC_RADIO_BY_ROWS))
    pInfo.type |= 1;

  if(IS_CHECKED(IDC_CHECK_OVERWRITE))
    pInfo.type |= 2;

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
static int TickTimer = 300;
//----------------------------------------------------------------------------
class infoDialog : public PModDialog
{
    typedef PModDialog baseClass;
  public:
    infoDialog(mainWin* parent, fillInfo& fi) :
        baseClass(parent, IDD_DIALOG_CHECK_N_COL), fInfo(fi) {}
    ~infoDialog() { destroy(); }

    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    fillInfo& fInfo;
    void testAutoCheck();
};
//----------------------------------------------------------------------------
bool infoDialog::create()
{
  if(!baseClass::create())
    return false;

  setLang(*this, IDT_DLG_COL);

  idc_id idc[] = {
     { IDOK, IDT_OK },
     { IDCANCEL, IDT_CANC },
     { IDC_STATIC_NCOL, IDT_NUM_COL },
     { IDC_BUTTON_AUTOCHECK, IDT_AUTO_CHECK },
     { IDC_STATIC_TIMER, IDT_TIMER },
  };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    setLang(*this, idc[i].idObj, idc[i].idText);

  SET_INT(IDC_EDIT_NUM_COL, fInfo.nCol);
  SET_INT(IDC_EDIT_TIMER, TickTimer);
  testAutoCheck();

  return true;
}
//-----------------------------------------------------------
LRESULT infoDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_AUTOCHECK:
          testAutoCheck();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
#define BASE_TICK 500
#define MAX_COL 100
#define MAX_TICK (1000 * 60 * 10)
#define INCR_TICK(t) (t / 2)
//----------------------------------------------------------------------------
static bool isValidDate(const FILETIME& ft)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  if(st.wYear > 2100 || st.wYear < 1900)
    return false;
  return true;
}
//----------------------------------------------------------------------------
static bool testAutoCheck(P_File& pf, int tick2, int& ncol)
{
  __int64 tick = tick2 ? tick2 : BASE_TICK;
  FILETIME ft;
  pf.P_read(&ft, sizeof(ft));
  ncol = 0;
  if(!isValidDate(ft))
    return false;
  union {
    FILETIME ft;
    DWORD dw[2];
    } U;
  tick *= SECOND_TO_I64 / 1000;
  pf.P_read(&U.ft, sizeof(U.ft));
  for(uint i = 0; i < MAX_COL; ++i) {
    __int64 diff = cMK_I64(U.ft) - cMK_I64(ft);
    if(diff > 0 && diff < tick) {
      ncol = i;
      break;
      }
    U.dw[0] = U.dw[1];
    pf.P_read(&U.dw[1], sizeof(U.dw[1]));
    }
  return ncol > 0;
}
//----------------------------------------------------------------------------
static bool testAutoCheck(LPCTSTR file, int& tick2, int& ncol)
{
  P_File pf(file, P_READ_ONLY);
  if(!pf.P_open())
    return false;
  int max_tick = tick2 < MAX_TICK ? MAX_TICK : tick2 *3;
  for(int tick = tick2 > 1 ? tick2 : BASE_TICK; tick < max_tick; tick += INCR_TICK(tick)) {
    pf.P_seek(0);
    if(testAutoCheck(pf, tick, ncol)) {
      tick2 = tick;
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
void infoDialog::testAutoCheck()
{
  int tick = BASE_TICK;
  do {
    int tick2 = 0;
    GET_INT(IDC_EDIT_TIMER, tick2);
    if(tick2)
      tick = tick2;
    } while(false);
  int ncol = 0;
  if(::testAutoCheck(fInfo.file, tick, ncol)) {
    SET_INT(IDC_EDIT_NUM_COL, ncol);
    SET_INT(IDC_EDIT_TIMER, tick);
    }
  else
    MessageBox(*this, getStringOrDef(IDT_ERR_NO_NUM, _T("Impossibile determinare il numero di colonne")),
        getStringOrDef(IDT_ERROR, _T("Errore")), MB_OK | MB_ICONSTOP);
}
//----------------------------------------------------------------------------
void infoDialog::CmOk()
{
  int ncol;
  GET_INT(IDC_EDIT_NUM_COL, ncol);
  if(ncol <= 0)
    return;
  GET_INT(IDC_EDIT_TIMER, TickTimer);
  fInfo.nCol = ncol;
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void mainWin::fill()
{
  fillInfo fInfo;
  _tcscpy_s(fInfo.file, CurrPath);
  if(IDOK == infoDialog(this, fInfo).modal())
    clientFile->fill(fInfo);
}
//----------------------------------------------------------------------------
class numColDialog : public PModDialog
{
    typedef PModDialog baseClass;
  public:
    numColDialog(mainWin* parent, int& pos, bool add) :
        baseClass(parent, IDD_DIALOG_ADD_COLUMN), Pos(pos), Add(add) {}
    ~numColDialog() { destroy(); }

    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void enableEdit();
    void CmOk();
    int& Pos;
    bool Add;
};
//----------------------------------------------------------------------------
bool numColDialog::create()
{
  if(!baseClass::create())
    return false;

  idc_id idc[] = {
     { IDOK, IDT_OK },
     { IDCANCEL, IDT_CANC },
     { IDC_RADIO_AFTER_, IDT_C_AFTER },
     { IDC_RADIO_FIRST, IDT_C_FIRST },
     { IDC_RADIO_LAST, IDT_C_LAST },
    };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    setLang(*this, idc[i].idObj, idc[i].idText);


  SET_INT(IDC_EDIT_AFTER, Pos);
  if(!Pos)
    SET_CHECK(IDC_RADIO_FIRST);
  else if(Pos < 0)
    SET_CHECK(IDC_RADIO_LAST);
  else
    SET_CHECK(IDC_RADIO_AFTER_);

  enableEdit();

  if(Add)
    setCaption(getStringOrDef(IDT_C_DLG_ADD, _T("Aggiungi colonna")));
  else
    setCaption(getStringOrDef(IDT_C_DLG_REM, _T("Rimuovi colonna")));

  return true;
}
//----------------------------------------------------------------------------
void numColDialog::enableEdit()
{
  bool enable = IS_CHECKED(IDC_RADIO_AFTER_);
  ENABLE(IDC_EDIT_AFTER, enable);
}
//-----------------------------------------------------------
LRESULT numColDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIO_FIRST:
        case IDC_RADIO_LAST:
        case IDC_RADIO_AFTER_:
          enableEdit();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void numColDialog::CmOk()
{
  GET_INT(IDC_EDIT_AFTER, Pos);
  if(IS_CHECKED(IDC_RADIO_FIRST))
    Pos = 0;
  else if(IS_CHECKED(IDC_RADIO_LAST))
    Pos = -1;
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
struct menuInfo
{
  LPCTSTR item;
  bool grayed;
};
//----------------------------------------------------------------------------
static int getResultMenu(PWin* owner, const menuInfo* mInfo, uint nElem)
{
  HMENU hmenu = CreatePopupMenu();
  if(!hmenu)
    return -1;
  const int firstId = 10001;
  for(uint i = 0; i < nElem; ++i)
    AppendMenu(hmenu, MF_STRING | (mInfo[i].grayed ? MF_GRAYED : 0), firstId + i, mInfo[i].item);

  UINT flag = TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN;
  POINT Pt;
  GetCursorPos(&Pt);
  int result = TrackPopupMenu(hmenu, flag, Pt.x, Pt.y, 0, *owner, 0);
  DestroyMenu(hmenu);
  result -= firstId;
  if(result < 0 || (uint)result >= nElem)
    return -1;
  return result;
}
//----------------------------------------------------------------------------
static void setWaitCursor(PWin* w)
{
  SetCursor(LoadCursor(0, IDC_WAIT));
  SetCapture(*w);
  EnableWindow(*w, false);
  getAppl()->pumpMessages();
}
//----------------------------------------------------------------------------
static void restoreCursor(PWin* w)
{
  ReleaseCapture();
  SetCursor(LoadCursor(0, IDC_ARROW));
  EnableWindow(*w, true);
}
//----------------------------------------------------------------------------
void mainWin::menuAdd()
{
  static menuInfo mi[] = {
      {  getStringOrDef(IDT_M_ADD_ROW, _T("Aggiungi riga")), false },
      {  getStringOrDef(IDT_M_ADD_COL, _T("Aggiungi colonna")), false },
      };
  uint nItem = SIZE_A(mi);
  uint res = getResultMenu(this, mi, nItem);
  switch(res) {
    case 0:
      addRow();
      break;
    case 1:
      addColumn();
      break;
    }
}
//----------------------------------------------------------------------------
void mainWin::menuRem()
{
  menuInfo mi[] = {
      { getStringOrDef(IDT_M_REM_ROW, _T("Rimuovi righe")), false },
      {  getStringOrDef(IDT_M_REM_COL, _T("Rimuovi colonna")), false },
      };
  mi[1].grayed = clientFile->getCurrNumCol() < 2;

  uint nItem = SIZE_A(mi);
  uint res = getResultMenu(this, mi, nItem);
  switch(res) {
    case 0:
      remRow();
      break;
    case 1:
      remColumn();
      break;
    }
}
//----------------------------------------------------------------------------
void mainWin::addRow()
{
  if(clientFile->getCurrNumCol() <= 0)
    return;
  menuInfo mi[] = {
      {  getStringOrDef(IDT_M_ADD_ROW_CURR, _T("Posizione corrente")), false },
      {  getStringOrDef(IDT_M_ADD_ROW_FIRST, _T("Prima riga")), false },
      {  getStringOrDef(IDT_M_ADD_ROW_APPEND, _T("Aggiungi in fondo")), false },
      };
  uint nItem = SIZE_A(mi);
  uint res = getResultMenu(this, mi, nItem);
  if(res < SIZE_A(mi)) {
    setWaitCursor(this);
    clientFile->addRow(res);
    restoreCursor(this);
    }
}
//----------------------------------------------------------------------------
void mainWin::remRow()
{
  if(clientFile->getCurrNumCol() <= 0)
    return;

  if(IDYES != MessageBox(*this, getStringOrDef(IDT_REM_ROW_REQ, _T("Vuoi eliminare le righe selezionate?")),
        getStringOrDef(IDT_REM_ROW_REQ, _T("Conferma eliminazione")), MB_YESNO | MB_ICONSTOP))
    return;
  setWaitCursor(this);
  clientFile->remRow();
  restoreCursor(this);
}
//----------------------------------------------------------------------------
void mainWin::addColumn()
{
  if(clientFile->getCurrNumCol() <= 0)
    return;
  static int col = 1;
  if(IDOK == numColDialog(this, col, true).modal()) {
    setWaitCursor(this);
    if(col < 0)
      col = clientFile->getCurrNumCol();
    else if(col > clientFile->getCurrNumCol())
      col = clientFile->getCurrNumCol();
    clientFile->addColumn(col);
    restoreCursor(this);
    }
}
//----------------------------------------------------------------------------
void mainWin::remColumn()
{
  if(clientFile->getCurrNumCol() < 2)
    return;
  static int col = 1;
  if(IDOK == numColDialog(this, col, false).modal()) {
    setWaitCursor(this);
    if(col < 0)
      col = clientFile->getCurrNumCol() - 1;
    else if(col >= clientFile->getCurrNumCol())
      col = clientFile->getCurrNumCol() - 1;
    clientFile->remColumn(col);
    restoreCursor(this);
    }
}
//----------------------------------------------------------------------------
void mainWin::copy()
{
  clientFile->copyCurrent();
}
//----------------------------------------------------------------------------
void mainWin::paste()
{
  static pasteInfo pi;
  if(IDOK == pasteDialog(this, pi).modal())
    clientFile->paste(pi);
}
//----------------------------------------------------------------------------
static void addString(HWND hList, LPCTSTR str, bool isLB = true)
{
  int wid = extent(hList, str);
  int msgGet;
  int msgSet;
  if(isLB) {
    msgGet = LB_GETHORIZONTALEXTENT;
    msgSet = LB_SETHORIZONTALEXTENT;
    }
  else  {
    msgGet = CB_GETDROPPEDWIDTH;
    msgSet = CB_SETDROPPEDWIDTH;
    }

  int awi = SendMessage(hList, msgGet, 0, 0);
  if(wid > awi) {
    SendMessage(hList, msgSet, wid, 0);
    }
  SendMessage(hList, isLB ? LB_ADDSTRING : CB_ADDSTRING, 0, LPARAM(str));
}
//----------------------------------------------------------------------------
bool openFileTrend(HWND owner, HWND hwlist)
{
  TCHAR flt[_MAX_PATH];
  makeFilterTrend(flt, SIZE_A(flt));
  infoOpenSave Info(filterExtTrend, flt, infoOpenSave::OPEN_F_MULTIPLE, lastIx);
  POpenSave open(owner);

  if(open.run(Info)) {
    lastIx = Info.ixFilter;
    LPCTSTR p = open.getFile();
    if(p) {
      TCHAR path[_MAX_PATH];
      _tcscpy_s(path, p);
      LPTSTR ppath = path + _tcslen(path);
      p += _tcslen(p) + 1;
      while(p && *p) {
        *ppath = 0;
        appendPath(path, p);
        int tick = 0;
        int ncol = 0;
        if(testAutoCheck(path, tick, ncol)) {
          TCHAR buff[_MAX_PATH + 64];
          wsprintf(buff, _T("%2d - %s"), ncol, path);
          addString(hwlist, buff);
          }
        p += _tcslen(p) + 1;
        }
      }
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
class exportDialog : public PModDialog
{
    typedef PModDialog baseClass;
  public:
    exportDialog(mainWin* parent, exportInfo& pi, bool noCurr) :
        baseClass(parent, IDD_DIALOG_EXPORT), eI(pi), noCurr(noCurr) {}
    ~exportDialog() { destroy(); }

    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    exportInfo& eI;
    bool noCurr;
    void checkEnable();
    void chooseFile();
    void remFile();
};
//----------------------------------------------------------------------------
bool exportDialog::create()
{
  if(!baseClass::create())
    return false;

  setLang(*this, IDT_DLG_EXP);

  idc_id idc[] = {
     { IDOK, IDT_OK },
     { IDCANCEL, IDT_CANC },
     { IDC_RADIO_CURR_FILE, IDT_EXP_CURR },
     { IDC_RADIO_FILES, IDT_EXP_FILES },
     { IDC_RADIO_TABBED, IDT_EXP_TAB },
     { IDC_RADIO_SQL, IDT_EXP_SQL },
     { IDC_CHECK_UNIQUE_TARGET, IDT_EXP_UNIQUE_T },
     { IDC_CHECK_SAVE_SEL, IDT_EXP_SEL },
  };

  for(uint i = 0; i < SIZE_A(idc); ++i)
    setLang(*this, idc[i].idObj, idc[i].idText);

  if(eI.type & 1)
    SET_CHECK(IDC_RADIO_SQL);
  else
    SET_CHECK(IDC_RADIO_TABBED);
  if(noCurr) {
    eI.type |= 2;
    ENABLE(IDC_RADIO_CURR_FILE, false);
    }
  if(eI.type & 2)
    SET_CHECK(IDC_RADIO_FILES);
  else
    SET_CHECK(IDC_RADIO_CURR_FILE);

  if(eI.type & 4)
    SET_CHECK(IDC_CHECK_UNIQUE_TARGET);

  if(eI.type & 8)
    SET_CHECK(IDC_CHECK_SAVE_SEL);

  checkEnable();
  return true;
}
//-----------------------------------------------------------
LRESULT exportDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIO_FILES:
        case IDC_RADIO_CURR_FILE:
          checkEnable();
          break;
        case IDC_BUTTON_CHOOSE_FILE:
          chooseFile();
          break;
        case IDC_BUTTON_CHOOSE_FILE_REM:
          remFile();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void exportDialog::chooseFile()
{
  openFileTrend(*this, GetDlgItem(*this, IDC_LIST_FILE));
}
//----------------------------------------------------------------------------
void exportDialog::remFile()
{
  HWND hwlist = GetDlgItem(*this, IDC_LIST_FILE);
  int nElem = SendMessage(hwlist, LB_GETSELCOUNT, 0, 0);
  int* v = new int[nElem];
  SendMessage(hwlist, LB_GETSELITEMS, nElem, (LPARAM)v);
  for(int i = nElem - 1; i >= 0; --i)
    SendMessage(hwlist, LB_DELETESTRING, v[i], 0);
  delete []v;
}
//----------------------------------------------------------------------------
void exportDialog::checkEnable()
{
  bool enable = IS_CHECKED(IDC_RADIO_FILES);
  ENABLE(IDC_CHECK_UNIQUE_TARGET, enable);
  ENABLE(IDC_BUTTON_CHOOSE_FILE, enable);
  ENABLE(IDC_BUTTON_CHOOSE_FILE_REM, enable);
  ENABLE(IDC_LIST_FILE, enable);
  ENABLE(IDC_CHECK_SAVE_SEL, !enable);
}
//----------------------------------------------------------------------------
void exportDialog::CmOk()
{
  eI.type = 0;
  if(IS_CHECKED(IDC_RADIO_SQL))
    eI.type |= 1;
  if(IS_CHECKED(IDC_RADIO_FILES)) {
    HWND hwlist = GetDlgItem(*this, IDC_LIST_FILE);
    int nElem = SendMessage(hwlist, LB_GETCOUNT, 0, 0);
    if(nElem <= 0) {
      MessageBox(*this, getStringOrDef(IDT_EXP_NO_FILE_ADDED, _T("Nessun file aggiunto alla lista")),
            getStringOrDef(IDT_GENERIC_WARNING, _T("Attenzione")), MB_OK | MB_ICONSTOP);
      return;
      }
    eI.type |= 2;
    eI.fileSet.setDim(nElem);
    eI.fieldNumSet.setDim(nElem);
    TCHAR path[_MAX_PATH + 64];
    for(int i = 0; i < nElem; ++i) {
      int tick = 0;
      int ncol = 0;
      SendMessage(hwlist, LB_GETTEXT, i, (LPARAM)path);
      eI.fieldNumSet[i] = _ttoi(path);
      LPCTSTR p = path;
      for(; *p; ++p) {
        if(_T('-') == *p) {
          ++p;
          ++p;
          break;
          }
        }
      if(!*p)
        p = path;
      eI.fileSet[i] = str_newdup(p);
      }
    if(IS_CHECKED(IDC_CHECK_UNIQUE_TARGET)) {
      eI.type |= 4;
      int ncol = eI.fieldNumSet[0];
      for(int i = 1; i < nElem; ++i) {
        if(ncol != eI.fieldNumSet[i]) {
          if(IDYES == MessageBox(*this, getStringOrDef(IDT_EXP_NO_SAME_COL, _T("I file hanno numero di colonne diverse!\nVuoi salvarli individualmente?")),
              getStringOrDef(IDT_GENERIC_WARNING, _T("Attenzione")), MB_YESNO | MB_ICONSTOP)) {
            eI.type &= ~4;
            break;
            }
          else {
            flushPAV(eI.fileSet);
            eI.fieldNumSet.reset();
            return;
            }
          }
        }
      }
    }
  else if(IS_CHECKED(IDC_CHECK_SAVE_SEL)) {
    P_ModListFiles* mlf = getParentWin<mainWin>(getParent())->getListFile();
    mlf->fillSelected(eI.fieldNumSet);
    if(!eI.fieldNumSet.getElem())
      return;
    eI.type |= 8;
    }

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void mainWin::menuExport()
{
  static DWORD type;
  exportInfo eI;
  eI.type = type;
  if(IDOK == exportDialog(this, eI, clientFile->getCurrNumCol() <= 0).modal())
    clientFile->exportRec(eI);
  type = eI.type;
}
//----------------------------------------------------------------------------
bool mainWin::idle(DWORD count)
{
  static uint Flag = 1 << 10;
  int numCol = clientFile->getCurrNumCol();
  uint flag = 0;
  if(!numCol)
    flag = (uint)-1;
  else {
    if(!clientFile->isDirty())
      flag |= 4;
    if(numCol < 2)
      flag |= 1;
    }
  if(!clientFile->hasSelected())
    flag |= 8;
  if(!IsClipboardFormatAvailable(myCF_TEXT))
    flag |= 16;

  if(Flag != flag) {
    Flag = flag;
    menuBtn->enableDisable(flag);
    }
  return baseClass::idle(count);
}
