//-------------------- pNumberImg.h ---------------------------
//-----------------------------------------------------------
#ifndef pNumberImg_H_
#define pNumberImg_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "fullView.h"
#include "pManTimer.h"
//-----------------------------------------------------------
#define POINT_OFFSET_IMG 10
struct infoNumberImg
{
  uint totNumber; // numero di cifre
  uint nDec;      // di cui decimali
  uint totImg;    // immagini disponibili (ora 11, per future espansioni). La sequenza attuale è 0-9 . -
  bool sign;      // se deve usare il segno '-' (e quindi lasciare uno spazio vuoto se positivo)
  COLORREF background; // se (DWORD)-1 allora è trasparente, altrimenti il colore passato
  PVect<PBitmap*> img; // immagini allocate, devono avere tutte la stessa dimensione ad eccezione del punto
  infoNumberImg() : totNumber(0), nDec(0), totImg(0), sign(false), background((DWORD)-1) {}
};
//-----------------------------------------------------------
class pNumberImg
{
  public:
    pNumberImg(const infoNumberImg& info) : Info(info) {}

    virtual void draw(HDC hdc, int number, const POINT& pt, const SIZE& sz);
  protected:
    const infoNumberImg& Info;
};
//-------------------------------------------------------------------
class P_TimerLed : public fullView
{
  private:
    typedef fullView baseClass;
  public:
    P_TimerLed(PWin* parent, const manageTimer& mt, LPCTSTR title = 0, HINSTANCE hInst = 0) :
        baseClass(parent, title, hInst), CW(0), Img(0), ManTimer(mt) { }

    P_TimerLed(PWin * parent, const manageTimer& mt, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst), CW(0), Img(0), ManTimer(mt) { }

    P_TimerLed(PWin * parent, const manageTimer& mt, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst), CW(0), Img(0), ManTimer(mt) { }

    ~P_TimerLed();

    bool create();
    void reset();
    void resetImg();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evPaint(HDC hdc, const PRect& r);
    virtual void resize();
    infoNumberImg Info;
    pMDC_custom mdc;
    class clientWin* CW;
    PBitmap* Img;
    const manageTimer& ManTimer;
};
//-----------------------------------------------------------
#endif
