//----------- manExcel.cpp ----------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "headerMsg.h"
#include "macro_utils.h"
#include "pStatic.h"
#include "p_file.h"
#include "manExcel.h"
#include "mainwin.h"
#include "resource.h"
#include <commctrl.h>
#include "BasicExcelVC6.hpp"
using namespace YExcel;
//---------------------------------------------------------------------
//---------------------------------------------------------------------
#define WM_CUSTOM_INFO (WM_FW_FIRST_FREE + 1)
//---------------------------------------------------------------------
static PVect<LPCTSTR> dummy1;
static PVect<LPTSTR> dummy2;
//---------------------------------------------------------------------
manExcel::manExcel(PWin* parent, uint idc, LPCTSTR excelFile, LPCTSTR basePath,
        const PVect<LPCTSTR>& filename, const PVect<LPTSTR>& lang)
    : baseClass(parent, idc), ExcelFile(str_newdup(excelFile)), basePath(str_newdup(basePath)),
      Filename(filename), Lang(lang), isImport(false), Running(false), idThread(0),
      hEventClose(CreateEvent(0, TRUE, 0, 0))
{}
//---------------------------------------------------------------------
manExcel::manExcel(PWin* parent, uint idc, LPCTSTR excelFile, LPCTSTR basePath)
    : baseClass(parent, idc), ExcelFile(str_newdup(excelFile)), basePath(str_newdup(basePath)),
      Filename(dummy1), Lang(dummy2), isImport(true), Running(false), idThread(0),
      hEventClose(CreateEvent(0, TRUE, 0, 0))
{}
//---------------------------------------------------------------------
manExcel::~manExcel()
{
  if(hEventClose) {
    SetEvent(hEventClose);
    while(true) {
      if(!idThread)
        break;
      Sleep(100);
      }
    CloseHandle(hEventClose);
    }
  delete []ExcelFile;
  delete []basePath;
}
//---------------------------------------------------------------------
bool manExcel::create()
{
  PStatic* st = new PStatic(this, IDC_STATIC_INFO);
  if(!baseClass::create())
    return false;
  st->setFont(D_FONT(16, 0, fBOLD, _T("arial")), true);
  TCHAR tit[_MAX_PATH];
  if(isImport) {
    wsprintf(tit, getStringOrDef(14, _T("Importa da %s")), ExcelFile);
    SET_TEXT(IDC_STATIC_INFO, getStringOrDef(15, _T("Lettura pagine ...")));
    }
  else {
    wsprintf(tit, getStringOrDef(16, _T("Esporta in %s")), ExcelFile);
    SET_TEXT(IDC_STATIC_INFO, getStringOrDef(17, _T("Scrittura pagine ...")));
    }
  setCaption(tit);
  uint nElem = Filename.getElem();
  SendMessage(GetDlgItem(*this, IDC_PROGRESSBAR), PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, nElem));
  SET_TEXT(IDOK, getStringOrDef(34, _T("Avvia")));
  PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
  return true;
}
//---------------------------------------------------------------------
void manExcel::endWithMsg(LPCTSTR msg, bool success)
{
  UINT flag = MB_OK | (success ? MB_ICONINFORMATION : MB_ICONSTOP);
  LPCTSTR tit = success ? getStringOrDef(18, _T("Successo")) : getStringOrDef(19, _T("Errore"));
  MessageBox(*this, msg, tit, flag);
  if(success)
    baseClass::CmOk();
  else
    baseClass::CmCancel();
}
//---------------------------------------------------------------------
LRESULT manExcel::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CUSTOM_INFO:
      switch(LOWORD(wParam)) {
        case 0:
          infoFileOnJob(HIWORD(wParam), (LPCTSTR)lParam);
          break;
        case 1:
          endWithMsg(getStringOrDef(20, _T("Export terminato")), true);
          break;
        case 2:
          endWithMsg(getStringOrDef(21, _T("Export terminato prematuramente")), false);
          break;
        case 3:
          endWithMsg(getStringOrDef(22, _T("Import terminato")), true);
          break;
        case 4:
          endWithMsg(getStringOrDef(23, _T("Import terminato prematuramente")), false);
          break;
      break;
      }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------------------
