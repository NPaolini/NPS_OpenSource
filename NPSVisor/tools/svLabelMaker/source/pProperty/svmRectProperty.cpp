//--------------- svmRectProperty.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmRectProperty.h"
//-----------------------------------------------------------
void rectProperty::clone(const Property& other_)
{
  baseClass::clone(other_);
  DYN_CAST(rectProperty);
  CLONE(cX);
  CLONE(cY);
}
//-----------------------------------------------------------
bool rectProperty::setOtherCommon(const unionProperty& uP)
{
  bool modified = baseClass::setOtherCommon(uP);
/* eupCX, eupCY */
  SET_MODIFIED(eupCX, cX)
  SET_MODIFIED(eupCY, cY)

  return modified;
}
//-----------------------------------------------------------
#define SET_INIT_UNION2(b, v, v2) \
    if(!(uProp.flagBits & PROP_BIT_POS(b))) { \
      uProp.v = v2; }
//-----------------------------------------------------------
void rectProperty::initUnion(unionProperty& uProp)
{
  baseClass::initUnion(uProp);
  SET_INIT_UNION(eupCX, cX)
  SET_INIT_UNION(eupCY, cY)

  SET_INIT_UNION2(eupW, Rect.right, Rect.right - Rect.left)
  SET_INIT_UNION2(eupH, Rect.bottom, Rect.bottom - Rect.top)
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "svmDialogProperty.h"
//-----------------------------------------------------------
bool svmRectDialogProperty::create()
{
  if(!baseClass::create())
    return false;

  rectProperty* rp = dynamic_cast<rectProperty*>(Prop);
  SET_INT(IDC_EDIT_CX, rp->cX);
  SET_INT(IDC_EDIT_CY, rp->cY);
  return true;
}
//-----------------------------------------------------------
void svmRectDialogProperty::CmOk()
{
  rectProperty* rp = dynamic_cast<rectProperty*>(tmpProp);
  GET_INT(IDC_EDIT_CX, rp->cX);
  GET_INT(IDC_EDIT_CY, rp->cY);
  baseClass::CmOk();
}

