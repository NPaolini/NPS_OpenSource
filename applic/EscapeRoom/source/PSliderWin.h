//----------- PSliderWin.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PSliderWin_H_
#define PSliderWin_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "headerMsg.h"
#include "pBitmap.h"
#include "resource.h"
//----------------------------------------------------------------------------
#define REG_WM(a) const UINT WM_##a = RegisterWindowMessage(_T("WM_") _T(#a))
//----------------------------------------------------------------------------
extern const UINT WM_CUSTOM_MSG_SLIDER;
//----------------------------------------------------------------------------
  #define CM_MOVE_SLIDER        1
  #define CM_MOVE_LEFT_SLIDER   2
  #define CM_MOVE_RIGHT_SLIDER  3

  #define CM_END_MOVE_SLIDER        4
  #define CM_END_MOVE_LEFT_SLIDER   5
  #define CM_END_MOVE_RIGHT_SLIDER  6
//----------------------------------------------------------------------------
/*
  l'oggetto slider è formato da una finestra, da uno slider per la posizione
  corrente e due slider (sx, dx) per un range. Se non esiste lo slider non
  viene usato, idem per sx-dx.
  Può essere in forma orizzontale o verticale e gli slider possono essere
  posizionati con il corrente in alto e gli altri in basso o viceversa.
  Nel caso di forma verticale per alto si intende sx.
*/

struct infoSlider
{
  uint idSkin;
  uint idSlider;
  uint idLeftSlider;
  uint idRightSlider;

  enum sFormat {
    sHorzCurrTop = 0,  // orizzontale, offset verticale slider normale = 0
    sVertCurrTop = 1,  // verticale, offset orizzontale slider normale = 0

    sHorzCurrBottom = 2,  // orizzontale, offset verticale slider sx-dx = 0
    sVertCurrBottom = 3,  // verticale, offset orizzontale slider sx-dx = 0
    };
  DWORD Format;
  COLORREF Bar;
  COLORREF selBar;
};
//----------------------------------------------------------------------------
struct infoSliderBmp
{
  uint id;
  PBitmap* bmp;

  infoSliderBmp() : bmp(0), id(0) {}
  ~infoSliderBmp() { delete bmp; }

  private:
    const infoSliderBmp& operator=(const infoSliderBmp&);
    infoSliderBmp(const infoSliderBmp&);
};
//----------------------------------------------------------------------------
class PSliderWin : public PWin
{
  public:
    PSliderWin(PWin * parent, uint id, int x, int y, int w, int h, const infoSlider& is,
            LPCTSTR title = 0, HINSTANCE hinst = 0);

    PSliderWin(PWin * parent, uint id, const PRect& r, const infoSlider& is,
            LPCTSTR title = 0, HINSTANCE hinst = 0);

    ~PSliderWin ();

    virtual bool create();
    void setLimits(DWORD left, DWORD right);
    void getLimits(DWORD& left, DWORD& right) { right = Right; left = Left; }

    void setSelections(DWORD left, DWORD right);
    void getSelections(DWORD& left, DWORD& right) { right = RightSel; left = LeftSel; }

    void setValue(DWORD val);
    DWORD getValue() const { return Value; }

    void hideShow(uint bitHide, uint bitShow);

  protected:

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evPaint(HDC hdc, bool eraseBkg);
    void evPaintBkg(HDC hdc);
    void evMouse(UINT message, WPARAM wParam, LPARAM lParam);

    void evMouseMove(const POINT& pt);
    void evMouseLDown(const POINT& pt);
    void evMouseLUp(const POINT& pt);

  private:
    infoSlider IS;
    DWORD Value;
    DWORD Left;
    DWORD Right;

    DWORD LeftSel;
    DWORD RightSel;

    DWORD BitsHide;

    enum whichDrag { dNone, dLeft, dRight, dValue, dMaxDrag };
    HCURSOR Curs[dMaxDrag];
    DWORD onDragging;
    DWORD onAnchor;

    // è il rapporto tra i pixel utili e il range di valori
    double Ratio;

    void calcRatio();
    bool isOverObject(uint idc, DWORD offset, const POINT& pt);
    PRect getRectObject(uint idc, DWORD offset);
    void pointToValue(const POINT& pt, const SIZE& sz);
    PRect getOldRect();
    void paintSlider(HDC hdc, const PRect& r, uint idc, DWORD format, DWORD val);
    void setPointer(uint type);

    PRect getPaintRect();
    SIZE szMargin;
    typedef PWin baseClass;

    void init();
    infoSliderBmp iSlBmp[4];
    PBitmap* getBitmap(uint id);
//    enum { slVal, slLeft, slRight };
    enum { slSkin, slVal, slLeft, slRight };
    bool isVisible(uint which);
};
//-----------------------------------------------------------
#endif
