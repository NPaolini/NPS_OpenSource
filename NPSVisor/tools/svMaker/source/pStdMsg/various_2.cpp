//--------- various_2.cpp -------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <stdio.h>
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
#include "plistbox.h"
#include "common_color.h"
//-------------------------------------------------------------------
class PColorBox_Text : public PColorBox
{
  private:
    typedef PColorBox baseClass;
  public:
    PColorBox_Text(PWin * parent, COLORREF colorText, COLORREF colorBkg, uint resid, HINSTANCE hinst = 0) :
      baseClass(parent, colorBkg,  resid, hinst), ColorText(colorText) {}

    COLORREF getColorText() const { return ColorText; }
    void setText(COLORREF color) { ColorText = color;   InvalidateRect(*this, 0, 0); }
  protected:
  private:
    COLORREF ColorText;
};

//-------------------------------------------------------------------
class dManageStdVars2 : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdVars2(const setOfString& set, PWin* parent, HINSTANCE hInst = 0) :
        baseClass(set, parent, IDD_STD_MSG_FLAG2, hInst)
        {}
    ~dManageStdVars2();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
  protected:
//    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    void fillCombo();
    void fillDataCtrl();
    void fillLB();
    void checkEnableLine();
    void addRow(int addr, float val);
    void addRow(LPCTSTR p);

    void addToLB();
    void remFromLB();

    void fillLB_DLL();
    void choose_DLL();
    void addToLB_DLL();
    void remFromLB_DLL();

    uint getBits();
    void setBits(uint mask);
    uint saveValues(P_File& pfCrypt, P_File& /*pfClear*/);
    void setValues(uint mask);

    void chooseColor(PColorBox_Text* box, bool fg);
    PColorBox_Text* PageColors[2];

};
//-------------------------------------------------------------------
basePage* allocStdVars2( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdVars2(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdVars2::~dManageStdVars2()
{
  destroy();
}
//-------------------------------------------------------------------
#define MAX_LEN_ADDR 6
#define MAX_LEN_VAL  8
#define MAX_BUFF_LB  (MAX_LEN_ADDR + MAX_LEN_VAL + 10)
//-------------------------------------------------------------------
#define MAX_LEN_DLL  60
#define MAX_BUFF_DLL (MAX_LEN_DLL + 10)

#define _BKCOLOR    RGB(250,250,250)
// colore sfondo evidenziato
#define _BKCOLOR_E  RGB(222,222,222)
// colore testo normale
#define _COLORT     RGB(0,0,169)
// colore testo evidenziato
#define _COLORT_E   RGB(0,0,0)
//-------------------------------------------------------------------
LPCTSTR makeColor(COLORREF& target, LPCTSTR p)
{
  if(p)  {
    int r = 255;
    int g = 255;
    int b = 255;
    _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
    target = RGB(r, g, b);
    p = findNextParamTrim(p, 3);
    }
  return p;
}
//-------------------------------------------------------------------
bool dManageStdVars2::create()
{
  PListBox* lb = new PListBox(this, IDC_LISTBOX_MEM_VARS);
  int t[] = {  -MAX_LEN_ADDR, -MAX_LEN_VAL };
  lb->SetTabStop(SIZE_A(t), t, 0);
  lb->SetColorSel(RGB(0, 0, 64), RGB(192, 240, 255));

  lb = new PListBox(this, IDC_LISTBOX_DLL);
  int t2[] = {  MAX_LEN_DLL };
  lb->SetTabStop(SIZE_A(t2), t2, 0);
  lb->SetColorSel(RGB(0, 0, 64), RGB(192, 240, 255));

  COLORREF txtColorNorm = _COLORT;
  COLORREF  bkgColorNorm = _BKCOLOR;
  COLORREF  txtColorSel = _COLORT_E;
  COLORREF  bkgColorSel = _BKCOLOR_E;
  LPCTSTR p = Set.getString(ID_COLOR_FONT_CHOOSE_PAGE);
  uint hFont = 0;
  if(p) {
    p = makeColor(txtColorNorm, p);
    p = makeColor(bkgColorNorm, p);
    p = makeColor(txtColorSel, p);
    p = makeColor(bkgColorSel, p);
    if(p) {
      hFont = _ttoi(p);
      p = findNextParamTrim(p);
      }
    }
  PageColors[0] = new PColorBox_Text(this, txtColorNorm, bkgColorNorm, IDC_STATIC_PAG_COL_NORM);
  PageColors[1] = new PColorBox_Text(this, txtColorSel, bkgColorSel, IDC_STATIC_PAG_COL_SEL);
  if(!baseClass::create())
    return false;

  if(hFont)
    SET_INT(IDC_EDIT_PAG_CHOOSE_H_FONT, hFont);
  if(p)
    SET_TEXT(IDC_EDIT_PAG_CHOOSE_FONT, p);
  fillDataCtrl();
  checkEnableLine();

  return true;
}
//-------------------------------------------------------------------
void dManageStdVars2::chooseColor(PColorBox_Text* box, bool fg)
{
  COLORREF col = fg ? box->getColorText() : box->getColor();
  if(choose_Color(*this, col)) {
    if(fg)
      box->setText(col);
    else
      box->set(col);
    }
}
//----------------------------------------------------------------------------
LRESULT dManageStdVars2::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_MEM_VAR:
          addToLB();
          break;
        case IDC_BUTTON_REM_MEM_VAR:
          remFromLB();
          break;
        case IDC_BUTTON_CHOOSE_DLL:
          choose_DLL();
          break;
        case IDC_BUTTON_ADD_DLL:
          addToLB_DLL();
          break;
        case IDC_BUTTON_REM_DLL:
          remFromLB_DLL();
          break;

        case IDC_RADIOBUTTON_LINE_STOP:
        case IDC_RADIOBUTTON_LINE_RUN:
        case IDC_RADIOBUTTON_LINE_BITS:
        case IDC_RADIOBUTTON_LINE_VALUE:
          checkEnableLine();
          break;
        case IDC_BUTTON_PAG_FGN:
          chooseColor(PageColors[0], true);
          break;
        case IDC_BUTTON_PAG_BGN:
          chooseColor(PageColors[0], false);
          break;
        case IDC_BUTTON_PAG_FGS:
          chooseColor(PageColors[1], true);
          break;
        case IDC_BUTTON_PAG_BGS:
          chooseColor(PageColors[1], false);
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
static bool existInList(uint addr, HWND hlb)
{
  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  if(count <= 0)
    return false;
  TCHAR buff[MAX_BUFF_LB];
  for(int i = 0; i < count; ++i) {
    SendMessage(hlb, LB_GETTEXT, i, (LPARAM)buff);
    buff[MAX_LEN_ADDR] = 0;
    int t = _ttoi(buff);
    if(t == addr)
      return true;
    }
  return false;
}
//-------------------------------------------------------------------
void dManageStdVars2::addToLB()
{
  int addr = 0;
  GET_INT(IDC_EDIT_MEM_ADDR, addr);
  TCHAR value[MAX_BUFF_LB];
  GET_TEXT(IDC_EDIT_MEM_VALUE, value);
  if(!*value)
    return;
  if(existInList(addr, GetDlgItem(*this, IDC_LISTBOX_MEM_VARS)))
    return;

  addRow(addr, (float)_tstof(value));
/*
  TCHAR buff[MAX_BUFF_DIM];
  formatRow(buff, addr, value);
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_MEM_VARS);
  int pos = SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(hlb, LB_SETCURSEL, pos, 0);
*/
}
//-------------------------------------------------------------------
static void unformatRow(LPCTSTR buff, LPTSTR addr, LPTSTR value)
{

  copyStr(addr, buff, MAX_LEN_ADDR);
  addr[MAX_LEN_ADDR] = 0;
  lTrim(trim(addr));

  copyStr(value, buff + MAX_LEN_ADDR + 1, MAX_LEN_VAL);
  value[MAX_LEN_VAL] = 0;
  trim(value);
  zeroTrim(value);
}
//-------------------------------------------------------------------
void dManageStdVars2::remFromLB()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_MEM_VARS);
  int pos = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  TCHAR buff[MAX_BUFF_LB];
  SendMessage(GetDlgItem(*this, IDC_LISTBOX_MEM_VARS), LB_GETTEXT, pos, (LPARAM)buff);

  TCHAR addr[MAX_BUFF_LB];
  TCHAR value[MAX_BUFF_LB];
  unformatRow(buff, addr, value);

  SetDlgItemText(*this, IDC_EDIT_MEM_ADDR, addr);
  SetDlgItemText(*this, IDC_EDIT_MEM_VALUE, value);
  SendMessage(hlb, LB_DELETESTRING, pos, 0);
  if(pos > 0)
    --pos;
  SendMessage(hlb, LB_SETCURSEL, pos, 0);
  PostMessage(hlb, WM_VSCROLL, 0, 0);
}
//-------------------------------------------------------------------
static bool existDllInList(LPCTSTR dll, HWND hlb)
{
  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  if(count <= 0)
    return false;
  TCHAR buff[MAX_BUFF_DLL];
  for(int i = 0; i < count; ++i) {
    SendMessage(hlb, LB_GETTEXT, i, (LPARAM)buff);
    buff[MAX_LEN_DLL] = 0;
    if(!_tcsicmp(buff, dll))
      return true;
    }
  return false;
}
//-------------------------------------------------------------------
void dManageStdVars2::choose_DLL()
{
  TCHAR target[_MAX_PATH] = { 0 };
  if(IDOK == openGenFile(this, target, false, _T("*.dll"), IDD_DIALOG_OPEN_ASSOC_FILE, 0, true, false).modal()) {
    _tcscat_s(target, _T(".dll"));
    SET_TEXT(IDC_EDIT_DLL, target);
    }
}
//-------------------------------------------------------------------
void dManageStdVars2::addToLB_DLL()
{
  TCHAR buff[MAX_BUFF_DLL];
  GET_TEXT(IDC_EDIT_DLL, buff);
  if(!*buff)
    return;
  buff[MAX_LEN_DLL] = 0;

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_DLL);
  if(existDllInList(buff, hlb))
    return;

  int pos = SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(hlb, LB_SETCURSEL, pos, 0);
}
//-------------------------------------------------------------------
void dManageStdVars2::remFromLB_DLL()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_DLL);
  int pos = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  TCHAR buff[MAX_BUFF_DLL];
  SendMessage(GetDlgItem(*this, IDC_LISTBOX_DLL), LB_GETTEXT, pos, (LPARAM)buff);

  trim(buff);
  SetDlgItemText(*this, IDC_EDIT_DLL, buff);
  SendMessage(hlb, LB_DELETESTRING, pos, 0);
  if(pos > 0)
    --pos;
  SendMessage(hlb, LB_SETCURSEL, pos, 0);
  PostMessage(hlb, WM_VSCROLL, 0, 0);
}
//-------------------------------------------------------------------
void dManageStdVars2::checkEnableLine()
{
  bool enable = false;
//  if(IS_CHECKED(IDC_RADIOBUTTON_LINE_STOP) {
//    }
//  else if(IS_CHECKED(IDC_RADIOBUTTON_LINE_RUN) {
//    }
//  else
  if(IS_CHECKED(IDC_RADIOBUTTON_LINE_BITS) || IS_CHECKED(IDC_RADIOBUTTON_LINE_VALUE))
    enable = true;

  uint idc[] = {
      IDC_EDIT_ADDR_STAT_LINE,
      IDC_EDIT_LINE_V1,
      IDC_EDIT_LINE_V2,
      IDC_EDIT_LINE_V3,
      IDC_EDIT_LINE_V4,
      IDC_EDIT_LINE_V5,
      IDC_EDIT_LINE_V6,
      IDC_EDIT_LINE_V7,
      IDC_EDIT_LINE_V8,
      };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
}
//------------------------------------------------------------------------------
/*
void dManageStdVars2::loadData()
{
  int pos = currPos;

  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {

    Rows[i]->setID(Saved[pos].id);
    Rows[i]->setLabel(Saved[pos].label);
    Rows[i]->setDescr(Saved[pos].text);
    }
}
*/
//----------------------------------------------------------------------------
HBRUSH dManageStdVars2::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  int id = ::GetDlgCtrlID(hWndChild);
/*
  if(WM_CTLCOLORSTATIC == ctlType) {
    TCHAR buff[100] = _T("");
    GetWindowText(hWndChild, buff, SIZE_A(buff));
    if(_T('1') <= *buff && *buff <= _T('9')) {
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
    }
*/
/**/
  if(WM_CTLCOLORSTATIC == ctlType) {
    uint groupBox[] = {
      IDC_GROUPBOX23,
      IDC_GROUPBOX17,
      IDC_GROUPBOX15,
      IDC_GROUPBOX13,
      IDC_GROUPBOX33,
      IDC_STATIC_LOCAL_MEM,
      IDC_STATIC_FONT_COLOR
      };
    for(uint i = 0; i < SIZE_A(groupBox); ++i)
      if(id == groupBox[i]) {
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
    if(IDC_STATIC_PAG_COL_NORM == id) {
        SetTextColor(hdc, PageColors[0]->getColorText());
        SetBkColor(hdc, PageColors[0]->getColor());
        return PageColors[0]->getBrush();
        }
    if(IDC_STATIC_PAG_COL_SEL == id) {
        SetTextColor(hdc, PageColors[1]->getColorText());
        SetBkColor(hdc, PageColors[1]->getColor());
        return PageColors[1]->getBrush();
        }

    uint noColor[] = {
      IDC_RADIO_IS_PLC_2,
      IDC_RADIO_IS_PLC_3,
      IDC_RADIO_IS_PLC_4,
      IDC_RADIO_IS_PLC_5,
      IDC_RADIO_IS_PLC_6,
      IDC_RADIO_IS_PLC_7,
      IDC_RADIO_IS_PLC_8,
      IDC_RADIO_IS_PLC_9,
      IDC_RADIOBUTTON_LINE_STOP,
      IDC_RADIOBUTTON_LINE_RUN,
      IDC_RADIOBUTTON_LINE_BITS,
      IDC_RADIOBUTTON_LINE_VALUE,
      IDC_CHECK_NO_SENDBIT1,
      IDC_CHECK_NO_SENDBIT2,
      IDC_CHECK_NO_SENDBIT3,
      IDC_CHECK_NO_SENDBIT4,
      IDC_CHECK_NO_SENDBIT5,
      IDC_CHECK_NO_SENDBIT6,
      IDC_CHECK_NO_SENDBIT7,
      IDC_CHECK_NO_SENDBIT8,

      IDC_CHECK_NO_MMAP_FILE1,
      IDC_CHECK_NO_MMAP_FILE2,
      IDC_CHECK_NO_MMAP_FILE3,
      IDC_CHECK_NO_MMAP_FILE4,
      IDC_CHECK_NO_MMAP_FILE5,
      IDC_CHECK_NO_MMAP_FILE6,
      IDC_CHECK_NO_MMAP_FILE7,
      IDC_CHECK_NO_MMAP_FILE8,
      IDC_CHECK_NO_MMAP_FILE9,
      };

    for(uint i = 0; i < SIZE_A(noColor); ++i)
      if(id == noColor[i])
        return 0;

    SetBkColor(hdc, bkgColor3);
    return (Brush3);
    }
/**/
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, bkgColor);
    return (Brush);
    }
  return 0;
}
//----------------------------------------------------------------------------
extern void fillCBTypeVal(HWND hwnd, int select);
//----------------------------------------------------------------------------
static uint idcPrf[] = {
      IDC_COMBOBOX_PRF_2,
      IDC_COMBOBOX_PRF_3,
      IDC_COMBOBOX_PRF_4,
      IDC_COMBOBOX_PRF_5,
      IDC_COMBOBOX_PRF_6,
      IDC_COMBOBOX_PRF_7,
      IDC_COMBOBOX_PRF_8,
      IDC_COMBOBOX_PRF_9,
      };
