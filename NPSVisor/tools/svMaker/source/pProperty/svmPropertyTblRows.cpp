//--------------- svmPropertyTblRows.cpp ------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyTblRows.h"
#include "PStatic.h"
#include "p_param_v.h"
#include "macro_utils.h"
//-----------------------------------------------------------
svmDialogTblRows::svmDialogTblRows(svmObject* owner, PropertyTableInfo* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), Prop(prop), bg(0)
{
  bg = CreateSolidBrush(Prop->infoRows.bg);
}
//-----------------------------------------------------------
svmDialogTblRows::~svmDialogTblRows()
{
  destroy();
  if(bg)
    DeleteObject(bg);
}
//-----------------------------------------------------------
static uint idcEd[] = {
    IDC_EDIT_COL_2_1,  IDC_EDIT_COL_3_1,
    IDC_EDIT_COL_2_2,  IDC_EDIT_COL_3_2,
    IDC_EDIT_COL_2_3,  IDC_EDIT_COL_3_3,
    IDC_EDIT_COL_2_4,  IDC_EDIT_COL_3_4,
    IDC_EDIT_COL_2_5,  IDC_EDIT_COL_3_5,
    IDC_EDIT_COL_2_6,  IDC_EDIT_COL_3_6,
    IDC_EDIT_COL_2_7,  IDC_EDIT_COL_3_7,
    IDC_EDIT_COL_2_8,  IDC_EDIT_COL_3_8,
    IDC_EDIT_COL_2_9,  IDC_EDIT_COL_3_9,
    IDC_EDIT_COL_2_10, IDC_EDIT_COL_3_10,
    IDC_EDIT_COL_2_11, IDC_EDIT_COL_3_11,
    IDC_EDIT_COL_2_12, IDC_EDIT_COL_3_12,
    IDC_EDIT_COL_2_13, IDC_EDIT_COL_3_13,
    IDC_EDIT_COL_2_14, IDC_EDIT_COL_3_14,
    IDC_EDIT_COL_2_15, IDC_EDIT_COL_3_15,
    IDC_EDIT_COL_2_16, IDC_EDIT_COL_3_16,
    };
