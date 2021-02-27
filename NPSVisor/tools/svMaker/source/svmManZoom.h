//------------------ svmManZoom.h ---------------------------
//-----------------------------------------------------------
#ifndef svmManZoom_H_
#define svmManZoom_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#define MAX_NAME_RELPATH _MAX_PATH
#define MAX_PAGES 8
//-----------------------------------------------------------
class svmManZoom
{
  public:
    enum zoomX { z_1_2, z_2_3, zOne, z_3_2, z_2, z_3, z_4, z_Max };

    svmManZoom() : CurrZoom(zOne), Mul(1), Div(1) {}

    zoomX getCurrZoom() { return CurrZoom; }
    void setCurrZoom(zoomX curr);

    bool calcFromScreen(PRect& r);
    bool calcFromScreen(SIZE& s);
    bool calcFromScreen(POINT& s) { return calcFromScreen(*(SIZE*)&s); }

    bool calcToScreen(PRect& r);
    bool calcToScreen(SIZE& s);
    bool calcToScreen(POINT& s) { return calcToScreen(*(SIZE*)&s); }
  private:
    zoomX CurrZoom;
    int Mul;
    int Div;

};
//-----------------------------------------------------------
#endif
