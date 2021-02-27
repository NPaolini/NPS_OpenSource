//--------------- svmTextProperty.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmTextProperty.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
HFONT textProperty::getFont(bool linked)
{
  if(linked) {
    svmFont& set = getFontObjLinked();
    return set.getFont2(idFont, Angle);
    }
  svmFont& set = getFontObj();
  return set.getFont2(idFont, Angle);
}
//-----------------------------------------------------------
void textProperty::clone(const Property& other_)
{
  baseClass::clone(other_);
  DYN_CAST(textProperty);
  delete []Text;
  Text = str_newdup(other.Text);
  CLONE(foreground);
  CLONE(background);
  CLONE(hAlign);
  CLONE(vAlign);
  CLONE(textType);
  CLONE(Angle);
  CLONE(idVar);
}
//-----------------------------------------------------------
void textProperty::setText(LPCTSTR newTxt)
{
  delete []Text;
  Text = str_newdup(newTxt);
}
//-----------------------------------------------------------
void textProperty::getVarText(LPTSTR buff, size_t sz)
{
  if(idVar)
    wsprintf(buff, _T("Var. %d"), idVar);
  else
    if(Text)
      _tcscpy_s(buff, sz, Text);
    else
      _tcscpy_s(buff, sz, _T("???"));
}
//-----------------------------------------------------------
bool textProperty::setOtherCommon(const unionProperty& uP)
{
  bool modified = false; //baseClass::setOtherCommon(uP);
/* eupFG, eupBG, eupFont, eupHAlign, eupVAlign */
  SET_MODIFIED(eupFG, foreground)
//  SET_MODIFIED(eupBG, background)
  SET_MODIFIED(eupHAlign, hAlign)
//  SET_MODIFIED(eupVAlign, vAlign)
  SET_MODIFIED(eupFont, idFont)

  return modified;
}
//-----------------------------------------------------------
void textProperty::initUnion(unionProperty& uProp)
{
  baseClass::initUnion(uProp);
  SET_INIT_UNION(eupFG, foreground)
  SET_INIT_UNION(eupHAlign, hAlign)
  SET_INIT_UNION(eupFont, idFont)
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "svmDialogProperty.h"
#include "svmDialogVariable.h"
//-----------------------------------------------------------
svmTextDialogProperty::~svmTextDialogProperty()
{
  destroy();
  if(hb_frg)
    DeleteObject(hb_frg);
  if(hb_bkg)
    DeleteObject(hb_bkg);
}
//-----------------------------------------------------------
bool svmTextDialogProperty::create()
{
  textProperty* rp = dynamic_cast<textProperty*>(Prop);
  c_frg = rp->foreground;
  hb_frg = CreateSolidBrush(c_frg);

  if(!baseClass::create())
    return false;

  SET_INT(IDC_EDIT_X, rp->Rect.left);
  SET_INT(IDC_EDIT_Y, -rp->Rect.top);
  SET_INT(IDC_EDIT_W, rp->Rect.Width());
  SET_INT(IDC_EDIT_H, -rp->Rect.Height());

  uint idcH[] = { IDC_RADIO_AL_SX, IDC_RADIO_AL_CENTER, IDC_RADIO_AL_DX };
  SET_CHECK(idcH[rp->hAlign]);

  uint idcV[] = { IDC_RADIO_AL_TOP, IDC_RADIO_AL_MID, IDC_RADIO_AL_BOTTOM };
  SET_CHECK(idcV[rp->vAlign]);

  SET_INT(IDC_EDIT_ANGLE, rp->Angle);
  SET_INT(IDC_EDIT_NUM_VAR, rp->idVar);

  uint idcT[] = { IDC_RADIO_SIMPLE, IDC_RADIO_FROM_VAR };
  SET_CHECK(idcT[rp->textType & textProperty::eVariable]);
  SET_CHECK_SET(IDC_CHECK_TXT_ON_BOX, rp->textType & textProperty::eboxed);
  SET_CHECK_SET(IDC_CHECK_TXT_MULTILINE, rp->textType & textProperty::eMultiline);

  if(!rp->idVar)
    SET_TEXT(IDC_EDIT_TEXT, rp->Text);
  svmFont& f = getFontObj();
  const typeFont* tf = f.getType(rp->idFont);
  setNameFont(this, IDC_STATIC_FONT, tf, true);

  checkAlignType();
  checkType();

  return true;
}
//-----------------------------------------------------------
LRESULT svmTextDialogProperty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_VAR:
          chooseVar();
          break;
        case IDC_RADIO_SIMPLE:
        case IDC_RADIO_FROM_VAR:
          checkType();
          break;

        case IDC_BUTTON_COL_LINE:
          chooseColor(*this, IDC_STATIC_COL_LINE, hb_frg, c_frg);
          break;
        case IDC_BUTTON_CHOOSE_FONT:
          chooseFont();
          break;
        case IDC_CHECK_TXT_ON_BOX:
        case IDC_CHECK_TXT_MULTILINE:
          checkAlignType();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmTextDialogProperty::chooseFont()
{
  textProperty* rp = dynamic_cast<textProperty*>(tmpProp);
  if(IDOK == svmChooseFont(rp->idFont, this).modal()) {
    svmFont& f = getFontObj();
    const typeFont* tf = f.getType(rp->idFont);
    setNameFont(this, IDC_STATIC_FONT, tf, true);
    }
}
//-----------------------------------------------------------
void svmTextDialogProperty::chooseVar()
{
  int selected;
  GET_INT(IDC_EDIT_NUM_VAR, selected);
  if(IDOK == svmDialogVariable(this, IDD_DIALOG_VARS, selected).modal()) {
    int old;
    GET_INT(IDC_EDIT_NUM_VAR, old);
    if(selected > 0 && selected != old)
      SET_INT(IDC_EDIT_NUM_VAR, selected);
    }
}
//-----------------------------------------------------------
static bool
//svmTextDialogProperty::
exist_var(int idVar)
{
  manageVariable& man_vars = getManVariables();
  PVect<infoVariable>& vars = man_vars.getVariables();
  uint nElem = vars.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(vars[i].id == idVar)
      return true;
  return false;
}
//-----------------------------------------------------------
/*
void svmTextDialogProperty::checkTransp()
{
  bool enable = !IS_CHECKED(IDC_CHECK_NO_BKG);
  ENABLE(IDC_BUTTON_COL_BKG, enable);
}
*/
//-----------------------------------------------------------
void svmTextDialogProperty::checkType()
{
  bool enable = IS_CHECKED(IDC_RADIO_FROM_VAR);
  ENABLE(IDC_EDIT_NUM_VAR, enable);
  ENABLE(IDC_BUTTON_CHOOSE_VAR, enable);
}
//-----------------------------------------------------------
void svmTextDialogProperty::checkAlignType()
{
  bool isSimple = !IS_CHECKED(IDC_CHECK_TXT_ON_BOX);
  if(isSimple) {
    ENABLE(IDC_RADIO_AL_TOP, false);
    ENABLE(IDC_RADIO_AL_MID, false);
    ENABLE(IDC_RADIO_AL_BOTTOM, false);

    ENABLE(IDC_CHECK_TXT_MULTILINE, false);
    ENABLE(IDC_EDIT_ANGLE, true);
    }
  else if(IS_CHECKED(IDC_CHECK_TXT_MULTILINE)) {
    ENABLE(IDC_RADIO_AL_TOP, false);
    ENABLE(IDC_RADIO_AL_MID, false);
    ENABLE(IDC_RADIO_AL_BOTTOM, false);
    ENABLE(IDC_EDIT_ANGLE, false);
    }
  else {
    ENABLE(IDC_RADIO_AL_TOP, true);
    ENABLE(IDC_RADIO_AL_MID, true);
    ENABLE(IDC_RADIO_AL_BOTTOM, true);

    ENABLE(IDC_CHECK_TXT_MULTILINE, true);
    ENABLE(IDC_EDIT_ANGLE, false);
    }
}
//-----------------------------------------------------------
HBRUSH svmTextDialogProperty::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  switch(id) {
    case IDC_STATIC_COL_LINE:
      SetTextColor(dc, c_bkg);
      SetBkColor(dc, c_frg);
      return hb_frg;
/*
    case IDC_STATIC_COL_BKG:
      SetTextColor(dc, c_frg);
      SetBkColor(dc, c_bkg);
      return hb_bkg;
*/
    }
  return 0;
}
//-----------------------------------------------------------
void svmTextDialogProperty::CmOk()
{
  textProperty* rp = dynamic_cast<textProperty*>(tmpProp);
  GET_INT(IDC_EDIT_NUM_VAR, rp->idVar);
  rp->textType = IS_CHECKED(IDC_RADIO_SIMPLE) ? 0 : 1;
  if(rp->textType) {
    if(!exist_var(rp->idVar)) {
      if(IDYES != MessageBox(*this, _T("La variabile immessa non esiste\r\nVuoi continuare lo stesso?"),
            _T("Attenzione!"), MB_YESNO | MB_ICONSTOP))
        return;
      }
    }
  int t;
  GET_INT(IDC_EDIT_X, t);
  rp->Rect.left = t;
  GET_INT(IDC_EDIT_Y, t);
  rp->Rect.top = -t;
  GET_INT(IDC_EDIT_W, t);
  rp->Rect.right = rp->Rect.left + t;
  GET_INT(IDC_EDIT_H, t);
  rp->Rect.bottom = rp->Rect.top - t;

  GET_INT(IDC_EDIT_LINE_TICKNESS, t);
  rp->foreground = c_frg;

  rp->vAlign = IS_CHECKED(IDC_RADIO_AL_TOP) ? 0 :
               IS_CHECKED(IDC_RADIO_AL_MID) ? 1 : 2;

  rp->hAlign = IS_CHECKED(IDC_RADIO_AL_SX) ? 0 :
               IS_CHECKED(IDC_RADIO_AL_CENTER) ? 1 : 2;

  GET_INT(IDC_EDIT_ANGLE, rp->Angle);

  if(IS_CHECKED(IDC_CHECK_TXT_ON_BOX))
    rp->textType |= textProperty::eboxed;
  if(IS_CHECKED(IDC_CHECK_TXT_MULTILINE))
    rp->textType |= textProperty::eMultiline;


  TCHAR txt[2048];
  GET_TEXT(IDC_EDIT_TEXT, txt);
  if(!rp->Text || _tcscmp(rp->Text, txt)) {
    delete []rp->Text;
    rp->Text = str_newdup(txt);
    }

  baseClass::CmOk();
}

//-----------------------------------------------------------

