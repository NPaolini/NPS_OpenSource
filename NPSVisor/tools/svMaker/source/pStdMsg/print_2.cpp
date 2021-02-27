//--------- print_2.cpp ---------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <stdio.h>
#include "infoText.h"
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
//-------------------------------------------------------------------
#include "commonSimpleRow.h"
#include "svmProperty.h"
#include "plistbox.h"
//----------------------------------------------------------------------------
class manPrintData
{
  public:
    manPrintData();
    ~manPrintData() { }

    void loadFromDialog(HWND hw);
    void fillDialog(HWND hw);

    void load(const setOfString& set, uint id, uint ix);
    bool save(P_File& pf, uint id);

    LPCTSTR getStringForLB(LPTSTR buff);
    LPCTSTR makeStringForLB(LPTSTR buff, HWND hw);
    void setDirty() { dirty = true; }
    void resetDirty() { dirty = false; }
    bool isDirty() const { return dirty; }
  private:
    TCHAR NameInfo[_MAX_PATH];
    TCHAR NameTempl[_MAX_PATH];
    TCHAR IdVars[256];
    int Prph;
    int Addr;
    int Bit;
    bool Preview;
    bool Setup;
    bool dirty;
};
//----------------------------------------------------------------------------
typedef PVect<manPrintData*> pvManPrintData;
//----------------------------------------------------------------------------
class dManageStdPrint2 : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdPrint2(const setOfString& set, PWin* parent, HINSTANCE hInst = 0);
    ~dManageStdPrint2();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    pvManPrintData mPrintData;
    int currpos;
    void loadData();
    void loadAll();
    void addCurrData();
    void remCurrData();
    void saveCurrData();
    bool existCurrData();

    void saveData(int pos);

    void chooseFile(uint idc_edit, bool templ);
    void setOtherFile(LPCTSTR file, LPTSTR path);
};
//-------------------------------------------------------------------
basePage* allocStdPrint2(const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdPrint2(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdPrint2::dManageStdPrint2(const setOfString& set, PWin* parent, HINSTANCE hInst) :
        baseClass(set, parent, IDD_STD_MSG_PRINT_2, hInst), currpos(0)
{  }
//-------------------------------------------------------------------
dManageStdPrint2::~dManageStdPrint2()
{
  destroy();
  flushPV(mPrintData);
}
//-------------------------------------------------------------------
bool dManageStdPrint2::create()
{
  if(!baseClass::create())
    return false;

  loadAll();
  SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_SETCURSEL, 0, 0);
  return true;
}
//-------------------------------------------------------------------
static void enableInfo(HWND hwnd)
{
  TCHAR t[_MAX_PATH];
  GetDlgItemText(hwnd, IDC_EDIT_T_NAME2, t, SIZE_A(t) - 1);
  bool enable = !t[0];
  EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_T_INFO_FILE), enable);
  EnableWindow(GetDlgItem(hwnd, IDC_EDIT_T_NAME), enable);
  EnableWindow(GetDlgItem(hwnd, IDC_EDIT_PV_ID), enable);
}
//-------------------------------------------------------------------
LRESULT dManageStdPrint2::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_T_GLOB_ADD:
          addCurrData();
          break;
        case IDC_BUTTON_T_GLOB_REM:
          remCurrData();
          break;
        case IDC_BUTTON_T_GLOB_MOD:
          saveCurrData();
          break;
        case IDC_LIST_T_ALL:
          switch(HIWORD(wParam)) {
            case LBN_SELCHANGE:
              loadData();
              break;
            }
          break;
        case IDC_BUTTON_T_TEMPL_FILE:
          chooseFile(IDC_EDIT_T_NAME2, true);
          break;
        case IDC_BUTTON_T_INFO_FILE:
          chooseFile(IDC_EDIT_T_NAME, false);
          break;

        case IDC_EDIT_T_EN_PRF:
        case IDC_EDIT_T_EN_ADDR:
        case IDC_EDIT_T_EN_NBIT:
        case IDC_EDIT_T_NAME2:
        case IDC_EDIT_T_NAME:
        case IDC_EDIT_PV_ID:
          if(EN_CHANGE != (HIWORD(wParam)))
            break;
        case IDC_CHECK_PV_PREVIEW:
        case IDC_CHECK_PV_SETUP:
          if(IDC_EDIT_T_NAME2 == LOWORD(wParam))
            enableInfo(*this);
          if(mPrintData.getElem())
            mPrintData[currpos]->setDirty();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
