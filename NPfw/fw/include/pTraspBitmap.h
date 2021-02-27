//-------------------- pTraspBitmap.h ---------------------------
//-----------------------------------------------------------
#ifndef PTRASPBITMAP_H_
#define PTRASPBITMAP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PBITMAP_H_
  #include "pBitmap.h"
#endif
//-----------------------------------------------------------
#define SAME_SCALE -0.000000001
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PTraspBitmapBase
{
  public:
    PTraspBitmapBase(PWin* parent, const POINT& pt);

    virtual ~PTraspBitmapBase();

    void moveToSimple(const POINT& newpos);

    void moveTo(const POINT& newpos);
    void setScale(double scaleX, double scaleY = SAME_SCALE);
    void Invalidate();

    void setEnable(bool set = true);

    // metodo da ridefinire per assegnare bitmap da disegnare,
    // è sufficiente che richiami drawBitmap()
    virtual void Draw(HDC hdc) = 0;

    // accessor
    const POINT& getPoint();  // coordinate dell'oggetto
    double getScaleX();  // scalatura attuale
    double getScaleY();  // scalatura attuale
    bool isEnabled();
    virtual PRect getRect() const = 0;

  protected:
    PWin *Parent;
    POINT Pos;
    double ScaleX;
    double ScaleY;
    bool Enabled;

    void drawBitmap(HDC hdc, const PBitmap& bmp);
    void drawBitmap(Gdiplus::Graphics& graphics, const PBitmap& Bmp);

    // costruttore di default, necessario per operatore di assegnazione
    // delle classi derivate
    PTraspBitmapBase() {}
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline const POINT& PTraspBitmapBase::getPoint() { return Pos; }
//-----------------------------------------------------------
inline double PTraspBitmapBase::getScaleX(){ return ScaleX; }
//-----------------------------------------------------------
inline double PTraspBitmapBase::getScaleY(){ return ScaleY; }
//-----------------------------------------------------------
//inline bool PTraspBitmapBase::isVisible() { return !Hided; }
//-----------------------------------------------------------
inline bool PTraspBitmapBase::isEnabled() { return Enabled; }
//-----------------------------------------------------------
inline void PTraspBitmapBase::setEnable(bool set)
{
  Enabled = set;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
class IMPORT_EXPORT PTraspBitmap : public PTraspBitmapBase
{
  public:
    PTraspBitmap(PWin* parent, PBitmap *bmp, const POINT& pt, bool autodelete = false);

    virtual ~PTraspBitmap();

    virtual void Draw(HDC hdc);
    virtual void Draw(Gdiplus::Graphics& graphics);

    virtual PRect getRect() const;
  protected:
    const PBitmap* getBmp() const;
  private:
    PBitmap *Bmp;
    bool autoDelete;

  public:
    // costruttore di copia e assegnazione
    PTraspBitmap(const PTraspBitmap&);
    PTraspBitmap& operator = (const PTraspBitmap&);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline const PBitmap* PTraspBitmap::getBmp() const
{
  return Bmp;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
