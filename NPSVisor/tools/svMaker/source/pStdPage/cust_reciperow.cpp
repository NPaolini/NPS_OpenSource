//--------- cust_recipeRow.cpp --------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include "macro_utils.h"
#include "pListbox.h"
#include "cust_recipeRow.h"
#include "svmProperty.h"
#include "infoText.h"
#include "common.h"
//----------------------------------------------------------------------------
#define ID_FIRST_NUM                1000001
#define ID_NUM_ROW                  1000002
#define ID_NUM_COL                  1000003
#define ID_FIRST_EDIT               1000004
//#define ID_WORD_PLC                 1000005
#define ID_INIT_RECIPE_ROW_IN_MEM   1000006
/*
#define ID_USE_FIRST_EDIT_FOR_TEXT  1000007
#define ID_RECIPE_ROW_FILE          1000008

#define ID_SAVE_NAME_RECIPE         1000010
#define ID_BIT_SEND                 1000011
#define ID_BIT_OKSEND               1000012
#define ID_PSW_LEVEL_SEND_RR        1000013
#define ID_PSW_LEVEL_SAVE_RR        1000014
*/
//----------------------------------------------------------------------------
#define ID_INIT_ADDR_PLC_RECIPEROW  1000005
//----------------------------------------------------------------------------
#define ID_IS_FIRST_TEXT            1000007
//----------------------------------------------------------------------------
#define ID_RECIPEROW_NAME           1000008
//----------------------------------------------------------------------------
#define ID_SAVE_ACTIVE_ROW          1000010
//----------------------------------------------------------------------------
#define ID_ENABLE_SEND              1000011
#define ID_ACTIVE_SEND              1000012
//----------------------------------------------------------------------------
#define ID_PSW_SEND                 1000013
#define ID_PSW_SAVE                 1000014
//----------------------------------------------------------------------------
#define ID_INDICATOR_BMP            1000015
//----------------------------------------------------------------------------
typedef PVect<int> pVectInt;
//----------------------------------------------------------------------------
static
void fillSet(pVectInt& v, LPCTSTR p, int offset = 0)
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
//----------------------------------------------------------------------------
bool runCustomRecipeRow(PWin* parent)
{
  return IDOK == cust_recipeRow(parent).modal();
}
//----------------------------------------------------------------------------
cust_recipeRow::cust_recipeRow(PWin* parent, uint resId, HINSTANCE hinstance) :
    PModDialog(parent, resId, hinstance),
        Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
        Brush3(CreateSolidBrush(bkgColor3)),
        hasFirstText(true), nCol(0),
        FirstNum(2001), NumRow(0), NumCol(0), FirstEdit(4001), InitOnMem(0)
{
}
//-------------------------------------------------------------------
cust_recipeRow::~cust_recipeRow()
{
  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
  DeleteObject(HGDIOBJ(Brush3));
  destroy();
}
//-------------------------------------------------------------------
static void hideCtrl(HWND par)
{
  uint ctrl[] = {
    IDC_COMBO_RROW_PRPH_SAVE_ACTIVE,
    IDC_EDIT_RROW_ADDR_SAVE_ACTIVE,
//    IDC_EDIT_RROW_ADDR_PLC_1,
    };
  for(uint i = 0; i < SIZE_A(ctrl); ++i)
    EnableWindow(GetDlgItem(par, ctrl[i]), FALSE);

}
//-------------------------------------------------------------------
bool cust_recipeRow::create()
{
  if(!baseClass::create())
    return false;

  loadData();
  return true;
}
//------------------------------------------------------------------------------
struct dataPrphSet
{
  uint id;
  uint idcPrph;
  uint idcAddr;
  uint idcDataType;
  uint idcNbit;
  uint idcOffs;
  void load(setOfString& set, HWND par);
  bool save(P_File* pf, HWND par);