void manExcel::infoFileOnJob(WORD flag, LPCTSTR file)
{
  switch(flag) {
    case 0:
      {
      SET_TEXT(IDC_STATIC_INFO_FILE, file);
      uint curr = SendMessage(GetDlgItem(*this, IDC_PROGRESSBAR), PBM_GETPOS, 0, 0);
      SendMessage(GetDlgItem(*this, IDC_PROGRESSBAR), PBM_SETPOS, curr + 1, 0);
      }
      break;
    case 1:
      SET_TEXT(IDC_STATIC_INFO, file);
      SET_TEXT(IDC_STATIC_INFO_FILE, getStringOrDef(24, _T("Attendere, potrebbero volerci alcuni minuti ...")));
      EnableWindow(GetDlgItem(*this, IDOK), false);
      break;
    case 2:
      SET_TEXT(IDC_STATIC_INFO_FILE, file);
      SET_TEXT(IDC_STATIC_INFO, getStringOrDef(25, _T("Salvataggio pagine ...")));
      EnableWindow(GetDlgItem(*this, IDOK), true);
      break;
    }
}
//---------------------------------------------------------------------
void manExcel::CmOk()
{
  if(Running) {
    if(IDYES == MessageBox(*this, getStringOrDef(26, _T("Terminare l'elaborazione?")), getStringOrDef(27, _T("Attenzione")), MB_YESNO | MB_ICONSTOP))
      baseClass::CmCancel();
    return;
    }
  Running = true;
  SET_TEXT(IDOK, getStringOrDef(28, _T("Annulla")));
  LPTHREAD_START_ROUTINE proc = isImport ?
      (LPTHREAD_START_ROUTINE)ImpProc : (LPTHREAD_START_ROUTINE)ExpProc;

  HANDLE hThread = CreateThread(0, 0, proc, this, 0, &idThread);
  if(!hThread) {
    MessageBox(*this, getStringOrDef(29, _T("Impossibile avviare il thread di lavoro")), getStringOrDef(30, _T("Error")), MB_OK | MB_ICONSTOP);
    baseClass::CmCancel();
    return;
    }
  CloseHandle(hThread);
}
//------------------------------------------------------------------------
#define START_BRACKET _T('{')
#define END_BRACKET   _T('}')
//------------------------------------------------------------------------
#define TRUE_MAX_LEN_SHEET 32
#define MAX_LEN_SHEET (TRUE_MAX_LEN_SHEET - 5)
//------------------------------------------------------------------------
static void copyRemoveSlash(LPTSTR path, LPCTSTR source, uint ix)
{
  uint len = _tcslen(source);
  if(!_tcsicmp(source + len - 4, PAGE_EXT))
    len -= 4;
  LPTSTR p = path;
  if(len < MAX_LEN_SHEET) {
    wsprintf(path, _T("%c%d%c"), START_BRACKET, ix, END_BRACKET);
    p = path + _tcslen(path);
    }
  for(uint i = 0; i < len; ++i) {
    if(_T('\\') == source[i] || _T('/') == source[i])
      p[i] = _T('#');
    else
      p[i] = source[i];
    }
  p[len] = 0;
  len = _tcslen(path);
  len = min(len, TRUE_MAX_LEN_SHEET);
  path[len] = 0;
}
//------------------------------------------------------------------------
static void copyRestoreSlash(LPTSTR path, LPCTSTR source)
{
  if(START_BRACKET == *source) {
    while(*source) {
      if(END_BRACKET == *source) {
        ++source;
        break;
        }
      ++source;
      }
    }
  uint len = _tcslen(source);
  appendPath(path, 0);
  LPTSTR p = path + _tcslen(path);
  for(uint i = 0; i < len; ++i) {
    if(_T('#') == source[i])
      p[i] = _T('\\');
    else
      p[i] = source[i];
    }
  p[len] = 0;
  if(_tcsicmp(p + len - 4, PAGE_EXT))
    copyStrZ(p + len, PAGE_EXT);

}
//------------------------------------------------------------------------
static void addExtIfNeed(LPTSTR path)
{
#if 0
  int len = _tcslen(path);
  if(len < 4) {
    if(!len)
      _tcscpy_s(path, _MAX_PATH, _T("cartel1.xls"));
    else
    _tcscat_s(path, _MAX_PATH, _T(".xls"));
    return;
    }
  int i;
  int j;
  for(i = len - 4, j = 0; i < len; ++i, ++j)
    if(tolower(path[i]) != _T(".xls")[j])
      break;
  if(i < len)
    _tcscat_s(path, _MAX_PATH, _T(".xls"));
#endif
}
//------------------------------------------------------------------------
bool writePage(BasicExcelWorksheet* sheet, setOfString& set, const PVect<LPTSTR>& lang, bool noSpace);
bool readLanguage(BasicExcelWorksheet* sheet, PVect<LPCTSTR>& lang);
bool readPage(BasicExcelWorksheet* sheet, setOfString& set);
//------------------------------------------------------------------------
unsigned FAR PASCAL ImpProc(void* cl)
{
  manExcel *Cl = reinterpret_cast<manExcel*>(cl);
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, Cl->ExcelFile);
  BasicExcel E;
  addExtIfNeed(path);
  PostMessage(*Cl, WM_CUSTOM_INFO, MAKEWPARAM(0, 1), (LPARAM)getStringOrDef(31, _T("Caricamento file ...")));
#ifdef UNICODE
    {
    char file[_MAX_PATH];
    copyStrZ(file, path);
    if(!E.Load(file)) {
      Cl->idThread = 0;
      PostMessage(*Cl, WM_CUSTOM_INFO, 4, 0);
      return -1;
      }
    }
#else
  if(!E.Load(path)) {
    Cl->idThread = 0;
    PostMessage(*Cl, WM_CUSTOM_INFO, 4, 0);
    return -1;
    }
