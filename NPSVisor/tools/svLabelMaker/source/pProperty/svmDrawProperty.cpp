//--------------- svmDrawProperty.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmDrawProperty.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
void drawProperty::clone(const Property& other_)
{
  baseClass::clone(other_);
  DYN_CAST(drawProperty);
  CLONE(idBrush);
}
//-----------------------------------------------------------
bool drawProperty::setOtherCommon(const unionProperty& uP)
{
  bool modified = baseClass::setOtherCommon(uP);
/* eupPen, eupBrush */
  SET_MODIFIED(eupBrush, idBrush)

  return modified;
}
//-----------------------------------------------------------
void drawProperty::initUnion(unionProperty& uProp)
{
  baseClass::initUnion(uProp);
  SET_INIT_UNION(eupBrush, idBrush)
}
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
svmDrawDialogProperty::~svmDrawDialogProperty()
{
  destroy();
  if(hb_bkg)
    DeleteObject(hb_bkg);
}
//-----------------------------------------------------------
bool svmDrawDialogProperty::create()
{
  drawProperty* rp = dynamic_cast<drawProperty*>(Prop);

  dualBrush* dBrush = getDualBrush();
  infoBrush iBrush = *dBrush->getIt(rp->idBrush);
  hb_bkg = iBrush.hbrush;
  iBrush.hbrush = 0;
  c_bkg = iBrush.color;

  if(!baseClass::create())
    return false;

  HWND hChild = GetDlgItem(*this, IDC_COMBO_BKG_TYPE);
  fillCB_BrushType(hChild);
  int sel = cNULL_COLOR == iBrush.color ? 0 : iBrush.style + 2;
  SendMessage(hChild, CB_SETCURSEL, sel, 0);

  SET_INT(IDC_EDIT_W, rp->Rect.Width());
  SET_INT(IDC_EDIT_H, -rp->Rect.Height());

  checkCbChange(IDC_COMBO_BKG_TYPE);
  return true;
}
//-----------------------------------------------------------
LRESULT svmDrawDialogProperty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_COMBO_BKG_TYPE:
        case IDC_COMBO_LINE_TYPE:
          switch(HIWORD(wParam)) {
            case CBN_SELCHANGE:
              checkCbChange(LOWORD(wParam));
              break;
            }
          break;
        case IDC_BUTTON_COL_BKG:
          chooseColor(*this, IDC_STATIC_COL_BKG, hb_bkg, c_bkg);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDrawDialogProperty::checkCbChange(uint idc)
{
  HWND hChild = GetDlgItem(*this, idc);
  int sel = SendMessage(hChild, CB_GETCURSEL, 0, 0);
  uint idBtn = 0;
  bool enable;
  switch(idc) {
    case IDC_COMBO_BKG_TYPE:
      idBtn = IDC_BUTTON_COL_BKG;
      enable = sel > 0;
      break;
    case IDC_COMBO_LINE_TYPE:
      idBtn = IDC_BUTTON_COL_LINE;
      enable = sel != 5;
      ENABLE(IDC_EDIT_LINE_TICKNESS, sel == 0);
      if(sel)
        SET_INT(IDC_EDIT_LINE_TICKNESS, 0);
      break;
    }
  ENABLE(idBtn, enable);
}
//-----------------------------------------------------------
HBRUSH svmDrawDialogProperty::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  switch(id) {
    case IDC_STATIC_COL_BKG:
      return hb_bkg;
    case IDC_STATIC_COL_LINE:
      return hb_line;
    }
  return 0;
}
//-----------------------------------------------------------
void svmDrawDialogProperty::CmOk()
{
  drawProperty* rp = dynamic_cast<drawProperty*>(tmpProp);

  dualBrush* dBrush = getDualBrush();
  infoBrush iBrush;
  iBrush.color = c_bkg;
  HWND hChild = GetDlgItem(*this, IDC_COMBO_BKG_TYPE);
  int t = SendMessage(hChild, CB_GETCURSEL, 0, 0);
  --t;
  if(t < 0)
    iBrush.color = cNULL_COLOR;
  --t;
  iBrush.style = t;
  rp->idBrush = dBrush->addAndGetId(&iBrush);

  baseClass::CmOk();
  rp = dynamic_cast<drawProperty*>(Prop);
  rp->Rect.right += rp->Rect.left;
  rp->Rect.bottom += rp->Rect.top;

}

