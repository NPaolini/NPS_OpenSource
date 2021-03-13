//---------- recipe.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "recipe.h"
#include "1.h"
#include "id_btn.h"
#include "mainclient.h"
#include "gestdata.h"
#include "lnk_body.h"
#include "def_dir.h"

#include "password.h"
#include "perif.h"
#include "newNormal.h"
#include "p_util.h"

#include "DSimpleEditor.h"

#include "DSerchFl.h"
#include <shellapi.h>
//----------------------------------------------------------------------------
//#define NAME_STR _T("PagRicetta.txt")
//----------------------------------------------------------------------------
#define NAME_STR getPageName()
//----------------------------------------------------------------------------
P_Body *getRecipe(int idPar, PWin *parent)
{
  P_Body *bd = new TD_Recipe(idPar, parent);
  return bd;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static int IdParent;
//----------------------------------------------------------------------------
//-----------------------------------------------------------
void makeNewRecipeFile(LPTSTR file)
{
  _tcscat_s(file, _MAX_PATH, PAGE_EXT);
}
//----------------------------------------------------------------------------
void makePathRecipeNew(setOfString& sStr, LPTSTR path, LPCTSTR file, bool history, LPCTSTR ext, bool addNewExt)
{
  LPCTSTR p = sStr.getString(ID_RECIPE_ABS_PATH);
  LPCTSTR p2 = sStr.getString(ID_RECIPE_SUB_PATH);
  if(p) {
    _tcscpy_s(path, _MAX_PATH, p);
    if(history)
      appendPath(path, getDir(dHistory));
    if(p2)
      appendPath(path, p2);
    createDirectoryRecurse(path);
    appendPath(path, file);
    _tcscat_s(path, _MAX_PATH, ext);
    }
  else if(p2) {
    path[0] = 0;
    makePath(path, dRicette, history, _T(""));
    appendPath(path, p2);
    createDirectoryRecurse(path);
    appendPath(path, file);
    _tcscat_s(path, _MAX_PATH, ext);
    }
  else
    makePath(path, file, dRicette, history, ext);
  if(addNewExt)
    makeNewRecipeFile(path);
}
//----------------------------------------------------------------------------
void makePathRecipeNew(setOfString& sStr, LPTSTR path, bool history, LPCTSTR ext, bool addNewExt)
{
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, path);
  path[0] = 0;
  makePathRecipeNew(sStr, path, t, history, ext, addNewExt);
}
//----------------------------------------------------------------------------
TD_Recipe::TD_Recipe(int idPar, PWin* parent, int resId, HINSTANCE hinst)
:
    P_DefBody(idPar, parent, resId, hinst), converted(false), sentByBit(0)
{
  if(idPar)
    IdParent = idPar;
}
//----------------------------------------------------------------------------
TD_Recipe::~TD_Recipe()
{
  destroy();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static
bool hasFzText(setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  return p && *p;
}
//----------------------------------------------------------------------------
#define CHECK_FZ_ENABLED(btn) \
  if(!hasFzText(sStr, btn) && sentByBit <= 0) \
    return 0; \
  --sentByBit;
//----------------------------------------------------------------------------
P_Body* TD_Recipe::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F2:
      CHECK_FZ_ENABLED(ID_F2)
      openText();
      return 0;


    case ID_F3:
      CHECK_FZ_ENABLED(ID_F3)
      load();
      return 0;

    case ID_F5:
      CHECK_FZ_ENABLED(ID_F5)
      save();
      return 0;
    case ID_F6:
      CHECK_FZ_ENABLED(ID_F6)
      copyFromClipboard();
      return 0;
    case ID_SHIFT_F6:
      CHECK_FZ_ENABLED(ID_SHIFT_F6)
      copyToClipboard();
      return 0;

    case ID_F7:
      CHECK_FZ_ENABLED(ID_F7)
      if(Send())
        msgBoxByLang(Par, ID_MSG_SEND_OK, ID_TITLE_SEND_OK, MB_ICONINFORMATION | MB_OK);
      return 0;

    case ID_F9:
      CHECK_FZ_ENABLED(ID_F9)
      if(copyMemory())
        msgBoxByLang(Par, ID_MSG_COPYMEM_OK, ID_TITLE_COPYMEM_OK, MB_ICONINFORMATION | MB_OK);
      return 0;

    case ID_F10:
      CHECK_FZ_ENABLED(ID_F10)
      setDefaultValue();
      return 0;

    case ID_F11:
      CHECK_FZ_ENABLED(ID_F11)
      erase();
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
#define DEF_FILE_TEXT _T("emptyFile")
//----------------------------------------------------------------------------
void TD_Recipe::openText()
{
  if(isReadOnly())
    return;
  TCHAR path[_MAX_PATH];
  if(!getRecipeFilename(path))
    return;
  LPCTSTR p = getPageString(ID_DEFAULT_EDITOR);
  bool simple = true;
  LPCTSTR ext = _T(".txt");
  if(!p || !_ttoi(p)) {
    p = findNextParamTrim(p);
    if(p)
      ext = p;
    simple = false;
    }

  makePathRecipeNew(sStr, path, false, ext, false);
//  makePath(path, dRicette, false, ext);
  if(!simple && !P_File::P_exist(path)) {
    if(p) {
      TCHAR t[_MAX_PATH] = DEF_FILE_TEXT;
      makePathRecipeNew(sStr, t, false, p, false);
//      makePath(t, dRicette, false, p);
      if(P_File::P_exist(t))
        CopyFile(t, path, false);
      else
        P_File(path, P_CREAT).P_open();
      }
    else
      P_File(path, P_CREAT).P_open();
    }
  if(simple)
    DSimpleEditor(this, path).modal();
  else
    ShellExecute(0, _T("open"), path, 0, 0, SW_SHOW);
}
//----------------------------------------------------------------------------
void TD_Recipe::prepareBitToFunct()
{
  if(!bitToFunct.getPrph()) {
    LPCTSTR p = getPageString(ID_RECIPE_BIT_TO_FUNCT);
    while(p) {
      uint prph = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      uint addr = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      uint type = _ttoi(p);
      bitToFunct.setPrph(prph);
      bitToFunct.setAddr(addr);
      bitToFunct.setType(type);
      bitToFunct.setnBit(0);
      break;
      }
    }
}
//----------------------------------------------------------------------------
void TD_Recipe::setReady(bool first)
{
  baseClass::setReady(true);
  convertRecipe();
  prepareBitToFunct();
  refresh();
}
//----------------------------------------------------------------------------
// le funzioni 1 e 12 sono escluse --- reinclude
#define MAX_BIT_FUNCT 12
//----------------------------------------------------------------------------
void TD_Recipe::sendBitToFunct(uint id_init, DWDATA dw)
{
  for(uint i = 0; i < MAX_BIT_FUNCT && dw; ++i, dw >>= 1)
    if(dw & 1) {
      if(sentByBit <= 0)
        sentByBit = 1;
      else
        ++sentByBit;
      PostMessage(*this, WM_POST_RECIPE_BIT_COMMAND, id_init + i, 0);
      }
}
//----------------------------------------------------------------------------
LRESULT TD_Recipe::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_POST_RECIPE_BIT_COMMAND:
      pushedBtn(wParam);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void TD_Recipe::refreshBody()
{
  baseClass::refreshBody();
  if(bitToFunct.getPrph()) {
    prfData data;
    prfData::tResultData result = bitToFunct.getData(this, data);
    DWDATA dw = data.U.dw;
    if(dw) {
      data.U.dw = 0;
      genericPerif* prph = getGenPerif(bitToFunct.getPrph());
      genericPerif::howSend old = prph->getAutoShow();
      prph->setAutoShow(genericPerif::WAIT_ASYNC);
      prph->set(data, true);
      prph->setAutoShow(old);
      if(dw & 0xffff)
        sendBitToFunct(ID_F1, dw);
      dw >>= 16;
      if(dw & 0xffff)
        sendBitToFunct(ID_SHIFT_F1, dw);
      }
    pVariable pv(bitToFunct);
    pv.setAddr(pv.getAddr() + 1);
    pv.getData(this, data);
    dw = data.U.dw;
    if(dw) {
      data.U.dw = 0;
      genericPerif* prph = getGenPerif(bitToFunct.getPrph());
      genericPerif::howSend old = prph->getAutoShow();
      prph->setAutoShow(genericPerif::WAIT_ASYNC);
      prph->set(data, true);
      prph->setAutoShow(old);
      sendBitToFunct(ID_CTRL_F1, dw);
      }
    }
}
//-----------------------------------------------------------------
bool TD_Recipe::copyMemory()
{
  return loadActMemRecipe();
}
//-----------------------------------------------------------------
bool TD_Recipe::Send()
{
  LPCTSTR p =  getPageString(ID_INIT_DATA_RECIPE);
  if(!p)
    return false;
  p = findNextParam(p, 4);
  if(p) {
    int level = _ttoi(p);
    PassWord psw;
    if(level != psw.getPsW(level, this))
      return false;
    psw.restartTime(0);
    }
/*
  // spostato direttamente dentro la routine globale di invio
  LPCTSTR p1 = getPageString(ID_OK_SEND);
  bool okToSend = true;
  if(p1) {
    int addr = 0;
    int bit = 0;
    int idPrf = WM_REAL_PLC;
    _stscanf_s(p1, _T("%d,%d,%d"), &addr, &bit, &idPrf);

    genericPerif* prf = Par->getGenPerif(idPrf);
    if(!prf)
      return false;

    prfData data;
    prfData::tData type = prf->getDefaultType();
    data.typeVar = type;

    data.lAddr = addr;
    prf->get(data);
    if(!(data.U.dw & (1 << bit)))
      okToSend = false;
    }

  if(!okToSend)  {
    msgBoxByLang(Par, ID_MSG_1, ID_MSG_2, MB_ICONINFORMATION | MB_OK);
    return false;
    }
*/
  bool result = sendRecipe();
  if(!result)  {
    msgBoxByLang(Par, ID_MSG_1, ID_MSG_2, MB_ICONINFORMATION | MB_OK);
    return false;
    }

  LPCTSTR p1 = getPageString(ID_FLAG_PLC);
  if(p1 && result) {
    int addr = 0;
    int bit = 0;
    int idPrf = WM_REAL_PLC;
    _stscanf_s(p1, _T("%d,%d,%d"), &addr, &bit, &idPrf);

    genericPerif* prf = Par->getGenPerif(idPrf);
    if(!prf)
      return false;

    prfData data;
    data.typeVar = prf->getDefaultType();
    data.lAddr = addr;
    prf->get(data);
    data.U.dw |= (1 << bit);
    return prfData::okData == prf->set(data);
    }

  return false;

}
//----------------------------------------------------------------------------
bool TD_Recipe::save()
{
  bool success = saveRecipe();
  showFileResult(success, this);
  return success;
}
//----------------------------------------------------------------------------
#define ID_TITLE_RECIPE 1000016
//--------------------------------------------------------------------------
#define KEY_RECIPE_FILE _T("Recipe Name")
//----------------------------------------------------------------------------
static bool getNameRecipe(LPTSTR target, PWin* w, setOfString& sos, LPCTSTR ext)
{
  TCHAR path[_MAX_PATH] = _T("*");
  makePathRecipeNew(sos, path, false, ext);

  smartPointerConstString sp = getStringByLang(sos, ID_TITLE_RECIPE);
  if(!sp)
    sp = smartPointerConstString(_T("Ricerca Ricette"), false);

  infoSearchParam isp;
  getKeyDWord(KEY_RECIPE_FILE, isp);
  isp.U.history = false;
  infoSearch info(sp, path, isp);
  if(!info.chooseParam(w, true))
    return false;

  info.copyDataTo(isp);
  setKeyDWord(KEY_RECIPE_FILE, isp);
  if(gSearchFile(info, w, target)) {
    int len = _tcslen(target);
    for(int i = len - 1; i > 0; --i) {
      if(_T('.') == target[i]) {
        target[i] = 0;
        break;
        }
      }
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool TD_Recipe::load()
{
  TCHAR path[_MAX_PATH];
  if(getNameRecipe(path, this, sStr, getRecipeExt())) {
    loadRecipe(path);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#define ID_INFO_REMOVE_RECIPE  1000017
#define ID_TITLE_REMOVE_RECIPE (ID_INFO_REMOVE_RECIPE + 1)
//----------------------------------------------------------------------------
bool TD_Recipe::erase()
{
  TCHAR name[_MAX_PATH];
  if(getNameRecipe(name, this, sStr, getRecipeExt())) {
    if(IDNO == msgBoxByLang(this, ID_INFO_REMOVE_RECIPE, ID_TITLE_REMOVE_RECIPE, MB_YESNO | MB_ICONSTOP))
      return false;

    TCHAR path[_MAX_PATH];
    makePathRecipeNew(sStr, path, name, false, getRecipeExt());
    DeleteFile(path);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void TD_Recipe::getFileStr(LPTSTR path)
{
  getFileStrCheckRemote(path, NAME_STR);
}
//----------------------------------------------------------------------------
bool TD_Recipe::create()
{
  if(!baseClass::create())
    return false;
  Attr.id = IDD_RECIPE;
  return true;
}
//-----------------------------------------------------------
void fillSet(pVectInt& v, LPCTSTR p, int offset)
{
  int i = offset;
  while(p && *p) {
    while(*p && !_istdigit((unsigned)*p) && _T('-') != *p)
      ++p;
    if(!*p)
      break;
    v[i] = _ttoi(p);
    ++i;
    p = findNextParam(p, 1);
    }
}
//-----------------------------------------------------------
#define ROUND_DWDATA(a) (((a) + sizeof(DWDATA) - 1) / sizeof(DWDATA))
//-----------------------------------------------------------
//#define ROUND_REAL(a) (DWDATA)((a) - ((a) < 0) + 0.5)
//----------------------------------------------------------------------------
void TD_Recipe::setDefaultValue()
{
  LPCTSTR p =  getPageString(ID_INIT_DEFAULT_VALUE);
  if(!p)
    return;
  pVectInt vAddr;
  fillSet(vAddr, p);

  uint nElem = vAddr.getElem();
  if(!nElem)
    return;

  p =  getPageString(ID_INIT_DEFAULT_VALUE + 1);
  if(!p)
    return;

  pVectInt vType;
  fillSet(vType, p);

  if(nElem != vType.getElem())
    return;

  p =  getPageString(ID_INIT_DEFAULT_VALUE + 2);
  if(!p)
    return;

  pVectInt vNorm;
  fillSet(vNorm, p);

  if(nElem != vNorm.getElem())
    return;

  p =  getPageString(ID_INIT_DEFAULT_VALUE + 3);
  if(!p)
    return;

  genericPerif* prphMem = getMemoryPrph();
  if(!prphMem)
    return;
  for(uint i = 0; i < nElem; ++i) {
    prfData data;
    data.lAddr = vAddr[i];
    switch(vType[i]) {
      case prfData::tBData:
      case prfData::tWData:
      case prfData::tDWData:
      default:
        data.typeVar = prfData::tDWData;
        data.U.dw = (DWDATA)ROUND_REAL(_tstof(p) / getNorm(vNorm[i]));
        break;

      case prfData::tFRData:
        data.typeVar = prfData::tFRData;
        data.U.fw = (fREALDATA)(_tstof(p) / getNorm(vNorm[i]));
        break;
      case prfData::tStrData:
        do {
          TCHAR tmp[100];
          int len = vNorm[i];
          copyStrZ(tmp, p, len);
          for(int j = 0; j < len; ++j)
            if(!tmp[j] || _T(',') == tmp[j]) {
              tmp[j] = 0;
              break;
              }
          data.setStr(len, tmp);
          } while(false);
        break;
      }
    prphMem->set(data);
    p = findNextParam(p, 1);
    }
}
//-----------------------------------------------------------
#define CHECK_CONTINUE(test, vDim) \
    if(test) { \
      if(vDim[i]) { \
        offs += ROUND_DWDATA(vDim[i]);\
        }\
      else\
        ++offs;\
      continue;\
      }
//-----------------------------------------------------------
static
int fillAllSet(setOfString& set, int ixInit, pVectInt& vPrf, pVectInt& vAddr,
                      pVectInt& vType, pVectInt& vTypeMem, pVectInt& vDimData)
{
  LPCTSTR p =  set.getString(ixInit);
  if(!p)
    return 0;
  fillSet(vPrf, p);

  uint nElem = vPrf.getElem();
  if(!nElem)
    return 0;

  p =  set.getString(ixInit + 1);
  if(!p)
    return 0;
  fillSet(vAddr, p);

  p =  set.getString(ixInit + 2);
  if(!p)
    return 0;
  fillSet(vType, p);

  p =  set.getString(ixInit + 3);
  if(p)
    fillSet(vDimData, p);

  p =  set.getString(ID_INIT_DATA_RECIPE + 10);
  if(p)
    fillSet(vTypeMem, p);

  if(nElem != vAddr.getElem() || nElem != vType.getElem())
    return 0;

  do {
    int nT = vType.getElem();
    for(int i = vTypeMem.getElem(); i < nT; ++i)
      vTypeMem[i] = vType[i];
    } while(false);

  for(uint i = vDimData.getElem(); i < nElem; ++i)
    vDimData[i] = 0;

  return nElem;
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
static
LPTSTR get_line(LPTSTR buff, int len)
{
  if(len < 0)
    return 0;
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(; i < len; ++i, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
static long getLong(LPCTSTR p)
{
  long res;
  _stscanf_s(p, _T("%ld"), &res);
  return res;
}
//----------------------------------------------------------------------------
static DWORD getDWord(LPCTSTR p)
{
  DWORD res;
  _stscanf_s(p, _T("%u"), &res);
  return res;
}
//----------------------------------------------------------------------------
static fREALDATA getReal(LPCTSTR p)
{
  TCHAR tmp[1024] = _T("\0");
  LPTSTR t = tmp;
  LPCTSTR sep = getString(ID_SEP_DEC);
  if(!sep)
    sep = _T(",");
  while(*p) {
    if(*sep == *p)
      *t++ = _T('.');
    else
      *t++ = *p;
    ++p;
    }
  *t = 0;
  fREALDATA res = (fREALDATA)_tstof(tmp);
  return res;
}
//----------------------------------------------------------------------------
static void getReal(LPTSTR t, fREALDATA val)
{
  TCHAR tmp[1024] = _T("\0");
  _stprintf_s(tmp, _T("%f\r\n"), val);

  LPTSTR p = tmp;
  LPCTSTR sep = getString(ID_SEP_DEC);
  if(!sep)
    sep = _T(",");
  while(*p) {
    if(_T('.') == *p)
      *t++ = *sep;
    else
      *t++ = *p;
    ++p;
    }
  *t = 0;
}
//----------------------------------------------------------------------------
void TD_Recipe::fillRowByClipboard(LPTSTR lptstr)
{
  LPCTSTR p =  sStr.getString(ID_INIT_DATA_RECIPE);
  if(!p)
    return;
  int initAddrMem;
  int indT1;
  int len;
  int indT2;
  int pswLevel = 0;
  // nuova specifica per periferica memoria
  int idMemPrph = 1;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &initAddrMem, &indT1, &len, &indT2, &pswLevel, &idMemPrph);

  pVectInt vPrf;
  pVectInt vAddr;
  pVectInt vType;
  pVectInt vDimData;
  pVectInt vTypeMem;
  uint nElem = fillAllSet(sStr, ID_INIT_DATA_RECIPE + 1, vPrf, vAddr, vType, vTypeMem, vDimData);
  if(!nElem)
    return;

  genericPerif* prfMem = Par->getGenPerif(idMemPrph);
  if(!prfMem)
    return;

  int offs = initAddrMem;
  int offsTxt = 0;
  int dim = _tcslen(lptstr);
  for(uint i = 0; lptstr && i < nElem; ++i) {
    LPTSTR next = get_line(lptstr, dim - offsTxt);

    if(prfData::tStrData == vTypeMem[i]) {
      prfData strData(vDimData[i], lptstr);
      strData.lAddr = offs;
      prfMem->set(strData);
      offs += ROUND_DWDATA(vDimData[i]);
      }
    else {
      prfData dataMem;
      dataMem.typeVar = vTypeMem[i];
      dataMem.lAddr = offs;
      dataMem.U.dw = 0;

      switch(vTypeMem[i]) {
        case prfData::tBData:
        case prfData::tWData:
        case prfData::tDWData:
        default:
          dataMem.U.dw = getDWord(lptstr);
          break;
        case prfData::tBsData:
        case prfData::tWsData:
        case prfData::tDWsData:
          dataMem.U.sdw = getLong(lptstr);
          break;

        case prfData::tFRData:
          dataMem.U.fw = getReal(lptstr);
          break;
        }
      ++offs;
      prfMem->set(dataMem, true);
      }

    int lenTxt = next ? next - lptstr : dim - offsTxt;
    offsTxt += lenTxt;
    lptstr = next;
    }
}
//----------------------------------------------------------------------------
int TD_Recipe::fieldToClipboard(PVect<LPCTSTR>& set)
{
  LPCTSTR p =  sStr.getString(ID_INIT_DATA_RECIPE);
  if(!p)
    return 0;
  int initAddrMem;
  int indT1;
  int len;
  int indT2;
  int pswLevel = 0;
  // nuova specifica per periferica memoria
  int idMemPrph = 1;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &initAddrMem, &indT1, &len, &indT2, &pswLevel, &idMemPrph);

  pVectInt vPrf;
  pVectInt vAddr;
  pVectInt vType;
  pVectInt vDimData;
  pVectInt vTypeMem;
  uint nElem = fillAllSet(sStr, ID_INIT_DATA_RECIPE + 1, vPrf, vAddr, vType, vTypeMem, vDimData);
  if(!nElem)
    return 0;

  genericPerif* prfMem = Par->getGenPerif(idMemPrph);
  if(!prfMem)
    return 0;

  int offs = initAddrMem;
  int dim = 0;
  for(uint i = 0; i < nElem; ++i) {
    if(prfData::tStrData == vTypeMem[i]) {
      prfData strData(vDimData[i]);
      strData.lAddr = offs;
      prfMem->get(strData);
      LPTSTR t = new TCHAR[vDimData[i] + 4];
      copyStrZ(t, strData.U.str.buff, vDimData[i]);
      _tcscat_s(t, vDimData[i] + 4, _T("\r\n"));
      dim += _tcslen(t);
      set[i] = t;
      offs += ROUND_DWDATA(vDimData[i]);
      }
    else {
      prfData dataMem;
      dataMem.typeVar = vTypeMem[i];
      dataMem.lAddr = offs;
      dataMem.U.dw = 0;
      prfMem->get(dataMem);
      TCHAR t[128];
      switch(vTypeMem[i]) {
        case prfData::tBData:
        case prfData::tWData:
        case prfData::tDWData:
        default:
          wsprintf(t, _T("%u\r\n"), dataMem.U.dw);
          break;
        case prfData::tBsData:
        case prfData::tWsData:
        case prfData::tDWsData:
          wsprintf(t, _T("%d\r\n"), dataMem.U.sdw);
          break;

        case prfData::tFRData:
          getReal(t, dataMem.U.fw);
          break;
        }
      set[i] = str_newdup(t);
      dim += _tcslen(t);
      ++offs;
      }
    }
  return dim;
}
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define myCF_TEXT CF_UNICODETEXT
#else
  #define myCF_TEXT CF_TEXT
#endif
//----------------------------------------------------------------------------
void TD_Recipe::copyFromClipboard()
{
  if(!IsClipboardFormatAvailable(myCF_TEXT))
    return;
  if(!OpenClipboard(*this))
    return;
  HGLOBAL hglb = GetClipboardData(myCF_TEXT);
  uint ix = 0;
  while(hglb) {
    LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
    if(!lptstr)
      break;
    fillRowByClipboard(lptstr);
    GlobalUnlock(hglb);
    break;
    }
  CloseClipboard();
}
//----------------------------------------------------------------------------
void TD_Recipe::copyToClipboard()
{
  if(!OpenClipboard(*this))
    return;
  EmptyClipboard();

  PVect<LPCTSTR> rows;
  uint dim = fieldToClipboard(rows);
  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (dim + 16) * sizeof(TCHAR));
  if(hglbCopy) {
    LPTSTR b = (LPTSTR)GlobalLock(hglbCopy);
    LPTSTR pb = b;
    int d = dim + 16;
    uint nElem = rows.getElem();
    for(uint i = 0; i < nElem; ++i) {
      _tcscpy_s(pb, d - (pb - b), rows[i]);
      pb += _tcslen(pb);
      }
    GlobalUnlock(hglbCopy);
    SetClipboardData(myCF_TEXT, hglbCopy);
    }
  CloseClipboard();
  flushPAV(rows);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
LPCTSTR get_RecipeExt(setOfString& set)
{
  LPCTSTR ext = set.getString(ID_RECIPE_EXT);
  if(!ext)
    ext = getExt(dRicette);
  return ext;
}
/**/
//-----------------------------------------------------------
genericPerif* get_MemoryPrph(mainClient* Par, setOfString& set)
{
  LPCTSTR p =  set.getString(ID_INIT_DATA_RECIPE);
  if(!p)
    return 0;
  p = findNextParam(p, 5);
  uint idMemPrph = 1;
  if(p)
    idMemPrph = _ttoi(p);
  if(!idMemPrph || idMemPrph > 20)
    idMemPrph = 1;

  genericPerif* prphMem = Par->getGenPerif(idMemPrph);
  return prphMem;
}
//-----------------------------------------------------------
enum errSendRecipe { no_err, errNoPrph, errNoEmg, errNoDataRecipe, errNoDataString, errNoMemPrph };
int send_recipe(mainClient* Par, setOfString& sStr)
{
  do {
    LPCTSTR p1 = sStr.getString(ID_OK_SEND);
    if(p1) {
      int addr = 0;
      int bit = 0;
      int idPrf = WM_REAL_PLC;
      _stscanf_s(p1, _T("%d,%d,%d"), &addr, &bit, &idPrf);

      genericPerif* prf = Par->getGenPerif(idPrf);
      if(!prf)
        return errNoPrph;

      prfData data;
      prfData::tData type = prf->getDefaultType();
      data.typeVar = type;

      data.lAddr = addr;
      prf->get(data);
      if(!(data.U.dw & (1 << bit)))
        return errNoEmg;
      }
    } while(false);
  LPCTSTR p =  sStr.getString(ID_INIT_DATA_RECIPE);
  if(!p)
    return errNoDataRecipe;
  int initAddrMem;
  int indT1;
  int len;
  int indT2;
  int pswLevel = 0;
  // nuova specifica per periferica memoria
  int idMemPrph = 1;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &initAddrMem, &indT1, &len, &indT2, &pswLevel, &idMemPrph);

  pVectInt vPrf;
  pVectInt vAddr;
  pVectInt vType;
  pVectInt vDimData;
  pVectInt vTypeMem;
  int nElem = fillAllSet(sStr, ID_INIT_DATA_RECIPE + 1, vPrf, vAddr, vType, vTypeMem, vDimData);
  if(!nElem)
    return errNoDataString;

  #define TOT_PRF_CHECK 21
  pVectInt totPrf;
  totPrf.setDim(TOT_PRF_CHECK);
  for(int i = 0; i < TOT_PRF_CHECK; ++i)
    totPrf[i] = 0;

  for(int i = 0; i < nElem; ++i) {
    uint idPrf = vPrf[i];
    if(idPrf >= TOT_PRF_CHECK)
      idPrf = 0;
    ++totPrf[idPrf];
    }

  genericPerif* prfMem = Par->getGenPerif(idMemPrph);
  if(!prfMem)
    return errNoMemPrph;

  // multiset solo su periferiche esterne
  for(int j = WM_PLC; j < TOT_PRF_CHECK; ++j) {
    if(!totPrf[j])
      continue;
    prfData* data = new prfData[totPrf[j]];

    int offs = initAddrMem;
    int ixData = 0;
    gestPerif* prf = Par->getPerif(j);
    if(!prf)
      continue;
    for(int i = 0; i < nElem; ++i) {
      CHECK_CONTINUE(vPrf[i] != j, vDimData)

      if(prfData::tStrData == vType[i]) {

        prfData memData(vDimData[i]);
        memData.lAddr = offs;
        prfMem->get(memData);
        data[ixData] = memData;
        offs += ROUND_DWDATA(vDimData[i]);
        }
      else {
        REALDATA temp = 0;
        prfData dataMem;
        dataMem.typeVar = vTypeMem[i];
        dataMem.lAddr = offs;
        prfMem->get(dataMem);

        switch(vTypeMem[i]) {
          case prfData::tBData:
          case prfData::tWData:
          case prfData::tDWData:
          default:
            temp = (REALDATA)dataMem.U.dw;
            break;
          case prfData::tBsData:
          case prfData::tWsData:
          case prfData::tDWsData:
            temp = (REALDATA)dataMem.U.sdw;
            break;

          case prfData::tFRData:
            temp = (REALDATA)dataMem.U.fw;
            break;
          }
        data[ixData].typeVar = vType[i];
        data[ixData].lAddr = vAddr[i];
        switch(vType[i]) {
          case prfData::tBData:
          case prfData::tWData:
            data[ixData].U.w = (WDATA)ROUND_REAL(temp);
            break;
          case prfData::tBsData:
          case prfData::tWsData:
            data[ixData].U.sw = (WDATA)ROUND_REAL(temp);
            break;
          case prfData::tDWData:
          default:
            data[ixData].U.dw = ROUND_REAL(temp);
          case prfData::tDWsData:
            data[ixData].U.sdw = ROUND_REAL(temp);
            break;
          case prfData::tFRData:
            data[ixData].U.fw = (fREALDATA)temp;
            break;
          }
        ++offs;
        }
      ++ixData;
      }
    gestPerif::howSend old = prf->setAutoShow(gestPerif::ALL_AND_SHOW);
    prf->multiSet(data, ixData);
    prf->setAutoShow(old);
    delete []data;
    }

  // se usa anche la memoria
  if(totPrf[PRF_MEMORY]) {
    int offs = initAddrMem;
    prfData data;
    for(int i = 0; i < nElem; ++i) {
      CHECK_CONTINUE(vPrf[i] != PRF_MEMORY, vDimData)
      genericPerif* prf = Par->getGenPerif(PRF_MEMORY);
      if(prfData::tStrData == vType[i]) {
        prfData strData(vDimData[i]);
        strData.lAddr = vAddr[i];
        prfMem->get(strData);
        prf->set(strData);
        offs += ROUND_DWDATA(vDimData[i]);
        }
      else {
        REALDATA temp = 0;
        prfData dataMem;
        dataMem.typeVar = vTypeMem[i];
        dataMem.lAddr = offs;
        prfMem->get(dataMem);

        switch(vTypeMem[i]) {
          case prfData::tBData:
          case prfData::tWData:
          case prfData::tDWData:
          default:
            temp = (REALDATA)dataMem.U.dw;
            break;
          case prfData::tBsData:
          case prfData::tWsData:
          case prfData::tDWsData:
            temp = (REALDATA)dataMem.U.sdw;
            break;

          case prfData::tFRData:
            temp = (REALDATA)dataMem.U.fw;
            break;
          }
        data.typeVar = vType[i];
        data.lAddr = vAddr[i];
        switch(vType[i]) {
          case prfData::tBData:
          case prfData::tWData:
            data.U.w = (WDATA)ROUND_REAL(temp);
            break;
          case prfData::tBsData:
          case prfData::tWsData:
            data.U.sw = (WDATA)ROUND_REAL(temp);
            break;
          case prfData::tDWData:
          default:
            data.U.dw = ROUND_REAL(temp);
          case prfData::tDWsData:
            data.U.sdw = ROUND_REAL(temp);
            break;
          case prfData::tFRData:
            data.U.fw = (fREALDATA)temp;
            break;
          }
        ++offs;
        prf->set(data);
        }
      }
    }
  prfData data;
  int addr1 = indT1;
  int addr2 = indT2;
  int len4 = len / 4 + ((len % 4) != 0);
  data.typeVar = prfData::tDWData;
  for(int i = 0; i < len4; ++i, ++addr1, ++addr2) {
    data.lAddr = addr1;
    prfMem->get(data);
    data.lAddr = addr2;
    prfMem->set(data);
    }
  return no_err;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool TD_Recipe::sendRecipe()
{
  if(isReadOnly())
    return true;
  return !send_recipe(Par, sStr);
}
//-----------------------------------------------------------
static
void unfillBuff(LPCTSTR buff, prfData& data)
{
  switch(data.typeVar) {
    case prfData::tBData:
    case prfData::tWData:
    case prfData::tDWData:
    default:
      data.U.dw = _ttoi(buff);
      break;

//#define NOT_UNIQUE_SIGNED_VAL
    case prfData::tBsData:
#ifdef NOT_UNIQUE_SIGNED_VAL
      data.U.sb = BsDATA(_ttoi(buff));
      break;
#endif
    case prfData::tWsData:
#ifdef NOT_UNIQUE_SIGNED_VAL
      data.U.sw = WsDATA(_ttoi(buff));
      break;
#endif
    case prfData::tDWsData:
      data.U.sdw = _ttoi(buff);
      break;

    case prfData::tFRData:
      data.U.fw = fREALDATA(_tstof(buff));
      break;
    case prfData::tRData:
      data.U.rw = _tstof(buff);
      break;
    case prfData::ti64Data:
      data.U.li.QuadPart = _tstoi64(buff);
      break;
    case prfData::tDateHour:
    case prfData::tDate:
      unformat_data(buff, data.U.ft, whichData());
      break;
    case prfData::tHour:
      unformat_time(buff, data.U.ft);
      break;
    case prfData::tStrData:
      {
      int addr = data.lAddr;
      data = prfData(buff);
      data.lAddr = addr;
      }
      break;
    }
}
//-----------------------------------------------------------
class manageRecipeRow : public manageRowOfBuff
{
  private:
    typedef manageRowOfBuff baseClass;
  public:
    manageRecipeRow(LPCTSTR filename, setOfString& sStr, genericPerif* prfMem, int initAddrMem) :
        baseClass(filename), sStr(sStr), prfMem(prfMem), initAddrMem(initAddrMem), curr(0) { init(); }
  protected:
    virtual bool manageLine(LPCTSTR row);
  private:
    setOfString& sStr;
    genericPerif* prfMem;
    int initAddrMem;
    pVectInt vPrf;
    pVectInt vTypeData;
    pVectInt vDimData;
    pVectInt vAddrData;
    int curr;
    void init();
};
//-----------------------------------------------------------
#define ROUND_4_V(a) ((a) ? ((a) / 4 + ((a) % 4 != 0)) : 1)
//-----------------------------------------------------------
void manageRecipeRow::init()
{
  LPCTSTR p =  sStr.getString(ID_INIT_DATA_RECIPE + 1);
  if(!p)
    return;
  fillSet(vPrf, p);

  p =  sStr.getString(ID_INIT_DATA_RECIPE + 10);
  if(p)
    fillSet(vTypeData, p);

  p =  sStr.getString(ID_INIT_DATA_RECIPE + 4);
  if(p)
    fillSet(vDimData, p);

  uint nElem = vPrf.getElem();

  vTypeData.setDim(nElem);
  for(uint i = vTypeData.getElem(); i < nElem; ++i)
    vTypeData[i] = prfData::tDWData;

  for(uint i = 0; i < vDimData.getElem(); ++i)
    if(!vDimData[i])
      vDimData[i] = sizeof(DWDATA);
  vDimData.setDim(nElem);
  for(uint i = vDimData.getElem(); i < nElem; ++i)
    vDimData[i] = sizeof(DWDATA);

  vAddrData.setDim(nElem);
  vAddrData[0] = initAddrMem;
  for(int i = 1; i < (int)nElem; ++i)
    vAddrData[i] = vAddrData[i - 1] + ROUND_4_V(vDimData[i - 1]);
}
//-----------------------------------------------------------
bool manageRecipeRow::manageLine(LPCTSTR row)
{
  if(curr >= (int)vPrf.getElem())
    return curr ? true : false;
  if(prfData::tStrData == vTypeData[curr]) {
    prfData data(vDimData[curr], row);
    data.lAddr = vAddrData[curr];
    prfMem->set(data);
    }
  else {
    prfData data;
    data.typeVar = vTypeData[curr];
    data.lAddr = vAddrData[curr];
    if(data.typeVar) {
      unfillBuff(row, data);
      prfMem->set(data);
      }
    }
  ++curr;
  return true;
}
//-----------------------------------------------------------
bool load_recipe_new(mainClient* Par, setOfString& sStr, LPCTSTR file, genericPerif* prfMem, int initAddrMem)
{
  return manageRecipeRow(file, sStr, prfMem, initAddrMem).run();
}
//-----------------------------------------------------------
bool load_recipe(mainClient* Par, setOfString& sStr, LPCTSTR name)
{
  LPCTSTR p =  sStr.getString(ID_INIT_DATA_RECIPE);
  if(!p)
    return false;

  TCHAR path[_MAX_PATH];
  makePathRecipeNew(sStr, path, name, false, get_RecipeExt(sStr));
  int initAddrMem = _ttoi(p);

  genericPerif* prfMem = get_MemoryPrph(Par, sStr);
  if(!prfMem)
    return false;
  bool success = load_recipe_new(Par, sStr, path, prfMem, initAddrMem);
  if(success) {
    p = findNextParam(p, 1);
    // se il nome della ricetta non è nel primo valore
    // recupera indirizzo e dimensione
    if(p) {
      int addr = _ttoi(p);
      if(addr) {
        p = findNextParam(p, 1);
        if(p) {
          int len = _ttoi(p);
          if(len) {
            prfData data;
            data.lAddr = addr;
            data.setStr(len, name);
            prfMem->set(data);
            }
          }
        }
      }
    return true;
    }
  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool TD_Recipe::loadRecipe(LPCTSTR name)
{
  return load_recipe(Par, sStr, name);
}
//-----------------------------------------------------------
bool TD_Recipe::getRecipeFilename(LPTSTR target)
{
  LPCTSTR p =  getPageString(ID_INIT_DATA_RECIPE);
  if(!p)
    return false;
  int initAddrMem = _ttoi(p);
  target[0] = 0;
  p = findNextParam(p, 1);
  genericPerif* prfMem = getMemoryPrph();
  if(!prfMem)
    return false;
  prfData data;
  data.typeVar = prfData::tDWData;

  if(p && _ttoi(p)) {
    int addr = _ttoi(p);
    if(addr) {
      p = findNextParam(p, 1);
      if(p) {
        int len = _ttoi(p);
        if(len) {
          data.lAddr = addr;
          data.setStr(len);
          prfMem->get(data);
          copyStr(target, data.U.str.buff, len + 1);
          }
        }
      }
    }
  else {
    data.lAddr = initAddrMem;
    prfMem->get(data);
    wsprintf(target, _T("%d"), data.U.dw);
    }
  if(!*target)
    return false;
  return true;
}
//-----------------------------------------------------------
static
void fillReal(LPTSTR buff, size_t dim, REALDATA v)
{
  _stprintf_s(buff, dim, _T("%f"), v);
  zeroTrim(buff);
}
//------------------------
static
void removeHour(LPTSTR buff)
{
  int len = _tcslen(buff);
  buff[len - 8] = 0;
}
//-----------------------------------------------------------
static
void fillBuff(LPTSTR buff, size_t dim, const prfData& data)
{
  buff[0] = 0;
  switch(data.typeVar) {
    case prfData::tBData:
      wsprintf(buff, _T("%d"), data.U.b);
      break;
    case prfData::tWData:
      wsprintf(buff, _T("%d"), data.U.w);
      break;
    case prfData::tDWData:
    default:
      wsprintf(buff, _T("%u"), data.U.dw);
      break;

    case prfData::tBsData:
      wsprintf(buff, _T("%d"), data.U.sb);
      break;
    case prfData::tWsData:
      wsprintf(buff, _T("%d"), data.U.sw);
      break;
    case prfData::tDWsData:
      wsprintf(buff, _T("%d"), data.U.sdw);
      break;

    case prfData::tFRData:
      fillReal(buff, dim, data.U.fw);
      break;
    case prfData::tRData:
      fillReal(buff, dim, data.U.rw);
      break;
    case prfData::ti64Data:
      _stprintf_s(buff, dim, _T("%I64d"), data.U.li.QuadPart);
      break;
    case prfData::tDateHour:
      set_format_data(buff, dim, data.U.ft, whichData(), _T(" - "));
      break;
    case prfData::tDate:
      set_format_data(buff, dim, data.U.ft, whichData(), _T("|"));
      removeHour(buff);
      break;
    case prfData::tHour:
      set_format_time(buff, dim, data.U.ft);
      break;
    case prfData::tStrData:
      {
      LPTSTR p = buff;
      LPCSTR a = (LPCSTR)data.U.str.buff;
      while(*a)
        *p++ = *a++;
      *p = 0;
      }
      break;
    }
  _tcscat_s(buff, dim, _T("\r\n"));
}
//-----------------------------------------------------------
bool TD_Recipe::saveRecipe()
{
  if(isReadOnly())
    return true;
  TCHAR path[_MAX_PATH];
  if(!getRecipeFilename(path))
    return false;

  makePathRecipeNew(sStr, path, false, getRecipeExt());
  if(P_File::P_exist(path))
    if(!showMsgFileExist(this))
      return false;

  P_File f(path, P_CREAT);
  if(!f.P_open())
    return false;

  LPCTSTR p =  getPageString(ID_INIT_DATA_RECIPE);
  if(!p)
    return false;
  int initAddrMem = _ttoi(p);

  genericPerif* prfMem = getMemoryPrph();
  if(!prfMem)
    return false;

  p =  getPageString(ID_INIT_DATA_RECIPE + 1);
  if(!p)
    return false;
  pVectInt vPrf;
  fillSet(vPrf, p);

  uint nElem = vPrf.getElem();

  p =  getPageString(ID_INIT_DATA_RECIPE + 10);
  pVectInt vTypeData;
  if(p)
    fillSet(vTypeData, p);

  p =  getPageString(ID_INIT_DATA_RECIPE + 4);
  pVectInt vDimData;
  if(p)
    fillSet(vDimData, p);

  vTypeData.setDim(nElem);
  for(uint i = vTypeData.getElem(); i < nElem; ++i)
    vTypeData[i] = prfData::tDWData;

  vDimData.setDim(nElem);
  for(uint i = vDimData.getElem(); i < nElem; ++i)
    vDimData[i] = sizeof(DWDATA);

  bool success = true;
  int addr = initAddrMem;
  for(uint i = 0; i < nElem && success; ++i) {
    prfData data;
    data.lAddr = addr;
    data.typeVar = (prfData::tData)vTypeData[i];
    if(prfData::tStrData == data.typeVar)
      data.setStr(vDimData[i]);
    prfMem->get(data);
    TCHAR buff[512];
    fillBuff(buff, SIZE_A(buff), data);
    if(!f.P_writeString(buff))
      success = false;
    addr += vDimData[i] ? (vDimData[i] / 4 + (vDimData[i] % 4 != 0)) : 1;
    }
  return success;
}
//-----------------------------------------------------------
bool TD_Recipe::loadActMemRecipe()
{
  LPCTSTR p =  getPageString(ID_INIT_DATA_RECIPE);
  if(!p)
    return false;
  int initAddrMem = _ttoi(p);

  int ixInit = ID_INIT_ACT_DATA;
  p =  getPageString(ixInit);

  pVectInt vPrf;
  pVectInt vAddr;
  pVectInt vType;
  pVectInt vDimData;
  pVectInt vTypeMem;
  if(!p)
    ixInit = ID_INIT_DATA_RECIPE + 1;
  int nElem = fillAllSet(sStr, ixInit, vPrf, vAddr, vType, vTypeMem, vDimData);
  if(!nElem)
    return false;

  prfData data;
  genericPerif* prfMem = getMemoryPrph();
  if(!prfMem)
    return false;
  int offs = initAddrMem;
  for(int i = 0; i < nElem; ++i) {
    CHECK_CONTINUE(vPrf[i] < 1, vDimData)

    int numPrf = vPrf[i];
    genericPerif *prf = Par->getGenPerif(numPrf);
    CHECK_CONTINUE(!prf, vDimData)

    if(prfData::tStrData == vType[i]) {
      prfData strData(vDimData[i]);
      strData.lAddr = vAddr[i];
      prf->get(strData);
      strData.lAddr = offs;
      prfMem->set(strData);
      offs += ROUND_DWDATA(vDimData[i]);
      }

    else {
      REALDATA temp = 0;
      data.typeVar = vType[i];
      data.lAddr = vAddr[i];
      prf->get(data);
      switch(vType[i]) {
        case prfData::tBData:
        case prfData::tWData:
        case prfData::tDWData:
        default:
          temp = (REALDATA)data.U.dw;
          break;

        case prfData::tFRData:
          temp = (REALDATA)data.U.fw;
          break;
        }

      switch(vTypeMem[i]) {
        case prfData::tBData:
        case prfData::tWData:
        case prfData::tDWData:
        default:
          data.typeVar = prfData::tDWData;
          data.lAddr = offs;
          data.U.dw = ROUND_REAL(temp);
          prfMem->set(data);
          break;
        case prfData::tFRData:
          data.typeVar = prfData::tFRData;
          data.lAddr = offs;
          data.U.fw = (fREALDATA)temp;
          prfMem->set(data);
          break;
        }
      ++offs;
      }
    }
  return true;
}
//-----------------------------------------------------------
LPCTSTR TD_Recipe::getRecipeExt()
{
  LPCTSTR ext = getPageString(ID_RECIPE_EXT);
  if(!ext)
    ext = getExt(dRicette);
  return ext;
}
/**/
//-----------------------------------------------------------
genericPerif* TD_Recipe::getMemoryPrph()
{
  LPCTSTR p =  getPageString(ID_INIT_DATA_RECIPE);
  if(!p)
    return 0;
  p = findNextParam(p, 5);
  uint idMemPrph = 1;
  if(p)
    idMemPrph = _ttoi(p);
  if(!idMemPrph || idMemPrph > 20)
    idMemPrph = 1;

  genericPerif* prphMem = Par->getGenPerif(idMemPrph);
  return prphMem;
}
//-----------------------------------------------------------
class manageConvertRecipe
{
  public:
    manageConvertRecipe(setOfString& sStr) :
        sStr(sStr) { init(); }

    void run(LPCTSTR filename);
  protected:
  private:
    setOfString& sStr;
    pVectInt vPrf;
    pVectInt vTypeData;
    pVectInt vDimData;
    void init();
};
//-----------------------------------------------------------
void manageConvertRecipe::init()
{
  LPCTSTR p =  sStr.getString(ID_INIT_DATA_RECIPE + 1);
  if(!p)
    return;
  fillSet(vPrf, p);

  p =  sStr.getString(ID_INIT_DATA_RECIPE + 10);
  if(p)
    fillSet(vTypeData, p);

  p =  sStr.getString(ID_INIT_DATA_RECIPE + 4);
  if(p)
    fillSet(vDimData, p);

  uint nElem = vPrf.getElem();

  for(uint i = 0; i < vTypeData.getElem(); ++i)
    if(!vTypeData[i])
      vTypeData[i] = prfData::tDWData;

  vTypeData.setDim(nElem);
  for(uint i = vTypeData.getElem(); i < nElem; ++i)
    vTypeData[i] = prfData::tDWData;

  for(uint i = 0; i < vDimData.getElem(); ++i)
    if(!vDimData[i])
      vDimData[i] = sizeof(DWDATA);

  vDimData.setDim(nElem);
  for(uint i = vDimData.getElem(); i < nElem; ++i)
    vDimData[i] = sizeof(DWDATA);
}
//-----------------------------------------------------------
void manageConvertRecipe::run(LPCTSTR path)
{
  P_File fs(path, P_READ_ONLY);
  if(!fs.P_open())
    return;

  TCHAR t[_MAX_PATH * 2];
  _tcscpy_s(t, path);
  makeNewRecipeFile(t);

  P_File ft(t, P_CREAT);
  if(!ft.P_open())
    return;
  uint len = (uint)fs.get_len();
  LPBYTE buff = new BYTE[len + 3];
  if(len != fs.P_read(buff, len)) {
    delete []buff;
    return;
    }

  uint nElem = vPrf.getElem();
  uint offs = 0;
  bool success = true;
  for(uint i = 0; i < nElem && success; ++i) {
    if(!vTypeData[i])
      _tcscpy_s(t, _T("0\r\n"));
    else {
      if(prfData::tStrData == vTypeData[i]) {
        copyStrZ(t, buff + offs, vDimData[i]);
        _tcscat_s(t, _T("0\r\n"));
        }
      else {
        prfData data;
        data.typeVar = (prfData::tData)vTypeData[i];
        CopyMemory(&data.U.b, buff + offs, vDimData[i]);
        fillBuff(t, SIZE_A(t), data);
        }
      }
    if(!ft.P_writeString(t))
      success = false;
    offs += vDimData[i];
    }
  fs.P_close();
  if(success) {
    fs.reback(_T(".bin.old"));
//    DeleteFile(path);
    }
  delete []buff;
}
//----------------------------------------------------------------------------
static bool isValidExt(LPCTSTR FileName, LPCTSTR ext)
{
  int len = _tcslen(FileName) - 4;
  return len > 0 && !_tcsicmp(ext, FileName + len);
}
//----------------------------------------------------------------------------
void TD_Recipe::convertRecipe()
{
  if(converted)
    return;
  manageConvertRecipe* mcr = 0;
  for(uint ii = 0; ii < 2; ++ii) {
    TCHAR path[_MAX_PATH] = _T("*");
    LPCTSTR ext = getRecipeExt();
    makePath(path, dRicette, toBool(ii), ext);

    WIN32_FIND_DATA fd;
    HANDLE hfff = FindFirstFile(path, &fd);
    if(INVALID_HANDLE_VALUE != hfff) {
      path[0] = 0;
      makePath(path, dRicette, toBool(ii), _T(""));
      LPTSTR p = path + _tcslen(path);
      if(!mcr)
        mcr = new manageConvertRecipe(sStr);
      do {
        if(FILE_ATTRIBUTE_DIRECTORY == fd.dwFileAttributes)
          continue;
        if(isValidExt(fd.cFileName, ext)) {
          appendPath(path, fd.cFileName);
          mcr->run(path);
          *p = 0;
          }
        } while(FindNextFile(hfff, &fd));
      FindClose(hfff);
      }
    }
  delete mcr;
  converted = true;
}
