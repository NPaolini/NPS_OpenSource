//-------- exp_BaseEdit.cpp ------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "exp_BaseEdit.h"
#include "exp_BaseContainer.h"
#include "POwnBtnImageStd.h"
#include <shlwapi.h>
#include <commdlg.h>
#include <limits.h>
#include "p_util.h"
#include  "headerMsg.h"
//-------------------------------------------------------------------
#define RET_EQU(a) if(a != other.a) return false
//-------------------------------------------------------------------
bool infoEdit::operator ==(const infoEdit& other)
{
  RET_EQU(bg);
  for(uint i = 0; i < SIZE_A(fg); ++i) {
    RET_EQU(fg[i]);
    }
  RET_EQU(fontHeight);
  RET_EQU(fontStyle);
  return !_tcsicmp(fontName, other.fontName);
}
//----------------------------------------------------------------------------
InsideEdit::InsideEdit(PWin* parent, uint id,  infoEdit* iedit, bool readonly) :
      baseClass(parent, id, PRect(0, 0, 10, 10), 0, 0), iEdit(*iedit), readOnly(readonly), onFocus(false),
      NumbRow(0), lastY(NOT_VALID_Y)
{
  Attr.style |= ES_NOHIDESEL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL | ES_WANTRETURN | WS_TABSTOP;
  Attr.style &= ~(WS_BORDER | WS_THICKFRAME | WS_DLGFRAME | ES_SUNKEN);
  Attr.exStyle &= ~WS_EX_CLIENTEDGE;

  if(readonly)
    Attr.style |= ES_READONLY;
  enableCapture();
}
//-------------------------------------------------------------------
void InsideEdit::mouseEnter(const POINT& pt, uint flag)
{
  baseClass::mouseEnter(pt, flag);
//  SetFocus(*this);
}
//-------------------------------------------------------------------
#define CHECK_TAB
/*
#ifndef CHECK_TAB
//-------------------------------------------------------------------
int InsideEdit::verifyKey()  { return false; }
#else
//-------------------------------------------------------------------
int InsideEdit::verifyKey()
{
//  if(*this == GetFocus()) {
    if(GetKeyState(VK_TAB)& 0x8000) {
      if(GetKeyState(VK_CONTROL)& 0x8000) {
        click(VK_SPACE);
        click(VK_SPACE);
        return false;
        }
      bool reverse = toBool(GetKeyState(VK_SHIFT)& 0x8000);
      PWin* first;
      if(readOnly && !reverse)
        first = getAppl()->getMainWindow();
      else
      {
        first = getParent();
        while(first) {
          PWin* p = dynamic_cast<PSplitWin*>(first->getParent());
          if(!p)
            break;
          first = p;
          }
        }
      HWND h = GetNextDlgTabItem(*first, *this, reverse);
      h = GetParent(h);
      if(h)
        SetFocus(h);
      }
//    }
  return false;
}
#endif
*/
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define myCF_TEXT CF_UNICODETEXT
#else
  #define myCF_TEXT CF_TEXT
#endif
//-------------------------------------------------------------------
bool InsideEdit::evPaste()
{
  if(!IsClipboardFormatAvailable(myCF_TEXT))
    return false;
  if(!OpenClipboard(*this))
    return false;
  HGLOBAL hglb = GetClipboardData(myCF_TEXT);
  CHARRANGE* cr = 0;
  if(hglb) {
    LPCTSTR lptstr = (LPCTSTR)GlobalLock(hglb);
    cr = new CHARRANGE;
    SendMessage(*this, EM_EXGETSEL, 0, (LPARAM)cr);
    cr->cpMax = cr->cpMin + _tcslen(lptstr);
    SETTEXTEX ste = { ST_SELECTION | ST_KEEPUNDO, 1200 };
    SendMessage(*this, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lptstr);
    GlobalUnlock(hglb);
    }
  CloseClipboard();
  if(cr) {
    PWin* w = getParentWin<exp_BaseContainer>(this);
    if(w)
      PostMessage(*w, WM_MY_CUSTOM, MAKEWPARAM(CM_COLORIZE, 0), (LPARAM)cr);
    }
  return true;
}
//-------------------------------------------------------------------
DEFINE_GUIDXXX(IID_ITextDocument,0x8CC497C0,0xA1DF,0x11CE,0x80,0x98,
                0x00,0xAA,0x00,0x47,0xBE,0x5D);
