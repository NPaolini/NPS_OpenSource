//----------- cust_graph.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdio.h>
//----------------------------------------------------------------------------
#include "Macro_utils.h"
#include "cust_graph.h"
#include "pEdit.h"
#include "p_Util.h"
#include "headerMsg.h"
#include "id_msg_common.h"
#include "svmProperty.h"
#include "plistbox.h"
#include "svmPropertyTrend.h"
//----------------------------------------------------------------------------
#define IDC_EDIT_GR_R_1 10000
#define FIRST_ID_ROW  IDC_EDIT_GR_R_1
//----------------------------------------------------------------------------
#define OFFSET_ID_ROW     20
#define OFFSET_ID_CLR     (OFFSET_ID_ROW * 3)
#define OFFSET_ID_PT_SCR  (OFFSET_ID_ROW * 4)
#define OFFSET_ID_PT_PRN  (OFFSET_ID_ROW * 5)
//----------------------------------------------------------------------------
#define INIT_X   4
#define INIT_Y  60
//----------------------------------------------------------------------------
#define WIDTH   28
#define HEIGHT  20
//----------------------------------------------------------------------------
#define D_X (WIDTH)
#define D_Y (HEIGHT + 1)
//----------------------------------------------------------------------------
PSimpleColorRow::PSimpleColorRow(PWin* parent, uint first_id) :
    firstId(first_id), Focus(false)
{
  PRect r(0, 0, WIDTH, HEIGHT);
  r.MoveTo(INIT_X, INIT_Y + (first_id - FIRST_ID_ROW) * D_Y);
  for(uint i = 0; i < SIZE_A(rgb); ++i) {
    rgb[i] = new P_EditFocus(parent, firstId + i * OFFSET_ID_ROW, r, 0, 3);
    rgb[i]->setFilter(new PRGBFilter);
    r.Offset(D_X, 0);
    }
  cbx = new PColorBox(parent, 0, firstId + OFFSET_ID_CLR, r);
}
//----------------------------------------------------------------------------
PSimpleColorRow::checkFocus PSimpleColorRow::setFocus(uint firstIdSB, HWND hwnd)
{
  bool focus = hwnd ? false : true;
  if(!focus)
    for(uint i = 0; i < SIZE_A(rgb); ++i)
      if(hwnd == *rgb[i]) {
        focus = true;
        break;
        }
  if(!focus && Focus) {
    Focus = false;
    return PColorRow::LOSS;
    }

  if(!hwnd || focus && !Focus) {
    PWin* parent = rgb[0]->getParent();
    for(int i = 0; i < 3; ++i) {
      HWND hsb = GetDlgItem(*parent, firstIdSB + i);
      int val[3];
      if(hsb) {
        TCHAR buff[20];
        GetWindowText(*rgb[i], buff, SIZE_A(buff));
        val[i] = _ttoi(buff);
        SendMessage(hsb, SBM_SETPOS, val[i], true);
        }
      cust_graph* cg = dynamic_cast<cust_graph*>(parent);
      if(cg)
        cg->setCurrColor(RGB(val[0], val[1], val[2]));
      }
    Focus = true;
    return PColorRow::GET;
    }
  return PColorRow::NOCHANGE;
}
//----------------------------------------------------------------------------
COLORREF PSimpleColorRow::getColor()
{
  int val[3];
  TCHAR buff[10];
  for(int i = 0; i < 3; ++i) {
    GetWindowText(*rgb[i], buff, SIZE_A(buff));
    val[i] = _ttoi(buff);
    }
  return RGB(val[0], val[1], val[2]);
}
//----------------------------------------------------------------------------
void PSimpleColorRow::setColorBySelf(uint firstIdSB)
{
  int val[3];
  TCHAR buff[10];
  PWin* parent = rgb[0]->getParent();
  for(int i = 0; i < 3; ++i) {
    GetWindowText(*rgb[i], buff, SIZE_A(buff));
    val[i] = _ttoi(buff);

    HWND hsb = GetDlgItem(*parent, firstIdSB + i);
    if(hsb)
      SendMessage(hsb, SBM_SETPOS, val[i], true);
    }
  cbx->set(RGB(val[0], val[1], val[2]));
}
//----------------------------------------------------------------------------
void PSimpleColorRow::setColor(COLORREF color)
{
  int val[3] = {
      GetRValue(color),
      GetGValue(color),
      GetBValue(color)
      };
  TCHAR buff[10];
  for(int i = 0; i < 3; ++i) {
    wsprintf(buff, _T("%d"), val[i]);
    SetWindowText(*rgb[i], buff);
    }
  cbx->set(color);
}
//----------------------------------------------------------------------------
void PSimpleColorRow::setAll(LPCTSTR p)
{
  int r = 0;
  int g = 0;
  int b = 0;
  _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
  setColor(RGB(r, g, b));
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PColorRow::PColorRow(PWin* parent, uint first_id) :
    baseClass(parent, first_id)
{
  PRect r(0, 0, WIDTH, HEIGHT);
  r.MoveTo(INIT_X, INIT_Y + (first_id - FIRST_ID_ROW) * D_Y);
  r.Offset(D_X * (SIZE_A(rgb) + 1) + 1, 0);

  ptScr = new P_EditFocus(parent, firstId + OFFSET_ID_PT_SCR, r, 0, 3);
  r.Offset(D_X, 0);
  ptPrn = new P_EditFocus(parent, firstId + OFFSET_ID_PT_PRN, r, 0, 3);
}
//----------------------------------------------------------------------------
/*
PColorRow::~PColorRow()
{
  delete cbx;
}
*/
//----------------------------------------------------------------------------
PSimpleColorRow::checkFocus PColorRow::setFocus(uint firstIdSB, HWND hwnd)
{
  if(hwnd) {
    bool focus = toBool(hwnd == *ptScr);
    if(!focus)
      focus = toBool(hwnd == *ptPrn);
    if(focus)
      return baseClass::setFocus(firstIdSB, *rgb[0]);
    }
  return baseClass::setFocus(firstIdSB, hwnd);
}
//----------------------------------------------------------------------------
WORD PColorRow::getPtScr()
{
  TCHAR buff[10];
  GetWindowText(*ptScr, buff, SIZE_A(buff));
  return (WORD)_ttoi(buff);
}
//----------------------------------------------------------------------------
void PColorRow::setPtScr(WORD val)
{
  TCHAR buff[10];
  wsprintf(buff, _T("%d"), val);
  SetWindowText(*ptScr, buff);
}
//----------------------------------------------------------------------------
WORD PColorRow::getPtPrn()
{
  TCHAR buff[10];
  GetWindowText(*ptPrn, buff, SIZE_A(buff));
  return (WORD)_ttoi(buff);
}
//----------------------------------------------------------------------------
void PColorRow::setPtPrn(WORD val)
{
  TCHAR buff[10];
  wsprintf(buff, _T("%d"), val);
  SetWindowText(*ptPrn, buff);
}
//----------------------------------------------------------------------------
void PColorRow::setAll(LPCTSTR p)
{
  int r = 0;
  int g = 0;
  int b = 0;
  int scr = 2;
  int prn = 5;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &r, &g, &b, &scr, &prn);
  setColor(RGB(r, g, b));
  setPtScr(scr);
  setPtPrn(prn);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool runCustomGraph(PWin* parent)
{
  return IDOK == cust_graph(parent).modal();
}
//----------------------------------------------------------------------------
cust_graph::cust_graph(PWin* parent, uint resId, HINSTANCE hinstance) :
    PModDialog(parent, resId, hinstance),
        Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
        Brush3(CreateSolidBrush(bkgColor3))
{
  for(int i = 0; i < MAX_GROUP; ++i)
    Rows[i] = new PColorRow(this, IDC_EDIT_GR_R_1 + i);

  axText = new PSimpleColorRow(this, IDC_EDIT_GR_R_1 + MAX_GROUP + 1);


  COLORREF base[] = { RGB(0xff, 0, 0), RGB(0, 0xff, 0), RGB(0, 0, 0xff) };
  for(uint i = 0; i < SIZE_A(BaseColor); ++i)
    BaseColor[i] = new PColorBox(this, base[i], IDC_STATIC_GR_BASE_R + i, getHInstance());
  CurrColor = new PColorBox(this, RGB(0,0,0), IDC_STATIC_GR_CURR_COLOR, getHInstance());
}
//----------------------------------------------------------------------------
cust_graph::~cust_graph()
{
  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
  DeleteObject(HGDIOBJ(Brush3));

  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  delete axText;
  destroy();
//  delete Bmp[1];
//  delete Bmp[0];
}
//----------------------------------------------------------------------------
#define LEN_LB_DIM_ID   7
#define LEN_LB_INFO    18
#define LEN_LB_TEXT   (100 * 10)
//----------------------------------------------------------------------------
#define MAX_LEN_LB   (LEN_LB_DIM_ID + LEN_LB_INFO + LEN_LB_TEXT + 5)
//----------------------------------------------------------------------------
bool cust_graph::create()
{
  PListBox* LB = new PListBox(this, IDC_LIST_GR_TEXT);
  int tabs[] = { LEN_LB_DIM_ID, LEN_LB_INFO, LEN_LB_TEXT };
  LB->SetTabStop(SIZE_A(tabs), tabs, 0);


  if(!PModDialog::create())
    return false;

  LB->setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT), false);
  LB->rescale(0, 0, 1.1);
  LB->rescale(1, 1, 0.8);
  LB->setIntegralHeight();

  LPCTSTR timeForm[] = { _T("ore"), _T("minuti"), _T("hh:mm:ss") };
  HWND ctrl = GetDlgItem(*this, IDC_COMBO_GR_FIX_TIME);
  for(uint i = 0; i < SIZE_A(timeForm); ++i)
    addStringToComboBox(ctrl, timeForm[i]);

  ctrl = GetDlgItem(*this, IDC_COMBO_GR_TIME_PSW);
  fillCBPswLevel(ctrl, 0);

  for(int i = 0; i < 3; ++i) {
    HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_GR_R + i);
    if(hsb)
      SendMessage(hsb, SBM_SETRANGE , 0, 255);
    }

  Rows[0]->setFocus(IDC_SCROLLBAR_GR_R);
  loadData();

  return true;
}
//----------------------------------------------------------------------------
LRESULT cust_graph::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_INITDIALOG:
      SetFocus(GetDlgItem(hwnd, IDC_EDIT_GR_R_1));
      return FALSE;

    case WM_CUSTOM_SET_FOCUS:
      do {
        HWND focus = GetFocus();
        for(int i = 0; i < MAX_GROUP; ++i)
          Rows[i]->setFocus(IDC_SCROLLBAR_GR_R, focus);
        axText->setFocus(IDC_SCROLLBAR_GR_R, focus);
        } while(false);
      break;


    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_GR_SAVE:
          saveData();
          break;
        case IDC_CHECK_GR_FIX:
          checkEnableScaleX();
          break;

        case IDC_CHECK_GR_LINE_TYPE:
          checkEnabletypeRow();
          break;

        case IDC_BUTTON_GR_MOD_LIST_TEXT:
          modifyLB();
          break;
        }
      switch(HIWORD(wParam)) {
        case EN_KILLFOCUS:
          if(IDC_EDIT_GR_XSCALE == LOWORD(wParam))
            checkEnableScaleX();
          break;

        case EN_CHANGE:
          do {
            int id = LOWORD(wParam);
            if(IDC_EDIT_GR_R_1 <= id && id < IDC_EDIT_GR_R_1 + OFFSET_ID_CLR) {
              id -= IDC_EDIT_GR_R_1;
#if 1
              id %= OFFSET_ID_ROW;
              if(id > MAX_GROUP)
                axText->setColorBySelf(IDC_SCROLLBAR_GR_R);
              else
                Rows[id]->setColorBySelf(IDC_SCROLLBAR_GR_R);
#else
              if(id > MAX_GROUP && id < OFFSET_ID_ROW)
                axText->->setColorBySelf(IDC_SCROLLBAR_GR_R);
              else {
                id %= MAX_GROUP;
                Rows[id]->setColorBySelf(IDC_SCROLLBAR_GR_R);
                }
#endif
              }
            } while(false);
          break;
        case LBN_DBLCLK:
          if(IDC_LIST_GR_TEXT == LOWORD(wParam))
            modifyLB();
          break;
        case CBN_SELCHANGE:
          if(IDC_COMBO_GR_TREND_NAME == LOWORD(wParam))
            changedCBTrendName();
          break;
        }
      break;
    case WM_HSCROLL:
      evHScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
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
  return PModDialog::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//------------------------------------------------------------------------------
