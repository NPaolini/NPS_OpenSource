//-------------------- PPanel.H ---------------------------
#ifndef PPANEL_H_
#define PPANEL_H_
//-----------------------------------------------------------
#include "precHeader.h"

#ifndef PDEF_H_
  #include "pDef.h"
#endif

#ifndef PRECT_H_
  #include "prect.h"
#endif
#ifndef PBITMAP_H_
  #include "pBitmap.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PPanelBorder
{
  public:
    enum eside { pbTop, pbRight, pbBottom, pbLeft };
    enum estyle { pbOff, pbOn };
    struct pbInfo
    {
      estyle style;
      uint tickness;
      COLORREF color;
      pbInfo() : style(pbOff), tickness(0), color(0) {}
    };
    PPanelBorder() : SimpleBlack(true) {}
    bool isSimple() const { return SimpleBlack; }
    void setSimple(bool set) { SimpleBlack = set; }
    void setBorder(eside which, const pbInfo& info) { Info[which] = info; SimpleBlack = false; }
    const pbInfo& getInfo(eside which) const { return Info[which]; }
    void paint(HDC hdc, const PRect& rect);
  private:
    #define maxSide (pbLeft + 1)
    pbInfo Info[maxSide];
    #undef maxSide
    bool SimpleBlack;
};
//-----------------------------------------------------------
class IMPORT_EXPORT PPanel
{
  public:

    enum bStyle {
         NO,              // senza bordo
         BORDER = 1 << 0, // bordo semplice (nero)
         UP = 1 << 1,     // simula un pannello rialzato
         DN = 1 << 2,     // simula un pannello incassato

         FILL = 1 << 3,   // flag di riempimento

         // unione dei vari bordi con il riempimento
         BORDER_FILL = BORDER | FILL,
         UP_FILL = UP | FILL,
         DN_FILL = DN | FILL,

         TRANSP = 1 << 4, // memorizza lo sfondo e lo ripristina

         // unione dei vari bordi con il trasparente
         BORDER_TRANSP = BORDER | TRANSP,
         UP_TRANSP = UP | TRANSP,
         DN_TRANSP = DN | TRANSP,
         };

    PPanel(const PRect& rect, COLORREF color, bStyle type);

    PPanel(const PPanel& other) : Bkg(0) { clone(other); }
    const PPanel& operator=(const PPanel& other) { clone(other); return *this; }

    virtual ~PPanel() { delete Bkg; }

    // da richiamarsi nell'evento paint, ricarica lo sfondo (se transparent)
    // e poi richiama draw() N.B. non va usata se tutto il repaint avviene nell'evento
    virtual void paint(HDC hdc);
    // da richiamarsi per il ridisegno forzato (con lo sfondo già caricato)
    virtual void draw(HDC hdc);
    void draw2(HWND win);

    virtual void hide(bool set);

    bool isHided() const;

    void setRect(const PRect &rect);
    void setBkgColor(COLORREF newColor);
    void setStyle(bStyle add, bStyle remove);

    void move(int offx, int offy);
    const PRect& getRect() const { return Rect; }
    PRect getRect() { return Rect; }

//  protected:
    COLORREF getColor() const;
    bStyle getStyle() const;

    bool isSimple() const { return Border.isSimple(); }
    void setSimple(bool set) { Border.setSimple(set); }
    void setBorder(PPanelBorder::eside which, const PPanelBorder::pbInfo& info) { Border.setBorder(which, info); }
    const PPanelBorder::pbInfo& getInfo(PPanelBorder::eside which) { return Border.getInfo(which); }

    void setAllBorder(const PPanelBorder& bdr) { Border = bdr; }
    const PPanelBorder& getAllBorder() { return Border; }
  protected:
    void clone(const PPanel& other);
  private:
    bool Hide;
    PRect Rect;
    COLORREF Color;
    bStyle Style;
    PBitmap* Bkg;
    PPanelBorder Border;
};
//-----------------------------------------------------------
inline void PPanel::setRect(const PRect& rect)
{
  Rect = rect;
}
//-----------------------------------------------------------
inline void PPanel::setBkgColor(COLORREF newColor)
{
  Color = newColor;
}
//-----------------------------------------------------------
inline void PPanel::setStyle(bStyle add, bStyle remove)
{
  Style = static_cast<bStyle>(Style | add);
  Style = static_cast<bStyle>(Style & ~remove);
}
//-----------------------------------------------------------
inline void PPanel::move(int offx, int offy)
{
  Rect.Offset(offx, offy);
}
//-----------------------------------------------------------
inline void PPanel::hide(bool set)
{
  Hide = set;
}
//-----------------------------------------------------------
inline bool PPanel::isHided() const
{
  return Hide;
}
//-----------------------------------------------------------
inline COLORREF PPanel::getColor() const
{
  return Color;
}
//-----------------------------------------------------------
inline PPanel::bStyle PPanel::getStyle() const
{
  return Style;
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
