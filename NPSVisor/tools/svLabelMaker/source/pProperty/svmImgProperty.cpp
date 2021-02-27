//--------------- svmImgProperty.cpp ------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmImgProperty.h"
//-----------------------------------------------------------
void imgProperty::clone(const Property& other_)
{
  baseClass::clone(other_);
  DYN_CAST(imgProperty);
  CLONE(idBmp);
  CLONE(imgType);
}
//-----------------------------------------------------------
bool imgProperty::setOtherCommon(const unionProperty& uP)
{
  bool modified = false; //baseClass::setOtherCommon(uP);
  return modified;
}
//-----------------------------------------------------------
