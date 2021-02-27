//--------- cust_recipe.cpp --------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include "macro_utils.h"
#include "pListbox.h"
#include "cust_recipe.h"
#include "svmProperty.h"
#include "infoText.h"
#include "common.h"
//----------------------------------------------------------------------------
#define ID_DEFAULT_EDITOR 1000005
#define ID_FLAG_PLC   1000006
#define ID_OK_SEND    1000007
//----------------------------------------------------------------------------
#define ID_RECIPE_EXT       1000100
//----------------------------------------------------------------------------
#define ID_INIT_DATA_RECIPE     1001001
#define ID_END_DATA_RECIPE      (ID_INIT_DATA_RECIPE + 10)
//----------------------------------------------------------------------------
#define ID_INIT_ACT_DATA        1001101
#define ID_END_ACT_DATA         (ID_INIT_ACT_DATA + 3)
//----------------------------------------------------------------------------
#define ID_RECIPE_ABS_PATH  1001151
#define ID_RECIPE_SUB_PATH  1001152
//----------------------------------------------------------------------------
#define ID_RECIPE_BIT_TO_FUNCT 1001155
//----------------------------------------------------------------------------
#define ID_INIT_DEFAULT_VALUE   1001201
#define ID_END_DEFAULT_VALUE    (ID_INIT_DEFAULT_VALUE + 3)
//----------------------------------------------------------------------------
#define ID_INIT_RECIPE_LB       1001300
//----------------------------------------------------------------------------
#define ID_INIT_NOTES           1001400
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
static infoLabel iLabel[] = {
  { 1000008, _T("Msg se bit abil. non attivo") },
  { 1000009, _T("Tit se bit abil. non attivo") },

//  { 1000010, _T("Msg Successo Salvataggio") },
//  { 1000011, _T("Tit Successo Salvataggio") },

  { 1000012, _T("Msg spedita con successo") },
  { 1000013, _T("Tit spedita con successo") },

  { 1000014, _T("Msg Dati copiati in memoria") },
  { 1000015, _T("Tit Dati copiati in memoria") },

  { 1000017, _T("Richiesta cancellazione ricetta") },
  { 1000018, _T("Tit richiesta canc. ricetta") },

//  { 1000100, _T("EStensione (default .rcp)") },

  };
