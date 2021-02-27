//------ my_opensave.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "popensave.h"
#include "p_manage_ini.h"
#include "common.h"
//----------------------------------------------------------------------------
//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\periphAddr7")
#define PERIPH_ADDR_INI_FILE _T("periphAddr.ini")
#define MAIN_PATH _T("\\Main")

#define OLD_PATH _T("OldPath")
#define MAX_ROW _T("MaxRow")
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
void setKeyParam(LPCTSTR keyName, LPCTSTR keyBlock, DWORD value)
{
  myManageIni ini(PERIPH_ADDR_INI_FILE);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, keyBlock);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyParam(LPCTSTR keyName, LPCTSTR keyBlock, DWORD& value)
{
  myManageIni ini(PERIPH_ADDR_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    value = (DWORD)t;
    }
}
//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPCTSTR path)
{
  myManageIni ini(PERIPH_ADDR_INI_FILE);
  ini.parse();
  ini_param param = { keyName, path };
  ini.addParam(param, keyBlock);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPTSTR path)
{
  path[0] = 0;
  myManageIni ini(PERIPH_ADDR_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str)
    _tcscpy_s(path, _MAX_PATH, str);
}
//----------------------------------------------------------------------------
void saveNumOfRow(DWORD maxRow, DWORD prph)
{
  TCHAR buff[100];
  wsprintf(buff, _T("%s_%02d"), MAX_ROW, prph);
  setKeyParam(buff, MAIN_PATH, maxRow);
}
//----------------------------------------------------------------------------
DWORD getNumOfRow(DWORD prph)
{
  TCHAR buff[100];
  wsprintf(buff, _T("%s_%02d"), MAX_ROW, prph);
  DWORD maxRow = 0;
  getKeyParam(buff, MAIN_PATH, maxRow);

  if(!maxRow || maxRow > MAX_ADDRESSES)
    maxRow = MAX_ADDRESSES;
  return maxRow;
}
//----------------------------------------------------------------------------
void checkBasePage()
{
  int successCount = 0;
  WIN32_FIND_DATA FindFileData;
  LPCTSTR toFind[] = {
    _T("NPS_*.exe"),
    _T("prph_*.exe"),
    _T("prph_*.ad?"),
    _T("prph_*.dat"),
    _T("job_data.dat"),
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
    setKeyPath(OLD_PATH, MAIN_PATH, path);
    }
}
//----------------------------------------------------------------------------
bool setFolderSVisor()
{
  TCHAR path[_MAX_PATH] = _T("");
  getKeyPath(OLD_PATH, MAIN_PATH, path);
  if(*path) {
    SetCurrentDirectory(path);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool chooseFolderSVisor()
{
  TCHAR path[_MAX_PATH] = _T("\0");
  getKeyPath(OLD_PATH, MAIN_PATH, path);

  if(PChooseFolder(path, _T("Scelta Cartella di lavoro del SVisor"))) {
    setKeyPath(OLD_PATH, MAIN_PATH, path);
    SetCurrentDirectory(path);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
