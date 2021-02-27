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
struct infoAdjusct
{
  double H;
  double V;
  infoAdjusct(double h =1, double v=1) : H(h), V(v) {}
};
//-----------------------------------------------------------
class svmManZoom
{
  public:
    enum zoomX { z_1_2, z_2_3, zOne, z_3_2, z_2, z_3, z_4, z_Max };

    svmManZoom() : CurrZoom(zOne), Mul(1), Div(1)  {}

    zoomX getCurrZoom() const { return CurrZoom; }
    void setCurrZoom(zoomX curr);

    bool calcFromScreen(PRect& r);
    bool calcFromScreen(SIZE& s);
    bool calcFromScreen(POINT& s) { return calcFromScreen(*(SIZE*)&s); }
    bool calcFromScreenH(LONG& s);
    bool calcFromScreenH(FLOAT& s);
    bool calcFromScreenH(double& s);
    bool calcFromScreenV(LONG& s);
    bool calcFromScreenV(FLOAT& s);
    bool calcFromScreenV(double& s);

    bool calcToScreen(PRect& r);
    bool calcToScreen(SIZE& s);
    bool calcToScreen(POINT& s) { return calcToScreen(*(SIZE*)&s); }
    bool calcToScreenH(LONG& s);
    bool calcToScreenH(FLOAT& s);
    bool calcToScreenH(double& s);
    bool calcToScreenV(LONG& s);
    bool calcToScreenV(FLOAT& s);
    bool calcToScreenV(double& s);

    static infoAdjusct getAdjuct() { return Adjuct; }
    static void setAdjuct(infoAdjusct v) { Adjuct = v; }
  private:
    zoomX CurrZoom;
    int Mul;
    int Div;
    static infoAdjusct Adjuct;

};
//-----------------------------------------------------------
#endif