// nell'id c'è il nome del file delle variabili.
// Il file non può essere lo stesso del template di stampa
#define ID_FILE_DATA      995000

// nell'id c'è la lista degli id delle variabili.
#define ID_FILE_DATA_VARS 995001
// numero di id presenti (ciascuno rappresenta una serie)
#define MAX_ID_DATA_VARS 5

#define ID_FILE_PAGE_DIM 995002


#define ID_FILE_DATA_INIT_SET_VARS 999001
//----------------------------------------------------------------------------
struct info4Search
{
  HWND owner;
  LPTSTR file;
  LPCTSTR* fltExt;
  LPCTSTR fltOpen;
  DWORD lastIx;
};
//----------------------------------------------------------------------------
bool openFileGeneric(info4Search& i4s)
{
  infoOpenSave Info(i4s.fltExt, i4s.fltOpen, infoOpenSave::OPEN_F, i4s.lastIx, i4s.file);
  POpenSave open(i4s.owner);

  bool success = false;
  if(open.run(Info)) {
    i4s.lastIx = Info.ixFilter;
    _tcscpy_s(i4s.file, _MAX_PATH, open.getFile());
    success = true;
    }
  return success;
}
//----------------------------------------------------------------------------
LPCTSTR filterExtT[] = { _T(".ptp"),  PAGE_EXT,  _T(".*"),  0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpenT =
  _T("File Template (ptp)\0")
  _T("*.ptp\0")
  _T("File Testo (npt)\0")
  _T("*")PAGE_EXT _T("\0")
  _T("Tutti i File (*.*)\0")
  _T("*.*\0");
//----------------------------------------------------------------------------
LPCTSTR filterExtTD[] = { _T(".ptd"),  PAGE_EXT,  _T(".*"),  0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpenTD =
  _T("File Dati (ptd)\0")
  _T("*.ptd\0")
  _T("File Testo (npt)\0")
  _T("*")PAGE_EXT _T("\0")
  _T("Tutti i File (*.*)\0")
  _T("*.*\0");
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool openFileText(HWND owner, LPTSTR file, bool templ)
{
  static DWORD lastIx;
  info4Search i4s = {
    owner, file, templ ? filterExtT : filterExtTD, templ ? filterOpenT : filterOpenTD, lastIx
    };
  int len = _tcslen(file) - 1;
  TCHAR path[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, path);
  if(_T('\\') == file[len]) {
    SetCurrentDirectory(file);
    file[0] = 0;
    }
  bool success = openFileGeneric(i4s);
  if(success)
    lastIx = i4s.lastIx;
  SetCurrentDirectory(path);
  return success;
}
//-----------------------------------------------------------
void dManageStdPrint2::setOtherFile(LPCTSTR file, LPTSTR path)
{
  setOfString set(file);
  LPCTSTR p = set.getString(ID_FILE_DATA);
  ENABLE(IDC_EDIT_PV_ID, true);
  bool enable = true;
  if(p) {
    LPCTSTR pVars = set.getString(ID_FILE_DATA_VARS);
    if(pVars) {
      SET_TEXT(IDC_EDIT_PV_ID, pVars);
      ENABLE(IDC_EDIT_PV_ID, false);
      }
    appendPath(path, p);
    TCHAR t[_MAX_PATH];
    translateFromCRNL(t, path);
    SET_TEXT(IDC_EDIT_T_NAME, t);
    enable = false;
    }
  ENABLE(IDC_BUTTON_T_INFO_FILE, enable);
  ENABLE(IDC_EDIT_T_NAME, enable);
}
//-----------------------------------------------------------
void dManageStdPrint2::chooseFile(uint idc_edit, bool templ)
{
  dataProject& dp = getDataProject();
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, dp.newPath);
  appendPath(path, 0);
  LPTSTR p = path + _tcslen(path);

  TCHAR t[_MAX_PATH];
  GET_TEXT(idc_edit, t);
  translateToCRNL(t, t);
  if(*t)
    appendPath(path, t);
  if(openFileText(*this, path, templ)) {
    translateFromCRNL(t, p);
    SET_TEXT(idc_edit, t);
    if(templ) {
      _tcscpy_s(t, p);
      int len = _tcslen(t);
      for(int i = len - 1; i >= 0; --i) {
        if(_T('\\') == t[i]) {
          t[i] = 0;
          break;
          }
        }
      setOtherFile(path, t);
      }
    if(mPrintData.getElem())
      mPrintData[currpos]->setDirty();
    }
}
//------------------------------------------------------------------------------
static
void remFromLb(HWND hwLb)
{
  int pos = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  int count = SendMessage(hwLb, LB_DELETESTRING, pos, 0);
  if(pos >= count)
    pos = count - 1;
  SendMessage(hwLb, LB_SETCURSEL, pos, 0);
}
//------------------------------------------------------------------------------
void dManageStdPrint2::loadAll()
{
  LPCTSTR p = Set.getString(ADDR_PRINT_SCREEN);
  if(p) {
    uint Prph = 0;
    uint Addr = 0;
    uint Type = 0;
    uint Bit = 0;
    uint Left = -1;
    uint Top = -1;
    uint Right = -1;
    uint Bottom = -1;
    uint Preview = 0;
    uint Setup = 0;
    uint fullScreen = 0;
//  prf,DWORD,type,Bit,preview,setup in memoria per stampa Video
//559,1,49,4,0,0,0
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"), &Prph, &Addr, &Type, &Bit, &Preview, &Setup, &fullScreen);

    SET_INT(IDC_EDIT_PS_PRPH, Prph);
    SET_INT(IDC_EDIT_PS_ADDR, Addr);
    SET_INT(IDC_EDIT_PS_TYPE, Type);
    SET_INT(IDC_EDIT_PS_BIT, Bit);

    SET_CHECK_SET(IDC_CHECK_PS_PREVIEW, Preview);
    SET_CHECK_SET(IDC_CHECK_PS_SETUP, Setup);
    SET_CHECK_SET(IDC_CHECK_PS_FULL, fullScreen);

    p = Set.getString(ID_MARGIN_PRINTSCREEN);
    if(p)
      _stscanf_s(p, _T("%d,%d,%d,%d"), &Left, &Top, &Right, &Bottom);

    SET_INT(IDC_EDIT_PS_LEFT, Left);
    SET_INT(IDC_EDIT_PS_TOP2, Top);
    SET_INT(IDC_EDIT_PS_RIGHT2, Right);
    SET_INT(IDC_EDIT_PS_BOTTOM, Bottom);
    }

  p = Set.getString(ADDR_SAVE_SCREEN);
  if(p) {
    uint Prph = 0;
    uint Addr = 0;
    uint Type = 0;
    uint Bit = 0;

    uint Prph2 = 0;
    uint Addr2 = 0;
    uint Len = 0;

//  558,1,49,4,0,1,50,80,expimage\\screen.jpg
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"), &Prph, &Addr, &Type, &Bit, &Prph2, &Addr2, &Len);
    p = findNextParamTrim(p, 7);
    if(p) {
      TCHAR path[_MAX_PATH];
      translateFromCRNL(path, p);
      SET_TEXT(IDC_EDIT_PS_DEF_FILE_N, path);
      }
    SET_INT(IDC_EDIT_PS_PRPH_S, Prph);
    SET_INT(IDC_EDIT_PS_ADDR_S, Addr);
    SET_INT(IDC_EDIT_PS_TYPE_S, Type);
    SET_INT(IDC_EDIT_PS_BIT_S, Bit);

    SET_INT(IDC_EDIT_PS_PRPH_N, Prph2);
    SET_INT(IDC_EDIT_PS_ADDR_N, Addr2);
    SET_INT(IDC_EDIT_PS_LEN_N, Len);
    }
  p = Set.getString(SAVE_SCREEN_ONLY_ONE_FILE);
  if(p && 1 == _ttoi(p))
    SET_CHECK(IDC_CHECK_ONLY_ONE_FILE);

  uint fullScreen = 0;
  p = findNextParamTrim(p);
  if(p)
    fullScreen = _ttoi(p);

  SET_CHECK_SET(IDC_CHECK_SS_FULL, fullScreen);

  flushPV(mPrintData);
  p = Set.getString(ID_PRINT_DATA_VAR);
  if(!p)
    return;

  int nElem = 0;
  HWND hwLb = GetDlgItem(*this, IDC_LIST_T_ALL);
  do {
    int id = _ttoi(p);
    manPrintData* mt = new manPrintData;
    mt->load(Set, id, nElem);
    TCHAR buff[500];
    LPCTSTR plb = mt->getStringForLB(buff);
    if(!plb)
      delete mt;
    else {
      SendMessage(hwLb, LB_ADDSTRING, 0, (LPARAM)buff);
      mPrintData[nElem] = mt;
      ++nElem;
      }
    p = findNextParam(p, 1);
    } while(p);

  if(nElem)
    mPrintData[0]->fillDialog(*this);

}
//------------------------------------------------------------------------------
void dManageStdPrint2::remCurrData()
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_T_ALL);
  int pos = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  int nElem = mPrintData.getElem();
  if(pos >= nElem)
    return;
  manPrintData* mt = mPrintData.remove(pos);
  delete mt;
  pos = SendMessage(hwLb, LB_DELETESTRING, pos, 0) - 1;
  if(pos < 0 || pos >= nElem - 1)
    pos = 0;
  if(nElem > 1)
    mPrintData[pos]->fillDialog(*this);
  currpos = pos;
}
//------------------------------------------------------------------------------
bool dManageStdPrint2::existCurrData()
{
  int nElem = mPrintData.getElem();
  if(!nElem)
    return false;
  TCHAR buff[500];
  LPCTSTR plb = mPrintData[0]->makeStringForLB(buff, *this);
  if(!plb)
    return true;
  TCHAR test[500];
  for(int i = 0; i < nElem; ++i) {
    mPrintData[i]->getStringForLB(test);
    if(!_tcsicmp(buff, test))
      return true;
    }
  return false;
}
//------------------------------------------------------------------------------
void dManageStdPrint2::addCurrData()
{
  if(existCurrData())
    return;
  int nElem = mPrintData.getElem();
  TCHAR buff[500];
/*
  if(nElem) {
    LPCTSTR plb = mPrintData[0]->makeStringForLB(buff, *this);
    if(!plb)
      return;
    TCHAR test[500];
    for(int i = 0; i < nElem; ++i) {
      mPrintData[i]->getStringForLB(test);
      if(!_tcsicmp(buff, test))
        return;
      }
    }
*/
  manPrintData* mt = new manPrintData;
  mt->loadFromDialog(*this);
  mt->getStringForLB(buff);
  int sel = SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_ADDSTRING, 0, (LPARAM)buff);
  currpos = sel;
  SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_SETCURSEL, sel, 0);
  mPrintData[nElem] = mt;
}
//------------------------------------------------------------------------------
void dManageStdPrint2::saveData(int pos)
{
  if(pos < 0)
    return;
  int nElem = mPrintData.getElem();
  if(pos >= nElem)
    return;
  HWND hwLb = GetDlgItem(*this, IDC_LIST_T_ALL);
  TCHAR buff[100];
  SendMessage(hwLb, LB_GETTEXT, pos, (LPARAM)buff);
  mPrintData[pos]->loadFromDialog(*this);

  TCHAR curr[100];
  if(mPrintData[pos]->getStringForLB(curr)) {
    if(_tcsicmp(curr, buff)) {
      SendMessage(hwLb, LB_DELETESTRING, pos, 0);
      SendMessage(hwLb, LB_INSERTSTRING, pos, (LPARAM)curr);
      SendMessage(hwLb, LB_SETCURSEL, pos, 0);
      }
    }
  mPrintData[pos]->resetDirty();
}
//------------------------------------------------------------------------------
void dManageStdPrint2::saveCurrData()
{
  int pos = SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_GETCURSEL, 0, 0);
  saveData(pos);
}
//------------------------------------------------------------------------------
void dManageStdPrint2::loadData()
{
  int pos = SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  int nElem = mPrintData.getElem();
  if(pos >= nElem)
    return;
  if(mPrintData[currpos]->isDirty()) {
    switch(MessageBox(*this, _T("I dati sono stati modificati, vuoi salvarli?"), _T("Attenzione"), MB_YESNOCANCEL | MB_ICONSTOP)) {
      case IDYES:
        saveData(currpos);
        SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_SETCURSEL, pos, 0);
        break;
      case IDCANCEL:
        SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_SETCURSEL, currpos, 0);
        return;
      case IDNO:
        mPrintData[currpos]->resetDirty();
        break;
      }
    }
  mPrintData[pos]->fillDialog(*this);
  currpos = pos;
}
//------------------------------------------------------------------------------
HBRUSH dManageStdPrint2::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    uint groupBox[] = {
      IDC_GROUPBOX_T,
      IDC_GROUPBOX_PS,
      IDC_GROUPBOX_PS2,
      IDC_GROUPBOX_PS3,
      };
    for(uint i = 0; i < SIZE_A(groupBox); ++i)
      if(GetDlgItem(*this, groupBox[i]) == hWndChild) {
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
//-------------------------------------------------------------------
#define OFFSET_PRINT_DATA_CODE_BASE    10
//-------------------------------------------------------------------
bool dManageStdPrint2::save(P_File& pfCrypt, P_File& pfClear)
{
  TCHAR buff[500];

  uint Prph = 0;
  uint Addr = 0;
  uint Type = 0;
  uint Bit = 0;
  uint Left = -1;
  uint Top = -1;
  uint Right = -1;
  uint Bottom = -1;
  uint Preview = 0;
  uint Setup = 0;
  uint fullScreen = 0;

  GET_INT(IDC_EDIT_PS_PRPH, Prph);
  GET_INT(IDC_EDIT_PS_ADDR, Addr);
  GET_INT(IDC_EDIT_PS_TYPE, Type);
  GET_INT(IDC_EDIT_PS_BIT, Bit);

  Preview = IS_CHECKED(IDC_CHECK_PS_PREVIEW);
  Setup = IS_CHECKED(IDC_CHECK_PS_SETUP);
  fullScreen = IS_CHECKED(IDC_CHECK_PS_FULL);

//  prf,DWORD,type,Bit,preview,setup in memoria per stampa Video
//559,1,49,4,0,0,0
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d\r\n"), ADDR_PRINT_SCREEN,
          Prph, Addr, Type, Bit, Preview, Setup, fullScreen);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  GET_INT(IDC_EDIT_PS_LEFT, Left);
  GET_INT(IDC_EDIT_PS_TOP2, Top);
  GET_INT(IDC_EDIT_PS_RIGHT2, Right);
  GET_INT(IDC_EDIT_PS_BOTTOM, Bottom);
  wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"), ID_MARGIN_PRINTSCREEN, Left, Top, Right, Bottom);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  uint Prph2 = 0;
  uint Addr2 = 0;
  uint Len = 0;

//  558,1,49,4,0,1,50,80,expimage\\screen.jpg
  TCHAR path[_MAX_PATH];
  GET_TEXT(IDC_EDIT_PS_DEF_FILE_N, path);

  GET_INT(IDC_EDIT_PS_PRPH_S, Prph);
  GET_INT(IDC_EDIT_PS_ADDR_S, Addr);
  GET_INT(IDC_EDIT_PS_TYPE_S, Type);
  GET_INT(IDC_EDIT_PS_BIT_S, Bit);

  GET_INT(IDC_EDIT_PS_PRPH_N, Prph2);
  GET_INT(IDC_EDIT_PS_ADDR_N, Addr2);
  GET_INT(IDC_EDIT_PS_LEN_N, Len);

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%s\r\n"), ADDR_SAVE_SCREEN,
          Prph, Addr, Type, Bit, Prph2, Addr2, Len, path);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;
  uint onlyOne = IS_CHECKED(IDC_CHECK_ONLY_ONE_FILE);
  fullScreen = IS_CHECKED(IDC_CHECK_SS_FULL);
  if(fullScreen || onlyOne) {
    wsprintf(buff, _T("%d,%d,%d\r\n"), SAVE_SCREEN_ONLY_ONE_FILE, onlyOne, fullScreen);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }
  int nElem = mPrintData.getElem();
  if(!nElem)
    return true;

  wsprintf(buff, _T("%d"), ID_PRINT_DATA_VAR);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  int idInit = ID_PRINT_DATA_VAR + 1;
  for(int i = 0; i < nElem; ++i, idInit += OFFSET_PRINT_DATA_CODE_BASE) {
    if(!writeStringChkUnicode(pfCrypt, _T(",")))
      return false;
    wsprintf(buff, _T("%d"), idInit);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }
  if(!writeStringChkUnicode(pfCrypt, _T("\r\n")))
    return false;

  for(int i = 0; i < nElem; ++i) {
    if(!mPrintData[i]->save(pfCrypt, i))
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
manPrintData::manPrintData() : Prph(0), Addr(0), Bit(0), Preview(false), Setup(false), dirty(false)
{
  NameInfo[0] = 0;
  NameTempl[0] = 0;
  IdVars[0] = 0;
}
//----------------------------------------------------------------------------
#define FormatInfoLB _T("P.%d_I.%d_B.%d")
//----------------------------------------------------------------------------
LPCTSTR manPrintData::getStringForLB(LPTSTR buff)
{
  if(!Prph)
    return 0;
  wsprintf(buff, FormatInfoLB, Prph, Addr, Bit);
  return buff;
}
//----------------------------------------------------------------------------
LPCTSTR manPrintData::makeStringForLB(LPTSTR buff, HWND hw)
{
  TCHAR t1[20];
  TCHAR t2[20];
  TCHAR t3[20];
  if(!GetDlgItemText(hw, IDC_EDIT_T_EN_PRF, t1, SIZE_A(t1)))
    return 0;
  if(!GetDlgItemText(hw, IDC_EDIT_T_EN_ADDR, t2, SIZE_A(t2)))
    return 0;
  if(!GetDlgItemText(hw, IDC_EDIT_T_EN_NBIT, t3, SIZE_A(t3)))
    return 0;
  wsprintf(buff, FormatInfoLB, _ttoi(t1), _ttoi(t2), _ttoi(t3));
  return buff;
}
//----------------------------------------------------------------------------
void manPrintData::fillDialog(HWND hwnd)
{
  SetDlgItemText(hwnd, IDC_EDIT_T_NAME, NameInfo);
  SetDlgItemText(hwnd, IDC_EDIT_T_NAME2, NameTempl);
  SetDlgItemText(hwnd, IDC_EDIT_PV_ID, IdVars);
  enableInfo(hwnd);

  SetDlgItemInt(hwnd, IDC_EDIT_T_EN_PRF, Prph, false);
  SetDlgItemInt(hwnd, IDC_EDIT_T_EN_ADDR, Addr, false);
  SetDlgItemInt(hwnd, IDC_EDIT_T_EN_NBIT, Bit, false);

  SendMessage(GetDlgItem(hwnd, IDC_CHECK_PV_PREVIEW), BM_SETCHECK,
        Preview ? BST_CHECKED : BST_UNCHECKED, 0);

  SendMessage(GetDlgItem(hwnd, IDC_CHECK_PV_SETUP), BM_SETCHECK,
        Setup ? BST_CHECKED : BST_UNCHECKED, 0);
  resetDirty();
}
//----------------------------------------------------------------------------
#define N_SZ(a) a, SIZE_A(a)
//----------------------------------------------------------------------------
void manPrintData::loadFromDialog(HWND hwnd)
{
  GetDlgItemText(hwnd, IDC_EDIT_T_NAME, N_SZ(NameInfo));
  GetDlgItemText(hwnd, IDC_EDIT_T_NAME2, N_SZ(NameTempl));
  GetDlgItemText(hwnd, IDC_EDIT_PV_ID, N_SZ(IdVars));

  Prph = GetDlgItemInt(hwnd, IDC_EDIT_T_EN_PRF, 0, false);
  Addr = GetDlgItemInt(hwnd, IDC_EDIT_T_EN_ADDR, 0, false);
  Bit = GetDlgItemInt(hwnd, IDC_EDIT_T_EN_NBIT, 0, false);

  Preview = SendMessage(GetDlgItem(hwnd, IDC_CHECK_PV_PREVIEW), BM_GETCHECK, 0, 0) ==
        BST_CHECKED;
  Setup = SendMessage(GetDlgItem(hwnd, IDC_CHECK_PV_SETUP), BM_GETCHECK, 0, 0) ==
        BST_CHECKED;
}
//----------------------------------------------------------------------------
void manPrintData::load(const setOfString& set, uint id, uint ix)
{
  LPCTSTR p = set.getString(id);
  if(!p)
    return;

//  prf,addr,bit per avvio stampa
//601,1,48,0
  _stscanf_s(p, _T("%d,%d,%d"), &Prph, &Addr, &Bit);

//  file contenente le informazioni su prf,addr,type,ndec,norm
//602,system\\stampa\\info-StampaRic.npt
  p = set.getString(id + 1);
  if(!p)
    return;
  translateFromCRNL(NameInfo, p);
//  _tcscpy(NameInfo, p);

//  file contenente le informazioni sugli oggetti e sulle posizioni di stampa
//603,system\\stampa\\template-StampaRic.npt
  p = set.getString(id + 2);
  if(!p)
    return;
  translateFromCRNL(NameTempl, p);
//  _tcscpy(NameTempl, p);

//  id su cui sono contenute prf,addr,type,ndec,norm nel file di info
//  seguono due variabili per preview e setup
//604,2000,2001,2002,2203,2204,1,0

  p = set.getString(id + 3);
  if(!p)
    return;

  LPCTSTR p2 = p;
  int nElem = 0;
  while(p2) {
    p2 = findNextParam(p2, 1);
    ++nElem;
    }
  nElem -= 2;
  if(nElem > 0) {
    p2 = findNextParam(p, nElem);
    int len = p2 - p - 1;
    copyStr(IdVars, p, len);
    IdVars[len] = 0;
    Preview = toBool(_ttoi(p2));
    p2 = findNextParam(p2, 1);
    Setup = toBool(_ttoi(p2));
    }
}
//----------------------------------------------------------------------------
bool manPrintData::save(P_File& pf, uint idBase)
{
  uint id = idBase * OFFSET_PRINT_DATA_CODE_BASE + 1 + ID_PRINT_DATA_VAR;

  TCHAR buff[500];

//  prf,addr,bit per avvio stampa
//601,1,48,0
  wsprintf(buff, _T("%d,%d,%d,%d\r\n"), id, Prph, Addr, Bit);
  if(!writeStringChkUnicode(pf, buff))
    return false;

  wsprintf(buff, _T("%d,%s\r\n"), id + 1, NameInfo);
  if(!writeStringChkUnicode(pf, buff))
    return false;
  wsprintf(buff, _T("%d,%s\r\n"), id + 2, NameTempl);
  if(!writeStringChkUnicode(pf, buff))
    return false;
  wsprintf(buff, _T("%d,%s,%d,%d\r\n"), id + 3, IdVars, Preview, Setup);
  if(!writeStringChkUnicode(pf, buff))
    return false;
  resetDirty();
  return true;
}
