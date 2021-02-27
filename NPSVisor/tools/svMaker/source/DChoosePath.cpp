//------- dChoosePath.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <shlobj.h>
#include <commdlg.h>
//----------------------------------------------------------------------------
#include "dChoosePath.h"
#include "macro_utils.h"
#include "pOpensave.h"
#include "common.h"
#include "sizer.h"
#include "dManageStdMsg.h"
#include "POwnBtnImageStd.h"
//----------------------------------------------------------------------------
static int lastRes = -1;
static SIZE lastSz = { 0, 0 };
//----------------------------------------------------------------------------
bool performInitData(PWin* parent, dataProject& data, bool first)
{
  return IDOK == DChoosePath(parent, data, first).modal();
}
//----------------------------------------------------------------------------
static void setResolution(HWND hwnd, int& lastRes, const uint* idc, SIZE& sz)
{
  sizer::eSizer szr[] = {
    sizer::s640x480, sizer::s800x600, sizer::s1024x768, sizer::s1280x1024, sizer::s1440x900,
    sizer::s1600x1200, sizer::s1680x1050, sizer::s1920x1440, sizer::sPersonalized,
    };
  sz.cx = GetDlgItemInt(hwnd, IDC_EDIT_RES_WIDTH, 0, 1);
  sz.cy = GetDlgItemInt(hwnd, IDC_EDIT_RES_HEIGHT, 0, 1);

  for(uint i = 0; i < SIZE_A(szr); ++i)
    if(BST_CHECKED == SendMessage(GetDlgItem(hwnd, idc[i]), BM_GETCHECK, 0, 0)) {
      sizer::setDim(szr[i], &sz, true);
      lastRes = i;
      break;
      }
  sizer::setCurrAsDefault();
}
//----------------------------------------------------------------------------
static void initResolution(HWND hwnd, int lastRes, SIZE sz)
{
  uint idc[] = {
    IDC_RADIOBUTTON__640x480,
    IDC_RADIOBUTTON__800x600,
    IDC_RADIOBUTTON__1024x768,
    IDC_RADIOBUTTON__1280x1024,
    IDC_RADIOBUTTON__1440x900,
    IDC_RADIOBUTTON__1600x1200,
    IDC_RADIOBUTTON__1680x1050,
    IDC_RADIOBUTTON__1920x1440,
    IDC_RADIOBUTTON__RES_PERS,
    };

  int ix = sizer::sPersonalized == lastRes ? SIZE_A(idc) - 1 : lastRes;
  SendMessage(GetDlgItem(hwnd, idc[ix]), BM_SETCHECK, BST_CHECKED, 0);
  SetDlgItemInt(hwnd, IDC_EDIT_RES_WIDTH, sz.cx, 1);
  SetDlgItemInt(hwnd, IDC_EDIT_RES_HEIGHT, sz.cy, 1);
}
//----------------------------------------------------------------------------
DChoosePath::DChoosePath(PWin* parent,  dataProject& data, bool first, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Data(data), First(first), LData(data), notValid(false)
{
  new POwnBtnImageStd(this, IDOK, new PBitmap(IDB_BITMAP_OK, getHInstance()), POwnBtnImageStd::wLeft, true);
  new POwnBtnImageStd(this, IDCANCEL, new PBitmap(IDB_BITMAP_CANC, getHInstance()), POwnBtnImageStd::wLeft, true);
}
//----------------------------------------------------------------------------
DChoosePath::~DChoosePath()
{
  manageCryptPage::setCanQuit();
  destroy();
}
//----------------------------------------------------------------------------
extern void makeStdMsgSet(setOfString& set);
//----------------------------------------------------------------------------
#ifndef ID_RESOLUTION_BASE
  #define ID_RESOLUTION_BASE 23
#endif
//----------------------------------------------------------------------------
void DChoosePath::checkPartFromStdMsg()
{
  GET_TEXT(IDC_EDIT_PATH, Data.newPath);

  setOfString setStd;
  makeStdMsgSet(setStd);
  LPCTSTR p = setStd.getString(7); // parti da nascondere
  uint v = 0;
  if(p) {
    v = _ttoi(p);
    v <<= 1;
    v |= Data.getWhatPage() & 1;
    }
  Data.setWhatPage(v);
  SET_CHECK_SET(IDC_CHECKBOX_HIDE_HEADER, Data.getWhatPage() & 2);
  SET_CHECK_SET(IDC_CHECKBOX_HIDE_FOOTER, Data.getWhatPage() & 4);

  p = setStd.getString(ID_RESOLUTION_BASE);
  if(!p)
    p = _T("2");
  lastRes = _ttoi(p);
  if(sizer::sPersonalized == lastRes) {
    p = findNextParamTrim(p, 3);
    if(p) {
      lastSz.cx = _ttoi(p);
      p = findNextParamTrim(p);
      if(p)
        lastSz.cy = _ttoi(p);
      }
    }
}
//----------------------------------------------------------------------------
void DChoosePath::checkBasePage()
{
  int successCount = 0;
  WIN32_FIND_DATA FindFileData;
  LPCTSTR toFind[] = {
    _T("nps*.exe"),
    _T("prph_*.exe"),
    _T("prph_*.ad7"),
    _T("prph_*.dat"),
    _T("job_base.dat"),
    _T("system\\")FIRST_PAGE,
    _T("system\\")STD_MSG_NAME,
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
    setKeyPath(BASE_PATH, path);
    }
}
//----------------------------------------------------------------------------
extern bool setBorderOffset(DWORD source);
extern bool getBorderOffset(DWORD& target);
//----------------------------------------------------------------------------
bool DChoosePath::create()
{
  if(!baseClass::create())
    return false;

  if(First) {
    ShowWindow(GetDlgItem(*this, IDCANCEL), SW_HIDE);
    checkBasePage();
    }
  getKeyPath(BASE_PATH, Data.newPath);
  SetDlgItemText(*this, IDC_EDIT_PATH, Data.newPath);
  checkPartFromStdMsg();

  checkEnable();
  initResolution(*this, lastRes, lastSz);

  checkUsedResolution();

  DWORD borderOffest = 0;
  getBorderOffset(borderOffest);
  int offsX = (short)LOWORD(borderOffest);
  int offsY = (short)HIWORD(borderOffest);
  SET_INT(IDC_EDIT_BORDER_OFFSET_X, offsX);
  SET_INT(IDC_EDIT_BORDER_OFFSET_Y, offsY);

  manageCryptPage::reset();

  return true;
}
//----------------------------------------------------------------------------
#ifndef ID_RESOLUTION
#define ID_RESOLUTION 99
#endif
#ifndef ID_CODE_PAGE_TYPE
#define ID_CODE_PAGE_TYPE 999999
#endif
#define MAX_FILE_TO_CHECK 20
//----------------------------------------------------------------------------
void DChoosePath::checkAll()
{
  checkPartFromStdMsg();

  checkEnable();

  checkUsedResolution();
};
//----------------------------------------------------------------------------
int cmpiStr(LPCTSTR str1, LPCTSTR str2, size_t len)
{
  for(size_t i = 0; i < len; ++i) {
    int t = _totlower((uint)str1[i]) - _totlower((uint)str2[i]);
    if(t < 0)
      return -1;
    else if(t > 0)
      return 1;
    }
  return 0;
}
//----------------------------------------------------------------------------
void DChoosePath::checkUsedResolution()
{
  GET_TEXT(IDC_EDIT_PATH, Data.newPath);

  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, Data.newPath);
  appendPath(path, Data.systemPath);
  LPTSTR pP = path + _tcslen(path);

  WIN32_FIND_DATA FindFileData;

  uint idc[] = {
    IDC_RADIOBUTTON__640x480,
    IDC_RADIOBUTTON__800x600,
    IDC_RADIOBUTTON__1024x768,
    IDC_RADIOBUTTON__1280x1024,
    IDC_RADIOBUTTON__1440x900,
    IDC_RADIOBUTTON__1600x1200,
    IDC_RADIOBUTTON__1680x1050,
    IDC_RADIOBUTTON__1920x1440,
    IDC_RADIOBUTTON__RES_PERS
    };
  uint curr = 0;
  SIZE sz = { 0, 0 };
  appendPath(path, _T("#")FIRST_PAGE);
  HANDLE hf = FindFirstFile(path, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    *pP = 0;
    appendPath(path, FindFileData.cFileName);
    manageCryptPage mcp;
    bool crypted = mcp.isCrypted(path);
    if(crypted)
      mcp.makePathAndDecrypt(path);
    setOfString sos(path);
    if(crypted)
      mcp.releaseFileDecrypt(path);
    if(!crypted && mcp.isCrypted())
      notValid = true;
    LPCTSTR p = sos.getString(ID_RESOLUTION);

    if(!p || (_T('-') != *p && !_istdigit((unsigned)*p)))
      curr = SIZE_A(idc) - 1;
    else {
      curr = _ttoi(p);
      if(sizer::sPersonalized == curr) {
        p = findNextParamTrim(p);
        if(p) {
          int v = _ttoi(p);
          SET_INT(IDC_EDIT_RES_WIDTH, v);
          p = findNextParamTrim(p);
          if(p) {
            v = _ttoi(p);
            SET_INT(IDC_EDIT_RES_HEIGHT, v);
            }
          }
        curr = SIZE_A(idc) - 1;
        }
      else
        ++curr;
      }
    FindClose(hf);
    }
  else
    curr = SIZE_A(idc) - 1;

  HWND hwnd = GetDlgItem(*this, idc[curr]);
  if(!IsWindowEnabled(hwnd))
    return;
  for(uint i = 0; i < SIZE_A(idc); ++i)
    SendMessage(GetDlgItem(*this, idc[i]), BM_SETCHECK, i == curr ? BST_CHECKED : BST_UNCHECKED, 0);
}
//----------------------------------------------------------------------------
LRESULT DChoosePath::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_PATH:
          chooseFolder(LOWORD(wParam));
          checkAll();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool DChoosePath::makePath(LPCTSTR path, LPCTSTR sub)
{
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, path);
  appendPath(t, sub);
  int len = _tcslen(t);
  appendPath(t, _T("*.*"));
  if(!P_File::P_exist(t)) {
    t[len] = 0;
    if(IDYES != MessageBox(*this, _T("Vuoi creare la cartella?"), t, MB_YESNO | MB_ICONINFORMATION))
      return false;
    createDirectoryRecurse(t);
    }
  return true;
}
//----------------------------------------------------------------------------
void DChoosePath::manageStdMsg()
{
  GET_TEXT(IDC_EDIT_PATH, Data.newPath);
  dManageStdMsg(this).modal();
}
//----------------------------------------------------------------------------
void DChoosePath::makeDirs()
{
  if(!makePath(Data.newPath, 0))
    return;
  if(!makePath(Data.newPath, Data.imagePath))
    return;
  if(!makePath(Data.newPath, Data.systemPath))
    return;
  makePath(Data.newPath, Data.systemDatPath);
}
//----------------------------------------------------------------------------
void DChoosePath::CmCancel()
{
  if(First)
    return;
  Data = LData;
  setKeyPath(BASE_PATH, Data.newPath);

  baseClass::CmCancel();
}
//----------------------------------------------------------------------------
extern
void loadCurrLang(bool& hideTitle);
//----------------------------------------------------------------------------
void DChoosePath::CmOk()
{
  if(notValid) {
    MessageBox(*this, _T("Progetto non abilitato"), _T("Errore"), MB_OK | MB_ICONSTOP);
    CmCancel();
    return;
    }
  GET_TEXT(IDC_EDIT_PATH, Data.newPath);
  makeDirs();
  DWORD what = 0;
  if(IS_CHECKED(IDC_CHECKBOX_HIDE_HEADER))
    what |= 2;
  if(IS_CHECKED(IDC_CHECKBOX_HIDE_FOOTER))
    what |= 4;

  Data.setAllWhatPage(what);

  setKeyPath(BASE_PATH, Data.newPath);

  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, Data.newPath);
  appendPath(t, Data.imagePath);
  setKeyPath(OLD_IMAGE_PATH, t);

  uint idc[] = {
    IDC_RADIOBUTTON__640x480,
    IDC_RADIOBUTTON__800x600,
    IDC_RADIOBUTTON__1024x768,
    IDC_RADIOBUTTON__1280x1024,
    IDC_RADIOBUTTON__1440x900,
    IDC_RADIOBUTTON__1600x1200,
    IDC_RADIOBUTTON__1680x1050,
    IDC_RADIOBUTTON__1920x1440,
    IDC_RADIOBUTTON__RES_PERS
    };
  setResolution(*this, lastRes, idc, lastSz);

  loadCurrLang(Data.hideTitle);

  int offsX;
  int offsY;
  GET_INT(IDC_EDIT_BORDER_OFFSET_X, offsX);
  GET_INT(IDC_EDIT_BORDER_OFFSET_Y, offsY);
  DWORD borderOffest = MAKEWPARAM(offsX, offsY);
  setBorderOffset(borderOffest);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void DChoosePath::checkEnable()
{
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool DChoosePath::chooseFolder(int idCtrl)
{
  struct linkId {
    int idEd;
    int idBtn;
    LPCTSTR key;
    bool onlySub;
    };
  const linkId ids[] = {
    { IDC_EDIT_PATH,
      IDC_BUTTON_PATH,
      BASE_PATH,
      false
    }
  };
  bool success = false;
  for(uint i = 0; i < SIZE_A(ids); ++i) {
    if(ids[i].idBtn == idCtrl) {
      TCHAR buff[_MAX_PATH];
      GET_TEXT(ids[0].idEd, buff);
      if(findFolder(ids[i].key, buff, _T("Scelta Cartella"), ids[i].onlySub)) {
        SetDlgItemText(*this, ids[i].idEd, buff);
        success = true;
        if(IDC_BUTTON_PATH == idCtrl) {
          manageCryptPage::reset();
          notValid = false;
          }
        if(i <= 1)
          checkUsedResolution();
        }
      break;
      }
    }
  return success;

}
//----------------------------------------------------------------------------
openPageFile::openPageFile(PWin* parent, dataProject& data, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Data(data)
{
}
//----------------------------------------------------------------------------
openPageFile::~openPageFile()
{
  destroy();
}
//----------------------------------------------------------------------------
bool openPageFile::create()
{
  if(!baseClass::create())
    return false;

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, Data.newPath);
  appendPath(path, Data.systemPath);
  fillLBWithPageFile(hlb, path, Data.getSubPath());
  setTitlePath(*this, Data.getSubPath());
  if(*Data.getPageName()) {
    TCHAR onlyName[_MAX_PATH];
    _tcscpy_s(onlyName, Data.getPageName());
    int len = _tcslen(onlyName);
    for(int i = len - 1; i >= 0; --i)
      if(_T('.') == onlyName[i]) {
        onlyName[i] = 0;
        break;
        }
    int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
    if(count > 0) {
      for(int i = 0; i < count; ++i) {
        SendMessage(hlb, LB_GETTEXT, i, (LPARAM)path);
        if(!_tcsicmp(path, onlyName)) {
          SendMessage(hlb, LB_SETCURSEL, i, 0);
          break;
          }
        }
      }
    }
  else
    SendMessage(hlb, LB_SETCURSEL, 0, 0);

  if(Data.getWhatPage() & 1) {
    SET_CHECK_UNCHECK(IDC_RADIOBUTTON_HEADER, IDC_RADIOBUTTON_BODY);
    }
  else {
    SET_CHECK_UNCHECK(IDC_RADIOBUTTON_BODY, IDC_RADIOBUTTON_HEADER);
    if(Data.getWhatPage() & 2)
      SET_CHECK(IDC_CHECKBOX_HIDE_HEADER);
    if(Data.getWhatPage() & 4)
      SET_CHECK(IDC_CHECKBOX_HIDE_FOOTER);
    }
  checkEnable();
  checkType(true);
  return true;
}
//----------------------------------------------------------------------------
LRESULT openPageFile::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIOBUTTON_HEADER:
        case IDC_RADIOBUTTON_BODY:
          checkEnable();
          break;
        case IDC_BUTTON_OPEN_SOURCE:
          openSource();
          CmCancel();
          break;
        }
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          CmOk();
          break;
        case LBN_SELCHANGE:
          checkType(true);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
