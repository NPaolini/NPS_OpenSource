//--------------- svmPropertyTblHeader.cpp ------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyTblHeader.h"
#include "PStatic.h"
#include "macro_utils.h"
//-----------------------------------------------------------
svmDialogTblHeader::svmDialogTblHeader(svmObject* owner, PropertyTableInfo* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), Prop(prop), bg(0)
{
  bg = CreateSolidBrush(Prop->type2);
}
//-----------------------------------------------------------
svmDialogTblHeader::~svmDialogTblHeader()
{
  destroy();
  if(bg)
    DeleteObject(bg);
}
//-----------------------------------------------------------
bool svmDialogTblHeader::create()
{
  new PStatic(this, IDC_STATIC_SAMPLE);
  uint idcText[] = { IDC_EDIT_COL1, IDC_EDIT_COL2, IDC_EDIT_COL3 };
  for(uint i = 0; i < NCOL_FULL; ++i)
    new langEdit(this, idcText[i]);

  if(!baseClass::create())
    return false;

  uint idcWidth[] = { IDC_EDIT_WCOL1, IDC_EDIT_WCOL2, IDC_EDIT_WCOL3 };
  for(uint i = 0; i < NCOL_FULL; ++i) {
    SET_TEXT(idcText[i], Prop->textHeader[i]);
    SET_INT(idcWidth[i], Prop->Width[i]);
    }
  LPCTSTR align[] = { _T("centro"), _T("sinistra"), _T("destra") };
  HWND hCBx = GetDlgItem(*this, IDC_COMBO_ALIGN);
  for(uint i = 0; i < SIZE_A(align); ++i)
    addStringToComboBox(hCBx, align[i]);
  SendMessage(hCBx, CB_SETCURSEL, Prop->alignText, 0);

  SET_INT(IDC_EDIT_HROW, Prop->HeightHeader);

  uint idCtrl;
  if(Prop->type1 & Property::DN)
    idCtrl = IDC_RADIOBUTTON_DOWN_BORDER;
  else if(Prop->type1 & Property::BORDER)
    idCtrl = IDC_RADIOBUTTON_SIMPLE_BORDER;
  else if(Prop->type1 & Property::UP)
    idCtrl = IDC_RADIOBUTTON_UP_BORDER;
  else
    idCtrl = IDC_RADIOBUTTON_STYLE_NONE;
  SET_CHECK(idCtrl);
  if(Prop->type1 & Property::FILL)
    SET_CHECK(IDC_CHECKBOX_FILL_PANEL);

  svmFont& f = getFontObj();
  const typeFont* tpf = f.getType(Prop->idFont);
  setNameFont(tpf);

  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogTblHeader::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_FG:
        case IDC_BG:
          chooseColor(IDC_FG == LOWORD(wParam));
          break;
        case IDC_FONT:
          chooseFont();
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
HBRUSH svmDialogTblHeader::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  if(GetDlgItem(*this, IDC_STATIC_SAMPLE) != hWndChild)
    return 0;
  SetTextColor(dc, Prop->foreground);
  SetBkColor(dc, Prop->type2);
  return bg;
}
//-----------------------------------------------------------
void svmDialogTblHeader::chooseColor(bool fgnd)
{
  COLORREF c = Prop->type2;
  if(choose_Color(*this, fgnd ? Prop->foreground : c)) {
    if(!fgnd) {
      DeleteObject(bg);
      Prop->type2 = c;
      bg = CreateSolidBrush(c);
      }
    InvalidateRect(GetDlgItem(*this, IDC_STATIC_SAMPLE), 0, 0);
    }
}
//-----------------------------------------------------------
void svmDialogTblHeader::setNameFont(const typeFont* tpf)
{
  ::setNameFont(this, IDC_STATIC_SAMPLE, tpf, true);
}
//-----------------------------------------------------------
void svmDialogTblHeader::chooseFont()
{
  uint idFont = Prop->idFont;
  if(IDOK == svmChooseFont(idFont, this).modal()) {
    svmFont& f = getFontObj();
    const typeFont* tpf = f.getType(idFont);
    setNameFont(tpf);
    Prop->idFont = idFont;
    InvalidateRect(GetDlgItem(*this, IDC_STATIC_SAMPLE), 0, 0);
    }
}
//-----------------------------------------------------------
void svmDialogTblHeader::CmOk()
{
  flushPAV(Prop->textHeader);
  TCHAR t[4096];
  uint idcText[] = { IDC_EDIT_COL1, IDC_EDIT_COL2, IDC_EDIT_COL3 };
  uint idcWidth[] = { IDC_EDIT_WCOL1, IDC_EDIT_WCOL2, IDC_EDIT_WCOL3 };
  for(uint i = 0; i < NCOL_FULL; ++i) {
    t[0] = 0;
    GET_TEXT(idcText[i], t);
    Prop->textHeader[i] = t[0] ? str_newdup(t) : 0;
    GET_INT(idcWidth[i], Prop->Width[i]);
    }
  HWND hCBx = GetDlgItem(*this, IDC_COMBO_ALIGN);
  Prop->alignText = SendMessage(hCBx, CB_GETCURSEL, 0, 0);

  GET_INT(IDC_EDIT_HROW, Prop->HeightHeader);

  Prop->type1 = 0;
  if(IS_CHECKED(IDC_RADIOBUTTON_DOWN_BORDER))
    Prop->type1 = Property::DN;
  else if(IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_BORDER))
    Prop->type1 = Property::BORDER;
  else if(IS_CHECKED(IDC_RADIOBUTTON_UP_BORDER))
    Prop->type1 = Property::UP;

  if(IS_CHECKED(IDC_CHECKBOX_FILL_PANEL))
    Prop->type1 |= Property::FILL;

  baseClass::CmOk();
}
