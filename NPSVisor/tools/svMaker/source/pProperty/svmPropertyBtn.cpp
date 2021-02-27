//--------------- svmPropertyBtn.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyBtn.h"
#include "macro_utils.h"
#include "common.h"
#include "svmNormal.h"
#include "svmPageProperty.h"
#include "svmPagePropAction.h"
#include "language_util.h"
#include "baseAdvBtnDlg.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
PropertyBtn::PropertyBtn() : functionLink(0), fgPress(GetSysColor(COLOR_BTNTEXT)),
            bgPress(GetSysColor(COLOR_BTNFACE)), normalText(0), pressedText(0),
            modelessName(0)
{
  Flags.flag = 0;
  allText[0] = allText[1] = 0;
  type1 = 2;
}
//-----------------------------------------------------------
PropertyBtn::~PropertyBtn()
{
  flushPAV(nameBmp);
  flushPAV(allText);
  delete []functionLink;
}
//-----------------------------------------------------------
void PropertyBtn::fillNameBmp()
{
  uint nElem = nameBmp.getElem();
  if(4 <= nElem)
    return;
  nameBmp.setDim(4);
  switch(nElem) {
   case 1:
     nameBmp[1] = str_newdup(nameBmp[0]);
     // fall through
   case 2:
     nameBmp[2] = str_newdup(nameBmp[0]);
     // fall through
   case 3:
     nameBmp[3] = str_newdup(nameBmp[1]);
     break;
   }
}
//-----------------------------------------------------------
void PropertyBtn::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyBtn* po = dynamic_cast<const PropertyBtn*>(&other);
  if(po && po != this) {
    flushPAV(nameBmp);

    int nElem = po->nameBmp.getElem();
    nameBmp.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      nameBmp[i] = str_newdup(po->nameBmp[i]);
    delete []functionLink;
    functionLink = 0;
    if(po->functionLink)
      functionLink = str_newdup(po->functionLink);

    flushPAV(allText);
    normalText = 0;
    pressedText = 0;
    nElem = po->allText.getElem();
    allText.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      allText[i] = po->allText[i] ? str_newdup(po->allText[i]) : 0;
    normalText = allText[0];
    pressedText = allText[1];
    delete []modelessName;
    modelessName = 0;
    if(po->modelessName)
      modelessName = str_newdup(po->modelessName);

    fgPress = po->fgPress;
    bgPress = po->bgPress;

    otherFg.reset();
    nElem = po->otherFg.getElem();
    otherFg.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      otherFg[i] = po->otherFg[i];

    otherBg.reset();
    nElem = po->otherBg.getElem();
    otherBg.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      otherBg[i] = po->otherBg[i];

    for(uint i = 0; i < SIZE_A(DataPrf); ++i)
      DataPrf[i] = po->DataPrf[i];

    Flags.flag = po->Flags.flag;
    textVal = po->textVal;
    colorVal = po->colorVal;
    bmpVal = po->bmpVal;
    }
}
//-----------------------------------------------------------
void PropertyBtn::cloneMinusProperty(const Property& other)
{
  clone(other);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmDialogBtn::svmDialogBtn(svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(owner, prop, parent, id, hInst), Bmp(0), onLink(false), lastAction(-1)
{
  new langEditDefCR(this, IDC_EDIT_TEXT_NORM);
  new langEditDefCR(this, IDC_EDIT_TEXT_PRESSED);
}
//-----------------------------------------------------------
void svmDialogBtn::allocBmp()
{
  delete Bmp;
  const PropertyBtn* po = dynamic_cast<const PropertyBtn*>(tmpProp);
  if(po) {
    const PVect<LPCTSTR>& names = po->getNames();
    if(names.getElem() > 0) {
      TCHAR name[_MAX_PATH];
      makeNameImage(name, names[0]);
      Bmp = new PBitmap(name);
      }
    }
}
//-----------------------------------------------------------
bool svmDialogBtn::create()
{
  ColorBtn[0].Btn = new  POwnBtn(this, IDC_BUTTON_FG_NORM);
  ColorBtn[0].color = Prop->foreground;

  ColorBtn[1].Btn = new  POwnBtn(this, IDC_BUTTON_BG_NORM);
  ColorBtn[1].color = Prop->background;

  PropertyBtn* pt = dynamic_cast<PropertyBtn*>(Prop);
  if(pt) {
    ColorBtn[2].Btn = new  POwnBtn(this, IDC_BUTTON_FG_PRESS);
    ColorBtn[2].color = pt->fgPress;

    ColorBtn[3].Btn = new  POwnBtn(this, IDC_BUTTON_BG_PRESS);
    ColorBtn[3].color = pt->bgPress;
    }

  if(!baseClass::create())
    return false;
  HWND hwCBA = GetDlgItem(*this, IDC_COMBOBOX_BTN_CHOOSE_ACTION);
  fillCBAction(hwCBA, 0);
//  addStringToComboBox(hwCBA, _T("Open Recipe with Listbox"));

  ENABLE(IDC_BUTTON_BTN_CHOOSE_ACTION, false);
  enableModeless(0);

  fillType();
  allocBmp();
  invalidateColor(0);
  invalidateColor(2);


  return true;
}
//-----------------------------------------------------------
void svmDialogBtn::fillType()
{
  static LPCTSTR typesShow[] = {
    _T("Solo Bitmap"),
    _T("Bitmap e Testo"),
    _T("Standard + Bitmap"),
    _T("nuovo Standard + Bitmap"),
    };
  HWND hcb = GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE);
  for(uint i = 0; i < SIZE_A(typesShow); ++i)
    addStringToComboBox(hcb, typesShow[i]);

  SendMessage(hcb, CB_SETCURSEL, Prop->type1, 0);

  static LPCTSTR typesAction[] = {
    _T("Azione"),
    _T("On Off"),
    _T("Primo nel Gruppo"),
    _T("Successivi nel Gruppo"),
    _T("On finché premuto"),
    // i pulsanti nascosti vengono creati solo nel settaggio delle proprietà della pagina
//    _T("Hide Button"),
    _T("Apri Modeless"),
    _T("Apri pagina da bit")
    };
  hcb = GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE_ACTION);
  for(uint i = 0; i < SIZE_A(typesAction); ++i)
    addStringToComboBox(hcb, typesAction[i]);

  int sel = Prop->type2;
  if(btnModeless <= sel)
    --sel;
  SendMessage(hcb, CB_SETCURSEL, sel, 0);

  const PropertyBtn* po = dynamic_cast<const PropertyBtn*>(Prop);
  if(po) {
    if(po->functionLink)
      SET_TEXT(IDC_EDIT_COMMAND, po->functionLink);
    if(po->normalText)
      SET_TEXT(IDC_EDIT_TEXT_NORM, po->normalText);
    if(po->pressedText)
      SET_TEXT(IDC_EDIT_TEXT_PRESSED, po->pressedText);
    if(po->modelessName)
      SET_TEXT(IDC_EDIT_BTN_MODELESS, po->modelessName);
    }
  checkEnabled(true);
  if(onLink)
    checkEditLink();

}
//-----------------------------------------------------------
void svmDialogBtn::personalize()
{
  PropertyBtn* po = dynamic_cast<PropertyBtn*>(tmpProp);
  TCHAR buff[500];
  GET_TEXT(IDC_EDIT_TEXT_NORM, buff);
  if(*buff && (!po->allText[0] || _tcscmp(buff, po->allText[0]))) {
    delete []po->allText[0];
    po->allText[0] = po->normalText = str_newdup(buff);
    }

  GET_TEXT(IDC_EDIT_TEXT_PRESSED, buff);
  if(*buff && (!po->allText[1] || _tcscmp(buff, po->allText[1]))) {
    delete []po->allText[1];
    po->allText[1] = po->pressedText = str_newdup(buff);
    }
  po->foreground = ColorBtn[0].color;
  po->background = ColorBtn[1].color;
  po->fgPress = ColorBtn[2].color;
  po->bgPress = ColorBtn[3].color;

  TCHAR old[_MAX_PATH] = _T("\0");
  const PVect<LPCTSTR>& names = po->getNames();
  if(names.getElem() > 0)
    _tcscpy_s(old, names[0]);

  if(IDOK == advancedBtnDlg(this, *po, IDD_BUTTON_ADVANCED).modal()) {
    SET_TEXT(IDC_EDIT_TEXT_NORM, po->allText[0]);
    SET_TEXT(IDC_EDIT_TEXT_PRESSED, po->allText[1]);
    ColorBtn[0].color = po->foreground;
    ColorBtn[1].color = po->background;
    ColorBtn[2].color = po->fgPress;
    ColorBtn[3].color = po->bgPress;
    invalidateColor(0);
    invalidateColor(2);
    if(names.getElem() > 0) {
      if(_tcsicmp(old, names[0]))
        allocBmp();
      }
    else if(*old) {
      delete Bmp;
      Bmp = 0;
      }
    }
}
//-----------------------------------------------------------
LRESULT svmDialogBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
//        case IDC_CHECKBOX_MOV:
//          checkEnabled();
//          break;
        case IDC_BUTTON_CHOOSE_BMP:
          chooseBmp();
          break;
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
          checkEnabled(false);
          break;
        case IDC_BUTTON_MODELESS:
          findModelessName();
          break;
        case IDC_BUTTON_BTN_CHOOSE_ACTION:
          chooseAction();
          break;
        case IDC_BUTTON_PERSONALIZE:
          personalize();
          break;
        }

      switch(HIWORD(wParam)) {
        case CBN_SELCHANGE:
          switch(LOWORD(wParam)) {
            case IDC_COMBOBOX_FLAG_TYPE:
            case IDC_COMBOBOX_FLAG_TYPE_ACTION:
              checkEnabled(IDC_COMBOBOX_FLAG_TYPE_ACTION == LOWORD(wParam));
              break;
            case IDC_COMBOBOX_BTN_CHOOSE_ACTION:
              chooseAction();
              break;

            case IDC_COMBOBOX_PERIFS:
              if(onLink)
                checkEditLink();
              break;
            }
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//extern bool isChangePage(LPCTSTR row);
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmDialogBtn::chooseAction()
{
  int code = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_BTN_CHOOSE_ACTION), CB_GETCURSEL, 0, 0);
  TCHAR buff[500] = _T("\0");
  if(-1 == lastAction || lastAction == code)
    GET_TEXT(IDC_EDIT_COMMAND, buff);
  lastAction = code;
  bool enableBtn = true;
  if(getChooseAction(this, buff, SIZE_A(buff), code, enableBtn)) {
    SET_TEXT(IDC_EDIT_COMMAND, buff);
    ENABLE(IDC_BUTTON_BTN_CHOOSE_ACTION, enableBtn);
    }
}
//-----------------------------------------------------------
void svmDialogBtn::chooseColor(int ix)
{
  if(choose_Color(*this, ColorBtn[ix].color)) {
    invalidateColor(ix);
    }
}
//-----------------------------------------------------------
int getIdOpenPage(PWin* win, int oldId)
{
//  menuPopup menu[getNumOpenPage()];
  P_SmartPointer<menuPopup*> menu(new menuPopup[getNumOpenPage()], true);
  fillMenuOpenPage(menu, getNumOpenPage(), oldId);

  return popupMenu(*win, menu, getNumOpenPage());
}
//-----------------------------------------------------------
void svmDialogBtn::findModelessName()
{
  TCHAR buff[500];
  svmPageBase data;
  GET_TEXT(IDC_EDIT_BTN_MODELESS, buff);
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE_ACTION), CB_GETCURSEL, 0, 0);
  sel -= btnHide;
  switch(sel) {
    case 0:
      do {
        data.pswLevel = _ttoi(buff);
        LPCTSTR p = findNextParam(buff, 1);
        if(p)
          _tcscpy_s(data.name, p);
        svmPagePropActionOpenName on(this, data);
        if(IDOK == on.modal()) {
          wsprintf(buff, _T("%d,%s"), data.pswLevel, data.name);
          SET_TEXT(IDC_EDIT_BTN_MODELESS, buff);
          }
        } while(false);
      break;
    case 1:
      do {
        int type = _ttoi(buff);
        type = getIdOpenPage(this, type);
        if(type <= 0)
          break;
        LPCTSTR p = findNextParam(buff, 1);
        if(p)
          data.pswLevel = _ttoi(p);
        p = findNextParam(p, 1);
        if(p)
          _tcscpy_s(data.name, p);
        svmPagePropActionOpenName on(this, data);
        if(IDOK == on.modal()) {
          wsprintf(buff, _T("%d,%d,%s"), type, data.pswLevel, data.name);
          SET_TEXT(IDC_EDIT_BTN_MODELESS, buff);
          }
        } while(false);
      break;
    }
}
//-----------------------------------------------------------
void svmDialogBtn::invalidateColor(int ix)
{
#if 1
  int baseIx = ix & ~1;
  COLORREF fg = ColorBtn[baseIx].color;
  COLORREF bg = ColorBtn[baseIx + 1].color;
  int ix_v = ix / 2;
  PropertyBtn* po = dynamic_cast<PropertyBtn*>(tmpProp);
  if(ix & 1) {
    po->otherBg[ix_v] = bg;
    }
  else {
    po->otherFg[ix_v] = fg;
    }
  POwnBtn::colorRect R = ColorBtn[baseIx].Btn->getColorRect();
  R.bkg = fg;
  R.txt = bg;
  ColorBtn[ix].Btn->setColorRect(R);

  R = ColorBtn[baseIx + 1].Btn->getColorRect();
  R.txt = fg;
  R.bkg = bg;
  ColorBtn[baseIx + 1].Btn->setColorRect(R);

  if(getHandle()) {
    InvalidateRect(*ColorBtn[baseIx].Btn, 0, 0);
    InvalidateRect(*ColorBtn[baseIx + 1].Btn, 0, 0);
    }
#else
  int ixDual = (ix & 1) ? ix - 1 : ix + 1;
  if(ixDual < ix) {
    --ix;
    ++ixDual;
    }
  COLORREF fg = ColorBtn[ix].color;
  COLORREF bg = ColorBtn[ixDual].color;

  POwnBtn::colorRect R = ColorBtn[ix].Btn->getColorRect();
  R.bkg = fg;
  R.txt = bg;
  ColorBtn[ix].Btn->setColorRect(R);

  R = ColorBtn[ixDual].Btn->getColorRect();
  R.txt = fg;
  R.bkg = bg;
  ColorBtn[ixDual].Btn->setColorRect(R);

  if(getHandle()) {
    InvalidateRect(*ColorBtn[ix].Btn, 0, 0);
    InvalidateRect(*ColorBtn[ixDual].Btn, 0, 0);
    }
#endif
}
//-----------------------------------------------------------
void svmDialogBtn::CmOk()
{
  tmpProp->type1 = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE), CB_GETCURSEL, 0, 0);
  tmpProp->type2 = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE_ACTION), CB_GETCURSEL, 0, 0);
  if(btnHide <= tmpProp->type2)
    ++tmpProp->type2;

  PropertyBtn* po = dynamic_cast<PropertyBtn*>(tmpProp);
  if(po) {
    TCHAR buff[500];
    GET_TEXT(IDC_EDIT_COMMAND, buff);
    delete []po->functionLink;
    po->functionLink = 0;
    if(*buff)
      po->functionLink = str_newdup(buff);
    // esistono forzatamente due elementi, il costruttore della proprietà ne mette due a zero
    delete []po->allText[0];
    po->allText[0] = po->normalText = 0;
    GET_TEXT(IDC_EDIT_TEXT_NORM, buff);
    if(*buff)
      po->allText[0] = po->normalText = str_newdup(buff);

    delete []po->allText[1];
    po->allText[1] = po->pressedText = 0;
    GET_TEXT(IDC_EDIT_TEXT_PRESSED, buff);
    if(*buff)
      po->allText[1] = po->pressedText = str_newdup(buff);
    delete []po->modelessName;
    po->modelessName = 0;
    GET_TEXT(IDC_EDIT_BTN_MODELESS, buff);
    if(*buff)
      po->modelessName = str_newdup(buff);

    po->foreground = ColorBtn[0].color;
    po->background = ColorBtn[1].color;
    po->fgPress = ColorBtn[2].color;
    po->bgPress = ColorBtn[3].color;

    // gli altri elementi, se nuovo tipo, vengono memorizzati nel dialogo di personalizzazione
    }

  baseClass::CmOk();
}
//-----------------------------------------------------------
static int idsHideShow1[] = {
//    IDC_STATICTEXT_PERIF,
    IDC_STATICTEXT_ADDR,
    IDC_STATICTEXT_TYPE,
    IDC_STATICTEXT_MASK,
    IDC_STATICTEX_NBITS,
    IDC_STATICTEXT_OFFSET,
//    IDC_COMBOBOX_PERIFS,
    IDC_EDIT_ADDR,
    IDC_COMBOBOX_TYPEVAL,
    IDC_EDIT_NORMALIZ,
    IDC_EDIT_NDEC,
    IDC_EDIT_NBITS,
    IDC_EDIT_OFFSET,
    IDC_CHECKBOX_NEGATIVE
    };
