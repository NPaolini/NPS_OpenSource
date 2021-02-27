//----------- fillDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "fillDlg.h"
#include "pEdit.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "pOwnBtnImageStd.h"
//----------------------------------------------------------------------------
#define MAX_CHECK 13
//----------------------------------------------------------------------------
struct fill_data
{
  DWORD lAddrStart;
  DWORD lenData;
  DWORD ipAddr;
  DWORD db;
  DWORD phAddr;
  int type;
  int action;
  double vMin;
  double vMax;
  DWORD port;
  double vMinP;
  double vMaxP;

  int step;
  DWORD stepLogic;
  DWORD stepDB;
  int OffsetAddr;
  TCHAR descr[MAX_TEXT];
  bool selected[MAX_CHECK];
  int textTransform;

  fill_data() : lAddrStart(0), lenData(0), ipAddr(0), db(0), phAddr(0), type(0), action(0),
                vMin(0), vMax(0), port(0), vMinP(0), vMaxP(0), step(101), stepLogic(0), stepDB(0),
                textTransform(0)
                {
                  ZeroMemory(descr, sizeof(descr)); ZeroMemory(selected, sizeof(selected));
                }
};
static fill_data fillData;
//----------------------------------------------------------------------------
PD_Fill::PD_Fill(baseRow cfg[], uint dimCfg, PWin* parent, uint resId, HINSTANCE hinstance) :
    PDialog(parent, resId, hinstance),  Cfg(cfg), dimCfg(dimCfg)
{
  int idBmp[] = { IDB_OK, IDB_CANC };
  int idBtn[] = { ID_OK, ID_CANCEL };

  HINSTANCE hi = getHInstance();
  POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0, 0, 0xff), 1, GetSysColor(COLOR_BTNTEXT));
  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], new PBitmap(idBmp[i], hi), true);
    btn->setColorRect(cr);
    }
}
//----------------------------------------------------------------------------
PD_Fill::~PD_Fill()
{
  destroy();
}
//----------------------------------------------------------------------------
void addString(HWND hList, LPCTSTR str)
{
  int wid = extent(hList, str);
  int awi = SendMessage(hList, CB_GETDROPPEDWIDTH, 0, 0);
  if(wid > awi)
    ::SendMessage(hList, CB_SETDROPPEDWIDTH, wid, 0);
  ::SendMessage(hList, CB_ADDSTRING, 0, LPARAM(str));
}
//----------------------------------------------------------------------------
static void fillCB(HWND hcb, LPCTSTR rows[], int select)
{
  SendMessage(hcb, CB_RESETCONTENT, 0, 0);
  int i = 0;
  while(rows[i]) {
    addString(hcb, rows[i]);
//    SendMessage(hcb, CB_ADDSTRING, 0, (LPARAM)rows[i]);
    ++i;
    }
  SendMessage(hcb, CB_SETCURSEL, select, 0);
}
//----------------------------------------------------------------------------
static void fillCB_2(HWND hcb, LPCTSTR prefix, DWORD allowed, int select)
{
  SendMessage(hcb, CB_RESETCONTENT, 0, 0);
  TCHAR buff[64];
  uint i = 1;
  int sel = -1;
  uint j = 0;
  while(allowed) {
    if(allowed & 1) {
      wsprintf(buff, _T("%s %d"), prefix, i);
      addString(hcb, buff);
      if(i == select)
        sel = j;
      ++j;
      }
    ++i;
    allowed >>= 1;
    }
  SendMessage(hcb, CB_SETCURSEL, sel, 0);
}
//----------------------------------------------------------------------------
static int unfillCB_2(HWND hcb, LPCTSTR prefix)
{
  TCHAR buff[64];
  GetWindowText(hcb, buff, SIZE_A(buff));
  LPCTSTR p = buff + _tcslen(prefix);
  return _ttoi(p);
}
//----------------------------------------------------------------------------
int getNumAndSelCB(HWND hcb, int& select)
{
  TCHAR buff[50];
  GetWindowText(hcb, buff, SIZE_A(buff));
  int num = _ttoi(buff);
  if(!num)
    select = -1;
  else
    select = SendMessage(hcb, CB_GETCURSEL, 0, 0);
  return num;
}
//----------------------------------------------------------------------------
static LPCTSTR cbType[] = {
  _T("0 - null"),
  _T("1 - bit"),
  _T("2 - byte"),
  _T("3 - word"),
  _T("4 - dword"),
  _T("5 - float"),
  _T("6 - int64"),
  _T("7 - real"),
  _T("11 - char"),
  _T("12 - short"),
  _T("13 - long"),
  _T("14 - string"),
  0
};
//----------------------------------------------------------------------------
static LPCTSTR cbAction[] = {
  _T("0 - null"),
  _T("1 - always read"),
  _T("2 - only first read"),
  _T("3 - first and always"),
  _T("4 - only by req"),
  0
};
//----------------------------------------------------------------------------
static LPCTSTR cbSize[] = {
  _T("1 - 8 bit"),
  _T("2 - 16 bit"),
  _T("4 - 32 bit"),
  0
};
//----------------------------------------------------------------------------
bool PD_Fill::create()
{
  if(!PDialog::create())
    return false;
  load();
  return true;
}
//----------------------------------------------------------------------------
/*
#if 1
#define SET_INT(id, val) SetDlgItemInt(*this, id, val, 0)
#else
#define SET_INT(id, val) SetIntOrSpace(*this, id, val)
static void SetIntOrSpace(HWND hwnd, uint id, uint val)
{
  if(val)
    SetDlgItemInt(hwnd, id, val, 0);
  else
    SetDlgItemText(hwnd, id, _T(""));
}
#endif
#define GET_INT(id, val) val = GetDlgItemInt(*this, id, 0, 0)
//----------------------------------------------------------------------------
#define IS_CHECKED(a) (BST_CHECKED == SendMessage(GetDlgItem(*this, a), BM_GETCHECK, 0, 0))
#define SET_CHECK(a) (SendMessage(GetDlgItem(*this, a), BM_SETCHECK, BST_CHECKED, 0))
*/
//----------------------------------------------------------------------------
#define SET_REAL(id, val) SetReal(*this, id, val)
static void SetReal(HWND hwnd, uint id, double val)
{
  TCHAR buff[50];
  _stprintf_s(buff, _T("%0.5f"), val);
  SetDlgItemText(hwnd, id, buff);
}
//----------------------------------------------------------------------------
#define GET_REAL(id, val) val = GetReal(*this, id)
static double GetReal(HWND hwnd, uint id)
{
  TCHAR buff[50];
  GetDlgItemText(hwnd, id, buff, SIZE_A(buff));
  return _tstof(buff);
}
//----------------------------------------------------------------------------
#define SET_CHECK_IF(a, b, c) setCheckIf(*this, (a), (b), (c))
inline void setCheckIf(HWND hwnd, int idCheck, int idEdit, bool active)
{
  SendMessage(GetDlgItem(hwnd, idCheck), BM_SETCHECK, active ? BST_CHECKED : BST_UNCHECKED, 0);
  EnableWindow(GetDlgItem(hwnd, idEdit), active);
}
//----------------------------------------------------------------------------
void fillCbDataType(HWND hcb, int num)
{
//  fillCB(hcb, cbType, sel);
  int sel = 0;
  for(uint i = 0; i < SIZE_A(cbSize) - 1; ++i)
    if(_ttoi(cbSize[i]) == num) {
      sel = i;
      break;
      }
  fillCB(hcb, cbSize, sel);
}
//----------------------------------------------------------------------------
static
DWORD getAddr(LPCTSTR address)
{
  int p1;
  int p2;
  int p3;
  int p4;
  _stscanf(address, _T("%d.%d.%d.%d"), &p4, &p3, &p2, &p1);
  DWORD addr = p1 | (p2 << 8) | (p3 << 16) | (p4 << 24);
  return addr;
}
//------------------------------------------------------------------
#define IP1(a) (((a) >> 24) & 0xff)
#define IP2(a) (((a) >> 16) & 0xff)
#define IP3(a) (((a) >> 8) & 0xff)
#define IP4(a) (((a) >> 0) & 0xff)
//------------------------------------------------------------------
static
void setAddr(LPTSTR address, DWORD val)
{
  wsprintf(address, _T("%d.%d.%d.%d"), IP1(val), IP2(val), IP3(val), IP4(val));
}
//----------------------------------------------------------------------------
#define VAL_4_NO_ACTION (1 << 8)
//----------------------------------------------------------------------------
void PD_Fill::load()
{
  SET_INT(IDC_EDIT_FILL_START, fillData.lAddrStart);
  SET_INT(IDC_EDIT_FILL_LEN, fillData.lenData);
  SET_INT(IDC_EDIT_FILL_DB, fillData.db);
  SET_INT(IDC_EDIT_FILL_ADDRESS, fillData.phAddr);

  SET_INT(IDC_EDIT_FILL_OFFSET_ADDR, fillData.OffsetAddr);

  SET_REAL(IDC_EDIT_FILL_MIN, fillData.vMin);
  SET_REAL(IDC_EDIT_FILL_MAX, fillData.vMax);
  SET_INT(IDC_EDIT_FILL_PORT, fillData.port);
  SET_REAL(IDC_EDIT_FILL_MIN_P, fillData.vMinP);
  SET_REAL(IDC_EDIT_FILL_MAX_P, fillData.vMaxP);
  TCHAR address[20];
  setAddr(address, fillData.ipAddr);
  SetDlgItemText(*this, IDC_IPADDRESS, address);
  if(50 <= fillData.step && fillData.step <= 100)
    fillData.step = 100;
  if(fillData.step >= 100)
    SET_INT(IDC_EDIT_FILL_STEP, fillData.step - 100);
  else if(fillData.step < 0)
    SET_INT(IDC_EDIT_FILL_STEP, fillData.step);
  else {
    TCHAR buff[20];
    wsprintf(buff, _T("1/%d"), fillData.step);
    SetDlgItemText(*this, IDC_EDIT_FILL_STEP, buff);
    }
  SetDlgItemText(*this, IDC_EDIT_FILL_DESCR, fillData.descr);
  SET_INT(IDC_EDIT_FILL_STEP_LOGIC, fillData.stepLogic);
  SET_INT(IDC_EDIT_FILL_STEP_DB, fillData.stepDB);
  fillCB(GetDlgItem(*this, IDC_COMBOBOX_TYPE), cbType, fillData.type);
  bool isSetup = toBool(fillData.action & 8);
  DWORD act = fillData.action & ~(8 | VAL_4_NO_ACTION);
  fillCB(GetDlgItem(*this, IDC_COMBOBOX_ACTION), cbAction, act);
  if(isSetup)
    SET_CHECK(IDC_CHECK_FILL_SETUP);


  SET_CHECK_IF(IDC_CHECKBOX_DB, IDC_EDIT_FILL_DB, fillData.selected[0]);
  SET_CHECK_IF(IDC_CHECKBOX_DB, IDC_EDIT_FILL_STEP_DB, fillData.selected[0]);

  SET_CHECK_IF(IDC_CHECKBOX_ADDRESS, IDC_EDIT_FILL_ADDRESS, fillData.selected[1]);
  SET_CHECK_IF(IDC_CHECKBOX_ADDRESS, IDC_EDIT_FILL_STEP, fillData.selected[1]);

  SET_CHECK_IF(IDC_CHECKBOX_TYPE, IDC_COMBOBOX_TYPE, fillData.selected[2]);
  SET_CHECK_IF(IDC_CHECKBOX_ACTION, IDC_COMBOBOX_ACTION, fillData.selected[3]);
  SET_CHECK_IF(IDC_CHECKBOX_ACTION, IDC_CHECK_FILL_SETUP, fillData.selected[3]);
  SET_CHECK_IF(IDC_CHECKBOX_IP, IDC_IPADDRESS, fillData.selected[4]);
  SET_CHECK_IF(IDC_CHECKBOX_MIN, IDC_EDIT_FILL_MIN, fillData.selected[5]);
  SET_CHECK_IF(IDC_CHECKBOX_MAX, IDC_EDIT_FILL_MAX, fillData.selected[6]);
  SET_CHECK_IF(IDC_CHECKBOX_PORT, IDC_EDIT_FILL_PORT, fillData.selected[7]);
  SET_CHECK_IF(IDC_CHECKBOX_MIN_P, IDC_EDIT_FILL_MIN_P, fillData.selected[8]);
  SET_CHECK_IF(IDC_CHECKBOX_MAX_P, IDC_EDIT_FILL_MAX_P, fillData.selected[9]);
  SET_CHECK_IF(IDC_CHECKBOX_DESCR, IDC_EDIT_FILL_DESCR, fillData.selected[10]);
  if(fillData.selected[11])
    SET_CHECK(IDC_CHECK_FORCE_FILL);

  SET_CHECK_IF(IDC_CHECKBOX_OFFSET_ADDR, IDC_EDIT_FILL_OFFSET_ADDR, fillData.selected[12]);

  SET_CHECK_SET(IDC_RADIO_OVERWRITE, false);
  SET_CHECK_SET(IDC_RADIO_PREFIX, false);
  SET_CHECK_SET(IDC_RADIO_SUFFIX, false);
  switch(fillData.textTransform) {
    case 0:
    default:
      SET_CHECK(IDC_RADIO_OVERWRITE);
      break;
    case 1:
      SET_CHECK(IDC_RADIO_PREFIX);
      break;
    case 2:
      SET_CHECK(IDC_RADIO_SUFFIX);
      break;
    }
  ENABLE(IDC_RADIO_OVERWRITE, fillData.selected[10]);
  ENABLE(IDC_RADIO_PREFIX, fillData.selected[10]);
  ENABLE(IDC_RADIO_SUFFIX, fillData.selected[10]);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
struct infoDescrNum
{
  int startNum;
  int nDigit;
  int FirstNum;
  infoDescrNum() : startNum(0), nDigit(0), FirstNum(0) {}
  void parse(LPCTSTR source);
};
//----------------------------------------------------------------------------
#define CHAR_DELIM _T('$')
//----------------------------------------------------------------------------
void infoDescrNum::parse(LPCTSTR source)
{
  while(*source) {
    if(CHAR_DELIM == *source) {
      ++source;
      break;
      }
    ++startNum;
    ++source;
    }
  if(!source)
    return;
  FirstNum = _ttoi(source);
  while(*source) {
    if(CHAR_DELIM == *source)
      break;
    if(!_istdigit(*source)) {
      nDigit = 0;
      break;
      }
    ++nDigit;
    ++source;
    }
}
//----------------------------------------------------------------------------
static void transformText(LPTSTR target, LPCTSTR old, LPCTSTR source, uint ix, DWORD whichCopy, const infoDescrNum& ian)
{
  TCHAR tmp[MAX_TEXT + 2] = { 0 };
  LPTSTR t = tmp;
  switch(whichCopy) {
    case 0:
    case 1:
      break;
    case 2:
      _tcscpy_s(tmp, old);
      t += _tcslen(t);
      break;
    }
  if(ian.nDigit) {
    copyStrZ(t, source, ian.startNum);
    t += _tcslen(t);
    TCHAR format[] = _T("%0xd");
    format[2] = ian.nDigit + _T('0');
    wsprintf(t, format, ian.FirstNum + ix);
    t += _tcslen(t);
    source += ian.startNum + ian.nDigit + 2;
    _tcscpy_s(t, SIZE_A(tmp) - (t - tmp), source);
    t += _tcslen(t);
    }
  else {
    _tcscpy_s(t, SIZE_A(tmp) - (t - tmp), source);
    t += _tcslen(t);
    }
  switch(whichCopy) {
    case 0:
    case 2:
      break;
    case 1:
      _tcscpy_s(t, SIZE_A(tmp) - (t - tmp), old);
      break;
    }
  _tcscpy_s(target, MAX_TEXT, tmp);
}
//----------------------------------------------------------------------------
bool PD_Fill::save()
{
  GET_INT(IDC_EDIT_FILL_LEN, fillData.lenData);
  if(!fillData.lenData)
    return false;

  GET_INT(IDC_EDIT_FILL_START, fillData.lAddrStart);
  if(fillData.lAddrStart + fillData.lenData > MAX_ADDRESSES)
    return false;

  GET_INT(IDC_EDIT_FILL_DB, fillData.db);
  GET_INT(IDC_EDIT_FILL_ADDRESS, fillData.phAddr);
  GET_INT(IDC_EDIT_FILL_OFFSET_ADDR, fillData.OffsetAddr);

  GET_REAL(IDC_EDIT_FILL_MIN, fillData.vMin);
  GET_REAL(IDC_EDIT_FILL_MAX, fillData.vMax);
  GET_INT(IDC_EDIT_FILL_PORT, fillData.port);
  GET_REAL(IDC_EDIT_FILL_MIN_P, fillData.vMinP);
  GET_REAL(IDC_EDIT_FILL_MAX_P, fillData.vMaxP);
  TCHAR address[20];
  GetDlgItemText(*this, IDC_IPADDRESS, address, SIZE_A(address));
  fillData.ipAddr = getAddr(address);
  TCHAR buff[20];
  GetDlgItemText(*this, IDC_EDIT_FILL_STEP, buff, SIZE_A(buff));
  bool negStep = false;
  int trueStep = 0;
  if(_T('1' == buff[0] && _T('/') == buff[1]))
    trueStep = fillData.step = _ttoi(buff + 2);
  else {
    fillData.step = _ttoi(buff);
    trueStep = fillData.step;
    if(fillData.step < 0) {
      negStep = true;
      trueStep = -fillData.step + 100;
      }
    else
      trueStep = fillData.step += 100;
    }

  GetDlgItemText(*this, IDC_EDIT_FILL_DESCR, fillData.descr, SIZE_A(fillData.descr));

  GET_INT(IDC_EDIT_FILL_STEP_LOGIC, fillData.stepLogic);
  GET_INT(IDC_EDIT_FILL_STEP_DB, fillData.stepDB);

  int type = getNumAndSelCB(GetDlgItem(*this, IDC_COMBOBOX_TYPE), fillData.type);
  int action = getNumAndSelCB(GetDlgItem(*this, IDC_COMBOBOX_ACTION), fillData.action);
  if(-1 == fillData.action)
    fillData.action = VAL_4_NO_ACTION;
  if(IS_CHECKED(IDC_CHECK_FILL_SETUP)) {
    action |= 8;
    fillData.action |= 8;
    }

  uint idCheck[] = {
        IDC_CHECKBOX_DB,
        IDC_CHECKBOX_ADDRESS,
        IDC_CHECKBOX_TYPE,
        IDC_CHECKBOX_ACTION,
        IDC_CHECKBOX_IP,
        IDC_CHECKBOX_MIN,
        IDC_CHECKBOX_MAX,
        IDC_CHECKBOX_PORT,
        IDC_CHECKBOX_MIN_P,
        IDC_CHECKBOX_MAX_P,
        IDC_CHECKBOX_DESCR,
        IDC_CHECK_FORCE_FILL,
        IDC_CHECKBOX_OFFSET_ADDR
        };

  for(uint i = 0; i < SIZE_A(fillData.selected); ++i)
    fillData.selected[i] = IS_CHECKED(idCheck[i]);

  fillData.textTransform = IS_CHECKED(IDC_RADIO_OVERWRITE) ? 0 : IS_CHECKED(IDC_RADIO_PREFIX) ? 1 : 2;

  int logicStep = fillData.stepLogic;
  if(logicStep <= 0)
    logicStep = 1;
  uint end = fillData.lAddrStart + fillData.lenData * logicStep;
  if(end > dimCfg)
    end = dimCfg;
  int addr = fillData.phAddr;

  bool forceNull = fillData.selected[11];
  if(!forceNull)
    forceNull = action && fillData.selected[3];

  infoDescrNum iAN;
  if(fillData.selected[10])
    iAN.parse(fillData.descr);

  TCHAR tmp[MAX_TEXT];
  if(forceNull) {
    if(trueStep >= 100) {
      int step = trueStep - 100;
      if(negStep)
        step = -step;
      int stepDB = 0;
      for(uint i = fillData.lAddrStart; i < end; i += logicStep) {
        if(fillData.selected[0]) {
          Cfg[i].db = fillData.db + stepDB;
          stepDB += fillData.stepDB;
          }
        if(fillData.selected[1]) {
          Cfg[i].addr = addr;
          addr += step;
          }
        if(fillData.selected[2])
          Cfg[i].type = type;
        if(fillData.selected[3])
          Cfg[i].action = action;
        if(fillData.selected[4])
          Cfg[i].ipAddr = fillData.ipAddr;
        if(fillData.selected[5])
          Cfg[i].vMin = fillData.vMin;
        if(fillData.selected[6])
          Cfg[i].vMax = fillData.vMax;
        if(fillData.selected[7])
          Cfg[i].port = fillData.port;
        if(fillData.selected[8])
          Cfg[i].vMinP = fillData.vMinP;
        if(fillData.selected[9])
          Cfg[i].vMaxP = fillData.vMaxP;
        if(fillData.selected[10]) {
          tmp[0] = 0;
          transformText(tmp, Cfg[i].text, fillData.descr, i - fillData.lAddrStart, fillData.textTransform, iAN);
          _tcscpy_s(Cfg[i].text, tmp);
          }
        if(fillData.selected[12])
          Cfg[i].addr += fillData.OffsetAddr;
        }
      }
    else {
      int step = trueStep;
      for(uint i = fillData.lAddrStart; i < end; i += step) {
        if(fillData.selected[0])
          Cfg[i].db = fillData.db;
        if(fillData.selected[1])
          Cfg[i].addr = addr++;
        if(fillData.selected[2])
          Cfg[i].type = type;
        if(fillData.selected[3])
          Cfg[i].action = action;
        if(fillData.selected[4])
          Cfg[i].ipAddr = fillData.ipAddr;
        if(fillData.selected[5])
          Cfg[i].vMin = fillData.vMin;
        if(fillData.selected[6])
          Cfg[i].vMax = fillData.vMax;
        if(fillData.selected[7])
          Cfg[i].port = fillData.port;
        if(fillData.selected[8])
          Cfg[i].vMinP = fillData.vMinP;
        if(fillData.selected[9])
          Cfg[i].vMaxP = fillData.vMaxP;
        if(fillData.selected[10]) {
          tmp[0] = 0;
          transformText(tmp, Cfg[i].text, fillData.descr, i - fillData.lAddrStart, fillData.textTransform, iAN);
          _tcscpy_s(Cfg[i].text, tmp);
          }
        if(fillData.selected[12])
          Cfg[i].addr += fillData.OffsetAddr;
        }
      }
    }
  else {
    if(trueStep >= 100) {
      int step = trueStep - 100;
      if(negStep)
        step = -step;
      int stepDB = 0;
      for(uint i = fillData.lAddrStart; i < end; i += logicStep) {
        if(!Cfg[i].action)
          continue;
        if(fillData.selected[0]) {
          Cfg[i].db = fillData.db + stepDB;
          stepDB += fillData.stepDB;
          }
        if(fillData.selected[1]) {
          Cfg[i].addr = addr;
          addr += step;
          }
        if(fillData.selected[2])
          Cfg[i].type = type;

        if(fillData.selected[4])
          Cfg[i].ipAddr = fillData.ipAddr;
        if(fillData.selected[5])
          Cfg[i].vMin = fillData.vMin;
        if(fillData.selected[6])
          Cfg[i].vMax = fillData.vMax;
        if(fillData.selected[7])
          Cfg[i].port = fillData.port;
        if(fillData.selected[8])
          Cfg[i].vMinP = fillData.vMinP;
        if(fillData.selected[9])
          Cfg[i].vMaxP = fillData.vMaxP;
        if(fillData.selected[10]) {
          tmp[0] = 0;
          transformText(tmp, Cfg[i].text, fillData.descr, i - fillData.lAddrStart, fillData.textTransform, iAN);
          _tcscpy_s(Cfg[i].text, tmp);
          }
        if(fillData.selected[12])
          Cfg[i].addr += fillData.OffsetAddr;
        }
      }
    else {
      int step = trueStep;
      for(uint i = fillData.lAddrStart; i < end; i += step) {
        if(!Cfg[i].action)
          continue;
        if(fillData.selected[0])
          Cfg[i].db = fillData.db;
        if(fillData.selected[1])
          Cfg[i].addr = addr++;
        if(fillData.selected[2])
          Cfg[i].type = type;
        if(fillData.selected[4])
          Cfg[i].ipAddr = fillData.ipAddr;
        if(fillData.selected[5])
          Cfg[i].vMin = fillData.vMin;
        if(fillData.selected[6])
          Cfg[i].vMax = fillData.vMax;
        if(fillData.selected[7])
          Cfg[i].port = fillData.port;
        if(fillData.selected[8])
          Cfg[i].vMinP = fillData.vMinP;
        if(fillData.selected[9])
          Cfg[i].vMaxP = fillData.vMaxP;
        if(fillData.selected[10]) {
          tmp[0] = 0;
          transformText(tmp, Cfg[i].text, fillData.descr, i - fillData.lAddrStart, fillData.textTransform, iAN);
          _tcscpy_s(Cfg[i].text, tmp);
          }
        if(fillData.selected[12])
          Cfg[i].addr += fillData.OffsetAddr;
        }
      }
    }
  return true;
}
//----------------------------------------------------------------------------
void PD_Fill::enableDisable(HWND check, uint idCtrl)
{
  bool enable = BST_CHECKED == SendMessage(check, BM_GETCHECK, 0, 0);
  EnableWindow(GetDlgItem(*this, idCtrl), enable);
}
//----------------------------------------------------------------------------
static void verifyStep(HWND hwnd, uint idCtrl)
{
  TCHAR buff[20];
  GetDlgItemText(hwnd, idCtrl, buff, SIZE_A(buff));
  DWDATA val = 0;
  if(_T('1' == buff[0] && _T('/') == buff[1]))
    val = _ttoi(buff + 2);
  else
    val = _ttoi(buff);
  if(!val)
    SetDlgItemText(hwnd, idCtrl, _T("0"));
}
//----------------------------------------------------------------------------
LRESULT PD_Fill::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case ID_OK:
          if(!save())
            break;
          EndDialog(*this, IDOK);
          return 0;
        case IDCANCEL:
        case ID_CANCEL:
          EndDialog(*this, IDCANCEL);
          return 0;

        case IDC_CHECKBOX_DB:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_DB);
          enableDisable((HWND) lParam, IDC_EDIT_FILL_STEP_DB);
          break;
        case IDC_CHECKBOX_ADDRESS:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_ADDRESS);
          enableDisable((HWND) lParam, IDC_EDIT_FILL_STEP);
          break;
        case IDC_CHECKBOX_TYPE:
          enableDisable((HWND) lParam, IDC_COMBOBOX_TYPE);
          break;
        case IDC_CHECKBOX_ACTION:
          enableDisable((HWND) lParam, IDC_COMBOBOX_ACTION);
          enableDisable((HWND) lParam, IDC_CHECK_FILL_SETUP);
          break;
        case IDC_CHECKBOX_MIN:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_MIN);
          break;
        case IDC_CHECKBOX_MAX:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_MAX);
          break;
        case IDC_CHECKBOX_OFFSET_ADDR:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_OFFSET_ADDR);
          break;
