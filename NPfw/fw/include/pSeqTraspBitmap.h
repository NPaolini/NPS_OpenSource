//-------------------- pSeqTraspBitmap.h ---------------------------
//-----------------------------------------------------------
#ifndef PSEQTRASPBITMAP_H_
#define PSEQTRASPBITMAP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PTRASPBITMAP_H_
  #include "pTraspBitmap.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PSeqTraspBitmap : public PTraspBitmapBase
{
  public:
    PSeqTraspBitmap(PWin* parent, PBitmap **bmp, UINT nBmp, const POINT& pt);

    virtual ~PSeqTraspBitmap();

    void setCurr(UINT curr, bool repaint = true);

    // non ha senso il flag di repaint, se repaint == false, si può
    // usare quella sopra, visto che non si opera nessun ridisegno
    void setCurr(UINT curr, HDC hdc);

    virtual void Draw(HDC hdc);

    // fornisce un nuovo bitmap e torna quello precedente,
    // torna zero se [pos] eccede la capacità o se le dimensioni
    // sono diverse
    PBitmap* replaceBmp(PBitmap* other, uint pos);

    // accessor
    UINT getCurr();     // numero del Dib corrente
    UINT getNBmp();     // numero totale di Dib
    PBitmap* getBmp(uint pos); // torna zero se pos >= nBmp


    bool setTransp(bool set) { bool old = dontUseTransp; dontUseTransp = !set; return old; }
    virtual PRect getRect() const;
  private:
    PBitmap **Bmp;
    UINT currBmp;
    UINT nBmp;
    // se occorre una sequenza che non necessita di trasparenza si può settare
    // questo flag
    bool dontUseTransp;
//    void FullDraw();
    void FullDraw(HDC hdc);
  public:
    // costruttore di copia e assegnazione
    PSeqTraspBitmap(const PSeqTraspBitmap&);
    PSeqTraspBitmap& operator = (const PSeqTraspBitmap&);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline UINT PSeqTraspBitmap::getCurr()   { return currBmp; }
//-----------------------------------------------------------
inline UINT PSeqTraspBitmap::getNBmp()   { return nBmp; }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