//-----------------------------------------------------------
static void performUndoRedo(HWND hwnd, bool undo)
{
  IRichEditOle* ireo = 0;
  if(SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&ireo)) {
    ITextDocument* itd;
    if(!FAILED(ireo->QueryInterface(IID_ITextDocument, (LPVOID*)&itd))) {
      long dummy;
      if(undo)
        itd->Undo(4, &dummy);
      else
        itd->Redo(4, &dummy);
      itd->Release();
      }
    ireo->Release();
    }
}
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//----------------------------------------------------------------------------
LRESULT InsideEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PASTE:
      if(!readOnly && evPaste())
        return 0;
      break;
    case WM_RBUTTONUP:
      onMenupopup();
      break;
    case WM_VSCROLL:
      if(GetFocus() != *this)
        SetFocus(*this);
    case WM_MOUSEWHEEL:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      SendMessage(*NumbRow, WM_MY_CUSTOM, MAKEWPARAM(CM_INVALIDATE, 0), 0);
      break;
/*
    case EM_UNDO:
      performUndoRedo(*this, true);
      return 0;
      break;
    case EM_REDO:
      performUndoRedo(*this, false);
      return 0;
      break;
*/
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
bool InsideEdit::checkVertPos()
{
  int pos = 0;
  if(useRichEdit2())
    pos = HIWORD(SendMessage(*this, EM_POSFROMCHAR, 0, 0));
  else {
    POINTL pOffset = {0, 0};
    SendMessage(*this, EM_POSFROMCHAR, (WPARAM)&pOffset, (LPARAM)0);
    pos = pOffset.y;
    }
  if(pos != lastY) {
    lastY = pos;
    InvalidateRect(*NumbRow, 0, 0);
    return true;
    }
  return false;
}
//-------------------------------------------------------------------
#define VK_V 0x56
//-------------------------------------------------------------------
bool InsideEdit::evKeyDown(UINT& key)
{
  if((GetKeyState(VK_SHIFT)& 0x8000) && (GetKeyState(VK_INSERT)& 0x8000) ||
     (GetKeyState(VK_V)& 0x8000) && (GetKeyState(VK_CONTROL)& 0x8000)) {
    PostMessage(*this, WM_PASTE, 0, 0);
    return true;
    }
  switch(key) {
    case VK_RETURN:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
      checkVertPos();
      break;
    case VK_PRIOR:
    case VK_NEXT:
      InvalidateRect(*NumbRow, 0, 0);
      break;
    }
  return false;
}
//-------------------------------------------------------------------
bool InsideEdit::evChar(UINT& key)
{
  return false;
}
//-------------------------------------------------------------------
bool InsideEdit::create()
{
  if(!baseClass::create())
    return false;
  setFormat();
  return true;
}
//-------------------------------------------------------------------
void InsideEdit::setReadOnly(bool set)
{
  readOnly = set;
  SendMessage(*this, EM_SETREADONLY, set, 0);
}
//-------------------------------------------------------------------
void InsideEdit::setFormat()
{
  SendMessage(*this, EM_EXLIMITTEXT, 0, -1);
  CHARFORMAT2 cf;
  ZeroMemory(&cf, sizeof(cf));
  cf.cbSize = sizeof(cf);
  cf.dwMask = CFM_ITALIC | CFM_BOLD | CFM_UNDERLINE;
  if(iEdit.fontStyle & fITALIC)
    cf.dwEffects |= CFE_ITALIC;
  if(iEdit.fontStyle & fBOLD)
    cf.dwEffects |= CFE_BOLD;
  if(iEdit.fontStyle & fUNDERLINED)
    cf.dwEffects |= CFE_UNDERLINE;

  cf.dwMask |= CFM_SIZE | CFM_COLOR | CFM_FACE;
  cf.yHeight = iEdit.fontHeight * 20;
  if(fLIGHT & iEdit.fontStyle) {
    cf.dwMask |= CFM_WEIGHT;
    cf.wWeight = FW_LIGHT;
    }
  else if(fTHIN & iEdit.fontStyle) {
    cf.dwMask |= CFM_WEIGHT;
    cf.wWeight = FW_THIN;
    }

  cf.crTextColor = iEdit.fg[0];
  cf.bCharSet = OEM_CHARSET;
  cf.bPitchAndFamily = DEFAULT_PITCH;
  _tcscpy_s(cf.szFaceName, iEdit.fontName);

  SendMessage(*this, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
  SendMessage(*this, EM_SETBKGNDCOLOR, 0, (LPARAM)iEdit.bg);
}
//-------------------------------------------------------------------
int InsideEdit::calcHeight()
{
  int line1 = SendMessage(*this, EM_LINEINDEX, 0, 0);
  int line2 = SendMessage(*this, EM_LINEINDEX, 1, 0);

  long pos1 = 0;
  long pos2 = 0;
  if(useRichEdit2()) {
    pos1 = HIWORD(SendMessage(*this, EM_POSFROMCHAR, line1, 0));
    pos2 = HIWORD(SendMessage(*this, EM_POSFROMCHAR, line2, 0));
    }
  else {
    POINTL pOffset = {0, 0};
    SendMessage(*this, EM_POSFROMCHAR, (WPARAM)&pOffset, (LPARAM)line1);
    pos1 = pOffset.y;
    SendMessage(*this, EM_POSFROMCHAR, (WPARAM)&pOffset, (LPARAM)line2);
    pos2 = pOffset.y;
    }
  return pos2 - pos1;
}
//-------------------------------------------------------------------
void InsideEdit::setInfo(const infoEdit& ed)
{
  if(iEdit == ed)
    return;
  iEdit = ed;
  NumbRow->freeBkg();
  setFormat();
}
//-------------------------------------------------------------------
void InsideEdit::onMenupopup()
{
  enum idMenu { id1 = 1000, id2, id3, id_x, id4, id5 };
  menuPopup menu[] = {
    { MF_STRING, id1, _T("Copia - CTRL+C") },
    { MF_STRING, id2, _T("Incolla - CTRL+V") },
    { MF_STRING, id3, _T("Taglia - CTRL+X") },
    { MF_SEPARATOR, 0, 0 },
    { MF_STRING, id4, _T("Annulla - CTRL+Z") },
    { MF_STRING, id5, _T("Ripeti - CTRL+Y") },
    };

  CHARRANGE cr;
  SendMessage(*this, EM_EXGETSEL, 0, (LPARAM)&cr);
  if(cr.cpMin == cr.cpMax) {
    menu[id1 - id1].flag |= MF_GRAYED;
    menu[id3 - id1].flag |= MF_GRAYED;
    }
  if(!SendMessage(*this, EM_CANPASTE, myCF_TEXT, 0))
    menu[id2 - id1].flag |= MF_GRAYED;

  if(!SendMessage(*this, EM_CANUNDO, 0, 0))
    menu[id4 - id1].flag |= MF_GRAYED;

  if(!SendMessage(*this, EM_CANREDO, 0, 0))
    menu[id5 - id1].flag |= MF_GRAYED;

  int res = popupMenu(*getParent(), menu, SIZE_A(menu));

  switch(res) {
    case id1:
      SendMessage(*this, WM_COPY, 0, 0);
      break;
    case id2:
      SendMessage(*this, EM_PASTESPECIAL, myCF_TEXT, 0);
      break;
    case id3:
      SendMessage(*this, WM_CUT, 0, 0);
      break;
    case id4:
//      performUndoRedo(*this, true);
      SendMessage(*this, EM_UNDO, 0, 0);
      break;
    case id5:
//      performUndoRedo(*this, false);
      SendMessage(*this, EM_REDO, 0, 0);
      break;
    }
}
//-----------------------------------------------------------
#define MAX_TIMER_ALL (5000)
#define TIMER_TIME    (10)
#define MAX_TIMER_COUNT (MAX_TIMER_ALL / TIMER_TIME)
//----------------------------------------------------------------------------
LRESULT PNumberRow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      if(!hBmpWork)
        makeBkg();
      if(hBmpWork) {
        PAINTSTRUCT Paint;
        HDC hdc = BeginPaint(hwnd, &Paint);
        evPaint(hdc);
        EndPaint(hwnd, &Paint);
        return 0;
        }
      break;
    case WM_ERASEBKGND:
      return 1;
    case WM_SIZE:
      freeBkg();
      break;
    case WM_LBUTTONUP:
      SetFocus(*Ed);
      break;

    case WM_MY_CUSTOM:
      switch(LOWORD(wParam)) {
        case CM_INVALIDATE:
          lastY = Ed->lastY;
          if(!refreshTimer)
            refreshTimer = SetTimer(hwnd, refreshTimer = 100, TIMER_TIME, 0);
          timerCount = 0;
          InvalidateRect(*this, 0, 0);
          break;
        }
      break;
    case WM_TIMER:
      if(refreshTimer == wParam) {
        Ed->checkVertPos();
        if(lastY != Ed->lastY) {
          if(++timerCount >= MAX_TIMER_COUNT) {
            KillTimer(hwnd, refreshTimer);
            refreshTimer = 0;
            }
          InvalidateRect(*this, 0, 0);
          }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void PNumberRow::makeBkg()
{
  freeBkg();
  HDC hdc = GetDC(*this);
  GetClientRect(*this, numberRect);
  hBmpWork = CreateCompatibleBitmap(hdc, numberRect.Width(), numberRect.Height());
  if(hBmpWork) {
    mdcWork = CreateCompatibleDC(hdc);
    oldObj = SelectObject(mdcWork, hBmpWork);
    }
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------
void PNumberRow::freeBkg()
{
  if(hBmpWork) {
    SelectObject(mdcWork, oldObj);
    DeleteDC(mdcWork);
    DeleteObject(hBmpWork);
    hBmpWork = 0;
    letter_height = 0;
    }
}
//-----------------------------------------------------------
#define DEF_COLOR_BKG RGB(245, 245, 245)
//-----------------------------------------------------------
void PNumberRow::evPaint(HDC hdc)
{
  if(!hBmpWork)
    makeBkg();
  HBRUSH br = CreateSolidBrush(DEF_COLOR_BKG);
  PRect r(numberRect);
  r.right -= 5;
  FillRect(mdcWork, r, br);
  DeleteObject(HGDIOBJ(br));

  br = CreateSolidBrush(Ed->iEdit.bg);
  r = numberRect;
  r.left = r.right - 5;
  FillRect(mdcWork, r, br);
  DeleteObject(HGDIOBJ(br));

  drawNumber();
  int width = numberRect.Width();
  int height = numberRect.Height();

  BitBlt(hdc, 0, 0, width, height, mdcWork, 0, 0, SRCCOPY);
}
//-------------------------------------------------------------------
PRect PNumberRow::getRectLine(int currLine)
{
  if(!letter_height)
    letter_height = Ed->calcHeight();
  PRect r(0, 0, 100, letter_height);
  POINTL pOrg = {0, 0};
  long char_index = SendMessage(*Ed, EM_CHARFROMPOS, 0, (LPARAM)&pOrg);
  long start_line = SendMessage(*Ed, EM_EXLINEFROMCHAR, 0, (LPARAM)char_index);
  r.MoveTo(0, (currLine - start_line) * letter_height);
  r.Inflate(0, 25);
  return r;
}
//-------------------------------------------------------------------
void PNumberRow::drawNumber()
{
  int nLine =  SendMessage(*Ed, EM_GETLINECOUNT, 0, 0);
  if(nLine < 2)
    return;
  if(!letter_height)
    letter_height = Ed->calcHeight();

  PRect r(numberRect);
  r.right -= 8;
  POINTL pOrg = {0, 0};
  long char_index = SendMessage(*Ed, EM_CHARFROMPOS, 0, (LPARAM)&pOrg);
  long start_line = SendMessage(*Ed, EM_EXLINEFROMCHAR, 0, (LPARAM)char_index);
  long offset = 0;
  if(useRichEdit2())
    offset = HIWORD(SendMessage(*Ed, EM_POSFROMCHAR, char_index, 0));
  else {
    POINTL pOffset = {0, 0};
    SendMessage(*Ed, EM_POSFROMCHAR, (WPARAM)&pOffset, (LPARAM)char_index);
    offset = pOffset.y;
    }
  POINTL pEnd = {0, numberRect.bottom};
  char_index = SendMessage(*Ed, EM_CHARFROMPOS, 0, (LPARAM)&pEnd);
  long stop_line = SendMessage(*Ed, EM_EXLINEFROMCHAR, 0, (LPARAM)char_index);
  r.Offset(0, offset);
  TCHAR buff[64];
  int oldMode = SetBkMode(mdcWork, TRANSPARENT);
  int oldC = SetTextColor(mdcWork, RGB(130,180,240));
  HFONT fnt = D_FONT(letter_height, 0, Ed->iEdit.fontStyle, Ed->iEdit.fontName);
  HGDIOBJ old = SelectObject(mdcWork, fnt);

  r.bottom = r.top + letter_height;
  for(int i = start_line; i <= stop_line; ++i)  {
    wsprintf(buff, _T("%d"), i + 1);
    DrawText(mdcWork, buff, _tcslen(buff), r, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    r.top += letter_height;
    r.bottom += letter_height;
    }
  DeleteObject(SelectObject(mdcWork, old));
  SetBkMode(mdcWork, oldMode);
}
//-----------------------------------------------------------
int popupMenu(HWND hwnd, menuPopup* item, int num, POINT* pt)
{
  HMENU hmenu = CreatePopupMenu();
  if(!hmenu)
    return 0;
  for(int i = 0; i < num; ++i)
    AppendMenu(hmenu, item[i].flag, item[i].id, item[i].text);
  const uint style = TPM_RETURNCMD | TPM_RIGHTALIGN | TPM_TOPALIGN;
  POINT Pt;
  if(pt)
    Pt = *pt;
  else
    GetCursorPos(&Pt);
  int result = TrackPopupMenu(hmenu, style, Pt.x, Pt.y, 0, hwnd, 0);
  DestroyMenu(hmenu);
  return result;
}
//-------------------------------------------------------------------
#define myWM_CHAR (WM_FW_FIRST_FREE +10)
//----------------------------------------------------------------------------
InsideEditFirst::InsideEditFirst(PWin* parent, uint id,  infoEdit* iedit, bool readonly) :
      baseClass(parent, id, iedit, readonly), onTabSpace(false), idTimer(0)
{
  ZeroMemory(&crSave, sizeof(crSave));
  ZeroMemory(&ptSave, sizeof(ptSave));
}
//-------------------------------------------------------------------
#ifndef CHECK_TAB
//-------------------------------------------------------------------
int InsideEditFirst::verifyKey()  { return false; }
#else
//-------------------------------------------------------------------
int InsideEditFirst::verifyKey()
{
  static bool first = true;
  if(GetKeyState(VK_TAB)& 0x8000) {
    if(GetKeyState(VK_CONTROL)& 0x8000) {
      if(!onTabSpace) {
        onTabSpace = true;
        SetTimer(*this, idTimer = 111, 50, 0);
        }
      return false;
      }
    bool reverse = toBool(GetKeyState(VK_SHIFT)& 0x8000);
    // l'edit delle variabili è il primo ed ha problemi nel passagio del focus dal dialogo dei pulsanti
    if(IDC_EDIT_VARS == Attr.id - ID_EDIT) {
      if(onFocus && !reverse) {
        first = !first;
        if(first) {
          onFocus = false;
          }
        return false;
        }
      }

    PWin* first;
    // qui si fa riferimento al readOnly che vale per l'ultimo pannello, se ce ne sono altri
    // occorre differenziare in altro modo (non va al primo controllo del dialogo dei pulsanti, ma al primo edit)
    if(reverse || readOnly)
      first = getAppl()->getMainWindow();
    else
    {
      first = getParent();
      while(first) {
        PWin* p = dynamic_cast<PSplitWin*>(first->getParent());
        if(!p)
          break;
        first = p;
        }
      }
    HWND h = GetNextDlgTabItem(*first, *this, reverse);
    if(h)
      SetFocus(h);
    }
  return false;
}
#endif
//----------------------------------------------------------------------------
bool InsideEditFirst::evKeyDown(UINT& key)
{
  if(GetKeyState(VK_TAB)& 0x8000 && GetKeyState(VK_CONTROL)& 0x8000)
    return true;
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool InsideEditFirst::evKeyUp(UINT& key)
{
  if(GetKeyState(VK_TAB)& 0x8000 && GetKeyState(VK_CONTROL)& 0x8000)
    return true;
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
void InsideEditFirst::reloadCurr()
{
 SendMessage(*this, EM_EXGETSEL, 0, (LPARAM)&crSave);
 if(IDC_EDIT_VARS == Attr.id - ID_EDIT)
   SendMessage(*this, EM_GETSCROLLPOS, 0, (LPARAM)&ptSave);
}
//----------------------------------------------------------------------------
LRESULT InsideEditFirst::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_VSCROLL:
      if(IDC_EDIT_VARS == Attr.id - ID_EDIT)
        SendMessage(*this, EM_GETSCROLLPOS, 0, (LPARAM)&ptSave);
      break;
    case WM_KILLFOCUS:
      SendMessage(*this, EM_EXGETSEL, 0, (LPARAM)&crSave);
      if(IDC_EDIT_VARS == Attr.id - ID_EDIT)
        SendMessage(*this, EM_GETSCROLLPOS, 0, (LPARAM)&ptSave);
      break;
    case WM_SETFOCUS:
      if(!onFocus) {
        onFocus = true;
        SendMessage(*this, EM_EXSETSEL, 0, (LPARAM)&crSave);
        if(IDC_EDIT_VARS == Attr.id - ID_EDIT)
          SendMessage(*this, EM_SETSCROLLPOS, 0, (LPARAM)&ptSave);
        }
      break;
    case WM_DESTROY:
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      break;
    case WM_TIMER:
      if(idTimer == wParam) {
        if(GetKeyState(VK_TAB)& 0x8000)
          break;
        KillTimer(*this, idTimer);
        }
      else
        break;
      // fall through
    case myWM_CHAR:
      click(VK_SPACE);
      click(VK_SPACE);
      onTabSpace = false;
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
int setRedraw::count = 0;
//----------------------------------------------------------------------------
void setRedraw::off(HWND ed)
{
  if(!count)
    SendMessage(ed, WM_SETREDRAW, FALSE, 0);
  ++count;
}
//----------------------------------------------------------------------------
void setRedraw::on(HWND ed)
{
  if(-- count <= 0) {
    SendMessage(ed, WM_SETREDRAW, TRUE, 0);
    count = 0;
    }
}