#endif
  uint nElem = E.GetTotalWorkSheets();
  bool success = true;
  TCHAR name[_MAX_PATH];
  PostMessage(*Cl, WM_CUSTOM_INFO, MAKEWPARAM(0, 2), (LPARAM)getStringOrDef(32, _T("Inizio scrittura pagine")));
  for(uint i = 0; i < nElem; ++i) {
    if(WAIT_TIMEOUT != WaitForSingleObject(Cl->hEventClose, 0)) {
      Cl->idThread = 0;
      PostMessage(*Cl, WM_CUSTOM_INFO, 4, 0);
      return 0;
      }
    BasicExcelWorksheet* sheet = E.GetWorksheet(i);
    if(sheet) {
      _tcscpy_s(path, Cl->basePath);
#ifdef UNICODE
      LPWSTR n = sheet->GetUnicodeSheetName();
      if(n)
        copyStrZ(name, n);
      else {
        LPSTR n2 = sheet->GetAnsiSheetName();
        if(n2)
          copyStrZ(name, n2);
        else
          name[0] = 0;
        }
      copyRestoreSlash(path, name);
#else
      LPSTR n = sheet->GetAnsiSheetName();
      if(n)
        copyStrZ(name, n);
      else {
        LPWSTR n2 = sheet->GetUnicodeSheetName();
        if(n2)
          copyStrZ(name, n2);
        else
          name[0] = 0;
        }
      copyRestoreSlash(path, name);
#endif
      PostMessage(*Cl, WM_CUSTOM_INFO, MAKEWPARAM(0, 0), (LPARAM)name);
      setOfString set;
      if(readPage(sheet, set)) {
        do {
          P_File pf(path);
          pf.appendback();
          } while(false);
        P_File pf(path, P_CREAT);
        if(set.setFirst() && pf.P_open()) {
          do {
            set.writeCurrent(pf, true);
            } while(set.setNext());
          }
        }
      }
    }
  Cl->idThread = 0;
  PostMessage(*Cl, WM_CUSTOM_INFO, 4 - success, 0);
  return 0;
}
//----------------------------------------------------------------------------
static bool isAlarmFile(LPCTSTR pFile)
{
  LPCTSTR noInclude =  _T("alarm")PAGE_EXT;

  if(!_tcsicmp(noInclude, pFile))
    return true;
/**/
  TCHAR noInclude2[] = _T("alarm_");
  if(!_tcsnicmp(noInclude2, pFile, SIZE_A(noInclude2) - 1)) {
    LPCTSTR t = pFile + SIZE_A(noInclude2) - 1;
    if(_istdigit((unsigned)t[0]) && !_tcsnicmp(PAGE_EXT, t + 1, 4))
      return true;
    }
  return false;
}
//------------------------------------------------------------------------
unsigned FAR PASCAL ExpProc(void* cl)
{
  manExcel *Cl = reinterpret_cast<manExcel*>(cl);
  BasicExcel E;
  BasicExcelWorksheet* sheet;
  E.New(1);
  TCHAR path[_MAX_PATH];
  copyRemoveSlash(path, Cl->Filename[0], 1);
  E.RenameWorksheet((size_t)0, path);
  bool success = true;
  while(success) {
    success = false;
    sheet = E.GetWorksheet(path);
    if(!sheet)
      break;
    PostMessage(*Cl, WM_CUSTOM_INFO, MAKEWPARAM(0, 0), (LPARAM)Cl->Filename[0]);
    _tcscpy_s(path, Cl->basePath);
    appendPath(path, Cl->Filename[0]);
    setOfString set(path);
    if(!writePage(sheet, set, Cl->Lang, isAlarmFile(Cl->Filename[0])))
      break;
    uint i;
    uint nElem = Cl->Filename.getElem();
    for(i = 1; i < nElem; ++i) {
      if(WAIT_TIMEOUT != WaitForSingleObject(Cl->hEventClose, 0)) {
        Cl->idThread = 0;
        return 0;
        }

      PostMessage(*Cl, WM_CUSTOM_INFO, MAKEWPARAM(0, 0), (LPARAM)Cl->Filename[i]);
      copyRemoveSlash(path, Cl->Filename[i], i + 1);
      sheet = E.AddWorksheet(path);
      if(!sheet)
        break;
      _tcscpy_s(path, Cl->basePath);
      appendPath(path, Cl->Filename[i]);
      setOfString set(path);
      if(!writePage(sheet, set, Cl->Lang, isAlarmFile(Cl->Filename[i])))
        break;
      }
    if(i < nElem)
      break;
    _tcscpy_s(path, Cl->ExcelFile);
    addExtIfNeed(path);
    PostMessage(*Cl, WM_CUSTOM_INFO, MAKEWPARAM(0, 1), (LPARAM)getStringOrDef(33, _T("Salvataggio su file ...")));
#ifdef UNICODE
    char file[_MAX_PATH];
    copyStrZ(file, path);
    E.SaveAs(file);
#else
    E.SaveAs(path);
#endif
    success = true;
    break;
    }
  Cl->idThread = 0;
  PostMessage(*Cl, WM_CUSTOM_INFO, 2 - success, 0);
  return 0;
 }
