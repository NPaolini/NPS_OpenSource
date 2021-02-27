//-------------------- pOwnBtn.h ---------------------------
//-----------------------------------------------------------
#ifndef POWNBTN_H_
#define POWNBTN_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef POWNWIN_H_
  #include "pOwnWin.h"
#endif
//-----------------------------------------------------------
#ifndef PBITMAP_H_
  #include "pBitmap.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
// mH = moltiplicatore tonalità
// mS = moltiplicatore saturazione
// mV = moltiplicatore luminosità
// se grayed == true il nero viene trasformato in grigio
// viene applicata a tutti i pixel del pulsante grayed
extern COLORREF trasform(COLORREF col, float mH, float mS, float mV, bool grayed);
//-----------------------------------------------------------
void trasformBmpEdge(HDC hdc, PBitmap* bmp);
//-----------------------------------------------------------
class IMPORT_EXPORT POwnBtn : public POwnWin
{
  public:
    POwnBtn(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    POwnBtn(PWin * parent, uint id, const PRect& r, LPCTSTR text = 0, HINSTANCE hinst = 0);

    POwnBtn(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~POwnBtn();

    // se bkg == -1 -> sfondo trasparente, disegna solo il contorno del pulsante
    struct colorRect {
      COLORREF bkg;
      COLORREF focus;
      uint tickness;
      COLORREF txt;
      colorRect(COLORREF bkg = GetSysColor(COLOR_BTNFACE),
                COLORREF focus = RGB(0xff, 0, 0), int tick = 1,
                COLORREF txt = GetSysColor(COLOR_BTNTEXT));
      };

    virtual void setColorRect(const colorRect& color);
    const colorRect& getColorRect() const;
    bool preProcessMsg(MSG& msg);

    void setDrawingEdge(bool set) { drawingEdge = set; }
    void setFlat(bool set);
    bool isFlatten() const { return Flat; }

    // sono stati ereditati due rami diversi che usano bitmap e per poterli
    // utilizzare entrambi dalla classe base si è aggiunto questo metodo
    virtual PBitmap* replaceBmp(PBitmap* other, uint pos = 0) { return 0; }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    colorRect rectColor;

    bool Flat; // usa sfondo trasparente quando il mouse non è sopra
    bool Over;
    bool drawingEdge;

    virtual void mouseEnter(const POINT& pt, uint flag);
    virtual void mouseLeave(const POINT& pt, uint flag);
    void InvalidateParent();

    virtual LPCTSTR getClassName() const;
    virtual bool isOwnedDraw();

    virtual int verifyKey();
    virtual bool evKeyDown(WPARAM& key);
    virtual bool evKeyUp(WPARAM& key);
    virtual bool evChar(WPARAM& key);

    virtual bool drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis);

    // per non costringere alla ridefinizione, anziché essere virtuale pura
    // per default non esegue alcuna azione
    virtual void drawFocusBefore(HDC /*hdc*/, LPDRAWITEMSTRUCT /*dis*/) { }

    virtual void drawSelected(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual void drawUnselected(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual void drawDisabled(HDC hdc, LPDRAWITEMSTRUCT dis);

    // per default disegna una serie di puntini grigi sopra l'immagine unselected
    // modificata, richiama la trasform ed altera la luminosità e la saturazione
//    virtual void grayBtn(HDC hdc, const PRect& rect);

    virtual void drawFocusAfter(HDC hdc, LPDRAWITEMSTRUCT dis);

    // richiamata al termine di drawItem() per ulteriori personalizzazioni
    // è preferibile usare questa anziché ridefinire drawItem() perché
    // nella drawItem() si usa un memoryDC per il paint per poi
    // blittare tutto sul dc
    virtual void endDrawItem(HDC /*hdc*/, LPDRAWITEMSTRUCT /*dis*/) { }

    enum howDraw { released, pushed, disabled, pushedDisabled };

    virtual void drawText(HDC hdc, const PRect& rect, howDraw style);
    // viene chiamata dopo aver disegnato il pulsante e prima di scriverci il testo
    virtual void drawCustom(HDC, const PRect&, howDraw /*pushed*/) {}

    virtual void invalidateIf();
    virtual void drawButton(HDC hdc, howDraw style, bool withFocus);
    virtual PRect getRectDrawButton(bool withFocus) const;

    virtual void adjustRectText(const PRect& rect, PRect& calc);

  private:
    typedef POwnWin baseClass;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline POwnBtn::POwnBtn(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, text, hinst), drawingEdge(true), Flat(false), Over(false)
{
  Attr.style |= BS_OWNERDRAW;
}
//-----------------------------------------------------------
inline POwnBtn::POwnBtn(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, r, text, hinst), drawingEdge(true), Flat(false), Over(false)
{
  Attr.style |= BS_OWNERDRAW;
}
//-----------------------------------------------------------
inline POwnBtn::POwnBtn(PWin * parent, uint resid, HINSTANCE hinst) :
        baseClass(parent, resid, BS_OWNERDRAW, hinst), drawingEdge(true), Flat(false), Over(false)
{ }
//-----------------------------------------------------------
inline POwnBtn::~POwnBtn()
{
  destroy();
}
//-----------------------------------------------------------
inline LPCTSTR POwnBtn::getClassName() const { return _T("button"); }
//-----------------------------------------------------------
inline bool POwnBtn::isOwnedDraw()
{
  return BS_OWNERDRAW == (GetWindowLongPtr(*this, GWL_STYLE) & BS_OWNERDRAW);
}
//-----------------------------------------------------------
inline void POwnBtn::setColorRect(const colorRect& color)
{
  rectColor = color;
}
//-----------------------------------------------------------
inline const POwnBtn::colorRect& POwnBtn::getColorRect() const
{
  return rectColor;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline POwnBtn::colorRect::colorRect(COLORREF bkg, COLORREF focus, int tick, COLORREF txt) :
  bkg(bkg), focus(focus), tickness(tick), txt(txt)
{ }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
