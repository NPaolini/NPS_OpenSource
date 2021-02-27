//--------- print_1.cpp ---------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <stdio.h>
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
#include "plistbox.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
class dManageStdPrint1 : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdPrint1(const setOfString& set, PWin* parent, HINSTANCE hInst = 0) :
        baseClass(set, parent, IDD_STD_MSG_PRINT_1, hInst)
        {
          uint idc[] = {
            IDC_EDIT_SA_TXT_1,
            IDC_EDIT_SA_TXT_2,
            IDC_EDIT_SA_TXT_3,
            IDC_EDIT_SA_TXT_4,
            IDC_EDIT_SRA_TXT_1,
            IDC_EDIT_SRA_TXT_2,
            IDC_EDIT_SRA_TXT_3,
            IDC_EDIT_SRA_TXT_4,
            };
          for(uint i = 0; i < SIZE_A(idc); ++i)
            new langEdit(this, idc[i]);
        }

    ~dManageStdPrint1();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
  protected:
//    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    void fillDataCtrl();
    void fillDataCtrlGen(uint id, uint idc[], uint idc2[4][3]);
    bool saveDataCtrlGen(P_File& pfCrypt, P_File& pfClear, uint id, uint idc[], uint idc2[4][3]);
};
//-------------------------------------------------------------------
basePage* allocStdPrint1( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdPrint1(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdPrint1::~dManageStdPrint1()
{
  destroy();
}
//-------------------------------------------------------------------
bool dManageStdPrint1::create()
{
  if(!baseClass::create())
    return false;


  fillDataCtrl();

  return true;
}
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT dManageStdPrint1::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
/*
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_MEM_VAR:
          addToLB();
          break;
        case IDC_BUTTON_REM_MEM_VAR:
          remFromLB();
          break;

        case IDC_RADIOBUTTON_LINE_STOP:
        case IDC_RADIOBUTTON_LINE_RUN:
        case IDC_RADIOBUTTON_LINE_BITS:
        case IDC_RADIOBUTTON_LINE_VALUE:
          checkEnableLine();
          break;
        }
      break;

    }
*/
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HBRUSH dManageStdPrint1::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    uint groupBox[] = {
      IDC_GROUPBOX_A,
      IDC_GROUPBOX_RA,
      };
    for(uint i = 0; i < SIZE_A(groupBox); ++i)
      if(GetDlgItem(*this, groupBox[i]) == hWndChild) {
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
    TCHAR buff[10] = _T("");
    GetWindowText(hWndChild, buff, SIZE_A(buff));
    if(_T('1') <= *buff && *buff <= _T('9')) {
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
/*
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
      };

    for(int i = 0; i < SIZE_A(noColor); ++i)
      if(GetDlgItem(*this, noColor[i]) == hWndChild)
        return 0;
*/
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
static LPCTSTR checkNewMode(const setOfString& set, LPCTSTR p)
{
  if(!p)
    return 0;
  uint len = _tcslen(p);
  for(uint i = 0; i < len; ++i)
    if(!_istdigit(p[i]))
      return p;
  DWORD id = _ttoi(p);
  if(id >= MIN_INIT_STD_MSG_TEXT) {
    LPCTSTR p2 = set.getString(id);
    if(p2)
      return p2;
    }
  return p;
}
//----------------------------------------------------------------------------
void dManageStdPrint1::fillDataCtrlGen(uint id, uint idc[], uint idc2[4][3])
{
  int nL;
  int hF = 0;
  int hR = 0;
  int left = -1;
  int top = -1;
  int right = -1;
  int bottom = -1;
  int flag = 0;
  TCHAR nameFont[200] = _T("\0");

  LPCTSTR p = Set.getString(id);
  if(!p)
    return;

// 540,4,14,50,100,100,100,100,0,comic sans ms

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d"),
        &nL, &hF, &hR,
        &left, &top, &right, &bottom,
        &flag);
  p = findNextParam(p, 8);
  if(p)
    _tcscpy_s(nameFont, p);
/*
  SET_INT(IDC_EDIT_SA_HFONT, hF);
  SET_INT(IDC_EDIT_SA_HROW, hR);
  SET_INT(IDC_EDIT_SA_LEFT, left);
  SET_INT(IDC_EDIT_SA_TOP, top);
  SET_INT(IDC_EDIT_SA_RIGHT, right);
  SET_INT(IDC_EDIT_SA_BOTTOM, bottom);
  if(flag & 1)
    SET_CHECK(IDC_CHECK_SA_PREVIEW);
  if(flag & 2)
    SET_CHECK(IDC_CHECK_SA_SETUP);
  SET_TEXT(IDC_EDIT_SA_NAMEFONT, nameFont);
*/
  SET_INT(idc[0], hF);
  SET_INT(idc[1], hR);
  SET_INT(idc[2], left);
  SET_INT(idc[3], top);
  SET_INT(idc[4], right);
  SET_INT(idc[5], bottom);
  if(flag & 1)
    SET_CHECK(idc[6]);
  if(flag & 2)
    SET_CHECK(idc[7]);
  SET_TEXT(idc[8], nameFont);

  nL = min(nL, 4);
/*
    int idc[][] = {
      { IDC_EDIT_SA_X_1, IDC_EDIT_SA_AL_1, IDC_EDIT_SA_TXT_1 },
      { IDC_EDIT_SA_X_2, IDC_EDIT_SA_AL_2, IDC_EDIT_SA_TXT_2 },
      { IDC_EDIT_SA_X_3, IDC_EDIT_SA_AL_3, IDC_EDIT_SA_TXT_3 },
      { IDC_EDIT_SA_X_4, IDC_EDIT_SA_AL_4, IDC_EDIT_SA_TXT_4 },
      };
*/
  for(int i = 0; i < nL; ++i) {
    p = Set.getString(id + i + 1);
    if(!p)
      continue;
    int x = 0;
    int al = 0;
    TCHAR txt[100];
    _stscanf_s(p, _T("%d,%d"), &x, &al);
    p = findNextParam(p, 2);
    p = checkNewMode(Set, p);
    if(p)
      _tcscpy_s(txt, p);
    SET_INT(idc2[i][0], x);
    SET_INT(idc2[i][1], al);
    SET_TEXT(idc2[i][2], txt);
    }
}
//----------------------------------------------------------------------------
static uint idcA[] = {
      IDC_EDIT_SA_HFONT,
      IDC_EDIT_SA_HROW,
      IDC_EDIT_SA_LEFT,
      IDC_EDIT_SA_TOP,
      IDC_EDIT_SA_RIGHT,
      IDC_EDIT_SA_BOTTOM,
      IDC_CHECK_SA_PREVIEW,
      IDC_CHECK_SA_SETUP,
      IDC_EDIT_SA_NAMEFONT,
      };

static uint idcA2[4][3] = {
      { IDC_EDIT_SA_X_1, IDC_EDIT_SA_AL_1, IDC_EDIT_SA_TXT_1 },
      { IDC_EDIT_SA_X_2, IDC_EDIT_SA_AL_2, IDC_EDIT_SA_TXT_2 },
      { IDC_EDIT_SA_X_3, IDC_EDIT_SA_AL_3, IDC_EDIT_SA_TXT_3 },
      { IDC_EDIT_SA_X_4, IDC_EDIT_SA_AL_4, IDC_EDIT_SA_TXT_4 },
      };
//----------------------------------------------------------------------------
static uint idcRA[] = {
      IDC_EDIT_SRA_HFONT,
      IDC_EDIT_SRA_HROW,
      IDC_EDIT_SRA_LEFT,
      IDC_EDIT_SRA_TOP,
      IDC_EDIT_SRA_RIGHT,
      IDC_EDIT_SRA_BOTTOM,
      IDC_CHECK_SRA_PREVIEW,
      IDC_CHECK_SRA_SETUP,
      IDC_EDIT_SRA_NAMEFONT,
      };

static uint idcRA2[4][3] = {
      { IDC_EDIT_SRA_X_1, IDC_EDIT_SRA_AL_1, IDC_EDIT_SRA_TXT_1 },
      { IDC_EDIT_SRA_X_2, IDC_EDIT_SRA_AL_2, IDC_EDIT_SRA_TXT_2 },
      { IDC_EDIT_SRA_X_3, IDC_EDIT_SRA_AL_3, IDC_EDIT_SRA_TXT_3 },
      { IDC_EDIT_SRA_X_4, IDC_EDIT_SRA_AL_4, IDC_EDIT_SRA_TXT_4 },
      };
//----------------------------------------------------------------------------
void dManageStdPrint1::fillDataCtrl()
{
  fillDataCtrlGen(ID_INIT_LABEL_PRINT_ALARM, idcA, idcA2);

  fillDataCtrlGen(ID_INIT_LABEL_PRINT_REPORT, idcRA, idcRA2);
}
//----------------------------------------------------------------------------
#define GET_INT_IF(idc, val)  \
  GetDlgItemText(*this, idc, buff, SIZE_A(buff)); \
  if(_istdigit((unsigned)*buff)) \
    val = _ttoi(buff)
//----------------------------------------------------------------------------
bool dManageStdPrint1::saveDataCtrlGen(P_File& pfCrypt, P_File& pfClear, uint id, uint idc[], uint idc2[4][3])
{
  int nL = 4;
  int hF = 0;
  int hR = 0;
  int left = -1;
  int top = -1;
  int right = -1;
  int bottom = -1;
  int flag = 0;
  TCHAR nameFont[200];

  GET_TEXT(idc[8], nameFont);
  if(!*nameFont)
    _tcscpy_s(nameFont, _T("arial"));
  //  return true;

  TCHAR buff[500];

  GET_INT(idc[0], hF);
  GET_INT(idc[1], hR);
  GET_INT_IF(idc[2], left);
  GET_INT_IF(idc[3], top);
  GET_INT_IF(idc[4], right);
  GET_INT_IF(idc[5], bottom);

  if(IS_CHECKED(idc[6]))
    flag |= 1;
  if(IS_CHECKED(idc[7]))
    flag |= 2;

// 540,4,14,50,100,100,100,100,0,comic sans ms

  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\r\n"), id,
        nL, hF, hR,
        left, top, right, bottom,
        flag, nameFont);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;
  for(int i = 0; i < nL; ++i) {
    int x = 0;
    int al = 0;
    TCHAR txt[100];
    GET_INT(idc2[i][0], x);
    GET_INT(idc2[i][1], al);
    GET_TEXT(idc2[i][2], txt);
    DWORD idText = getGlobalStdMsgText();
    wsprintf(buff, _T("%d,%d,%d,%d\r\n"), id + 1 + i, x, al, idText);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    wsprintf(buff, _T("%d,%s\r\n"), idText, txt);
    if(!writeStringChkUnicode(pfClear, pfCrypt, buff))
      return false;
    }
  return true;
}
//-------------------------------------------------------------------
bool dManageStdPrint1::save(P_File& pfCrypt, P_File& pfClear)
{
  if(saveDataCtrlGen(pfCrypt, pfClear, ID_INIT_LABEL_PRINT_ALARM, idcA, idcA2))
    return saveDataCtrlGen(pfCrypt, pfClear, ID_INIT_LABEL_PRINT_REPORT, idcRA, idcRA2);
  return false;
}
//-------------------------------------------------------------------