//-------------------------------------------------------------------
void dManageStdVars2::fillCombo()
{
  for(uint i = 0; i < SIZE_A(idcPrf); ++i) {
    LPCTSTR p = Set.getString(ID_TYPE_ADDRESS_PLC + i);
    int sel = -1;
    if(p) {
      sel = _ttoi(p);
      p = findNextParamTrim(p);
      }
    if(p) {
      int noBit = _ttoi(p);
      if(noBit)
        SET_CHECK(IDC_CHECK_NO_SENDBIT1 + i);
      p = findNextParamTrim(p);
      }
    if(p) {
      int noMMAPF = _ttoi(p);
      if(noMMAPF)
        SET_CHECK(IDC_CHECK_NO_MMAP_FILE1 + i);
      }

    fillCBTypeVal(GetDlgItem(*this, idcPrf[i]), sel);
    }
 LPCTSTR p = Set.getString(ID_USE_MAPPING_BY_STD_MSG);
 if(p && _ttoi(p))
   SET_CHECK(IDC_CHECK_NO_MMAP_FILE9);
}
//-------------------------------------------------------------------
void dManageStdVars2::addRow(int addr, float val)
{
  TCHAR buff[MAX_BUFF_LB];
  fillStr(buff, _T(' '), MAX_BUFF_LB);
  LPTSTR p = buff;
  TCHAR tmp[50];
  wsprintf(tmp, _T("%d"), addr);
  int len = _tcslen(tmp);
  copyStr(p + MAX_LEN_ADDR - len, tmp, len);
  p += MAX_LEN_ADDR;
  *p++ = _T('\t');
  _stprintf_s(tmp, SIZE_A(tmp), _T("%f"), val);
  zeroTrim(tmp);
  copyStr(p, tmp, _tcslen(tmp));
  p += MAX_LEN_VAL;
  *p++ = 0;

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_MEM_VARS);
  int pos = SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(hlb, LB_SETCURSEL, pos, 0);
}
//-------------------------------------------------------------------
void dManageStdVars2::addRow(LPCTSTR p)
{
  int addr = _ttoi(p);
  p = findNextParam(p, 1);
  if(p) {
    float val = (float)_tstof(p);
    addRow(addr, val);
    }
}
//-------------------------------------------------------------------
void dManageStdVars2::fillLB()
{
  LPCTSTR p = Set.getString(INIT_FIXED_DATA);
  if(!p)
    return;
  int nElem = _ttoi(p);
  for(int i = 0; i < nElem; ++i) {
    p = Set.getString(INIT_FIXED_DATA + 1 + i);
    if(!p)
      continue;
    addRow(p);
    }
}
//-------------------------------------------------------------------
void dManageStdVars2::fillLB_DLL()
{
  LPCTSTR p = Set.getString(ID_DLL_NAME);
  if(!p)
    return;

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_DLL);
  int pos = 0;

  pvvChar target;
  uint nElem = splitParam(target, p);
  for(uint i = 0; i < nElem; ++i) {
    pos = SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)target[i].getVect());
    }

  SendMessage(hlb, LB_SETCURSEL, pos, 0);
}
//-------------------------------------------------------------------
static uint idcValues[] = {
      IDC_EDIT_LINE_V1,
      IDC_EDIT_LINE_V2,
      IDC_EDIT_LINE_V3,
      IDC_EDIT_LINE_V4,
      IDC_EDIT_LINE_V5,
      IDC_EDIT_LINE_V6,
      IDC_EDIT_LINE_V7,
      IDC_EDIT_LINE_V8,
      };
