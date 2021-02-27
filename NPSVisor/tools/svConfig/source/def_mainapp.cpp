//------ def_mainapp.cpp -----------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
#include <dos.h>
//----------------------------------------------------------------------------
#include "config.h"
//----------------------------------------------------------------------------
//static PWin* mainWin = 0;
//PWin* getMain() { return mainWin; }
bool openFileSetup(HWND owner, LPTSTR file);
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
static TCHAR currPath[_MAX_PATH];
LPCTSTR getCurrPath() { return currPath; }
//----------------------------------------------------------------------------
myManageIni2::myManageIni2(LPCTSTR filename) : baseClass(filename)
{
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, getCurrPath());
  dirName(dirName(t));
  appendPath(t, _T("npsVisor.ini"));
  delete []Filename;
  Filename = str_newdup(t);
}
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE, //hPrevInstance,
                     LPSTR,     //lpCmdLine,
                     int       nCmdShow)
{

  InitCommonControls();
  if(openFileSetup(0, currPath))
    config(currPath).setup(0, hInstance);

  return 0;
}

//----------------------------------------------------------------------------
#include "popensave.h"
//----------------------------------------------------------------------------
//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\npsvConfig")
#define OLD_PATH  _T("OldPath")
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPCTSTR path)
{
  myManageIni ini(INI_FILE_NAME);
  ini.parse();
  ini_param param = { keyName, path };
  ini.addParam(param, keyBlock);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPTSTR path)
{
  path[0] = 0;
  myManageIni ini(INI_FILE_NAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str)
    _tcscpy_s(path, _MAX_PATH, str);
}
//----------------------------------------------------------------------------
class myOpenSave : public POpenSave
{
  public:
    myOpenSave(HWND owner, LPCTSTR keyBase, LPCTSTR keyString) : POpenSave(owner),
        KeyBase(str_newdup(keyBase)), KeyString(str_newdup(keyString))  {  }
    ~myOpenSave() { delete []KeyString; delete []KeyBase; }
    void setChoosedPath() { setPathFromHistory(); }
  protected:
    LPTSTR KeyBase;
    LPTSTR KeyString;
    virtual void setPathFromHistory();
    virtual void savePathToHistory();
};
//----------------------------------------------------------------------------
void myOpenSave::setPathFromHistory()
{
  TCHAR path[_MAX_PATH] = {};
  getKeyPath(KeyString, KeyBase, path);
  if(*path)
    SetCurrentDirectory(path);
}
//----------------------------------------------------------------------------
void myOpenSave::savePathToHistory()
{
  TCHAR oldPath[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, oldPath);
  setKeyPath(KeyString, KeyBase, oldPath);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR filterExt[] = { _T(".dat"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filter =
  _T("Setup file (dat)\0")
  _T("setup*.dat\0");
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define PAGE_EXT _T(".npt")
#define FIRST_PAGE _T("main_page") PAGE_EXT
//----------------------------------------------------------------------------
#define STD_MSG_NAME _T("base_data") PAGE_EXT
//----------------------------------------------------------------------------
void checkBasePage()
{
  int successCount = 0;
  WIN32_FIND_DATA FindFileData;
  LPCTSTR toFind[] = {
    _T("nps*.exe"),
    _T("prph_*.exe"),
    _T("#prph_*.ad7"),
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
    appendPath(path, _T("system"));
    setKeyPath(OLD_PATH, MAIN_PATH, path);
    }
}
//----------------------------------------------------------------------------
bool openFileSetup(HWND owner, LPTSTR file)
{
  checkBasePage();

  DWORD lastIx = 0;
  infoOpenSave Info(filterExt, filter, infoOpenSave::OPEN_F, lastIx);
  myOpenSave open(owner, MAIN_PATH, OLD_PATH);

  if(open.run(Info)) {
    _tcscpy(file, open.getFile());
    open.setChoosedPath();
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