//-----------------------------------------------------------
bool svmDialogTblRows::create()
{
  for(uint i = 0; i < SIZE_A(idcEd); ++i)
    new langEdit(this, idcEd[i]);
  new PStatic(this, IDC_STATIC_SAMPLE_COLOR_OFF);

  if(!baseClass::create())
    return false;

  SET_INT(IDC_EDIT_H_ROW, Prop->heightRows);

  LPCTSTR align[] = { _T("centro"), _T("sinistra"), _T("destra") };
  HWND hCBx = GetDlgItem(*this, IDC_COMBO_ALIGN);
  for(uint i = 0; i < SIZE_A(align); ++i)
    addStringToComboBox(hCBx, align[i]);
  SendMessage(hCBx, CB_SETCURSEL, Prop->infoRows.alignText, 0);

  uint style = Prop->infoRows.style;
  uint idCtrl;
  if(style & Property::DN)
    idCtrl = IDC_RADIOBUTTON_DOWN_BORDER;
  else if(style & Property::BORDER)
    idCtrl = IDC_RADIOBUTTON_SIMPLE_BORDER;
  else if(style & Property::UP)
    idCtrl = IDC_RADIOBUTTON_UP_BORDER;
  else
    idCtrl = IDC_RADIOBUTTON_STYLE_NONE;
  SET_CHECK(idCtrl);
  if(style & Property::FILL)
    SET_CHECK(IDC_CHECKBOX_FILL_PANEL);

  for(uint i = 0, j = 0; i < SIZE_A(idcEd); i += 2, ++j) {
    SET_TEXT(idcEd[i], Prop->Second[j]);
    SET_TEXT(idcEd[i + 1], Prop->Third[j]);
    }

  svmFont& f = getFontObj();
  const typeFont* tpf = f.getType(Prop->infoRows.idFont);
  setNameFont(tpf);
  SET_CHECK(IDC_RADIO_BY_FIRST_COL);
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogTblRows::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_FG_OFF:
        case IDC_BG_OFF:
          chooseColor(IDC_FG_OFF == LOWORD(wParam));
          break;
        case IDC_FONT:
          chooseFont();
          break;
        case IDC_PASTE:
          copyFromClipboard();
          break;
        case IDC_COPY_TO_CLIP:
          copyToClipboard();
          break;
        case IDC_BUTTON_CLEAR_ALL:
          for(uint i = 0; i < SIZE_A(idcEd); ++i)
            SET_TEXT(idcEd[i], _T(""));
          break;
        }
      break;
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
//-----------------------------------------------------------
HBRUSH svmDialogTblRows::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  if(GetDlgItem(*this, IDC_STATIC_SAMPLE_COLOR_OFF) != hWndChild)
    return 0;
  SetTextColor(dc, Prop->infoRows.fg);
  SetBkColor(dc, Prop->infoRows.bg);
  return bg;
}
//-----------------------------------------------------------
void svmDialogTblRows::chooseColor(bool fgnd)
{
  if(choose_Color(*this, fgnd ? Prop->infoRows.fg : Prop->infoRows.bg)) {
    if(!fgnd) {
      DeleteObject(bg);
      bg = CreateSolidBrush(Prop->infoRows.bg);
      }
    InvalidateRect(GetDlgItem(*this, IDC_STATIC_SAMPLE_COLOR_OFF), 0, 0);
    }
}
//-----------------------------------------------------------
void svmDialogTblRows::setNameFont(const typeFont* tpf)
{
  ::setNameFont(this, IDC_STATIC_SAMPLE_COLOR_OFF, tpf, true);
}
//-----------------------------------------------------------
void svmDialogTblRows::chooseFont()
{
  uint idFont = Prop->infoRows.idFont;
  if(IDOK == svmChooseFont(idFont, this).modal()) {
    svmFont& f = getFontObj();
    const typeFont* tpf = f.getType(idFont);
    setNameFont(tpf);
    Prop->infoRows.idFont = idFont;
    InvalidateRect(GetDlgItem(*this, IDC_STATIC_SAMPLE_COLOR_OFF), 0, 0);
    }
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
static
LPTSTR get_line(LPTSTR buff, int len)
{
  if(len < 0)
    return 0;
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(; i < len; ++i, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
void svmDialogTblRows::fillRowByClipboard(LPTSTR lptstr)
{
  int offs = 0;
  int dim = _tcslen(lptstr);
  bool fullCol = IS_CHECKED(IDC_RADIO_BY_FIRST_COL);
  int initRow = 0;
  GET_INT(IDC_EDIT_BY_ROW, initRow);
  if(initRow > 0)
    --initRow;
  if(fullCol) {
    for(int j = initRow * 2; lptstr && j < NROW_FULL * 2; j += 2) {
      LPTSTR next = get_line(lptstr, dim - offs);
      pvvChar target;
      uint elem = splitParam(target, lptstr, _T('\t'));
      SET_TEXT(idcEd[j], &target[0]);
      if(elem > 1)
        SET_TEXT(idcEd[j + 1], &target[1]);
      int len = next ? next - lptstr : dim - offs;
      offs += len;
      lptstr = next;
      }
    }
  else {
    for(int j = initRow * 2 + 1; lptstr && j < NROW_FULL * 2; j += 2) {
      LPTSTR next = get_line(lptstr, dim - offs);
      pvvChar target;
      uint elem = splitParam(target, lptstr, _T('\t'));
      SET_TEXT(idcEd[j], &target[0]);
      int len = next ? next - lptstr : dim - offs;
      offs += len;
      lptstr = next;
      }
    }
}
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define myCF_TEXT CF_UNICODETEXT
#else
  #define myCF_TEXT CF_TEXT
#endif
//----------------------------------------------------------------------------
void svmDialogTblRows::copyFromClipboard()
{
  if(!IsClipboardFormatAvailable(myCF_TEXT))
    return;
  if(!OpenClipboard(*this))
    return;
  HGLOBAL hglb = GetClipboardData(myCF_TEXT);
  uint ix = 0;
  while(hglb) {
    LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
    if(!lptstr)
      break;
    fillRowByClipboard(lptstr);
    GlobalUnlock(hglb);
    break;
    }
  CloseClipboard();
}
//----------------------------------------------------------------------------
void svmDialogTblRows::copyToClipboard()
{
  if(!OpenClipboard(*this))
    return;
  EmptyClipboard();

  int dim = 2;
  for(int i = 0; i < NROW_FULL * 2; ++i)
    dim += GetWindowTextLength(GetDlgItem(*this, idcEd[i])) + 1;
  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (dim + 2) * sizeof(TCHAR));
  if(hglbCopy) {
    LPTSTR b = (LPTSTR)GlobalLock(hglbCopy);
    LPTSTR pb = b;
    TCHAR buff[4096];
    for(int i = 0; i < NROW_FULL * 2; ++i) {
      GET_TEXT(idcEd[i], buff);
      if(i & 1)
        _tcscat_s(buff, _T("\n"));
      else
        _tcscat_s(buff, _T("\t"));
      _tcscpy_s(pb, dim - (pb - b), buff);
      pb += _tcslen(pb);
      }
    GlobalUnlock(hglbCopy);
    SetClipboardData(myCF_TEXT, hglbCopy);
    }
  CloseClipboard();
}
//-----------------------------------------------------------
void svmDialogTblRows::CmOk()
{
  GET_INT(IDC_EDIT_H_ROW, Prop->heightRows);

  HWND hCBx = GetDlgItem(*this, IDC_COMBO_ALIGN);
  Prop->infoRows.alignText = SendMessage(hCBx, CB_GETCURSEL, 0, 0);

  Prop->infoRows.style = 0;
  if(IS_CHECKED(IDC_RADIOBUTTON_DOWN_BORDER))
    Prop->infoRows.style = Property::DN;
  else if(IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_BORDER))
    Prop->infoRows.style = Property::BORDER;
  else if(IS_CHECKED(IDC_RADIOBUTTON_UP_BORDER))
    Prop->infoRows.style = Property::UP;

  if(IS_CHECKED(IDC_CHECKBOX_FILL_PANEL))
    Prop->infoRows.style |= Property::FILL;
  flushPAV(Prop->Second);
  flushPAV(Prop->Third);
  TCHAR t[4096];
  for(uint i = 0, j = 0; i < SIZE_A(idcEd); i += 2, ++j) {
    GET_TEXT(idcEd[i], t);
    Prop->Second[j] = *t ? str_newdup(t) : 0;

    GET_TEXT(idcEd[i + 1], t);
    Prop->Third[j] = *t ? str_newdup(t) : 0;
    }

  baseClass::CmOk();
}
//-----------------------------------------------------------
