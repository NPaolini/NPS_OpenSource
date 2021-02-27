//-------------------- POwnBtnImage.h ---------------------------
//-----------------------------------------------------------
#ifndef POWNBTNIMAGE_H_
#define POWNBTNIMAGE_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef POWNBTN_H_
  #include "pOwnBtn.h"
#endif

#ifndef P_VECT_H_
  #include "p_Vect.h"
#endif

#ifndef PSEQTRASPBITMAP_H_
  #include "pSeqTraspBitmap.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
/*
  Disegna un pulsante dai bitmap passati.
  Se si passa un solo bitmap disegna il pulsante nello stile standard
  e disegna il bitmap all'interno. Se si passano più bitmap, il primo indica
  il pulsante non premuto, il secondo il pulsante premuto. Se si passa un terzo
  bitmap questo viene usato per disegnare il focus, altrimenti, per default,
  il focus viene rappresentato da un riquadro nero intorno a tutta l'area.
  Se si passa un quarto bitmap questo viene usato per il focus con pulsante
  premuto, mentre il terzo è usato per il focus con pulsante sollevato.
  Per default, il focus viene disegnato dopo il disegno del pulsante
  nel metodo drawFocusAfter(), mentre il metodo drawFocusBefore() è solo un
  segnaposto per eventuali specializzazioni.
  Se il pulsante non prende le dimensioni dal bitmap, i bitmap verranno
  scalati(non distorti) per adattarsi alle dimensioni passate nel ctor.

  Se non si passa nessun bitmap si richiama semplicemente la classe genitrice.
  Nel pulsante standard (quasi!) con un bitmap, questi viene disegnato a sinistra
  e l'eventuale testo centrato nello spazio rimanente a destra.

  Se si usa più di un bitmap, il testo non viene scritto.
*/
//-----------------------------------------------------------
void trasformBmp(HDC hdc, PBitmap* bmp, float mH, float mS, float mV, bool grayed);
double scaleImage(const SIZE& winSz, SIZE& bmpSz);
//-----------------------------------------------------------
class IMPORT_EXPORT POwnBtnImage : public POwnBtn
{
  public:
    POwnBtnImage(PWin * parent, uint id, int x, int y, int w, int h,
          const PVect<PBitmap*>& image, bool sizeByImage = true, LPCTSTR text = 0,
          HINSTANCE hinst = 0);
    POwnBtnImage(PWin * parent, uint id, const PRect& r,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, LPCTSTR text = 0, HINSTANCE hinst = 0);

    POwnBtnImage(PWin * parent, uint resid,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, HINSTANCE hinst = 0);

    virtual ~POwnBtnImage();

    virtual void setColorRect(const colorRect& color);

    PBitmap* replaceBmp(PBitmap* other, uint pos = 0);

  protected:
    PSeqTraspBitmap* Image;
    PBitmap** Bmp;
    bool SizeByImage;

    double oldScale;
    virtual void drawText(HDC hdc, const PRect& rect, POwnBtn::howDraw style);

    void allocImage(const PVect<PBitmap*>& image);
    virtual void setCoord(PRect& rect);

    virtual void drawSelected(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual void drawUnselected(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual void drawDisabled(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual void drawFocusAfter(HDC hdc, LPDRAWITEMSTRUCT dis);

    virtual void drawCustom(HDC hdc, const PRect& rect, POwnBtn::howDraw style);
  private:
    void adjuctImage(const SIZE& winSz);
    typedef POwnBtn baseClass;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline
POwnBtnImage::POwnBtnImage(PWin * parent, uint id, int x, int y, int w, int h,
            const PVect<PBitmap*>& image, bool sizeByImage, LPCTSTR text, HINSTANCE hinst) :
      POwnBtn(parent, id, x, y, w, h, text, hinst), SizeByImage(sizeByImage)

{
  allocImage(image);
}
//-----------------------------------------------------------
inline
POwnBtnImage::POwnBtnImage(PWin * parent, uint id, const PRect& r,
            const PVect<PBitmap*>& image, bool sizeByImage, LPCTSTR text, HINSTANCE hinst) :
      POwnBtn(parent, id, r, text, hinst), SizeByImage(sizeByImage)
{
  allocImage(image);
}
//-----------------------------------------------------------
inline POwnBtnImage::POwnBtnImage(PWin * parent, uint resid,
            const PVect<PBitmap*>& image, bool sizeByImage, HINSTANCE hinst) :
        POwnBtn(parent, resid, hinst), SizeByImage(sizeByImage)
{
  allocImage(image);
}
//-----------------------------------------------------------
inline
PBitmap* POwnBtnImage::replaceBmp(PBitmap* other, uint pos)
{
  return Image->replaceBmp(other, pos);
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
