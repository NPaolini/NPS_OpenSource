//---------- mainWin.cpp -----------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "mainwin.h"
#include "P_ModListFiles.h"
#include "P_ModListCodes.h"
#include "P_ModEdit.h"
#include "pMenuBtn.h"
#include "pSplitwin.h"
#include "popensave.h"
#include "p_util.h"
#include "p_file.h"
#include "p_manage_ini.h"
#include "resource.h"
#include "manExcel.h"
#include "macro_utils.h"
#include "chooseExport.h"
//----------------------------------------------------------------------------
#define SUB_BASE_KEY _T("Software\\NPS_Programs\\svLang")
#define REL_SYSTEM_PATH _T("LastPath")
#define FILL_INFO _T("fillInfo")
//----------------------------------------------------------------------------
#define COORDS_PATH _T("coords")
#define SIZE_PATH   _T("size")
#define SPLIT_1_2_PATH   _T("split_1_2")
//----------------------------------------------------------------------------
#define INI_FILENAME _T("svLang.ini")
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
static int getDec(TCHAR car)
{
  if(car >= _T('0') && car <= _T('9'))
    return car - _T('0');
  if(car >= _T('a') && car <= _T('f'))
    return car - _T('a') + 10;
  if(car >= _T('A') && car <= _T('F'))
    return car - _T('A') + 10;
  return 0;
}
//----------------------------------------------------------------------------
static int hexToInt(LPCTSTR buff)
{
  return (getDec(buff[0]) << 4) + getDec(buff[1]);
}
//----------------------------------------------------------------------------
bool writeKey(LPCTSTR name, LPCTSTR keyBlock, LPCBYTE value, int len)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  TCHAR t[256];
  for (int i = 0, j = 0; i < len; ++i, j += 2) {
    wsprintf(t + j, _T("%02X"), value[i]);
  }
  ini_param param = { name, t };
  ini.addParam(param, MAIN_PATH);
  return ini.save();
}
//----------------------------------------------------------------------------
bool readKey(LPCTSTR name, LPCTSTR keyBlock, LPBYTE value, int len, bool writeIfNotFound)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(name, MAIN_PATH);
  if(!str) {
    if(writeIfNotFound)
      return writeKey(name, keyBlock, value, len);
    return false;
    }
  int l = _tcslen(str);
  l = min(l, len * 2);
  int j = 0;
  for(int i = 0; i < l; i += 2, ++j) {
    TCHAR t[3] = {};
    t[0] = str[i];
    t[1] = str[i + 1];
    value[j] = (BYTE) hexToInt(t);
    }
  for(; j < len; ++j)
    value[j] = 0;
  return true;
}
//----------------------------------------------------------------------------
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
void setFillInfo(const fillInfo& fInfo)
{
  writeKey(FILL_INFO, MAIN_PATH, (LPCBYTE)&fInfo, sizeof(fInfo));
}
//----------------------------------------------------------------------------
void getFillInfo(fillInfo& fInfo)
{
  readKey(FILL_INFO, MAIN_PATH, (LPBYTE)&fInfo, sizeof(fInfo), false);
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
//----------------------------------------------------------------------------
static setOfString gSet;
//----------------------------------------------------------------------------
mainWin::mainWin(HINSTANCE hInstance) :
      PMainWin(_T("npsvLang"), hInstance), mainSplit(0)
{
  GetModuleDirName(SIZE_A(CurrPath), CurrPath);
  appendPath(CurrPath, _T("npsvLang.lng"));
  gSet.add(CurrPath);
  ZeroMemory(CurrPath, sizeof(CurrPath));
}
//----------------------------------------------------------------------------
mainWin::~mainWin()
{
  destroy();
  delete mainSplit;
}
//------------------------------------------------------------------------------
P_ModListCodes* getListCode(PWin* child)
{
  mainWin* mv = getParentWin<mainWin>(child);
  return mv->getListCode();
}
//------------------------------------------------------------------------------
P_ModListFiles* getListFile(PWin* child)
{
  mainWin* mv = getParentWin<mainWin>(child);
  return mv->getListFile();
}
//------------------------------------------------------------------------------
P_ModEdit* getEditCode(PWin* child)
{
  mainWin* mv = getParentWin<mainWin>(child);
  return mv->getEditCode();
}
//-----------------------------------------------------------
#define CLASS_NAME_SVMAKER _T("npsvMaker_ClassWin")
//-----------------------------------------------------------
static LPCTSTR getMsgWarn()
{
  return getStringOrDef(50, _T("Il programma npsvMaker è in esecuzione,\r\n")
         _T("poiché può interferire col npsvLang è\r\n")
         _T("preferibile non averli entrambi in funzione,\r\n")
         _T("o almeno che non lavorino sullo stesso progetto."));
}
//-----------------------------------------------------------
static void checkRunningSvMaker(PWin* owner)
{
  if(FindWindow(CLASS_NAME_SVMAKER, 0))
    MessageBox(*owner, getMsgWarn(), getStringOrDef(27, _T("Attenzione")), MB_OK | MB_ICONINFORMATION);
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
//------------------------------------------------------------------------------
#define IDD_MENU_BTN 1
#define IDD_MAIN_SPLIT 2
#define IDD_CLIENT_SPLIT 3
#define IDD_CODE 4
#define IDD_FILE 5
//------------------------------------------------------------------------------
bool mainWin::create()
{
  DWORD split_1_2 = 0;
  getKeyParam(SPLIT_1_2_PATH, &split_1_2);

  menuBtn = new pMenuBtn(this, IDD_MENU_BTN);

  int perc = 550;
  if(LOWORD(split_1_2))
    perc = LOWORD(split_1_2);
  mainClientSplit = new PSplitWin(this, IDD_MAIN_SPLIT, perc, 4, PSplitter::PSW_VERT);

  perc = 350;
  if(HIWORD(split_1_2))
    perc = HIWORD(split_1_2);

  clientSplit = new PSplitWin(mainClientSplit, IDD_CLIENT_SPLIT, perc, 4);


  clientCode = new P_ModListCodes(mainClientSplit, IDD_CODE);
  clientEdit = new P_ModEdit(clientSplit, IDD_EDIT);

  clientFile = new P_ModListFiles(clientSplit, IDD_FILE);

  mainClientSplit->setWChild(clientCode, clientSplit);
  clientSplit->setWChild(clientFile, clientEdit);
  mainSplit = new PSplitter(this, menuBtn, mainClientSplit);
  mainSplit->setDim1(OFFS_BTN * 2 + SIZE_BTN, false);
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

  haccel = LoadAccelerators(getHInstance(), MAKEINTRESOURCE(IDR_ACCELERATOR1));

  bool success = mainSplit->create();
  checkRunningSvMaker(this);

  checkBasePage();
  TCHAR path[_MAX_PATH];
  getPath(path);
  if(*path) {
    LPTSTR p = path + _tcslen(path);
    appendPath(path, STD_MSG_NAME);
    if(P_File::P_exist(path)) {
      *p = 0;
      setPathAndTitle(path);
      }
    else
      setFolder();
    }
  return success;
}
//----------------------------------------------------------------------------
void mainWin::checkBasePage()
{
  int successCount = 0;
  WIN32_FIND_DATA FindFileData;
  LPCTSTR toFind[] = {
    _T("nps*.exe"),
    _T("prph_*.exe"),
    _T("prph_*.ad7"),
    _T("prph_*.dat"),
    _T("job_base.dat"),
    _T("system\\") FIRST_PAGE,
    _T("system\\") STD_MSG_NAME,
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
    setPath(path);
    }
}
//----------------------------------------------------------------------------
bool mainWin::preProcessMsg(MSG& msg)
{
  if(TranslateAccelerator(getHandle(), haccel, &msg))
    return true;
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
#define SAVE_QUERY_MSG getStringOrDef(51, _T("Uno o più file sono stati modificati\r\nVuoi salvare il progetto prima di proseguire?"))
#define SAVE_QUERY_TIT getStringOrDef(27, _T("Attenzione"))
#define SAVE_QUERY_FLAG (MB_YESNOCANCEL | MB_ICONINFORMATION)
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
      do {
        int perc1 = mainClientSplit->getSplitter()->getPerc();
        int perc2 = clientSplit->getSplitter()->getPerc();
        setKeyParam(SPLIT_1_2_PATH, MAKELONG(perc1, perc2));
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
        case CM_BTN_UPD:
          clientFile->refresh(CurrPath);
          break;
        case CM_BTN_FOLDER:
          setFolder();
          break;
        case CM_BTN_SAVE:
          save();
          break;
        case CM_BTN_EXPORT:
          exportExcel();
          break;
        case CM_BTN_IMPORT:
          importExcel();
          break;
        }
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case ID_CM_F2:
          clientCode->setPrev();
          break;
        case ID_CM_F3:
          clientCode->setNext();
          break;
        case ID_CM_F4:
          fill();
          break;
        case ID_CM_F5:
          clientEdit->toggleNameLang();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
#define SYS_STR _T("\\system")
//----------------------------------------------------------------------------
void checkSystem(LPTSTR path)
{
  int len = _tcslen(path);
  for(int i = 0; i < len; ++i)
    path[i] = _totlower(path[i]);

  LPTSTR f = _tcsstr(path, SYS_STR);
  if(f)
    *f = 0;
}
//----------------------------------------------------------------------------
void mainWin::setPathAndTitle(LPCTSTR path)
{
  TCHAR oldPath[_MAX_PATH];
  SetCurrentDirectory(path);
  _tcscpy_s(oldPath, path);
  checkSystem(oldPath);
  _tcscpy_s(CurrPath, _MAX_PATH, oldPath);
  _tcscpy_s(oldPath, _T("NP-svLang - "));
  _tcscat_s(oldPath, CurrPath);
  setCaption(oldPath);
  appendPath(CurrPath, SYS_STR);
  setPath(CurrPath);
  clientFile->refresh(CurrPath);
  menuBtn->setUnicode();
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
  _tcscpy_s(oldPath, CurrPath);
  checkSystem(oldPath);
  if(PChooseFolder(oldPath, getStringOrDef(36, _T("Cartella progetto")), false, oldPath)) {
    setPathAndTitle(oldPath);
    }
}
//----------------------------------------------------------------------------
void mainWin::save()
{
  if(!clientFile->isDirty()) {
    MessageBox(*this, getStringOrDef(37, _T("Nessuna modifica apportata")), getStringOrDef(38, _T("Inutile salvare")), MB_OK | MB_ICONINFORMATION);
    return;
    }
  if(!clientFile->save(menuBtn->saveOnUnicode()))
    MessageBox(*this, getStringOrDef(39, _T("Errore nel salvataggio!")), getStringOrDef(27, _T("Attenzione")), MB_OK | MB_ICONSTOP);
  else
    MessageBox(*this, getStringOrDef(40, _T("Salvataggio riuscito")), getStringOrDef(41, _T("Salvataggio file")), MB_OK | MB_ICONINFORMATION);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR filterExtExcel[] = { _T(".xls"), _T(".*"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterExcel =
  _T("Excel File (xls)\0*.xls\0")
  _T("Tutti i File (*.*)\0*.*\0");
//----------------------------------------------------------------------------
static DWORD lastIx;
//----------------------------------------------------------------------------
bool openFileExcel(HWND owner, LPTSTR file, LPCTSTR CurrPath)
{
  infoOpenSave Info(filterExtExcel, filterExcel, infoOpenSave::OPEN_F, lastIx);
  POpenSave open(owner);

  TCHAR basePath[_MAX_PATH];
  _tcscpy_s(basePath, CurrPath);
  checkSystem(basePath);
  SetCurrentDirectory(basePath);
  if(open.run(Info)) {
    lastIx = Info.ixFilter;
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool saveFileExcel(HWND owner, LPTSTR file, LPCTSTR CurrPath)
{
  infoOpenSave Info(filterExtExcel, filterExcel, infoOpenSave::SAVE_AS, lastIx);
  POpenSave save(owner);

  TCHAR basePath[_MAX_PATH];
  _tcscpy_s(basePath, CurrPath);
  checkSystem(basePath);
  SetCurrentDirectory(basePath);

  if(save.run(Info)) {
    lastIx = Info.ixFilter;
    _tcscpy_s(file, _MAX_PATH, save.getFile());
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void mainWin::exportExcel()
{
  if(clientFile->isDirty()) {
    UINT res = MessageBox(*this, getStringOrDef(42, _T("Salvare le modifiche correnti?")),
              getStringOrDef(43, _T("Ci sono dati non salvati")), MB_YESNOCANCEL | MB_ICONINFORMATION);
    switch(res) {
      case IDYES:
        if(!clientFile->save(menuBtn->saveOnUnicode()))
          if(IDYES != MessageBox(*this, getStringOrDef(44, _T("Errore nel salvataggio!\r\nVuoi continuare lo stesso?")),
              getStringOrDef(27, _T("Attenzione")), MB_YESNO | MB_ICONSTOP))
            return;
        break;
      case IDNO:
        break;
      case IDCANCEL:
        return;
      }
    }
  TCHAR path[_MAX_PATH];
  if(!saveFileExcel(*this, path, CurrPath))
    return;

  PVect<LPCTSTR> filename;
  PVect<LPTSTR> lang;
  clientFile->fillListFile(filename);
  clientEdit->getTitle(lang);
  if(IDOK == PD_chooseExport(this, filename).modal())
    bool success = IDOK == manExcel(this, IDD_DIALOG_IMP_EXP, path, CurrPath, filename, lang).modal();
  releaseVect(filename);
  releaseVect(lang);
}
//----------------------------------------------------------------------------
void mainWin::importExcel()
{
  TCHAR path[_MAX_PATH];
  if(!openFileExcel(*this, path, CurrPath))
    return;
  manExcel(this, IDD_DIALOG_IMP_EXP, path, CurrPath).modal();
  clientFile->refresh(CurrPath);
}
//----------------------------------------------------------------------------
LPCTSTR getStringOrDef(uint code, LPCTSTR def)
{
  LPCTSTR p = gSet.getString(code);
  return p ? p : def;
}
//----------------------------------------------------------------------------
class fillDialog : public PModDialog
{
    typedef PModDialog baseClass;
  public:
    fillDialog(mainWin* parent, fillInfo& fi, PVect<LPTSTR>& lang) :
        baseClass(parent, IDD_DIALOG_FILL), fInfo(fi), Lang(lang) {}
    ~fillDialog() { destroy(); }

    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void checkSel();
    fillInfo& fInfo;
    PVect<LPTSTR>& Lang;
};
//----------------------------------------------------------------------------
#define ID_TITLE_FILL 57
//----------------------------------------------------------------------------
const uint idcCheck[] = { IDC_CHECK1, IDC_CHECK2, IDC_CHECK3, IDC_CHECK4, IDC_CHECK5,
      IDC_CHECK6, IDC_CHECK7, IDC_CHECK8, IDC_CHECK9, IDC_CHECK10 };
//----------------------------------------------------------------------------
bool fillDialog::create()
{
  if(!baseClass::create())
    return false;

  setLang(*this, ID_TITLE_FILL);

  struct idcText {
    uint idc;
    uint idText;
    };
  idcText it[] = {
    { IDOK, 58 },
    { IDCANCEL, 28 },
    { IDC_RADIO1, 55 },
    { IDC_RADIO2, 56 },
    { IDC_RADIO3, 70 },
    { IDC_CHECK11, 54 },
    { IDC_STATIC_SOURCE, 52 },
    { IDC_STATIC_TARGET, 53 },
    };
  for(uint i = 0; i < SIZE_A(it); ++i)
    setLang(*this, it[i].idc, it[i].idText);

  HWND hwCbx = GetDlgItem(*this, IDC_COMBO1);
  TCHAR t[64];
  for(uint i = 0; i < MAX_LANG; ++i) {
    LPCTSTR p = Lang[i];
    if(!*p) {
      wsprintf(t, _T("[%d]"), i + 1);
      p = t;
      }
    SendMessage(hwCbx, CB_ADDSTRING, 0, (LPARAM)p);
    SetDlgItemText(*this, idcCheck[i], p);
    if(fInfo.target & (1 << i))
      SET_CHECK(idcCheck[i]);
    }
  SendMessage(hwCbx, CB_SETCURSEL, fInfo.source, 0);
  if(fInfo.action & 1)
    SET_CHECK(IDC_RADIO2);
  else if(fInfo.action & 4)
    SET_CHECK(IDC_RADIO3);
  else
    SET_CHECK(IDC_RADIO1);
  if(fInfo.action & 2)
    SET_CHECK(IDC_CHECK11);
  checkSel();
  return true;
}
//-----------------------------------------------------------
void fillDialog::checkSel()
{
  HWND hwCbx = GetDlgItem(*this, IDC_COMBO1);
  int source = SendMessage(hwCbx, CB_GETCURSEL, 0, 0);
  if(source < 0)
    return;

  for(uint i = 0; i < MAX_LANG; ++i) {
    if(i == source) {
      SET_CHECK_SET(idcCheck[i], false);
      ENABLE(idcCheck[i], false);
      }
    else
      ENABLE(idcCheck[i], true);
    }
}
//-----------------------------------------------------------
LRESULT fillDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_COMBO1:
          switch(HIWORD(wParam)) {
            case CBN_SELCHANGE:
              checkSel();
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void fillDialog::CmOk()
{
  HWND hwCbx = GetDlgItem(*this, IDC_COMBO1);
  fInfo.source = SendMessage(hwCbx, CB_GETCURSEL, 0, 0);
  fInfo.target = 0;
  for(uint i = 0; i < MAX_LANG; ++i) {
    if(IS_CHECKED(idcCheck[i]))
      fInfo.target |= 1 << i;
    }
  fInfo.action = 0;
  if(IS_CHECKED(IDC_RADIO2))
    fInfo.action = 1;
  else if(IS_CHECKED(IDC_RADIO3))
    fInfo.action = 4;
  if(IS_CHECKED(IDC_CHECK11))
    fInfo.action |= 2;
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void mainWin::fillFieldAllFile(const fillInfo& fi)
{
  int oldSel = clientFile->getCurrSel();
  if(clientFile->setFirst()) {
    do {
      clientCode->fillField(fi);
      }  while(clientFile->setNext());
    }
  clientFile->setCurrSel(oldSel);
}
//----------------------------------------------------------------------------
void mainWin::fill()
{
  static fillInfo fInfo;
  getFillInfo(fInfo);
  PVect<LPTSTR> lang;
  clientEdit->getTitle(lang);
  if(IDOK == fillDialog(this, fInfo, lang).modal()) {
    setFillInfo(fInfo);
    if(fInfo.action & 4)
      fillFieldAllFile(fInfo);
    else {
      switch(fInfo.action & 1) {
        case 0:
          clientEdit->fillField(fInfo);
          break;
        case 1:
          clientCode->fillField(fInfo);
          break;
        }
      }
    }
  clientEdit->setTitle(lang);
}
//----------------------------------------------------------------------------
