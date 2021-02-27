//------------- alarm.cpp -------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <stdio.h>
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
//-------------------------------------------------------------------
#define OFFSET_CTRL 20
//-------------------------------------------------------------------
#define OFFSET_H 16
//-------------------------------------------------------------------
#define MAX_ROW_PRF 9
//-------------------------------------------------------------------
class dManageStdAlarm : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdAlarm(const setOfString& set, PWin* parent, HINSTANCE hInst = 0) :
        baseClass(set, parent, IDD_STD_MSG_ALARM, hInst)
        {}
    ~dManageStdAlarm();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
  protected:
//    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:

    void fillDataCtrl();
    void checkEnableLine(uint pos);
    void createCtrl(int pos, int offs);
};
//-------------------------------------------------------------------
basePage* allocStdAlarm( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdAlarm(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdAlarm::~dManageStdAlarm()
{
  destroy();
}
//-------------------------------------------------------------------
static uint idcEdit[] = {
        IDC_EDIT_ALARM_INIT_2,
        IDC_EDIT_ALARM_NWORD_2,
        IDC_EDIT_ALARM_OFFS_EV_2,
        IDC_EDIT_ALARM_NWORD_EV_2,
//        IDC_CHECK_USE_STATUS_FLAG_2,
        IDC_EDIT_ALARM_ADDR_STATUS_2,
        IDC_EDIT_ALARM_BIT_MASK_2,
        IDC_EDIT_ALARM_BIT_MASK_EV_2,
        IDC_EDIT_ALARM_DATA_TYPE_2,
        IDC_EDIT_RESET_ADDR_2,
        IDC_EDIT_RESET_BIT_2,
        };
//-------------------------------------------------------------------
bool dManageStdAlarm::create()
{
  for(uint i = 0; i < SIZE_A(idcEdit); ++i)
    new PEdit(this, idcEdit[i]);

  if(!baseClass::create())
    return false;

  PRect r1;
  GetWindowRect(GetDlgItem(*this, IDC_CHECK_USE_STATUS_FLAG_2), r1);
  PRect r2;
  GetWindowRect(GetDlgItem(*this, IDC_CHECK_USE_STATUS_FLAG_3), r2);

  int offs = r2.top - r1.top;
  for(int i = 1; i < MAX_ROW_PRF; ++i)
    createCtrl(i, offs);

  fillDataCtrl();
  for(int i = 0; i < MAX_ROW_PRF; ++i)
    checkEnableLine(i);

  return true;
}
//-------------------------------------------------------------------
void dManageStdAlarm::createCtrl(int pos, int offs)
{

  HFONT font = (HFONT)SendMessage(GetDlgItem(*this, idcEdit[0]), WM_GETFONT, 0, 0);
  for(uint i = 0; i < SIZE_A(idcEdit); ++i) {
    HWND base = GetDlgItem(*this, idcEdit[i]);
    PRect r;
    GetWindowRect(base, r);
    MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
    r.Offset(0, pos * offs);
    PEdit* ed = new PEdit(this, idcEdit[i] + pos, r);
    ed->create();
    ed->setFont(font);
    }

}
//----------------------------------------------------------------------------
LRESULT dManageStdAlarm::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      if(LOWORD(wParam) >= IDC_CHECK_USE_STATUS_FLAG_2 && LOWORD(wParam) < IDC_CHECK_USE_STATUS_FLAG_2 + 8) {
        checkEnableLine(LOWORD(wParam) - IDC_CHECK_USE_STATUS_FLAG_2);
        break;
        }
//      switch(LOWORD(wParam)) {
//          break;
//        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
void dManageStdAlarm::checkEnableLine(uint pos)
{
  bool enable = IS_CHECKED(IDC_CHECK_USE_STATUS_FLAG_2 + pos);

  uint idc[] = {
      IDC_EDIT_ALARM_ADDR_STATUS_2,
      IDC_EDIT_ALARM_BIT_MASK_2,
      IDC_EDIT_ALARM_BIT_MASK_EV_2,
      };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i] + pos, enable);
}
//------------------------------------------------------------------------------
HBRUSH dManageStdAlarm::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
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
      IDC_GROUPBOX16,
      IDC_GROUPBOX15,
      IDC_GROUPBOX21,
      };
    for(uint i = 0; i < SIZE_A(groupBox); ++i)
      if(id == groupBox[i]) {
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }

    uint noColor[] = {
      IDC_CHECK_AUTO_OPEN_ALARM,
      IDC_CHECK_USE64BIT,
      IDC_CHECK_USE_STATUS_FLAG_2,
      IDC_CHECK_USE_STATUS_FLAG_3,
      IDC_CHECK_USE_STATUS_FLAG_4,
      IDC_CHECK_USE_STATUS_FLAG_5,
      IDC_CHECK_USE_STATUS_FLAG_6,
      IDC_CHECK_USE_STATUS_FLAG_7,
      IDC_CHECK_USE_STATUS_FLAG_8,
      IDC_CHECK_USE_STATUS_FLAG_9,
      IDC_CHECK_USE_STATUS_FLAG_10,
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
void dManageStdAlarm::fillDataCtrl()
{
  LPCTSTR p = Set.getString(ID_MANAGE_RESET);
  for(int i = 0; i < MAX_ROW_PRF - 1; ++i) {
    if(!p)
      break;
    int addr = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    int bit = _ttoi(p);
    SET_INT(IDC_EDIT_RESET_ADDR_2 + i, addr);
    SET_INT(IDC_EDIT_RESET_BIT_2 + i, bit);
    p = findNextParamTrim(p);
    }
  p = Set.getString(ID_FILTER_ALARM);
  if(p) {
    int addr;
    int autoOpen;
    int warningLevel = 0;
    int use64bitFilter = 0;
    _stscanf_s(p, _T("%d,%d,%d,%d"), &addr, &autoOpen, &warningLevel, &use64bitFilter);
    SET_INT(IDC_EDIT_ALARM_FILTER, addr);
    SET_CHECK_SET(IDC_CHECK_AUTO_OPEN_ALARM, autoOpen);
    SET_INT(IDC_EDIT_WARNING_LEVEL, warningLevel);
    SET_CHECK_SET(IDC_CHECK_USE64BIT, use64bitFilter);
    }

  for(int i = 0; i < MAX_ROW_PRF; ++i) {
    p = Set.getString(ID_MANAGE_ALARM_PLC + i);
    if(!p)
      continue;
    int addr = 0;
    int numWord = 0;
    int offsEvent = 0;
    int numWordEvent = 0;
    int useStatusFlag = 0;
    int wordStatus = 0;
    int bitMaskAlarm = 0;
    int bitMaskEvent = 0;
    int typeWord = 0;
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d"), &addr, &numWord,
          &offsEvent, &numWordEvent, &useStatusFlag, &wordStatus,
          &bitMaskAlarm, &bitMaskEvent, &typeWord);
    SET_INT(IDC_EDIT_ALARM_INIT_2 + i, addr);
    SET_INT(IDC_EDIT_ALARM_NWORD_2 + i, numWord);
    SET_INT(IDC_EDIT_ALARM_OFFS_EV_2 + i, offsEvent);
    SET_INT(IDC_EDIT_ALARM_NWORD_EV_2 + i, numWordEvent);
    if(i != MAX_ROW_PRF - 1)
      SET_CHECK_SET(IDC_CHECK_USE_STATUS_FLAG_2 + i, useStatusFlag);
    SET_INT(IDC_EDIT_ALARM_ADDR_STATUS_2 + i, wordStatus);
    SET_INT(IDC_EDIT_ALARM_BIT_MASK_2 + i, bitMaskAlarm);
    SET_INT(IDC_EDIT_ALARM_BIT_MASK_EV_2 + i, bitMaskEvent);
    SET_INT(IDC_EDIT_ALARM_DATA_TYPE_2 + i, typeWord);
    }
  ENABLE(IDC_CHECK_USE_STATUS_FLAG_10, false);
  ENABLE(IDC_EDIT_ALARM_DATA_TYPE_2 + MAX_ROW_PRF - 1, false);
  ENABLE(IDC_EDIT_RESET_ADDR_2 + MAX_ROW_PRF - 1, false);
  ENABLE(IDC_EDIT_RESET_BIT_2 + MAX_ROW_PRF - 1, false);
}
//-------------------------------------------------------------------
bool dManageStdAlarm::save(P_File& pfCrypt, P_File& pfClear)
{
  TCHAR buff[500];
  int addr;
  int val;
  wsprintf(buff, _T("%d"), ID_MANAGE_RESET);
  LPTSTR p = buff + _tcslen(buff);
  for(int i = 0; i < MAX_ROW_PRF - 1; ++i) {
    addr = 0;
    val = -1;
    TCHAR t[20];
    GET_TEXT(IDC_EDIT_RESET_BIT_2 + i, t);
    if(*t) {
      val = _ttoi(t);
      GET_INT(IDC_EDIT_RESET_ADDR_2 + i, addr);
      }
    wsprintf(p, _T(",%d,%d"), addr, val);
    p += _tcslen(p);
    }
  wsprintf(p, _T("\r\n"));
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  GET_INT(IDC_EDIT_ALARM_FILTER, addr);
  if(addr) {
    val = IS_CHECKED(IDC_CHECK_AUTO_OPEN_ALARM) ? 1 : 0;
    int warningLevel;
    GET_INT(IDC_EDIT_WARNING_LEVEL, warningLevel);
    int use64bitFilter = IS_CHECKED(IDC_CHECK_USE64BIT) ? 1 : 0;
    wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"), ID_FILTER_ALARM, addr, val, warningLevel, use64bitFilter);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  for(int i = 0; i < MAX_ROW_PRF; ++i) {
    int numWord = 0;
    GET_INT(IDC_EDIT_ALARM_NWORD_2 + i, numWord);
    if(!numWord)
      continue;
    int addr = 0;
    int offsEvent = 0;
    int numWordEvent = 0;
    int wordStatus = 0;
    int bitMaskAlarm = 0;
    int bitMaskEvent = 0;
    int typeWord = 0;
    GET_INT(IDC_EDIT_ALARM_INIT_2 + i, addr);
    GET_INT(IDC_EDIT_ALARM_OFFS_EV_2 + i, offsEvent);
    GET_INT(IDC_EDIT_ALARM_NWORD_EV_2 + i, numWordEvent);
    int useStatusFlag = IS_CHECKED(IDC_CHECK_USE_STATUS_FLAG_2 + i) ? 1 : 0;
    GET_INT(IDC_EDIT_ALARM_ADDR_STATUS_2 + i, wordStatus);
    GET_INT(IDC_EDIT_ALARM_BIT_MASK_2 + i, bitMaskAlarm);
    GET_INT(IDC_EDIT_ALARM_BIT_MASK_EV_2 + i, bitMaskEvent);
    GET_INT(IDC_EDIT_ALARM_DATA_TYPE_2 + i, typeWord);
    if(!typeWord) {
      typeWord = 4;
      SET_INT(IDC_EDIT_ALARM_DATA_TYPE_2 + i, typeWord);
      }

    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"), ID_MANAGE_ALARM_PLC + i,
        addr, numWord, offsEvent, numWordEvent, useStatusFlag,
        wordStatus, bitMaskAlarm, bitMaskEvent, typeWord);

    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }
  return true;
}
//-------------------------------------------------------------------
