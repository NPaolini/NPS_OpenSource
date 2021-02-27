//-------- printGraph.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PRINTGRAPH_H_
#define PRINTGRAPH_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "printGeneric.h"
#include "pGraph.h"
#include "p_vect.h"
#include "Commonlgraph.h"
//----------------------------------------------------------------------------
#include "1.h"
//----------------------------------------------------------------------------
#define MAX_TITLE_PRINT 255
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
#define N_BIT_DWORD 32
//----------------------------------------------------------------------------
struct infoPrintGraph
{
  // il grafico viene adattato al numero di pagine richiesto
  int numPage;

  // tipo di trend, -1 = temporale, altrimenti l'id del campo per la scala X
  int trendType;

  // vengono caricati direttamente i buffer usati per la rappresentazione a video
  //const
  lgCoord* yGraph[N_BIT_DWORD]; // aumentato per l'utilizzo con l'oggetto trend
//  lgCoord* yGraph[MAX_LINEAR_GRAPH];
  //const
  lgCoord* XCoord;
  int numGraph;
  int nElem;

  // le coordinate in X sono riferite alla larghezza della finestra su video
  // moltiplicata per MUL_W_DATA (attualmente 1000)
  int pixWidth;

  // informazioni per ogni serie di dati
  struct field
  {
    // le altezze vanno da zero a 0x7fff e sono relative al range (vMin - vMax)
    float vMin;
    float vMax;
    COLORREF color;
    int tickness;
    int nDec;
    TCHAR info[MAX_DESCR + 2];
  };

  // se trend temporale vengono usate come FILETIME, altrimenti come double
  __int64 from;
  __int64 to;

  // se trend non temporale indica i decimali nel numero
  int nDec;

  // serie di dati da stampare
  PVect<field> FieldSet;

  // numero di righe per grafico (non supera comunque la spaziatura minima)
  int numRow;

  // spaziatura minima tra le righe (in decimi di mm)
  int dimMinRow;

  union {
    uint flags;
    struct {
      uint graphOver      : 1; // 1 = sovrapposto
      uint forceLabel     : 1;
      uint activeLabel    : 7;
//      uint preview        : 1;
//      uint setup          : 1;
      } f;
    } U;

  TCHAR title[MAX_TITLE_PRINT];
};
//----------------------------------------------------------------------------
struct internalInfoPrint
{
  COLORREF cAxes;
  uint tickness;

  COLORREF cHorzLine;
  uint horzStyle;

  COLORREF cVertLine;
  uint vertStyle;

  uint topGraph;

  union {
    uint flags;
    struct {
      uint alwaysYAxes    : 1; // per adesso, per non complicare i calcoli, è sempre uno
      uint drawHorzLines  : 1;
      uint drawVertLines  : 1;
      uint drawLabels     : 1;
      } f;
    } U;

  internalInfoPrint() : cAxes(RGB(0, 0, 0)), tickness(5), cHorzLine(RGB(0xc0, 0xc0, 0xc0)),
      cVertLine(RGB(0xc0, 0xc0, 0xc0)), horzStyle(PS_DASH), vertStyle(PS_SOLID),
      topGraph(100)
      {
        U.f.alwaysYAxes = 1;
        U.f.drawHorzLines = 1;
        U.f.drawVertLines = 1;
        U.f.drawLabels = 1;
      }
};
//----------------------------------------------------------------------------
#define MAX_PAGE 20
//----------------------------------------------------------------------------
#define BASE_VERT_LINE   6
#define VERT_LINE_TIME  12
#define VERT_LINE_OTHER 10
//----------------------------------------------------------------------------
#define MAX_VERT_LINE_TIME (BASE_VERT_LINE * VERT_LINE_TIME)
#define MAX_VERT_LINE_OTHER (BASE_VERT_LINE * VERT_LINE_OTHER)
//----------------------------------------------------------------------------
#define VERT_LINE (BASE_VERT_LINE * vertLineInside)
//----------------------------------------------------------------------------
class printGraph : public printGeneric
{
  private:
    typedef printGeneric baseClass;
  public:
    printGraph(PWin* par, svPrinter* printer, const infoPrintGraph& ipg);

  protected:
    virtual bool beginDoc(bool showSetup);
    virtual bool makeHeader();
//    virtual bool makeFooter();
    virtual int addRow();
  private:
    const infoPrintGraph& IPG;

    PRect currDrawing;
    internalInfoPrint IIP;

    uint lenPage[MAX_PAGE + 1];
    float ratioX;

    int colCoord[BASE_VERT_LINE + 1];
    int vertLineInside;

    virtual LPCTSTR getTitle();
//    virtual void run(bool preview, bool setup);


    void calcOffset();
    int getInit_Y_Axes();
    bool drawGraph(int ix);
    bool drawBottomLabels();
    bool drawHorzGrid_LeftLabels(int x1, int x2, int y1, int y2, int ix);
    bool drawVertGrid(bool fullArea);
    bool drawAxes(bool fullArea, int ix);

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
