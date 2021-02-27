//--------- language.cpp --------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <stdio.h>
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
#include "plistbox.h"
//-------------------------------------------------------------------
static int gCurrLang;
//-------------------------------------------------------------------
void setCurrLang(int curr) { gCurrLang = curr; }
//-------------------------------------------------------------------
extern
void makeStdMsgSet(setOfString& set);
//-------------------------------------------------------------------
void loadCurrLang(bool& hideTitle)
{
  setOfString Set;
  makeStdMsgSet(Set);

  LPCTSTR pl = Set.getString(ID_LANGUAGE);
  gCurrLang = 0;
  if(pl)
    gCurrLang = _ttoi(pl);

  pl = Set.getString(ID_HIDE_BAR);
  hideTitle = false;
  if(pl && 1 == _ttoi(pl))
    hideTitle = true;
}
//-------------------------------------------------------------------
int getCurrLang() { return gCurrLang; }
//-------------------------------------------------------------------
class dManageStdLang : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdLang(const setOfString& set, PWin* parent, HINSTANCE hInst = 0) :
        baseClass(set, parent, IDD_STD_MSG_LANG, hInst)
        {}
    ~dManageStdLang();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
  protected:
//    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    void fillDataCtrl();
    void checkEnable();
    void invalidateClient(int curr);
};
//-------------------------------------------------------------------
basePage* allocStdLang( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdLang(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdLang::~dManageStdLang()
{
  destroy();
}
//-------------------------------------------------------------------
bool dManageStdLang::create()
{
  if(!baseClass::create())
    return false;


  fillDataCtrl();

  return true;
}
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT dManageStdLang::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECK_ENABLE_LANG:
          checkEnable();
          break;
        case IDC_RADIO_LANG_1:
        case IDC_RADIO_LANG_2:
        case IDC_RADIO_LANG_3:
        case IDC_RADIO_LANG_4:
        case IDC_RADIO_LANG_5:
        case IDC_RADIO_LANG_6:
        case IDC_RADIO_LANG_7:
        case IDC_RADIO_LANG_8:
        case IDC_RADIO_LANG_9:
        case IDC_RADIO_LANG_10:
          invalidateClient(LOWORD(wParam) - IDC_RADIO_LANG_1 + 1);
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void dManageStdLang::invalidateClient(int curr)
{
  svmMainClient* mc = getMainClient(this);
  if(mc) {
    setCurrLang(curr);
    InvalidateRect(*mc, 0, 0);
    }
}
//----------------------------------------------------------------------------
HBRUSH dManageStdLang::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  long id = GetWindowLong(hWndChild, GWL_ID);
  if(id >= IDC_RADIO_LANG_1 && id <= IDC_RADIO_LANG_10 || -1 == id)
    return 0;
  if(WM_CTLCOLORSTATIC == ctlType) {
/*
    uint groupBox[] = {
      IDC_GROUPBOX_A,
      IDC_GROUPBOX_RA,
      };
    for(int i = 0; i < SIZE_A(groupBox); ++i)
      if(GetDlgItem(*this, groupBox[i]) == hWndChild) {
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
*/
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
void dManageStdLang::fillDataCtrl()
{
  LPCTSTR p = Set.getString(ID_LANGUAGE);
  if(!p) {
    SET_TEXT(IDC_EDIT_DESCR_1, _T("Italiano"));
    SET_CHECK(IDC_RADIO_LANG_1);
    }
  else {
    int active = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p) {
      SET_TEXT(IDC_EDIT_DESCR_1, _T("Italiano"));
//      SET_CHECK(IDC_RADIO_LANG_1);
      }
    else {
      pvvChar target;
      uint nElem = splitParam(target, p);
      for(uint i = 0; i < MAX_LANG && i < nElem; ++i)
        SET_TEXT(IDC_EDIT_DESCR_1 + i, &target[i]);
      if(active) {
        SET_CHECK(IDC_RADIO_LANG_1 + active - 1);
        SET_CHECK(IDC_CHECK_ENABLE_LANG);
        }
      }
    }
  checkEnable();
}
//----------------------------------------------------------------------------
void dManageStdLang::checkEnable()
{
  bool checked = IS_CHECKED(IDC_CHECK_ENABLE_LANG);
  for(uint i = 0; i < MAX_LANG; ++i) {
    EnableWindow(GetDlgItem(*this, IDC_EDIT_DESCR_1 + i), checked);
    EnableWindow(GetDlgItem(*this, IDC_RADIO_LANG_1 + i), checked);
    }
}
//-------------------------------------------------------------------
bool dManageStdLang::save(P_File& pfCrypt, P_File& pfClear)
{
  PVect<LPCTSTR> pvRow;
  int active = 0;
//  int ix = 0;
  for(uint i = 0; i < MAX_LANG; ++i) {
    TCHAR buff[500];
    GET_TEXT(IDC_EDIT_DESCR_1 + i, buff);
    lTrim(trim(buff));
//    if(*buff) {
      pvRow[i] = str_newdup(buff);
      if(IS_CHECKED(IDC_RADIO_LANG_1 + i))
        active = i + 1;
//      ++ix;
//      }
    }
  if(!IS_CHECKED(IDC_CHECK_ENABLE_LANG))
    active = 0;
  setCurrLang(active);

  bool success = false;
  do {
    TCHAR t[50];
    wsprintf(t, _T("%d,%d"), ID_LANGUAGE, active);
    if(!writeStringChkUnicode(pfCrypt,t))
      break;
    for(int i = 0; i < MAX_LANG; ++i) {
      if(!writeStringChkUnicode(pfCrypt,_T(",")))
        break;

      if(*pvRow[i] && !writeStringChkUnicode(pfCrypt,pvRow[i]))
        break;
      }
    if(!writeStringChkUnicode(pfCrypt,_T("\r\n")))
      break;
    success = true;
    } while(false);
  flushPAV(pvRow);
  return success;
}
//-------------------------------------------------------------------
