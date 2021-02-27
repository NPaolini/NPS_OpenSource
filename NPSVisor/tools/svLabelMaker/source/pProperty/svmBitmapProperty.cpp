//--------------- svmBitmapProperty.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmBitmapProperty.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
void bitmapProperty::clone(const Property& other_)
{
  baseClass::clone(other_);
  DYN_CAST(bitmapProperty);
  CLONE(idBmp);
}
//-----------------------------------------------------------
bool bitmapProperty::setOtherCommon(const unionProperty& uP)
{
  return false;
}
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
svmBitmapDialogProperty::~svmBitmapDialogProperty()
{
  destroy();
}
//-----------------------------------------------------------
bool svmBitmapDialogProperty::create()
{
  if(!baseClass::create())
    return false;
  bitmapProperty* rp = dynamic_cast<bitmapProperty*>(Prop);
  SET_INT(IDC_EDIT_X, rp->Rect.left);
  SET_INT(IDC_EDIT_Y, -rp->Rect.top);
  SET_INT(IDC_EDIT_W, rp->Rect.Width());
  SET_INT(IDC_EDIT_H, -rp->Rect.Height());
  dual* dbmp = getDualBmp();
  LPCTSTR file = dbmp->getIt(rp->idBmp);
  if(file)
    SET_TEXT(IDC_EDIT_IMG_FILE, file);

  return true;
}
//-----------------------------------------------------------
LRESULT svmBitmapDialogProperty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_BMP:
          chooseBmp();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmBitmapDialogProperty::chooseBmp()
{
  TCHAR file[_MAX_PATH];
  GET_TEXT(IDC_EDIT_IMG_FILE, file);
  LPTSTR p = file;
  if(openFileImageWithCopy(*this, p, false))
    SET_TEXT(IDC_EDIT_IMG_FILE, file);
}
//-----------------------------------------------------------
void svmBitmapDialogProperty::CmOk()
{
  bitmapProperty* rp = dynamic_cast<bitmapProperty*>(tmpProp);
  int t;
  GET_INT(IDC_EDIT_X, t);
  rp->Rect.left = t;
  GET_INT(IDC_EDIT_Y, t);
  rp->Rect.top = -t;
  GET_INT(IDC_EDIT_W, t);
  rp->Rect.right = rp->Rect.left + t;
  GET_INT(IDC_EDIT_H, t);
  rp->Rect.bottom = rp->Rect.top - t;
  dual* dbmp = getDualBmp();
  TCHAR f[_MAX_PATH];
  GET_TEXT(IDC_EDIT_IMG_FILE, f);
  rp->idBmp = dbmp->addAndGetId(f);
  baseClass::CmOk();
}

