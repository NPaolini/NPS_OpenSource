//------------ alarm_file_Dlg.cpp --------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "alarm_file_Dlg.h"
#include "common.h"
#include "fillAlarmAssoc.h"
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
#define bkgColor2 RGB(0xdf,0xff,0xff)
//----------------------------------------------------------------------------
void gRunGlobalAlarmTExt(PWin* parent)
{
  PD_alarm_file_Dlg* dlg = new PD_alarm_file_Dlg(parent);
  dlg->modal();
  delete dlg;
}
//----------------------------------------------------------------------------
 bool PD_alarm_file_Dlg::retFile;
//----------------------------------------------------------------------------
PD_alarm_file_Dlg::PD_alarm_file_Dlg(PWin* parent, LPTSTR file, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), currPos(0), dirty(false),
    Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
    oldStat(1), Target(file)
{
  ZeroMemory(Filename, sizeof(Filename));
  for(int i = 0; i < MAX_GROUP; ++i)
    Rows[i] = 0;
  ClearAll();
  setFileName(file);

  int idBmp[] = { IDB_OPEN, IDB_SAVE, IDB_FILL, IDB_CLEAR, IDB_DONE };
  int idBtn[] = { IDC_BUTTON_OPEN_ALARM, IDC_BUTTON_SAVE_ALARM, IDC_BUTTON_FILL, IDC_BUTTON_CLEAR, IDC_CLOSE_ALARM };

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
}
//----------------------------------------------------------------------------
PD_alarm_file_Dlg::~PD_alarm_file_Dlg()
{
  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
  flushPV(Bmp);

  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
}
//----------------------------------------------------------------------------
void PD_alarm_file_Dlg::setFileName(LPCTSTR file)
{
  if(file && *file) {
    _tcscpy_s(Filename, file);
    TCHAR t[_MAX_PATH * 2] = _T("Gestione File per Allarmi associati - ");
    _tcscat_s(t, file);
    setCaption(t);
    }
  else
    setCaption(_T("Gestione File per Allarmi associati"));
}
//----------------------------------------------------------------------------
void PD_alarm_file_Dlg::Clear(int from, int to)
{
  dirty = true;
  from = min((uint)from, SIZE_A(Cfg) - 1);
  to = min((uint)to, SIZE_A(Cfg) - 1);
  for(int i = from; i <= to; ++i)
    Cfg[i].clear();
}
//----------------------------------------------------------------------------
bool PD_alarm_file_Dlg::create()
{
  POINT pt = { X_INIT, Y_INIT };
  Rows[0] = new PRow_Assoc_File(this, IDC_EDIT_FIRST_FIELD, pt, -1);

  pt.y += H_EDIT - 1;
  for(int i = 1; i < MAX_GROUP - 1; ++i) {
    Rows[i] = new PRow_Assoc_File(this, IDC_EDIT_FIRST_FIELD + i, pt, 0);
    pt.y += H_EDIT - 1;
    }

  Rows[MAX_GROUP - 1] = new PRow_Assoc_File(this, IDC_EDIT_FIRST_FIELD + MAX_GROUP - 1, pt, 1);

  if(!baseClass::create())
    return false;
  PRect r;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1), r);
  int y = r.top;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_2), r);
  int h = r.top - y;

  PVect<infoPosCtrl> ipc;

  uint idc[] = { IDC_STATIC_VALUE, IDC_STATIC_DESCR };
  for(uint i = 0; i < SIZE_A(idc); ++i) {
    GetWindowRect(GetDlgItem(*this, idc[i]), r);
    ipc[i].x = r.left;
    ipc[i].y = y;
    ipc[i].w = r.Width();
    ipc[i].h = h - 1;
    }

  Rows[0]->offset(ipc);
  for(uint j = 1; j < MAX_GROUP; ++j) {
    GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1 + j), r);
    for(uint i = 0; i < SIZE_A(idc); ++i)
      ipc[i].y = r.top;
    Rows[j]->offset(ipc);
    }

  if(Target) {
    loadFile();
    loadData();
    }
  else
    ShowWindow(GetDlgItem(*this, IDC_CHECK_RET), false);
  setScrollRange();
  resetDirty();
  if(retFile)
    SET_CHECK(IDC_CHECK_RET);
  return true;
}
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//----------------------------------------------------------------------------
LRESULT PD_alarm_file_Dlg::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_MOUSEWHEEL:
      evMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
      return 0;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            return 0;
        case IDC_CLOSE_ALARM:
          if(!saveOnExit())
            return 0;
          CmOk();
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
        case IDC_BUTTON_OPEN_ALARM:
          openAssocFile();
          break;
        case IDC_BUTTON_SAVE_ALARM:
          saveData();
          break;
        case IDC_BUTTON_FILL:
          fill();
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
void PD_alarm_file_Dlg::CmOk()
{
  retFile = IS_CHECKED(IDC_CHECK_RET);
  if(retFile)
    baseClass::CmOk();
  else
    baseClass::CmCancel();
}
//----------------------------------------------------------------------------
static void setProjectPath()
{
  TCHAR path[_MAX_PATH];
  makeSystemFilename(path, _T(""));
  int len = _tcslen(path);
  if(len > 0 && _T('\\') == path[len - 1])
    path[len - 1] = 0;
  SetCurrentDirectory(path);
}
//----------------------------------------------------------------------------
bool openFile(PWin* owner, LPTSTR file)
{
  return IDOK == openGenFile(owner, file, false, ALL_ALARM_ASSOC_EXT).modal();
}
//------------------------------------------------------------------
bool openSaveFile(PWin* owner, LPTSTR file)
{
  return IDOK == openGenFile(owner, file, true, ALL_ALARM_ASSOC_EXT).modal();
}
//----------------------------------------------------------------------------
void PD_alarm_file_Dlg::openAssocFile()
{
  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, Filename);
  if(openFile(this, file)) {
    setFileName(file);
    loadFile();
    loadData();
    }
}
//----------------------------------------------------------------------------
void PD_alarm_file_Dlg::fill()
{
  baseRowAssocFile* tmpCfg = new baseRowAssocFile[MAX_ADDRESSES];
  for(int i = 0; i < MAX_ADDRESSES; ++i)
    tmpCfg[i] = Cfg[i];

  if(IDOK == PD_FillAssocFile(tmpCfg, this).modal())  {
    for(int i = 0; i < MAX_ADDRESSES; ++i)
      Cfg[i] = tmpCfg[i];

    loadData();
    dirty = true;
    }
  delete []tmpCfg;
}
//------------------------------------------------------------------------------
bool PD_alarm_file_Dlg::saveData()
{
  saveCurrData();
  do {
    TCHAR file[_MAX_PATH];
    _tcscpy_s(file, Filename);
    if(openSaveFile(this, file))
      setFileName(file);
    } while(false);
  if(!*Filename)
    return false;
  TCHAR path[_MAX_PATH];
  makeSystemFilename(path, Filename);
  _tcscat_s(path,ALARM_ASSOC_EXT);
  do {
    P_File pf(path);
    pf.appendback();
    } while(false);

  P_File fileClear(path, P_CREAT);

  bool success = fileClear.P_open();
  if(success) {
    LPTSTR buff = new TCHAR[4096];
    for(uint i = 0; i < MAX_ADDRESSES; ++i) {
      if(Cfg[i].assocText[0]) {
        wsprintf(buff, _T("%d,%s\r\n"), Cfg[i].value + 1, Cfg[i].assocText);
        writeStringChkUnicode(fileClear, buff);
        }
      }
    delete []buff;
    resetDirty();
    }
  return success;
}
//------------------------------------------------------------------------------
void PD_alarm_file_Dlg::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    dirty |= Rows[i]->isDirty();
    Cfg[pos].value = Rows[i]->getValue();
    Rows[i]->getDescr(Cfg[pos].assocText, SIZE_A(Cfg[pos].assocText));
    }
}
//------------------------------------------------------------------------------
void PD_alarm_file_Dlg::loadData()
{
  int pos = currPos;

  bool oldDirty = dirty;

  for(int i = 0; i < MAX_GROUP && pos < MAX_ADDRESSES; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos + 1);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);
    Rows[i]->setValue(Cfg[pos].value);
    Rows[i]->setDescr(Cfg[pos].assocText);
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + i), 0, 0);
    Rows[i]->resetDirty();
    }
  dirty = oldDirty;
}
//----------------------------------------------------------------------------
bool PD_alarm_file_Dlg::isRowEmpty(uint pos)
{
  pos += IDC_EDIT_FIRST_FIELD;
  int v = GetDlgItemInt(*this, pos, 0, true);
  if(v)
    return false;
  pos += OFFSET_DESCR;
  TCHAR buff[MAX_TEXT];
  GetWindowText(GetDlgItem(*this, pos), buff, SIZE_A(buff));
  if(*buff)
    return false;
  return true;
}
//----------------------------------------------------------------------------
bool PD_alarm_file_Dlg::saveOnExit()
{
  saveCurrData();
  if(dirty) {
    int choose = MessageBox(*this, _T("Vuoi effettuare il salvataggio?"),
                                   _T("I dati sono stati modificati"), MB_YESNOCANCEL | MB_ICONINFORMATION);
    switch(choose) {
      case IDCANCEL:
        return false;
      case IDYES:
        if(!saveData())
          return false;
        break;
      }
    }
  if(Target)
    _tcscpy_s(Target, MAX_PATH, Filename);
  return true;
}
//----------------------------------------------------------------------------
bool PD_alarm_file_Dlg::loadFile()
{
  if(!*Filename)
    return false;
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, Filename);
  _tcscat_s(path,ALARM_ASSOC_EXT);
  setOfString Set;
  makeSystemSet(Set, path);
  if(!Set.setFirst())
    return false;

  ClearAll();

  for(uint i = 0; i < MAX_ADDRESSES; ++i) {
    DWORD id = Set.getCurrId();
    LPCTSTR p = Set.getCurrString();
    Cfg[i].value = id - 1;
    _tcscpy_s(Cfg[i].assocText, p);
    if(!Set.setNext())
      break;
    }
  dirty = false;
  return true;
}
//------------------------------------------------------------------------------
void PD_alarm_file_Dlg::setScrollRange()
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
bool PD_alarm_file_Dlg::preProcessMsg(MSG& msg)
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
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//-----------------------------------------------------------------------------
void PD_alarm_file_Dlg::evMouseWheel(short delta, short x, short y)
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
//------------------------------------------------------------------------------
void PD_alarm_file_Dlg::evVScrollBar(HWND child, int flags, int pos)
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
HBRUSH PD_alarm_file_Dlg::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
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
    return 0;
    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    if(IDC_EDIT_GOTO_ROW == id) {
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
void PD_alarm_file_Dlg::checkBtn()
{
}
//-----------------------------------------------------------------------------
