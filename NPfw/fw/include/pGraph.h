//-------------------- pGraph.H ---------------------------
#ifndef PGRAPH_H_
#define PGRAPH_H_
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PPANEL_H_
  #include "ppanel.h"
#endif
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PGraph : public PPanel
{
  public:
    PGraph(const PRect& rect, COLORREF colorBkg, bStyle type,
            COLORREF colorFrg, bool horz = true, bool to_right = true);
    virtual ~PGraph() {}
    virtual void draw(HDC hdc);

    void setPerc(int p);
    int getPerc() const;

    void setColor(COLORREF c);

    bool isHorz() const;
    bool isToRight() const;

    virtual void resize(const PRect& newRect) { setRect(newRect); }
  protected:
    COLORREF getBarColor() const;
    virtual void drawBkg(HDC hdc);
    virtual void drawBar(HDC hdc);

  private:
    COLORREF bar;
    int perc;
    bool Horz;
    bool toRight;
};
//-----------------------------------------------------------
inline int PGraph::getPerc() const
{
  return perc;
}
//-----------------------------------------------------------
inline void PGraph::setColor(COLORREF c)
{
  bar = c;
}
//-----------------------------------------------------------
inline bool PGraph::isHorz() const
{
  return Horz;
}
//-----------------------------------------------------------
inline bool PGraph::isToRight() const
{
  return toRight;
}
//-----------------------------------------------------------
inline COLORREF PGraph::getBarColor() const
{
  return bar;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//typedef short int int16;
typedef long lgCoord;
//-----------------------------------------------------------
class IMPORT_EXPORT PLinearGraph : public PPanel
{
  public:
    PLinearGraph(const PRect& rect, COLORREF colorBkg, bStyle type,
            COLORREF colorFrg, bool has_axis = true);

    virtual void draw(HDC hdc);

    void setColor(COLORREF c);
    void setTick(int d);
    void setMaxPoint(int npoints);

    bool hasAxis() const;

    struct sPoint {
      lgCoord x;
      lgCoord y;
      sPoint() : x(0), y(0) {}
      sPoint(lgCoord x, lgCoord y) : x(x), y(y) {}
      };

  protected:
    // il range verticale deve essere tra 1 e rect.Height() - 2.
    // Se torna <= zero indica la fine prematura dei dati
    // il range orizzontale va da 1 a rect.Width() - 2 se nPoints == 0
    // altrimenti nPoints
    virtual sPoint getNext() = 0;

    virtual void drawAx(HDC hdc);
    virtual void drawLines(HDC hdc);

    COLORREF getLineColor() const;
    int getTick() const;
    int getNPoints() const;
  private:
    COLORREF linear;
    bool axis;
    int tick; // spessore linea, default = 1
    int nPoints;
};
//-----------------------------------------------------------
inline void PLinearGraph::setColor(COLORREF c)
{
  linear = c;
}
//-----------------------------------------------------------
inline void PLinearGraph::setTick(int d)
{
  tick = d;
}
//-----------------------------------------------------------
inline void PLinearGraph::setMaxPoint(int npoints)
{
  nPoints = npoints;
}
//-----------------------------------------------------------
inline COLORREF PLinearGraph::getLineColor() const
{
  return linear;
}
//-----------------------------------------------------------
inline int PLinearGraph::getTick() const
{
  return tick;
}
//-----------------------------------------------------------
inline int PLinearGraph::getNPoints() const
{
  return nPoints;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
