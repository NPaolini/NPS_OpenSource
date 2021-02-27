//------- svmPageProperty.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "svmPageProperty.h"
#include "macro_utils.h"
#include "pOpensave.h"
#include "svmDefObj.h"
#include "svmProperty.h"
#include "svmPagePropAction.h"
#include "dChoosePath.h"
#include "1.h"
#include "id_btn.h"
#include "svmSendVarByPage.h"
//----------------------------------------------------------------------------
#define BKG RGB(0xff, 0xff, 0xef)
//----------------------------------------------------------------------------
void addOneToRecipe(LPTSTR buff);
void remOneToRecipe(LPTSTR buff);
void fillCBTypeShow(HWND hwnd, int sel);
void fillCB_BtnFzBeha(HWND hwnd, int sel);
//----------------------------------------------------------------------------
svmPageProperty::svmPageProperty(PWin* parent, setOfString& set, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Set(set), bkgEdit(CreateSolidBrush(BKG))
{
  for(int i = 0; i < MAX_BTN; ++i)
    new langEdit(this, IDC_EDIT_TITLE_F1 + i);

  new langEdit(this, IDC_EDIT_PAGE_TITLE);

  for(uint i = 0; i < SIZE_A(iPage); ++i)
    iPage[i] = 0;

}
//----------------------------------------------------------------------------
svmPageProperty::~svmPageProperty()
{
  destroy();
  DeleteObject(bkgEdit);
  for(uint i = 0; i < SIZE_A(iPage); ++i)
    delete iPage[i];
}
//----------------------------------------------------------------------------
extern bool getUseExtKeyb();
//----------------------------------------------------------------------------
bool svmPageProperty::create()
{
  if(!baseClass::create())
    return false;
  LPCTSTR p = Set.getString(ID_INIT_BMP);
  if(p) {
    int x;
    int y;
    int scaleX;
    int scaleY;
    int flag;
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &x, &y, &scaleX, &scaleY, &flag);
    p = findNextParam(p, 5);
    p = getOnlyName(p);
    SET_TEXT(IDC_EDIT_BKG_FILENAME, p);

    SET_INT(IDC_EDIT_SCALE_X, scaleX);
    SET_INT(IDC_EDIT_SCALE_Y, scaleY);
    if(flag >= 1000) {
      flag -= 1000;
      SET_CHECK(IDC_CHECKBOX_TRANSPARENT);
      }
    int ctrl;
    switch(flag) {
      case bCENTERED:
        ctrl = IDC_RADIOBUTTON_CENTERED;
        break;
      case bSCALED:
        ctrl = IDC_RADIOBUTTON_SCALED;
        break;
      case bFILLED:
        ctrl = IDC_RADIOBUTTON_FILLED;
        break;
      case bTILED:
      default:
        ctrl = IDC_RADIOBUTTON_TILED;
        break;
      }
    SET_CHECK(ctrl);
    }
  else
    SET_CHECK(IDC_RADIOBUTTON_FILLED);

  checkEnable();

  fillAction();

/*
 local hide/show header/footer
 i valori sono:
 00.01 -  1 -> nasconde header
 00.11 -  3 -> visualizza header
 01.00 -  4 -> nasconde footer
 11.00 - 12 -> visualizza footer

 01.01 -  5 -> nasconde entrambi
 01.11 -  7 -> visualizza header + nasconde footer
 11.01 - 13 -> visualizza footer + nasconde header
 11.11 - 15 -> visualizza entrambi

la prima pagina non deve avere il codice, deve prendere quello di default
dallo std_msg

#define ID_LOCAL_SHOW 98
*/

  do {
    HWND hCB = GetDlgItem(*this, IDC_COMBOBOX_SHOWTYPE);
    p = Set.getString(ID_CODE_PAGE_TYPE);
    p = findNextParam(p, 1);

    int sel = 0;
    if(p) {
      int v = _ttoi(p);
      if(v & 1)
        sel = 1;
      if(v & 2)
        sel |= 2;
      v = sel;
      sel = 0;
      const int vals[] = {  3, 2, 1, 0 };
      for(uint i = 0; i < SIZE_A(vals); ++i) {
        if(vals[i] == v) {
          sel = i + 1;
          break;
          }
        }
      }
    fillCBTypeShow(hCB, sel);
    } while(false);
  do {
    HWND hCB = GetDlgItem(*this, IDC_COMBOBOX_BTN_FZ_BEHA);
    p = Set.getString(ID_CODE_BTN_FZ_BEHA);

    int sel = 0;
    if(p)
      sel = _ttoi(p);
    fillCB_BtnFzBeha(hCB, sel);
    } while(false);



  for(int i = 0; i < MAX_BTN; ++i) {
    p = Set.getString(ID_INIT_MENU + i);
    oldSel[i] = 0;
    if(p) {
      TCHAR t[1000];
      translateFromCRNL(t, p);
      SET_TEXT(IDC_EDIT_TITLE_F1 + i, t);
      p = Set.getString(ID_INIT_ACTION_MENU + i);
      if(p)
        setAction(p, i);
      else {
        wsprintf(t, _T(" ")/*, aCustomAction*/);
        setAction(t, i);
        }
      }
    }
  p = Set.getString(ID_DEF_SEND_EDIT);
  if(!p || _ttoi(p))
    SET_CHECK(IDC_CHECKBOX_AUTO_UPDATE_EDIT);

//  p = Set.getString(ID_SHOW_SIMPLE_SEND);
//  if(!p || !_ttoi(p))
//    SET_CHECK(IDC_CHECKBOX_SHOW_ON_SENDING);

  p = Set.getString(ID_TITLE);
  if(p)
    SET_TEXT(IDC_EDIT_PAGE_TITLE, p);

  p = Set.getString(ID_MODAL_RECT);
  if(p) {
    SET_CHECK(IDC_CHECKBOX_PP_MOD_PAGE);
    int x = -1;
    int y = -1;
    int w = -1;
    int h = -1;
    _stscanf_s(p, _T("%d,%d,%d,%d"), &x, &y, &w, &h);
    SET_INT(IDC_EDIT_MODAL_X, x);
    SET_INT(IDC_EDIT_MODAL_Y, y);
    SET_INT(IDC_EDIT_MODAL_W, w);
    SET_INT(IDC_EDIT_MODAL_H, h);
    }
  p = Set.getString(ID_MODELESS_FIX);
  if(p && _ttoi(p))
    SET_CHECK(IDC_CHECKBOX_PP_MODELESS);

  p = Set.getString(ID_NO_CAPTION_MODAL);
  if(p && _ttoi(p)) {
    SET_CHECK(IDC_CHECKBOX_PP_NO_TITLE);
    p = findNextParamTrim(p);
    if(p && _ttoi(p))
      SET_CHECK(IDC_CHECKBOX_PP_TRANSP);
    }
  checkModalUse();

  p = Set.getString(ID_LINK_PAGE_DYN);
  if(p) {
    int x = 0;
    int y = 0;
    TCHAR buff[200];
    _stscanf_s(p, _T("%d,%d,%s"), &x, &y, buff, SIZE_A(buff));
    x = REV__X(x);
    y = REV__Y(y);
    SET_INT(IDC_EDIT_LINKED_PAGE_X, x);
    SET_INT(IDC_EDIT_LINKED_PAGE_Y, y);
    SET_TEXT(IDC_EDIT_LINKED_PAGE, buff);
    }

  if(!getUseExtKeyb()) {
    ShowWindow(GetDlgItem(*this, IDC_BUTTON_PREV_PAGE), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_BUTTON_NEXT_PAGE), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_STATIC_PAGE_NUM), SW_HIDE);
    }
  else {
    for(uint i = 0; i < SIZE_A(iPage); ++i)
      iPage[i] = new infoPage;
    iPage[0]->loadPage(this);
    ENABLE(IDC_BUTTON_PREV_PAGE, false);

    iPage[1]->setPage(this);
    for(int i = 0; i < MAX_BTN; ++i) {
      p = Set.getString(ID_F13 + i);
      oldSel[i] = 0;
      if(p) {
        TCHAR t[1000];
        translateFromCRNL(t, p);
        SET_TEXT(IDC_EDIT_TITLE_F1 + i, t);
        p = Set.getString(ID_F13 + MAX_BTN + i);
        if(p)
          setAction(p, i);
        else {
          wsprintf(t, _T(" ")/*, aCustomAction*/);
          setAction(t, i);
          }
        }
      }
    iPage[1]->loadPage(this);
    iPage[2]->setPage(this);
    for(int i = 0; i < MAX_BTN; ++i) {
      p = Set.getString(ID_S5 + i);
      oldSel[i] = 0;
      if(p) {
        TCHAR t[1000];
        translateFromCRNL(t, p);
        SET_TEXT(IDC_EDIT_TITLE_F1 + i, t);
        p = Set.getString(ID_S5 + MAX_BTN + i);
        if(p)
          setAction(p, i);
        else {
          wsprintf(t, _T(" ")/*, aCustomAction*/);
          setAction(t, i);
          }
        }
      }
    iPage[2]->loadPage(this);
    iPage[0]->setPage(this);

    }
  return true;
}
//----------------------------------------------------------------------------
void svmPageProperty::checkModalUse()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_PP_MOD_PAGE);
  uint ids[] = {
    IDC_CHECKBOX_PP_MODELESS,
    IDC_EDIT_MODAL_X,
    IDC_EDIT_MODAL_Y,
    IDC_EDIT_MODAL_W,
    IDC_EDIT_MODAL_H,
    IDC_CHECKBOX_PP_NO_TITLE,
    IDC_CHECKBOX_PP_TRANSP
    };
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], enable);

  enable &= IS_CHECKED(IDC_CHECKBOX_PP_NO_TITLE);
  ENABLE(IDC_CHECKBOX_PP_TRANSP, enable);
}
//----------------------------------------------------------------------------
void svmPageProperty::infoRow::loadRow(PWin* owner, uint row)
{
  GetDlgItemText(*owner, IDC_EDIT_TITLE_F1 + row, label, SIZE_A(label));
  HWND hCBx = GetDlgItem(*owner, IDC_COMBOBOX_ACTION_F1 + row);
  actSel = SendMessage(hCBx, CB_GETCURSEL, 0, 0);
  GetDlgItemText(*owner, IDC_EDIT_RESULT_F1 + row, result, SIZE_A(result));
}
//----------------------------------------------------------------------------
void svmPageProperty::infoRow::setRow(PWin* owner, uint row)
{
  SetDlgItemText(*owner, IDC_EDIT_TITLE_F1 + row, label);
  HWND hCBx = GetDlgItem(*owner, IDC_COMBOBOX_ACTION_F1 + row);
  SendMessage(hCBx, CB_SETCURSEL, actSel, 0);
  SetDlgItemText(*owner, IDC_EDIT_RESULT_F1 + row, result);
}
//----------------------------------------------------------------------------
void svmPageProperty::infoPage::loadPage(PWin* owner)
{
  for(uint i = 0; i < MAX_BTN; ++i)
    iRow[i].loadRow(owner, i);
}
//----------------------------------------------------------------------------
void svmPageProperty::infoPage::setPage(PWin* owner)
{
  for(uint i = 0; i < MAX_BTN; ++i)
    iRow[i].setRow(owner, i);
}
//----------------------------------------------------------------------------
void svmPageProperty::chgBtnText(int ix)
{
  switch(ix) {
    case 0:
      for(uint i = 0; i < MAX_BTN; ++i) {
        TCHAR text[20];
        wsprintf(text, _T("F%d"), i + 1);
        SET_TEXT(IDC_STATICTEXT_1 + i, text);
        }
      break;
    case 1:
      for(uint i = 0; i < 8; ++i) {
        TCHAR text[20];
        wsprintf(text, _T("F%d"), i + 13);
        SET_TEXT(IDC_STATICTEXT_1 + i, text);
        }
      for(uint i = 0; i < 4; ++i) {
        TCHAR text[20];
        wsprintf(text, _T("S%d"), i + 1);
        SET_TEXT(IDC_STATICTEXT_1 + 8 + i, text);
        }
      break;
    case 2:
      for(uint i = 0; i < MAX_BTN; ++i) {
        TCHAR text[20];
        wsprintf(text, _T("S%d"), i + 5);
        SET_TEXT(IDC_STATICTEXT_1 + i, text);
        }
      break;
    }
}
//----------------------------------------------------------------------------
void svmPageProperty::chgPage(bool next)
{
  int val;
  GET_INT(IDC_STATIC_PAGE_NUM, val);
  iPage[val - 1]->loadPage(this);
  if(next) {
    ENABLE(IDC_BUTTON_PREV_PAGE, true);
    if(++val >= 3)
      ENABLE(IDC_BUTTON_NEXT_PAGE, false);
    }
  else {
    ENABLE(IDC_BUTTON_NEXT_PAGE, true);
    if(--val <= 1)
      ENABLE(IDC_BUTTON_PREV_PAGE, false);
    SET_INT(IDC_STATIC_PAGE_NUM, val);
    }
  if(val < 1)
    val = 1;
  else if(val > 3)
    val = 3;
  SET_INT(IDC_STATIC_PAGE_NUM, val);
  chgBtnText(val - 1);
  iPage[val - 1]->setPage(this);
}
//----------------------------------------------------------------------------
LRESULT svmPageProperty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIOBUTTON_CENTERED:
        case IDC_RADIOBUTTON_SCALED:
        case IDC_RADIOBUTTON_FILLED:
        case IDC_RADIOBUTTON_TILED:
          checkEnable();
          break;

        case IDC_BUTTON_PREV_PAGE:
          chgPage(false);
          break;
        case IDC_BUTTON_NEXT_PAGE:
          chgPage(true);
          break;
        case IDC_CHECKBOX_PP_MOD_PAGE:
        case IDC_CHECKBOX_PP_NO_TITLE:
          checkModalUse();
          break;

        case IDC_BUTTON_DLG_ACTION_F1:
        case IDC_BUTTON_DLG_ACTION_F2:
        case IDC_BUTTON_DLG_ACTION_F3:
        case IDC_BUTTON_DLG_ACTION_F4:
        case IDC_BUTTON_DLG_ACTION_F5:
        case IDC_BUTTON_DLG_ACTION_F6:
        case IDC_BUTTON_DLG_ACTION_F7:
        case IDC_BUTTON_DLG_ACTION_F8:
        case IDC_BUTTON_DLG_ACTION_F9:
        case IDC_BUTTON_DLG_ACTION_F10:
        case IDC_BUTTON_DLG_ACTION_F11:
        case IDC_BUTTON_DLG_ACTION_F12:
          actionChangedByBtn((int) LOWORD(wParam));
          break;

        case IDC_BUTTON_CHOOSE_BKG:
          do {
            TCHAR buff[_MAX_PATH];
            GET_TEXT(IDC_EDIT_BKG_FILENAME, buff);
            LPTSTR pBuff = buff;
            if(openFileImageWithCopy(*this, pBuff))
                SetDlgItemText(*this, IDC_EDIT_BKG_FILENAME, buff);
            } while(false);
          break;

        case IDC_BUTTON_CHOOSE_LINKED_PAGE:
          do {
            TCHAR buff[_MAX_PATH];
            TCHAR oldSub[_MAX_PATH];
            dataProject& Data = getDataProject();
            _tcscpy_s(oldSub, Data.getSubPath());

            GET_TEXT(IDC_EDIT_LINKED_PAGE, buff);
            if(IDOK == getPageFile(this, buff).modal())
                SetDlgItemText(*this, IDC_EDIT_LINKED_PAGE, buff);
            Data.setSubPath(oldSub);
           } while(false);
          break;
        case IDB_CHOOSE_SEND_VAR:
          openChooseVarToSend();
          break;
        }
      switch(HIWORD(wParam)) {
        case CBN_SELCHANGE:
          actionChanged((int) LOWORD(wParam), (HWND) lParam);
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
//----------------------------------------------------------------------------
HBRUSH svmPageProperty::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  for(int i = 0; i < MAX_BTN; ++i) {
    if(GetDlgItem(*this, IDC_EDIT_RESULT_F1 + i) == hWndChild) {
      SetBkColor(dc, BKG);
      SetTextColor(dc, 0);
      return bkgEdit;
      }
    }
  if(GetDlgItem(*this, IDC_COMBOBOX_ACTION_F1) == hWndChild ||
     GetDlgItem(*this, IDC_COMBOBOX_ACTION_F12) == hWndChild) {
    SetBkColor(dc, BKG);
    SetTextColor(dc, 0);
    return bkgEdit;
    }
  return 0;
}
//----------------------------------------------------------------------------
void svmPageProperty::openChooseVarToSend()
{
  svmSendVarByPage(Set, this).modal();
//  PModDialog(this, IDD_SEND_OPEN_CLOSE).modal();
}
//----------------------------------------------------------------------------
#define ADDING_PSW _T("PSW(x) - ")
#define POS_PSW  4
#define PREFIX_PSW _T("PSW(")
//----------------------------------------------------------------------------
#define PREFIX_BTN _T("BTN-")
//----------------------------------------------------------------------------
#define PREFIX_PRF _T("PRF-")
//----------------------------------------------------------------------------
/*
enum nameOrId { niNONE, niNAME, niID, niALL };
//----------------------------------------------------------------------------
void svmPageProperty::getBodyFile(LPTSTR buff)
{
  switch(what) {
    case niNONE:
      *buff = 0;
      break;
    case niNAME:
      _tcscpy(buff, _T("prova"));
      break;
    case niID:
      buff[0] = 0;
      if(prefix)
        _tcscpy(buff, prefix);
      _tcscat(buff, _T("-1"));
      break;
    case niALL:
      do {
        svmPageBase Info;
        LPCTSTR p = getPsw(buff, Info.pswLevel);
        Info.id = _ttoi(p);
        if(!Info.id)
          _tcscpy(Info.name, p);
        svmPagePropActionOpenPage op(this, Info);
        if(IDOK == op.modal()) {
          LPTSTR pBuff = setPsw(buff, Info.pswLevel);
          if(Info.id)
            wsprintf(pBuff, _T("%d"), Info.id);
          else
            wsprintf(pBuff, _T("%s"), Info.name);
          }
        } while(false);
      break;
    }
}
*/
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool getIdActionOnlyPsw(PWin* parent, LPTSTR buff, int code, bool prefix)
{
  LPCTSTR title;
  svmPageBase Info;
  LPCTSTR p = getPsw(buff, Info.pswLevel, prefix);
  switch(code) {
    case aPreviousPage:
      title = _T("Pagina Precedente");
      break;
    case aShutdown:
      title = _T("Fine Programma");
      break;
    case aShowTree:
      title = _T("Visualizza Cause/Oper");
      break;
    case aPrintTree:
      title = _T("Stampa Cause/Oper");
      break;
    case aShowAlarm:
      title = _T("Pagina Allarmi");
      break;

    }
//  if(prefix)
//    Info.id = _ttoi(p + SIZE_A(PREFIX_PSW) - 1);
//  else
    Info.id = p ? _ttoi(p) : 0;
  svmPagePropAction op(parent, Info, IDD_ACTION_ONLY_PSW, title);
  if(IDOK == op.modal()) {
    LPTSTR pBuff = setPsw(buff, Info.pswLevel, prefix);
//    if(prefix)
//      wsprintf(pBuff, _T("%s%d"), PREFIX_PSW, Info.id);
//    else
//      wsprintf(pBuff, _T("%d"), Info.id);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool getCTRL_IdAction(PWin* parent, LPTSTR buff, size_t lenBuff, bool prefix)
{
  LPCTSTR title = _T("Scelta Azione CTRL");
  svmPageBase Info;
  LPCTSTR p = getPsw(buff, Info.pswLevel, prefix);
  Info.id = _ttoi(p);
  svmPagePropActionGetCTRL_Action op(parent, Info, title);
  if(IDOK == op.modal()) {
    if(!Info.id)
      Info.pswLevel = 0;
    LPTSTR pBuff = setPsw(buff, Info.pswLevel, prefix);
    wsprintf(pBuff, _T("%d"), Info.id);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool getIdAction(PWin* parent, LPTSTR buff, size_t lenBuff, int code, bool prefix)
{
  LPCTSTR title;
  svmPageBase Info;
  LPCTSTR p = getPsw(buff, Info.pswLevel, prefix);
  switch(code) {
    case aSendAll:
      title = _T("Scelta periferica per Invio totale");
      break;

    case aReceiveAll:
      title = _T("Scelta periferica per Ricezione totale");
      break;

    case aSendOne:
    case aSendToggle:
      do {
        if(prefix && !cmpStr(p, PREFIX_BTN, SIZE_A(PREFIX_BTN) - 1))
          _tcscpy_s(Info.name, p + SIZE_A(PREFIX_BTN) - 1);
        else
          _tcscpy_s(Info.name, p);
        svmPagePropActionGetIdBtn op(parent, Info, _T("Scelta Dati da inviare"));
        if(IDOK == op.modal()) {
          LPTSTR pBuff = setPsw(buff, Info.pswLevel, prefix);
          if(prefix)
            wsprintf(pBuff, _T("%s%s"), PREFIX_BTN, Info.name);
          else
            wsprintf(pBuff, _T("%s"), Info.name);
          return true;
          }
        } while(false);
      return false;
    default:
      return false;
    }
  if(prefix)
    Info.id = _ttoi(p + SIZE_A(PREFIX_PRF) - 1);
  else
    Info.id = _ttoi(p);
  svmPagePropActionGetPerif op(parent, Info, title);
  if(IDOK == op.modal()) {
    LPTSTR pBuff = setPsw(buff, Info.pswLevel, prefix);
    if(prefix)
      wsprintf(pBuff, _T("%s%d"), PREFIX_PRF, Info.id);
    else
      wsprintf(pBuff, _T("%d"), Info.id);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool getIdPrintScreen(PWin* parent, LPTSTR buff)
{
  svmPageBase Info;
  LPCTSTR p = findNextParamTrim(buff);
  int low = 0;
  int high = 0;
  if(p)
    low = _ttoi(p);
  p = findNextParamTrim(p);
  if(p)
    high = _ttoi(p);
  Info.pswLevel = MAKELONG(low, high);
  svmPagePropActionGetPrintScreen op(parent, Info);
  if(IDOK == op.modal()) {
    wsprintf(buff, _T("%d,%d,%d"), aPrintScreen, LOWORD(Info.pswLevel), HIWORD(Info.pswLevel));
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool getIdSpin(PWin* parent, LPTSTR buff, bool prefix)
{
  svmPageBase Info;
  if(prefix) {
    LPCTSTR p = getPsw(buff, Info.pswLevel, prefix);
    _tcscpy_s(Info.name, p);
    }
  else {
    LPCTSTR p = findNextParamTrim(buff);
    if(p)
      Info.pswLevel = _ttoi(p);
    p = findNextParamTrim(p);
    if(p)
      _tcscpy_s(Info.name, p);
    }
  svmPagePropActionSpin op(parent, Info);
  if(IDOK == op.modal()) {
    if(prefix) {
      LPTSTR pBuff = setPsw(buff, Info.pswLevel, prefix);
      wsprintf(pBuff, _T("%s"), Info.name);
      }
    else
      wsprintf(buff, _T("%d,%d,%s"), aSendSpin, Info.pswLevel, Info.name);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool getExportAction(PWin* parent, LPTSTR buff, bool prefix)
{
//  LPCTSTR title;
  svmPageBase Info;
  LPCTSTR p = getPsw(buff, Info.pswLevel, prefix);
  if(prefix)
    Info.id = cmpStr(p, _T("History"), SIZE_A(_T("History")) - 1) ? 0 : 1;
  else
    Info.id = *p - _T('0');
  svmPagePropActionGetExport op(parent, Info);
  if(IDOK == op.modal()) {
    LPTSTR pBuff = setPsw(buff, Info.pswLevel, prefix);
    if(prefix)
      wsprintf(pBuff, _T("%s"), Info.id ? _T("History") : _T("Current"));
    else
      wsprintf(pBuff, _T("%d"), Info.id);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool getPageFile(PWin* parent, LPTSTR buff, size_t lenBuff, int code, bool prefix)
{
  LPCTSTR title;
  svmPageBase Info;
  LPCTSTR p = getPsw(buff, Info.pswLevel, prefix);
  switch(code) {
    case aNewPage:
      do {
        Info.id = _ttoi(p);
        if(!Info.id) {
          if(_T('0') == *p)
            p = findNextParam(p, 1);
          if(p)
            _tcscpy_s(Info.name, p);
          }
        svmPagePropActionOpenPage op(parent, Info);
        if(IDOK == op.modal()) {
          LPTSTR pBuff = setPsw(buff, Info.pswLevel, prefix);
          if(Info.id)
            wsprintf(pBuff, _T("%d"), Info.id);
          else {
            if(prefix)
              wsprintf(pBuff, _T("%s"), Info.name);
            else
              wsprintf(pBuff, _T("0,%s"), Info.name);
            }
          return true;
          }
        } while(false);
      return false;

    case aOpenTrend:
      title = _T("Scelta Pagina Trend");
      break;

    case aOpenfileRecipe:
      title = _T("Scelta Pagina Ricette");
      break;

    case aOpenfileRecipeDan:
      p = findNextParam(p, 1);
      title = _T("Scelta Pagina Ricette con ListBox");
      break;

    case aOpenRowRecipe:
      title = _T("Scelta Pagina Ricette con Righe");
      break;

    case aOpenMaint:
      title = _T("Scelta Pagina Manutenzione");
      break;

    case aOpenModBody:
      title = _T("Scelta Pagina Modale");
      break;
    default:
      return false;
    }
  _tcscpy_s(Info.name, p);
  svmPagePropActionOpenName on(parent, Info, title);
  if(IDOK == on.modal()) {
    LPTSTR pBuff = setPsw(buff, Info.pswLevel, prefix);
    uint len = lenBuff - (pBuff - buff);
    if(aOpenfileRecipeDan == code) {
      _tcscpy_s(pBuff, len, _T("1,"));
      _tcscat_s(pBuff, len, Info.name);
      }
    else
      _tcscpy_s(pBuff, len, Info.name);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void svmPageProperty::actionChangedByBtn(int idCtrl)
{
  idCtrl -= IDC_BUTTON_DLG_ACTION_F1;
  idCtrl += IDC_COMBOBOX_ACTION_F1;
  HWND hCBx = GetDlgItem(*this, idCtrl);

  uint code = SendMessage(hCBx, CB_GETCURSEL, 0, 0);
  int pos = idCtrl - IDC_COMBOBOX_ACTION_F1;
  oldSel[pos] = code;
  actionChanged(idCtrl, hCBx);
}
//----------------------------------------------------------------------------
void addOneToRecipe(LPTSTR buff, size_t lenBuff)
{
  LPTSTR p = (LPTSTR)findNextParam(buff, 2);
  if(!p)
    return;

  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, buff);
  uint len = lenBuff - (p - buff);
  _tcscpy_s(p, len, _T("1,"));
  LPCTSTR p2 = findNextParam(tmp, 2);
  _tcscat_s(p, len, p2);
}
//----------------------------------------------------------------------------
void remOneToRecipe(LPTSTR buff, size_t lenBuff)
{
  LPTSTR p = (LPTSTR)findNextParam(buff, 2);
  if(!p)
    return;
  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, buff);
  uint len = lenBuff - (p - buff);
  LPCTSTR p2 = findNextParam(tmp, 3);
  _tcscpy_s(p, len, p2);
}
//----------------------------------------------------------------------------
bool getChooseAction(PWin* parent, LPTSTR buff, size_t lenBuff, uint code, bool& enableBtn)
{
  bool success = true;

  bool isRecipeDan = code == aOpenfileRecipeDan;
  if(isRecipeDan)
    code = aOpenfileRecipe;

  uint offs = _tcslen(buff);
  if(aCustomAction != code) {
    if(offs) {
      TCHAR tmp[512];
      _tcscpy_s(tmp, buff);
      LPCTSTR p = findNextParamTrim(tmp);
      wsprintf(buff, _T("%d,%s"), code, p);
      p = findNextParamTrim(buff);
      offs = p - buff;
      }
    else {
      wsprintf(buff, _T("%d,"), code);
      offs = _tcslen(buff);
      }
    }
  LPTSTR pBuff = buff + offs;
  switch(code) {
    case aNoAction:
      enableBtn = false;
      buff[0] = 0;
    default:
      break;

    case aNewPage:
    case aOpenTrend:
    case aOpenfileRecipe:
    case aOpenRowRecipe:
    case aOpenModBody:
    case aOpenMaint:
      success = getPageFile(parent, pBuff, lenBuff - offs, code, false);
      break;

    case aSendAll:
    case aReceiveAll:
    case aSendOne:
    case aSendToggle:
      success = getIdAction(parent, pBuff, lenBuff - offs, code, false);
      break;

    case aPreviousPage:
    case aShutdown:
    case aShowTree:
    case aPrintTree:
    case aShowAlarm:
      success = getIdActionOnlyPsw(parent, pBuff, code, false);
      break;

    case aChgPsw:
      if(pBuff)
        _tcscpy_s(pBuff, lenBuff - (pBuff - buff), _T("-0"));
      success = true;
      break;

    case aPrintScreen:
      success = getIdPrintScreen(parent, buff);
      break;

    case aExport:
      success = getExportAction(parent, pBuff, false);
      break;

    case aSendSpin:
      success = getIdSpin(parent, buff, false);
      break;

    case aCTRL_Action:
      success = getCTRL_IdAction(parent, pBuff, lenBuff - offs, false);
      break;

    case aCustomAction:
      enableBtn = false;
      break;

    }
  if(success && isRecipeDan)
    addOneToRecipe(buff, lenBuff);

  return success;
}
//----------------------------------------------------------------------------
void svmPageProperty::actionChanged(int idCtrl, HWND hwnd)
{
  if(IDC_COMBOBOX_SHOWTYPE == idCtrl || IDC_COMBOBOX_BTN_FZ_BEHA == idCtrl)
    return;
  uint code = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  bool enableEdit = false;
  bool enableBtn = true;

  int pos = idCtrl - IDC_COMBOBOX_ACTION_F1;
  TCHAR buff[500];
  GET_TEXT(IDC_EDIT_RESULT_F1 + pos, buff);
  bool success = true;

  bool isRecipeDan = code == aOpenfileRecipeDan;
  if(isRecipeDan) {
    code = aOpenfileRecipe;
    remOneToRecipe(buff, SIZE_A(buff));
    }

  switch(code) {
    case aNoAction:
      enableBtn = false;
    default:
//      buff[0] = 0;
      break;

    case aNewPage:
    case aOpenTrend:
    case aOpenfileRecipe:
    case aOpenRowRecipe:
    case aOpenModBody:
    case aOpenMaint:
      success = getPageFile(this, buff, SIZE_A(buff), code, true);
      break;

    case aSendAll:
    case aReceiveAll:
    case aSendOne:
    case aSendToggle:
      success = getIdAction(this, buff, SIZE_A(buff), code, true);
      break;

    case aPreviousPage:
    case aShutdown:
    case aShowTree:
    case aPrintTree:
    case aShowAlarm:
      success = getIdActionOnlyPsw(this, buff, code, true);
      break;

    case aChgPsw:
      wsprintf(buff, _T("%d"), aChgPsw);
      success = true;
      break;

    case aPrintScreen:
      success = getIdPrintScreen(this, buff);
      break;

    case aExport:
      success = getExportAction(this, buff, true);
      break;

    case aSendSpin:
      success = getIdSpin(this, buff, true);
      break;

    case aCTRL_Action:
      success = getCTRL_IdAction(this, buff, SIZE_A(buff), true);
      break;

    case aCustomAction:
      enableBtn = false;
      enableEdit = true;
      break;

    }
  if(!success)
    SendMessage(hwnd, CB_SETCURSEL, oldSel[pos], 0);
  else {
    if(isRecipeDan)
      addOneToRecipe(buff, SIZE_A(buff));
    SET_TEXT(IDC_EDIT_RESULT_F1 + pos, buff);
    ENABLE(IDC_EDIT_RESULT_F1 + pos, enableEdit);
    ENABLE(IDC_BUTTON_DLG_ACTION_F1 + pos, enableBtn);
    oldSel[pos] = code;
    }
  if(enableEdit)
    SetFocus(GetDlgItem(*this, IDC_EDIT_RESULT_F1 + pos));
}
//----------------------------------------------------------------------------
void svmPageProperty::fillDataBtn(LPTSTR target, int id)
{
  perifForBtnAction data;
  LPCTSTR p = Set.getString(id + ADD_INIT_VAR);
  if(p) {
    int prf;
    int type;
    int neg = 0;
    int addr;
    __int64 mask = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &prf, &addr, &type);
    p = findNextParam(p, 3);
    if(p) {
      switch(type) {
        case 5:
        case 7:
          *(double*)&mask = _tstof(p);
          break;
        default:
          mask = _tstoi64(p);
          break;
        }
      p = findNextParam(p);
      if(p)
        neg = _ttoi(p);
      }

    data.U.s.prf = prf;
    data.addr = addr;
    data.U.s.type = type;
    data.mask = mask;
    data.U.s.neg = toBool(neg);
    }
  p = Set.getString(id + ADD_INIT_VAR + ADD_INIT_BITFIELD);
  if(p) {
    int nbit;
    int offs;
    _stscanf_s(p, _T("%d,%d"),&nbit, &offs);
    data.U.s.nbit = nbit;
    data.U.s.offs = offs;
    }
  _tcscpy_s(target, 255, PREFIX_BTN);
  target += _tcslen(target);
  data.toString(target, data);
}
//----------------------------------------------------------------------------
void svmPageProperty::setAction(LPCTSTR pRow, int pos)
{
  if((uint)pos >= MAX_BTN)
    return;
  int code = 0;
  int psw = 0;
  int third = 0;

  _stscanf_s(pRow, _T("%d,%d"), &code, &psw);
  LPCTSTR p = findNextParam(pRow, 2);
  if(ONLY_CONFIRM == psw || ONLY_CONFIRM_ONE_CHAR == psw)
    psw = ONLY_CONFIRM;
  else if((uint)psw > 3)
    psw = 0;

  TCHAR buff[500];
  LPTSTR pBuff = setPsw(buff, psw, true);

  bool enableEdit = false;
  bool enableBtn = true;

  switch(code) {
    case aNoAction:
      buff[0] = 0;
      enableBtn = false;
      break;

    case aNewPage:
      third = _ttoi(p);
      if(third) {
        wsprintf(pBuff, _T("%d"), third);
        break;
        }
      p = findNextParam(p, 1);
      if(p)
        _tcscpy_s(pBuff, 255, p);
      break;

    case aOpenfileRecipe:
      if(_ttoi(p)) {
        p = findNextParam(p, 1);
        code = aOpenfileRecipeDan;
        }
      // fall throught
    case aOpenTrend:
    case aOpenRowRecipe:
    case aOpenModBody:
    case aOpenMaint:
    case aCTRL_Action:
      _tcscpy_s(pBuff, 255, p);
      break;

    case aPreviousPage:
    case aShutdown:
    case aShowTree:
    case aPrintTree:
    case aShowAlarm:
      break;

    case aChgPsw:
    case aPrintScreen:
      _tcscpy_s(buff, pRow);
      break;

    case aSendAll:
    case aReceiveAll:
      third = _ttoi(p);
      wsprintf(pBuff, _T("%s%d"), PREFIX_PRF, third);
      break;

    case aExport:
      third = _ttoi(p);
      _tcscpy_s(pBuff, 255, third ? _T("History") : _T("Current"));
      break;

    case aSendOne:
    case aSendToggle:
      third = _ttoi(p);
#if 1
      p = Set.getString(third);
      if(p)
        fillDataBtn(pBuff, third);
#else
      #error cercare il codice del button abbinato e ricavare i dati
      wsprintf(pBuff, _T("%s%d"), PREFIX_BTN, third);
#endif
      break;

    case aSendSpin:
      _tcscat_s(pBuff, 255, p);
      break;

    default:
      code = aCustomAction;
    case aCustomAction:
      _tcscpy_s(buff, pRow);
      enableEdit = true;
      enableBtn = false;
      break;

    }
  HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_ACTION_F1 + pos);
  SendMessage(hCBx, CB_SETCURSEL, code, 0);
  SET_TEXT(IDC_EDIT_RESULT_F1 + pos, buff);
  ENABLE(IDC_EDIT_RESULT_F1 + pos, enableEdit);
  ENABLE(IDC_BUTTON_DLG_ACTION_F1 + pos, enableBtn);
  if(enableEdit)
    SetFocus(GetDlgItem(*this, IDC_EDIT_RESULT_F1 + pos));

  oldSel[pos] = code;
}
//----------------------------------------------------------------------------
void fillCB_BtnFzBeha(HWND hwnd, int sel)
{
  static LPCTSTR typeFz[] = {
    _T("Usa Default"),
    _T("Disabilita uso del testo da pagina"),
    _T("Forza uso del testo da pagina"),
    _T("Forza anche uso del prefisso"),
    _T("Disabilita uso del prefisso"),
    };
  for(uint j = 0; j < SIZE_A(typeFz); ++j)
    addStringToComboBox(hwnd, typeFz[j]);
  SendMessage(hwnd, CB_SETCURSEL, sel, 0);
}
//----------------------------------------------------------------------------
void fillCBTypeShow(HWND hwnd, int sel)
{
  static LPCTSTR typeShow[] = {
    _T("Nessuna modifica"),
    _T("Nasconde header e footer"),
    _T("Visualizza header + nasconde footer"),
    _T("Visualizza footer + nasconde header"),
    _T("Visualizza entrambi"),
    };
  for(uint j = 0; j < SIZE_A(typeShow); ++j)
    addStringToComboBox(hwnd, typeShow[j]);
  SendMessage(hwnd, CB_SETCURSEL, sel, 0);
}
//----------------------------------------------------------------------------
static LPCTSTR g_Action[] = {
  _T("Nessuna Azione"),
  _T("Apri Pagina"),
  _T("Invia tutti i Dati"),
  _T("Ricevi tutti i Dati"),
  _T("Esegue Export"),
  _T("Invia Dato specifico"),
  _T("Apri Pagina Trend"),
  _T("Apri Pagina Ricette"),
  _T("Apri Pagina Ricette con righe"),
  _T("Apri Pagina Modale"),
  _T("Cambio Password"),
  _T("Btn per Keyboard(touch screen)"),
  _T("Pagina Precedente"),
  _T("Fine Programma"),
  _T("Visualizza Cause/Oper"),
  _T("Stampa Cause/Oper"),
  _T("Pagina Allarmi"),
  _T("Print Screen"),
  _T("Apri Pagina Manutenzione"),
  _T("Azione Personalizzata"),
  _T("Apri Pagina Ricette con Listbox"),
  _T("Inverti bit Dato specifico"),
  _T("Aziona lo Spin"),
  _T("Azione da CTRL"),
};
//----------------------------------------------------------------------------
LPCTSTR getTextAction(uint sel)
{
  if(sel >= SIZE_A(g_Action))
    sel = 0;
  return g_Action[sel];
}
//----------------------------------------------------------------------------
void fillCBAction(HWND hwnd, int sel)
{
  for(uint j = 0; j < SIZE_A(g_Action); ++j)
    addStringToComboBox(hwnd, g_Action[j]);
  SendMessage(hwnd, CB_SETCURSEL, sel, 0);
}
//----------------------------------------------------------------------------
static int ixsOpenPage[] = {
    aNewPage,
    aOpenTrend,
    aOpenfileRecipe,
    aOpenRowRecipe,
    aOpenMaint,
    aOpenfileRecipeDan
  };
//----------------------------------------------------------------------------
static LPCTSTR actionOpenPage[] = {
  _T("Apri Pagina"),
  _T("Apri Pagina Trend"),
  _T("Apri Pagina Ricette"),
  _T("Apri Pagina Ricette con righe"),
  _T("Apri Pagina Manutenzione"),
  _T("Apri Pagina Ricette con Listbox"),
  };
//----------------------------------------------------------------------------
uint getNumOpenPage() { return SIZE_A(ixsOpenPage); }
//----------------------------------------------------------------------------
void fillCBActionOpenPage(HWND hwnd, int sel)
{
  int s = -1;
  for(uint j = 0; j < SIZE_A(actionOpenPage); ++j) {
    if(j < 0 && ixsOpenPage[j] == sel)
      s = j;
    addStringToComboBox(hwnd, actionOpenPage[j]);
    }
  SendMessage(hwnd, CB_SETCURSEL, s, 0);
}
//----------------------------------------------------------------------------
void fillMenuOpenPage(menuPopup* menu, int nElem, int sel)
{
  nElem = min((uint)nElem, SIZE_A(actionOpenPage));
  for(int i = 0; i < nElem; ++i) {
    menu[i].flag = MF_STRING;
    if(ixsOpenPage[i] == sel)
      menu[i].flag |= MF_CHECKED;
    menu[i].id = ixsOpenPage[i];
    menu[i].text = actionOpenPage[i];
    }
}
//----------------------------------------------------------------------------
int getIxActionByOpenPage(int sel)
{
  if(sel < 0 || (uint)sel >= SIZE_A(ixsOpenPage))
    sel = 0;
  return ixsOpenPage[sel];
}
//----------------------------------------------------------------------------
bool isKeyb4TouchScreen(LPCTSTR row)
{
  return 11 == _ttoi(row);
}
//----------------------------------------------------------------------------
int getKeyb4TouchScreenCode()
{
  return 11;
}
//----------------------------------------------------------------------------
bool isChangePage(LPCTSTR row)
//bool isChangePage(int sel)
{
  int sel = _ttoi(row);
  switch(sel) {
    case 1:
    case 6:
    case 7:
    case 8:
//    case 9:
    case 18:
    // il case della ricetta con listbox viene gestito e cambiato in ricetta semplice,
    // ma per la nuova funzione di apertura pagina da bit è meglio gestire il salvataggio
    // dei dati a parte e quindi deve tornare true anche per il caso 20.
    case 20:
      return true;
    default:
      return false;
    }
}
//----------------------------------------------------------------------------
bool isChgPsw(LPCTSTR row)
{
  return 10 == _ttoi(row);
}
//----------------------------------------------------------------------------
int getChgPswCode()
{
  return 10;
}
//----------------------------------------------------------------------------
bool isSendOneData(LPCTSTR row)
{
  return 5 == _ttoi(row);
}
//----------------------------------------------------------------------------
bool isSendToggleData(LPCTSTR row)
{
  return 21 == _ttoi(row);
}
//----------------------------------------------------------------------------
int getSendOneDataCode()
{
  return 5;
}
//----------------------------------------------------------------------------
int getSendToggleDataCode()
{
  return 21;
}
//----------------------------------------------------------------------------
void svmPageProperty::fillAction()
{
/*
  static LPCTSTR action[] = {
    _T("No Action"),
    _T("Open Page"),
    _T("Send All Data"),
    _T("Receive All Data"),
    _T("Perform Export"),
    _T("Send Specified Data"),
    _T("Open Trend"),
    _T("Open File Recipe"),
    _T("Open Row Recipe"),
    _T("Open Modal Body"),
    _T("Custom Action"),
    };
*/
  for(int i = 0; i < MAX_BTN; ++i) {
    ENABLE(IDC_EDIT_RESULT_F1 + i, false);
    ENABLE(IDC_BUTTON_DLG_ACTION_F1 + i, false);

    HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_ACTION_F1 + i);
    fillCBAction(hCBx, 0);
/*
    for(int j = 0; j < SIZE_A(action); ++j)
      addStringToComboBox(hCBx, action[j]);
    SendMessage(hCBx, CB_SETCURSEL, 0, 0);
*/
    }
/*
  HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_ACTION_F1);
  SendMessage(hCBx, CB_SETCURSEL, aCustomAction, 0);
  EnableWindow(hCBx, false);
  SET_TEXT(IDC_EDIT_RESULT_F1, _T("Previous Page or End Program"));

  hCBx = GetDlgItem(*this, IDC_COMBOBOX_ACTION_F12);
  SendMessage(hCBx, CB_SETCURSEL, aCustomAction, 0);
  EnableWindow(hCBx, false);
  SET_TEXT(IDC_EDIT_RESULT_F12, _T("Alarm Management"));
*/
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR //svmPageProperty::
getPsw(LPCTSTR buff, int& psw, bool prefix)
{
  psw = 0;
  if(prefix) {
    if(!cmpStr(buff, PREFIX_PSW, SIZE_A(PREFIX_PSW) - 1)) {
      psw = _ttoi(buff + SIZE_A(PREFIX_PSW) - 1);
      if(ONLY_CONFIRM_ONE_CHAR == psw)
        psw = ONLY_CONFIRM;
      buff += SIZE_A(ADDING_PSW) - 1;
      }
    }
  else if(buff) {
    psw = _ttoi(buff);
    while(*buff && _istdigit(*buff))
      ++buff;
    if(*buff && _T(',') == *buff)
      ++buff;
    }
  return buff;
}
//----------------------------------------------------------------------------
LPTSTR //svmPageProperty::
#if 0
setPsw(LPTSTR buff, size_t lenBuff, int psw, bool prefix)
{
  if(prefix) {
    _tcscpy_s(buff, lenBuff, ADDING_PSW);
    buff[POS_PSW] = psw + _T('0');
    }
  else
    wsprintf(buff, _T("%d,"), psw);
  LPTSTR pBuff = buff + _tcslen(buff);
  return pBuff;
}
#else
setPsw(LPTSTR buff, int psw, bool prefix)
{
  if(prefix) {
    _tcscpy_s(buff, SIZE_A(ADDING_PSW), ADDING_PSW);
    if(ONLY_CONFIRM == psw)
      psw = ONLY_CONFIRM_ONE_CHAR;
    buff[POS_PSW] = psw + _T('0');
    }
  else
    wsprintf(buff, _T("%d,"), psw);
  LPTSTR pBuff = buff + _tcslen(buff);
  return pBuff;
}
#endif
//----------------------------------------------------------------------------
void svmPageProperty::setBtnLabel(int ix, int btn, LPCTSTR row)
{
  if(!*row)
    return;
  TCHAR buff[256];
  GET_TEXT(IDC_EDIT_TITLE_F1 + ix, buff);
  if(!*buff)
//    if(0 == btn)
//      _tcscpy(buff, _T("Previous Page"));
//    else if(11 == btn)
//      _tcscpy(buff, _T("Alarm Page"));
//    else
      _tcscpy_s(buff, _T("???"));
  TCHAR tmp[256];
  translateToCRNL(tmp, buff);
  Set.replaceString(btn, str_newdup(tmp), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setNewPage(int ix, int btn, LPCTSTR row)
{
  setBtnLabel(ix, btn, row);
  TCHAR buff[100];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  if(_ttoi(p))
    wsprintf(buff, _T("%d,%d,%d"), aNewPage, psw, _ttoi(p));
  else
    wsprintf(buff, _T("%d,%d,0,%s"), aNewPage, psw, p);

  Set.replaceString(btn + MAX_BTN, str_newdup(buff), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setOpen(int action, int ix, int btn, LPCTSTR row, bool recipeDan)
{
  setBtnLabel(ix, btn, row);
  TCHAR buff[100];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  if(recipeDan)
    wsprintf(buff, _T("%d,%d,1,%s"), aOpenfileRecipe, psw, p);
  else
    wsprintf(buff, _T("%d,%d,%s"), action, psw, p);
  Set.replaceString(btn + MAX_BTN, str_newdup(buff), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setSendReceive(int action, int ix, int btn, LPCTSTR row)
{
  setBtnLabel(ix, btn, row);
  TCHAR buff[100];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  p += SIZE_A(PREFIX_PRF) - 1;

  wsprintf(buff, _T("%d,%d,%d"), action, psw, _ttoi(p));

  Set.replaceString(btn + MAX_BTN, str_newdup(buff), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setCTRL_Action(int ix, int btn, LPCTSTR row)
{
  setBtnLabel(ix, btn, row);
  TCHAR buff[100];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  wsprintf(buff, _T("%d,%d,%d"), aCTRL_Action, psw, _ttoi(p));

  Set.replaceString(btn + MAX_BTN, str_newdup(buff), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setPrintScreen(int action, int ix, int btn, LPCTSTR row)
{
  setBtnLabel(ix, btn, row);
/*
  TCHAR buff[100];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  p += SIZE_A(PREFIX_PRF) - 1;

  wsprintf(buff, _T("%d,%d,%d"), action, psw, _ttoi(p));
*/
  Set.replaceString(btn + MAX_BTN, str_newdup(row), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setOnlyPsw(int action, int ix, int btn, LPCTSTR row)
{
  setBtnLabel(ix, btn, row);
  TCHAR buff[100];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  wsprintf(buff, _T("%d,%d"), action, psw);

  Set.replaceString(btn + MAX_BTN, str_newdup(buff), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setExport(int ix, int btn, LPCTSTR row)
{
  setBtnLabel(ix, btn, row);
  TCHAR buff[100];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  int hist = cmpStr(p, _T("History"), SIZE_A(_T("History")) - 1) ? 0 : 1;
  wsprintf(buff, _T("%d,%d,%d"), aExport, psw, hist);

  Set.replaceString(btn + MAX_BTN, str_newdup(buff), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setSpin(int ix, int btn, LPCTSTR row)
{
  setBtnLabel(ix, btn, row);
  TCHAR buff[100];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  wsprintf(buff, _T("%d,%d,%s"), aSendSpin, psw, p);

  Set.replaceString(btn + MAX_BTN, str_newdup(buff), true);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static
LPTSTR getSendOneToggleByBtn(uint code, LPCTSTR row, setOfString& set)
{
  perifForBtnAction data;
  row = findNextParam(row, 1);
  int psw = _ttoi(row);
  row = findNextParam(row, 1);
  int allowedBtn = _ttoi(row);

  TCHAR buff[500];
  wsprintf(buff, _T("%d,%d,"), code, psw);
  LPTSTR pBuff = buff + _tcslen(buff);

  manageObjId moi(allowedBtn, ID_INIT_VAR_BTN);

  allowedBtn = moi.getFirstExtendId();
  LPCTSTR p = set.getString(allowedBtn);
  if(p) {
    int prf;
    int type;
    int neg = 0;
    int addr;
    __int64 mask = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &prf, &addr, &type);
    p = findNextParam(p, 3);
    if(p) {
      switch(type) {
        case 5:
        case 7:
          *(double*)&mask = _tstof(p);
          break;
        default:
          mask = _tstoi64(p);
          break;
        }
      p = findNextParam(p);
      if(p)
        neg = _ttoi(p);
      }

    data.U.s.prf = prf;
    data.addr = addr;
    data.U.s.type = type;
    data.mask = mask;
    data.U.s.neg = toBool(neg);
    }
  allowedBtn = moi.getSecondExtendId();
  p = set.getString(allowedBtn);
  if(p) {
    int nbit;
    int offs;
    _stscanf_s(p, _T("%d,%d"),&nbit, &offs);
    data.U.s.nbit = nbit;
    data.U.s.offs = offs;
    }
  data.toString(pBuff, data);
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
LPTSTR getSendOneByBtn(LPCTSTR row, setOfString& set)
{
#if 1
  return getSendOneToggleByBtn(getSendOneDataCode(), row, set);
#else
  perifForBtnAction data;
  row = findNextParam(row, 1);
  int psw = _ttoi(row);
  row = findNextParam(row, 1);
  int allowedBtn = _ttoi(row);

  TCHAR buff[500];
  wsprintf(buff, _T("%d,%d,"), getSendOneDataCode(), psw);
  LPTSTR pBuff = buff + _tcslen(buff);

  LPCTSTR p = set.getString(allowedBtn + ADD_INIT_VAR);
  if(p) {
    int prf;
    int type;
    int neg;
    int addr;
    int mask;
    _stscanf(p, _T("%d,%d,%d,%d,%d"), &prf, &addr, &type, &mask, &neg);
    data.U.s.prf = prf;
    data.addr = addr;
    data.U.s.type = type;
    data.mask = mask;
    data.U.s.neg = toBool(neg);
    }
  p = set.getString(allowedBtn + ADD_INIT_VAR + ADD_INIT_BITFIELD);
  if(p) {
    int nbit;
    int offs;
    _stscanf(p, _T("%d,%d"),&nbit, &offs);
    data.U.s.nbit = nbit;
    data.U.s.offs = offs;
    }
  data.toString(pBuff, data);
  return str_newdup(buff);
#endif
}
//----------------------------------------------------------------------------
LPTSTR getSendToggleByBtn(LPCTSTR row, setOfString& set)
{
  return getSendOneToggleByBtn(getSendToggleDataCode(), row, set);
}
//----------------------------------------------------------------------------
int setSendOneByBtn(LPCTSTR row, int allowedBtn, P_File& pf)
{
  TCHAR buff[256];

  row = findNextParam(row, 1);
  int psw;
  LPCTSTR p = getPsw(row, psw, false);

  perifForBtnAction data;
  data.fromString(data, p);


  wsprintf(buff, _T("%d,0,0,10,10,1,5,0\r\n"), allowedBtn);
  writeStringChkUnicode(pf, buff);

  manageObjId moi(allowedBtn, ID_INIT_VAR_BTN);

  allowedBtn = moi.getFirstExtendId();
  TCHAR tmp[256];
  switch(data.U.s.type) {
    case 5:
      _stprintf_s(tmp, _T("%f"), (float)*(double*)&data.mask);
      break;
    case 7:
      _stprintf_s(tmp, _T("%f"), *(double*)&data.mask);
      break;
    default:
      _stprintf_s(tmp, _T("%I64d"), data.mask);
      break;
    }

  wsprintf(buff, _T("%d,%d,%d,%d,%s,%d\r\n"),
        allowedBtn,
        data.U.s.prf,
        data.addr,
        data.U.s.type,
        tmp,
        data.U.s.neg);
  writeStringChkUnicode(pf, buff);

  allowedBtn = moi.getSecondExtendId();
  wsprintf(buff, _T("%d,%d,%d\r\n"), allowedBtn, data.U.s.nbit, data.U.s.offs);
  writeStringChkUnicode(pf, buff);

  return psw;
}
//----------------------------------------------------------------------------
void svmPageProperty::setSendOne(uint code, int ix, int btn, LPCTSTR row, int allowedBtn)
{
  setBtnLabel(ix, btn, row);
  TCHAR buff[256];

  int psw;
  LPCTSTR p = getPsw(row, psw, true);

  p += SIZE_A(PREFIX_BTN) - 1;

  perifForBtnAction data;
  data.fromString(data, p);

  manageObjId moi(0, ID_INIT_VAR_BTN);
  allowedBtn = moi.calcAndSetExtendId(allowedBtn);

  wsprintf(buff, _T("%d,%d,%d"), code, psw, allowedBtn);
  Set.replaceString(btn + MAX_BTN, str_newdup(buff), true);

  wsprintf(buff, _T("0,0,10,10,1,5,0"));
  Set.replaceString(allowedBtn, str_newdup(buff), true);

  allowedBtn = moi.getFirstExtendId();

  TCHAR tmp[256];
  switch(data.U.s.type) {
    case 5:
      _stprintf_s(tmp, _T("%f"), (float)*(double*)&data.mask);
      break;
    case 7:
      _stprintf_s(tmp, _T("%f"), *(double*)&data.mask);
      break;
    default:
      _stprintf_s(tmp, _T("%I64d"), data.mask);
      break;
    }
  wsprintf(buff, _T("%d,%d,%d,%s,%d"),
        data.U.s.prf,
        data.addr,
        data.U.s.type,
        tmp,
        data.U.s.neg);
  Set.replaceString(allowedBtn, str_newdup(buff), true);

  allowedBtn = moi.getSecondExtendId();
  wsprintf(buff, _T("%d,%d"), data.U.s.nbit, data.U.s.offs);
  Set.replaceString(allowedBtn, str_newdup(buff), true);
}
//----------------------------------------------------------------------------
void svmPageProperty::setCustom(int ix, int btn, LPCTSTR row)
{
  setBtnLabel(ix, btn, row);
  Set.replaceString(btn + MAX_BTN, str_newdup(row), true);
}
//----------------------------------------------------------------------------
extern void resetSavePredef(setOfString& Set);
//----------------------------------------------------------------------------
void svmPageProperty::CmOk()
{
  LPTSTR oldPageType = str_newdup(Set.getString(ID_CODE_PAGE_TYPE));
  resetSavePredef(Set);
//  Set.reset();
  TCHAR buff[500];
  // il primo e l'ultimo pulsante sono riservat1
//  setBtnLabel(0, _T(" "));
//  setBtnLabel(MAX_BTN - 1, _T(" "));
  int allowedBtn = 0;

  uint idsBtn[] = { ID_INIT_MENU, ID_F13, ID_S5 };
  uint repeat = SIZE_A(idsBtn);
  if(!iPage[0])
    repeat = 1;
  else {
    int val;
    GET_INT(IDC_STATIC_PAGE_NUM, val);
    iPage[val - 1]->loadPage(this);
    }
  for(uint k = 0; k < repeat; ++k) {
    if(iPage[k])
      iPage[k]->setPage(this);
    for(int i = 0; i < MAX_BTN; ++i) {
      HWND hCBx = GetDlgItem(*this, IDC_COMBOBOX_ACTION_F1 + i);
      int pos = SendMessage(hCBx, CB_GETCURSEL, 0, 0);
      if(pos > 0) {
        GET_TEXT(IDC_EDIT_RESULT_F1 + i, buff);
        // se non c'è testo, ma è selezionato custom carica la label
        if(!*buff && aCustomAction == pos)
          GET_TEXT(IDC_EDIT_TITLE_F1 + i, buff);
        }
      else
        GET_TEXT(IDC_EDIT_TITLE_F1 + i, buff);
//      buff[0] = 0;

      switch(pos) {

        case aNewPage:
          setNewPage(i, idsBtn[k] + i, buff);
          break;

        case aOpenTrend:
        case aOpenfileRecipe:
        case aOpenRowRecipe:
        case aOpenModBody:
        case aOpenfileRecipeDan:
        case aOpenMaint:
          setOpen(pos, i, idsBtn[k] + i, buff, aOpenfileRecipeDan == pos);
          break;

        case aSendAll:
        case aReceiveAll:
          setSendReceive(pos, i, idsBtn[k] + i, buff);
          break;

        case aPreviousPage:
        case aShutdown:
        case aShowTree:
        case aPrintTree:
        case aShowAlarm:
          setOnlyPsw(pos, i, idsBtn[k] + i, buff);
          break;

        case aChgPsw:
        case aPrintScreen:
          setPrintScreen(pos, i, idsBtn[k] + i, buff);
          break;


        case aExport:
          setExport(i, idsBtn[k] + i, buff);
          break;

        case aSendOne:
        case aSendToggle:
          setSendOne(pos, i, idsBtn[k] + i, buff, allowedBtn);
          ++allowedBtn;
          break;

        case aSendSpin:
          setSpin(i, idsBtn[k] + i, buff);
          break;

        case aCTRL_Action:
          setCTRL_Action(i, idsBtn[k] + i, buff);
          break;

        case aNoAction:
        default:
          if(!*buff)
            break;
        case aCustomAction:
          setCustom(i, idsBtn[k] + i, buff);
          break;
        }
      }
    }

  TCHAR name[256];
  GET_TEXT(IDC_EDIT_PAGE_TITLE, name);
  Set.replaceString(ID_TITLE, str_newdup(name), true);

  GET_TEXT(IDC_EDIT_BKG_FILENAME, name);
  if(*name) {
    uint style = 0;
    if(IS_CHECKED(IDC_RADIOBUTTON_SCALED))
      style = bSCALED;
    else if(IS_CHECKED(IDC_RADIOBUTTON_FILLED))
      style = bFILLED;
    else if(IS_CHECKED(IDC_RADIOBUTTON_CENTERED))
      style = bCENTERED;
    else
      style = bTILED;

    bool transp = IS_CHECKED(IDC_CHECKBOX_TRANSPARENT);
    int scaleX;
    GET_INT(IDC_EDIT_SCALE_X, scaleX);
    int scaleY;
    GET_INT(IDC_EDIT_SCALE_Y, scaleY);

    if(bTILED == style || bFILLED == style)
      transp = false;

    if(bSCALED == style || bFILLED == style) {
      scaleX = 0;
      scaleY = 0;
      }
    if(transp)
      style += 1000;
    dataProject& dP = getDataProject();
    TCHAR full[_MAX_PATH];
    _tcscpy_s(full, dP.imagePath);
    appendPath(full, name);
    wsprintf(buff, _T("0,0,%d,%d,%d,%s"), scaleX, scaleY, style, full);
    Set.replaceString(ID_INIT_BMP, str_newdup(buff), true);
    }
  else
    Set.replaceString(ID_INIT_BMP, 0, false, true);

  GET_TEXT(IDC_EDIT_LINKED_PAGE, name);
  if(*name) {
    int x;
    int y;
    GET_INT(IDC_EDIT_LINKED_PAGE_X, x);
    GET_INT(IDC_EDIT_LINKED_PAGE_Y, y);
    x = R__X(x);
    y = R__Y(y);
    TCHAR buff[200];
    wsprintf(buff, _T("%d,%d,%s"), x, y, name);
    Set.replaceString(ID_LINK_PAGE_DYN, str_newdup(buff), true);
    }
  else
    Set.replaceString(ID_LINK_PAGE_DYN, 0, false, true);

  bool autoSend = IS_CHECKED(IDC_CHECKBOX_AUTO_UPDATE_EDIT);
  if(!autoSend)
    _tcscpy_s(buff, _T("0"));
  else
    _tcscpy_s(buff, _T("1"));
  Set.replaceString(ID_DEF_SEND_EDIT, str_newdup(buff), true);

//  bool show = IS_CHECKED(IDC_CHECKBOX_SHOW_ON_SENDING);
//  if(!show)
    _tcscpy_s(buff, _T("1"));
//  else
//    _tcscpy_s(buff, _T("0"));
  Set.replaceString(ID_SHOW_SIMPLE_SEND, str_newdup(buff), true);

  if(IS_CHECKED(IDC_CHECKBOX_PP_MOD_PAGE)) {
    int x;
    int y;
    int w;
    int h;
    GET_INT(IDC_EDIT_MODAL_X, x);
    GET_INT(IDC_EDIT_MODAL_Y, y);
    GET_INT(IDC_EDIT_MODAL_W, w);
    GET_INT(IDC_EDIT_MODAL_H, h);
    if(w < 50)
      w = -1;
    if(h < 50)
      h = -1;
    wsprintf(buff, _T("%d,%d,%d,%d"), x, y, w, h);
    Set.replaceString(ID_MODAL_RECT, str_newdup(buff), true);
    if(IS_CHECKED(IDC_CHECKBOX_PP_MODELESS))
      Set.replaceString(ID_MODELESS_FIX, str_newdup(_T("1")), true);
    else
      Set.replaceString(ID_MODELESS_FIX, 0);

    if(IS_CHECKED(IDC_CHECKBOX_PP_NO_TITLE)) {
      if(IS_CHECKED(IDC_CHECKBOX_PP_TRANSP))
        Set.replaceString(ID_NO_CAPTION_MODAL, str_newdup(_T("1,1")), true);
      else
        Set.replaceString(ID_NO_CAPTION_MODAL, str_newdup(_T("1,0")), true);
      }
    else
      Set.replaceString(ID_NO_CAPTION_MODAL, 0);
    }
  else {
    Set.replaceString(ID_MODAL_RECT, 0);
    Set.replaceString(ID_MODELESS_FIX, 0);
    Set.replaceString(ID_NO_CAPTION_MODAL, 0);
    }

  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_SHOWTYPE), CB_GETCURSEL, 0, 0);
  if(sel > 0) {
    int t = 0;
    LPCTSTR p = oldPageType;
    if(p)
      t = _ttoi(p);
    int vals[] = {  3, 2, 1, 0 };
    TCHAR buff[64];
    wsprintf(buff, _T("%d,%d"), t, vals[sel - 1]);
    Set.replaceString(ID_CODE_PAGE_TYPE, str_newdup(buff), true);
    delete []oldPageType;
    }
  else
    Set.replaceString(ID_CODE_PAGE_TYPE, oldPageType, true);

  sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_BTN_FZ_BEHA), CB_GETCURSEL, 0, 0);
  if(sel > 0) {
    TCHAR buff[64];
    wsprintf(buff, _T("%d"), sel);
    Set.replaceString(ID_CODE_BTN_FZ_BEHA, str_newdup(buff), true);
    }
  else
    Set.replaceString(ID_CODE_BTN_FZ_BEHA, 0, true);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void svmPageProperty::checkEnable()
{
  bool enable = IS_CHECKED(IDC_RADIOBUTTON_TILED);
  enable |= IS_CHECKED(IDC_RADIOBUTTON_CENTERED);
  ENABLE(IDC_EDIT_SCALE_X, enable);
  ENABLE(IDC_EDIT_SCALE_Y, enable);

  enable = IS_CHECKED(IDC_RADIOBUTTON_CENTERED);
  enable |= IS_CHECKED(IDC_RADIOBUTTON_SCALED);
  ENABLE(IDC_CHECKBOX_TRANSPARENT, enable);
}
//----------------------------------------------------------------------------
