//--------------- svmPropertyTblFirstCol.cpp ------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyTblFirstCol.h"
#include "common.h"
#include "PStatic.h"
#include "macro_utils.h"
//-----------------------------------------------------------
svmDialogTblFirstCol::svmDialogTblFirstCol(svmObject* owner, PropertyTableInfo* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), Prop(prop), bgOff(0), bgOn(0)
{
  bgOff = CreateSolidBrush(Prop->infoFirst[0].bg);
  bgOn = CreateSolidBrush(Prop->infoFirst[1].bg);
}
//-----------------------------------------------------------
svmDialogTblFirstCol::~svmDialogTblFirstCol()
{
  destroy();
  if(bgOff)
    DeleteObject(bgOff);
  if(bgOn)
    DeleteObject(bgOn);
}
//-----------------------------------------------------------
bool svmDialogTblFirstCol::create()
{
  new PStatic(this, IDC_STATIC_SAMPLE_FONT);
  new PStatic(this, IDC_STATIC_SAMPLE_COLOR_OFF);
  new PStatic(this, IDC_STATIC_SAMPLE_COLOR_ON);

  new langEdit(this, IDC_EDIT_TEXT_OFF);
  new langEdit(this, IDC_EDIT_TEXT_ON);
  if(!baseClass::create())
    return false;

  LPCTSTR align[] = { _T("centro"), _T("sinistra"), _T("destra") };
  HWND hCBx = GetDlgItem(*this, IDC_COMBO_ALIGN);
  for(uint i = 0; i < SIZE_A(align); ++i)
    addStringToComboBox(hCBx, align[i]);
  SendMessage(hCBx, CB_SETCURSEL, Prop->infoFirst[0].alignText, 0);

  uint idc[] = { IDC_RADIO_SCALE, IDC_RADIO_DISTORCE, IDC_RADIO_NO_REDIM };
  SET_CHECK(idc[Prop->infoFirst[1].alignText]);


  uint idCtrl;
  uint style = Prop->infoFirst[0].style;
  bool useBmp = toBool(style & USE_BMP);
  style &= ~USE_BMP;
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

  style = Prop->infoFirst[1].style;
  if(style & Property::DN)
    idCtrl = IDC_RADIOBUTTON_DOWN_BORDER2;
  else if(style & Property::BORDER)
    idCtrl = IDC_RADIOBUTTON_SIMPLE_BORDER2;
  else if(style & Property::UP)
    idCtrl = IDC_RADIOBUTTON_UP_BORDER2;
  else
    idCtrl = IDC_RADIOBUTTON_STYLE_NONE2;
  SET_CHECK(idCtrl);
  if(style & Property::FILL)
    SET_CHECK(IDC_CHECKBOX_FILL_PANEL3);

  style = Prop->styleShow;
  if(useBmp) {
    SET_CHECK(IDC_RADIO_USE_BMP);
    if(style & 8) {
      SET_CHECK(IDC_CHECK_USE_PANEL);
      style &= ~8;
      }
    SET_TEXT(IDC_EDIT_BMP_OFF, Prop->First[0]);
    SET_TEXT(IDC_EDIT_BMP_ON, Prop->First[1]);
    }
  else {
    SET_CHECK(IDC_RADIO_USE_TEXT);
    SET_TEXT(IDC_EDIT_TEXT_OFF, Prop->First[0]);
    SET_TEXT(IDC_EDIT_TEXT_ON, Prop->First[1]);
    }

  uint idcAct[] = { IDC_RADIO1_HIDE_SHOW, IDC_RADIO1_HIDE_LAMP, IDC_RADIO1_SHOW_LAMP,
                    IDC_RADIO2_SWAP, IDC_RADIO2_HIDE_LAMP, IDC_RADIO2_SHOW_LAMP };
  SET_CHECK(idcAct[style]);

  svmFont& f = getFontObj();
  const typeFont* tpf = f.getType(Prop->infoFirst[0].idFont);
  setNameFont(tpf);

  check();

  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogTblFirstCol::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIO_USE_BMP:
        case IDC_CHECK_USE_PANEL:
        case IDC_RADIO_USE_TEXT:
        case IDC_RADIO1_HIDE_SHOW:
        case IDC_RADIO1_HIDE_LAMP:
        case IDC_RADIO1_SHOW_LAMP:
        case IDC_RADIO2_SWAP:
        case IDC_RADIO2_HIDE_LAMP:
        case IDC_RADIO2_SHOW_LAMP:
          check();
          break;
        case IDC_FG_OFF:
        case IDC_BG_OFF:
        case IDC_FG_ON:
        case IDC_BG_ON:
          chooseColor(LOWORD(wParam));
          break;
        case IDC_FONT:
          chooseFont();
          break;
        case IDC_BTN_OFF:
          openBmpName(IDC_EDIT_BMP_OFF);
          break;
        case IDC_BTN_ON:
          openBmpName(IDC_EDIT_BMP_ON);
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
void svmDialogTblFirstCol::openBmpName(uint idcEdit)
{
  TCHAR file[_MAX_PATH];
  GET_TEXT(idcEdit, file);
  LPTSTR pf = file;
  if(openFileImageWithCopy(*this, pf, false))
    SET_TEXT(idcEdit, file);
}
//-----------------------------------------------------------
void svmDialogTblFirstCol::check()
{
  uint idcDisByText[] = {
      IDC_CHECK_USE_PANEL, IDC_BTN_OFF, IDC_BTN_ON,
      IDC_RADIO_SCALE, IDC_RADIO_DISTORCE, IDC_RADIO_NO_REDIM,
      };

  uint idcDisByBmp[] = {
      IDC_EDIT_TEXT_OFF, IDC_EDIT_TEXT_ON, IDC_COMBO_ALIGN,
      IDC_FONT, IDC_FG_OFF, IDC_FG_ON,
      };
  bool useBmp = IS_CHECKED(IDC_RADIO_USE_BMP);

  for(uint i = 0; i < SIZE_A(idcDisByText); ++i)
    ENABLE(idcDisByText[i], useBmp);

  for(uint i = 0; i < SIZE_A(idcDisByBmp); ++i)
    ENABLE(idcDisByBmp[i], !useBmp);

  uint idcOne[] = {  IDC_RADIO1_HIDE_SHOW, IDC_RADIO1_HIDE_LAMP, IDC_RADIO1_SHOW_LAMP };
  bool one = false;
  for(uint i = 0; i < SIZE_A(idcOne); ++i) {
    if(IS_CHECKED(idcOne[i])) {
      one = true;
      break;
      }
    }
  if(useBmp)
    ENABLE(IDC_BTN_ON, !one);
  else
    ENABLE(IDC_EDIT_TEXT_ON, !one);

  bool alternPanel = IS_CHECKED(IDC_CHECK_USE_PANEL);
  if(useBmp)
    one = !alternPanel;
  uint idcStyle2[] = { IDC_RADIOBUTTON_STYLE_NONE2, IDC_RADIOBUTTON_DOWN_BORDER2, IDC_RADIOBUTTON_SIMPLE_BORDER2,
        IDC_RADIOBUTTON_UP_BORDER2, IDC_CHECKBOX_FILL_PANEL3, IDC_BG_ON };

  for(uint i = 0; i < SIZE_A(idcStyle2); ++i)
    ENABLE(idcStyle2[i], !one);
  if(!useBmp)
    ENABLE(IDC_FG_ON, !one);
}
//-----------------------------------------------------------
HBRUSH svmDialogTblFirstCol::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  if(GetDlgItem(*this, IDC_STATIC_SAMPLE_COLOR_OFF) == hWndChild || GetDlgItem(*this, IDC_STATIC_SAMPLE_FONT) == hWndChild) {
    SetTextColor(dc, Prop->infoFirst[0].fg);
    SetBkColor(dc, Prop->infoFirst[0].bg);
    return bgOff;
    }
  if(GetDlgItem(*this, IDC_STATIC_SAMPLE_COLOR_ON) == hWndChild) {
    SetTextColor(dc, Prop->infoFirst[1].fg);
    SetBkColor(dc, Prop->infoFirst[1].bg);
    return bgOn;
    }
  return 0;
}
//-----------------------------------------------------------
void svmDialogTblFirstCol::chooseColor(uint idc)
{
  uint idcTarget;
  COLORREF* col = 0;
  HBRUSH* hbr = 0;
  switch(idc) {
    case IDC_FG_OFF:
      idcTarget = IDC_STATIC_SAMPLE_COLOR_OFF;
      col = &Prop->infoFirst[0].fg;
      break;
    case IDC_BG_OFF:
      idcTarget = IDC_STATIC_SAMPLE_COLOR_OFF;
      col = &Prop->infoFirst[0].bg;
      hbr = &bgOff;
      break;
    case IDC_FG_ON:
      idcTarget = IDC_STATIC_SAMPLE_COLOR_ON;
      col = &Prop->infoFirst[1].fg;
      break;
    case IDC_BG_ON:
      idcTarget = IDC_STATIC_SAMPLE_COLOR_ON;
      col = &Prop->infoFirst[1].bg;
      hbr = &bgOn;
      break;
    }
  if(choose_Color(*this, *col)) {
    if(hbr) {
      DeleteObject(*hbr);
      *hbr = CreateSolidBrush(*col);
      }
    InvalidateRect(GetDlgItem(*this, idcTarget), 0, 0);
    }
}
//-----------------------------------------------------------
void svmDialogTblFirstCol::chooseFont()
{
  uint idFont = Prop->infoFirst[0].idFont;
  if(IDOK == svmChooseFont(idFont, this).modal()) {
    svmFont& f = getFontObj();
    const typeFont* tpf = f.getType(idFont);
    setNameFont(tpf);
    Prop->infoFirst[0].idFont = idFont;
    PStatic* t = dynamic_cast<PStatic*>(PWin::getWindowPtr(GetDlgItem(*this, IDC_STATIC_SAMPLE_COLOR_OFF)));
    if(t)
      t->setFont(tpf->hFont);
    t = dynamic_cast<PStatic*>(PWin::getWindowPtr(GetDlgItem(*this, IDC_STATIC_SAMPLE_COLOR_ON)));
    if(t)
      t->setFont(tpf->hFont);

    InvalidateRect(GetDlgItem(*this, IDC_STATIC_SAMPLE_FONT), 0, 0);
    }
}
//-----------------------------------------------------------
void svmDialogTblFirstCol::setNameFont(const typeFont* tpf)
{
  ::setNameFont(this, IDC_STATIC_SAMPLE_FONT, tpf, true);
}
//-----------------------------------------------------------
void svmDialogTblFirstCol::CmOk()
{
  flushPAV(Prop->First);
  TCHAR t[4096];
  bool useBmp = IS_CHECKED(IDC_RADIO_USE_BMP);

  Prop->styleShow = 0;
   uint idcAct[] = { IDC_RADIO1_HIDE_SHOW, IDC_RADIO1_HIDE_LAMP, IDC_RADIO1_SHOW_LAMP,
                  IDC_RADIO2_SWAP, IDC_RADIO2_HIDE_LAMP, IDC_RADIO2_SHOW_LAMP };
  for(uint i = 0; i < SIZE_A(idcAct); ++i) {
    if(IS_CHECKED(idcAct[i])) {
      Prop->styleShow = i;
      break;
      }
    }
  if(useBmp) {
    if(IS_CHECKED(IDC_CHECK_USE_PANEL))
      Prop->styleShow |= 8;
    GET_TEXT(IDC_EDIT_BMP_OFF, t);
    Prop->First[0] = *t ? str_newdup(t) : 0;
    GET_TEXT(IDC_EDIT_BMP_ON, t);
    Prop->First[1] = *t ? str_newdup(t) : 0;
    }
  else {
    GET_TEXT(IDC_EDIT_TEXT_OFF, t);
    Prop->First[0] = *t ? str_newdup(t) : 0;
    GET_TEXT(IDC_EDIT_TEXT_ON, t);
    Prop->First[1] = *t ? str_newdup(t) : 0;
    }

  Prop->infoFirst[0].style = useBmp ? USE_BMP : 0;

  if(IS_CHECKED(IDC_RADIOBUTTON_DOWN_BORDER))
    Prop->infoFirst[0].style |= Property::DN;
  else if(IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_BORDER))
    Prop->infoFirst[0].style |= Property::BORDER;
  else if(IS_CHECKED(IDC_RADIOBUTTON_UP_BORDER))
    Prop->infoFirst[0].style |= Property::UP;

  if(IS_CHECKED(IDC_CHECKBOX_FILL_PANEL))
    Prop->infoFirst[0].style |= Property::FILL;

  Prop->infoFirst[1].style = 0;

  if(IS_CHECKED(IDC_RADIOBUTTON_DOWN_BORDER2))
    Prop->infoFirst[1].style |= Property::DN;
  else if(IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_BORDER2))
    Prop->infoFirst[1].style |= Property::BORDER;
  else if(IS_CHECKED(IDC_RADIOBUTTON_UP_BORDER2))
    Prop->infoFirst[1].style |= Property::UP;

  if(IS_CHECKED(IDC_CHECKBOX_FILL_PANEL3))
    Prop->infoFirst[1].style |= Property::FILL;

  HWND hCBx = GetDlgItem(*this, IDC_COMBO_ALIGN);
  Prop->infoFirst[0].alignText = SendMessage(hCBx, CB_GETCURSEL, 0, 0);

  uint idc[] = { IDC_RADIO_SCALE, IDC_RADIO_DISTORCE, IDC_RADIO_NO_REDIM };
  for(uint i = 0; i < SIZE_A(idc); ++i) {
    if(IS_CHECKED(idc[i])) {
      Prop->infoFirst[1].alignText = i;
      break;
      }
    }

  baseClass::CmOk();
}
