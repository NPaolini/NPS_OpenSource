//--------------- svmPropertyText.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyText.h"
#include "svmPropertyChoose.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "language_util.h"
#include "clipboard_control.h"
//-----------------------------------------------------------
PropertyText::~PropertyText()
{
  delete []Text;
  flushPV(TextColor);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PropertyText::setText(LPCTSTR txt)
{
  if(txt != Text) {
    delete []Text;
    Text = str_newdup(txt);
    }
}
//-----------------------------------------------------------
void PropertyText::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyText* po = dynamic_cast<const PropertyText*>(&other);
  if(po) {
    if(po != this) {
      setText(po->Text);
      flushPV(TextColor);

      const setOfPTextColor& oSet = po->getSet();
      int nElem = oSet.getElem();
      TextColor.setDim(nElem);
      for(int i = 0; i < nElem; ++i)
        TextColor[i] = new textAndColor(*oSet[i]);

      useColor = po->usingColor();
      Tick = po->Tick;
      forTitle = po->forTitle;
      const alternateTextSet& at = po->getAltObj();
      altText.setFileName(at.getFileName());
      altText.setPrph(at.getPrph());
      altText.setAddr(at.getAddr());
      altText.setDim (at.getDim());
      }
    }
  else {
    const PropertyChoose* po = dynamic_cast<const PropertyChoose*>(&other);
    if(po) {
      flushPV(TextColor);
      const setOfPText& oSet = po->getSet();
      int nElem = oSet.getElem();
      TextColor.setDim(nElem);

      // prima creiamo la lista usando i testi e, se usati, i valori
      if(po->usingCustomSend()) {
        for(int i = 0; i < nElem; ++i) {
          LPCTSTR p = oSet[i]->getText();
          float val = (float)_tstof(p);
          p = findNextParamTrim(p);
          TextColor[i] = new textAndColor(p, 0, 0xffffff, val);
          }
        simpleVarText = PropertyText::var_text_value;
        }
      else {
        for(int i = 0; i < nElem; ++i)
          TextColor[i] = new textAndColor(oSet[i]->getText(), 0, 0xffffff, 0);
        simpleVarText = PropertyText::var_text;
        }
      if(nElem)
        setText(TextColor[0]->text);

      // poi la riempiamo con i colori, se presenti
      const setOfPTextColor& oSetCol = po->getSetColor();
      int nElemCol = oSetCol.getElem();
      for(int i = 0; i < nElemCol && i < nElem; ++i) {
        TextColor[i]->fgColor = oSetCol[i]->fgColor;
        TextColor[i]->bgColor = oSetCol[i]->bgColor;
        }

      useColor = po->usingColor();

      Tick = 0;
      forTitle = false;
      const alternateTextSet& at = po->getAltObj();
      altText.setFileName(at.getFileName());
      altText.setPrph(at.getPrph());
      altText.setAddr(at.getAddr());
      altText.setDim (at.getDim());
      }
    }
}
//-----------------------------------------------------------
void PropertyText::cloneMinusProperty(const Property& other)
{
#if 0
  clone(other);
#else
  baseClass::clone(other);
  const PropertyText* po = dynamic_cast<const PropertyText*>(&other);
  if(po && po != this) {
    setText(po->Text);
    useColor = po->usingColor();
    Tick = po->Tick;
    forTitle = po->forTitle;
    }
#endif
}
//-----------------------------------------------------------
svmDialogText::svmDialogText(svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(owner, prop, parent, id, hInst)
{
  new langEditDefCR(this, IDC_EDIT_SIMPLE_TEXT);
}
//-----------------------------------------------------------
bool svmDialogText::create()
{
  if(!baseClass::create())
    return false;

  ENABLE(IDC_CHECK_TITLE_OR_SIMPLE, false);
  PropertyText* pt = dynamic_cast<PropertyText*>(Prop);

  int idCtrl;
  switch(Prop->simpleVarText) {
    case PropertyText::simple:
    default:
      idCtrl = IDC_RADIOBUTTON_SIMPLE_TEXT;
      ENABLE(IDC_CHECK_TITLE_OR_SIMPLE, true);
      if(pt->forTitle) {
        SET_CHECK(IDC_CHECK_TITLE_OR_SIMPLE);
        ENABLE(IDC_EDIT_SIMPLE_TEXT, false);
        }
      break;
    case PropertyText::var_text:
      idCtrl = IDC_RADIOBUTTON_VAR_TEXT;
      break;
    case PropertyText::var_value:
      idCtrl = IDC_RADIOBUTTON_VAR_VALUE;
      break;
    case PropertyText::var_text_value:
      idCtrl = IDC_RADIOBUTTON_VAR_TEXT_VALUE;
      break;
    case PropertyText::var_text_value_text:
      idCtrl = IDC_RADIOBUTTON_VAR_TEXT_VALUE2;
      break;
    }
  SET_CHECK(idCtrl);

  UINT align = Prop->alignText;
  idCtrl = Property::aBottom == (align & Property::aBottom) ? IDC_RADIOBUTTON_A_BOTTOM :
         Property::aMid == (align & Property::aMid) ? IDC_RADIOBUTTON_A_VCENTER :
                                     IDC_RADIOBUTTON_A_TOP;
  SET_CHECK(idCtrl);

  if(pt)
    SET_TEXT(IDC_EDIT_SIMPLE_TEXT, pt->getText());
  checkSimpleOrVar();
  checkTypeVar();
  checkBorder();
  if(pt->getBlinkTime()) {
    SET_CHECK(IDC_CHECK_BLINK);
    SET_INT(IDC_EDIT_BLINK, pt->getBlinkTime());
    }
  checkBlink();
  checkTitle();

  if(IsWindowEnabled(GetDlgItem(*this, IDC_RADIOBUTTON_BASE_10))) {
    switch(Prop->BaseNum) {
      case ebTen:
      default:
        SET_CHECK(IDC_RADIOBUTTON_BASE_10);
        break;
      case ebBinary:
        SET_CHECK(IDC_RADIOBUTTON_BASE_2);
        break;
      case ebHex:
        SET_CHECK(IDC_RADIOBUTTON_BASE_16);
        break;
      }
    }
  else
    SET_CHECK(IDC_RADIOBUTTON_BASE_10);

  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogText::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIOBUTTON_SIMPLE_TEXT:
        case IDC_RADIOBUTTON_VAR_TEXT:
        case IDC_RADIOBUTTON_VAR_VALUE:
        case IDC_RADIOBUTTON_VAR_TEXT_VALUE:
        case IDC_RADIOBUTTON_VAR_TEXT_VALUE2:
          checkSimpleOrVar();
          break;
        case IDC_BUTTON_TEXT_AND_COLOR:
          chooseTextAndColor();
          break;
        case IDC_COMBOBOX_TYPEVAL:
          checkTypeVar();
          break;
        case IDC_RADIOBUTTON_STYLE_NONE:
        case IDC_RADIOBUTTON_SIMPLE_BORDER:
        case IDC_RADIOBUTTON_UP_BORDER:
        case IDC_RADIOBUTTON_DOWN_BORDER:
          checkBorder();
          break;
        case ID_BTN_PERSONALIZE_BORDER:
          if(makeBorderStyle(tmpProp->Border, tmpProp->background, this)) {
            DeleteObject(bg);
            bg = CreateSolidBrush(tmpProp->background);
            InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_BACKGROUND), 0, 0);
            }
          break;
        case IDC_COMBOBOX_VIS_PERIFS:
        case IDC_CHECK_BLINK:
          checkBlink();
          break;
        case IDC_CHECK_TITLE_OR_SIMPLE:
          checkTitle();
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogText::checkBase()
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  int typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  bool enable = false;
  switch(typeVal) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
    case 11:
    case 12:
    case 13:
      enable = true;
      break;
    }
  uint idc[] = { IDC_RADIOBUTTON_BASE_10, IDC_RADIOBUTTON_BASE_2, IDC_RADIOBUTTON_BASE_16 };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
}
//-----------------------------------------------------------
void svmDialogText::CmOk()
{
  HWND hw = GetDlgItem(*this, IDC_EDIT_SIMPLE_TEXT);
  int lenTxt = GetWindowTextLength(hw);
  LPTSTR buff = new TCHAR[lenTxt + 2];
  GetWindowText(hw, buff, lenTxt + 1);
  UINT align = 0;
  PropertyText* pt = dynamic_cast<PropertyText*>(tmpProp);
  if(pt) {
    LPCTSTR old = pt->getText();
    if(!old || _tcscmp(old, buff))
      pt->setText(buff);
    if(IS_CHECKED(IDC_RADIOBUTTON_VAR_TEXT))
      pt->simpleVarText = PropertyText::var_text;
    else if(IS_CHECKED(IDC_RADIOBUTTON_VAR_VALUE))
      pt->simpleVarText = PropertyText::var_value;
    else if(IS_CHECKED(IDC_RADIOBUTTON_VAR_TEXT_VALUE))
      pt->simpleVarText = PropertyText::var_text_value;
    else if(IS_CHECKED(IDC_RADIOBUTTON_VAR_TEXT_VALUE2))
      pt->simpleVarText = PropertyText::var_text_value_text;
    else 
      pt->simpleVarText = PropertyText::simple;

    if(IS_CHECKED(IDC_RADIOBUTTON_A_TOP))
      align = Property::aTop;
    else if(IS_CHECKED(IDC_RADIOBUTTON_A_VCENTER))
      align = Property::aMid;
    else
      align = Property::aBottom;

    pt->forTitle = IS_CHECKED(IDC_CHECK_TITLE_OR_SIMPLE);
    }
  delete []buff;
  pt->setBlinkTime(0);
  if(IS_CHECKED(IDC_CHECK_BLINK)) {
    int tick;
    GET_INT(IDC_EDIT_BLINK, tick);
    pt->setBlinkTime(tick);
    }
  if(IS_CHECKED(IDC_RADIOBUTTON_BASE_2))
    tmpProp->BaseNum = ebBinary;
  else if(IS_CHECKED(IDC_RADIOBUTTON_BASE_16))
    tmpProp->BaseNum = ebHex;
  else
    tmpProp->BaseNum = ebTen;
  baseClass::CmOk();
  if(PropertyText::simple == Prop->simpleVarText) {
    Prop->perif = 0;
    Prop->addr = 0;
    Prop->typeVal = 0;
    Prop->nDec = 0;
    Prop->nBits = 0;
    Prop->offset = 0;
    Prop->normaliz = 0;
    }

  Prop->alignText |= align;
}
//-----------------------------------------------------------
void svmDialogText::checkBlink()
{
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_VIS_PERIFS), CB_GETCURSEL, 0, 0);
  if(sel <= 0) {
    SET_CHECK_SET(IDC_CHECK_BLINK, false);
    ENABLE(IDC_CHECK_BLINK, false);
    }
  else
    ENABLE(IDC_CHECK_BLINK, true);
  bool enable = IS_CHECKED(IDC_CHECK_BLINK);
  ENABLE(IDC_EDIT_BLINK, enable);
}
//-----------------------------------------------------------
void svmDialogText::checkBorder()
{
  bool enable = IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_BORDER);
  ENABLE(ID_BTN_PERSONALIZE_BORDER, enable);
}
//-----------------------------------------------------------
void svmDialogText::checkTypeVar(bool byCheckSimple)
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_TYPEVAL);
  int typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  if(14 == typeVal) {
    SetWindowText(GetDlgItem(*this, IDC_STATIC_NDEC), _T("Len"));
    ENABLE(IDC_EDIT_NBITS, false);
    ENABLE(IDC_EDIT_OFFSET, false);
    }
  else {
    SetWindowText(GetDlgItem(*this, IDC_STATIC_NDEC), _T("N Dec"));
    int n;
    GET_INT(IDC_EDIT_NDEC, n);
    if(n < 0 || n > 25)
      SET_INT(IDC_EDIT_NDEC, 0);
    if(byCheckSimple)
      return;
    ENABLE(IDC_EDIT_NBITS, true);
    ENABLE(IDC_EDIT_OFFSET, true);
    }
  checkBase();
}
//-----------------------------------------------------------
void svmDialogText::checkSimpleOrVar()
{
  bool enable = !IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_TEXT);
  ENABLE(IDC_COMBOBOX_PERIFS, enable);
  ENABLE(IDC_EDIT_ADDR, enable);
  ENABLE(IDC_COMBOBOX_TYPEVAL, enable);
  ENABLE(IDC_EDIT_NBITS, enable);
  ENABLE(IDC_EDIT_OFFSET, enable);
  ENABLE(IDC_CHECKBOX_NEGATIVE, enable);
  ENABLE(IDC_BUTTON_TEXT_AND_COLOR, enable);
  ENABLE(IDC_BUTTON_NORMALIZ, enable);
  ENABLE(IDC_EDIT_NORMALIZ, enable);
  ENABLE(IDC_EDIT_NDEC, enable);
  ENABLE(IDC_CHECK_TITLE_OR_SIMPLE, !enable);
  if(enable) {
    LPCTSTR title;
    if(IS_CHECKED(IDC_RADIOBUTTON_VAR_TEXT)) {
      title = _T("Var Text-Color");
      ENABLE(IDC_BUTTON_NORMALIZ, false);
      ENABLE(IDC_EDIT_NORMALIZ, false);
      ENABLE(IDC_EDIT_NDEC, false);
      }
    else if(IS_CHECKED(IDC_RADIOBUTTON_VAR_TEXT_VALUE) || IS_CHECKED(IDC_RADIOBUTTON_VAR_TEXT_VALUE2)) {
      title = _T("Text and Color");
      ENABLE(IDC_EDIT_NDEC, false);
      }
    else {
      title = _T("Var Value-Color");
      int n;
      GET_INT(IDC_EDIT_NORMALIZ, n);
      if(n < 0)
        SET_INT(IDC_EDIT_NORMALIZ, 0);
      checkTypeVar(true);
      }
    SetWindowText(GetDlgItem(*this, IDC_BUTTON_TEXT_AND_COLOR), title);
    }
  else
    checkTitle();
}
//-----------------------------------------------------------
void svmDialogText::checkTitle()
{
  if(!IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_TEXT))
    return;
  bool enable = IS_CHECKED(IDC_CHECK_TITLE_OR_SIMPLE);
  ENABLE(IDC_EDIT_SIMPLE_TEXT, !enable);
  SET_TEXT(IDC_CHECK_TITLE_OR_SIMPLE, enable ?  _T("Titolo") : _T("Testo semplice"));
}
//-----------------------------------------------------------
class svmListBoxVarColorText : public PListBox
{
  private:
    typedef PListBox baseClass;
  public:
    svmListBoxVarColorText(PWin* parent, uint id = IDC_LISTBOX_FONT_ALLOCATED, int len = 255, HINSTANCE hinst = 0):
       PListBox(parent, id, len, hinst) { }

