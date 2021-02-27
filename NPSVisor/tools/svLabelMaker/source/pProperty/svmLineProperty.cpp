//--------------- svmLineProperty.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmLineProperty.h"
//-----------------------------------------------------------
void lineProperty::clone(const Property& other_)
{
  baseClass::clone(other_);
  DYN_CAST(lineProperty);
  CLONE(idPen);
  CLONE(dir);
}
//-----------------------------------------------------------
bool lineProperty::setOtherCommon(const unionProperty& uP)
{
  bool modified = false; //baseClass::setOtherCommon(uP);
/* eupPen, eupBrush */
  SET_MODIFIED(eupPen, idPen)

  return modified;
}
//-----------------------------------------------------------
void lineProperty::initUnion(unionProperty& uProp)
{
  baseClass::initUnion(uProp);
  SET_INIT_UNION(eupPen, idPen)
}
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
svmLineDialogProperty::~svmLineDialogProperty()
{
  destroy();
  if(hb_line)
    DeleteObject(hb_line);
}
//-----------------------------------------------------------
bool svmLineDialogProperty::create()
{
  lineProperty* rp = dynamic_cast<lineProperty*>(Prop);
  dualPen* dPen = getDualPen();
  infoPen iPen = *dPen->getIt(rp->idPen);
  hb_line = CreateSolidBrush(iPen.color);
  iPen.hpen = 0;
  c_line = iPen.color;

  if(!baseClass::create())
    return false;

  HWND hChild = GetDlgItem(*this, IDC_COMBO_LINE_TYPE);
  fillCB_PenType(hChild, alsoNoLine);
  int sel = iPen.style;
  SendMessage(hChild, CB_SETCURSEL, sel, 0);

  SET_INT(IDC_EDIT_X, rp->Rect.left);
  SET_INT(IDC_EDIT_Y, -rp->Rect.top);
  SET_INT(IDC_EDIT_W, rp->Rect.right);
  SET_INT(IDC_EDIT_H, -rp->Rect.bottom);

  SET_INT(IDC_EDIT_LINE_TICKNESS, iPen.tickness);

  checkCbChange(IDC_COMBO_LINE_TYPE);
  checkTickness();
  return true;
}
//-----------------------------------------------------------
LRESULT svmLineDialogProperty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_EDIT_LINE_TICKNESS:
          switch(HIWORD(wParam)) {
            case EN_KILLFOCUS:
              checkTickness();
              break;
            }
          break;
        case IDC_COMBO_LINE_TYPE:
          switch(HIWORD(wParam)) {
            case CBN_SELCHANGE:
              checkCbChange(LOWORD(wParam));
              break;
            }
          break;
        case IDC_BUTTON_COL_LINE:
          chooseColor(*this, IDC_STATIC_COL_LINE, hb_line, c_line);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmLineDialogProperty::checkTickness()
{
  int v;
  GET_INT(IDC_EDIT_LINE_TICKNESS, v);
  if(v > 1)
    SendMessage(GetDlgItem(*this, IDC_COMBO_LINE_TYPE), CB_SETCURSEL, 0, 0);
}
//-----------------------------------------------------------
void svmLineDialogProperty::checkCbChange(uint idc)
{
  HWND hChild = GetDlgItem(*this, idc);
  int sel = SendMessage(hChild, CB_GETCURSEL, 0, 0);
  uint idBtn = 0;
  bool enable;
  switch(idc) {
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
HBRUSH svmLineDialogProperty::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  switch(id) {
    case IDC_STATIC_COL_LINE:
      return hb_line;
    }
  return 0;
}
//-----------------------------------------------------------
void svmLineDialogProperty::CmOk()
{
  lineProperty* rp = dynamic_cast<lineProperty*>(tmpProp);
  int t;
  GET_INT(IDC_EDIT_X, t);
  rp->Rect.left = t;
  GET_INT(IDC_EDIT_Y, t);
  rp->Rect.top = -t;
  GET_INT(IDC_EDIT_W, t);
  rp->Rect.right = t;
  GET_INT(IDC_EDIT_H, t);
  rp->Rect.bottom = -t;

  GET_INT(IDC_EDIT_LINE_TICKNESS, t);
  dualPen* dPen = getDualPen();
  infoPen iPen;
  iPen.tickness = t;
  iPen.color = c_line;
  HWND hChild = GetDlgItem(*this, IDC_COMBO_LINE_TYPE);
  iPen.style = SendMessage(hChild, CB_GETCURSEL, 0, 0);
  rp->idPen = dPen->addAndGetId(&iPen);

  baseClass::CmOk();
}

//-----------------------------------------------------------

