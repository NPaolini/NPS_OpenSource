//--------- various_1.cpp --------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "infoText.h"
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
//-------------------------------------------------------------------
#include "svmProperty.h"
#include "POwnBtnImageStd.h"
//-------------------------------------------------------------------
static infoLabelValue iLabelEdit[] = {
  { 2, _T("Network Path"), IDC_STATICTEXT_NETWORK, IDC_EDIT_STD_NETWORK },

  { 4, _T("Timer Applicazione"), IDC_STATICTEXT_APPL_TIMER, IDC_EDIT_APPL_TIMER },
  { -4, _T("Refresh lamp. (ms)"), IDC_STATICTEXT_APPL_TIMER2, IDC_EDIT_APPL_TIMER2 },
  { 9, _T("Timer reset Edit (def. 30 sec)"), IDC_STATICTEXT_EDIT_RESET, IDC_EDIT_TIME_RESET_EDIT },
//  { 14, _T("Nome file header"), IDC_STATICTEXT_HEADER_NAME, IDC_EDIT_HEADER_NAME },
  { 23, _T("Scelta monitor"), IDC_STATICTEXT_MONITOR, IDC_EDIT_MONITOR },
  { 30, _T("Ritardo avvio driver (msec)"), IDC_STATICTEXT_DELAY_DRIVER, IDC_EDIT_DELAY_DRIVER },
  { 31, _T("Ind. Data-Ora corrente (prf1)"), IDC_STATICTEXT_CURR_DATETIME, IDC_EDIT_CURR_DATETIME },
  { 61, _T("Ritardo avvio da autorun (sec)"), IDC_STATICTEXT_DELAY_AUTORUN, IDC_EDIT_DELAY_AUTORUN },
  { 99, _T("Carattere per password"), IDC_STATICTEXT_PSW_CHAR, IDC_EDIT_PSW_CHAR },
  { 130, _T("Altezza font Allarmi/Manutenzione"), IDC_STATICTEXT_FONT_HEIGHT, IDC_EDIT_FONT_HEIGHT },
  { 131, _T("Nome del Font"), IDC_STATICTEXT_FONT_NAME, IDC_EDIT_FONT_NAME },
  { 132, _T("Larghezza fissa carattere"), IDC_STATICTEXT_FONT_WIDTH, IDC_EDIT_FONT_WIDTH },
  { 155, _T("Millisecondi attesa per send/receive"), IDC_STATICTEXT_TIMEOUT_SEND_REC, IDC_EDIT_TIMEOUT_SEND_REC },
};
//-------------------------------------------------------------------
static infoLabelValue iLabelCombo[] = {
  // combobox
  { 5, _T("Separatore per Export"), IDC_STATICTEXT_EXP_SEP, IDC_COMBOBOX_EXP_SEP },
  { 6, _T("Separatore per decimali"), IDC_STATICTEXT_DEC_SEP, IDC_COMBOBOX_DEC_SEP },
  { 7, _T("Parti da nascondere"), IDC_STATICTEXT_HIDE_PART, IDC_COMBOBOX_HIDE_PART },
  { 23, _T("Risoluzione Base"), IDC_STATICTEXT_RESOLUTION, IDC_COMBOBOX_RESOLUTION },
//  { 23, _T("Scelta monitor"), IDC_STATICTEXT_MONITOR, IDC_COMBOBOX_MONITOR },
  { 81, _T("Formato Data"), IDC_STATICTEXT_DATE_FORM, IDC_COMBOBOX_DATE_TYPE },
};
//-------------------------------------------------------------------
static infoLabelValue iLabelCheck[] = {
  // checkbox
  { 3, _T("Pagine in locale"), 0, IDC_CHECKBOX_LOCAL_PAGE },
  { 8, _T("Nascondere anche il Time"), 0, IDC_CHECKBOX_HIDE_TIME },
  { 13, _T("In sola lettura (insieme al cod.2)"), 0, IDC_CHECKBOX_READ_ONLY },
  { 22, _T("Comportamento predefinito F1"), 0, IDC_CHECKBOX_PREDEF_F1 },
  { 22, _T("Comportamento predefinito F12"), 0, IDC_CHECKBOX_PREDEF_F12 },
  { 38, _T("Visualizza allarmi non più attivi"), 0, IDC_CHECKBOX_SHOW_ALARM_INACTIVE },
  { 39, _T("Nasconde Tipi Allarme"), 0, IDC_CHECKBOX_HIDE_ALARM_TYPE },
  { 45, _T("Tastiera estesa"), 0, IDC_CHECKBOX_EXT_KEYB },
  { 112, _T("Non memorizza percorso pagine"), 0, IDC_CHECKBOX_NOQUEUE_PAGE },

  { 46, _T("Usa testo da tasti funzione per pulsanti"), 0, IDC_CHECK_BTN_FZ_TEXT },
  { 46, _T("Aggiungi F? - "), 0, IDC_CHECK_BTN_FZ_TEXT_PRFX },
  { 46, _T("Disabilita se senza testo"), 0, IDC_CHECK_BTN_FZ_DISABLE },
  { 37, _T("Nascondi barra del Titolo"), 0, IDC_CHECK_BTN_HIDE_TITLE },
};
//-------------------------------------------------------------------
static infoLabelValue iLabelCustom[] = {
  { 39, _T("Modalità refresh allarmi"), IDC_STATICTEXT_REFRESH_ALARM, IDC_COMBOBOX_REFRESH_ALARM },
  { 39, _T("Allarme più recente in testa"), 0, IDC_CHECKBOX_ORD_ALARM },
  { 39, _T("Visualizza Data Allarme"), 0, IDC_CHECKBOX_DATA_ALARM },
  { 39, _T("Nascondi Prph Allarme"), 0, IDC_CHECKBOX_HIDE_ALARM_PRPH },
  { 39, _T("Nascondi Prph Report"), 0, IDC_CHECKBOX_HIDE_REPORT_PRPH },
};
//-------------------------------------------------------------------
//static int numberId[] = {