//----------------------------------------------------------------------------
#define LEN_LB_DIM_ID   9
#define LEN_LB_INFO    28
#define LEN_LB_TEXT   1024
//----------------------------------------------------------------------------
#define MAX_LEN_LB   (LEN_LB_DIM_ID + LEN_LB_INFO + LEN_LB_TEXT + 5)
//----------------------------------------------------------------------------
#define LEN_S_PRPH    6
#define LEN_S_ADDR    6
#define LEN_S_TYPE    8
#define LEN_S_DIM     6
#define LEN_S_NOTES   255
//----------------------------------------------------------------------------
#define MAX_LEN_LB_SEND   ((LEN_S_PRPH + LEN_S_ADDR + LEN_S_TYPE + LEN_S_DIM + 5) * 2 + LEN_S_TYPE + 4096)
//----------------------------------------------------------------------------
#define LEN_D_DIM     4
#define LEN_D_VALUE  50
//----------------------------------------------------------------------------
#define MAX_LEN_LB_DEF   (LEN_S_ADDR + LEN_S_TYPE + LEN_D_DIM + LEN_D_VALUE + 5)
//----------------------------------------------------------------------------
#define LEN_LL_NUM    5
#define LEN_LL_ADDR   6
#define LEN_LL_DIM    6
#define LEN_LL_ALGN   7
#define LEN_LL_DEC    4
//----------------------------------------------------------------------------
#define MAX_LEN_LB_LB   (LEN_LL_NUM + LEN_LL_ADDR + LEN_LL_DIM + LEN_LL_ALGN + LEN_LL_DEC + 6)
//----------------------------------------------------------------------------
bool runCustomRecipe(PWin* parent, bool hasListbox)
{
  return IDOK == cust_recipe(parent, hasListbox).modal();
}
//----------------------------------------------------------------------------
cust_recipe::cust_recipe(PWin* parent, bool hasListbox, uint resId, HINSTANCE hinstance) :
    PModDialog(parent, resId, hinstance),
        Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
        Brush3(CreateSolidBrush(bkgColor3)),
        hasListbox(hasListbox)
{
}
//-------------------------------------------------------------------
cust_recipe::~cust_recipe()
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
    IDC_EDIT_RCP_NUM_LB,
    IDC_EDIT_RCP_ADDR_LB,
    IDC_EDIT_RCP_DIM_LB,
    IDC_COMBO_RCP_ALIGN_LB,
    IDC_EDIT_RCP_DEC_LB,
    IDC_LIST_RCP_LB,
    IDC_BUTTON_RCP_LB_ADD,
    IDC_BUTTON_RCP_LB_REM,
    IDC_BUTTON_RCP_LB_UP,
    IDC_BUTTON_RCP_LB_DN,
    IDC_EDIT_RCP_HFONT,
    IDC_EDIT_RCP_NAMEFONT,
    IDC_CHECK_RCP_UNDERL,
    IDC_CHECK_RCP_ITALIC,
    IDC_CHECK_RCP_BOLD,
    };
  for(uint i = 0; i < SIZE_A(ctrl); ++i)
    EnableWindow(GetDlgItem(par, ctrl[i]), FALSE);

}
//-------------------------------------------------------------------
bool cust_recipe::create()
{
  new langEdit(this, IDC_EDIT_RECIPE_NOTE);
  PListBox* LBTxt = new PListBox(this, IDC_LIST_RCP_TEXT);
  {
  int tabs[] = { LEN_LB_DIM_ID, LEN_LB_INFO, LEN_LB_TEXT };
  LBTxt->SetTabStop(SIZE_A(tabs), tabs, 0);
  }

  PListBox* LBSend = new PListBox(this, IDC_LIST_RCP_SEND_COPY);
  {
  int tabs[] = { LEN_S_PRPH, LEN_S_ADDR, LEN_S_TYPE, LEN_S_DIM, LEN_S_PRPH, LEN_S_ADDR, LEN_S_TYPE, LEN_S_DIM, LEN_S_TYPE, LEN_S_NOTES };
  LBSend->SetTabStop(SIZE_A(tabs), tabs, 0);
  for(uint i = 0; i < SIZE_A(tabs) - 1; ++i)
    LBSend->setAlign(i, PListBox::aCenter);
  }

  PListBox* LBDef = new PListBox(this, IDC_LIST_RCP_DEF);
  {
  int tabs[] = { LEN_S_ADDR, LEN_S_TYPE, LEN_D_DIM, LEN_D_VALUE };
  LBDef->SetTabStop(SIZE_A(tabs), tabs, 0);
  for(uint i = 0; i < SIZE_A(tabs) - 1; ++i)
    LBDef->setAlign(i, PListBox::aCenter);
  }

  PListBox* LB_LB = new PListBox(this, IDC_LIST_RCP_LB);
  {
  int tabs[] = { LEN_LL_NUM, LEN_LL_ADDR, LEN_LL_DIM, LEN_LL_ALGN, LEN_LL_DEC  };
  LB_LB->SetTabStop(SIZE_A(tabs), tabs, 0);
  for(uint i = 0; i < SIZE_A(tabs); ++i)
    LB_LB->setAlign(i, PListBox::aCenter);
  }


  if(!baseClass::create())
    return false;

  LBTxt->setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT), false);
  LBTxt->rescale(0, 0, 1.1);
  LBTxt->rescale(1, 1, 0.8);
  LBTxt->setIntegralHeight();

  LBSend->setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT), false);
  LBSend->rescale(0, 0, 0.9);
  LBSend->rescale(8, 8, 0.6);
  LBSend->setIntegralHeight();

  LBDef->setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT), false);
  LBDef->rescale(0, 0, 0.95);
  LBDef->setIntegralHeight();

  LB_LB->setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT), false);
  LB_LB->rescale(0, 1, 1.1);
  LB_LB->setIntegralHeight();

  if(!hasListbox)
    hideCtrl(*this);

  loadData();
  return true;
}
//------------------------------------------------------------------------------
void cust_recipe::loadData()
{
  setOfString set;
  loadTmpCustom(set);

  fillLBText(set);
  fillLBSend(set);
  fillLBDef(set);
  fillLBLB(set);

  int prphMem = 1;
  int psw = 0;
  LPCTSTR p = set.getString(ID_INIT_DATA_RECIPE);
  if(p) {
    int addrInit = 0;
    int addrName = 0;
    int len = 0;
    int addrCurr = 0;
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &addrInit, &addrName, &len, &addrCurr, &psw, &prphMem);
    SET_INT(IDC_EDIT_RCP_INIT_MEM, addrInit);
    SET_INT(IDC_EDIT_RCP_ADDR_NAME, addrName);
    SET_INT(IDC_EDIT_RCP_LEN_NAME, len);
    SET_INT(IDC_EDIT_RCP_ADDR_ACTIVE, addrCurr);
    }
  if(1000 == psw)
    psw = 4;
  fillCBPswLevel(GetDlgItem(*this, IDC_COMBO_RCP_PSW_LEVEL), psw);

  p = set.getString(ID_INIT_RECIPE_LB);
  if(p) {
    int h = 16;
    int style = 2;
    _stscanf_s(p, _T("%d,%d"), &h, &style);

    p = findNextParam(p, 2);
    SET_TEXT(IDC_EDIT_RCP_NAMEFONT, p);
    SET_INT(IDC_EDIT_RCP_HFONT, h);
    if(style & fITALIC)
      SET_CHECK(IDC_CHECK_RCP_ITALIC);
    if(style & fBOLD)
      SET_CHECK(IDC_CHECK_RCP_BOLD);
    if(style & fUNDERLINED)
      SET_CHECK(IDC_CHECK_RCP_UNDERL);
    }

  p = set.getString(ID_DEFAULT_EDITOR);
  if(p && _ttoi(p))
    SET_CHECK(IDC_CHECK_DEF_EDITOR);
  p = findNextParamTrim(p);
  if(p)
    SET_TEXT(IDC_EDIT_RCP_EDITOR_EXT, p);

  p = set.getString(ID_RECIPE_EXT);
  SET_TEXT(IDC_EDIT_RCP_EXT, p);

  p = set.getString(ID_RECIPE_ABS_PATH);
  SET_TEXT(IDC_EDIT_ABS_PATH, p);
  p = set.getString(ID_RECIPE_SUB_PATH);
  SET_TEXT(IDC_EDIT_SUB_PATH, p);

  int prphFlag = 2;
  p = set.getString(ID_FLAG_PLC);
  if(p) {
    int addr = 0;
    int bit = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &addr, &bit, &prphFlag);
    SET_INT(IDC_EDIT_RCP_ADDR_ACT, addr);
    SET_INT(IDC_EDIT_RCP_BIT_ACT, bit);
    }

  int prphEna = 2;
  p = set.getString(ID_OK_SEND);
  if(p) {
    int addr = 0;
    int bit = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &addr, &bit, &prphEna);
    SET_INT(IDC_EDIT_RCP_ADDR_ENA, addr);
    SET_INT(IDC_EDIT_RCP_BIT_ENA, bit);
    }

  uint idc_p[] = {
      IDC_COMBO_RCP_PRPH_SEND,
      IDC_COMBO_RCP_PRPH_COPY,
      IDC_COMBO_RCP_PRPH_MEM,
      IDC_COMBO_RCP_PRPH_ACT,
      IDC_COMBO_RCP_PRPH_ENA,
      };
  uint pos[SIZE_A_c(idc_p)] = { 0, 0, prphMem, prphFlag, prphEna };
  for(uint i = 0; i < SIZE_A(idc_p); ++i)
    fillCBPerif(GetDlgItem(*this, idc_p[i]), pos[i]);

  uint idc_t[] = {
      IDC_COMBO_RCP_TYPE_SEND,
      IDC_COMBO_RCP_TYPE_COPY,
      IDC_COMBO_RCP_TYPE_DEF,
      IDC_COMBO_RCP_TYPE_ON_MEM,
      };
  for(uint i = 0; i < SIZE_A(idc_t); ++i)
    fillCBTypeVal(GetDlgItem(*this, idc_t[i]), 0);

  p = set.getString(ID_RECIPE_BIT_TO_FUNCT);
  uint bitprph = 0;
  uint bitaddr = 0;
  uint bittype = 0;
  while(p) {
    bitprph = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    bitaddr = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    bittype = _ttoi(p);
    break;
    }
  fillCBPerif(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_BIT_FUNCT), bitprph);
  fillCBTypeVal(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_BIT_FUNCT), bittype);
  SET_INT(IDC_EDIT_RCP_ADDR_BIT_FUNCT, bitaddr);


  HWND h = GetDlgItem(*this, IDC_COMBO_RCP_ALIGN_LB);
  LPCTSTR align[] = { _T("Sx"), _T("Centro"), _T("Dx") };
  for(uint i = 0; i < SIZE_A(align); ++i)
    addStringToComboBox(h, align[i]);
  SendMessage(h, CB_SETCURSEL, 0, 0);

}
//------------------------------------------------------------------------------
static void addRowText(HWND hlb, uint ix, LPCTSTR txt)
{
  TCHAR buff[MAX_LEN_LB + 1];
  fillStr(buff, _T(' '), MAX_LEN_LB);
  TCHAR t[20];
  LPTSTR p = buff;
  wsprintf(t, _T("%d"), iLabel[ix].Id);
  copyStr(p, t, _tcslen(t));
  p += LEN_LB_DIM_ID;
  *p++ = _T('\t');

  copyStr(p, iLabel[ix].Label, _tcslen(iLabel[ix].Label));
  p += LEN_LB_INFO;
  *p++ = _T('\t');
  if(txt) {
    uint l = _tcslen(txt);
    l = min(l, LEN_LB_TEXT);
    copyStr(p, txt, l);
    }
  p += LEN_LB_TEXT;
  *p++ = 0;
  SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
}
//------------------------------------------------------------------------------
void cust_recipe::fillLBText(setOfString& set)
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_RCP_TEXT);
  for(uint i = 0; i < SIZE_A(iLabel); ++i) {
    LPCTSTR p = set.getString(iLabel[i].Id);
    addRowText(hlb, i, p);
    }
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
//-----------------------------------------------------------
static int addRowSend(HWND hlb, int val[], bool append, LPCTSTR notes)
{
  TCHAR buff[MAX_LEN_LB_SEND + 1];
  fillStr(buff, _T(' '), MAX_LEN_LB_SEND);
  TCHAR t[20];
  LPTSTR p = buff;
  int tabs[] = { LEN_S_PRPH, LEN_S_ADDR, LEN_S_TYPE, LEN_S_DIM, LEN_S_PRPH, LEN_S_ADDR, LEN_S_TYPE, LEN_S_DIM, LEN_S_TYPE };

  for(uint i = 0; i < SIZE_A(tabs); ++i) {
    wsprintf(t, _T("%d"), val[i]);
    copyStr(p, t, _tcslen(t));
    p += tabs[i];
    *p++ = _T('\t');
    }
  *p = 0;
  if(notes)
    _tcscat_s(buff, notes);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel >= 0 && append) {
    ++sel;
    if(SendMessage(hlb, LB_GETCOUNT, 0, 0) <= sel)
      sel = -1;
    }
  return SendMessage(hlb, LB_INSERTSTRING, sel, (LPARAM)buff);
