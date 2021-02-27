//----------- pWizardChild5.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include <lm.h>
//----------------------------------------------------------------------------
#include "wizardDlg.h"
#include "pWizardChild5.h"
#include "common.h"
#include "svmObject.h"
#include "svmPropertyBtn.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MB_BTN (MB_OK | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
pWizardChild5::pWizardChild5(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{
}
//----------------------------------------------------------------------------
pWizardChild5::~pWizardChild5()
{
  destroy();
}
//----------------------------------------------------------------------------
bool pWizardChild5::create()
{
  if(!baseClass::create())
    return false;

  fillCBPerif(GetDlgItem(*this, IDC_COMBO_WZ_LKP_PRPH), 0);
  fillCBTypeVal(GetDlgItem(*this, IDC_COMBO_WZ_LKP_TYPE_PRPH), 0);

  SET_CHECK(IDC_CHECK_WZ_LKP_MODAL);
  checkEnable();
  return true;
}
//----------------------------------------------------------------------------
LRESULT pWizardChild5::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_LNK_CHOOSE_PAGE:
          chooseFile(IDC_EDIT_WZ_LKP_LINKED_PAGE);
          break;
        case IDC_BUTTON_LNK_CHOOSE_PAGE_BTN:
          chooseFile(IDC_EDIT_WZ_LKP_BTN_PAGE);
          break;
        case IDC_CHECK_WZ_LKP_MODAL:
          checkEnable();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void pWizardChild5::checkEnable()
{
  uint ids[] = { IDC_COMBO_WZ_LKP_PRPH, IDC_EDT_WZ_LKP_ADDR_PRPH, IDC_COMBO_WZ_LKP_TYPE_PRPH,
        IDC_EDT_WZ_LKP_NBIT, IDC_EDT_WZ_LKP_OFFS, IDC_EDIT_WZ_LKP_BTN_PAGE,
        IDC_CHECK_WZ_PLK_OPEN_PAGE, IDC_BUTTON_LNK_CHOOSE_PAGE_BTN };
  bool enable = IS_CHECKED(IDC_CHECK_WZ_LKP_MODAL);
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], enable);
}
//----------------------------------------------------------------------------
#define NAME_WIZ5 _T("Pagine Linkate")
//----------------------------------------------------------------------------
void pWizardChild5::getTitle(LPTSTR append, size_t lenBuff)
{
  _tcscpy_s(append, lenBuff, NAME_WIZ5);
}
//----------------------------------------------------------------------------
void pWizardChild5::chooseFile(uint idcEdit)
{
  TCHAR file[_MAX_PATH];
  GET_TEXT(idcEdit, file);
  if(IDOK == openGenFile(this, file, false, _T("*")PAGE_EXT).modal())
    SET_TEXT(idcEdit, file);
}
//----------------------------------------------------------------------------
static uint getLastBtn(LPCTSTR file)
{
  setOfString set(file);
  for(uint i = 0;; ++i) {
    manageObjId moi(0, ID_INIT_VAR_BTN);
    moi.calcAndSetExtendId(i);
    if(!set.getString(moi.getId()) && !set.getString(moi.getFirstExtendId()))
      return moi.getId();
    }
  return ID_INIT_VAR_BTN;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class createPageFile
{
  public:
    createPageFile(PWin* owner, uint ix, LPCTSTR path, LPCTSTR prefix) :
      Owner(owner), Ix(ix), Path(path), Prefix(prefix) {}
    actionChoose runClear(LPCTSTR title, actionChoose act);
    actionChoose runSharp(LPCTSTR pageBase, int offs, actionChoose act);
  private:
    PWin* Owner;
    uint Ix;
    LPCTSTR Path;
    LPCTSTR Prefix;
    void makeFullPath(LPTSTR buff, bool sharp);
    void makeFilename(LPTSTR buff, bool sharp);
    actionChoose runCheck(bool sharp, actionChoose act);
};
//----------------------------------------------------------------------------
void createPageFile::makeFullPath(LPTSTR buff, bool sharp)
{
  if(sharp)
    wsprintf(buff, _T("%s\\#%s%d")PAGE_EXT, Path, Prefix, Ix);
  else
    wsprintf(buff, _T("%s\\%s%d")PAGE_EXT, Path, Prefix, Ix);
}
//----------------------------------------------------------------------------
void createPageFile::makeFilename(LPTSTR buff, bool sharp)
{
  if(sharp)
    wsprintf(buff, _T("#%s%d")PAGE_EXT, Prefix, Ix);
  else
    wsprintf(buff, _T("%s%d")PAGE_EXT, Prefix, Ix);
}
//----------------------------------------------------------------------------
actionChoose createPageFile::runCheck(bool sharp, actionChoose act)
{
  if(eacApplyAll == act)
    return eacApplyAll;
  act = eacApply;
  TCHAR buff[_MAX_PATH];
  makeFullPath(buff, sharp);
  if(P_File::P_exist(buff)) {
    makeFilename(buff, sharp);
    act = getActionChoose(Owner, act, buff);
    }
  return act;
}
//----------------------------------------------------------------------------
actionChoose createPageFile::runClear(LPCTSTR title, actionChoose act)
{
  act = runCheck(false, act);
  switch(act) {
    case eacApply:
    case eacApplyAll:
      break;
    default:
      return act;
    }

  TCHAR buff[_MAX_PATH];
  makeFullPath(buff, false);
  do {
    P_File pf(buff);
    pf.appendback();
    } while(false);

  P_File pf(buff, P_CREAT);
  if(!pf.P_open())
    return eacNull;
  wsprintf(buff, _T("1,%s%d\n\r"), title, Ix);
  return pf.P_writeString(buff) > 0 ? act : eacNull;
}
//----------------------------------------------------------------------------
actionChoose createPageFile::runSharp(LPCTSTR pageBase, int offs, actionChoose act)
{
  act = runCheck(true, act);
  switch(act) {
    case eacApply:
    case eacApplyAll:
      break;
    default:
      return act;
    }

  TCHAR buff[_MAX_PATH];
  makeFullPath(buff, true);
  do {
    P_File pf(buff);
    pf.appendback();
    } while(false);

  P_File pf(buff, P_CREAT);
  if(!pf.P_open())
    return eacNull;
  wsprintf(buff, _T("28,%d,"), offs * Ix);
  if(!pf.P_writeString(buff))
    return eacNull;
  if(!pf.P_writeString(translateFromCRNL(buff, pageBase)))
    return eacNull;
  if(!pf.P_writeString(PAGE_EXT _T("\n\r")))
    return eacNull;
  uint x = 10 + Ix * 10;
  uint y = 10 + Ix % 10 * 30;
  wsprintf(buff, _T("33,%d,%d,-1,-1\n\r"), x, y);
  return pf.P_writeString(buff) > 0 ? act : eacNull;
}
//----------------------------------------------------------------------------
actionChoose createPageClear(PWin* w, uint ix, LPCTSTR path, LPCTSTR prefix, LPCTSTR title, actionChoose act)
{
   createPageFile cpf(w, ix, path, prefix);
   return cpf.runClear(title, act);
}
//----------------------------------------------------------------------------
actionChoose createPageSharp(PWin* w, uint ix, LPCTSTR path, LPCTSTR prefix, LPCTSTR pageBase, int offs, actionChoose act)
{
   createPageFile cpf(w, ix, path, prefix);
   return cpf.runSharp(pageBase, offs, act);
}
//----------------------------------------------------------------------------
static uint getLastMsg(setOfString& set, uint nPage)
{
  if(!nPage)
    nPage = 1;
  for(uint id = FIRST_ID_MSG; id < FIRST_ID_MSG + 100000; ++id) {
    if(!set.getString(id)) {
      bool foundEmpty = true;
      uint j = 1;
      for(; j <= nPage; ++j) {
        if(set.getString(id + j)) {
          foundEmpty = false;
          break;
          }
        }
      if(!foundEmpty) {
        id += j;
        continue;
        }
      return id + 1;
      }
    }
  return FIRST_ID_MSG;
}
//----------------------------------------------------------------------------
static bool saveSet(setOfString& set, LPCTSTR file)
{
  do {
    P_File pf(file);
    pf.appendback();
    } while(false);
  P_File pf(file, P_CREAT);
  if(!pf.P_open())
    return false;

  if(!set.setFirst())
    return true;
  do {
    if(!set.writeCurrent(pf, true))
      return false;
    } while(set.setNext());
  return true;
}
//----------------------------------------------------------------------------
static void makeFileName(LPTSTR path)
{
  int l = _tcslen(path);
  for(int i = l - 1; i > 0; --i) {
    if(_T('\\') == path[i]) {
      for(int j = 0; j < i; ++j)
        path[j] = path[j + i];
      path[i] = 0;
      }
    }
}
//----------------------------------------------------------------------------
static bool makeDirName(LPTSTR path)
{
  int l = _tcslen(path);
  for(int i = l - 1; i > 0; --i)
    if(_T('\\') == path[i]) {
      path[i] = 0;
      return true;
      }
  return false;
}
//----------------------------------------------------------------------------
static void makeFilenamePath(LPTSTR target, LPCTSTR path, LPCTSTR file, bool sharp)
{
  _tcscpy_s(target, _MAX_PATH, path);
  TCHAR tFile[_MAX_PATH];
  _tcscpy_s(tFile, file);
  if(makeDirName(tFile)) {
    appendPath(target, tFile);
    createDirectoryRecurse(target);
    _tcscpy_s(tFile, file);
    makeFileName(tFile);
    }
  if(sharp) {
    appendPath(target, _T("#"));
    _tcscat_s(target, _MAX_PATH, tFile);
    }
  else
    appendPath(target, tFile);
  _tcscat_s(target, _MAX_PATH, PAGE_EXT);
}
//----------------------------------------------------------------------------
extern void makeVersion(LPTSTR p);
//----------------------------------------------------------------------------
static bool addBtn(LPCTSTR file, LPCTSTR path, uint prph, uint addr, uint type, uint nbit, uint offs, uint offsAddr, uint nPage, LPCTSTR title, LPCTSTR lnkpage)
{
  TCHAR buff[_MAX_PATH];
  makeFilenamePath(buff, path, file, true);
  setOfString setSharp(buff);

  if(!setSharp.setFirst()) {
    makeVersion(buff);
    setSharp.replaceString(ID_VERSION, str_newdup(buff), true, true);
//    setSharp.replaceString(ID_RESOLUTION, str_newdup(_T("0")), true, true);
    setSharp.replaceString(ID_CODE_PAGE_TYPE, str_newdup(_T("0,3")), true, true);
    }

  makeFilenamePath(buff, path, file, false);
  setOfString setClear(buff);
  uint idMsg = getLastMsg(setClear, nPage);

  LPCTSTR p = setSharp.getString(ID_VAR_BTN);
  uint nbtn = p ? _ttoi(p) : 0;
  wsprintf(buff, _T("%d"), nPage ? (nbtn + nPage) : nbtn + 1);
  setSharp.replaceString(ID_VAR_BTN, str_newdup(buff), true, true);

  uint typeopen = nPage ? btnModeless : btnOnOff;
  uint x = 5 + rand() % 10;
  uint y = 5 + rand() % 10;
  uint negative = nPage ? 0 : 1;
  for(uint i = 0; i < nPage || negative; ++i) {
    manageObjId moi(0, ID_INIT_VAR_BTN);
    uint id = moi.calcAndSetExtendId(nbtn + i);

    wsprintf(buff, _T("%d,%d,50,24,%d,%d,0,192,192,192,0,192,0,0,0,0,0,0,0,%d,%d,%d"),
            x, y, btnS_StdAndBmp, typeopen, ID_INIT_FONT, idMsg + i, idMsg + i);
    setSharp.replaceString(id, str_newdup(buff), true, true);

    id = moi.getFirstExtendId();
    wsprintf(buff, _T("%d,%d,%d,0,%d"), prph, addr + offsAddr * i, type, negative);
    setSharp.replaceString(id, str_newdup(buff), true, true);

    id = moi.getSecondExtendId();
    wsprintf(buff, _T("%d,%d,0,0,0,0,0,0"), nbit, offs);
    setSharp.replaceString(id, str_newdup(buff), true, true);

    if(nPage) {
      id = moi.getThirdExtendId();
      wsprintf(buff, _T("0,%s%d")PAGE_EXT, lnkpage, i);
      setSharp.replaceString(id, str_newdup(buff), true, true);
      }
    else
      break;
    x += 60;
    if(x >= 610) {
      x = 10;
      y += 30;
      }
    }
  if(nPage) {
    for(uint i = 0; i < nPage; ++i, ++idMsg) {
      wsprintf(buff, _T("%d"), i);
//      wsprintf(buff, _T("%s%d"), title, i);
      setClear.replaceString(idMsg, str_newdup(buff), true, true);
      }
    }
  else {
//    setClear.replaceString(idMsg, str_newdup(title), true, true);
    setClear.replaceString(idMsg, str_newdup(_T("Chiudi")), true, true);
    }
  makeFilenamePath(buff, path, file, true);
  if(!saveSet(setSharp, buff))
    return false;

  makeFilenamePath(buff, path, file, false);
  if(!saveSet(setClear, buff))
    return false;
  return true;
}
//----------------------------------------------------------------------------
#define MAX_PAGE_TO_CREATE 200
//----------------------------------------------------------------------------
bool pWizardChild5::save(setOfString& Set)
{
/**/
  uint nPage;
  srand(GetTickCount());
  GET_INT(IDC_EDT_WZ_LKP_NUM_FILE, nPage);
  if(!nPage || nPage > MAX_PAGE_TO_CREATE)
    return false;
  int offsAddr;
  GET_INT(IDC_EDT_WZ_LKP_ADDR_OFFS, offsAddr);
  if(!offsAddr)
    return false;
  bool isModeless = IS_CHECKED(IDC_CHECK_WZ_LKP_MODAL);
  smartPointerString path(new TCHAR[_MAX_PATH], true);
  const dataProject& dP = getDataProject();
  _tcscpy_s(path, _MAX_PATH, dP.newPath);
  appendPath(path, dP.systemPath);
  TCHAR buff[_MAX_PATH];
  GET_TEXT(IDC_EDIT_WZ_LKP_TITLE, buff);
  if(!*buff)
    return false;
  smartPointerConstString title(str_newdup(buff), true);

  GET_TEXT(IDC_EDIT_WZ_LKP_PREFIX, buff);
  lTrim(trim(buff));
  if(!*buff)
    return false;
  smartPointerConstString prefix(str_newdup(buff), true);

  GET_TEXT(IDC_EDIT_WZ_LKP_LINKED_PATH, buff);
  lTrim(trim(buff));
  LPTSTR p = path + _tcslen(path);
  if(*buff) {
    appendPath(path, buff);
    createDirectoryRecurse(path);
    *p = 0;
    }
  GET_TEXT(IDC_EDIT_WZ_LKP_BTN_PAGE, buff);
  trim(lTrim(buff));
  smartPointerConstString btnpage(str_newdup(buff), true);
  if(isModeless) {
    uint prph = SendMessage(GetDlgItem(*this, IDC_COMBO_WZ_LKP_PRPH), CB_GETCURSEL, 0, 0);
    if(!prph)
      return false;
    uint type = SendMessage(GetDlgItem(*this, IDC_COMBO_WZ_LKP_TYPE_PRPH), CB_GETCURSEL, 0, 0);
    if(!type)
      return false;

    GET_TEXT(IDC_EDIT_WZ_LKP_LINKED_PAGE, buff);
    lTrim(trim(buff));
//    smartPointerConstString lnkpage(str_newdup(buff), true);

    uint addr;
    GET_INT(IDC_EDT_WZ_LKP_ADDR_PRPH, addr);
    uint nbit;
    GET_INT(IDC_EDT_WZ_LKP_NBIT, nbit);
    uint offs;
    GET_INT(IDC_EDT_WZ_LKP_OFFS, offs);

    if(!addBtn(buff, path, prph, addr, type, nbit, offs, offsAddr, 0, title, 0))
      return false;

    GET_TEXT(IDC_EDIT_WZ_LKP_LINKED_PATH, buff);
    lTrim(trim(buff));
    appendPath(buff, prefix);
    if(!addBtn(btnpage, path, prph, addr, type, nbit, offs, offsAddr, nPage, title, buff))
      return false;
    }
  GET_TEXT(IDC_EDIT_WZ_LKP_LINKED_PATH, buff);
  lTrim(trim(buff));
  appendPath(path, buff);
  GET_TEXT(IDC_EDIT_WZ_LKP_LINKED_PAGE, buff);
  lTrim(trim(buff));
  if(!*buff)
    return false;
  actionChoose act = eacApply;
  for(uint i = 0; i < nPage; ++i) {
    act = createPageClear(this, i, path, prefix, title, act);
    if(eacNull == act)
      return false;
    if(eacSkipAll == act)
      break;
    act = createPageSharp(this, i, path, prefix, buff, offsAddr, act);
    if(eacNull == act)
      return false;
    if(eacSkipAll == act)
      break;
    }
  if(isModeless && IS_CHECKED(IDC_CHECK_WZ_PLK_OPEN_PAGE)) {
    _tcscpy_s(path, _MAX_PATH, dP.newPath);
    appendPath(path, dP.systemPath);
    makeFilenamePath(buff, path, btnpage, true);
    Set.add(buff);
    makeFilenamePath(buff, path, btnpage, false);
    Set.add(buff);
    }
  return true;
}
//----------------------------------------------------------------------------
