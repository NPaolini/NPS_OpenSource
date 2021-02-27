//------------------ sizer.cpp ----------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------
#include "sizer.h"
#include "defin.h"
//-----------------------------------------------------------------
SIZE sizer::DimScreen;
SIZE sizer::DimDefault;
//UINT sizer::user;
double sizer::ratioX;
double sizer::ratioY;
sizer sizer::sizerInit;
sizer::eSizer sizer::currSizerDim = sizer::s800x600;
sizer::eSizer sizer::currSizerDef = sizer::s800x600;
//-----------------------------------------------------------------
static sizer::eSizer getSizeBy(UINT w)
{
  switch(w) {
    case 640:
      return sizer::s640x480;
    case 800:
      return sizer::s800x600;
    case 1024:
      return sizer::s1024x768;
    case 1280:
      return sizer::s1280x1024;
    case 1440:
      return sizer::s1440x900;
    case 1600:
      return sizer::s1600x1200;
    case 1680:
      return sizer::s1680x1050;
    case 1920:
      return sizer::s1920x1440;
    default:
      return sizer::sOutOfRange;
    }
}
//-----------------------------------------------------------------
sizer::sizer()
{
  DimScreen.cx = GetSystemMetrics(SM_CXSCREEN);
  DimScreen.cy = GetSystemMetrics(SM_CYSCREEN);
  DimDefault.cx = (LONG)DEF_X_SCREEN;
  DimDefault.cy = (LONG)DEF_Y_SCREEN;
  currSizerDim = getSizeBy(getWidth());
  if(sOutOfRange == currSizerDim)
    currSizerDim = sPersonalized;
  calcRatio();
}
//-----------------------------------------------------------------
bool sizer::setDim(eSizer newdim, const SIZE* sz, bool ignoreByMaker)
{
  if(sz && sPersonalized == newdim) {
    setPersonalizedDim(*sz);
    return true;
    }
  int cx = GetSystemMetrics(SM_CXSCREEN);
  int cy = GetSystemMetrics(SM_CYSCREEN);
  if(ignoreByMaker) {
    cx = INT_MAX;
    cy = INT_MAX;
    }
  switch(newdim) {
    case s640x480:
      DimScreen.cx = 640;
      DimScreen.cy = 480;
      break;
    case s800x600:
      if(800 > cx || 600 > cy)
        return false;
      DimScreen.cx = 800;
      DimScreen.cy = 600;
      break;
    case s1024x768:
      if(1024 > cx || 768 > cy)
        return false;
      DimScreen.cx = 1024;
      DimScreen.cy = 768;
      break;
    case s1280x1024:
      if(1280 > cx || 1024 > cy)
        return false;
      DimScreen.cx = 1280;
      DimScreen.cy = 1024;
      break;
    case s1440x900:
      if(1400 > cx || 900 > cy)
        return false;
      DimScreen.cx = 1440;
      DimScreen.cy = 900;
      break;
    case s1600x1200:
      if(1600 > cx || 1200 > cy)
        return false;
      DimScreen.cx = 1600;
      DimScreen.cy = 1200;
      break;
    case s1680x1050:
      if(1680 > cx || 1050 > cy)
        return false;
      DimScreen.cx = 1680;
      DimScreen.cy = 1050;
      break;
    case s1920x1440:
      if(1920 > cx || 1440 > cy)
        return false;
      DimScreen.cx = 1920;
      DimScreen.cy = 1440;
      break;

    default:
      return false;
    }
  currSizerDim = newdim;
  calcRatio();
  return true;
}
//-----------------------------------------------------------------
bool sizer::setDefault(eSizer newdef, const SIZE* sz)
{
  if(sz && sPersonalized == newdef) {
    setPersonalizedDefault(*sz);
    return true;
    }
  switch(newdef) {
    case s640x480:
      DimDefault.cx = 640;
      DimDefault.cy = 480;
      break;
    case s800x600:
      DimDefault.cx = 800;
      DimDefault.cy = 600;
      break;
    case s1024x768:
      DimDefault.cx = 1024;
      DimDefault.cy = 768;
      break;
    case s1280x1024:
      DimDefault.cx = 1280;
      DimDefault.cy = 1024;
      break;
    case s1440x900:
      DimDefault.cx = 1440;
      DimDefault.cy = 900;
      break;

    case s1600x1200:
      DimDefault.cx = 1600;
      DimDefault.cy = 1200;
      break;
    case s1680x1050:
      DimDefault.cx = 1680;
      DimDefault.cy = 1050;
      break;
    case s1920x1440:
      DimDefault.cx = 1920;
      DimDefault.cy = 1440;
      break;
    default:
      return false;
    }
  currSizerDef = newdef;
  calcRatio();
  return true;
}
//-----------------------------------------------------------------
void sizer::setPersonalizedDim(const SIZE& sz)
{
  currSizerDim = sPersonalized;
  DimScreen = sz;
  calcRatio();
}
//-----------------------------------------------------------------
void sizer::setPersonalizedDefault(const SIZE& sz)
{
  currSizerDef = sPersonalized;
  DimDefault = sz;
  calcRatio();
}
//-----------------------------------------------------------------
void sizer::init()
{
  if(!DimScreen.cx)
    sizer sz;
}
//-----------------------------------------------------------------
void sizer::setCurrAsDefault()
{
  DimDefault = DimScreen;
  currSizerDef = currSizerDim;
  ratioX = 1;
  ratioY = 1;
}
//-----------------------------------------------------------------
void sizer::calcRatio()
{
  ratioX = getWidth();
  ratioX /= DimDefault.cx;

  ratioY = getHeight();
  ratioY /= DimDefault.cy;
}
//-----------------------------------------------------------------
sizer::eSizer sizer::getCurr(SIZE& sz)
{
  sz = DimScreen;
  return currSizerDim;
/*
  if(sPersonalized == currSizerDim)
    return sPersonalized;
  return getSizeBy(getWidth());
*/
}
//-----------------------------------------------------------------
sizer::eSizer sizer::getDefault(SIZE& sz)
{
  sz = DimDefault;
  return currSizerDef;
/*
  if(sPersonalized == currSizerDef)
    return sPersonalized;
  return getSizeBy(DimDefault.cx);
*/
}
//-----------------------------------------------------------------