  protected:
    virtual void paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
};
//------------------------------------------------------------------
void svmListBoxVarColorText::paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(1 == ix) {
    uint item = drawInfo.itemID;
    TCHAR buff[500];
    SendMessage(getHandle(), LB_GETTEXT, item, (LPARAM)buff);
    COLORREF fg;
    COLORREF bg;
    _stscanf_s(buff, _T("%d,%d"), &fg, &bg);

    PRect r2(r);
    r2.Inflate(-1, -2);
    r2.Offset(1, 0);
    HBRUSH hbrush = CreateSolidBrush(bg);
    FillRect(hdc, r2, hbrush);
    int oldBkMode = SetBkMode(hdc, TRANSPARENT);
    COLORREF oldTextColor = SetTextColor(hdc, fg);
    DrawText(hdc, _T("Esempio"), -1, r2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    DeleteObject(hbrush);
    SetTextColor(hdc, oldTextColor);
    SetBkMode(hdc, oldBkMode);
    }
  PListBox::paintCustom(hdc, r, ix, drawInfo);
}
//-----------------------------------------------------------
#define DIM_INFO_COLORS 30
#define DIM_VALUE_COLORS 10
#define DIM_SHOW_COLORS 6
#define DIM_TEXT (40 * 10)
//-----------------------------------------------------------
void svmDialogText::chooseTextAndColor()
{
  PropertyText* pt = dynamic_cast<PropertyText*>(tmpProp);
  if(pt) {
    COLORREF based[2] = { pt->foreground, pt->background };
    uint useColor = pt->usingColor() ? 1 : 0;
    if(pt->typeVal == 1)
      useColor |= 2;
    if(IS_CHECKED(IDC_RADIOBUTTON_VAR_VALUE))
      useColor |= 4;
    else if(IS_CHECKED(IDC_RADIOBUTTON_VAR_TEXT_VALUE) || IS_CHECKED(IDC_RADIOBUTTON_VAR_TEXT_VALUE2))
      useColor |= 33;
    svmChooseTextAndColor(pt, useColor, based, pt->getSet(), this).modal();
    pt->setUseColor(toBool(useColor));
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmChooseTextAndColor::svmChooseTextAndColor(PropertyText* prop, uint& useColorAndBits, COLORREF* baseColor,
    setOfPTextColor& txtColor, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), Prop(prop), txtColor(txtColor), baseColor(baseColor),
            foreground(baseColor[0]), background(baseColor[1]), useColorAndBits(useColorAndBits),
            fg(0), bg(0), lastSel(-1), needSort(false)
{
  LB = new svmListBoxVarColorText(this);
  int tabs[] = { DIM_INFO_COLORS, DIM_SHOW_COLORS, DIM_VALUE_COLORS, DIM_TEXT };
  int show[] = { 0, 1, 1, 1 };
  if(useColorAndBits & (4 + 8)) {
    useColorAndBits |= 1;
    show[3] = 0;
    needSort = true;
    }
  else if(useColorAndBits & 16) {
    show[2] = 0;
    show[3] = 0;
    tabs[1] = DIM_TEXT / 2;
    }
  else if(useColorAndBits & 32) {
    needSort = true;
    }
  else
    show[2] = 0;
  LB->SetTabStop(SIZE_A(tabs), tabs, show);

  LB->SetColor(foreground, background);
  LB->SetColorSel(background, foreground);

  BtnFg = new POwnBtn(this, IDC_BUTTON_FG_VAR_TEXT);
  BtnBg = new POwnBtn(this, IDC_BUTTON_BG_VAR_TEXT);

  invalidateColor(3);
  new langEdit(this, IDC_EDIT_VAR_TEXT);
}

//-----------------------------------------------------------
svmChooseTextAndColor::~svmChooseTextAndColor()
{
  if(fg)
    DeleteObject(fg);
  if(bg)
    DeleteObject(bg);
  destroy();
}
//-----------------------------------------------------------
void moveCtrls(int deltaY, HWND parent, int id)
{
  PRect r;
  HWND child = GetDlgItem(parent, id);
  GetWindowRect(child, r);
  r.Offset(0, deltaY);
  MapWindowPoints(HWND_DESKTOP, parent, (LPPOINT)(LPRECT)r, 2);
  SetWindowPos(child, 0, r.left, r.top, r.Width(), r.Height(), SWP_NOSIZE | SWP_NOZORDER);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::moveUpCtrls(int first)
{
  int ids[] = {
      IDC_EDIT_VAR_TEXT,
      IDC_CHECKBOX_NO_COLORS,
      IDC_STATICTEXT_VALUE_COLOR,
      IDC_EDIT_VAR_TEXT_VALUE,
      IDC_BUTTON_FG_VAR_TEXT,
      IDC_BUTTON_BG_VAR_TEXT,
      IDC_BUTTON_ADD_VAR_TEXT,
      IDC_BUTTON_MOD_VAR_TEXT,
      IDC_BUTTON_DEL_VAR_TEXT,
      IDC_BUTTON_UP_VAR_TEXT,
      IDC_BUTTON_DOWN_VAR_TEXT,
      IDOK,
      IDCANCEL
      };
  PRect rTop;
  GetWindowRect(GetDlgItem(*this, IDC_LISTBOX_VAR_TEXT), rTop);
  PRect rFirst;
  GetWindowRect(GetDlgItem(*this, first), rFirst);
  int delta =  rTop.top - rFirst.top + R__Y(10);

  for(uint i = 0; i < SIZE_A(ids); ++i)
    moveCtrls(delta, *this, ids[i]);
}
//-----------------------------------------------------------
bool svmChooseTextAndColor::create()
{
  if(!baseClass::create())
    return false;
  fill();
  if(useColorAndBits & 32) {
    SET_CHECK_SET(IDC_CHECKBOX_NO_COLORS, false);
    ShowWindow(GetDlgItem(*this, IDC_CHECKBOX_NO_COLORS), SW_HIDE);
    setCaption(_T("Testo e colore variabile dal Valore"));
    }
  else if(useColorAndBits & 16) {
    ShowWindow(GetDlgItem(*this, IDC_CHECKBOX_NO_COLORS), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_EDIT_VAR_TEXT), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_STATICTEXT_VALUE_COLOR), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_EDIT_VAR_TEXT_VALUE), SW_HIDE);

    ShowWindow(GetDlgItem(*this, IDC_UNIQUE_TEXT), SW_HIDE);

    moveUpCtrls(IDC_BUTTON_ADD_VAR_TEXT);
    reduceHeight();
    setCaption(_T("Colore variabile"));
    }
  else if(useColorAndBits & 8) {
    ShowWindow(GetDlgItem(*this, IDC_CHECKBOX_NO_COLORS), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_EDIT_VAR_TEXT), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_BUTTON_FG_VAR_TEXT), SW_HIDE);

    ShowWindow(GetDlgItem(*this, IDC_UNIQUE_TEXT), SW_HIDE);

    moveUpCtrls(IDC_EDIT_VAR_TEXT_VALUE);
    reduceHeight();
    setCaption(_T("Colore variabile dal Valore"));
    }
  else {
    if(!(useColorAndBits & 1))
      SET_CHECK(IDC_CHECKBOX_NO_COLORS);
    if(useColorAndBits & 4) {
      ShowWindow(GetDlgItem(*this, IDC_CHECKBOX_NO_COLORS), SW_HIDE);
      ShowWindow(GetDlgItem(*this, IDC_EDIT_VAR_TEXT), SW_HIDE);

      ShowWindow(GetDlgItem(*this, IDC_UNIQUE_TEXT), SW_HIDE);

      moveUpCtrls(IDC_STATICTEXT_VALUE_COLOR);
      reduceHeight();
      setCaption(_T("Colore variabile dal Valore"));
      }
    else {
      ShowWindow(GetDlgItem(*this, IDC_STATICTEXT_VALUE_COLOR), SW_HIDE);
      ShowWindow(GetDlgItem(*this, IDC_EDIT_VAR_TEXT_VALUE), SW_HIDE);
      }
    }
  enableColor();
  if(needSort) {
    ShowWindow(GetDlgItem(*this, IDC_BUTTON_UP_VAR_TEXT), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_BUTTON_DOWN_VAR_TEXT), SW_HIDE);
    }
  if(Prop) {
    alternateTextSet& altSet = Prop->getAltObj();
    HWND hwnd = GetDlgItem(*this, IDC_COMBO_CHOOSE_FILE_PRPH);
    if(hwnd)
      fillCBPerif(hwnd, altSet.getPrph());
    SET_TEXT(IDC_EDIT_CHOOSE_FILE_FIX, altSet.getFileName());
    SET_INT(IDC_EDIT_CHOOSE_FILE_ADDR, altSet.getAddr());
    SET_INT(IDC_EDIT_CHOOSE_FILE_LEN, altSet.getDim());
    bool useVar = false;
    if(altSet.getPrph())
      useVar = true;
    SET_CHECK_SET(IDC_RADIO_CHOOSE_FILE_FIX, !useVar);
    SET_CHECK_SET(IDC_RADIO_CHOOSE_FILE_VAR, useVar);
    checkAlternate();
    }
  return true;
}
//-----------------------------------------------------------
void svmChooseTextAndColor::reduceHeight()
{
  uint ids[] = { IDC_COMBO_CHOOSE_FILE_PRPH, IDC_EDIT_CHOOSE_FILE_ADDR, IDC_EDIT_CHOOSE_FILE_LEN, IDC_RADIO_CHOOSE_FILE_VAR,
      IDC_RADIO_CHOOSE_FILE_FIX, IDC_EDIT_CHOOSE_FILE_FIX };
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], false);
  PRect r;
  GetWindowRect(*this, r);
  PRect r2;
  GetWindowRect(GetDlgItem(*this, IDC_GROUPBOX9), r2);
  r.bottom = r2.top - 10;
  SetWindowPos(*this, 0, 0, 0, r.Width(), r.Height(), SWP_NOMOVE | SWP_NOZORDER);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::checkAlternate()
{
  if(!Prop)
    return;

  uint ids[] = { IDC_COMBO_CHOOSE_FILE_PRPH, IDC_EDIT_CHOOSE_FILE_ADDR, IDC_EDIT_CHOOSE_FILE_LEN };
  bool useVar = IS_CHECKED(IDC_RADIO_CHOOSE_FILE_VAR);
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], useVar);
  ENABLE(IDC_EDIT_CHOOSE_FILE_FIX, !useVar);
}
//-----------------------------------------------------------
LRESULT svmChooseTextAndColor::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_FG_VAR_TEXT:
        case IDC_BUTTON_BG_VAR_TEXT:
          chooseColor(IDC_BUTTON_FG_VAR_TEXT == LOWORD(wParam));
          break;
        case IDC_BUTTON_ADD_VAR_TEXT:
          add();
          break;
        case IDC_BUTTON_MOD_VAR_TEXT:
          mod();
          break;
        case IDC_BUTTON_DEL_VAR_TEXT:
          rem();
          break;
        case IDC_BUTTON_UP_VAR_TEXT:
        case IDC_BUTTON_DOWN_VAR_TEXT:
          move(IDC_BUTTON_UP_VAR_TEXT == LOWORD(wParam));
          break;
        case IDC_CHECKBOX_NO_COLORS:
          enableColor();
          break;
        case IDC_UNIQUE_TEXT:
          replaceText();
          break;
        case IDC_BUTTON_COPY_TO_CLIP:
          copyToClip();
          break;
        case IDC_BUTTON_PASTE_FROM_CLIP:
          pasteFromClip();
          break;
        case IDC_RADIO_CHOOSE_FILE_FIX:
        case IDC_RADIO_CHOOSE_FILE_VAR:
          checkAlternate();
          break;
        }
      break;

    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
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
void svmChooseTextAndColor::copyToClip()
{
  uint tabs[] = { DIM_INFO_COLORS, DIM_SHOW_COLORS, DIM_VALUE_COLORS, DIM_TEXT };
  infoClipControl iclb(*LB, tabs, SIZE_A(tabs));
  iclb.calc_dim_buff_and_alloc();
  myClipboard_ListBox clip(this);
  clip.copyFromControl(iclb);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::pasteFromClip()
{
  uint tabs[] = { DIM_INFO_COLORS, DIM_SHOW_COLORS, DIM_VALUE_COLORS, DIM_TEXT };
  infoClipControl iclb(*LB, tabs, SIZE_A(tabs));
  iclb.calc_dim_buff_and_alloc();
  myClipboard_ListBox clip(this);
  clip.pasteToControl(iclb);
}
//-----------------------------------------------------------
textAndColor* svmChooseTextAndColor::unformat(int pos)
{
  TCHAR buff[500];
  SendMessage(*LB, LB_GETTEXT, pos, (LPARAM)buff);
  buff[DIM_INFO_COLORS] = 0;
  trim(buff);
  _stscanf_s(buff, _T("%d,%d"), &foreground, &background);
  LPTSTR p = buff + DIM_INFO_COLORS + DIM_SHOW_COLORS + DIM_VALUE_COLORS + 3;
  trim(p);
  textAndColor* ptac = new textAndColor(p);

  buff[DIM_INFO_COLORS] = 0;
  trim(buff);
  _stscanf_s(buff, _T("%d,%d"), &ptac->fgColor, &ptac->bgColor);
  p = buff + DIM_INFO_COLORS + DIM_SHOW_COLORS + 2;
  p[DIM_VALUE_COLORS] = 0;
  trim(p);
  ptac->value = (float)_tstof(p);
  return ptac;
}
//-----------------------------------------------------------
void svmChooseTextAndColor::CmOk()
{
  useColorAndBits = !IS_CHECKED(IDC_CHECKBOX_NO_COLORS);
  flushPV(txtColor);
  int count = SendMessage(*LB, LB_GETCOUNT, 0 ,0);
  if(count > 0) {
    txtColor.setDim(count);
    for(int i = 0; i < count; ++i)
      txtColor[i] = unformat(i);
    }
  if(Prop) {
    alternateTextSet& altSet = Prop->getAltObj();
    bool useVar = IS_CHECKED(IDC_RADIO_CHOOSE_FILE_VAR);
    if(useVar) {
      altSet.setFileName(0);
      HWND hwnd = ::GetDlgItem(*this, IDC_COMBO_CHOOSE_FILE_PRPH);
      uint val = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
      altSet.setPrph(val);
      GET_INT(IDC_EDIT_CHOOSE_FILE_ADDR, val);
      altSet.setAddr(val);
      GET_INT(IDC_EDIT_CHOOSE_FILE_LEN, val);
      altSet.setDim(val);
      }
    else {
      TCHAR t[_MAX_PATH];
      GET_TEXT(IDC_EDIT_CHOOSE_FILE_FIX, t);
      trim(lTrim(t));
      if(*t)
        altSet.setFileName(t);
      else
        altSet.setFileName(0);
      altSet.setPrph(0);
      altSet.setAddr(0);
      altSet.setDim(0);
      }
    }
  baseClass::CmOk();
}
//-----------------------------------------------------------
HBRUSH svmChooseTextAndColor::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  uint idCtl = IDC_EDIT_VAR_TEXT;
  if(useColorAndBits & 4)
    idCtl = IDC_EDIT_VAR_TEXT_VALUE;

  if(GetDlgItem(*this, idCtl) == hWndChild) {
    SetBkColor(dc, background);
    SetTextColor(dc, foreground);
    return bg;
    }
  return 0;
}
//-----------------------------------------------------------
void svmChooseTextAndColor::chooseColor(bool fgnd)
{
  if(choose_Color(*this, fgnd ? foreground : background)) {
    invalidateColor(fgnd ? 1 : 2);
    }
}
//-----------------------------------------------------------
void svmChooseTextAndColor::fill()
{
  SendMessage(*LB, LB_RESETCONTENT, 0, 0);
  int nElem = txtColor.getElem();
  for(int i = 0; i < nElem; ++i)
    add(*txtColor[i]);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::add()
{
  int len = GET_TEXTLENGHT(IDC_EDIT_VAR_TEXT);
  if(!len && !(useColorAndBits & (4 + 8 + 16)))
    return;
  textAndColor tac;
  tac.text = new TCHAR[len + 1];
  GET_P_TEXT(IDC_EDIT_VAR_TEXT, tac.text, len + 1);
  tac.fgColor = foreground;
  tac.bgColor = background;
  TCHAR t[50];
  GET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, t);
  tac.value = (float)_tstof(t);
  add(tac);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::rem()
{
  int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  SendMessage(*LB, LB_DELETESTRING, sel, 0);
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::mod()
{
  lastSel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(lastSel < 0)
    return;
  TCHAR buff[500];
  SendMessage(*LB, LB_GETTEXT, lastSel, (LPARAM)buff);
  SendMessage(*LB, LB_DELETESTRING, lastSel, 0);
  buff[DIM_INFO_COLORS] = 0;
  trim(buff);
  _stscanf_s(buff, _T("%d,%d"), &foreground, &background);
  LPTSTR p = buff + DIM_INFO_COLORS + DIM_SHOW_COLORS + DIM_VALUE_COLORS + 3;
  trim(p);
  SET_TEXT(IDC_EDIT_VAR_TEXT, p);
  p = buff + DIM_INFO_COLORS + DIM_SHOW_COLORS + 2;
  p[DIM_VALUE_COLORS] = 0;
  trim(p);
  SET_TEXT(IDC_EDIT_VAR_TEXT_VALUE, p);
  invalidateColor(3);
  SendMessage(*LB, LB_SETCURSEL, lastSel, 0);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::move(bool up)
{
  int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
  if(!sel && up)
    return;
  int count = SendMessage(*LB, LB_GETCOUNT, 0, 0);
  if(sel == count - 1 && !up)
    return;

  TCHAR buff[500];
  SendMessage(*LB, LB_GETTEXT, sel, (LPARAM)buff);
  SendMessage(*LB, LB_DELETESTRING, sel, 0);
  sel += up ? -1 : 1;
  SendMessage(*LB, LB_INSERTSTRING, sel, (LPARAM)buff);
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::enableColor()
{
  bool enable = !IS_CHECKED(IDC_CHECKBOX_NO_COLORS);
  ENABLE(IDC_BUTTON_FG_VAR_TEXT, enable);
  ENABLE(IDC_BUTTON_BG_VAR_TEXT, enable);
  enable &= !(useColorAndBits & 2);
  ENABLE(IDC_EDIT_VAR_TEXT_VALUE, enable);
}
//-----------------------------------------------------------
#define OFFS_VAL (DIM_INFO_COLORS + DIM_SHOW_COLORS + 2)
//----------------------------------------------------------------------------
#define dPRECISION 0.0000001
//----------------------------------------------------------------------------
#define EQU(v1, v2) (fabs((v1) - (v2)) < dPRECISION)
#define LESS(v1, v2) (((v1) - (v2)) < -dPRECISION)
#define GREAT(v1, v2) (((v1) - (v2)) > dPRECISION)
//-----------------------------------------------------------
inline int cmpRow(LPCTSTR toAdd, LPCTSTR row)
{
  double vAdd = _tstof(toAdd + OFFS_VAL);
  double vCheck = _tstof(row + OFFS_VAL);
  if(GREAT(vAdd, vCheck))
    return 1;
  if(LESS(vAdd, vCheck))
    return -1;
  return 0;
}
//-----------------------------------------------------------
//void svmChooseTextAndColor::addSorted(LPCTSTR toAdd)
static
void addSorted(HWND hlb, LPCTSTR toAdd)
{
  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
  int pos = count;
  for(int i = 0; i < count; ++i) {
    TCHAR buff[500];
    SendMessage(hlb, LB_GETTEXT, i, (LPARAM)buff);
    if(cmpRow(toAdd, buff) < 0) {
      pos = i;
      break;
      }
    }
  int sel = SendMessage(hlb, LB_INSERTSTRING, pos, (LPARAM)toAdd);
  SendMessage(hlb, LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
static
void replaceText(HWND hlb, LPCTSTR toReplace)
{
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);

  for(int i = 0; i < count; ++i) {
    TCHAR buff[500];
    SendMessage(hlb, LB_GETTEXT, 0, (LPARAM)buff);
    uint offs = DIM_INFO_COLORS + DIM_SHOW_COLORS + DIM_VALUE_COLORS + 3;
    _tcscpy_s(buff + offs, SIZE_A(buff) - offs, toReplace);
    SendMessage(hlb, LB_DELETESTRING, 0, 0);
    SendMessage(hlb, LB_INSERTSTRING, -1, (LPARAM)buff);
    }

  SendMessage(hlb, LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::replaceText()
{
  TCHAR buff[500];
  GET_TEXT(IDC_EDIT_VAR_TEXT, buff);
  trim(buff);
  if(!*buff) {
    int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
    if(sel < 0) {
      if(SendMessage(*LB, LB_GETCOUNT, 0, 0) <= 0)
        return;
      sel = 0;
      }
    SendMessage(*LB, LB_GETTEXT, sel, (LPARAM)buff);
    LPTSTR p = buff + DIM_INFO_COLORS + DIM_SHOW_COLORS + DIM_VALUE_COLORS + 3;
    trim(p);
    ::replaceText(*LB, p);
    }
  else
    ::replaceText(*LB, buff);
}
//-----------------------------------------------------------
void svmChooseTextAndColor::add(textAndColor& info)
{
  TCHAR buff[500];
  fillStr(buff, _T(' '), SIZE_A(buff));
  LPTSTR p = buff;
  TCHAR t[50];
  wsprintf(t, _T("%d,%d"), info.fgColor, info.bgColor);
  copyStr(p, t, _tcslen(t));
  p += DIM_INFO_COLORS;
  *p++ = TAB;
  p += DIM_SHOW_COLORS;
  *p++ = TAB;
  _stprintf_s(t, SIZE_A(t), _T("%0.2f"), info.value);
  copyStr(p, t, _tcslen(t));
  p += DIM_VALUE_COLORS;
  *p++ = TAB;
  if(info.text)
    copyStr(p, info.text, _tcslen(info.text));
  p += DIM_TEXT;
  *p++ = TAB;
  *p = 0;

  if(needSort)
    addSorted(*LB, buff);
  else {
    int sel = SendMessage(*LB, LB_INSERTSTRING, lastSel, (LPARAM)buff);
    lastSel = -1;
    SendMessage(*LB, LB_SETCURSEL, sel, 0);
    }
}
//-----------------------------------------------------------
void svmChooseTextAndColor::invalidateColor(DWORD bits)
{
  if(bits & 1) {
    if(fg)
      DeleteObject(fg);
    fg = CreateSolidBrush(foreground);
    }
  if(bits & 2) {
    if(bg)
      DeleteObject(bg);
    bg = CreateSolidBrush(background);
    }
  do {
    POwnBtn::colorRect R = BtnFg->getColorRect();
    R.bkg = foreground;
    R.txt = background;
    BtnFg->setColorRect(R);
    } while(false);

  do {
    POwnBtn::colorRect R = BtnBg->getColorRect();
    R.bkg = background;
    R.txt = foreground;
    BtnBg->setColorRect(R);
    } while(false);

  if(getHandle()) {
    InvalidateRect(*BtnFg, 0, 0);
    InvalidateRect(*BtnBg, 0, 0);
    InvalidateRect(GetDlgItem(*this, IDC_EDIT_VAR_TEXT), 0, 0);
    InvalidateRect(GetDlgItem(*this, IDC_EDIT_VAR_TEXT_VALUE), 0, 0);
    }
}


//-----------------------------------------------------------
alternateTextSet::alternateTextSet() : Filename(0), Prph(0), Addr(0), Dim(0) {}
//-----------------------------------------------------------
alternateTextSet::~alternateTextSet() { delete []Filename; }
//-----------------------------------------------------------
bool alternateTextSet::load(setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  delete Filename;
  Filename = 0;
  bool success = false;
  while(p) {
    Prph = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    if(!Prph)
      Filename = str_newdup(p);
    else {
      Addr = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      Dim = _ttoi(p);
      }
    success = true;
    break;
    }
  return success;
}
//-----------------------------------------------------------
bool alternateTextSet::save(P_File& pf, uint id)
{
  if(!Filename && !Prph)
    return false;
  TCHAR t[_MAX_PATH];
  if(Prph) {
    wsprintf(t, _T("%d,%d,%d,%d\r\n"), id, Prph, Addr, Dim);
    writeStringChkUnicode(pf, t);
    }
  else {
    wsprintf(t, _T("%d,0,"), id);
    writeStringChkUnicode(pf, t);
    translateFromCRNL(t, Filename);
    writeStringChkUnicode(pf, t);
    writeStringChkUnicode(pf, _T("\r\n"));
    }
  return true;
}
