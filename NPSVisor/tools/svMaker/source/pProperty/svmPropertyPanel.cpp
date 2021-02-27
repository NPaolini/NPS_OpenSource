//--------------- svmPropertyPanel.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyPanel.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "PTextPanelStatic.h"
//-----------------------------------------------------------
// N.B. type1 è usato per il time del blink
//-----------------------------------------------------------
bool svmDialogPanel::create()
{
  if(!baseClass::create())
    return false;

  ShowWindow(GetDlgItem(*this, IDC_CHECKBOX_TRANSP_PANEL), SW_HIDE);
  check();
  if(Prop->type1) {
    SET_CHECK(IDC_CHECK_BLINK);
    SET_INT(IDC_EDIT_BLINK, Prop->type1);
    }
  checkBlink();
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogPanel::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECKBOX_FILL_PANEL:
        case IDC_RADIOBUTTON_STYLE_NONE:
        case IDC_RADIOBUTTON_SIMPLE_BORDER:
        case IDC_RADIOBUTTON_UP_BORDER:
        case IDC_RADIOBUTTON_DOWN_BORDER:
          check();
          break;
        case ID_BTN_PERSONALIZE_BORDER:
          makeBorderStyle(tmpProp->Border, tmpProp->background, this);
          break;
        case IDC_COMBOBOX_VIS_PERIFS:
        case IDC_CHECK_BLINK:
          checkBlink();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogPanel::CmOk()
{
  tmpProp->type1 = 0;
  if(IS_CHECKED(IDC_CHECK_BLINK)) {
    int tick;
    GET_INT(IDC_EDIT_BLINK, tick);
    tmpProp->type1 = tick;
    }
  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmDialogPanel::checkBlink()
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
void svmDialogPanel::check()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_FILL_PANEL);

  bool checked = IS_CHECKED(IDC_RADIOBUTTON_STYLE_NONE);
  if(!enable && checked) {
    SET_CHECK_SET(IDC_RADIOBUTTON_STYLE_NONE, false);
    SET_CHECK(IDC_RADIOBUTTON_SIMPLE_BORDER);
    }
  ENABLE(IDC_RADIOBUTTON_STYLE_NONE, enable);

  enable = IS_CHECKED(IDC_RADIOBUTTON_SIMPLE_BORDER);
  ENABLE(ID_BTN_PERSONALIZE_BORDER, enable);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmBorderStyle : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmBorderStyle(PPanelBorder& border, COLORREF& bkgColor, PWin* parent, uint id = IDD_BORDER_STYLE,
          HINSTANCE hInst = 0) : Border(border), bkgColor(bkgColor), Panel(0), baseClass(parent, id, hInst) {}
    ~svmBorderStyle() { destroy(); }

    virtual bool create();
  protected:
    PPanelBorder& Border;
    COLORREF& bkgColor;
    PTextPanelStatic* Panel;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void chooseColor(uint which);
    void refresh(uint which);
    void chooseBkgColor();
    void fillCtrl(const PPanelBorder::pbInfo& pb, uint idcheck, uint idtick);
    void checkAll();
    void invalidatePanel();
};
//-----------------------------------------------------------
bool makeBorderStyle(PPanelBorder& border, COLORREF& bkgColor, PWin* parent)
{
  return IDOK == svmBorderStyle(border, bkgColor, parent).modal();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmBorderStyle::fillCtrl(const PPanelBorder::pbInfo& pb, uint idcheck, uint idtick)
{
  if(PPanelBorder::pbOn == pb.style)
    SET_CHECK(idcheck);
  SET_INT(idtick, pb.tickness);
}
//-----------------------------------------------------------
bool svmBorderStyle::create()
{
  Panel = new PTextPanelStatic(this, IDC_STATIC_PANEL);
  if(!baseClass::create())
    return false;

  PTextFixedPanel* panel = Panel->getPanelText();
  panel->setAllBorder(Border);
  panel->setBkgColor(bkgColor);
  if(Border.isSimple())
    SET_CHECK(IDC_CHECK_SIMPLE_BORDER);
  uint check[] = { IDC_CHECK_VIS_TOP, IDC_CHECK_VIS_RIGHT, IDC_CHECK_VIS_BOTTOM, IDC_CHECK_VIS_LEFT };
  uint tick[] = { IDC_EDIT_TICK_TOP, IDC_EDIT_TICK_RIGHT, IDC_EDIT_TICK_BOTTOM, IDC_EDIT_TICK_LEFT };
  for(uint i = 0; i <= PPanelBorder::pbLeft; ++i) {
    const PPanelBorder::pbInfo& pb = Border.getInfo((PPanelBorder::eside)i);
    fillCtrl(pb, check[i], tick[i]);
    }
  checkAll();
  return true;
}
//-----------------------------------------------------------
void svmBorderStyle::checkAll()
{
  bool enable = !IS_CHECKED(IDC_CHECK_SIMPLE_BORDER);
  uint idc [] = { IDC_CHECK_VIS_TOP, IDC_CHECK_VIS_RIGHT, IDC_CHECK_VIS_BOTTOM, IDC_CHECK_VIS_LEFT,
                  IDC_EDIT_TICK_TOP, IDC_EDIT_TICK_RIGHT, IDC_EDIT_TICK_BOTTOM, IDC_EDIT_TICK_LEFT,
                  IDC_BTN_COLOR_TOP, IDC_BTN_COLOR_RIGHT, IDC_BTN_COLOR_BOTTOM, IDC_BTN_COLOR_LEFT
                };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
}
//-----------------------------------------------------------
void svmBorderStyle::invalidatePanel()
{
  PRect r;
  GetWindowRect(*Panel, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
  r.Inflate(20, 20);
  InvalidateRect(*this, r, 1);
}
//-----------------------------------------------------------
void svmBorderStyle::refresh(uint which)
{
  uint check[] = { IDC_CHECK_VIS_TOP, IDC_CHECK_VIS_RIGHT, IDC_CHECK_VIS_BOTTOM, IDC_CHECK_VIS_LEFT };
  uint tick[] = { IDC_EDIT_TICK_TOP, IDC_EDIT_TICK_RIGHT, IDC_EDIT_TICK_BOTTOM, IDC_EDIT_TICK_LEFT };
  uint tickness;
  GET_INT(tick[which], tickness);
  bool checked = IS_CHECKED(check[which]);

  PTextFixedPanel* panel = Panel->getPanelText();
  PPanelBorder::pbInfo pb = panel->getInfo((PPanelBorder::eside)which);
  pb.style = (PPanelBorder::estyle)checked;
  pb.tickness = tickness;
  panel->setBorder((PPanelBorder::eside)which, pb);
  invalidatePanel();
}
//-----------------------------------------------------------
void svmBorderStyle::chooseColor(uint which)
{
  PTextFixedPanel* panel = Panel->getPanelText();
  PPanelBorder::pbInfo pb = panel->getInfo((PPanelBorder::eside)which);
  COLORREF color = pb.color;
  if(choose_Color(*this, color)) {
    pb.color = color;
    panel->setBorder((PPanelBorder::eside)which, pb);
    invalidatePanel();
    }
}
//-----------------------------------------------------------
void svmBorderStyle::chooseBkgColor()
{
  PTextFixedPanel* panel = Panel->getPanelText();
  COLORREF bkg = panel->getColor();
  if(choose_Color(*this, bkg)) {
    panel->setBkgColor(bkg);
    invalidatePanel();
    }
}
//-----------------------------------------------------------
LRESULT svmBorderStyle::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BTN_COLOR_TOP:
           chooseColor(PPanelBorder::pbTop);
           break;
        case IDC_BTN_COLOR_RIGHT:
           chooseColor(PPanelBorder::pbRight);
           break;
        case IDC_BTN_COLOR_BOTTOM:
           chooseColor(PPanelBorder::pbBottom);
           break;
        case IDC_BTN_COLOR_LEFT:
           chooseColor(PPanelBorder::pbLeft);
           break;

        case IDC_CHECK_VIS_TOP:
          refresh(PPanelBorder::pbTop);
           break;
        case IDC_CHECK_VIS_RIGHT:
          refresh(PPanelBorder::pbRight);
           break;
        case IDC_CHECK_VIS_BOTTOM:
          refresh(PPanelBorder::pbBottom);
           break;
        case IDC_CHECK_VIS_LEFT:
          refresh(PPanelBorder::pbLeft);
           break;

        case IDC_BTN_COLOR_BKG:
          chooseBkgColor();
          break;

        case IDC_CHECK_SIMPLE_BORDER:
          checkAll();
          break;
        }
      switch(HIWORD(wParam)) {
        case EN_CHANGE:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_TICK_TOP:
              refresh(PPanelBorder::pbTop);
               break;
            case IDC_EDIT_TICK_RIGHT:
              refresh(PPanelBorder::pbRight);
               break;
            case IDC_EDIT_TICK_BOTTOM:
              refresh(PPanelBorder::pbBottom);
               break;
            case IDC_EDIT_TICK_LEFT:
              refresh(PPanelBorder::pbLeft);
               break;
            }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmBorderStyle::CmOk()
{
  PTextFixedPanel* panel = Panel->getPanelText();
  bkgColor = panel->getColor();
  Border = panel->getAllBorder();
  bool simple = IS_CHECKED(IDC_CHECK_SIMPLE_BORDER);
  Border.setSimple(simple);
  baseClass::CmOk();
}