  dataPrphSet(uint id = 0, uint prph = 0, uint addr = 0, uint dataType = 0, uint nbit = 0, uint offs = 0)
    : id(id), idcPrph(prph), idcAddr(addr), idcDataType(dataType), idcNbit(nbit), idcOffs(offs) {}
};
//------------------------------------------------------------------------------
void dataPrphSet::load(setOfString& set, HWND par)
{
  uint prph = 0;
  int addr = 0;
  uint dataType = 0;
  uint nbit = 0;
  uint offs = 0;
  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &prph, &addr, &dataType, &nbit, &offs);
  fillCBPerif(GetDlgItem(par, idcPrph), prph);
  SetDlgItemInt(par, idcAddr, addr, 0);
  fillCBTypeVal(GetDlgItem(par, idcDataType), dataType);
  SetDlgItemInt(par, idcNbit, nbit, 0);
  SetDlgItemInt(par, idcOffs, offs, 0);
}
//------------------------------------------------------------------------------
bool dataPrphSet::save(P_File* pf, HWND par)
{
  uint prph = SendMessage(GetDlgItem(par, idcPrph), CB_GETCURSEL, 0 ,0);
  int addr = GetDlgItemInt(par, idcAddr, 0 ,TRUE);
  uint dataType = SendMessage(GetDlgItem(par, idcDataType), CB_GETCURSEL, 0 ,0);
  uint nbit = GetDlgItemInt(par, idcNbit, 0 ,0);
  uint offs = GetDlgItemInt(par, idcOffs, 0 ,0);
  TCHAR buff[500];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d\r\n"), id, prph, addr, dataType, nbit, offs);
  return toBool(writeStringForceUnicode(*pf, buff));
}
//------------------------------------------------------------------------------
#define MAX_COLUMN  20
#define MAX_DIM_FILENAME 30
//------------------------------------------------------------------------------
void cust_recipeRow::loadData()
{
  setOfString set;
  loadTmpCustom(set);

  LPCTSTR p = set.getString(ID_PSW_SEND);
  uint lev = p ? _ttoi(p) : 0;
  if(lev > 4)
    lev = 4;
  fillCBPswLevel(GetDlgItem(*this, IDC_COMBO_RROW_PSW_SEND), lev);

  p = set.getString(ID_PSW_SAVE);
  lev = p ? _ttoi(p) : 0;
  if(lev > 4)
    lev = 4;
  fillCBPswLevel(GetDlgItem(*this, IDC_COMBO_RROW_PSW_SAVE), lev);

  p = set.getString(ID_IS_FIRST_TEXT);
  hasFirstText = p && _ttoi(p);

  if(!hasFirstText)
    hideCtrl(*this);
  else {
    EnableWindow(GetDlgItem(*this, IDC_EDIT_RROW_ADDR_PLC_1), FALSE);
    p = set.getString(ID_SAVE_ACTIVE_ROW);
    uint prph = p ? _ttoi(p) : 0;
    p = findNextParam(p, 1);
    uint addr = p ? _ttoi(p) : 0;
    fillCBPerif(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_SAVE_ACTIVE), prph);
    SetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_SAVE_ACTIVE, addr, 0);
    }

  dataPrphSet(ID_ACTIVE_SEND, IDC_COMBO_RROW_PRPH_ACTIVE, IDC_EDIT_RROW_ADDR_ACTIVE,
        IDC_COMBO_RROW_TYPE_ACTIVE, IDC_EDIT_RROW_NBIT_ACTIVE, IDC_EDIT_RROW_OFFS_ACTIVE).load(set, *this);

  dataPrphSet(ID_ENABLE_SEND, IDC_COMBO_RROW_PRPH_ENABLE, IDC_EDIT_RROW_ADDR_ENABLE,
        IDC_COMBO_RROW_TYPE_ENABLE, IDC_EDIT_RROW_NBIT_ENABLE, IDC_EDIT_RROW_OFFS_ENABLE).load(set, *this);

  p = set.getString(ID_RECIPEROW_NAME);
  if(p)
    SET_TEXT(IDC_EDIT_RROW_FILENAME, p);

  p = set.getString(ID_INDICATOR_BMP);
  if(!p)
    SET_TEXT(IDC_EDIT_RROW_INDICATOR, _T("image\\\\indicator.bmp"));

  else {
    TCHAR t[_MAX_PATH];
    translateFromCRNL(t, p);
    SET_TEXT(IDC_EDIT_RROW_INDICATOR, t);
    }
  p = set.getString(ID_INIT_ADDR_PLC_RECIPEROW);
  pVectInt V;
  fillSet(V, p);

  uint nElem = V.getElem();
  uint ix = 1;

  if(nElem) {
    SetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_PLC_NUM_ACTIVE, V[0], TRUE);
    for(; ix < nElem && ix < MAX_COLUMN; ++ix)
      SetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_PLC_1 + ix - 1, V[ix], TRUE);
    }
  for(; ix <= MAX_COLUMN; ++ix) {
    ShowWindow(GetDlgItem(*this, IDC_EDIT_RROW_ADDR_PLC_1 + ix - 1), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_STATIC_IX_RROW_1 + ix - 1), SW_HIDE);
    }
  nCol = nElem - 1;

  if(nCol <= MAX_COLUMN / 2) {
    PRect r;
    GetWindowRect(*this, r);
    PRect rc;
    GetWindowRect(GetDlgItem(*this, IDC_STATIC_IX_RROW_11), rc);
    r.right = rc.left;
    SetWindowPos(*this, 0, 0, 0, r.Width(), r.Height(), SWP_NOMOVE | SWP_NOZORDER);
    }
  p = set.getString(ID_FIRST_NUM);
  if(p)
    FirstNum = _ttoi(p);
  p = set.getString(ID_NUM_ROW);
  if(p)
    NumRow = _ttoi(p);
  p = set.getString(ID_NUM_COL);
  if(p)
    NumCol = _ttoi(p);
  p = set.getString(ID_FIRST_EDIT);
  if(p)
    FirstEdit = _ttoi(p);
  p = set.getString(ID_INIT_RECIPE_ROW_IN_MEM);
  if(p)
    InitOnMem = _ttoi(p);
}
//------------------------------------------------------------------------------
LRESULT cust_recipeRow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_RROW_SAVE:
          CmOk();
          break;
        }
      break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//------------------------------------------------------------------------------