//  return SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
}
//------------------------------------------------------------------------------
void cust_recipe::fillLBSend(setOfString& set)
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY);

  pVectInt vPrf;
  pVectInt vAddr;
  pVectInt vType;
  pVectInt vDimData;
  pVectInt vTypeMem;
  PVect<LPCTSTR> vNotes;
  do {
    pVectInt vIdNotes;
    LPCTSTR p =  set.getString(ID_INIT_DATA_RECIPE + 11);
    if(p) {
      fillSet(vIdNotes, p);
      uint nElem = vIdNotes.getElem();
      if(nElem) {
        vNotes.setDim(nElem);
        for(uint i = 0; i < nElem; ++i)
          vNotes[i] = str_newdup(set.getString(vIdNotes[i]));
        }
      }
    } while(false);
  int nElem = fillAllSet(set, ID_INIT_DATA_RECIPE + 1, vPrf, vAddr, vType, vTypeMem, vDimData);
  if(!nElem)
    return;

  int ixInit = ID_INIT_ACT_DATA;
  LPCTSTR p =  set.getString(ixInit);

  pVectInt vPrf2;
  pVectInt vAddr2;
  pVectInt vType2;
  pVectInt vDimData2;
  pVectInt vTypeMem2;
  if(!p)
    ixInit = ID_INIT_DATA_RECIPE + 1;
  if(nElem != fillAllSet(set, ixInit, vPrf2, vAddr2, vType2, vTypeMem2, vDimData2))
    return;

  for(int i = vNotes.getElem(); i < nElem; ++i)
    vNotes[i] = 0;
  for(int i = 0; i < nElem; ++i) {
    int val[] = {
        vPrf[i], vAddr[i], vType[i], vDimData[i],
        vPrf2[i], vAddr2[i], vType2[i], vDimData2[i],
        vTypeMem[i] };

    addRowSend(hlb, val, true, vNotes[i]);
    }
  flushPAV(vNotes);
}
//------------------------------------------------------------------------------
static int addRowDef(HWND hlb, int dat[], LPCTSTR pVal, bool append)
{
  TCHAR buff[MAX_LEN_LB_DEF + 1];
  fillStr(buff, _T(' '), MAX_LEN_LB_DEF);
  TCHAR t[20];
  LPTSTR p = buff;
  int tabs[] = { LEN_S_ADDR, LEN_S_TYPE, LEN_D_DIM, LEN_D_VALUE };

  for(uint i = 0; i < SIZE_A(tabs) - 1; ++i) {
    wsprintf(t, _T("%d"), dat[i]);
    copyStr(p, t, _tcslen(t));
    p += tabs[i];
    *p++ = _T('\t');
    }
  copyStr(p, pVal, _tcslen(pVal));
  p += tabs[SIZE_A(tabs) - 1];
  *p = 0;
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel >= 0 && append) {
    ++sel;
    if(SendMessage(hlb, LB_GETCOUNT, 0, 0) <= sel)
      sel = -1;
    }
  return SendMessage(hlb, LB_INSERTSTRING, sel, (LPARAM)buff);
//  return SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
}
//------------------------------------------------------------------------------
void cust_recipe::fillLBDef(setOfString& set)
{
  LPCTSTR p =  set.getString(ID_INIT_DEFAULT_VALUE);
  if(!p)
    return;
  HWND hlb = GetDlgItem(*this, IDC_LIST_RCP_DEF);
  pVectInt vAddr;
  fillSet(vAddr, p);

  int nElem = vAddr.getElem();
  p =  set.getString(ID_INIT_DEFAULT_VALUE + 1);
  if(!p)
    return;
  pVectInt vType;
  fillSet(vType, p);

  if(nElem != vType.getElem())
    return;

  p =  set.getString(ID_INIT_DEFAULT_VALUE + 2);
  if(!p)
    return;
  pVectInt vDimData;
  fillSet(vDimData, p);

  if(nElem != vDimData.getElem())
    return;
  p =  set.getString(ID_INIT_DEFAULT_VALUE + 3);
  if(!p)
    return;
  for(int i = 0; i < nElem; ++i) {
    int val[] = { vAddr[i], vType[i], vDimData[i] };
    LPCTSTR p2 = findNextParam(p, 1);
    TCHAR buff[500];
    int len = p2 ? p2 - p - 1 : _tcslen(p);
    copyStr(buff, p, len);
    buff[len] = 0;
    addRowDef(hlb, val, buff, true);
    p = p2;
    }
}
//------------------------------------------------------------------------------
static int addRowLbLb(HWND hlb, int dat[], bool append)
{
  TCHAR buff[MAX_LEN_LB_LB + 1];
  fillStr(buff, _T(' '), MAX_LEN_LB_LB);
  TCHAR t[20];
  LPTSTR p = buff;
  int tabs[] = { LEN_LL_NUM, LEN_LL_ADDR, LEN_LL_DIM, LEN_LL_ALGN, LEN_LL_DEC };

  for(uint i = 0; i < SIZE_A(tabs); ++i) {
    int align = -1;
    if(2 == i) {
      if(dat[i] < 0) {
        align = 1;
        dat[i] = -dat[i];
        }
      else if(dat[i] > 10000) {
        align = 0;
        dat[i] -= 10000;
        }
      }
    if(4 == i)
      wsprintf(t, _T("%d"), dat[i - 1]);
    else
      wsprintf(t, _T("%d"), dat[i]);
    copyStr(p, t, _tcslen(t));
    p += tabs[i];
    *p++ = _T('\t');
    if(2 == i) {
      ++i;
      switch(align) {
        case -1:
          copyStr(p, _T("Sx"), 2);
          break;
        case 0:
          copyStr(p, _T("C"), 1);
          break;
        case 1:
          copyStr(p, _T("Dx"), 2);
          break;
        }
      p += tabs[i];
      *p++ = _T('\t');
      }
    }
  *p = 0;
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel >= 0 && append) {
    ++sel;
    if(SendMessage(hlb, LB_GETCOUNT, 0, 0) <= sel)
      sel = -1;
    }
  return SendMessage(hlb, LB_INSERTSTRING, sel, (LPARAM)buff);
