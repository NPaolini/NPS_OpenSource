//-------------------- PSplitter.h --------------------------
//-----------------------------------------------------------
#ifndef PSPLITTER_H_
#define PSPLITTER_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
#define PERC_FROM_FLOAT(v) ((uint)((v) * 1000))
//-----------------------------------------------------------
class IMPORT_EXPORT PSplitter
{
  public:
    enum style { PSW_HORZ = 1, PSW_VERT };
    enum lock { PSL_NONE, PSL_FIRST, PSL_SECOND, PSL_BOTH };
    enum styleDraw { PSD_NONE, PSD_LEFT, PSD_RIGHT, PSD_BOTH, PSD_FILL,
         PSD_FILL_LEFT, PSD_FILL_RIGHT, PSD_FILL_BOTH, };

    PSplitter(PWin* parent, PWin* w1, PWin* w2, uint perc = 500, uint width = 3, uint style = PSW_HORZ);
    virtual ~PSplitter();

    bool create(); // richiamata dalla win parent, deve sistemare le due child

    void setW1(PWin* w) { W1 = w; }
    void setW2(PWin* w) { W2 = w; }

    virtual void windowProcSplit(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void setPerc(uint perc, bool refresh = true);
    void setDim1(uint width, bool refresh = true);
    void setDim2(uint width, bool refresh = true);
    void setSplitterWidth(uint width, bool refresh = true);
    void setStyle(uint style, bool refresh = true);

    uint getPerc() const { return Perc; }
    uint getDim1() const { return Dim1; }
    uint getDim2() const;
    uint getSplitterWidth() const { return Width; }
    uint getStyle() const { return Style; }

    void update();
    bool resize();

    void copyDataTo(PSplitter* other);
    void copyDataFrom(PSplitter* other);

    PWin* getParent() const { return Parent; }
    PWin* getW1() const { return W1; }
    PWin* getW2() const { return W2; }

    void setLock(lock type) { locked = type; }
    lock whichLock() const { return locked; }

    void setDrawStyle(styleDraw sd) { drawStyle = sd; }
    styleDraw getDrawStyle() const  { return drawStyle; }

    bool setReadOnly(bool set) { bool t = ReadOnly; ReadOnly = set; return t; }
  protected:
    PWin* Parent;
    PWin* W1;
    PWin* W2;
    int Dim1;     // per una misura esatta quando ridimensionato col mouse
    uint Perc;    // in millesimi anziché in centesimi
    uint Width;   // larghezza dello splitter
    uint Style;   // verticale/orizzontale
    styleDraw drawStyle;

    int lastPos; // posizione precedente, per ripristino disegno
    uint stat;    // stato corrente, 1 == onDragging

    lock locked;
    // aggiunta per poter bloccarne la dimensione, viene usata solo come
    // valore precedente
    int Dim2;

    bool ReadOnly;
    virtual COLORREF getBkgColor() const { return GetSysColor(COLOR_BTNFACE); }
    virtual COLORREF getLightColor() const { return GetSysColor(COLOR_BTNHILIGHT); }
    virtual COLORREF getDarkColor() const { return GetSysColor(COLOR_BTNSHADOW); }
    virtual COLORREF getXoredColor() const { return RGB(0xff, 0xff, 0xff); }


    virtual void evPaint(HDC hdc);
    virtual void evPaintHorz(HDC hdc);
    virtual void evPaintVert(HDC hdc);

    void drawXorHorz(HDC hdc, int pos, const PRect& r);
    void drawXorVert(HDC hdc, int pos, const PRect& r);

    void evMouse(UINT message, WPARAM wp, LPARAM lp);
    void evMouseUp(const POINT& pt);
    void evMouseDown(const POINT& pt);

    void evMouseMoveHorz(const POINT& pt);
    void evMouseMoveVert(const POINT& pt);

    void updateVert();
    void updateHorz();

    bool calcByPerc();
    bool checkWidth(uint dimF);

};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PSplitter::PSplitter(PWin* parent, PWin* w1, PWin* w2, uint perc, uint width, uint style) :
      Parent(parent), W1(w1), W2(w2), Width(width), Perc(perc), Style(style), Dim1(-1),
      lastPos(0), stat(0), locked(PSL_NONE), Dim2(-1), drawStyle(PSD_FILL_BOTH), ReadOnly(false)
{
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