void cust_graph::evHScrollBar(HWND child, int flags, int pos)
{
  int curr = SendMessage(child, SBM_GETPOS, 0, 0);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = 255;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - 16;
      break;
    case SB_PAGERIGHT:
      pos = curr + 16;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if(pos > 255)
    pos = 255;
  SendMessage(child, SBM_SETPOS, pos, true);

  int val[3];
  for(int i = 0; i < 3; ++i) {
    HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_GR_R + i);
    val[i] = SendMessage(hsb, SBM_GETPOS, 0, 0);
    }
  PSimpleColorRow* cr = 0;
  if(axText->hasFocus())
    cr = axText;
  else
    for(int i = 0; i < MAX_GROUP; ++i)
      if(Rows[i]->hasFocus()) {
        cr = Rows[i];
        break;
        }
  if(cr) {
    cr->setColor(RGB(val[0], val[1], val[2]));
    CurrColor->set(RGB(val[0], val[1], val[2]));
    }
}
//------------------------------------------------------------------------------
struct infoLb {
  uint id;
  LPCTSTR info;
  };
//------------------------------------------------------------------------------
static infoLb iLb[] =
{
    { 1000020, _T("Scelta Grafici") },
    { 1000021, _T("Ok") },
    { 1000022, _T("Annulla") },
    { 1000023, _T("Aggiungi") },
    { 1000024, _T("Togli") },
    { 1000025, _T("Sposta Su") },
    { 1000026, _T("Sposta Giu") },
    { 1000070, _T("Stampa Grafici") },
    { 1000071, _T("Num. di pagine") },
    { 1000076, _T("Scelta File Trend") },
    { 1000080, _T("Imp. Timer Salv.") },
    { 1000081, _T("mSec") },
};
//------------------------------------------------------------------------------
//#define WRT_TEXT writeStringChkUnicode
#define WRT_TEXT writeStringForceUnicode
//------------------------------------------------------------------------------
void cust_graph::saveData()
{
  P_File* pfCrypt = makeFileTmpCustom(true);
  P_File* pfClear = makeFileTmpCustom(false);
  if(!pfCrypt || !pfClear) {
    delete pfCrypt;
    delete pfClear;
    return;
    }
  bool success = true;
  TCHAR buff[1500];
  HWND hlb = GetDlgItem(*this, IDC_LIST_GR_TEXT);
  int nElem = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  for(int i = 0; i < nElem; ++i) {
    SendMessage(hlb, LB_GETTEXT, i, (LPARAM)buff);
    uint code = _ttoi(buff);
    LPTSTR p = buff + LEN_LB_DIM_ID + LEN_LB_INFO + 2;
    p[LEN_LB_TEXT] = 0;
    lTrim(trim(p));
    TCHAR t[MAX_LEN_LB + 1];
    _tcsncpy_s(t, p, MAX_LEN_LB);
    t[MAX_LEN_LB] = 0;
    wsprintf(buff, _T("%d,%s\r\n"), code, t);
    if(!WRT_TEXT(*pfClear, *pfCrypt, buff)) {
      success = false;
      break;
      }
    }
  if(success) {
    for(int i = 0; i < MAX_GROUP; ++i) {
      COLORREF c = Rows[i]->getColor();
      wsprintf(buff, _T("%d,%d,%d,%d,%d,%d\r\n"), ID_COLOR_1 + i,
          GetRValue(c), GetGValue(c), GetBValue(c),
          Rows[i]->getPtScr(), Rows[i]->getPtPrn());
      if(!WRT_TEXT(*pfCrypt, buff)) {
        success = false;
        break;
        }
      }
    }
  while(success) {
    success = false;
    int h = 16;
    GET_INT(IDC_EDIT_GR_HFONT, h);
    int style = 0;
    if(IS_CHECKED(IDC_CHECK_GR_ITALIC))
      style |= fITALIC;
    if(IS_CHECKED(IDC_CHECK_GR_BOLD))
      style |= fBOLD;
    if(IS_CHECKED(IDC_CHECK_GR_UNDERL))
      style |= fUNDERLINED;

    GET_TEXT(IDC_EDIT_GR_NAMEFONT, buff);

    TCHAR buff2[500];
    wsprintf(buff2, _T("%d,%d,%d,%s\r\n"), ID_FONT_LISTBOX, h, style, buff);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;


//    GET_TEXT(IDC_EDIT_GR_TREND_NAME, buff);
    GET_TEXT(IDC_COMBO_GR_TREND_NAME, buff);
    wsprintf(buff2, _T("%d,%s\r\n"), ID_TREND_NAME, buff);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    GET_TEXT(IDC_EDIT_GR_FILE_RANGE, buff);
    wsprintf(buff2, _T("%d,%s\r\n"), ID_FILE_RANGE_PATH, buff);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    GET_TEXT(IDC_EDIT_GR_PATH, buff);
    translateFromCRNL(buff2, buff);
    wsprintf(buff, _T("%d,%s\r\n"), ID_FILE_TREND_PATH, buff2);
    if(!WRT_TEXT(*pfCrypt, buff))
      break;

    h = IS_CHECKED(IDC_CHECK_GR_HISTORY);
    wsprintf(buff2, _T("%d,%d\r\n"), ID_FILE_USE_HISTORY, h);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    h = IS_CHECKED(IDC_CHECK_GR_LOCAL);
    wsprintf(buff2, _T("%d,%d\r\n"), ID_USE_LOCAL_PATH_FOR_TREND, h);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    int preview = IS_CHECKED(IDC_CHECK_GR_PREVIEW);
    int setup = IS_CHECKED(IDC_CHECK_GR_SETUP);
    wsprintf(buff2, _T("%d,%d,%d\r\n"), ID_PRINT_TREND_PREVIEW, preview, setup);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    int row = 0;
    GET_INT(IDC_EDIT_GR_N_ROW_LINE, row);
    if(row) {
      int type = IS_CHECKED(IDC_CHECK_GR_LINE_TYPE);
      int round = 0;
      GET_INT(IDC_EDIT_GR_ROUND_LINE, round);
      wsprintf(buff2, _T("%d,%d,%d,%d\r\n"), ID_SET_TYPE_ROW, type, row, round);
      if(!WRT_TEXT(*pfCrypt, buff2))
        break;
      }
    GET_TEXT(IDC_EDIT_GR_TIME_REFRESH, buff);
    if(_ttoi(buff)) {
      wsprintf(buff2, _T("%d,%s\r\n"), ID_WAIT_FIXED_REFRESH, buff);
      if(!WRT_TEXT(*pfCrypt, buff2))
        break;
      }

    int num = 0;
    GET_INT(IDC_EDIT_GR_N_LABEL, num);
    if(num) {
      int dec = 0;
      GET_INT(IDC_EDIT_GR_DEC_LABEL, dec);
      wsprintf(buff2, _T("%d,%d,%d\r\n"), ID_NUM_LABEL_ROW, num, dec);
      if(!WRT_TEXT(*pfCrypt, buff2))
        break;
      }

#if 1
    buff[0] = _T('0');
    buff[1] = 0;
#else
    GET_TEXT(IDC_EDIT_GR_DIM_HEADER, buff);
#endif
    wsprintf(buff2, _T("%d,%s\r\n"), ID_SIZEOF_HEADER, buff);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

#if 1
    buff[0] = _T('8');
    buff[1] = 0;
#else
    GET_TEXT(IDC_EDIT_GR_OFFS_STRUCT, buff);
#endif
    wsprintf(buff2, _T("%d,%s\r\n"), ID_OFFSET_INIT_STAT_FILE, buff);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    h = IS_CHECKED(IDC_CHECK_GR_UNIQUE_RANGE);
    wsprintf(buff2, _T("%d,%d\r\n"), ID_NUM_DATA_STAT_FILE, h);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    int valX = -1;
    int fix = IS_CHECKED(IDC_CHECK_GR_FIX);
    GET_INT(IDC_EDIT_GR_XSCALE, valX);
    int typeFix = SendMessage(GetDlgItem(*this, IDC_COMBO_GR_FIX_TIME), CB_GETCURSEL, 0, 0);

    wsprintf(buff2, _T("%d,%d,%d,%d\r\n"), ID_TREND_X_BASED, valX, fix, typeFix);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    COLORREF c = axText->getColor();
    wsprintf(buff2, _T("%d,%d,%d,%d\r\n"), ID_TREND_AX_COLOR, GetRValue(c), GetGValue(c), GetBValue(c));
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    int psw = SendMessage(GetDlgItem(*this, IDC_COMBO_GR_TIME_PSW), CB_GETCURSEL, 0, 0);
    if(4 == psw)
      psw = 1000;

    wsprintf(buff2, _T("%d,%d\r\n"), ID_TREND_TIME_PSW, psw);
    if(!WRT_TEXT(*pfCrypt, buff2))
      break;

    success = true;
    break;
    }
  delete pfCrypt;
  delete pfClear;

  if(success)
    PModDialog::CmOk();
}
//------------------------------------------------------------------------------
void cust_graph::checkEnableScaleX()
{
  TCHAR buff[50];
  GET_TEXT(IDC_EDIT_GR_XSCALE, buff);
  bool ena1 = false;
  bool ena2 = false;
  if(_ttoi(buff) < 0) {
    ena1 = true;
    ena2 = IS_CHECKED(IDC_CHECK_GR_FIX);
    }
  ENABLE(IDC_CHECK_GR_FIX, ena1);
  ENABLE(IDC_COMBO_GR_FIX_TIME, ena2);
}
//------------------------------------------------------------------------------
void cust_graph::checkEnabletypeRow()
{
  bool ena = IS_CHECKED(IDC_CHECK_GR_LINE_TYPE);
  ENABLE(IDC_EDIT_GR_ROUND_LINE, !ena);
  if(ena)
    SET_TEXT(IDC_STATIC_LINE_TYPE, _T("Intervallo"));
  else
    SET_TEXT(IDC_STATIC_LINE_TYPE, _T("N. righe - Arr."));
}
//------------------------------------------------------------------------------
static void addRow(HWND hlb, uint ix, LPCTSTR txt)
{
  TCHAR buff[MAX_LEN_LB + 1];
  fillStr(buff, _T(' '), MAX_LEN_LB);
  TCHAR t[20];
  LPTSTR p = buff;
  wsprintf(t, _T("%d"), iLb[ix].id);
  copyStr(p, t, _tcslen(t));
  p += LEN_LB_DIM_ID;
  *p++ = _T('\t');

  copyStr(p, iLb[ix].info, _tcslen(iLb[ix].info));
  p += LEN_LB_INFO;
  *p++ = _T('\t');
  if(txt)
    copyStr(p, txt, _tcslen(txt));
  p += LEN_LB_TEXT;
  *p++ = 0;
  SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
}
//------------------------------------------------------------------------------
void cust_graph::fillLB(setOfString& set)
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_GR_TEXT);
  for(uint i = 0; i < SIZE_A(iLb); ++i) {
    LPCTSTR p = set.getString(iLb[i].id);
    addRow(hlb, i, p);
    }
}
//------------------------------------------------------------------------------
void cust_graph::changedCBTrendName()
{
  HWND hwCB = GetDlgItem(*this, IDC_COMBO_GR_TREND_NAME);
  int sel = SendMessage(hwCB, CB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  PVect<infoTrendName*> nameSet;
  do {
    setOfString set;
    makeStdMsgSet(set);
    getAllTrendName(nameSet, set);
    } while(false);
  uint nElem = nameSet.getElem();
  TCHAR name[_MAX_PATH];
  GET_TEXT(IDC_COMBO_GR_TREND_NAME, name);
  SET_TEXT(IDC_EDIT_GR_PATH, _T(""));
  SET_CHECK_SET(IDC_CHECK_GR_HISTORY, false);
  for(uint i = 0; i < nElem; ++i) {
    if(!_tcsicmp(name, nameSet[i]->name)) {
      SET_TEXT(IDC_EDIT_GR_PATH, nameSet[i]->match);
      SET_CHECK_SET(IDC_CHECK_GR_HISTORY, nameSet[i]->history);
      break;
      }
    }
  flushPV(nameSet);
}
//------------------------------------------------------------------------------
void cust_graph::loadData()
{
  setOfString set;
  loadTmpCustom(set);
  int focus = 0;
  for(int i = 0; i < MAX_GROUP; ++i) {
    LPCTSTR p = set.getString(ID_COLOR_1 + i);
    if(p)
      Rows[i]->setAll(p);
    if(Rows[i]->hasFocus())
      focus = i;
    }
  LPCTSTR p = set.getString(ID_FONT_LISTBOX);
  if(p) {
    int h = 16;
    int style = 2;
    _stscanf_s(p, _T("%d,%d"), &h, &style);

    p = findNextParam(p, 2);
    SET_TEXT(IDC_EDIT_GR_NAMEFONT, p);
    SET_INT(IDC_EDIT_GR_HFONT, h);
    if(style & fITALIC)
      SET_CHECK(IDC_CHECK_GR_ITALIC);
    if(style & fBOLD)
      SET_CHECK(IDC_CHECK_GR_BOLD);
    if(style & fUNDERLINED)
      SET_CHECK(IDC_CHECK_GR_UNDERL);
    }

  p = set.getString(ID_TREND_AX_COLOR);
  if(p)
    axText->setAll(p);

  p = set.getString(ID_TREND_TIME_PSW);
  if(p) {
    uint v = _ttoi(p);
    if(v > 3)
      v = 4;
    SendMessage(GetDlgItem(*this, IDC_COMBO_GR_TIME_PSW), CB_SETCURSEL, v, 0);
    }

  HWND hwCB = GetDlgItem(*this, IDC_COMBO_GR_TREND_NAME);
  SendMessage(hwCB, CB_RESETCONTENT, 0, 0);
  PVect<infoTrendName*> nameSet;
  do {
    setOfString set;
    makeStdMsgSet(set);
    getAllTrendName(nameSet, set);
    } while(false);
  uint nElem = nameSet.getElem();
  p = set.getString(ID_TREND_NAME);
  int sel = -1;
  for(uint i = 0; i < nElem; ++i) {
    SendMessage(hwCB, CB_ADDSTRING, 0, (LPARAM)nameSet[i]->name);
    if(p && !_tcsicmp(p, nameSet[i]->name))
      sel = i;
    }
  if(sel >= 0) {
    SendMessage(hwCB, CB_SETCURSEL, sel, 0);
    SET_TEXT(IDC_EDIT_GR_PATH, nameSet[sel]->match);
    SET_CHECK_SET(IDC_CHECK_GR_HISTORY, nameSet[sel]->history);
    }
  flushPV(nameSet);

#if 0
  p = set.getString(ID_FILE_TREND_PATH);
  if(p) {
    TCHAR tmp[_MAX_PATH] = _T("\0");
    translateFromCRNL(tmp, p);
    SET_TEXT(IDC_EDIT_GR_PATH, tmp);
    }
#endif

  p = set.getString(ID_FILE_RANGE_PATH);
  if(p)
    SET_TEXT(IDC_EDIT_GR_FILE_RANGE, p);

  p = set.getString(ID_WAIT_FIXED_REFRESH);
  if(p)
    SET_INT(IDC_EDIT_GR_TIME_REFRESH, _ttoi(p));
#if 0
  p = set.getString(ID_FILE_USE_HISTORY);
  if(p && _ttoi(p))
      SET_CHECK(IDC_CHECK_GR_HISTORY);
#endif
  p = set.getString(ID_USE_LOCAL_PATH_FOR_TREND);
  if(p && _ttoi(p))
      SET_CHECK(IDC_CHECK_GR_LOCAL);

  p = set.getString(ID_PRINT_TREND_PREVIEW);
  if(p) {
    int preview = 1;
    int setup = 0;
    _stscanf_s(p, _T("%d,%d"), &preview, &setup);
    if(preview)
      SET_CHECK(IDC_CHECK_GR_PREVIEW);
    if(setup)
      SET_CHECK(IDC_CHECK_GR_SETUP);
    }

  p = set.getString(ID_SET_TYPE_ROW);
  if(p) {
    int type = 0;
    int row = 0;
    int round = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &type, &row, &round);
    if(type)
      SET_CHECK(IDC_CHECK_GR_LINE_TYPE);
    SET_INT(IDC_EDIT_GR_N_ROW_LINE, row);
    SET_INT(IDC_EDIT_GR_ROUND_LINE, round);
    }

  p = set.getString(ID_NUM_LABEL_ROW);
  if(p) {
    int num = 0;
    int dec = 0;
    _stscanf_s(p, _T("%d,%d"), &num, &dec);
    SET_INT(IDC_EDIT_GR_N_LABEL, num);
    SET_INT(IDC_EDIT_GR_DEC_LABEL, dec);
    }

#if 1
  SET_TEXT(IDC_EDIT_GR_DIM_HEADER, _T("0"));
#else
  p = set.getString(ID_SIZEOF_HEADER);
  if(p)
    SET_TEXT(IDC_EDIT_GR_DIM_HEADER, p);
#endif

#if 1
  SET_TEXT(IDC_EDIT_GR_OFFS_STRUCT, _T("8"));
#else
  p = set.getString(ID_OFFSET_INIT_STAT_FILE);
  if(p)
    SET_TEXT(IDC_EDIT_GR_OFFS_STRUCT, p);
#endif

  p = set.getString(ID_NUM_DATA_STAT_FILE);
  if(p) {
    LPCTSTR p2 = findNextParam(p, 1);
    if(p2)
      p = p2;
    int v = _ttoi(p);
    if(v)
      SET_CHECK(IDC_CHECK_GR_UNIQUE_RANGE);
    }

  p = set.getString(ID_TREND_X_BASED);
  if(p) {
    int valX = -1;
    int fix = 1;
    int typeFix = 2;
    _stscanf_s(p, _T("%d,%d,%d"), &valX, &fix, &typeFix);
    SET_INT(IDC_EDIT_GR_XSCALE, valX);
    if(fix)
      SET_CHECK(IDC_CHECK_GR_FIX);
    SendMessage(GetDlgItem(*this, IDC_COMBO_GR_FIX_TIME), CB_SETCURSEL, typeFix, 0);
    }

  fillLB(set);

  checkEnableScaleX();
  checkEnabletypeRow();

  Rows[focus]->setFocus(IDC_SCROLLBAR_GR_R);
  InvalidateRect(*this, 0, false);
/*
  int pos = currPos;
  int focus = 0;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos + 1);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);

    Rows[i]->setColor(Cfg[pos].realColor);
    Rows[i]->setCode(Cfg[pos].code);
    if(Rows[i]->hasFocus())
      focus = i;
    }
  Rows[focus]->setFocus(IDC_SCROLLBAR_GR_R);
  InvalidateRect(*this, 0, false);
*/
}
//----------------------------------------------------------------------------
HBRUSH cust_graph::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    do {
      PColorBox* box = axText->getBox();
      if(*box == hWndChild) {
        SetBkColor(hdc, box->getColor());
        return box->getBrush();
        }
      } while(false);
    for(int i = 0; i < MAX_GROUP; ++i) {
      PColorBox* box = Rows[i]->getBox();
      if(*box == hWndChild) {
        SetBkColor(hdc, box->getColor());
        return box->getBrush();
        }
      }
    for(uint i = 0; i < SIZE_A(BaseColor); ++i)
      if(*BaseColor[i] == hWndChild) {
        SetBkColor(hdc, BaseColor[i]->getColor());
        return BaseColor[i]->getBrush();
        }
    if(*CurrColor == hWndChild) {
      SetBkColor(hdc, CurrColor->getColor());
      return CurrColor->getBrush();
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
//------------------------------------------------------------------------------
class PD_ModText : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PD_ModText(PWin* parent, LPTSTR target, uint resId = IDD_DIALOG_GR_CHG_TEXT, HINSTANCE hinstance = 0)
        : baseClass(parent, resId, hinstance), Target(target) { new langEdit(this, IDC_EDIT_TEXT); }
    virtual ~PD_ModText() { destroy(); }
  private:
    LPTSTR Target;
  public:
    virtual bool create();
    void CmOk();

  protected:
//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

};
//------------------------------------------------------------------------------
bool PD_ModText::create()
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
void PD_ModText::CmOk()
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
void cust_graph::modifyLB()
{
  HWND hlb = GetDlgItem(*this, IDC_LIST_GR_TEXT);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[MAX_LEN_LB + 1];
  SendMessage(hlb, LB_GETTEXT, sel, (LPARAM)buff);
  if(IDOK == PD_ModText(this, buff).modal()) {
    SendMessage(hlb, LB_DELETESTRING, sel, 0);
    SendMessage(hlb, LB_INSERTSTRING, sel, (LPARAM)buff);
    }
}
//----------------------------------------------------------------------------