#if 1
        case IDC_CHECKBOX_MIN_P:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_MIN_P);
          break;
        case IDC_CHECKBOX_MAX_P:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_MAX_P);
          break;
        case IDC_CHECKBOX_PORT:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_PORT);
          break;
        case IDC_CHECKBOX_DESCR:
          enableDisable((HWND) lParam, IDC_RADIO_OVERWRITE);
          enableDisable((HWND) lParam, IDC_RADIO_PREFIX);
          enableDisable((HWND) lParam, IDC_RADIO_SUFFIX);
          enableDisable((HWND) lParam, IDC_EDIT_FILL_DESCR);
          break;

#else
        case IDC_CHECKBOX_NORM:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_NORM);
          break;
#endif
        case IDC_CHECKBOX_IP:
#if 1
          enableDisable((HWND) lParam, IDC_IPADDRESS);
#else
          enableDisable((HWND) lParam, IDC_EDIT_FILL_IP_1);
          enableDisable((HWND) lParam, IDC_EDIT_FILL_IP_2);
          enableDisable((HWND) lParam, IDC_EDIT_FILL_IP_3);
          enableDisable((HWND) lParam, IDC_EDIT_FILL_IP_4);
#endif
          break;
        }
      switch(HIWORD(wParam)) {
        case EN_KILLFOCUS:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_FILL_STEP:
              if(SendMessage((HWND)lParam, EM_GETMODIFY, 0, 0))
                verifyStep(*this, IDC_EDIT_FILL_STEP);
              break;
            }
          break;
        }
      break;

    }
  return PDialog::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PD_Copy::PD_Copy(infoCopy& icopy, PWin* parent, uint resId, HINSTANCE hinstance) :
    PDialog(parent, resId, hinstance),  iCopy(icopy)
{
  int idBmp[] = { IDB_OK, IDB_CANC };
  int idBtn[] = { ID_OK, ID_CANCEL };

  HINSTANCE hi = getHInstance();
  POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0, 0, 0xff), 1, GetSysColor(COLOR_BTNTEXT));
  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], new PBitmap(idBmp[i], hi), true);
    btn->setColorRect(cr);
    }
}
//----------------------------------------------------------------------------
PD_Copy::~PD_Copy()
{
  destroy();
}
//----------------------------------------------------------------------------
bool PD_Copy::create()
{
  if(!PDialog::create())
    return false;
  load();
  return true;
}
//----------------------------------------------------------------------------
LRESULT PD_Copy::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case ID_OK:
          if(!save())
            break;
          EndDialog(*this, IDOK);
          return 0;
        case IDCANCEL:
        case ID_CANCEL:
          EndDialog(*this, IDCANCEL);
          return 0;

        case IDC_CHECK_MOVE:
          enableDisable((HWND) lParam, IDC_EDIT_COPY_N_BLOCK, true);
          enableDisable((HWND) lParam, IDC_EDIT_COPY_OFFS_BLOCK, true);
          break;
        case IDC_CHECKBOX_DB:
          enableDisable((HWND) lParam, IDC_EDIT_COPY_DB);
          break;
        case IDC_CHECKBOX_ADDRESS:
          enableDisable((HWND) lParam, IDC_EDIT_COPY_ADDRESS);
          enableDisable((HWND) lParam, IDC_CHECKBOX_ADDRESS_PROGR);
          break;
        case IDC_CHECKBOX_COPY_FROM:
          enableDisable((HWND) lParam, IDC_COMBO_COPY_FROM);
          break;
        }
      break;

    }
  return PDialog::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_Copy::load()
{
  SET_INT(IDC_EDIT_COPY_START, iCopy.sourceAddr);
  SET_INT(IDC_EDIT_COPY_START2, iCopy.targetAddr);
  SET_INT(IDC_EDIT_COPY_LEN, iCopy.numData);
  SET_INT(IDC_EDIT_COPY_N_BLOCK, iCopy.numBlock);
  SET_INT(IDC_EDIT_COPY_OFFS_BLOCK, iCopy.offsBlock);

  SET_INT(IDC_EDIT_COPY_DB, iCopy.dbOffs);
  SET_INT(IDC_EDIT_COPY_ADDRESS, iCopy.addrOffs);
  if(iCopy.sourcePrph) {
    fillCB_2(GetDlgItem(*this, IDC_COMBO_COPY_FROM), _T("Prph"), iCopy.sourcePrph, iCopy.currPrph);
    SET_CHECK_IF(IDC_CHECKBOX_COPY_FROM, IDC_COMBO_COPY_FROM, toBool(iCopy.flag & infoCopy::ePrph));
    }
  else
    ENABLE(IDC_CHECKBOX_COPY_FROM, false);

  SET_CHECK_IF(IDC_CHECKBOX_DB, IDC_EDIT_COPY_DB, toBool(iCopy.flag & infoCopy::eDb));
  SET_CHECK_IF(IDC_CHECKBOX_ADDRESS, IDC_EDIT_COPY_ADDRESS, toBool(iCopy.flag & infoCopy::eAddr));
  SET_CHECK_IF(IDC_CHECKBOX_ADDRESS, IDC_CHECKBOX_ADDRESS_PROGR, toBool(iCopy.flag & infoCopy::eAddr));
  if(toBool(iCopy.flag & infoCopy::eNoFirstOffset))
    SET_CHECK(IDC_CHECKBOX_OFFSET);
  if(toBool(iCopy.flag & infoCopy::eOffsetProgr))
    SET_CHECK(IDC_CHECKBOX_ADDRESS_PROGR);
  if(iCopy.checkNoAction)
    SET_CHECK(IDC_CHECK_NO_ACTION);
  if(iCopy.move) {
    SET_CHECK(IDC_CHECK_MOVE);
    ENABLE(IDC_EDIT_COPY_N_BLOCK, false);
    ENABLE(IDC_EDIT_COPY_OFFS_BLOCK, false);
    }
}
//----------------------------------------------------------------------------
bool PD_Copy::save()
{
  GET_INT(IDC_EDIT_COPY_LEN, iCopy.numData);
  if(!iCopy.numData)
    return false;

  GET_INT(IDC_EDIT_COPY_START, iCopy.sourceAddr);
  if(iCopy.sourceAddr + iCopy.numData > MAX_ADDRESSES)
    return false;

  GET_INT(IDC_EDIT_COPY_START2, iCopy.targetAddr);
  if(iCopy.targetAddr + iCopy.numData > MAX_ADDRESSES)
    return false;

  GET_INT(IDC_EDIT_COPY_N_BLOCK, iCopy.numBlock);
  GET_INT(IDC_EDIT_COPY_OFFS_BLOCK, iCopy.offsBlock);

  GET_INT(IDC_EDIT_COPY_DB, iCopy.dbOffs);
  GET_INT(IDC_EDIT_COPY_ADDRESS, iCopy.addrOffs);

  iCopy.currPrph = unfillCB_2(GetDlgItem(*this, IDC_COMBO_COPY_FROM), _T("Prph"));

  iCopy.flag = 0;
  if(IS_CHECKED(IDC_CHECKBOX_DB))
    iCopy.flag |= infoCopy::eDb;
  if(IS_CHECKED(IDC_CHECKBOX_ADDRESS))
    iCopy.flag |= infoCopy::eAddr;
  if(IS_CHECKED(IDC_CHECKBOX_COPY_FROM))
    iCopy.flag |= infoCopy::ePrph;
  if(IS_CHECKED(IDC_CHECKBOX_OFFSET))
    iCopy.flag |= infoCopy::eNoFirstOffset;
  if(IS_CHECKED(IDC_CHECKBOX_ADDRESS_PROGR))
    iCopy.flag |= infoCopy::eOffsetProgr;


  iCopy.checkNoAction = IS_CHECKED(IDC_CHECK_NO_ACTION);
  iCopy.move = IS_CHECKED(IDC_CHECK_MOVE);
  if(iCopy.move) {
    iCopy.numBlock = 0;
    iCopy.offsBlock = 0;
    }
  return true;
}
//----------------------------------------------------------------------------
void PD_Copy::enableDisable(HWND check, uint idCtrl, bool reverse)
{
  bool enable = BST_CHECKED == SendMessage(check, BM_GETCHECK, 0, 0);
  if(reverse)
    enable = !enable;
  EnableWindow(GetDlgItem(*this, idCtrl), enable);
}
