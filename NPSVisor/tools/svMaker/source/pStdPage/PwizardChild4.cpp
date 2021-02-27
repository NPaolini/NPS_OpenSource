//----------- pWizardChild4.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include <lm.h>
//#include <Lmwksta.h>
//----------------------------------------------------------------------------
#include "wizardDlg.h"
#include "pWizardChild4.h"
#include "common.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define ROUND_DWDATA(a) (((a) + sizeof(DWDATA) - 1) / sizeof(DWDATA))
//----------------------------------------------------------------------------
#define MB_BTN (MB_OK | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
#define LEN_LB_TEXT    13
#define LEN_LB_TYPE     7
#define LEN_LB_DIM      5
#define LEN_LB_PRPH    10
#define LEN_LB_ADDR     8
#define LEN_LB_IN_LB    8

#define MAX_LEN_LB (LEN_LB_TEXT + LEN_LB_TYPE + LEN_LB_DIM + LEN_LB_PRPH + \
                    LEN_LB_ADDR + LEN_LB_TYPE + LEN_LB_IN_LB + LEN_LB_ADDR + LEN_LB_TEXT + 10)
//----------------------------------------------------------------------------
pWizardChild4::pWizardChild4(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{

}
//----------------------------------------------------------------------------
pWizardChild4::~pWizardChild4()
{
  destroy();
}
//----------------------------------------------------------------------------
bool pWizardChild4::create()
{
  PListBox* lb = new PListBox(this, IDC_LIST_WZ_RECIPE);
  int tabs[] = { LEN_LB_TEXT, LEN_LB_TYPE, LEN_LB_DIM, LEN_LB_PRPH,
                 LEN_LB_ADDR, LEN_LB_TYPE, LEN_LB_IN_LB, LEN_LB_ADDR, LEN_LB_TEXT };
  lb->SetTabStop(SIZE_A(tabs), tabs, 0);
  for(uint i = 1; i < SIZE_A(tabs) - 1; ++i)
    lb->setAlign(i, PListBox::aCenter);

  if(!baseClass::create())
    return false;

  lb->setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT), false);
  lb->setIntegralHeight();

  fillCBPerif(GetDlgItem(*this, IDC_COMBO_WZ_RCP_MEM), 0);
  fillCBPerif(GetDlgItem(*this, IDC_COMBO_WZ_RCP_PRPH), 0);

  fillCBTypeVal(GetDlgItem(*this, IDC_COMBO_WZ_RCP_TYPE_MEM), 0);
  fillCBTypeVal(GetDlgItem(*this, IDC_COMBO_WZ_RCP_TYPE_PRPH), 0);

  return true;
}
//----------------------------------------------------------------------------
#define NAME_WIZ4   _T("Ricette")
#define NAME_WIZ4LB _T("Ricette con Listbox")
//----------------------------------------------------------------------------
void pWizardChild4::getTitle(LPTSTR append, size_t lenBuff)
{
  switch(getItem()) {
    case pRecipe:
      _tcscpy_s(append, lenBuff, NAME_WIZ4);
      break;
    case pRecipeLB:
      _tcscpy_s(append, lenBuff, NAME_WIZ4LB);
      break;
    default:
      baseClass::getTitle(append, lenBuff);
      break;
    }
}
//----------------------------------------------------------------------------
void pWizardChild4::setItem(int ix)
{
  baseClass::setItem(ix);
  bool enable = pRecipeLB == getItem();

  uint ids[] = { IDC_CHECK_IN_LB, IDC_EDT_WZ_RCP_ADDR_LB, IDC_EDIT_WZ_RCP_LABEL_FLT };
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], enable);
}
//----------------------------------------------------------------------------
LRESULT pWizardChild4::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_WZ_RCP_ADD:
          addRow(false);
          break;
        case IDC_BUTTON_WZ_RCP_REM:
          remRow();
          break;
        case IDC_BUTTON_WZ_RCP_MOD:
          modRow();
          break;
        case IDC_BUTTON_WZ_RCP_UP:
          moveRow(true);
          break;
        case IDC_BUTTON_WZ_RCP_DN:
          moveRow(false);
          break;
        case IDC_EDIT_WZ_RCP_INIT_ADDR:
        case IDC_EDIT_WZ_RCP_LEN_NAME:
          switch(HIWORD(wParam)) {
//            case EN_KILLFOCUS:
            case EN_CHANGE:
              refreshAddr();
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void pWizardChild4::refreshAddr()
{
  int init;
  GET_INT(IDC_EDIT_WZ_RCP_INIT_ADDR, init);
  int len;
  GET_INT(IDC_EDIT_WZ_RCP_LEN_NAME, len);
  len = ROUND_DWDATA(len);
  SET_INT(IDC_STATIC_TRUE_INIT_ADDR, init - len * 2);
}
//----------------------------------------------------------------------------
void pWizardChild4::addRow(bool append)
{
  TCHAR buff[MAX_LEN_LB + 10];
  fillStr(buff, _T(' '), MAX_LEN_LB);
  LPTSTR p = buff;

  TCHAR t[200];
  GET_TEXT(IDC_EDIT_WZ_RCP_LABEL, t);
  int len = _tcslen(t);
  len = min(len, LEN_LB_TEXT);
  copyStr(p, t, len);
  p += LEN_LB_TEXT;
  *p++ = _T('\t');

  int pos = SendMessage(GetDlgItem(*this, IDC_COMBO_WZ_RCP_TYPE_MEM), CB_GETCURSEL, 0, 0);
  if(pos < 0)
    pos = 0;
  wsprintf(t, _T("%d"), pos);
  len = _tcslen(t);
  copyStr(p, t, len);
  p += LEN_LB_TYPE;
  *p++ = _T('\t');

  GET_INT(IDC_EDIT_WZ_RCP_DIM, pos);
  wsprintf(t, _T("%d"), pos);
  len = _tcslen(t);
  copyStr(p, t, len);
  p += LEN_LB_DIM;
  *p++ = _T('\t');

  pos = SendMessage(GetDlgItem(*this, IDC_COMBO_WZ_RCP_PRPH), CB_GETCURSEL, 0, 0);
  if(pos < 0)
    pos = 0;
  wsprintf(t, _T("%d"), pos);
  len = _tcslen(t);
  copyStr(p, t, len);
  p += LEN_LB_PRPH;
  *p++ = _T('\t');

  GET_INT(IDC_EDT_WZ_RCP_ADDR_PRPH, pos);
  wsprintf(t, _T("%d"), pos);
  len = _tcslen(t);
  copyStr(p, t, len);
  p += LEN_LB_ADDR;
  *p++ = _T('\t');

  pos = SendMessage(GetDlgItem(*this, IDC_COMBO_WZ_RCP_TYPE_PRPH), CB_GETCURSEL, 0, 0);
  if(pos < 0)
    pos = 0;
  wsprintf(t, _T("%d"), pos);
  len = _tcslen(t);
  copyStr(p, t, len);
  p += LEN_LB_TYPE;
  *p++ = _T('\t');

  if(pRecipeLB == getItem() && IS_CHECKED(IDC_CHECK_IN_LB)) {
    *p = _T('X');
    p += LEN_LB_IN_LB;
    *p++ = _T('\t');
    GET_INT(IDC_EDT_WZ_RCP_ADDR_LB, pos);
    wsprintf(t, _T("%d"), pos);
    len = _tcslen(t);
    copyStr(p, t, len);
    p += LEN_LB_ADDR;
    *p++ = _T('\t');

    GET_TEXT(IDC_EDIT_WZ_RCP_LABEL_FLT, t);
    len = _tcslen(t);
    len = min(len, LEN_LB_TEXT);
    copyStr(p, t, len);
    p += LEN_LB_TEXT;
    *p++ = _T('\t');
    }
  else {
    p += LEN_LB_IN_LB;
    *p++ = _T('\t');
    p += LEN_LB_ADDR;
    *p++ = _T('\t');
    p += LEN_LB_TEXT;
    *p++ = _T('\t');
    }
  *p = 0;
  HWND hlb = GetDlgItem(*this, IDC_LIST_WZ_RECIPE);

  pos = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(pos >= 0 && append) {
    ++pos;
    if(SendMessage(hlb, LB_GETCOUNT, 0, 0) <= pos)
      pos = -1;
    }

  SendMessage(hlb, LB_INSERTSTRING, pos, (LPARAM)buff);

  SendMessage(hlb, LB_SETCURSEL, pos, 0);
  SendMessage(hlb, LB_SETCARETINDEX, pos, FALSE);

}
//----------------------------------------------------------------------------
void pWizardChild4::remRow()
{
  remFromLb(GetDlgItem(*this, IDC_LIST_WZ_RECIPE));
}
//----------------------------------------------------------------------------
void pWizardChild4::modRow()
{
  int append = remFromLb(GetDlgItem(*this, IDC_LIST_WZ_RECIPE));
  if(-1 == append)
    return;
  addRow(toBool(append));
}
//----------------------------------------------------------------------------
void pWizardChild4::moveRow(bool up)
{
  moveOnLb(GetDlgItem(*this, IDC_LIST_WZ_RECIPE), up);
}
//----------------------------------------------------------------------------
static LPCTSTR stdText[] =
{
  _T("1000018,Warning\r\n"),
  _T("1000017,Are You Sure to erase the Recipe ?\r\n"),
  _T("1000015,Messagge\r\n"),
  _T("1000014,Actual Data Copied. Press ENTER key to continue\r\n"),
  _T("1000013,Message\r\n"),
  _T("1000012,Recipe Sended. Press ENTER key to continue\r\n"),
  _T("1000011,Message\r\n"),
  _T("1000010,Recipe Write successfully. Press Enter to continue....\r\n"),
  _T("1000009,Attention\r\n"),
  _T("1000008,Impossible to send a data whit the Machine in Run\r\n"),
};
//----------------------------------------------------------------------------
struct infoRow
{
  TCHAR edLabel[LEN_LB_TEXT + 2];
  int edType;
  int edDim;

  int prph;
  int addr;
  int type;

  bool onLb;

  int fltAddr;
  TCHAR fltLabel[LEN_LB_TEXT + 2];

//  infoRow() : edLabel(0), fltLabel(0) {}
//  ~infoRow() { delete []edLabel; delete []fltLabel; }
};
//----------------------------------------------------------------------------
void pWizardChild4::unfillRow(LPTSTR row, infoRow& ir)
{
//  int tabs[] = { LEN_LB_TEXT, LEN_LB_TYPE, LEN_LB_DIM, LEN_LB_PRPH,
//                 LEN_LB_ADDR, LEN_LB_TYPE, LEN_LB_IN_LB, LEN_LB_ADDR, LEN_LB_TEXT };
  LPCTSTR p = row;
  TCHAR t[LEN_LB_TEXT + 2];
  copyStr(t, p, LEN_LB_TEXT);
  t[LEN_LB_TEXT] = 0;
  _tcscpy_s(ir.edLabel, trim(t));

  p += LEN_LB_TEXT + 1;
  ir.edType = _ttoi(p);

  p += LEN_LB_TYPE + 1;
  ir.edDim = _ttoi(p);

  p += LEN_LB_DIM + 1;
  ir.prph = _ttoi(p);

  p += LEN_LB_PRPH + 1;
  ir.addr = _ttoi(p);

  p += LEN_LB_ADDR + 1;
  ir.type = _ttoi(p);

  p += LEN_LB_TYPE + 1;
  ir.onLb = _T('X') == *p;

  p += LEN_LB_IN_LB + 1;
  ir.fltAddr = _ttoi(p);

  p += LEN_LB_ADDR + 1;
  copyStr(t, p, LEN_LB_TEXT);
  t[LEN_LB_TEXT] = 0;
  _tcscpy_s(ir.fltLabel, trim(t));

}
//----------------------------------------------------------------------------
bool pWizardChild4::savePrph(P_File* pf, LPTSTR buff, uint id, PVect<infoRow*>& vIr)
{
  wsprintf(buff, _T("%d"), id);
  if(!writeStringForceUnicode(*pf, buff))
    return false;
  int count = vIr.getElem();
  for(int i = 0; i < count; ++i) {
    wsprintf(buff, _T(",%d"), vIr[i]->prph);
    if(!writeStringForceUnicode(*pf, buff))
      return false;
    }
  return toBool(writeStringForceUnicode(*pf, _T("\r\n")));
}
//----------------------------------------------------------------------------
bool pWizardChild4::saveAddr(P_File* pf, LPTSTR buff, uint id, PVect<infoRow*>& vIr)
{
  wsprintf(buff, _T("%d"), id);
  if(!writeStringForceUnicode(*pf, buff))
    return false;
  int count = vIr.getElem();
  for(int i = 0; i < count; ++i) {
    wsprintf(buff, _T(",%d"), vIr[i]->addr);
    if(!writeStringForceUnicode(*pf, buff))
      return false;
    }
  return toBool(writeStringForceUnicode(*pf, _T("\r\n")));
}
//----------------------------------------------------------------------------
bool pWizardChild4::saveType(P_File* pf, LPTSTR buff, uint id, PVect<infoRow*>& vIr)
{
  wsprintf(buff, _T("%d"), id);
  if(!writeStringForceUnicode(*pf, buff))
    return false;
  int count = vIr.getElem();
  for(int i = 0; i < count; ++i) {
    wsprintf(buff, _T(",%d"), vIr[i]->type);
    if(!writeStringForceUnicode(*pf, buff))
      return false;
    }
  return writeStringForceUnicode(*pf, _T("\r\n")) > 0;
}
//----------------------------------------------------------------------------
bool pWizardChild4::saveDim(P_File* pf, LPTSTR buff, uint id, PVect<infoRow*>& vIr)
{
  wsprintf(buff, _T("%d"), id);
  if(!writeStringForceUnicode(*pf, buff))
    return false;
  int count = vIr.getElem();
  for(int i = 0; i < count; ++i) {
    if(14 == vIr[i]->type)
      wsprintf(buff, _T(",%d"), vIr[i]->edDim);
    else
      wsprintf(buff, _T(",%d"), 0);
    if(!writeStringForceUnicode(*pf, buff))
      return false;
    }
  return writeStringForceUnicode(*pf, _T("\r\n")) > 0;
}
//----------------------------------------------------------------------------
int getDimType(int type, int def)
{
  if(type <= 5)
    return 1;
  if(type <= 10)
    return 2;
  return ROUND_DWDATA(def);
}
//----------------------------------------------------------------------------
struct id_info
{
  uint id;
  LPCTSTR text;
} idInfo[] = {
    { 1, _T("Recipe") },
    { 2, _T("Previous page") },
    { 3, _T("Open Text Note") },
    { 6, _T("Save Recipe") },
    { 8, _T("Active Recipe") },
    { 10, _T("Copy Actual Data") },
    { 12, _T("Erase Recipe") },
    { 13, _T("Alarms") },
    { 100, _T("16,0,0,arial") },

    { 32, _T("4,0") },

    { 4, _T("Load Recipe") },
};
//----------------------------------------------------------------------------
id_info idInfoLb[] = {
    { 4, _T("List All Recipe") },
    { 5, _T("List Filtered Recipe") },
    { 9, _T("Load Recipe from List") },
    { 11, _T("Default Data") },
};
//----------------------------------------------------------------------------
bool pWizardChild4::save(setOfString& Set)
{
  P_File* pf = makeFileTmpCustom(true);
  if(!pf)
    return false;

  P_File* pfClear = makeFileTmpCustom(false);
  if(!pfClear) {
    delete pf;
    return false;
    }
  bool result = performSave(Set, pf, pfClear);
  delete pf;
  delete pfClear;
  return result;
}
//----------------------------------------------------------------------------
bool pWizardChild4::performSave(setOfString& Set, P_File* pf, P_File* pfClear)
{
  Set.reset();

  HWND hlb = GetDlgItem(*this, IDC_LIST_WZ_RECIPE);

  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  if(count <= 0)
    return false;

  int prphMem = SendMessage(GetDlgItem(*this, IDC_COMBO_WZ_RCP_MEM), CB_GETCURSEL, 0, 0);
  if(prphMem <= 0)
    return false;
  int initAddr;
  GET_INT(IDC_EDIT_WZ_RCP_INIT_ADDR, initAddr);
  int lenName;
  GET_INT(IDC_EDIT_WZ_RCP_LEN_NAME, lenName);

  for(uint i = 0; i < SIZE_A(stdText); ++i)
    if(!writeStringForceUnicode(*pfClear, stdText[i]))
     return false;

  TCHAR buff[500];
  int addrName = initAddr - ROUND_DWDATA(lenName) * 2;
  int addrCurrName = addrName + ROUND_DWDATA(lenName);
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d\r\n"), 1001001, initAddr, addrName, lenName,
          addrCurrName, 0, prphMem);
  if(!writeStringForceUnicode(*pf, buff))
    return false;

  PVect<infoRow*> vIr;

  for(int i = 0; i < count; ++i) {
    SendMessage(hlb, LB_GETTEXT, i, (LPARAM)buff);
    infoRow* ir =new infoRow;
    unfillRow(buff, *ir);
    vIr[i] = ir;
    }

  if(!savePrph(pf, buff, 1001002, vIr))
    return false;
  if(!saveAddr(pf, buff, 1001003, vIr))
    return false;
  if(!saveType(pf, buff, 1001004, vIr))
    return false;
  if(!saveDim(pf, buff, 1001005, vIr))
    return false;

  if(!savePrph(pf, buff, 1001101, vIr))
    return false;
  if(!saveAddr(pf, buff, 1001102, vIr))
    return false;
  if(!saveType(pf, buff, 1001103, vIr))
    return false;
  if(!saveDim(pf, buff, 1001104, vIr))
    return false;

  int nFieldFlt = 0;
  if(pRecipeLB == getItem()) {
    if(!writeStringForceUnicode(*pf, _T("1001300,26,2,comic sans ms\r\n1001301")))
      return false;
    PVect<int> vAddr;
    PVect<int> vDim;
    PVect<int> vDec;
    for(int i = 1; i < count; ++i) {
      if(vIr[i]->onLb) {
        int ix = vAddr.getElem();
        vAddr[ix] = vIr[i]->fltAddr;
        if(vAddr[ix])
          ++nFieldFlt;
        vDim[ix] = 14 == vIr[i]->edType ? vIr[i]->edDim : 10;
        if(5 == vIr[i]->edType || 7 == vIr[i]->edType)
          vDec[ix] = 2;
        else
          vDec[ix] = 0;
        wsprintf(buff, _T(",%d"), i);
        if(!writeStringForceUnicode(*pf, buff))
          return false;
        }
      }
    if(!writeStringForceUnicode(*pf, _T("\r\n")))
      return false;
    count = vAddr.getElem();

    if(!writeStringForceUnicode(*pf, _T("1001302")))
      return false;
    for(int i = 0; i < count; ++i) {
      wsprintf(buff, _T(",%d"), vAddr[i]);
      if(!writeStringForceUnicode(*pf, buff))
        return false;
      }
    if(!writeStringForceUnicode(*pf, _T("\r\n")))
      return false;

    if(!writeStringForceUnicode(*pf, _T("1001303")))
      return false;
    for(int i = 0; i < count; ++i) {
      wsprintf(buff, _T(",%d"), vDim[i]);
      if(!writeStringForceUnicode(*pf, buff))
        return false;
      }
    if(!writeStringForceUnicode(*pf, _T("\r\n")))
      return false;

    if(!writeStringForceUnicode(*pf, _T("1001304")))
      return false;
    for(int i = 0; i < count; ++i) {
      wsprintf(buff, _T(",%d"), vDec[i]);
      if(!writeStringForceUnicode(*pf, buff))
        return false;
      }
    if(!writeStringForceUnicode(*pf, _T("\r\n")))
      return false;
    for(uint i = 0; i < SIZE_A(idInfoLb); ++i)
      Set.replaceString(idInfoLb[i].id, str_newdup(idInfoLb[i].text), true);
    }
  else {
    int ix = SIZE_A(idInfo) - 1;
    Set.replaceString(idInfo[ix].id, str_newdup(idInfo[ix].text), true);
    }

  for(uint i = 0; i < SIZE_A(idInfo) - 1; ++i)
    Set.replaceString(idInfo[i].id, str_newdup(idInfo[i].text), true);


  int nField = vIr.getElem();
  int x = 10;
  int y = 10;
  int w = 100;
  int h = 24;
  int initI = 0;
  int initA = initAddr;
  if(lenName) {
    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), x, y, w, h,
                      100, 0, 128, 255, 196, 225, 255, 1, 0, 800001);
    Set.replaceString(2001, str_newdup(buff), true);

    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), x + w + 2, y, w, h,
                      100, 0, 70, 140, 196, 225, 255, lenName, 0);
    Set.replaceString(4001, str_newdup(buff), true);

    wsprintf(buff, _T("%d,%d,%d,%d,%d"), prphMem, addrName, 14, 0, lenName);
    Set.replaceString(4301, str_newdup(buff), true);
    Set.replaceString(800001, str_newdup(vIr[0]->edLabel), true);
    ++initI;
    y += h + 2;
    ++initA;
    }

  for(int i = initI; i < nField; ++i) {
    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), x, y, w, h,
                      100, 0, 64, 0, 187, 255, 187, 1, 0, 800001 + i);
    Set.replaceString(2001 + i, str_newdup(buff), true);

    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), x, y + h + 2, w, h,
                      100, 0, 64, 0, 187, 255, 187, vIr[i]->edDim, 0);
    Set.replaceString(4001 + i, str_newdup(buff), true);

    wsprintf(buff, _T("%d,%d,%d,%d,%d"), prphMem, initA, vIr[i]->edType, 0, 0);//vIr[i]->edDim);
    Set.replaceString(4301 + i, str_newdup(buff), true);

    Set.replaceString(800001 + i, str_newdup(vIr[i]->edLabel), true);

    initA += getDimType(vIr[i]->edType, vIr[i]->edDim);

    x += w + 2;
    if(x + w >= 790) {
      x = 10;
      y += h * 2 + 2;
      }
    }
  if(nFieldFlt) {
    x = 10;
    y += h * 3 + 2;
    int ix = nField;
    for(int i = 1; i < nField; ++i) {
      if(vIr[i]->onLb && vIr[i]->fltAddr) {

        wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), x, y, w, h,
                      100, 0, 0, 255, 255, 255, 128, 1, 0, 800001 + ix);
        Set.replaceString(2001 + ix, str_newdup(buff), true);

        wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), x, y + h + 2, w, h,
                      100, 0, 0, 255, 255, 255, 128, vIr[i]->edDim, 0);
        Set.replaceString(4001 + ix, str_newdup(buff), true);

        wsprintf(buff, _T("%d,%d,%d,%d,%d"), prphMem, vIr[i]->fltAddr, vIr[i]->edType,
                    0, vIr[i]->edDim);
        Set.replaceString(4301 + ix, str_newdup(buff), true);

        Set.replaceString(800001 + ix, str_newdup(vIr[i]->fltLabel), true);
        ++ix;
        x += w + 2;
        if(x + w >= 790) {
          x = 10;
          y += h * 2 + 2;
          }
        }
      }
    }
  x += 100;
  if(x + 300 > 790) {
    x = 10;
    y += h * 3 + 2;
    }

  if(pRecipeLB == getItem()) {
    wsprintf(buff, _T("%d,%d,%d,%d,%d"), x, y, 300, 100, 3);
    Set.replaceString(9001, str_newdup(buff), true);
    Set.replaceString(9301, str_newdup(_T("0,0,0,242,245,177,156,54,20,224,221,84")), true);
    Set.replaceString(48, str_newdup(_T("1")), true);
    }
  nField += nFieldFlt;
  wsprintf(buff, _T("%d"), nField);
  Set.replaceString(41, str_newdup(buff), true);
  Set.replaceString(43, str_newdup(buff), true);

  dataProject& dp = getDataProject();
  dp.setPageType(pRecipeLB == getItem() ? ptcRecipeListbox : ptcRecipe);
  return Set.setFirst();
}