//-------------------------------------------------------------------
uint dManageStdVars2::getBits()
{
  uint bits = 0;
  for(int i = SIZE_A(idcValues) - 1; i >= 0; --i) {
    TCHAR v[20];
    GET_TEXT(idcValues[i], v);
    if(*v)
      bits |= 1 << _ttoi(v);
    }
  return bits;
}
//-------------------------------------------------------------------
void dManageStdVars2::setBits(uint mask)
{
  uint pos = 0;
  for(uint i = 0; pos < SIZE_A(idcValues) && mask; ++i, mask >>= 1)
    if(mask & 1) {
      SET_INT(idcValues[pos], i);
      ++pos;
      }
}
//-------------------------------------------------------------------
uint dManageStdVars2::saveValues(P_File& pfCrypt, P_File& /*pfClear*/)
{
  uint count = 0;
  TCHAR buff[50];
  for(uint i = 0; i < SIZE_A(idcValues); ++i) {
    int v;
    GET_INT(idcValues[i], v);
    if(!v)
      continue;
    wsprintf(buff, _T("%d,%d\r\n"), ID_MANAGE_RUNNING + 1 + i, v);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return 0;
    ++count;
    }
  return count;
}
//-------------------------------------------------------------------
void dManageStdVars2::setValues(uint mask)
{
  for(uint i = 0; i < SIZE_A(idcValues); ++i, mask >>= 1) {
    int val = 0;
    LPCTSTR p = Set.getString(ID_MANAGE_RUNNING + 1 + i);
    if(p)
      SET_TEXT(idcValues[i], p);
    }
}
//-------------------------------------------------------------------
void dManageStdVars2::fillDataCtrl()
{
  LPCTSTR p = Set.getString(ID_NUM_PRF_PLC);
  int sel = 0;
  if(p)
    sel = _ttoi(p) - 2;
  SET_CHECK(IDC_RADIO_IS_PLC_2 + sel);

  p = Set.getString(ID_ADDR_ALWAYS_LOCAL);
  if(p)
    SET_TEXT(IDC_EDIT_LOCAL_DWORD, p);

  fillLB();
  fillLB_DLL();
  fillCombo();

  p = Set.getString(ID_MANAGE_RUNNING);
  if(!p)
    return;
  int type = 0;
  int addr = 0;
  int mask = 0;
  _stscanf_s(p, _T("%d,%d,%d"), &type, &addr, &mask);
  switch(type) {
    case 0:
      SET_CHECK(IDC_RADIOBUTTON_LINE_STOP);
      break;
    case 1:
      SET_CHECK(IDC_RADIOBUTTON_LINE_RUN);
      break;
    case 2:
      SET_CHECK(IDC_RADIOBUTTON_LINE_BITS);
      SET_INT(IDC_EDIT_ADDR_STAT_LINE, addr);
      setBits(mask);
      break;
    case 3:
      SET_CHECK(IDC_RADIOBUTTON_LINE_VALUE);
      SET_INT(IDC_EDIT_ADDR_STAT_LINE, addr);
      setValues(mask);
      break;
    }

  p = Set.getString(ID_USE_TOUCH_KEYB);
  if(p && _ttoi(p)) {
    SET_CHECK(IDC_CHECK_TOUCH_SCREEN);
    p = findNextParam(p, 1);
    SET_TEXT(IDC_EDIT_KEYB_ALPHA, p);
    p = Set.getString(ID_NUMB_TOUCH_KEYB);
    if(p && *p)
      SET_TEXT(IDC_EDIT_KEYB_NUMB, p);
    }
}
//-------------------------------------------------------------------
static
LPTSTR colorToStr(LPTSTR p, const COLORREF& col)
{
  wsprintf(p, _T(",%d,%d,%d"), GetRValue(col), GetGValue(col), GetBValue(col));
  return p + _tcslen(p);
}
//-------------------------------------------------------------------
bool dManageStdVars2::save(P_File& pfCrypt, P_File& pfClear)
{
  TCHAR buff[500];
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_MEM_VARS);
  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  if(count < 0)
    count = 0;
  wsprintf(buff, _T("%d,%d\r\n"), INIT_FIXED_DATA, count);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  for(int i = 0; i < count; ++i) {
    TCHAR t[MAX_BUFF_LB];
    SendMessage(hlb, LB_GETTEXT, i, (LPARAM)t);

    TCHAR addr[MAX_BUFF_LB];
    TCHAR value[MAX_BUFF_LB];
    unformatRow(t, addr, value);
    wsprintf(buff, _T("%d,%s,%s\r\n"), INIT_FIXED_DATA + 1 + i, addr, value);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  hlb = GetDlgItem(*this, IDC_LISTBOX_DLL);
  count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  if(count > 0) {
    wsprintf(buff, _T("%d"), ID_DLL_NAME, count);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    buff[0] = _T(',');
    for(int i = 0; i < count; ++i) {
      SendMessage(hlb, LB_GETTEXT, i, (LPARAM)(buff + 1));
      if(!writeStringChkUnicode(pfCrypt, buff))
        return false;
      }
    if(!writeStringChkUnicode(pfCrypt, _T("\r\n")))
      return false;
    }

  for(uint i = 0; i < SIZE_A(idcPrf); ++i) {
    int pos = SendMessage(GetDlgItem(*this, idcPrf[i]), CB_GETCURSEL, 0, 0);
    int noBit = IS_CHECKED(IDC_CHECK_NO_SENDBIT1 + i);
    int noMMAPF = IS_CHECKED(IDC_CHECK_NO_MMAP_FILE1 + i);
    if(pos > 0) {
      wsprintf(buff, _T("%d,%d,%d,%d\r\n"), ID_TYPE_ADDRESS_PLC + i, pos, noBit, noMMAPF);
      if(!writeStringChkUnicode(pfCrypt, buff))
        return false;
      }
    if(IS_CHECKED(IDC_RADIO_IS_PLC_2 + i)) {
      wsprintf(buff, _T("%d,%d\r\n"), ID_NUM_PRF_PLC, i + 2);
      if(!writeStringChkUnicode(pfCrypt, buff))
        return false;
      }
    }
  if(IS_CHECKED(IDC_CHECK_NO_MMAP_FILE9)) {
    wsprintf(buff, _T("%d,1\r\n"), ID_USE_MAPPING_BY_STD_MSG);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  if(IS_CHECKED(IDC_RADIOBUTTON_LINE_STOP))
    wsprintf(buff, _T("%d,%d,0,0\r\n"), ID_MANAGE_RUNNING, 0);

  else if(IS_CHECKED(IDC_RADIOBUTTON_LINE_RUN))
    wsprintf(buff, _T("%d,%d,0,0\r\n"), ID_MANAGE_RUNNING, 1);

  else if(IS_CHECKED(IDC_RADIOBUTTON_LINE_BITS)) {
    int addr = 0;
    GET_INT(IDC_EDIT_ADDR_STAT_LINE, addr);
    int mask = getBits();
    wsprintf(buff, _T("%d,%d,%d,%d\r\n"), ID_MANAGE_RUNNING, 2, addr, mask);
    }

  else if(IS_CHECKED(IDC_RADIOBUTTON_LINE_VALUE)) {
    uint count = saveValues(pfCrypt, pfClear);
    int addr = 0;
    GET_INT(IDC_EDIT_ADDR_STAT_LINE, addr);
    wsprintf(buff, _T("%d,%d,%d,%d\r\n"), ID_MANAGE_RUNNING, 3, addr, count);
    }

  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  GET_TEXT(IDC_EDIT_LOCAL_DWORD, buff);
  if(*buff) {
    TCHAR t[50];
    wsprintf(t, _T("%d,"), ID_ADDR_ALWAYS_LOCAL);
    if(!writeStringChkUnicode(pfCrypt, t))
      return false;
    _tcscat_s(buff, _T("\r\n"));
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  if(IS_CHECKED(IDC_CHECK_TOUCH_SCREEN)) {
    TCHAR t[512];
    TCHAR t2[512];
    GET_TEXT(IDC_EDIT_KEYB_ALPHA, t2);
    translateFromCRNL(t, t2);
    wsprintf(buff, _T("%d,1,%s\r\n"), ID_USE_TOUCH_KEYB, t);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    GET_TEXT(IDC_EDIT_KEYB_NUMB, t2);
    if(*t2) {
      translateFromCRNL(t, t2);
      wsprintf(buff, _T("%d,%s\r\n"), ID_NUMB_TOUCH_KEYB, t);
      if(!writeStringChkUnicode(pfCrypt, buff))
        return false;
      }
    }
  wsprintf(buff, _T("%d"), ID_COLOR_FONT_CHOOSE_PAGE);
  LPTSTR p = buff + _tcslen(buff);
  p = colorToStr(p, PageColors[0]->getColorText());
  p = colorToStr(p, PageColors[0]->getColor());
  p = colorToStr(p, PageColors[1]->getColorText());
  p = colorToStr(p, PageColors[1]->getColor());
  TCHAR t[512];
  GET_TEXT(IDC_EDIT_PAG_CHOOSE_FONT, t);
  trim(lTrim(t));
  int hf = 0;
  GET_INT(IDC_EDIT_PAG_CHOOSE_H_FONT, hf);
  if(hf && *t)
    wsprintf(p, _T(",%d,%s\r\n"), hf, t);
  else
    wsprintf(p, _T("\r\n"));
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;
  return true;
}
//-------------------------------------------------------------------