HBRUSH cust_recipeRow::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  long id = GetWindowLong(hWndChild, GWL_ID);
  if(WM_CTLCOLORSTATIC == ctlType) {
    if(id >= IDC_STATIC_IX_RROW_1 && id <= IDC_STATIC_IX_RROW_20) {
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
    SetBkColor(hdc, bkgColor3);
    return (Brush3);

    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, bkgColor);
    return (Brush);
    }
  return 0;
}
//------------------------------------------------------------------------------
void cust_recipeRow::CmOk()
{
  if(saveData())
    baseClass::CmOk();
}
//------------------------------------------------------------------------------
bool cust_recipeRow::saveData()
{

  P_File* pf = makeFileTmpCustom(true);
  if(!pf)
    return false;

  P_File* pfClear = makeFileTmpCustom(false);
  if(!pfClear) {
    delete pf;
    return false;
    }

  bool success = true;
  TCHAR buff[500];

  do {
    int ids[] = { ID_FIRST_NUM, ID_NUM_ROW, ID_NUM_COL, ID_FIRST_EDIT, ID_INIT_RECIPE_ROW_IN_MEM };
    int vals[] = { FirstNum, NumRow, NumCol, FirstEdit, InitOnMem };
    for(uint i = 0; i < SIZE_A(ids); ++i) {
      wsprintf(buff, _T("%d,%d\r\n"), ids[i], vals[i]);
      if(!writeStringForceUnicode(*pf, buff))
        break;
      }
    } while(false);

  do {
    if(hasFirstText) {
      wsprintf(buff, _T("%d,%d\r\n"), ID_IS_FIRST_TEXT, 1);
      if(!writeStringForceUnicode(*pf, buff))
        break;
      uint prph = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_SAVE_ACTIVE), CB_GETCURSEL, 0 ,0);
      uint addr = GetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_SAVE_ACTIVE, 0 ,0);
      wsprintf(buff, _T("%d,%d,%d\r\n"), ID_SAVE_ACTIVE_ROW, prph, addr);
      if(!writeStringForceUnicode(*pf, buff))
        break;
      }
    if(!dataPrphSet(ID_ACTIVE_SEND, IDC_COMBO_RROW_PRPH_ACTIVE, IDC_EDIT_RROW_ADDR_ACTIVE,
        IDC_COMBO_RROW_TYPE_ACTIVE, IDC_EDIT_RROW_NBIT_ACTIVE, IDC_EDIT_RROW_OFFS_ACTIVE).save(pf, *this))
      break;

    if(!dataPrphSet(ID_ENABLE_SEND, IDC_COMBO_RROW_PRPH_ENABLE, IDC_EDIT_RROW_ADDR_ENABLE,
        IDC_COMBO_RROW_TYPE_ENABLE, IDC_EDIT_RROW_NBIT_ENABLE, IDC_EDIT_RROW_OFFS_ENABLE).save(pf, *this))
      break;

    uint pswSend = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_PSW_SEND), CB_GETCURSEL,0 ,0);
    if(pswSend > 3)
      pswSend = 1000;
    uint pswSave = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_PSW_SAVE), CB_GETCURSEL,0 ,0);
    if(pswSave > 3)
      pswSave = 1000;

    wsprintf(buff, _T("%d,%d\r\n%d,%d\r\n"), ID_PSW_SEND, pswSend, ID_PSW_SAVE, pswSave);
    if(!writeStringForceUnicode(*pf, buff))
      break;

    TCHAR name[_MAX_PATH];
    GetDlgItemText(*this, IDC_EDIT_RROW_FILENAME, name, MAX_DIM_FILENAME);
    wsprintf(buff, _T("%d,%s\r\n"), ID_RECIPEROW_NAME, name);
    if(!writeStringForceUnicode(*pf, buff))
      break;

    GET_TEXT(IDC_EDIT_RROW_INDICATOR, name);
    trim(lTrim(name));
    if(*name) {
      wsprintf(buff, _T("%d,%s\r\n"), ID_INDICATOR_BMP, name);
      if(!writeStringForceUnicode(*pf, buff))
        break;
      }

    uint addr4SendActive = GetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_PLC_NUM_ACTIVE, 0 ,0);
    wsprintf(buff, _T("%d,%d"), ID_INIT_ADDR_PLC_RECIPEROW, addr4SendActive);
    uint i = 0;
    if(hasFirstText) {
      _tcscat_s(buff, _T(",-1"));
      ++i;
      }
    for(; i < nCol; ++i) {
      int v = GetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_PLC_1 + i, 0, TRUE);
      wsprintf(name, _T(",%d"), v);
      _tcscat_s(buff, name);
      }
    _tcscat_s(buff, _T("\r\n"));
    if(!writeStringForceUnicode(*pf, buff))
      break;

    success = true;
    } while(false);

  delete pf;
  delete pfClear;

  return success;
}
//-------------------------------------------------------------------
