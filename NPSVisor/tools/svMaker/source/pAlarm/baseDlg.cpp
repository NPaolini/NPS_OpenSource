//------------ baseDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "baseDlg.h"
#include "alarmEditorDlg.h"
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
#define bkgColor2 RGB(0xdf,0xff,0xff)
//----------------------------------------------------------------------------
PD_Base::PD_Base(PWin* parent, uint idPrph, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  idPrph(idPrph), currPos(0), dirty(false),
    Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
    oldStat(1)
{
  int idBmp[] = { IDB_SAVE, IDB_SAVE, IDB_FILL, IDB_CLEAR, IDB_COPY, IDB_DONE };
  int idBtn[] = { IDC_BUTTON_SAVE_ALARM, IDC_BUTTON_SAVE_AS, IDC_BUTTON_FILL, IDC_BUTTON_CLEAR, IDC_BUTTON_COPY, IDC_CLOSE_ALARM };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());
    POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], Bmp[i]);
    if(i < SIZE_A(idBmp) - 1) {
      if(IDB_CLEAR == idBmp[i]) {
        POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0xff, 0x7f, 0), 1, GetSysColor(COLOR_BTNTEXT));
        btn->setColorRect(cr);
        }
      else {
        POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0, 0, 0xff), 1, GetSysColor(COLOR_BTNTEXT));
        btn->setColorRect(cr);
        }
      }
    }
  new PEditCR(this, IDC_EDIT_GOTO_ROW, IDC_BUTTON_GOTO_ROW);
  new PEditCR(this, IDC_EDIT_SEARCH_TEXT, IDC_BUTTON_SEARCH_TEXT);
}
//----------------------------------------------------------------------------
PD_Base::~PD_Base()
{
  destroy();
  flushPV(Bmp);

  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
}
//----------------------------------------------------------------------------
bool PD_Base::create()
{
  if(!baseClass::create())
    return false;
  ENABLE(IDC_BUTTON_COPY, false);
  return true;
}
//----------------------------------------------------------------------------
void PD_Base::setScrollRange()
{
  HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
  if(hsb) {
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nPage = PAGE_SCROLL_LEN;
    si.nMin = 0;
    si.nMax = MAX_ADDRESSES - 1;
    SendMessage(hsb, SBM_SETSCROLLINFO, 1, (LPARAM)&si);
    }
}
//----------------------------------------------------------------------------
bool PD_Base::preProcessMsg(MSG& msg)
{
  switch(msg.message) {
    case WM_KEYDOWN:
      switch(msg.wParam) {
        case VK_NEXT:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
          break;
        case VK_PRIOR:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
          break;
        }
      break;
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
void PD_Base::getInfo(commonInfo& ci)
{
  saveCurrData();
  ci.currRow = currPos;
  GET_INT(IDC_EDIT_GOTO_ROW, ci.currGotoRow);
}
//----------------------------------------------------------------------------
void PD_Base::setInfo(const commonInfo& ci)
{
  if(currPos != ci.currRow) {
    HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
    evVScrollBar(child, SB_THUMBTRACK, ci.currRow);
    }
  SET_INT(IDC_EDIT_GOTO_ROW, ci.currGotoRow);
}
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//-----------------------------------------------------------------------------
void PD_Base::evMouseWheel(short delta, short /*x*/, short /*y*/)
{
  int tD = delta;
  tD *= PAGE_SCROLL_LEN;
  tD /= WHEEL_DELTA;
  uint msg = WM_VSCROLL;
  HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);

  if(IsWindowEnabled(child)) {
    int curr = SendMessage(child, SBM_GETPOS, 0, 0);
    curr -= tD;
    if(curr < 0)
      curr = 0;
    else if(curr > (int)MAX_V_SCROLL)
      curr = MAX_V_SCROLL;
    SendMessage(*this, msg, MAKEWPARAM(SB_THUMBTRACK, curr),(LPARAM)child);
    }
}
//----------------------------------------------------------------------------
LRESULT PD_Base::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CLIENT_WHEEL:
      evMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
      return 0;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            return 0;
        case IDC_CLOSE_ALARM:
          PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(IDC_CLOSE_ALARM, 0), 0);
          break;
        case IDC_BUTTON_CLEAR:
          do {
            static int from = -1;
            static int to = -1;
            bool ok = IDOK == ClearChoose(this, from, to).modal();
            if(ok) {
              if(-1 == from && -1 == to)
                ClearAll();
              else
                Clear(from - 1, to - 1);
              loadData();
              }
            } while(false);
          break;
        case IDC_BUTTON_SAVE_ALARM:
          saveData();
          break;
        case IDC_BUTTON_SAVE_AS:
          PostMessage(*getParent(), WM_CLIENT_REQ, MAKEWPARAM(CM_SAVE_ALL, 0), 0);
          break;
        case IDC_BUTTON_FILL:
          fill();
          break;
        case IDC_BUTTON_COPY:
          Copy();
          break;
        case IDC_BUTTON_GOTO_ROW:
          do {
            DWORD pos;
            GET_INT(IDC_EDIT_GOTO_ROW, pos);
            if(pos)
              --pos;
            HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
            evVScrollBar(child, SB_THUMBTRACK, pos);
            } while(false);
          break;
        case IDC_BUTTON_SEARCH_TEXT:
          searchText();
          break;
        }
      break;

    case WM_CUSTOM_VSCROLL:
      lParam = (LPARAM)GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
      // passa sotto
    case WM_VSCROLL:
      evVScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
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
bool PD_Base::saveOnExit()
{
  if(dirty) {
    TCHAR buff[128];
    wsprintf(buff, _T("I dati sono stati modificati [Alarm %d]"), getId());
    int choose = MessageBox(*this, _T("Vuoi effettuare il salvataggio?"),
                                   buff, MB_YESNOCANCEL | MB_ICONINFORMATION);
    switch(choose) {
      case IDCANCEL:
        return false;
      case IDYES:
        if(!saveData())
          return false;
        break;
      }
    }
  return true;
}
//------------------------------------------------------------------------------
void PD_Base::evVScrollBar(HWND child, int flags, int pos)
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
      pos = MAX_V_SCROLL;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - PAGE_SCROLL_LEN;
      break;
    case SB_PAGERIGHT:
      pos = curr + PAGE_SCROLL_LEN;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if(pos > (int)MAX_V_SCROLL)
    pos = MAX_V_SCROLL;
  SendMessage(child, SBM_SETPOS, pos, true);

  saveCurrData();
  currPos = pos;
  loadData();
}
//------------------------------------------------------------------------------
HBRUSH PD_Base::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  if(WM_CTLCOLORSTATIC == ctlType) {
    for(int i = 0; i < MAX_GROUP; ++i) {
      if(IDC_STATICTEXT_1 + i == id) {
        if(isRowEmpty(i)) {
          SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
          return GetSysColorBrush(COLOR_BTNFACE);
          }
        SetTextColor(hdc, RGB(0, 0, 128));
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
      }
    if(id >= IDC_EDIT_FIRST_FIELD + OFFSET_ASSOC && id < IDC_EDIT_FIRST_FIELD + OFFSET_ASSOC + MAX_GROUP) {
      TCHAR t[64];
      GetWindowText(hWndChild, t, SIZE_A(t));
      if(*t) {
        SetTextColor(hdc, RGB(0, 0, 128));
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
      else {
        SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
        return GetSysColorBrush(COLOR_BTNFACE);
        }
      }
    return 0;
    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    if(IDC_EDIT_GOTO_ROW == id || IDC_EDIT_SEARCH_TEXT == id) {
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
    LONG tId = id - IDC_EDIT_FIRST_FIELD;
    tId %= DEF_OFFSET_CTRL;
    bool isEmpty = isRowEmpty(tId);
    if(!isEmpty) {
      SetTextColor(hdc, RGB(0, 0, 128));
      SetBkColor(hdc, bkgColor);
      return (Brush);
      }

    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    return GetSysColorBrush(COLOR_BTNFACE);
    }
  return 0;
}
//------------------------------------------------------------------------------
void PD_Base::checkBtn()
{
  enum statBtn {
    sAllEnabled = 1,
    sAllDisabled = 2,
    sSaveDisabled = 3,
    };
  int newStat = 0;
  if(dirty)
    newStat = sAllEnabled;
  else {
    PD_alarmEditorDlg* aed = getParentWin<PD_alarmEditorDlg>(this);
    if(aed && aed->isDirty(this))
      newStat = sSaveDisabled;
    else
      newStat = sAllDisabled;
    }

  if(newStat != oldStat) {
    HWND hSave = GetDlgItem(*this, IDC_BUTTON_SAVE);
    HWND hSaveAs = GetDlgItem(*this, IDC_BUTTON_SAVE_AS);
    switch(newStat) {
      case sAllEnabled:
        EnableWindow(hSave, true);
        if(sAllDisabled == oldStat)
          EnableWindow(hSaveAs, true);
        break;

      case sAllDisabled:
        EnableWindow(hSaveAs, false);
        if(sAllEnabled == oldStat)
          EnableWindow(hSave, false);
        break;

      case sSaveDisabled:
        if(sAllEnabled == oldStat)
          EnableWindow(hSave, false);
        else if(sAllDisabled == oldStat)
          EnableWindow(hSaveAs, true);
        break;
      }
    }
  oldStat = newStat;
}
//-----------------------------------------------------------------------------