//-----------------------------------------------------------
static int idsHideShow2[] = {
    IDC_EDIT_COMMAND,
    IDC_COMBOBOX_BTN_CHOOSE_ACTION,
    IDC_BUTTON_BTN_CHOOSE_ACTION
    };
//----------------------------------------------------------------------------
extern bool getUseExtKeyb();
//-----------------------------------------------------------
static LPCTSTR FunctionLink[] = {
  _T("Altro"),
  _T("F1"),
  _T("F2"),
  _T("F3"),
  _T("F4"),
  _T("F5"),
  _T("F6"),
  _T("F7"),
  _T("F8"),
  _T("F9"),
  _T("F10"),
  _T("F11"),
  _T("F12"),

  _T("F13"),
  _T("F14"),
  _T("F15"),
  _T("F16"),
  _T("F17"),
  _T("F18"),
  _T("F19"),
  _T("F20"),
  _T("S1"),
  _T("S2"),
  _T("S3"),
  _T("S4"),

  _T("S5"),
  _T("S6"),
  _T("S7"),
  _T("S8"),
  _T("S9"),
  _T("S10"),
  _T("S11"),
  _T("S12"),
  _T("S13"),
  _T("S14"),
  _T("S15"),
  _T("S16"),
  };
//-----------------------------------------------------------
bool svmDialogBtn::checkZeroAction()
{
  SET_TEXT(IDC_GROUPBOX_PERIF, _T("Comandi Funzione"));
  SET_TEXT(IDC_STATICTEXT_PERIF, _T("Tasti Funzione"));
  HWND hcb = GetDlgItem(*this, IDC_COMBOBOX_PERIFS);

  int repeat = SIZE_A(FunctionLink);
  if(!getUseExtKeyb())
    repeat = 13;
  SendMessage(hcb, CB_RESETCONTENT, 0, 0);
  for(int i = 0; i < repeat; ++i)
    addStringToComboBox(hcb, FunctionLink[i]);

  SendMessage(hcb, CB_SETCURSEL, tmpProp->perif, 0);
  for(uint i = 0; i < SIZE_A(idsHideShow1); ++i)
    ShowWindow(GetDlgItem(*this, idsHideShow1[i]), SW_HIDE);

  for(uint i = 0; i < SIZE_A(idsHideShow2); ++i) {
    ShowWindow(GetDlgItem(*this, idsHideShow2[i]), SW_SHOWNORMAL);
    ENABLE(idsHideShow2[i], !tmpProp->perif);
    }
  if(!tmpProp->perif) {
    hcb = GetDlgItem(*this, IDC_COMBOBOX_BTN_CHOOSE_ACTION);
    TCHAR t[500];
    GET_TEXT(IDC_EDIT_COMMAND, t);
    int sel = _ttoi(t);
    SendMessage(hcb, CB_SETCURSEL, sel, 0);
    }
  onLink = true;
  return false;
//  return true;
}
//-----------------------------------------------------------
void svmDialogBtn::checkEditLink()
{
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_PERIFS), CB_GETCURSEL, 0, 0);
  bool enable = !sel;
  for(uint i = 0; i < SIZE_A(idsHideShow2); ++i)
    ENABLE(idsHideShow2[i], enable);
}
//-----------------------------------------------------------
bool svmDialogBtn::checkNormalAction()
{
  if(onLink) {
    SET_TEXT(IDC_GROUPBOX_PERIF, _T("Definizioni Periferiche"));
    SET_TEXT(IDC_STATICTEXT_PERIF, _T("Perif"));
    HWND hcb = GetDlgItem(*this, IDC_COMBOBOX_PERIFS);

    SendMessage(hcb, CB_RESETCONTENT, 0, 0);
    fillCBPerif(hcb, tmpProp->perif);

    for(uint i = 0; i < SIZE_A(idsHideShow1); ++i)
      ShowWindow(GetDlgItem(*this, idsHideShow1[i]), SW_SHOWNORMAL);

    for(uint i = 0; i < SIZE_A(idsHideShow2); ++i)
      ShowWindow(GetDlgItem(*this, idsHideShow2[i]), SW_HIDE);

    onLink = false;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
bool svmDialogBtn::checkhide()
{
  return checkNormalAction();
}
//-----------------------------------------------------------
void svmDialogBtn::enableModeless(int enable)
{
  HWND hwnd = GetDlgItem(*this, IDC_EDIT_BTN_MODELESS);
  EnableWindow(hwnd, enable > 0);
  hwnd = GetDlgItem(*this, IDC_BUTTON_MODELESS);
  EnableWindow(hwnd, enable > 0);
  switch(enable) {
    case 1:
      SetWindowText(hwnd, _T("Modeless"));
      break;
    case 2:
      SetWindowText(hwnd, _T("Apri Pagina"));
      break;
    }
}
//-----------------------------------------------------------
void svmDialogBtn::checkEnabled(bool all)
{
  bool enable = false;
  if(all) {
    int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE_ACTION), CB_GETCURSEL, 0, 0);
    bool enablePsw = true;
    switch(sel) {
      case btnAction:
        enable = checkZeroAction();
        enablePsw = false;
        break;
      case btnOnOff:
      case btnFirstGroup:
      case btnNextGroup:
      case btnPressing:
        enable = checkNormalAction();
        break;
      case btnHide:
      case btnHide + 1:
        enable = checkhide();
        break;
      }
    enableModeless(sel - btnHide + 1);
    ENABLE(IDC_COMBOBOX_PSWLEVEL, enablePsw);
    if(!enable) {
      sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE), CB_GETCURSEL, 0, 0);
      ENABLE(IDC_BUTTON_PERSONALIZE, sel == btnS_NewStdAndBmp);
      }
    }

  if(!all || enable) {

    int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE), CB_GETCURSEL, 0, 0);
    bool enable = sel != btnS_OnlyBmp;
    ENABLE(IDC_BUTTON_FG_NORM, enable);
    ENABLE(IDC_BUTTON_FG_PRESS, enable);
    ENABLE(IDC_EDIT_TEXT_PRESSED, enable);
    ENABLE(IDC_EDIT_TEXT_NORM, enable);
    if(enable)
      enable = !IS_CHECKED(IDC_CHECKBOX_TRANSP_PANEL) && (sel == btnS_StdAndBmp || sel == btnS_NewStdAndBmp);
    ENABLE(IDC_BUTTON_BG_NORM, enable);
    ENABLE(IDC_BUTTON_BG_PRESS, enable);
    enable = sel == btnS_NewStdAndBmp;
    ENABLE(IDC_BUTTON_PERSONALIZE, enable);
    }
}
//-----------------------------------------------------------
/**/
//-----------------------------------------------------------
void svmDialogBtn::chooseBmp()
{
  PropertyBtn* pt = dynamic_cast<PropertyBtn*>(tmpProp);
  if(pt) {
    TCHAR old[_MAX_PATH] = _T("\0");
    const PVect<LPCTSTR>& names = pt->getNames();
    if(names.getElem() > 0)
      _tcscpy_s(old, names[0]);
    if(IDOK == svmChooseBitmap(pt->getNames(), this).modal()) {
      if(names.getElem() > 0) {
        if(_tcsicmp(old, names[0]))
          allocBmp();
        }
      else if(*old) {
        delete Bmp;
        Bmp = 0;
        }
      }
    }
}
//-----------------------------------------------------------
void saveFullPrph(P_File& pf, LPTSTR buff, int id, const fullPrph& data)
{
  manageObjId moi(id);
  id = moi.getFirstExtendId();
  wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"),
            id,
            data.prph,
            data.addr,
            data.typeVal,
            data.normaliz
            );
  writeStringChkUnicode(pf, buff);
  id = moi.getSecondExtendId();
  wsprintf(buff, _T("%d,%d,%d\r\n"),
            id,
            data.nBits,
            data.offset
            );
  writeStringChkUnicode(pf, buff);
}
//-----------------------------------------------------------
void loadFullPrph(uint id, setOfString& set, fullPrph& data)
{
  manageObjId moi(id);
  id = moi.getFirstExtendId();
  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d"),
            &data.prph,
            &data.addr,
            &data.typeVal,
            &data.normaliz
            );
  id = moi.getSecondExtendId();
  p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%d"),
            &data.nBits,
            &data.offset
            );
}
//-----------------------------------------------------------
