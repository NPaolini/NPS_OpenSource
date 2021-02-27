//--------------- svmPropertyListBox.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyListBox.h"
#include "macro_utils.h"
#include "PListBox.h"
//-----------------------------------------------------------
bool svmDialogListBox::create()
{
  ColorItem[0].Btn = new  POwnBtn(this, IDC_BUTTON_FG_NORM);
  ColorItem[0].color = Prop->foreground;

  ColorItem[1].Btn = new  POwnBtn(this, IDC_BUTTON_BG_NORM);
  ColorItem[1].color = Prop->background;

  ColorItem[2].Btn = new  POwnBtn(this, IDC_BUTTON_FG_PRESS);
  ColorItem[2].color = Prop->type1;

  ColorItem[3].Btn = new  POwnBtn(this, IDC_BUTTON_BG_PRESS);
  ColorItem[3].color = Prop->type2;

  if(!baseClass::create())
    return false;

  invalidateColor(0);
  invalidateColor(2);

  checkEnable();
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogListBox::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_FG_NORM:
          chooseColor(0);
          break;
        case IDC_BUTTON_BG_NORM:
          chooseColor(1);
          break;
        case IDC_BUTTON_FG_PRESS:
          chooseColor(2);
          break;
        case IDC_BUTTON_BG_PRESS:
          chooseColor(3);
          break;
        case IDC_CHECKBOX_TRANSP_PANEL:
          checkEnable();
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogListBox::CmOk()
{
  tmpProp->foreground = ColorItem[0].color;
  tmpProp->background = ColorItem[1].color;
  tmpProp->type1 = ColorItem[2].color;
  tmpProp->type2 = ColorItem[3].color;
  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmDialogListBox::chooseColor(int ix)
{
  if(choose_Color(*this, ColorItem[ix].color)) {
    invalidateColor(ix);
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmDialogListBox::invalidateColor(int ix)
{
  int ixDual = (ix & 1) ? ix - 1 : ix + 1;
  if(ixDual < ix) {
    --ix;
    ++ixDual;
    }
  COLORREF fg = ColorItem[ix].color;
  COLORREF bg = ColorItem[ixDual].color;

  POwnBtn::colorRect R = ColorItem[ix].Btn->getColorRect();
  R.bkg = fg;
  R.txt = bg;
  ColorItem[ix].Btn->setColorRect(R);

  R = ColorItem[ixDual].Btn->getColorRect();
  R.txt = fg;
  R.bkg = bg;
  ColorItem[ixDual].Btn->setColorRect(R);

  if(getHandle()) {
    InvalidateRect(*ColorItem[ix].Btn, 0, 0);
    InvalidateRect(*ColorItem[ixDual].Btn, 0, 0);
    }
}
//-----------------------------------------------------------
void svmDialogListBox::checkEnable()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_TRANSP_PANEL);
  ENABLE(IDC_BUTTON_FG_NORM, enable);
  ENABLE(IDC_BUTTON_BG_NORM, enable);
  ENABLE(IDC_BUTTON_FG_PRESS, enable);
  ENABLE(IDC_BUTTON_BG_PRESS, enable);
}
