//--------------- svmPropertyChoose.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyChoose.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "language_util.h"
#include "clipboard_control.h"
//-----------------------------------------------------------
void chooseText::clone(const chooseText& other)
{
  if(&other != this)
    setText(other.getText());
}
//-----------------------------------------------------------
void chooseText::setText(LPCTSTR txt)
{
  if(txt != text) {
    delete []text;
    text = str_newdup(txt);
    }
}
//-----------------------------------------------------------
PropertyChoose::PropertyChoose() : useCustomSend(false), useColor(false)
{
  Text[0] = new chooseText(_T("Esempio Scelta"));
  lbColors[0] = GetSysColor(COLOR_WINDOWTEXT);
  lbColors[1] = GetSysColor(COLOR_WINDOW);
  lbColors[2] = GetSysColor(COLOR_HIGHLIGHTTEXT);
  lbColors[3] = GetSysColor(COLOR_HIGHLIGHT);
  alignText |= Property::aMid;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PropertyChoose::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyChoose* po = dynamic_cast<const PropertyChoose*>(&other);
  if(po) {
    if(po != this) {
      do {
        flushPV(Text);
        const setOfPText& oSet = po->getSet();
        int nElem = oSet.getElem();
        Text.setDim(nElem);
        for(int i = 0; i < nElem; ++i)
          Text[i] = new chooseText(*oSet[i]);

        for(int i = 0; i < MAX_LB_COLORS; ++i)
          lbColors[i] = po->lbColors[i];
        } while(false);
      do {
        flushPV(TextColor);

        const setOfPTextColor& oSet = po->getSetColor();
        int nElem = oSet.getElem();
        TextColor.setDim(nElem);
        for(int i = 0; i < nElem; ++i)
          TextColor[i] = new textAndColor(*oSet[i]);
        } while(false);

      useColor = po->usingColor();
      useCustomSend = po->usingCustomSend();

      const alternateTextSet& at = po->getAltObj();
      altText.setFileName(at.getFileName());
      altText.setPrph(at.getPrph());
      altText.setAddr(at.getAddr());
      altText.setDim (at.getDim());
      }
    }
  else {
    const PropertyText* po = dynamic_cast<const PropertyText*>(&other);
    if(po) {
      switch(po->simpleVarText) {
        case PropertyText::var_text_value:
        case PropertyText::var_text_value_text:
          useCustomSend = true;
          break;
        default: 
          useCustomSend = false;
          break;
        }
      flushPV(Text);
      flushPV(TextColor);
      useColor = po->usingColor();
      
      const setOfPTextColor& oSet = po->getSet();
      int nElem = oSet.getElem();
      Text.setDim(nElem);
      if(useCustomSend) {
        if(useColor) {
          TextColor.setDim(nElem);
          for(int i = 0; i < nElem; ++i) 
            TextColor[i] = new textAndColor(0, oSet[i]->fgColor, oSet[i]->bgColor, 0);
          }
        for(int i = 0; i < nElem; ++i) {
          TCHAR t[1024];
          _stprintf_s(t, _T("%0.2f,%s"),  oSet[i]->value, oSet[i]->text);
          Text[i] = new chooseText(t);
          }
        }
      else {
        for(int i = 0; i < nElem; ++i) 
          Text[i] = new chooseText(oSet[i]->text);
        }
        
      const alternateTextSet& at = po->getAltObj();
      altText.setFileName(at.getFileName());
      altText.setPrph(at.getPrph());
      altText.setAddr(at.getAddr());
      altText.setDim (at.getDim());
      }
    }
}
//-----------------------------------------------------------
void PropertyChoose::cloneMinusProperty(const Property& other)
{
  clone(other);
}
//-----------------------------------------------------------
svmDialogChoose::svmDialogChoose(svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(owner, prop, parent, id, hInst)
{ }
//-----------------------------------------------------------
bool svmDialogChoose::create()
{
  if(!baseClass::create())
    return false;

  PropertyChoose* pt = dynamic_cast<PropertyChoose*>(Prop);
  if(pt)
    if(pt->usingColor())
      SET_CHECK(IDC_CHECKBOX_USE_COLORS);

  checkEnable();
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogChoose::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_TEXT:
          chooseText();
          break;
        case IDC_CHECKBOX_USE_COLORS:
          checkEnable();
          break;

        case IDC_BUTTON_TEXT_AND_COLOR:
          chooseTextAndColor();
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogChoose::checkEnable()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_USE_COLORS);
  ENABLE(IDC_BUTTON_TEXT_AND_COLOR, enable);
  ENABLE(IDC_BUTTON_CHOOSE_FG, !enable);
  ENABLE(IDC_BUTTON_CHOOSE_BG, !enable);
}
//-----------------------------------------------------------
void svmDialogChoose::CmOk()
{
  PropertyChoose* pt = dynamic_cast<PropertyChoose*>(tmpProp);
  if(pt) {
    pt->setUseColor(IS_CHECKED(IDC_CHECKBOX_USE_COLORS));
    }
  baseClass::CmOk();
  Prop->alignText |= Property::aMid;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#define DIM_TEXT (40 * 10)
#define DIM_BUFF_TEXT (DIM_TEXT + 4)
//-----------------------------------------------------------
void svmDialogChoose::chooseText()
{
  PropertyChoose* pt = dynamic_cast<PropertyChoose*>(tmpProp);
  if(pt)
    svmChooseText(pt, this).modal();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmDialogChoose::chooseTextAndColor()
{
  PropertyChoose* pt = dynamic_cast<PropertyChoose*>(tmpProp);
  if(pt) {
    COLORREF based[2] = { pt->foreground, pt->background };
    uint useColor = 16;
    setOfPTextColor& set = pt->getSetColor();
    if(!set.getElem())
      set[0] = new textAndColor(0, pt->foreground, pt->background, 0);
    if(IDOK == svmChooseTextAndColor(0, useColor, based,set, this).modal()) {
      if(set.getElem()) {
        pt->foreground = set[0]->fgColor;
        pt->background = set[0]->bgColor;
        updateColor(true);
        updateColor(false);
        }
      }
    }
}
//-----------------------------------------------------------
svmChooseText::svmChooseText(PropertyChoose* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), Prop(prop), enlarged(false), selectedForMod(-1)
{
  LB = new PListBox(this, IDC_LISTBOX_VAR_TEXT);
  int tabs[] = { DIM_TEXT };
  LB->SetTabStop(SIZE_A(tabs), tabs, 0);


  LB->SetColor(Prop->lbColors[0], Prop->lbColors[1]);
  LB->SetColorSel(Prop->lbColors[2], Prop->lbColors[3]);
  new langEdit(this, IDC_EDIT_VAR_TEXT);
}

//-----------------------------------------------------------
svmChooseText::~svmChooseText()
{
  destroy();
}
//-----------------------------------------------------------
bool svmChooseText::create()
{
  if(!baseClass::create())
    return false;
  if(Prop->usingCustomSend())
    SET_CHECK(IDC_CHECK_USE_CUSTOM_SEND);

  fill();
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
  return true;
}
//-----------------------------------------------------------
void svmChooseText::checkAlternate()
{
  uint ids[] = { IDC_COMBO_CHOOSE_FILE_PRPH, IDC_EDIT_CHOOSE_FILE_ADDR, IDC_EDIT_CHOOSE_FILE_LEN };
  bool useVar = IS_CHECKED(IDC_RADIO_CHOOSE_FILE_VAR);
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], useVar);
  ENABLE(IDC_EDIT_CHOOSE_FILE_FIX, !useVar);
}
//-----------------------------------------------------------
LRESULT svmChooseText::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_VAR_TEXT:
          add();
          break;
        case IDC_BUTTON_DEL_VAR_TEXT:
          rem();
          break;
        case IDC_BUTTON_MODIFY_VAR_TEXT:
          mod();
          break;
        case IDC_BUTTON_UP_VAR_TEXT:
        case IDC_BUTTON_DOWN_VAR_TEXT:
          move(IDC_BUTTON_UP_VAR_TEXT == LOWORD(wParam));
          break;

        case IDC_BUTTON_FG_NORM_LB:
          chooseColor(0);
          break;
        case IDC_BUTTON_BG_NORM_LB:
          chooseColor(1);
          break;
        case IDC_BUTTON_FG_SELECT_LB:
          chooseColor(2);
          break;
        case IDC_BUTTON_BG_SELECT_LB:
          chooseColor(3);
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

    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmChooseText::copyToClip()
{
  uint tabs[] = { DIM_TEXT };
  infoClipControl iclb(*LB, tabs, SIZE_A(tabs));
  iclb.calc_dim_buff_and_alloc();
  myClipboard_ListBox clip(this);
  clip.copyFromControl(iclb);
}
//-----------------------------------------------------------
void svmChooseText::pasteFromClip()
{
  uint tabs[] = { DIM_TEXT };
  infoClipControl iclb(*LB, tabs, SIZE_A(tabs));
  iclb.calc_dim_buff_and_alloc();
  myClipboard_ListBox clip(this);
  clip.pasteToControl(iclb);
}
//-----------------------------------------------------------
void svmChooseText::chooseColor(int ix)
{
  if(choose_Color(*this, Prop->lbColors[ix])) {
    LB->SetColor(Prop->lbColors[0], Prop->lbColors[1]);
    LB->SetColorSel(Prop->lbColors[2], Prop->lbColors[3]);
    InvalidateRect(*LB, 0, 0);
    }
}
//-----------------------------------------------------------
void svmChooseText::CmOk()
{
  setOfPText& set = Prop->getSet();
  flushPV(set);

  int count = SendMessage(*LB, LB_GETCOUNT, 0 ,0);
  if(count > 0) {
    set.setDim(count);
    TCHAR buff[DIM_BUFF_TEXT];
    for(int i = 0; i < count; ++i) {
      SendMessage(*LB, LB_GETTEXT, i, (LPARAM)buff);
      set[i] = new chooseText(trim(buff));
      }
    }
  Prop->setUseCustomSend(IS_CHECKED(IDC_CHECK_USE_CUSTOM_SEND));

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
  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmChooseText::fill()
{
  SendMessage(*LB, LB_RESETCONTENT, 0, 0);
  const setOfPText& set = Prop->getSet();
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i)
    add(*set[i]);
#if 0
  // essendo cresciuta la dimensione del testo a causa del multi linguaggio
  // non possiamo più adattare la listbox
  PRect r;
  GetClientRect(*LB, r);
  double width = r.Width();
  width /= DIM_TEXT + 1;
  LB->setPixelWidthChar(width);
  LB->recalcWidth();
#endif
  LB->setIntegralHeight();
}
//-----------------------------------------------------------
extern void calc_WidthLB(bool& enlarged, bool add, PListBox* LB);
//-----------------------------------------------------------
void svmChooseText::enableForMod(bool enable)
{
  uint idcToDisable[] = {
    IDC_LISTBOX_VAR_TEXT, IDC_BUTTON_FG_NORM_LB, IDC_BUTTON_BG_NORM_LB,
    IDC_BUTTON_FG_SELECT_LB, IDC_BUTTON_BG_SELECT_LB, IDC_BUTTON_MODIFY_VAR_TEXT,
    IDC_BUTTON_DEL_VAR_TEXT, IDC_BUTTON_UP_VAR_TEXT, IDC_BUTTON_DOWN_VAR_TEXT,
    IDC_CHECK_USE_CUSTOM_SEND, IDOK, IDCANCEL
    };
  for(uint i = 0; i < SIZE_A(idcToDisable); ++i)
    ENABLE(idcToDisable[i], enable);
}
//-----------------------------------------------------------
void svmChooseText::add()
{
  int len = GET_TEXTLENGHT(IDC_EDIT_VAR_TEXT);
  if(!len)
    return;
  TCHAR buff[DIM_BUFF_TEXT];
  GET_TEXT(IDC_EDIT_VAR_TEXT, buff);
  chooseText ct(buff);
  add(ct, selectedForMod);
  if(selectedForMod >= 0)
    enableForMod(true);
  selectedForMod = -1;
}
//-----------------------------------------------------------
void svmChooseText::rem()
{
  int sel = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[DIM_BUFF_TEXT];
  SendMessage(*LB, LB_GETTEXT, sel, (LPARAM)buff);
  SET_TEXT(IDC_EDIT_VAR_TEXT, trim(buff));

  SendMessage(*LB, LB_DELETESTRING, sel, 0);
//  SendMessage(*LB, LB_SETCURSEL, sel, 0);

  calc_WidthLB(enlarged, false, LB);
  int count = SendMessage(*LB, LB_GETCOUNT, 0, 0);
  if(sel >= count)
    sel = count - 1;
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
  SendMessage(*LB, LB_SETCARETINDEX, sel, 0);
  if(enlarged)
    SendMessage(*LB, WM_VSCROLL, 0, 0);
}
//-----------------------------------------------------------
void svmChooseText::mod()
{
  selectedForMod = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(selectedForMod < 0)
    return;
  enableForMod(false);
  rem();
}
//-----------------------------------------------------------
void svmChooseText::move(bool up)
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

  TCHAR buff[DIM_BUFF_TEXT];
  SendMessage(*LB, LB_GETTEXT, sel, (LPARAM)buff);
  SendMessage(*LB, LB_DELETESTRING, sel, 0);
  sel += up ? -1 : 1;
  SendMessage(*LB, LB_INSERTSTRING, sel, (LPARAM)buff);
  SendMessage(*LB, LB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
void svmChooseText::add(const chooseText& info, int sel)
{
  TCHAR buff[DIM_BUFF_TEXT];
  fillStr(buff, _T(' '), SIZE_A(buff));
  LPTSTR p = buff;
  copyStr(buff, info.getText(), _tcslen(info));
  p += DIM_TEXT;
  *p++ = TAB;
  *p = 0;
  if(sel == -1)
    sel = SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)buff);
  else
    sel = SendMessage(*LB, LB_INSERTSTRING, sel, (LPARAM)buff);

  SendMessage(*LB, LB_SETCURSEL, sel, 0);
  calc_WidthLB(enlarged, true, LB);
}
//-----------------------------------------------------------
