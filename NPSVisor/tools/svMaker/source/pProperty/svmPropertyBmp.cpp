//--------------- svmPropertyBmp.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyBmp.h"
#include "macro_utils.h"
#include "common.h"
#include "svmNormal.h"
//#include "POwnBtn.h"
//-----------------------------------------------------------
/*
bmpMoveInfo::bmpMoveInfo(const bmpMoveInfo& other) : rect(other.rect), dir(other.dir)
{
  for(int i = 0; i < SIZE_A(vI); ++i)
    vI[i] = other.vI[i];
}
//-----------------------------------------------------------
const bmpMoveInfo& bmpMoveInfo::operator =(const bmpMoveInfo& other)
{
  if(&other == this) {
    for(int i = 0; i < SIZE_A(vI); ++i)
      vI[i] = other.vI[i];
    dir = other.dir;
    rect = other.rect;
    }
  return *this;
}
*/
//-----------------------------------------------------------
PropertyBmp::PropertyBmp() : /*mInfo(0),*/ timeCycle(0)
{
}
//-----------------------------------------------------------
PropertyBmp::~PropertyBmp()
{
  flushPAV(nameBmp);
//  delete mInfo;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PropertyBmp::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyBmp* po = dynamic_cast<const PropertyBmp*>(&other);
  if(po && po != this) {
    flushPAV(nameBmp);

    int nElem = po->nameBmp.getElem();
    nameBmp.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      nameBmp[i] = str_newdup(po->nameBmp[i]);
/*
    if(mInfo) {
      delete mInfo;
      mInfo = 0;
      }
    if(po->mInfo)
      mInfo = new bmpMoveInfo(*po->mInfo);
*/
    timeCycle = po->timeCycle;
    }
}
//-----------------------------------------------------------
void PropertyBmp::cloneMinusProperty(const Property& other)
{
  clone(other);
/*
  baseClass::clone(other);
  const PropertyBmp* po = dynamic_cast<const PropertyBmp*>(&other);
  if(po && po != this) {
    setText(po->Text);
    useColor = po->usingColor();
    }
*/
}
//-----------------------------------------------------------
void svmDialogBmp::allocBmp()
{
  delete Bmp;
  const PropertyBmp* po = dynamic_cast<const PropertyBmp*>(tmpProp);
  if(po) {
    const PVect<LPCTSTR>& names = po->getNames();
    if(names.getElem() > 0) {
      TCHAR name[_MAX_PATH];
      makeNameImage(name, names[0]);
      Bmp = new PBitmap(name);
      SIZE sz = Bmp->getSize();
      wsprintf(name, _T("Real Image size = %dx%d"), sz.cx, sz.cy);
      SET_TEXT(IDC_STATICTEXT_REAL_SIZE, name);
      }
    else SET_TEXT(IDC_STATICTEXT_REAL_SIZE, _T(""));
    }
}
//-----------------------------------------------------------
bool svmDialogBmp::create()
{
  if(!baseClass::create())
    return false;
  fillType();
  const PropertyBmp* po = dynamic_cast<const PropertyBmp*>(tmpProp);
  SET_INT(IDC_TIMECYCLE, po->timeCycle);
  allocBmp();
  checkHeight();
/*
  int idCtrl;
  switch(Prop->simpleVarText) {
    case PropertyBmp::simple:
    default:
      idCtrl = IDC_RADIOBUTTON_SIMPLE_TEXT;
      break;
    case PropertyBmp::var_text:
      idCtrl = IDC_RADIOBUTTON_VAR_TEXT;
      break;
    case PropertyBmp::var_value:
      idCtrl = IDC_RADIOBUTTON_VAR_VALUE;
      break;
    }

  SET_CHECK(idCtrl);
  PropertyBmp* pt = dynamic_cast<PropertyBmp*>(Prop);
  if(pt)
    SET_TEXT(IDC_EDIT_SIMPLE_TEXT, pt->getText());
  checkSimpleOrVar();
*/
  return true;
}
//-----------------------------------------------------------
void svmDialogBmp::fillType()
{
  static LPCTSTR types[] = {
    _T("fisso - 1 Bmp"),
    _T("on_off - 2 Bmp"),
    _T("fisso con lamp - 1 Bmp"),
    _T("doppio lamp - 2 Bmp"),
    _T("sequenza variabile"),
    _T("sequenza fissa"),
    _T("sequenza animata"),
    };
  HWND hcb = GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE);
  for(uint i = 0; i < SIZE_A(types); ++i)
    addStringToComboBox(hcb, types[i]);

  if(useMove(Prop))
    SET_CHECK(IDC_CHECKBOX_MOV);

  if(useShowHide(Prop)) {
    SET_CHECK(IDC_RADIOBUTTON_SHOW_HIDE);
    SET_CHECK_SET(IDC_RADIOBUTTON_ALTERNATE, 0);
  }
  else {
    SET_CHECK(IDC_RADIOBUTTON_ALTERNATE);
    SET_CHECK_SET(IDC_RADIOBUTTON_SHOW_HIDE, 0);
  }

  SendMessage(hcb, CB_SETCURSEL, Prop->type1, 0);
  checkEnabled();
}
//-----------------------------------------------------------
LRESULT svmDialogBmp::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
/*
        case IDC_CHECKBOX_MOV:
          checkEnabled();
          break;
        case IDC_BUTTON_MOVIM:
          movingData();
          break;
*/
        case IDC_BUTTON_CHOOSE_BMP:
          chooseBmp();
          break;
        }

      switch(HIWORD(wParam)) {
        case CBN_SELCHANGE:
          switch(LOWORD(wParam)) {
            case IDC_COMBOBOX_FLAG_TYPE:
              checkEnabled();
              break;
            }
        case EN_CHANGE:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_W:
              checkHeight();
              break;
            }
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogBmp::checkHeight()
{
  int h;
  GET_INT(IDC_EDIT_W, h);

  if(Bmp && Bmp->isValid()) {
    SIZE sz = Bmp->getSize();
    h = (int)((double)h / (double)sz.cx * (double)sz.cy);
    }

  SET_INT(IDC_EDIT_H, h);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmDialogBmp::CmOk()
{
  tmpProp->type1 = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE), CB_GETCURSEL, 0, 0);
  WORD move = 0; //IS_CHECKED(IDC_CHECKBOX_MOV);
  WORD show = IS_CHECKED(IDC_RADIOBUTTON_SHOW_HIDE);
  tmpProp->type2 = MAKELONG(move, show);

  PropertyBmp* po = dynamic_cast<PropertyBmp*>(tmpProp);

  GET_INT(IDC_TIMECYCLE, po->timeCycle);

  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmDialogBmp::checkEnabled()
{
//  bool enable = IS_CHECKED(IDC_CHECKBOX_MOV);
//  ENABLE(IDC_BUTTON_MOVIM, enable);
  int sel = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_FLAG_TYPE), CB_GETCURSEL, 0, 0);
  bool enable = sel == bAnimSeq;
  ENABLE(IDC_RADIOBUTTON_ALTERNATE, enable);
  ENABLE(IDC_RADIOBUTTON_SHOW_HIDE, enable);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmDialogBmp::chooseBmp()
{
  PropertyBmp* pt = dynamic_cast<PropertyBmp*>(tmpProp);
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
      checkHeight();
      }
    }
}
//-----------------------------------------------------------
#if 0
class svmMovingData : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmMovingData(bmpMoveInfo& mInfo, PWin* parent, uint id = IDD_BMP_MOVIM, HINSTANCE hInst = 0);
    ~svmMovingData();

    virtual bool create();
  protected:
    bmpMoveInfo& mInfo;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fill();
    void chooseNorm(int idBtn);
};
//-----------------------------------------------------------
void svmDialogBmp::movingData()
{
  PropertyBmp* pt = dynamic_cast<PropertyBmp*>(tmpProp);
  if(pt) {
    bmpMoveInfo* pI = pt->mInfo;
    if(!pI)
      pI = new bmpMoveInfo;

    pI->rect.left = pt->Rect.left;
    pI->rect.top = pt->Rect.top;
    if(IDOK == svmMovingData(*pI, this).modal()) {
      if(!pt->mInfo)
        pt->mInfo = pI;
      }
    else if(!pt->mInfo)
      delete pI;
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmMovingData::svmMovingData(bmpMoveInfo& mInfo, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), mInfo(mInfo)
{

}
//-----------------------------------------------------------
svmMovingData::~svmMovingData()
{
  destroy();
}
//-----------------------------------------------------------
bool svmMovingData::create()
{
  if(!baseClass::create())
    return false;
  fill();
  return true;
}
//-----------------------------------------------------------
LRESULT svmMovingData::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_BUTTON_NORMALIZ_MIN_X:
        case IDC_BUTTON_NORMALIZ_MIN_Y:
        case IDC_BUTTON_NORMALIZ_MAX_X:
        case IDC_BUTTON_NORMALIZ_MAX_Y:
        case IDC_BUTTON_NORMALIZ_CURR_X:
        case IDC_BUTTON_NORMALIZ_CURR_Y:
          chooseNorm(LOWORD(wParam));
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
#define FULL_ROW(a) { IDC_COMBOBOX_PERIFS_##a, IDC_EDIT_ADDR_##a, IDC_COMBOBOX_TYPEVAL_##a, IDC_EDIT_NORM_##a }
//-----------------------------------------------------------
void svmMovingData::CmOk()
{
  struct lk {
    int idPrf;
    int idAddr;
    int idType;
    int idNorm;
    };

  const lk LK[] = {
    FULL_ROW(MIN_X),
    FULL_ROW(MIN_Y),
    FULL_ROW(MAX_X),
    FULL_ROW(MAX_Y),
    FULL_ROW(CURR_X),
    FULL_ROW(CURR_Y),
    };

  for(int i = 0; i < SIZE_A(LK); ++i) {
    HWND hwnd = GetDlgItem(*this, LK[i].idPrf);
    mInfo.vI[i].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    hwnd = GetDlgItem(*this, LK[i].idType);
    mInfo.vI[i].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    GET_INT(LK[i].idAddr, mInfo.vI[i].addr);
    GET_INT(LK[i].idNorm, mInfo.vI[i].normaliz);
    }

  int x;
  int y;
  int w;
  int h;
  GET_INT(IDC_EDIT_X, x);
  GET_INT(IDC_EDIT_Y, y);
  GET_INT(IDC_EDIT_W, w);
  GET_INT(IDC_EDIT_H, h);
  mInfo.rect = PRect(0, 0, R__X(w), R__Y(h));
  mInfo.rect.Offset(R__X(x), R__Y(y));
  mInfo.dir = 0;
  if(IS_CHECKED(IDC_CHECKBOX_REVERSE_X))
    mInfo.dir |= 1;
  if(IS_CHECKED(IDC_CHECKBOX_REVERSE_Y))
    mInfo.dir |= 2;

  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmMovingData::fill()
{
  struct lk {
    int idPrf;
    int idAddr;
    int idType;
    int idNorm;
    };

  const lk LK[] = {
    FULL_ROW(MIN_X),
    FULL_ROW(MIN_Y),
    FULL_ROW(MAX_X),
    FULL_ROW(MAX_Y),
    FULL_ROW(CURR_X),
    FULL_ROW(CURR_Y),
    };

  for(int i = 0; i < SIZE_A(LK); ++i) {
    HWND hwnd = GetDlgItem(*this, LK[i].idPrf);
    fillCBPerif(hwnd, mInfo.vI[i].perif);

    hwnd = GetDlgItem(*this, LK[i].idType);
    fillCBTypeVal(hwnd, mInfo.vI[i].typeVal);

    SET_INT(LK[i].idAddr, mInfo.vI[i].addr);
    SET_INT(LK[i].idNorm, mInfo.vI[i].normaliz);
    }

  SET_INT(IDC_EDIT_X, REV__X(mInfo.rect.left));
  SET_INT(IDC_EDIT_Y, REV__Y(mInfo.rect.top));
  SET_INT(IDC_EDIT_W, REV__X(mInfo.rect.Width()));
  SET_INT(IDC_EDIT_H, REV__Y(mInfo.rect.Height()));

  if(mInfo.dir & 1)
    SET_CHECK(IDC_CHECKBOX_REVERSE_X);
  if(mInfo.dir & 2)
    SET_CHECK(IDC_CHECKBOX_REVERSE_Y);
}
//-----------------------------------------------------------
void svmMovingData::chooseNorm(int idBtn)
{
  struct lk {
    int ibtn;
    int ied;
    };
  const lk LK[] = {
    { IDC_BUTTON_NORMALIZ_MIN_X, IDC_EDIT_NORM_MIN_X },
    { IDC_BUTTON_NORMALIZ_MIN_Y, IDC_EDIT_NORM_MIN_Y },
    { IDC_BUTTON_NORMALIZ_MAX_X, IDC_EDIT_NORM_MAX_X },
    { IDC_BUTTON_NORMALIZ_MAX_Y, IDC_EDIT_NORM_MAX_Y },
    { IDC_BUTTON_NORMALIZ_CURR_X, IDC_EDIT_NORM_CURR_X },
    { IDC_BUTTON_NORMALIZ_CURR_Y, IDC_EDIT_NORM_CURR_Y },
    };
  int ix = -1;
  for(int i = 0; i < SIZE_A(LK); ++i)
    if(LK[i].ibtn == idBtn) {
      ix = i;
      break;
      }
  if(ix < 0)
    return;

  uint result;
  GET_INT(LK[ix].ied, result);
  if(svmSetupNormal(this, result))
    SET_INT(LK[ix].ied, result);
}
#endif
