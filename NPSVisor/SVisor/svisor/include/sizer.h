//--------------- sizer.h ------------------------------------
#ifndef SIZER_H__
#define SIZER_H__
//-----------------------------------------------------------------
#include "precHeader.h"
#include "setPack.h"
//-----------------------------------------------------------------
// Gestisce le dimensioni della finestra del supervisore.
// Occorre chiamare le routine di questa classe anziché la
// Api -> GetSystemMetrics(SM_C?SCREEN)
//-----------------------------------------------------------------
class sizer
{
  public:
    static UINT getWidth();
    static UINT getHeight();
    static SIZE getDim();
    static double getRatioX();
    static double getRatioY();

    enum eSizer {
      s640x480, s800x600, s1024x768, s1280x1024, s1440x900, s1600x1200, s1680x1050,
      s1920x1440, sOutOfRange, sPersonalized = 100 };

    static bool setDim(eSizer newdim, const SIZE* sz = 0, bool ignoreByMaker = false);
    static bool setDefault(eSizer newdef, const SIZE* sz = 0);

    static void setCurrAsDefault();

    static eSizer getCurr(SIZE& sz);
    static eSizer getDefault(SIZE& sz);

    static void init();
  private:
    static SIZE DimScreen;
    static double ratioX;
    static double ratioY;

    static SIZE DimDefault;
    static sizer sizerInit;
    sizer();

    static void calcRatio();

    static eSizer currSizerDim;
    static eSizer currSizerDef;

    static void setPersonalizedDim(const SIZE& sz);
    static void setPersonalizedDefault(const SIZE& sz);
};
//-----------------------------------------------------------------
//-----------------------------------------------------------------
inline UINT sizer::getWidth() { return DimScreen.cx; }
//-----------------------------------------------------------------
inline UINT sizer::getHeight() { return DimScreen.cy; }
//-----------------------------------------------------------------
inline SIZE sizer::getDim() { return DimScreen; }
//-----------------------------------------------------------------
inline double sizer::getRatioX() { return ratioX; }
//-----------------------------------------------------------------
inline double sizer::getRatioY() { return ratioY; }
//-----------------------------------------------------------------
//-----------------------------------------------------------------
#define REV__X(a) ((int)(((double)(a) + 0.5) / sizer::getRatioX()))
#define REV__Y(a) ((int)(((double)(a) + 0.5) / sizer::getRatioY()))
//-----------------------------------------------------------------
#define REV__Xf(a) ((double)(a) / sizer::getRatioX())
#define REV__Yf(a) ((double)(a) / sizer::getRatioY())
//-----------------------------------------------------------------
#define R__X(a) ((int)(sizer::getRatioX() * (a) + 0.5))
#define R__Y(a) ((int)(sizer::getRatioY() * (a) + 0.5))

#define R__Xf(a) (sizer::getRatioX() * (a))
#define R__Yf(a) (sizer::getRatioY() * (a))
//-----------------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------------
#endif