//-------------------------------------------------------------------
/*
#define CAST_TO_I_T(a) (*(infoText*)&(a))
//-------------------------------------------------------------------
void getDataCtrl(setOfPInfoText& target, const setOfString& set)
{
  flushPAV(target);
  target.setDim(SIZE_A(iLabelEdit) + SIZE_A(iLabelCombo) + SIZE_A(iLabelCheck));
  int i = 0;
  for(int j = 0; j < SIZE_A(iLabelEdit); ++i, ++j)
    target[i] = new infoText(CAST_TO_I_T(iLabelEdit[j]), set);
  for(; i < SIZE_A(iLabelCombo); ++i)
    target[i] = new infoText(CAST_TO_I_T(iLabelCombo[j]), set);
  for(; i < SIZE_A(iLabelCheck); ++i)
    target[i] = new infoText(CAST_TO_I_T(iLabelCheck[j]), set);
}
*/
//----------------------------------------------------------------------------
//#define IS_CHECKED(id) (BST_CHECKED == SendMessage(GetDlgItem(*this, (id)), BM_GETCHECK, 0, 0))
//----------------------------------------------------------------------------
//#define MAX_TEXT 200
//#define MAX_LABEL 80
//----------------------------------------------------------------------------
//-------------------------------------------------------------------
class dManageStdVars1 : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdVars1(const setOfString& set, PWin* parent, HINSTANCE hInst = 0) :
        baseClass(set, parent, IDD_STD_MSG_FLAG1, hInst), oldRes(sizer::sOutOfRange)
        { oldSz.cx = oldSz.cy = 0; }
    ~dManageStdVars1();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
  protected:
//    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:

    void fillCombo();
    void fillDataCtrl(const setOfString& set);
    void findNetwork();
    void chooseMonitor();

    SIZE oldSz;
    int oldRes;
};
//-------------------------------------------------------------------
basePage* allocStdVars1( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdVars1(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdVars1::~dManageStdVars1()
{
  destroy();
}
//-------------------------------------------------------------------
bool dManageStdVars1::create()
{
  if(!baseClass::create())
    return false;

  fillDataCtrl(Set);

  return true;
}
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT dManageStdVars1::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_FIND_NETWORK:
          findNetwork();
          break;
        case IDC_CHECK_BTN_FZ_TEXT_PRFX:
          if(IS_CHECKED(IDC_CHECK_BTN_FZ_TEXT_PRFX))
            SET_CHECK(IDC_CHECK_BTN_FZ_TEXT);
          break;
        case IDC_CHECK_BTN_FZ_DISABLE:
          if(IS_CHECKED(IDC_CHECK_BTN_FZ_DISABLE))
            SET_CHECK(IDC_CHECK_BTN_FZ_TEXT);
          break;
        case IDC_BUTTON_CHOOSE_MONITOR:
          chooseMonitor();
          break;
        }
/*
      switch(HIWORD(wParam)) {
        case EN_KILLFOCUS:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_MAX_ROW:
              if(SendMessage((HWND)lParam, EM_GETMODIFY, 0, 0))
                changeNumRow();
              break;
            }
          break;
        case EN_CHANGE:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_MAX_ROW:
              break;
            default:
              dirty = true;
            }
          break;
        }
*/
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
void dManageStdVars1::findNetwork()
{
  TCHAR path[_MAX_PATH];
  GetDlgItemText(*this, IDC_EDIT_STD_NETWORK, path, SIZE_A(path));
  TCHAR t[_MAX_PATH];
  if(PChooseFolder(translateToCRNL(t, path), _T("NetWork Path"), false, 0))
    SetDlgItemText(*this, IDC_EDIT_STD_NETWORK, translateFromCRNL(path, t));
}
//------------------------------------------------------------------------------
/*
void dManageStdVars1::loadData()
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
HBRUSH dManageStdVars1::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  int id = ::GetDlgCtrlID(hWndChild);
  if(WM_CTLCOLORSTATIC == ctlType) {
    TCHAR buff[100] = _T("");
    GetWindowText(hWndChild, buff, SIZE_A(buff));
    if(_T('1') <= *buff && *buff <= _T('9')) {
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }

    for(uint i = 0; i < SIZE_A(iLabelEdit); ++i) {
      if(id == iLabelEdit[i].idCtrlLabel) {
        SetBkColor(hdc, bkgColor3);
        return (Brush3);
        }
      }
    for(uint i = 0; i < SIZE_A(iLabelCombo); ++i) {
      if(id == iLabelCombo[i].idCtrlLabel) {
        SetBkColor(hdc, bkgColor3);
        return (Brush3);
        }
      }
    for(uint i = 0; i < SIZE_A(iLabelCheck); ++i) {
      if(id == iLabelCheck[i].idCtrlLabel) {
        SetBkColor(hdc, bkgColor3);
        return (Brush3);
        }
      }
    for(uint i = 0; i < SIZE_A(iLabelCustom); ++i) {
      if(id == iLabelCustom[i].idCtrlLabel) {
        SetBkColor(hdc, bkgColor3);
        return (Brush3);
        }
      }
    return 0;
    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, bkgColor);
    return (Brush);
    }
  return 0;
}
//----------------------------------------------------------------------------
//-------------------------------------------------------------------
void dManageStdVars1::fillCombo()
{
  uint ixC = 0;
  LPCTSTR expSep[] = { _T("\\t"), _T(";"), _T("|") };
  HWND ctrl = GetDlgItem(*this, iLabelCombo[ixC].idCtrlSet);
  SendMessage(ctrl, CB_ADDSTRING, 0, (LPARAM)_T(""));
  for(uint i = 0; i < SIZE_A(expSep); ++i)
    addStringToComboBox(ctrl, expSep[i]);

  ++ixC;
  LPCTSTR expDec[] = { _T("."), _T(",") };
  ctrl = GetDlgItem(*this, iLabelCombo[ixC].idCtrlSet);
  SendMessage(ctrl, CB_ADDSTRING, 0, (LPARAM)_T(""));
  for(uint i = 0; i < SIZE_A(expDec); ++i)
    addStringToComboBox(ctrl, expDec[i]);

  ++ixC;
  LPCTSTR hidePart[] = { _T("Show All"), _T("Hide Header"), _T("Hide Footer"), _T("Hide Both") };
  ctrl = GetDlgItem(*this, iLabelCombo[ixC].idCtrlSet);
  for(uint i = 0; i < SIZE_A(hidePart); ++i)
    addStringToComboBox(ctrl, hidePart[i]);

  ++ixC;
#define IX_PERS_RES 9
  LPCTSTR resForm[] = { _T("ignora"), _T("640x480"), _T("800x600"), _T("1024x768"),
      _T("1280x1024"), _T("1440x900 (16:10)"), _T("1600x1200"), _T("1680x1050 (16:10)"),
      _T("1920x1440"), _T("personalizzate") };
  ctrl = GetDlgItem(*this, iLabelCombo[ixC].idCtrlSet);
  for(uint i = 0; i < SIZE_A(resForm); ++i)
    addStringToComboBox(ctrl, resForm[i]);
/*
  ++ixC;
  LPCTSTR monitorForm[] = { _T("Singolo o primario"), _T("Secondario - max res."), _T("Secondario - sVisor res."), };
  ctrl = GetDlgItem(*this, iLabelCombo[4].idCtrlSet);
  for(int i = 0; i < SIZE_A(monitorForm); ++i)
    addStringToComboBox(ctrl, monitorForm[i]);
*/
  ++ixC;
  LPCTSTR dataForm[] = { /*_T("default"),*/ _T("dd/mm/yyyy"), _T("mm/dd/yyyy"), _T("yyyy/mm/dd") };
  ctrl = GetDlgItem(*this, iLabelCombo[ixC].idCtrlSet);
  for(uint i = 0; i < SIZE_A(dataForm); ++i)
    addStringToComboBox(ctrl, dataForm[i]);

  LPCTSTR refreshAlarm[] = { _T("disabilitato"), _T("a richiesta"), _T("automatico") };
  ctrl = GetDlgItem(*this, iLabelCustom[0].idCtrlSet);
  for(uint i = 0; i < SIZE_A(dataForm); ++i)
    addStringToComboBox(ctrl, refreshAlarm[i]);
}
//-------------------------------------------------------------------
bool verifyCtrl(LPCTSTR p, uint ctrl)
{
  if(IDC_CHECKBOX_PREDEF_F1 == ctrl)
    return toBool(_ttoi(p) & 1);
  if(IDC_CHECKBOX_PREDEF_F12 == ctrl)
    return toBool(_ttoi(p) & 2);

  if(IDC_CHECK_BTN_FZ_TEXT == ctrl)
    return toBool(_ttoi(p) & 1);
  if(IDC_CHECK_BTN_FZ_TEXT_PRFX == ctrl)
    return toBool(_ttoi(p) & 2);
  if(IDC_CHECK_BTN_FZ_DISABLE == ctrl)
    return toBool(_ttoi(p) & 4);
  return true;
}
//-------------------------------------------------------------------
void dManageStdVars1::fillDataCtrl(const setOfString& set)
{
  SetWindowText(GetDlgItem(*this, IDC_EDIT_MONITOR), _T("10,0"));
  for(uint i = 0; i < SIZE_A(iLabelEdit); ++i) {
    HWND ctrl = GetDlgItem(*this, iLabelEdit[i].idCtrlLabel);
    SetWindowText(ctrl, iLabelEdit[i].label);
    int ix = iLabelEdit[i].id;
    if(ix < 0)
      ix = -ix;
    LPCTSTR p = set.getString(ix);
    if(p) {
      ctrl = GetDlgItem(*this, iLabelEdit[i].idCtrlSet);
      if(2 == iLabelEdit[i].id) {
        TCHAR buff[_MAX_PATH + 20];
        translateFromCRNL(buff, p);
        SetWindowText(ctrl, buff);
        }
      else if(4 == iLabelEdit[i].id) {
        int t = _ttoi(p);
        TCHAR s[16];
        wsprintf(s, _T("%d"), t);
        SetWindowText(ctrl, s);
        }
      else if(-4 == iLabelEdit[i].id) {
        p = findNextParamTrim(p);
        SetWindowText(ctrl, p);
        }
      else if(23 == iLabelEdit[i].id) {
        p = findNextParamTrim(p);
        pvvChar target;
        uint n = splitParam(target, p);
        if(n >= 2) {
          TCHAR t[64];
          wsprintf(t, _T("%s,%s"), &target[0], &target[1]);
          SetWindowText(ctrl, t);
          if(n >= 4) {
            SetWindowText(GetDlgItem(*this, IDC_EDIT_VAR1_RES_PERS_W), &target[2]);
            SetWindowText(GetDlgItem(*this, IDC_EDIT_VAR1_RES_PERS_H), &target[3]);
            oldSz.cx = _ttoi(&target[2]);
            oldSz.cy = _ttoi(&target[3]);
            }
          }
        }
      else
        SetWindowText(ctrl, p);
      }
    }

  fillCombo();
  for(uint i = 0; i < SIZE_A(iLabelCombo); ++i) {
    HWND ctrl = GetDlgItem(*this, iLabelCombo[i].idCtrlLabel);
    SetWindowText(ctrl, iLabelCombo[i].label);
    LPCTSTR p = set.getString(iLabelCombo[i].id);
    ctrl = GetDlgItem(*this, iLabelCombo[i].idCtrlSet);
    if(p) {
      if(i < 2) {
        TCHAR buff[100];
        translateFromCRNL(buff, p);
        SetWindowText(ctrl, buff);
        }
      else {
        int ix = _ttoi(p);
        if(81 == iLabelCombo[i].id)
          --ix;
        else if(23 == iLabelCombo[i].id) {
          oldRes = ix;
          if(100 == ix)
            ix = IX_PERS_RES;
//          else
//            ++ix;
          }
        SendMessage(ctrl, CB_SETCURSEL, ix, 0);
        }
      }
    else
      SendMessage(ctrl, CB_SETCURSEL, 0, 0);
    }

  for(uint i = 0; i < SIZE_A(iLabelCheck); ++i) {
    HWND ctrl = GetDlgItem(*this, iLabelCheck[i].idCtrlLabel);
    SetWindowText(ctrl, iLabelCheck[i].label);
    LPCTSTR p = set.getString(iLabelCheck[i].id);
    if(p && _ttoi(p)) {
      if(verifyCtrl(p, iLabelCheck[i].idCtrlSet)) {
        ctrl = GetDlgItem(*this, iLabelCheck[i].idCtrlSet);
        SendMessage(ctrl, BM_SETCHECK, BST_CHECKED, 0);
        }
      }
    }

  do  {
    HWND ctrl = GetDlgItem(*this, iLabelCustom[0].idCtrlLabel);
    SetWindowText(ctrl, iLabelCustom[0].label);
    LPCTSTR p = set.getString(iLabelCustom[0].id);
    ctrl = GetDlgItem(*this, iLabelCustom[0].idCtrlSet);
    p = findNextParam(p, 2);
    if(p) {
      int ix = _ttoi(p) + 1;
      SendMessage(ctrl, CB_SETCURSEL, ix, 0);
      }
    else
      SendMessage(ctrl, CB_SETCURSEL, 2, 0);
    } while(false);

  do  {
    HWND ctrl = GetDlgItem(*this, iLabelCustom[1].idCtrlLabel);
    SetWindowText(ctrl, iLabelCustom[1].label);
    LPCTSTR p = set.getString(iLabelCustom[1].id);
    p = findNextParam(p, 1);
    if(!p || _ttoi(p)) {
      if(verifyCtrl(p, iLabelCustom[1].idCtrlSet)) {
        ctrl = GetDlgItem(*this, iLabelCustom[1].idCtrlSet);
        SendMessage(ctrl, BM_SETCHECK, BST_CHECKED, 0);
        }
      }
    } while(false);
  do  {
    HWND ctrl = GetDlgItem(*this, iLabelCustom[2].idCtrlLabel);
    SetWindowText(ctrl, iLabelCustom[2].label);
    LPCTSTR p = set.getString(iLabelCustom[2].id);
    p = findNextParam(p, 3);
    if(p && _ttoi(p)) {
      ctrl = GetDlgItem(*this, iLabelCustom[2].idCtrlSet);
      SendMessage(ctrl, BM_SETCHECK, BST_CHECKED, 0);
      }
    } while(false);
  do  {
    HWND ctrl = GetDlgItem(*this, iLabelCustom[3].idCtrlLabel);
    SetWindowText(ctrl, iLabelCustom[3].label);
    LPCTSTR p = set.getString(iLabelCustom[3].id);
    p = findNextParam(p, 4);
    if(p && _ttoi(p)) {
      ctrl = GetDlgItem(*this, iLabelCustom[3].idCtrlSet);
      SendMessage(ctrl, BM_SETCHECK, BST_CHECKED, 0);
      }
    } while(false);
  do  {
    HWND ctrl = GetDlgItem(*this, iLabelCustom[4].idCtrlLabel);
    SetWindowText(ctrl, iLabelCustom[4].label);
    LPCTSTR p = set.getString(iLabelCustom[4].id);
    p = findNextParam(p, 5);
    if(p && _ttoi(p)) {
      ctrl = GetDlgItem(*this, iLabelCustom[4].idCtrlSet);
      SendMessage(ctrl, BM_SETCHECK, BST_CHECKED, 0);
      }
    } while(false);

//IDC_CHECKBOX_DATA_ALARM
}
//-------------------------------------------------------------------
extern void setUseExtKeyb(bool use);
//-------------------------------------------------------------------
bool dManageStdVars1::save(P_File& pfCrypt, P_File& pfClear)
{
  TCHAR buff[500];
  TCHAR monit[64] = _T("");
  bool changedRes = false;
  SIZE sz;
  int currRes = sizer::getCurr(sz);

  for(uint i = 0; i < SIZE_A(iLabelEdit); ++i) {
    HWND ctrl = GetDlgItem(*this, iLabelEdit[i].idCtrlSet);
    buff[0] = 0;
    GetWindowText(ctrl, buff, SIZE_A(buff));
    if(4 == iLabelEdit[i].id) {
      TCHAR t[20];
      wsprintf(t, _T("%d,"), iLabelEdit[i].id);
      if(!writeStringChkUnicode(pfCrypt, t))
        return false;
      if(!_ttoi(buff))
        _tcscpy_s(buff, _T("300"));
      if(!writeStringChkUnicode(pfCrypt, buff))
        return false;
      }
    else if(-4 == iLabelEdit[i].id) {
      if(*buff) {
        if(!writeStringChkUnicode(pfCrypt, _T(",")))
          return false;
        if(!writeStringChkUnicode(pfCrypt, buff))
          return false;
        }
      if(!writeStringChkUnicode(pfCrypt, _T("\r\n")))
        return false;
      }

    else if(*buff) {
      if(23 == iLabelEdit[i].id) {
        _tcscpy_s(monit, buff);
        TCHAR t1[20];
        GetWindowText(GetDlgItem(*this, IDC_EDIT_VAR1_RES_PERS_W), t1, SIZE_A(t1));
        TCHAR t2[20];
        GetWindowText(GetDlgItem(*this, IDC_EDIT_VAR1_RES_PERS_H), t2, SIZE_A(t2));
        uint v = _ttoi(t1);
        if(v != oldSz.cx || v != sz.cx) {
          changedRes = true;
          oldSz.cx = v;
          }
        v = _ttoi(t2);
        if(v != oldSz.cy || v != sz.cy) {
          changedRes = true;
          oldSz.cy = v;
          }
        TCHAR t[40];
        wsprintf(t, _T(",%s,%s"), t1, t2);
        _tcscat_s(monit, t);
        }
      else {
        TCHAR t[20];
        wsprintf(t, _T("%d,"), iLabelEdit[i].id);
        if(!writeStringChkUnicode(pfCrypt, t))
          return false;
        if(!writeStringChkUnicode(pfCrypt, buff))
          return false;
        if(!writeStringChkUnicode(pfCrypt, _T("\r\n")))
          return false;
        }
      }
    }

//  int res = 0;
  for(uint i = 0; i < SIZE_A(iLabelCombo); ++i) {
    HWND ctrl = GetDlgItem(*this, iLabelCombo[i].idCtrlSet);
    if(i < 2) {
      GetWindowText(ctrl, buff, SIZE_A(buff));
      if(*buff) {
        TCHAR t[20];
        wsprintf(t, _T("%d,"), iLabelCombo[i].id);
        if(!writeStringChkUnicode(pfCrypt, t))
          return false;
        if(!writeStringChkUnicode(pfCrypt, buff))
          return false;
        if(!writeStringChkUnicode(pfCrypt, _T("\r\n")))
          return false;
        }
      }
    else {
      int ix = SendMessage(ctrl, CB_GETCURSEL, 0, 0);
      if(81 == iLabelCombo[i].id)// || 23 == iLabelCombo[i].id)
        ++ix;
      else if(23 == iLabelCombo[i].id) {
        if(IX_PERS_RES == ix)
          ix = 100;
//        else
//          --ix;
        wsprintf(buff, _T("%d,%d,%s\r\n"), iLabelCombo[i].id, ix, monit);
        if(!writeStringChkUnicode(pfCrypt, buff))
          return false;
//        if(ix != oldRes || ix != currRes || 100 == ix && changedRes) {
        if(ix > 0 && (ix - 1 != currRes || 100 == ix && changedRes)) {
          if(IDYES == MessageBox(*this, _T("Aggiornare la risoluzione?"), _T("Attenzione"), MB_YESNO | MB_ICONINFORMATION)) {
            sizer::eSizer s = 100 == ix ? sizer::sPersonalized : (sizer::eSizer)(ix - 1);
            sizer::setDim(s, &oldSz, true);
            dManageStdMsg* par = getParentWin<dManageStdMsg>(getParent());
            if(par)
              par->changedResolution();
            }
          }
/*
        if(IDC_COMBOBOX_MONITOR == iLabelCombo[i].idCtrlSet) {
          wsprintf(buff, _T("%d,%d,%d\r\n"), iLabelCombo[i].id, res, ix);
          if(!writeStringChkUnicode(pfCrypt, buff))
            return false;
          }
        else {
          res = ix;
          }
*/
        continue;
        }
      if(ix > 0) {
        wsprintf(buff, _T("%d,%d\r\n"), iLabelCombo[i].id, ix);
        if(!writeStringChkUnicode(pfCrypt, buff))
          return false;
        }
      }
    }
  uint predefBtn = 0;
  uint useTextFz = 0;
  uint codeTextFz = 0;
  uint code = 22;
  int cod39 = 0;
  for(uint i = 0; i < SIZE_A(iLabelCheck); ++i) {
    if(IS_CHECKED(iLabelCheck[i].idCtrlSet)) {
      if(IDC_CHECKBOX_PREDEF_F1 == iLabelCheck[i].idCtrlSet) {
        predefBtn |= 1;
        code = iLabelCheck[i].id;
        continue;
        }
      if(IDC_CHECKBOX_PREDEF_F12 == iLabelCheck[i].idCtrlSet) {
        predefBtn |= 2;
        code = iLabelCheck[i].id;
        continue;
        }
      if(IDC_CHECK_BTN_FZ_TEXT == iLabelCheck[i].idCtrlSet) {
        useTextFz |= 1;
        codeTextFz = iLabelCheck[i].id;
        continue;
        }
      if(IDC_CHECK_BTN_FZ_TEXT_PRFX == iLabelCheck[i].idCtrlSet) {
        useTextFz |= 2;
        continue;
        }
      if(IDC_CHECK_BTN_FZ_DISABLE == iLabelCheck[i].idCtrlSet) {
        useTextFz |= 4;
        continue;
        }

      if(39 == iLabelCheck[i].id) {
        cod39 = 1;
        continue;
        }
      wsprintf(buff, _T("%d,1\r\n"), iLabelCheck[i].id);
      if(!writeStringChkUnicode(pfCrypt, buff))
        return false;
      }
    }

  if(useTextFz & 1) {
    wsprintf(buff, _T("%d,%d\r\n"), codeTextFz, useTextFz);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  setUseExtKeyb(IS_CHECKED(IDC_CHECKBOX_EXT_KEYB));

  if(code) {
    wsprintf(buff, _T("%d,%d\r\n"), code, predefBtn);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }

  do {
    HWND ctrl = GetDlgItem(*this, iLabelCustom[0].idCtrlSet);
    int refresh = SendMessage(ctrl, CB_GETCURSEL, 0, 0) - 1;

    int ord = IS_CHECKED(iLabelCustom[1].idCtrlSet);

    int date = IS_CHECKED(iLabelCustom[2].idCtrlSet);
    int hidePrphAlarm = IS_CHECKED(iLabelCustom[3].idCtrlSet);
    int hidePrphReport = IS_CHECKED(iLabelCustom[4].idCtrlSet);
     wsprintf(buff, _T("39,%d,%d,%d,%d,%d,%d\r\n"), cod39, ord, refresh, date, hidePrphAlarm, hidePrphReport);
     if(!writeStringChkUnicode(pfCrypt, buff))
       return false;
    } while(false);

  return true;
}
//-------------------------------------------------------------------
class D_choose_Monitor : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    D_choose_Monitor(PWin* parent, LPTSTR choose) : baseClass(parent, IDD_STD_MSG_CHOOSE_MONITOR),
      Choose(choose) {}
    ~D_choose_Monitor() { destroy(); }

    virtual bool create();
    virtual void CmOk();
  private:
    LPTSTR Choose;
};
//-------------------------------------------------------------------
bool D_choose_Monitor::create()
{
  int idBmp[] = { IDB_BITMAP_OK, IDB_BITMAP_CANC };
  int idBtn[] = { IDOK, IDCANCEL };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true);
    }
  if(!baseClass::create())
    return false;
  int ixMon = _ttoi(Choose);
  bool offSVisor = false;
  LPCTSTR p = findNextParamTrim(Choose);
  if(p)
    offSVisor = toBool(_ttoi(p));
  SET_CHECK_SET(IDC_CHECK_OFFSET, offSVisor);
  switch(ixMon) {
    case 0:
    case 10:
    default:
      SET_CHECK(IDC_RADIO1);
      break;
    case 20:
      SET_CHECK(IDC_RADIO2);
      break;
    case 30:
      SET_CHECK(IDC_RADIO3_1);
      break;
    case 40:
      SET_CHECK(IDC_RADIO4_1);
      break;
    case 130:
      SET_CHECK(IDC_RADIO3_2);
      break;
    case 140:
      SET_CHECK(IDC_RADIO4_2);
      break;
    }
  return true;
}
//-------------------------------------------------------------------
void D_choose_Monitor::CmOk()
{
  int ixMon = 10;
  if(IS_CHECKED(IDC_RADIO2))
    ixMon = 20;
  else if(IS_CHECKED(IDC_RADIO3_1))
    ixMon = 30;
  else if(IS_CHECKED(IDC_RADIO4_1))
    ixMon = 40;
  else if(IS_CHECKED(IDC_RADIO3_2))
    ixMon = 130;
  else if(IS_CHECKED(IDC_RADIO4_2))
    ixMon = 140;
  wsprintf(Choose, _T("%d,%d"), ixMon, IS_CHECKED(IDC_CHECK_OFFSET));
  baseClass::CmOk();
}
//-------------------------------------------------------------------
void dManageStdVars1::chooseMonitor()
{
  TCHAR buff[500];
  GET_TEXT(IDC_EDIT_MONITOR, buff);
  if(IDOK == D_choose_Monitor(this, buff).modal())
    SET_TEXT(IDC_EDIT_MONITOR, buff);
}
