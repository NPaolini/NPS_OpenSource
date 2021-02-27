//--------- various_2.cpp -------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <stdio.h>
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
#include "plistbox.h"
#include "language_util.h"
//-------------------------------------------------------------------
#define MAX_LEN_PRPH 4
#define MAX_LEN_ADDR 6
#define MAX_LEN_TYPE 4
#define MAX_LEN_NBIT 4
#define MAX_LEN_OFFS 4
#define MAX_LEN_LEVEL 4
#define MAX_LEN_DESCR (1024 * 16)

#define MAX_BUFF_LB  (MAX_LEN_PRPH + MAX_LEN_ADDR + MAX_LEN_TYPE + MAX_LEN_NBIT + MAX_LEN_OFFS + MAX_LEN_LEVEL + MAX_LEN_DESCR + 10)
//-------------------------------------------------------------------
struct infoRow
{
  uint prph;
  uint addr;
  uint type;
  uint nbit;
  uint offs;
  uint level;
  TCHAR descr[MAX_LEN_DESCR + 1];
  infoRow() :  prph(0), addr(0), type(0), nbit(0), offs(0), level(0)
  {
    ZeroMemory(descr, sizeof(descr));
  }
};
//-------------------------------------------------------------------
struct wrapInfoRow
{
  wrapInfoRow(infoRow* ir) : Ir(ir) {}
  wrapInfoRow() : Ir(new infoRow) {}
  ~wrapInfoRow() { delete Ir; }
  infoRow& operator *() { return *Ir; }
  infoRow* Ir;
};
//-------------------------------------------------------------------
class dManageStdVars3 : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdVars3(const setOfString& set, PWin* parent, HINSTANCE hInst = 0) :
        baseClass(set, parent, IDD_STD_MSG_FLAG3, hInst)
        {}
    ~dManageStdVars3();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    void fillCombo();
    void fillDataCtrl();
    void fillLB();
    void fillDataPsw();
    void fillDataPing();
    void addRow(LPCTSTR p);

    void addToLB();
    void remFromLB();
    void moveRow(bool up);
};
//-------------------------------------------------------------------
basePage* allocStdVars3( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdVars3(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdVars3::~dManageStdVars3()
{
  destroy();
}
//-------------------------------------------------------------------
bool dManageStdVars3::create()
{
  PListBox* lb = new PListBox(this, IDC_LISTBOX_VARS_TRAY);
  int t[] = {  MAX_LEN_PRPH, MAX_LEN_ADDR, MAX_LEN_TYPE, MAX_LEN_NBIT, MAX_LEN_OFFS, MAX_LEN_LEVEL, MAX_LEN_DESCR };
  lb->SetTabStop(SIZE_A(t), t, 0);
  lb->SetColorSel(RGB(0, 0, 64), RGB(192, 240, 255));

  for(uint i = 0; i < SIZE_A(t) - 1; ++i)
    lb->setAlign(i, PListBox::aCenter);

  new langEdit(this, IDC_EDIT_TRAY_TIPS_HEAD);
  new langEdit(this, IDC_EDIT_TRAY_TIPS);
  new langEdit(this, IDC_EDIT_TRAY_TIPS_OK);

  if(!baseClass::create())
    return false;

  fillDataCtrl();

  return true;
}
//-------------------------------------------------------------------
LRESULT dManageStdVars3::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_TRAY:
          addToLB();
          break;
        case IDC_BUTTON_REM_TRAY:
          remFromLB();
          break;
        case IDC_BUTTON_UP_TRAY:
          moveRow(true);
          break;
        case IDC_BUTTON_DN_TRAY:
          moveRow(false);
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
LPTSTR fillAndSet(LPTSTR buff, uint v, uint len)
{
  fillStr(buff, _T(' '), len);
  TCHAR t[64];
  wsprintf(t, _T("%d"), v);
  uint l = min(len, _tcslen(t));
  copyStr(buff, t, l);
  buff[len] = _T('\t');
  return buff + len + 1;
}
//-------------------------------------------------------------------
static void formatRow(LPTSTR buff, const infoRow& iR)
{
  LPTSTR p = buff;
  p = fillAndSet(p, iR.prph, MAX_LEN_PRPH);
  p = fillAndSet(p, iR.addr, MAX_LEN_ADDR);
  p = fillAndSet(p, iR.type, MAX_LEN_TYPE);
  p = fillAndSet(p, iR.nbit, MAX_LEN_NBIT);
  p = fillAndSet(p, iR.offs, MAX_LEN_OFFS);
  p = fillAndSet(p, iR.level, MAX_LEN_LEVEL);
  copyStrZ(p, iR.descr, MAX_LEN_DESCR);
}
//-------------------------------------------------------------------
void addRowToLB(HWND hlb, const infoRow& iR)
{
  TCHAR buff[MAX_BUFF_LB];
  formatRow(buff, iR);

  int pos = SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(hlb, LB_SETCURSEL, pos, 0);
}
//-------------------------------------------------------------------
void dManageStdVars3::addToLB()
{
  wrapInfoRow wrap;
  infoRow& iR = *wrap;
  GET_INT(IDC_EDIT_TRAY_ADDR, iR.addr);
  GET_INT(IDC_EDIT_TRAY_NBIT, iR.nbit);
  GET_INT(IDC_EDIT_TRAY_OFFS, iR.offs);
  iR.prph = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TRAY_PRPH), CB_GETCURSEL, 0, 0);
  iR.type = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TRAY_DATATYPE), CB_GETCURSEL, 0, 0);
  iR.level = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TRAY_LEVEL), CB_GETCURSEL, 0, 0) + 1;
  GET_TEXT(IDC_EDIT_TRAY_TIPS, iR.descr);

  addRowToLB(GetDlgItem(*this, IDC_LISTBOX_VARS_TRAY), iR);
}
//-------------------------------------------------------------------
LPCTSTR unSet(LPCTSTR buff, uint& v, uint len)
{
  TCHAR t[64];
  copyStrZ(t, buff, len);
  v = _ttoi(t);
  return buff + len + 1;
}
//-------------------------------------------------------------------
static void unformatRow(LPCTSTR buff, infoRow& iR)
{
  LPCTSTR p = buff;
  p = unSet(p, iR.prph, MAX_LEN_PRPH);
  p = unSet(p, iR.addr, MAX_LEN_ADDR);
  p = unSet(p, iR.type, MAX_LEN_TYPE);
  p = unSet(p, iR.nbit, MAX_LEN_NBIT);
  p = unSet(p, iR.offs, MAX_LEN_OFFS);
  p = unSet(p, iR.level, MAX_LEN_LEVEL);
  copyStrZ(iR.descr, p, MAX_LEN_DESCR);
  lTrim(trim(iR.descr));
}
//-------------------------------------------------------------------
void dManageStdVars3::moveRow(bool up)
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_VARS_TRAY);
  int pos = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(pos - up < 0)
    return;
  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  if(pos + !up > count)
    return;
  TCHAR buff[MAX_BUFF_LB];
  SendMessage(hlb, LB_GETTEXT, pos, (LPARAM)buff);
  SendMessage(hlb, LB_DELETESTRING, pos, 0);
  pos += up ? -1 : 1;
  pos = SendMessage(hlb, LB_INSERTSTRING, pos, (LPARAM)buff);
  SendMessage(hlb, LB_SETCURSEL, pos, 0);
}
//-------------------------------------------------------------------
void dManageStdVars3::remFromLB()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_VARS_TRAY);
  int pos = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  TCHAR buff[MAX_BUFF_LB];
  SendMessage(hlb, LB_GETTEXT, pos, (LPARAM)buff);

  wrapInfoRow wrap;
  infoRow& iR = *wrap;
  unformatRow(buff, iR);

  SET_INT(IDC_EDIT_TRAY_ADDR, iR.addr);
  SET_INT(IDC_EDIT_TRAY_NBIT, iR.nbit);
  SET_INT(IDC_EDIT_TRAY_OFFS, iR.offs);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TRAY_PRPH), CB_SETCURSEL, iR.prph, 0);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TRAY_DATATYPE), CB_SETCURSEL, iR.type, 0);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_TRAY_LEVEL), CB_SETCURSEL, iR.level - 1, 0);
  SET_TEXT(IDC_EDIT_TRAY_TIPS, iR.descr);
  SendMessage(hlb, LB_DELETESTRING, pos, 0);
  if(pos > 0)
    --pos;
  SendMessage(hlb, LB_SETCURSEL, pos, 0);
  PostMessage(hlb, WM_VSCROLL, 0, 0);
}
//------------------------------------------------------------------------------
HBRUSH dManageStdVars3::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  int id = ::GetDlgCtrlID(hWndChild);
  if(WM_CTLCOLORSTATIC == ctlType) {
    uint groupBox[] = {
      IDC_GROUPBOX17,
      IDC_GROUPBOX23,
      };
    for(uint i = 0; i < SIZE_A(groupBox); ++i)
      if(id == groupBox[i]) {
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
//----------------------------------------------------------------------------
extern void fillCBPerif(HWND hwnd, int select);
extern void fillCBTypeVal(HWND hwnd, int select);
//-----------------------------------------------------------
void fillCBLevel(HWND hwnd, int select)
{
  LPCTSTR cbLevel[] = {
    _T("1 - segnalazione"),
    _T("2 - allarme"),
    _T("3 - emergenza"),
    };
  int nElem = SIZE_A(cbLevel);
  for(int i = 0; i < nElem; ++i)
    addStringToComboBox(hwnd, cbLevel[i]);
  if(select < 0 || select >= nElem)
    select = 0;
  SendMessage(hwnd, CB_SETCURSEL, select, 0);
}
//-----------------------------------------------------------
static void fillCBLevelPsw(HWND hwnd, int select)
{
  LPCTSTR cbLevel[] = {
    _T("1 - minimo"),
    _T("2 - medio"),
    _T("3 - massimo"),
    };
  int nElem = SIZE_A(cbLevel);
  for(int i = 0; i < nElem; ++i)
    addStringToComboBox(hwnd, cbLevel[i]);
  if(select < 0 || select >= nElem)
    select = 0;
  SendMessage(hwnd, CB_SETCURSEL, select, 0);
}
//-------------------------------------------------------------------
void dManageStdVars3::fillCombo()
{
  fillCBPerif(GetDlgItem(*this, IDC_COMBOBOX_TRAY_PRPH), 0);
  fillCBTypeVal(GetDlgItem(*this, IDC_COMBOBOX_TRAY_DATATYPE), 0);
  fillCBLevel(GetDlgItem(*this, IDC_COMBOBOX_TRAY_LEVEL), 0);

  fillCBPerif(GetDlgItem(*this, IDC_COMBOBOX_PSW_PRPH), 0);
  fillCBLevelPsw(GetDlgItem(*this, IDC_COMBOBOX_PSW_LEVEL), 0);

  fillCBPerif(GetDlgItem(*this, IDC_COMBOBOX_PING_PRPH), 0);
  fillCBTypeVal(GetDlgItem(*this, IDC_COMBOBOX_PING_TYPE), 0);
}
//-------------------------------------------------------------------
LPCTSTR unSet(LPCTSTR buff, uint& v)
{
  if(!buff)
    return 0;
  v = _ttoi(buff);
  return findNextParamTrim(buff);
}
//-------------------------------------------------------------------
void dManageStdVars3::addRow(LPCTSTR buff)
{
  wrapInfoRow wrap;
  infoRow& iR = *wrap;
  LPCTSTR p = buff;
  p = unSet(p, iR.prph);
  p = unSet(p, iR.addr);
  p = unSet(p, iR.type);
  p = unSet(p, iR.nbit);
  p = unSet(p, iR.offs);
  p = unSet(p, iR.level);
  if(p) {
    uint id = _ttoi(p);
    p = Set.getString(id);
    if(p) {
      copyStrZ(iR.descr, p, MAX_LEN_DESCR);
      lTrim(trim(iR.descr));
      }
    }
  addRowToLB(GetDlgItem(*this, IDC_LISTBOX_VARS_TRAY), iR);
}
//-------------------------------------------------------------------
void dManageStdVars3::fillLB()
{
  LPCTSTR p = Set.getString(INIT_TRAY_DATA);
  if(!p)
    return;
  int nElem = _ttoi(p);
  if(!nElem)
    return;
  p = findNextParamTrim(p);
  if(p) {
    uint id = _ttoi(p);
    LPCTSTR p2 = Set.getString(id);
    if(p2)
      SET_TEXT(IDC_EDIT_TRAY_TIPS_OK, p2);
    }
  p = findNextParamTrim(p);
  if(p) {
    uint id = _ttoi(p);
    p = Set.getString(id);
    }
  if(!p)
    p = _T("NPS - sVisor");
  SET_TEXT(IDC_EDIT_TRAY_TIPS_HEAD, p);

  for(int i = 0; i < nElem; ++i) {
    p = Set.getString(INIT_TRAY_DATA + 1 + i);
    if(!p)
      continue;
    addRow(p);
    }
}
//-------------------------------------------------------------------
void dManageStdVars3::fillDataPsw()
{
  LPCTSTR p = Set.getString(ID_PASSWORD_TIME_LEASE);
  if(!p)
    return;
// id,maxTime,prph,addr,maxLevel
  int v = _ttoi(p);
  SET_INT(IDC_EDIT_PSW_TIME, v);
  p = findNextParamTrim(p);
  if(!p)
    return;
  v = _ttoi(p);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PSW_PRPH), CB_SETCURSEL, v, 0);
  p = findNextParamTrim(p);
  if(!p)
    return;
  v = _ttoi(p);
  SET_INT(IDC_EDIT_PSW_ADDR, v);
  p = findNextParamTrim(p);
  if(!p)
    return;
  v = _ttoi(p);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PSW_LEVEL), CB_SETCURSEL, v - 1, 0);
}
//-------------------------------------------------------------------
void dManageStdVars3::fillDataPing()
{
  LPCTSTR p = Set.getString(ID_RESET_BIT_AS_PING);
  if(!p)
    return;
// id,prph,addr,type,bit
  int v = _ttoi(p);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PING_PRPH), CB_SETCURSEL, v, 0);
  p = findNextParamTrim(p);
  if(!p)
    return;
  v = _ttoi(p);
  SET_INT(IDC_EDIT_PING_ADDR, v);
  p = findNextParamTrim(p);
  if(!p)
    return;
  v = _ttoi(p);
  SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PING_TYPE), CB_SETCURSEL, v, 0);
  p = findNextParamTrim(p);
  if(!p)
    return;
  v = _ttoi(p);
  SET_INT(IDC_EDIT_PING_BIT, v);
}
//-------------------------------------------------------------------
void dManageStdVars3::fillDataCtrl()
{
  fillLB();
  fillCombo();
  fillDataPsw();
  fillDataPing();
  LPCTSTR p = Set.getString(USE_NUMBER_SEPARATOR);
  if(p && _ttoi(p))
    SET_CHECK(IDC_CHECK_USE_NUMBER_SEPARATOR);
}
//-------------------------------------------------------------------
bool dManageStdVars3::save(P_File& pfCrypt, P_File& pfClear)
{
  TCHAR buff[MAX_BUFF_LB];
  do {
    int maxTime = 0;
    int addr = 0;
    GET_INT(IDC_EDIT_PSW_TIME, maxTime);
    GET_INT(IDC_EDIT_PSW_ADDR, addr);
    int prph = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PSW_PRPH), CB_GETCURSEL, 0, 0);
    int maxLevel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PSW_LEVEL), CB_GETCURSEL, 0, 0) + 1;
    wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"), ID_PASSWORD_TIME_LEASE, maxTime, prph, addr, maxLevel);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    } while(false);

  do {
    int bit = 0;
    int addr = 0;
    GET_INT(IDC_EDIT_PING_BIT, bit);
    GET_INT(IDC_EDIT_PING_ADDR, addr);
    int prph = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PING_PRPH), CB_GETCURSEL, 0, 0);
    int type = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PING_TYPE), CB_GETCURSEL, 0, 0);
    wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"), ID_RESET_BIT_AS_PING, prph, addr, type, bit);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    } while(false);

  if(IS_CHECKED(IDC_CHECK_USE_NUMBER_SEPARATOR)) {
    wsprintf(buff, _T("%d,1\r\n"), USE_NUMBER_SEPARATOR);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_VARS_TRAY);
  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  if(count <= 0)
    return true;

  uint id1 = getGlobalStdMsgText();
  uint id2 = getGlobalStdMsgText();
  wsprintf(buff, _T("%d,%d,%d,%d\r\n"), INIT_TRAY_DATA, count, id1, id2);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  do {
    TCHAR t[MAX_BUFF_LB];
    GET_TEXT(IDC_EDIT_TRAY_TIPS_OK, t);
    wsprintf(buff, _T("%d,%s\r\n"), id1, t);
    if(!writeStringChkUnicode(pfClear, pfCrypt, buff))
      return false;
    GET_TEXT(IDC_EDIT_TRAY_TIPS_HEAD, t);
    wsprintf(buff, _T("%d,%s\r\n"), id2, t);
    if(!writeStringChkUnicode(pfClear, pfCrypt, buff))
      return false;
    } while(false);

  for(int i = 0; i < count; ++i) {
    SendMessage(hlb, LB_GETTEXT, i, (LPARAM)buff);
    id1 = getGlobalStdMsgText();
    wrapInfoRow wrap;
    infoRow& iR = *wrap;
    unformatRow(buff, iR);
    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d\r\n"), INIT_TRAY_DATA + 1 + i, iR.prph, iR.addr, iR.type, iR.nbit, iR.offs, iR.level, id1);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    wsprintf(buff, _T("%d,%s\r\n"), id1, iR.descr);
    if(!writeStringChkUnicode(pfClear, pfCrypt, buff))
      return false;
    }
  return true;
}
//-------------------------------------------------------------------
