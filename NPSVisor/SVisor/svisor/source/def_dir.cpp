//--------- def_dir.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "1.h"
#include "def_dir.h"
#include "p_date.h"
#include "log_stat.h"
#include "p_name.h"
//----------------------------------------------------------------------------
// percorso base
static TCHAR baseDir[_MAX_PATH] = _T("");
//----------------------------------------------------------------------------
// percorso locale base
static TCHAR localBaseDir[_MAX_PATH] = _T("");
//----------------------------------------------------------------------------
// percorso relativo, da aggiungere al percorso base
static TCHAR allSimpleDir[MAX_ID_CREATE_FOLDER][MAX_LEN_NAME_FOLDER + 2];
//----------------------------------------------------------------------------
// percorso relativo di default, da aggiungere al percorso base
static LPCTSTR allDefSimpleDir[] = {
  _T("alarm\\"),
  _T("recipe\\"),
  _T("order\\"),
  _T("oper\\"),
  _T("turn\\"),
  _T("system\\"),
  _T("history\\"),
  _T("winder\\"),
  _T("alarm\\"),
  _T("exRep\\"),
  _T("exRep\\"),
  };
//----------------------------------------------------------------------------
// percorso assoluto (base + relativo)
static TCHAR allDir[SIZE_A_c(allSimpleDir)][_MAX_PATH];
//----------------------------------------------------------------------------
static LPCTSTR defExt[] = {
  _T(".dat"),
  _T(".rcp"),
  _T(".ord"),
  _T(".dat"),
  _T(".dat"),
  _T(".dat"),
  _T(""),
  _T(".wnd"),
  _T(".rep"),
  _T(".ext"),
  _T(".tab"),

  };