extern void openPageFileSource(PWin* parent, LPCTSTR filename, dataProject& data);
//----------------------------------------------------------------------------
void openPageFile::openSource()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[_MAX_PATH];
  SendMessage(hlb, LB_GETTEXT, sel, (LPARAM) buff);
  if(!*buff || _T('<') == *buff)
    return;
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, Data.newPath);
  appendPath(path, Data.systemPath);
  appendPath(path, Data.getSubPath());
  openPageFileSource(this, buff, Data);
}
//----------------------------------------------------------------------------
static bool re_Path(HWND hwlb, dataProject& data, LPTSTR target)
{
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, data.newPath);
  appendPath(path, data.systemPath);
  TCHAR subpath[_MAX_PATH];
  _tcscpy_s(subpath, data.getSubPath());
  if(rePath(hwlb, subpath, path, target)) {
    data.setSubPath(subpath);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
static uint idcPageType[] = {
    IDC_RADIOBUTTON_P_STANDARD,
    IDC_RADIOBUTTON_P_RECIPE,
    IDC_RADIOBUTTON__P_RECIPE_LB,
    IDC_RADIOBUTTON_P_RECIPE_ROW,
    IDC_RADIOBUTTON_P_GRAPH_TREND,
    };
//----------------------------------------------------------------------------
void openPageFile::CmOk()
{
  for(uint i = 0; i < SIZE_A(idcPageType); ++i) {
    if(IS_CHECKED(idcPageType[i])) {
      Data.setPageType(i);
      break;
      }
    }

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);

  TCHAR buff[_MAX_PATH];
  if(re_Path(hlb, Data, buff))
    return;
  _tcscat_s(buff, PAGE_EXT);
  Data.setPageName(buff);

  DWORD what = 0;
  if(IS_CHECKED(IDC_RADIOBUTTON_HEADER))
    what = 1;
  else {
    if(IS_CHECKED(IDC_CHECKBOX_HIDE_HEADER))
      what |= 2;
    if(IS_CHECKED(IDC_CHECKBOX_HIDE_FOOTER))
      what |= 4;
    }
  Data.setWhatPage(what);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void openPageFile::checkEnable()
{
  bool enable = IS_CHECKED(IDC_RADIOBUTTON_BODY);
  ENABLE(IDC_CHECKBOX_HIDE_HEADER, enable);
  ENABLE(IDC_CHECKBOX_HIDE_FOOTER, enable);
  if(!enable) {
    for(uint i = 0; i < SIZE_A(idcPageType); ++i)
      ENABLE(idcPageType[i], false);
    }
  else
    checkType(false);
}
//----------------------------------------------------------------------------
void loadOneSet(setOfString& set, dataProject& datap, LPCTSTR filename)
{
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, datap.newPath);
  appendPath(path, datap.systemPath);
  appendPath(path, datap.getSubPath());
  appendPath(path, filename);
  _tcscat_s(path, PAGE_EXT);

  manageCryptPage mcp;
  bool crypted = mcp.isCrypted(path);
  if(crypted)
    mcp.makePathAndDecrypt(path);
  set.add(path);
  if(crypted)
    mcp.releaseFileDecrypt(path);
}
//----------------------------------------------------------------------------
static void loadSet(setOfString& set, dataProject& datap, LPCTSTR filename)
{
  loadOneSet(set, datap, filename);
  TCHAR file[_MAX_PATH] = _T("#");
  _tcscat_s(file, filename);
  loadOneSet(set, datap, file);
}
//----------------------------------------------------------------------------
void openPageFile::checkType(bool chgFile)
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[_MAX_PATH];
  SendMessage(hlb, LB_GETTEXT, sel, (LPARAM) buff);
  if(!*buff || _T('<') == *buff)
    return;
  setOfString sos;
  loadSet(sos, Data, buff);
  LPCTSTR p = sos.getString(ID_CODE_PAGE_TYPE);
  int code = -1;
  if(p)
    code = _ttoi(p);

  if(chgFile) {
    p = findNextParam(p, 1);
    int what = 0;
    if(p)
      what = _ttoi(p);
    if(-1 == what) {
      SET_CHECK_SET(IDC_RADIOBUTTON_BODY, 0);
      SET_CHECK(IDC_RADIOBUTTON_HEADER);
      }
    else {
      SET_CHECK(IDC_RADIOBUTTON_BODY);
      SET_CHECK_SET(IDC_RADIOBUTTON_HEADER, 0);
      SET_CHECK_SET(IDC_CHECKBOX_HIDE_HEADER, 1 & what);
      SET_CHECK_SET(IDC_CHECKBOX_HIDE_FOOTER, 2 & what);
      }
    checkEnable();
    }

  for(uint i = 0; i < SIZE_A(idcPageType); ++i) {
    SET_CHECK_SET(idcPageType[i], code == i);
    ENABLE(idcPageType[i], -1 == code || code == i);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
getPageFile::getPageFile(PWin* parent,  LPTSTR target, bool simpleOpen, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Target(target), simpleOpen(simpleOpen)
{
}
//----------------------------------------------------------------------------
getPageFile::~getPageFile()
{
  destroy();
}
//----------------------------------------------------------------------------
bool getPageFile::create()
{
  if(!baseClass::create())
    return false;

  if(simpleOpen) {
    PRect r;
    GetWindowRect(*this, r);
    HWND hr = GetDlgItem(*this, IDC_GROUPBOX1);
    PRect r2;
    GetWindowRect(hr, r2);
    r.right = r2.left;
    SetWindowPos(*this, 0, 0, 0, r.Width(), r.Height(), SWP_NOZORDER | SWP_NOMOVE);
    SetWindowText(*this, _T("Scelta File"));
    }

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);
  TCHAR path[_MAX_PATH];
  const dataProject& Data = getDataProject();

  _tcscpy_s(path, Data.newPath);
  appendPath(path, Data.systemPath);
  fillLBWithPageFile(hlb, path, Data.getSubPath());

  setTitlePath(*this, Data.getSubPath());
  if(*Target) {
    TCHAR onlyName[_MAX_PATH];
    _tcscpy_s(onlyName, Target);
    int len = _tcslen(onlyName);
    for(int i = len - 1; i >= 0; --i)
      if(_T('.') == onlyName[i]) {
        onlyName[i] = 0;
        break;
        }

    SetDlgItemText(*this, IDC_EDIT_PAGEFILE, onlyName);

    int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
    if(count > 0) {
      for(int i = 0; i < count; ++i) {
        SendMessage(hlb, LB_GETTEXT, i, (LPARAM)path);
        if(!_tcsicmp(path, onlyName)) {
          SendMessage(hlb, LB_SETCURSEL, i, 0);
          break;
          }
        }
      }
    }
  else
    SendMessage(hlb, LB_SETCURSEL, -1, 0);

  checkEnable();
  return true;
}
//----------------------------------------------------------------------------
LRESULT getPageFile::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case LBN_SELCHANGE:
          lbToEdit();
          break;

        case LBN_DBLCLK:
          LB_DblClick();
          break;
        case EN_CHANGE:
          checkEnable();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void getPageFile::LB_DblClick()
{
//  const
  dataProject& Data = getDataProject();
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);

  TCHAR buff[_MAX_PATH];
  if(re_Path(hlb, Data, buff))
    return;
  CmOk();
}
//----------------------------------------------------------------------------
void getPageFile::CmOk()
{
  TCHAR buff[_MAX_PATH];
  GET_TEXT(IDC_EDIT_PAGEFILE, buff);
  if(!buff)
    return;
//  const
  dataProject& Data = getDataProject();
  if(*Data.getSubPath()) {
    _tcscpy_s(Target, _MAX_PATH, Data.getSubPath());
    appendPath(Target, buff);
    }
  else
    _tcscpy_s(Target, _MAX_PATH, buff);
  _tcscat_s(Target, _MAX_PATH, PAGE_EXT);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void getPageFile::lbToEdit()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[_MAX_PATH];
  SendMessage(hlb, LB_GETTEXT, sel, (LPARAM) buff);
  if(_T('<') == *buff)
    return;
  SET_TEXT(IDC_EDIT_PAGEFILE, buff);
}
//----------------------------------------------------------------------------
void getPageFile::checkEnable()
{
  bool enable = toBool(GetWindowTextLength(GetDlgItem(*this, IDC_EDIT_PAGEFILE)));
  ENABLE(IDOK, enable);
}
//----------------------------------------------------------------------------
TCHAR savePageFile::file[_MAX_PATH] = _T("\0");
//----------------------------------------------------------------------------
savePageFile::savePageFile(PWin* parent,  dataProject& data, bool simpleOpen, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, file, simpleOpen, resId, hinstance), Data(data)
{
    _tcscpy_s(file, data.getPageName());
}
//----------------------------------------------------------------------------
savePageFile::~savePageFile()
{
  destroy();
}
//----------------------------------------------------------------------------
bool savePageFile::create()
{
  if(!baseClass::create())
    return false;
  checkType();
  enableChange();
  int sel = Data.getPageType();
  SET_CHECK(idcPageType[sel]);

  return true;
}
//----------------------------------------------------------------------------
void savePageFile::CmOk()
{
  TCHAR buff[_MAX_PATH];
  GET_TEXT(IDC_EDIT_PAGEFILE, buff);
  if(!buff)
    return;
  _tcscat_s(buff, PAGE_EXT);
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, Data.newPath);
  appendPath(path, Data.systemPath);
  appendPath(path, Data.getSubPath());
  appendPath(path, buff);

  if(P_File::P_exist(path)) {
    int result = MessageBox(*this, _T("Il file esiste,\r\nvuoi sovrascriverlo?"),
        _T("Warning"), MB_YESNOCANCEL | MB_ICONINFORMATION);
    switch(result) {
      case IDYES:
        break;

      case IDNO:
        CmCancel();
        // fall throught

      case IDCANCEL:
        return;
      }
    }
  if(_tcschr(buff, _T('\\'))) {
    TCHAR buff2[_MAX_PATH];
    _tcscpy_s(buff2, buff);
    dirName(buff2);
    _tcscpy_s(path, Data.getSubPath());
    appendPath(path, buff2);
    Data.setSubPath(path);
    _tcscpy_s(buff2, Data.newPath);
    appendPath(buff2, Data.systemPath);
    appendPath(buff2, path);
    createDirectoryRecurse(buff2);
    int l = _tcslen(buff);
    for(int i = l - 1; i; --i) {
      if(_T('\\') == buff[i]) {
        Data.setPageName(buff + i + 1);
        break;
        }
      }
    }
  else
    Data.setPageName(buff);

  if(IS_CHECKED(IDC_CHECK_FORCE_CHANGE)) {
    for(uint i = 0; i < SIZE_A(idcPageType); ++i) {
      if(IS_CHECKED(idcPageType[i])) {
        Data.setPageType(i);
        break;
        }
      }

    DWORD what = 0;
    if(IS_CHECKED(IDC_RADIOBUTTON_HEADER))
      what = 1;
    else {
      if(IS_CHECKED(IDC_CHECKBOX_HIDE_HEADER))
        what |= 2;
      if(IS_CHECKED(IDC_CHECKBOX_HIDE_FOOTER))
        what |= 4;
      }
    Data.setWhatPage(what);
    }
  // N.B. non deve chiamare la classe base
  PModDialog::CmOk();
}
//----------------------------------------------------------------------------
LRESULT savePageFile::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECK_FORCE_CHANGE:
          enableChange();
          break;

        case IDC_RADIOBUTTON_HEADER:
        case IDC_RADIOBUTTON_BODY:
          checkEnableChg();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void savePageFile::enableChange()
{
  bool enable = IS_CHECKED(IDC_CHECK_FORCE_CHANGE);
  for(uint i = 0; i < SIZE_A(idcPageType); ++i)
    ENABLE(idcPageType[i], enable);
  ENABLE(IDC_RADIOBUTTON_HEADER, enable);
  ENABLE(IDC_RADIOBUTTON_BODY, enable);
  ENABLE(IDC_CHECKBOX_HIDE_HEADER, enable);
  ENABLE(IDC_CHECKBOX_HIDE_FOOTER, enable);
  if(enable)
    checkEnableChg();
}
//----------------------------------------------------------------------------
void savePageFile::checkEnableChg()
{
  bool enable = IS_CHECKED(IDC_RADIOBUTTON_BODY);
  ENABLE(IDC_CHECKBOX_HIDE_HEADER, enable);
  ENABLE(IDC_CHECKBOX_HIDE_FOOTER, enable);
  for(uint i = 0; i < SIZE_A(idcPageType); ++i)
    ENABLE(idcPageType[i], enable);
}
//----------------------------------------------------------------------------
void savePageFile::checkType()
{
  DWORD what = Data.getWhatPage();
  if(1 & what) {
    SET_CHECK_SET(IDC_RADIOBUTTON_BODY, 0);
    SET_CHECK(IDC_RADIOBUTTON_HEADER);
    }
  else {
    SET_CHECK(IDC_RADIOBUTTON_BODY);
    SET_CHECK_SET(IDC_RADIOBUTTON_HEADER, 0);
    SET_CHECK_SET(IDC_CHECKBOX_HIDE_HEADER, 2 & what);
    SET_CHECK_SET(IDC_CHECKBOX_HIDE_FOOTER, 4 & what);
    }
}
//----------------------------------------------------------------------------
saveSubPath::saveSubPath()
{
  dataProject& Data = getDataProject();
  _tcscpy_s(old, Data.getSubPath());
}
//----------------------------------------------------------------------------
void saveSubPath::restore()
{
  dataProject& Data = getDataProject();
  Data.setSubPath(old);
}