//  return SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
}
//------------------------------------------------------------------------------
void cust_recipe::fillLBLB(setOfString& set)
{
  LPCTSTR p =  set.getString(ID_INIT_RECIPE_LB + 1);
  if(!p)
    return;
  HWND hlb = GetDlgItem(*this, IDC_LIST_RCP_LB);
  pVectInt vField;
  fillSet(vField, p);

  int nElem = vField.getElem();
  p =  set.getString(ID_INIT_RECIPE_LB + 2);
  if(!p)
    return;
  pVectInt vAddr;
  fillSet(vAddr, p);

  if(nElem != vAddr.getElem())
    return;

  p =  set.getString(ID_INIT_RECIPE_LB + 3);
  if(!p)
    return;
  pVectInt vDimData;
  fillSet(vDimData, p);

  if(nElem != vDimData.getElem())
    return;

  p =  set.getString(ID_INIT_RECIPE_LB + 4);
  if(!p)
    return;
  pVectInt vNDec;
  fillSet(vNDec, p);

  if(nElem != vNDec.getElem())
    return;

  for(int i = 0; i < nElem; ++i) {
    int val[] = { vField[i], vAddr[i], vDimData[i], vNDec[i] };
    addRowLbLb(hlb, val, true);
    }
}
//------------------------------------------------------------------------------
void cust_recipe::checkLabelDef()
{
  HWND hlb = GetDlgItem(*this, IDC_COMBO_RCP_TYPE_DEF);
  int sel = SendMessage(hlb, CB_GETCURSEL, 0, 0);
  if(14 == sel)
    SET_TEXT(IDC_STATIC_NORM_DIM, _T("Dim"));
  else
    SET_TEXT(IDC_STATIC_NORM_DIM, _T("Norm"));
}
//------------------------------------------------------------------------------
void cust_recipe::checkCurrSelSend()
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(-1 == sel)
    SET_TEXT(IDC_STATIC_CURR_SEND, _T(""));
  else
    SET_INT(IDC_STATIC_CURR_SEND, sel);
}
//----------------------------------------------------------------------------
LRESULT cust_recipe::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_RCP_SAVE:
          CmOk();
          break;
        case IDC_BUTTON_RCP_MOD_LIST_TEXT:
          modifyLB();
          break;

        case IDC_BUTTON_RCP_SEND_ADD:
          addSendCopy();
          checkCurrSelSend();
          break;
        case IDC_BUTTON_RCP_SEND_MOD:
          modSendCopy();
          checkCurrSelSend();
          break;
        case IDC_BUTTON_RCP_SEND_REM:
          remFromLb(GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY));
          checkCurrSelSend();
          break;
        case IDC_BUTTON_RCP_SEND_UP:
          moveOnLb(GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY), true);
          checkCurrSelSend();
          break;
        case IDC_BUTTON_RCP_SEND_DN:
          moveOnLb(GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY), false);
          checkCurrSelSend();
          break;

        case IDC_BUTTON_RCP_DEF_ADD:
          addDef();
          break;
        case IDC_BUTTON_RCP_DEF_MOD:
          modDef();
          break;
        case IDC_BUTTON_RCP_DEF_REM:
          remFromLb(GetDlgItem(*this, IDC_LIST_RCP_DEF));
          break;
        case IDC_BUTTON_RCP_DEF_UP:
          moveOnLb(GetDlgItem(*this, IDC_LIST_RCP_DEF), true);
          break;
        case IDC_BUTTON_RCP_DEF_DN:
          moveOnLb(GetDlgItem(*this, IDC_LIST_RCP_DEF), false);
          break;

        case IDC_BUTTON_RCP_LB_ADD:
          addLbLb();
          break;
        case IDC_BUTTON_RCP_LB_MOD:
          modLbLb();
          break;
        case IDC_BUTTON_RCP_LB_REM:
          remFromLb(GetDlgItem(*this, IDC_LIST_RCP_LB));
          break;
        case IDC_BUTTON_RCP_LB_UP:
          moveOnLb(GetDlgItem(*this, IDC_LIST_RCP_LB), true);
          break;
        case IDC_BUTTON_RCP_LB_DN:
          moveOnLb(GetDlgItem(*this, IDC_LIST_RCP_LB), false);
          break;
        case IDC_BUTTON_RCP_LB_COPY:
          copyFromLb();
          break;
        case IDC_BUTTON_RCP_LB_PASTE:
          pasteToLb();
          break;


        }
      switch(HIWORD(wParam)) {
        case CBN_SELCHANGE:
          if(IDC_COMBO_RCP_TYPE_DEF == LOWORD(wParam))
            checkLabelDef();
//          break;
//        case LBN_SELCHANGE:
          if(IDC_LIST_RCP_SEND_COPY == LOWORD(wParam))
            checkCurrSelSend();
          break;
        case LBN_DBLCLK:
          switch(LOWORD(wParam)) {
            case IDC_LIST_RCP_TEXT:
              modifyLB();
              break;
            case IDC_LIST_RCP_SEND_COPY:
              unFillSend();
              break;
            case IDC_LIST_RCP_DEF:
              unFillDef();
              break;
            case IDC_LIST_RCP_LB:
              unFillLbLb();
              break;
            }
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
#if 1
enum eSendCopy { esc_prphSend, esc_addrSend, esc_typeSend, esc_dimSend,
                 esc_prphCopy, esc_addrCopy, esc_typeCopy, esc_dimCopy, esc_typeMem, esc_max
               };
struct sendCopy
{
  int value[esc_max];
  TCHAR notes[4096];
  sendCopy() { ZeroMemory(value, sizeof(value)); ZeroMemory(notes, sizeof(notes)); }
};
#else
struct sendCopy
{
  int prphSend;
  int addrSend;
  int typeSend;
  int dimSend;
  int prphCopy;
  int addrCopy;
  int typeCopy;
  int dimCopy;
  int typeMem;
  TCHAR notes[4096];
};
#endif
//------------------------------------------------------------------------------
//(LEN_S_PRPH + LEN_S_ADDR + LEN_S_TYPE + LEN_S_DIM + 5) * 2 + LEN_S_TYPE)
static void unfillRowSendCopy(LPCTSTR row, sendCopy& sc)
{
  LPCTSTR p = row;
  sc.value[esc_prphSend] = _ttoi(p);
  p += LEN_S_PRPH + 1;
  sc.value[esc_addrSend] = _ttoi(p);
  p += LEN_S_ADDR + 1;
  sc.value[esc_typeSend] = _ttoi(p);
  p += LEN_S_TYPE + 1;
  sc.value[esc_dimSend] = _ttoi(p);
  p += LEN_S_DIM + 1;

  sc.value[esc_prphCopy] = _ttoi(p);
  p += LEN_S_PRPH + 1;
  sc.value[esc_addrCopy] = _ttoi(p);
  p += LEN_S_ADDR + 1;
  sc.value[esc_typeCopy] = _ttoi(p);
  p += LEN_S_TYPE + 1;
  sc.value[esc_dimCopy] = _ttoi(p);
  p += LEN_S_DIM + 1;
  sc.value[esc_typeMem] = _ttoi(p);
  p += LEN_S_TYPE + 1;
  _tcscpy_s(sc.notes, p);
}
//------------------------------------------------------------------------------
static LPTSTR copy_str_tab(LPTSTR t, LPCTSTR s, uint len)
{
  uint i = 0;
  for(; i < len; ++i)
    if(s[i])
      t[i] = s[i];
    else
      break;
  for(; i < len; ++i)
    t[i] = _T(' ');
  t[len] = _T('\t');
  return t + len + 1;
}
//------------------------------------------------------------------------------
static void fillRowSendCopy(LPTSTR row, const sendCopy& sc)
{
  LPTSTR p = row;
  TCHAR t[64];
  uint dim[] = { LEN_S_PRPH, LEN_S_ADDR, LEN_S_TYPE, LEN_S_DIM, LEN_S_PRPH, LEN_S_ADDR,
      LEN_S_TYPE, LEN_S_DIM, LEN_S_TYPE };
  for(uint i = 0; i < SIZE_A(dim); ++i) {
    wsprintf(t, _T("%d"), sc.value[i]);
    p = copy_str_tab(p, t, dim[i]);
    }
  *p = 0;
  if(*sc.notes)
    _tcscat_s(row, MAX_LEN_LB_SEND, sc.notes);
}
//------------------------------------------------------------------------------
struct wrapBuffLb
{
  TCHAR buff[MAX_LEN_LB_SEND + 1];
};
//------------------------------------------------------------------------------
static int prepareForClipboard(wrapBuffLb& buff, const sendCopy& sc)
{
  LPTSTR p = buff.buff;
  for(uint i = 0; i < esc_max; ++i) {
    wsprintf(p, _T("%d\t"), sc.value[i]);
    p += _tcslen(p);
    }
  wsprintf(p, _T("%s\r\n"), sc.notes);
  p += _tcslen(p);
  return p - buff.buff;
}
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define myCF_TEXT CF_UNICODETEXT
#else
  #define myCF_TEXT CF_TEXT
#endif
//------------------------------------------------------------------------------
void cust_recipe::copyFromLb()
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY);
  int count = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  if(!count)
    return;
  if(!OpenClipboard(*this))
    return;
  EmptyClipboard();

  TCHAR buff[MAX_LEN_LB_SEND + 1];
  PVect<wrapBuffLb> buffSet;
  buffSet.setDim(count);
  sendCopy sc;
  int len = 0;
  for(int i = 0; i < count; ++i) {
    SendMessage(hwLb, LB_GETTEXT, i, (LPARAM)buff);
    unfillRowSendCopy(buff, sc);
    len += prepareForClipboard(buffSet[i], sc);
    }
  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 16) * sizeof(TCHAR));
  if(hglbCopy) {
    LPTSTR b = (LPTSTR)GlobalLock(hglbCopy);
    LPTSTR pb = b;
    int d = len + 16;
    for(int i = 0; i < count; ++i) {
      _tcscpy_s(pb, d - (pb - b), buffSet[i].buff);
      pb += _tcslen(pb);
      }
    GlobalUnlock(hglbCopy);
    SetClipboardData(myCF_TEXT, hglbCopy);
    }
  CloseClipboard();
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
static
LPCTSTR get_line(LPTSTR target, LPCTSTR source)
{
  int len = _tcslen(source);
  if(len <= 0)
    return 0;
  int i;
  LPCTSTR p = source;
  for(i = 0; i < len; ++i, ++p, ++target) {
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
    *target = *p;
    }
  if(i == len)
    return 0;
  *target = 0;
  ++p;
  if(++i == len)
    return 0;

  for(; i < len; ++i, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
static LPCTSTR makeForLb(LPCTSTR p, LPTSTR buff)
{
  buff[0] = 0;
  TCHAR tmp[MAX_LEN_LB_SEND + 1];
  p = get_line(tmp, p);
  pvvChar target;
  uint elem = splitParam(target, tmp, _T('\t'));
  uint nValue = min(elem, esc_max);
  sendCopy sc;
  for(uint i = 0; i < nValue; ++i)
    sc.value[i] = _ttoi(&target[i]);
  if(elem > esc_max)
    _tcscpy_s(sc.notes, &target[esc_max]);

  fillRowSendCopy(buff, sc);
  return p;
}
//------------------------------------------------------------------------------
void cust_recipe::fillRowByClipboard(LPCTSTR clip)
{
  static uint Choose = 0;
  if(IDOK != PD_ActionPaste(this, Choose).modal())
    return;
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY);
  int count = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  int sel =  SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  int toRemove = 0;
  switch(Choose) {
    case eap_replaceAll:
      SendMessage(hwLb, LB_RESETCONTENT, 0, 0);
    case eap_append:
      sel = -1;
      break;
    case eap_insert:
      if(sel >= 0)
        break;
    case eap_replase:
      if(sel < 0) {
        MessageBox(*this, _T("Selezionare una riga della lista"), _T("Attenzione!"), MB_OK | MB_ICONINFORMATION);
        return;
        }
      toRemove = count - sel;
      break;
    }
  LPCTSTR p = clip;
  TCHAR buff[MAX_LEN_LB_SEND + 1];
  while(p) {
    p = makeForLb(p, buff);
    if(toRemove) {
      SendMessage(hwLb, LB_DELETESTRING, sel, 0);
      --toRemove;
      }
    SendMessage(hwLb, LB_INSERTSTRING, sel, (LPARAM)buff);
    if(sel >= 0)
      ++sel;
    }
}
//------------------------------------------------------------------------------
void cust_recipe::pasteToLb()
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
//------------------------------------------------------------------------------
void cust_recipe::addSendCopy(bool append)
{
  int prphSend = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_SEND), CB_GETCURSEL, 0, 0);
  if(-1 == prphSend)
    return;
  int typeSend = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_SEND), CB_GETCURSEL, 0, 0);
  if(typeSend < 0)
    typeSend = 0;
  if(!typeSend && prphSend)
    return;

  int typeMem = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_ON_MEM), CB_GETCURSEL, 0, 0);
  if(typeMem <= 0)
    typeMem = typeSend;

  int prphCopy = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_COPY), CB_GETCURSEL, 0, 0);
  if(prphCopy <= 0)
    prphCopy = prphSend;
  int typeCopy = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_COPY), CB_GETCURSEL, 0, 0);
  if(typeCopy <= 0)
    typeCopy = typeSend;

  int addrSend;
  GET_INT(IDC_EDIT_RCP_ADDR_SEND, addrSend);

  int addrCopy;
  GET_INT(IDC_EDIT_RCP_ADDR_COPY, addrCopy);

  int dimSend;
  GET_INT(IDC_EDIT_RCP_DIM_SEND, dimSend);

  int dimCopy;
  GET_INT(IDC_EDIT_RCP_DIM_COPY, dimCopy);

  int val[] = {
    prphSend,
    addrSend,
    typeSend,
    dimSend,
    prphCopy,
    addrCopy,
    typeCopy,
    dimCopy,
    typeMem
    };
  TCHAR notes[4096];
  GET_TEXT(IDC_EDIT_RECIPE_NOTE, notes);

  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY);
  int pos = addRowSend(hwLb, val, append, notes);
  SendMessage(hwLb, LB_SETCURSEL, pos, 0);
  SendMessage(hwLb, LB_SETCARETINDEX, pos, FALSE);
}
//------------------------------------------------------------------------------
void cust_recipe::addDef(bool append)
{
  int type = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_DEF), CB_GETCURSEL, 0, 0);
  if(type <= 0)
    return;
  int addr;
  GET_INT(IDC_EDIT_RCP_ADDR_DEF, addr);
  int dim;
  GET_INT(IDC_EDIT_RCP_DIM_DEF, dim);
  TCHAR buff[LEN_D_VALUE + 2];
  GET_TEXT(IDC_EDIT_RCP_VAL_DEF, buff);
  int val[] = { addr, type, dim };

  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_DEF);
  int pos = addRowDef(hwLb, val, buff, append);
  SendMessage(hwLb, LB_SETCURSEL, pos, 0);
  SendMessage(hwLb, LB_SETCARETINDEX, pos, FALSE);
}
//------------------------------------------------------------------------------
void cust_recipe::addLbLb(bool append)
{
  int align = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_ALIGN_LB), CB_GETCURSEL, 0, 0);
  if(align < 0)
    return;
  int dim;
  GET_INT(IDC_EDIT_RCP_DIM_LB, dim);
  if(!dim)
    return;
  int num;
  GET_INT(IDC_EDIT_RCP_NUM_LB, num);
  int addr;
  GET_INT(IDC_EDIT_RCP_ADDR_LB, addr);
  int dec;
  GET_INT(IDC_EDIT_RCP_DEC_LB, dec);

  if(1 == align)
    dim += 10000;
  else if(2 == align)
    dim = - dim;
  int val[] = { num, addr, dim, dec };
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_LB);
  int pos = addRowLbLb(hwLb, val, append);
  SendMessage(hwLb, LB_SETCURSEL, pos, 0);
  SendMessage(hwLb, LB_SETCARETINDEX, pos, FALSE);
}
//------------------------------------------------------------------------------
static bool saveSet(P_File* pf, uint id, pVectInt& v)
{
  TCHAR buff[50];
  wsprintf(buff, _T("%d"), id);
  if(!writeStringForceUnicode(*pf, buff))
    return false;
  int nElem = v.getElem();
  for(int i = 0; i < nElem; ++i) {
    wsprintf(buff, _T(",%d"), v[i]);
    if(!writeStringForceUnicode(*pf, buff))
      return false;
    }
  return writeStringForceUnicode(*pf, _T("\r\n")) > 0;
}
//------------------------------------------------------------------------------
static bool saveSetNotes(P_File* pf, uint id, PVect<LPCTSTR>& v)
{
  int nElem = v.getElem();
  if(!nElem)
    return true;
  TCHAR buff[50];
  wsprintf(buff, _T("%d"), id);
  if(!writeStringForceUnicode(*pf, buff))
    return false;
  for(int i = 0; i < nElem; ++i) {
    wsprintf(buff, _T(",%d"), ID_INIT_NOTES + i);
    if(!writeStringForceUnicode(*pf, buff))
      return false;
    }
  if(!writeStringForceUnicode(*pf, _T("\r\n")))
    return false;

  for(int i = 0; i < nElem; ++i) {
    if(!v[i] || !*v[i])
      continue;
    wsprintf(buff, _T("%d,"), ID_INIT_NOTES + i);
    if(!writeStringForceUnicode(*pf, buff))
      return false;
    if(!writeStringForceUnicode(*pf, v[i]))
      return false;
    if(!writeStringForceUnicode(*pf, _T("\r\n")))
      return false;
    }
  return true;
}
//------------------------------------------------------------------------------
bool cust_recipe::saveLbSend(P_File* pf)
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY);
  int count = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  if(!count)
    return true;

  pVectInt vPrf;
  pVectInt vAddr;
  pVectInt vType;
  pVectInt vDimData;
  pVectInt vTypeMem;

  pVectInt vPrf2;
  pVectInt vAddr2;
  pVectInt vType2;
  pVectInt vDimData2;

  PVect<LPCTSTR> vNotes;

  TCHAR buff[MAX_LEN_LB_SEND + 1];
  sendCopy sc;
  for(int i = count - 1; i >= 0; --i) {
    SendMessage(hwLb, LB_GETTEXT, i, (LPARAM)buff);
    unfillRowSendCopy(buff, sc);
    vPrf[i] = sc.value[esc_prphSend];
    vAddr[i] = sc.value[esc_addrSend];
    vType[i] = sc.value[esc_typeSend];
    vDimData[i] = sc.value[esc_dimSend];
    vTypeMem[i] = sc.value[esc_typeMem];
    vPrf2[i] = sc.value[esc_prphCopy];
    vAddr2[i] = sc.value[esc_addrCopy];
    vType2[i] = sc.value[esc_typeCopy];
    vDimData2[i] = sc.value[esc_dimCopy];
    vNotes[i] = str_newdup(sc.notes);
    }
  bool success = false;
  while(!success) {
    if(!saveSet(pf, ID_INIT_DATA_RECIPE + 1, vPrf))
      break;
    if(!saveSet(pf, ID_INIT_DATA_RECIPE + 2, vAddr))
      break;
    if(!saveSet(pf, ID_INIT_DATA_RECIPE + 3, vType))
      break;
    if(!saveSet(pf, ID_INIT_DATA_RECIPE + 4, vDimData))
      break;

    if(!saveSet(pf, ID_INIT_DATA_RECIPE + 10, vTypeMem))
      break;

    if(!saveSet(pf, ID_INIT_ACT_DATA + 0, vPrf2))
      break;
    if(!saveSet(pf, ID_INIT_ACT_DATA + 1, vAddr2))
      break;
    if(!saveSet(pf, ID_INIT_ACT_DATA + 2, vType2))
      break;
    if(!saveSet(pf, ID_INIT_ACT_DATA + 3, vDimData2))
      break;

    if(!saveSetNotes(pf, ID_INIT_DATA_RECIPE + 11, vNotes))
      break;

    success = true;
    };
  flushPAV(vNotes);
  return success;
}
//------------------------------------------------------------------------------
static bool saveSetStr(P_File* pf, uint id, PVect<LPCTSTR>& v)
{
  TCHAR buff[50];
  wsprintf(buff, _T("%d"), id);
  if(!writeStringForceUnicode(*pf, buff))
    return false;
  int nElem = v.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(!writeStringForceUnicode(*pf, _T(",")))
      return false;
    if(!writeStringForceUnicode(*pf, v[i]))
      return false;
    }
  return writeStringForceUnicode(*pf, _T("\r\n")) > 0;
}
//------------------------------------------------------------------------------
struct rcpDef
{
  int addr;
  int type;
  int dim;
  TCHAR value[LEN_D_VALUE + 2];
};
//------------------------------------------------------------------------------
//#define MAX_LEN_LB_DEF   (LEN_S_ADDR + LEN_S_TYPE + LEN_D_DIM + LEN_D_VALUE + 5)
static void unfillRowRcpDef(LPCTSTR row, rcpDef& rd)
{
  LPCTSTR p = row;
  rd.addr = _ttoi(p);
  p += LEN_S_ADDR + 1;
  rd.type = _ttoi(p);
  p += LEN_S_TYPE + 1;
  rd.dim = _ttoi(p);
  p += LEN_D_DIM + 1;

  copyStr(rd.value, p, LEN_D_VALUE);
  rd.value[LEN_D_VALUE] = 0;
  trim(lTrim(rd.value));
}
//------------------------------------------------------------------------------
bool cust_recipe::saveLbDef(P_File* pf)
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_DEF);
  int count = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  if(!count)
    return true;

  pVectInt vAddr;
  pVectInt vType;
  pVectInt vDimData;
  PVect<LPCTSTR> vValue;

  TCHAR buff[MAX_LEN_LB_DEF + 1];
  rcpDef rd;
  for(int i = count - 1; i >= 0; --i) {
    SendMessage(hwLb, LB_GETTEXT, i, (LPARAM)buff);
    unfillRowRcpDef(buff, rd);
    vAddr[i] = rd.addr;
    vType[i] = rd.type;
    vDimData[i] = rd.dim;
    vValue[i] = str_newdup(rd.value);
    }
  bool success = false;
  while(!success) {
    if(!saveSet(pf, ID_INIT_DEFAULT_VALUE + 0, vAddr))
      break;
    if(!saveSet(pf, ID_INIT_DEFAULT_VALUE + 1, vType))
      break;
    if(!saveSet(pf, ID_INIT_DEFAULT_VALUE + 2, vDimData))
      break;

    if(!saveSetStr(pf, ID_INIT_DEFAULT_VALUE + 3, vValue))
      break;
    success = true;
    };
  flushPAV(vValue);
  return success;
}
//------------------------------------------------------------------------------
struct lbLb
{
  int num;
  int addr;
  int dim;
  int dec;
};
//------------------------------------------------------------------------------
//#define MAX_LEN_LB_LB   (LEN_LL_NUM + LEN_LL_ADDR + LEN_LL_DIM + LEN_LL_ALGN + LEN_LL_DEC + 6)
static void unfillRowLbLb(LPCTSTR row, lbLb& ll)
{
  LPCTSTR p = row;
  ll.num = _ttoi(p);
  p += LEN_LL_NUM + 1;
  ll.addr = _ttoi(p);
  p += LEN_LL_ADDR + 1;
  ll.dim = _ttoi(p);
  p += LEN_LL_DIM + 1;
  switch(*p) {
/*
    case _T('S'):
    default:
      align = -1;
      break;
*/
    case _T('C'):
      ll.dim += 10000;
      break;
    case _T('D'):
      ll.dim = - ll.dim;
      break;
    }
  p += LEN_LL_ALGN + 1;
  ll.dec = _ttoi(p);
}
//------------------------------------------------------------------------------
bool cust_recipe::saveLbLb(P_File* pf)
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_LB);
  int count = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  if(!count)
    return true;

  pVectInt vNum;
  pVectInt vAddr;
  pVectInt vDim;
  pVectInt vDec;

  TCHAR buff[MAX_LEN_LB_LB + 1];
  lbLb ll;
  for(int i = count - 1; i >= 0; --i) {
    SendMessage(hwLb, LB_GETTEXT, i, (LPARAM)buff);
    unfillRowLbLb(buff, ll);
    vNum[i] = ll.num;
    vAddr[i] = ll.addr;
    vDim[i] = ll.dim;
    vDec[i] = ll.dec;
    }
  bool success = false;
  while(!success) {
    if(!saveSet(pf, ID_INIT_RECIPE_LB + 1, vNum))
      break;
    if(!saveSet(pf, ID_INIT_RECIPE_LB + 2, vAddr))
      break;
    if(!saveSet(pf, ID_INIT_RECIPE_LB + 3, vDim))
      break;
    if(!saveSet(pf, ID_INIT_RECIPE_LB + 4, vDec))
      break;
    success = true;
    }
  return success;
}
//------------------------------------------------------------------------------
bool cust_recipe::saveData()
{
/**/
  P_File* pf = makeFileTmpCustom(true);
  if(!pf)
    return false;

  P_File* pfClear = makeFileTmpCustom(false);
  if(!pfClear) {
    delete pf;
    return false;
    }

  bool success = true;
  TCHAR buff[MAX_LEN_LB + 2];
  HWND hlb = GetDlgItem(*this, IDC_LIST_RCP_TEXT);
  int nElem = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  for(int i = 0; i < nElem; ++i) {
    SendMessage(hlb, LB_GETTEXT, i, (LPARAM)buff);
    uint code = _ttoi(buff);
    LPTSTR p = buff + LEN_LB_DIM_ID + LEN_LB_INFO + 2;
    p[LEN_LB_TEXT] = 0;
    lTrim(trim(p));
    if(*p) {
      TCHAR t[MAX_LEN_LB + 1];
      wsprintf(t, _T("%d,%s\r\n"), code, p);
      if(!writeStringForceUnicode(*pfClear, t)) {
        success = false;
        break;
        }
      }
    }

  do {
    if(!success)
      break;
    success = false;

    GET_TEXT(IDC_EDIT_RCP_EXT, buff);
    if(!*buff)
      _tcscpy_s(buff, _T(".rcp"));
    TCHAR t[50];
    wsprintf(t, _T("%d,%s\r\n"), ID_RECIPE_EXT, buff);
    if(!writeStringForceUnicode(*pf, t))
      break;

    GET_TEXT(IDC_EDIT_ABS_PATH, buff);
    trim(lTrim(buff));
    if(*buff) {
      wsprintf(t, _T("%d,"), ID_RECIPE_ABS_PATH);
      if(!writeStringForceUnicode(*pf, t))
        break;
      TCHAR pb[_MAX_PATH];
      translateFromCRNL(pb, buff);
      if(!writeStringForceUnicode(*pf, pb))
        break;
      if(!writeStringForceUnicode(*pf, _T("\r\n")))
        break;
      }
    GET_TEXT(IDC_EDIT_SUB_PATH, buff);
    trim(lTrim(buff));
    if(*buff) {
      wsprintf(t, _T("%d,"), ID_RECIPE_SUB_PATH);
      if(!writeStringForceUnicode(*pf, t))
        break;
      TCHAR pb[_MAX_PATH];
      translateToCRNL(pb, buff);
      if(!writeStringForceUnicode(*pf, pb))
        break;
      if(!writeStringForceUnicode(*pf, _T("\r\n")))
        break;
      }

    int addrInit;
    int addrName;
    int len;
    int addrCurr;
    GET_INT(IDC_EDIT_RCP_INIT_MEM, addrInit);
    GET_INT(IDC_EDIT_RCP_ADDR_NAME, addrName);
    GET_INT(IDC_EDIT_RCP_LEN_NAME, len);
    GET_INT(IDC_EDIT_RCP_ADDR_ACTIVE, addrCurr);
    int prphMem = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_MEM), CB_GETCURSEL,0 ,0);
    int psw = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PSW_LEVEL), CB_GETCURSEL,0 ,0);
    if(psw > 3)
      psw = 1000;

    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d\r\n"), ID_INIT_DATA_RECIPE,
          addrInit, addrName, len, addrCurr, psw, prphMem);
    if(!writeStringForceUnicode(*pf, buff))
      break;

    if(!saveLbSend(pf))
      break;

    if(!saveLbDef(pf))
      break;

    int addr = 0;
    int bit = 0;
    int prphFlag = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_ACT), CB_GETCURSEL,0 ,0);
    GET_INT(IDC_EDIT_RCP_ADDR_ACT, addr);
    GET_INT(IDC_EDIT_RCP_BIT_ACT, bit);

    wsprintf(buff, _T("%d,%d,%d,%d\r\n"), ID_FLAG_PLC, addr, bit, prphFlag);
    if(!writeStringForceUnicode(*pf, buff))
      break;

    int prphEna = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_ENA), CB_GETCURSEL,0 ,0);
    GET_INT(IDC_EDIT_RCP_ADDR_ENA, addr);
    GET_INT(IDC_EDIT_RCP_BIT_ENA, bit);

    wsprintf(buff, _T("%d,%d,%d,%d\r\n"), ID_OK_SEND, addr, bit, prphEna);
    if(!writeStringForceUnicode(*pf, buff))
      break;

    bool use_def = IS_CHECKED(IDC_CHECK_DEF_EDITOR);
    GET_TEXT(IDC_EDIT_RCP_EDITOR_EXT, t);
    if(use_def || *t) {
      wsprintf(buff, _T("%d,%d,%s\r\n"), ID_DEFAULT_EDITOR, use_def, t);
      if(!writeStringForceUnicode(*pf, buff))
        break;
      }
    if(hasListbox) {
      int style = 0;
      if(IS_CHECKED(IDC_CHECK_RCP_ITALIC))
        style |= fITALIC;
      if(IS_CHECKED(IDC_CHECK_RCP_BOLD))
        style |= fBOLD;
      if(IS_CHECKED(IDC_CHECK_RCP_UNDERL))
        style |= fUNDERLINED;
      int h;
      GET_INT(IDC_EDIT_RCP_HFONT, h);
      TCHAR nf[100];
      GET_TEXT(IDC_EDIT_RCP_NAMEFONT, nf);

      wsprintf(buff, _T("%d,%d,%d,%s\r\n"), ID_INIT_RECIPE_LB, h, style, nf);
      if(!writeStringForceUnicode(*pf, buff))
        break;

      if(!saveLbLb(pf))
        break;
      }
    uint bitprph = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_BIT_FUNCT), CB_GETCURSEL, 0, 0);
    uint bittype = SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_BIT_FUNCT), CB_GETCURSEL, 0, 0);
    uint bitaddr = 0;
    GET_INT(IDC_EDIT_RCP_ADDR_BIT_FUNCT, bitaddr);
    if(bitprph && bittype) {
      wsprintf(buff, _T("%d,%d,%d,%d\r\n"), ID_RECIPE_BIT_TO_FUNCT, bitprph, bitaddr, bittype);
      if(!writeStringForceUnicode(*pf, buff))
        break;
      }
    success = true;
    } while(false);
  delete pf;
  delete pfClear;

  return success;
}
//-------------------------------------------------------------------
void cust_recipe::unFillSend()
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY);
  int sel = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[MAX_LEN_LB_SEND + 1];
  sendCopy sc;
  SendMessage(hwLb, LB_GETTEXT, sel, (LPARAM)buff);
  unfillRowSendCopy(buff, sc);

  SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_SEND), CB_SETCURSEL, sc.value[esc_prphSend], 0);
  SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_SEND), CB_SETCURSEL, sc.value[esc_typeSend], 0);
  SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_ON_MEM), CB_SETCURSEL, sc.value[esc_typeMem], 0);
  SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_PRPH_COPY), CB_SETCURSEL, sc.value[esc_prphCopy], 0);
  SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_COPY), CB_SETCURSEL, sc.value[esc_typeCopy], 0);

  SET_INT(IDC_EDIT_RCP_ADDR_SEND, sc.value[esc_addrSend]);
  SET_INT(IDC_EDIT_RCP_ADDR_COPY, sc.value[esc_addrCopy]);
  SET_INT(IDC_EDIT_RCP_DIM_SEND, sc.value[esc_dimSend]);
  SET_INT(IDC_EDIT_RCP_DIM_COPY, sc.value[esc_dimCopy]);
  SET_TEXT(IDC_EDIT_RECIPE_NOTE, sc.notes);
}
//-------------------------------------------------------------------
void cust_recipe::unFillDef()
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_DEF);
  int sel = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[MAX_LEN_LB_DEF + 1];
  rcpDef rd;
  SendMessage(hwLb, LB_GETTEXT, sel, (LPARAM)buff);
  unfillRowRcpDef(buff, rd);

  SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_TYPE_DEF), CB_SETCURSEL, rd.type, 0);
  SET_INT(IDC_EDIT_RCP_ADDR_DEF, rd.addr);
  SET_INT(IDC_EDIT_RCP_DIM_DEF, rd.dim);
  SET_TEXT(IDC_EDIT_RCP_VAL_DEF, rd.value);
}
//-------------------------------------------------------------------
void cust_recipe::unFillLbLb()
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_RCP_LB);
  int sel = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;

  TCHAR buff[MAX_LEN_LB_LB + 1];
  lbLb ll;
  SendMessage(hwLb, LB_GETTEXT, sel, (LPARAM)buff);
  unfillRowLbLb(buff, ll);

  int align = 0;
  if(ll.dim > 10000) {
    ll.dim -= 10000;
    align = 1;
    }
  else if(ll.dim < 0) {
    ll.dim = -ll.dim;
    align = 2;
    }
  SendMessage(GetDlgItem(*this, IDC_COMBO_RCP_ALIGN_LB), CB_SETCURSEL, align, 0);
  SET_INT(IDC_EDIT_RCP_DIM_LB, ll.dim);
  SET_INT(IDC_EDIT_RCP_NUM_LB, ll.num);
  SET_INT(IDC_EDIT_RCP_ADDR_LB, ll.addr);
  SET_INT(IDC_EDIT_RCP_DEC_LB, ll.dec);
}
//-------------------------------------------------------------------
void cust_recipe::modSendCopy()
{
  int append = remFromLb(GetDlgItem(*this, IDC_LIST_RCP_SEND_COPY));
  if(-1 == append)
    return;
  addSendCopy(toBool(append));
}
//-------------------------------------------------------------------
void cust_recipe::modDef()
{
  int append = remFromLb(GetDlgItem(*this, IDC_LIST_RCP_DEF));
  if(-1 == append)
    return;
  addDef(toBool(append));
}
//-------------------------------------------------------------------
void cust_recipe::modLbLb()
{
  int append = remFromLb(GetDlgItem(*this, IDC_LIST_RCP_LB));
  if(-1 == append)
    return;
  addLbLb(toBool(append));
}
//------------------------------------------------------------------------------
HBRUSH cust_recipe::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    static uint stat[] = {
      IDC_STATIC_RCP_1,
      IDC_STATIC_RCP_2,
      IDC_STATIC_RCP_3,
      IDC_STATIC_RCP_4,
      IDC_STATIC_RCP_5,
      IDC_STATIC_RCP_6,
      IDC_STATIC_RCP_7,
      IDC_STATIC_RCP_8,
      };
    for(uint i = 0; i < SIZE_A(stat); ++i) {
      if(GetDlgItem(*this, stat[i]) == hWndChild) {
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
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
//-------------------------------------------------------------------
void cust_recipe::CmOk()
{

  if(saveData())
    baseClass::CmOk();
}
//------------------------------------------------------------------------------
class PD_ModTextRCP : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PD_ModTextRCP(PWin* parent, LPTSTR target, uint resId = IDD_DIALOG_GR_CHG_TEXT, HINSTANCE hinstance = 0)
        : baseClass(parent, resId, hinstance), Target(target) { new langEdit(this, IDC_EDIT_TEXT); }
    virtual ~PD_ModTextRCP() { destroy(); }
  private:
    LPTSTR Target;
  public:
    virtual bool create();
    void CmOk();

  protected:
//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

};
//------------------------------------------------------------------------------
bool PD_ModTextRCP::create()
{
  if(!baseClass::create())
    return false;
  TCHAR buff[LEN_LB_TEXT + 2];
  LPCTSTR p = Target;
  copyStr(buff, p, LEN_LB_DIM_ID);
  buff[LEN_LB_DIM_ID] = 0;
  trim(buff);
  SET_TEXT(IDC_STATIC_COD, buff);
  p += LEN_LB_DIM_ID + 1;

  copyStr(buff, p, LEN_LB_INFO);
  buff[LEN_LB_INFO] = 0;
  trim(buff);
  SET_TEXT(IDC_STATIC_INFO, buff);
  p += LEN_LB_INFO + 1;

  copyStr(buff, p, LEN_LB_TEXT);
  buff[LEN_LB_TEXT] = 0;
  lTrim(trim(buff));
  SET_TEXT(IDC_EDIT_TEXT, buff);
  return true;
}
//------------------------------------------------------------------------------
void PD_ModTextRCP::CmOk()
{
  TCHAR buff[LEN_LB_TEXT + 2];
  GET_TEXT(IDC_EDIT_TEXT, buff);
  lTrim(trim(buff));
  LPTSTR p = Target + LEN_LB_DIM_ID + LEN_LB_INFO + 2;
  fillStr(p, _T(' '), LEN_LB_TEXT);
  int len = _tcslen(buff);
  copyStr(p, buff, len);
  baseClass::CmOk();
}
//------------------------------------------------------------------------------
void cust_recipe::modifyLB()
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_RCP_TEXT);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[MAX_LEN_LB + 1];
  SendMessage(hlb, LB_GETTEXT, sel, (LPARAM)buff);
  if(IDOK == PD_ModTextRCP(this, buff).modal()) {
    SendMessage(hlb, LB_DELETESTRING, sel, 0);
    SendMessage(hlb, LB_INSERTSTRING, sel, (LPARAM)buff);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