//----------------------------------------------------------------------------
LPCTSTR getDir(eDir which)
{
  return allSimpleDir[which];
}
//----------------------------------------------------------------------------
LPCTSTR getExt(eDir path)
{
  return defExt[path];
}
//----------------------------------------------------------------------------
/*
LPCTSTR getExtExport()
{
  return _T(".tab");
}
*/
//----------------------------------------------------------------------------
static void getPath(LPCTSTR path, LPTSTR file)
{
  TCHAR pt[_MAX_PATH];
  _tcscpy_s(pt, SIZE_A(pt), file);
  int len = _tcslen(path);
  _tcsncpy_s(file, _MAX_PATH, path, len + 1);
  _tcscat_s(file, _MAX_PATH, pt);
}
//----------------------------------------------------------------------------
void makeAllDir()
{
  bool found = false;
  for(int i = 0; i < MAX_ID_CREATE_FOLDER; ++i) {
    LPCTSTR p = getString(ID_CREATE_FOLDER + i);
    allSimpleDir[i][0] = 0;
    if(p)  {
      int len = _tcslen(p);
      if(len > 0) {
        found = true;
        if(len > MAX_LEN_NAME_FOLDER)
          len = MAX_LEN_NAME_FOLDER;
        copyStr(allSimpleDir[i], p, len);
        allSimpleDir[i][len] = 0;
        if(allSimpleDir[i][len - 1] != _T('\\'))
          _tcscat_s(allSimpleDir[i], SIZE_A(allSimpleDir[i]), _T("\\"));
        }
      }
    }
  if(!found) {
    for(int i = 0; i < MAX_ID_CREATE_FOLDER; ++i)
      _tcscpy_s(allSimpleDir[i], SIZE_A(allSimpleDir[i]), allDefSimpleDir[i]);
    }
  setBasePath(baseDir);
  for(int i = 0; i < SIZE_A_c(allDir); ++i)
    CreateDirectory(allDir[i], 0);
}
//----------------------------------------------------------------------------
void getPath(LPTSTR path, eDir which)
{
  getPath(allDir[which], path);
}
//----------------------------------------------------------------------------
void getRelPath(LPTSTR path, eDir which)
{
  getPath(allSimpleDir[which], path);
}
//----------------------------------------------------------------------------
void makeExportPath(LPTSTR result, LPCTSTR name, bool history, eDir ext)
{
  if(!_tcschr(allSimpleDir[dExport], _T('\\')))
    makePath(result, name, dExport, history, ext);
  else {
    _tcscpy_s(result, _MAX_PATH, allSimpleDir[dExport]);
//    if(history)
//      appendPath(result, allSimpleDir[dHistory]);
    appendPath(result, name);
    if(dUseExtDefault == ext)
      ext = dExport;
    _tcscat_s(result, _MAX_PATH, getExt(ext));
    }
}
//----------------------------------------------------------------------------
void makePath(LPTSTR result, LPCTSTR name, eDir which, bool history, eDir ext)
{
  _tcscpy_s(result, _MAX_PATH, baseDir);
  if(history)
    _tcscat_s(result, _MAX_PATH, allSimpleDir[dHistory]);
  appendPath(result, allSimpleDir[which]);
  appendPath(result, name);
  if(dUseExtDefault == ext)
    ext = which;
  _tcscat_s(result, _MAX_PATH, getExt(ext));
}
//----------------------------------------------------------------------------
void makePath(LPTSTR file, eDir which, bool history, eDir ext)
{
  TCHAR pt[_MAX_PATH];
  _tcscpy_s(pt, SIZE_A(pt), file);
  makePath(file, pt, which, history, ext);
}
//----------------------------------------------------------------------------
void makePath(LPTSTR result, LPCTSTR name, eDir which, bool history, LPCTSTR ext)
{
  _tcscpy_s(result, _MAX_PATH, baseDir);
  if(history)
    appendPath(result, allSimpleDir[dHistory]);
  appendPath(result, allSimpleDir[which]);
  appendPath(result, name);
  _tcscat_s(result, _MAX_PATH, ext);
}
//----------------------------------------------------------------------------
void makePath(LPTSTR file, eDir which, bool history, LPCTSTR ext)
{
  TCHAR pt[_MAX_PATH];
  _tcscpy_s(pt, SIZE_A(pt), file);
  makePath(file, pt, which, history, ext);
}
//----------------------------------------------------------------------------
static inline void buildPath(LPCTSTR basepath, LPCTSTR path, LPTSTR result)
{
  _tcscpy_s(result, _MAX_PATH, basepath);
  appendPath(result, path);
}
//----------------------------------------------------------------------------
void setBasePath(LPCTSTR base)
{
  if(base && *base) {
    _tcscpy_s(baseDir, SIZE_A(baseDir), base);
    appendPath(baseDir, 0);
    }
  else
    _tcscpy_s(baseDir, SIZE_A(baseDir), _T(".\\"));

  GetCurrentDirectory(_MAX_PATH, localBaseDir);
  appendPath(localBaseDir, 0);

  for(int i = 0; i < SIZE_A_c(allDir); ++i)
    buildPath(baseDir, allSimpleDir[i], allDir[i]);
}
//----------------------------------------------------------------------------
LPCTSTR getLocalBaseDir()
{
  return localBaseDir;
}
//----------------------------------------------------------------------------
LPCTSTR getBaseDir()
{
  return baseDir;
}
//----------------------------------------------------------------------------
void getBasePath(LPTSTR file)
{
  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, SIZE_A(tmp), file);
  _tcscpy_s(file, _MAX_PATH, baseDir);
  appendPath(file, tmp);
}
//----------------------------------------------------------------------------
static void makedir(LPTSTR path)
{
  createDirectoryRecurse(path);
}
//----------------------------------------------------------------------------
bool makeLocalDir(LPCTSTR path)
{
  if(path[0] == _T('\\') && path[1] == _T('\\')) { // è un percorso di rete
    TCHAR tmp[_MAX_PATH];  // crea un percorso relativo simile al
    _tcscpy_s(tmp, SIZE_A(tmp), path + 2);// percorso di rete togliendo i due backslash
    makedir(tmp);         // iniziali
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
typedef void (*make_YearOrMonth)(LPTSTR name, int sub);
//----------------------------------------------------------------------------
static void checkForHistoryGen(LPCTSTR path, make_YearOrMonth fz, int lenCheck)
{
  TCHAR t[_MAX_PATH];
  int len = _tcslen(path);
  _tcsncpy_s(t, SIZE_A(t), path, len + 1);
  int i;
  for(i = len - 1; i > 0 && t[i] != _T('.'); --i);
  // crea il nome del file del mese/anno precedente
  fz(t + i - lenCheck, 1);
  _tcscat_s(t, path + i);

  WIN32_FIND_DATA ff;
  HANDLE hfff = FindFirstFile(t, &ff);
  if(INVALID_HANDLE_VALUE != hfff) {
    // se trova il file, probabilmente è cambiato il mese/anno
#if 1
    int i;
    bool first = true;
    for(i = len - 1; i > 0; --i) {
      if(_T('\\') == t[i]) {
        if(!first)
          break;        // secondo backslash
        first = false;  // primo backslash
        }
      }
    TCHAR newfile[_MAX_PATH];
    _tcsncpy_s(newfile, SIZE_A(newfile), t, i);
    newfile[i] = 0;
    TCHAR tHist[_MAX_PATH] = _T("");
    getRelPath(tHist, dHistory);
    appendPath(newfile, tHist);
    appendPath(newfile, t + i + 1);
    i = _tcslen(newfile);
    for(--i; i > 0; --i) {
      if(_T('\\') == newfile[i]) {
        newfile[i] = 0;
        createDirectoryRecurse(newfile);
        newfile[i] = _T('\\');
        break;
        }
      }

#else
    for(--len; len > 0 && t[len] != _T('\\'); --len)
      ;
    TCHAR newfile[_MAX_PATH];
    _tcsncpy_s(newfile, SIZE_A(newfile), t, len);
    newfile[len] = 0;
    TCHAR tHist[_MAX_PATH] = _T("");
    getRelPath(tHist, dHistory);
    appendPath(newfile, tHist);
    CreateDirectory(newfile, 0);
    appendPath(newfile, ff.cFileName);
#endif
    MoveFile(t, newfile);
    FindClose(hfff);
    }
}
//----------------------------------------------------------------------------
void checkForHistory(LPCTSTR path)
{
  checkForHistoryGen(path, makeYear, MAXLEN_NAME_FILE_DAT - 4);
}
//----------------------------------------------------------------------------
void checkForHistoryMonth(LPCTSTR path)
{
  checkForHistoryGen(path, makeYearAndMonth, MAXLEN_NAME_FILE_DAT - 2);
}
//----------------------------------------------------------------------------
