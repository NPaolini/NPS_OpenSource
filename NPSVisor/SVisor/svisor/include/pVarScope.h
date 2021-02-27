//-------- PVarScope.h --------------------------------------------------------
#ifndef PVARSCOPE_H_
#define PVARSCOPE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

#include "pstatic.h"
#include "setPack.h"
//----------------------------------------------------------------------------
struct infoScope
{
  COLORREF Bkg;
  COLORREF grid;
  COLORREF axe;
  // linea in costruzione
  COLORREF lineShow1;
  // linea precedente (alla dx di quella in costruzione)
  COLORREF lineShow2;

  // il tipo è sempre dword e indica la posizione corrente di scrittura nel plc
  uint addrReadyPos;

  // memorizzati in variabili
  // numero totale di dati, deve essere un multiplo del numero dei blocchi
  uint totData;

  double minVal;
  double maxVal;
  // numero di blocchi in cui sono suddivisi i dati
  uint nBlock;

  // numero di righe per griglia
  uint nRow;

  bool onlyOneData;

  // nuovi dati per la gestione dell'asse x variabile
  bool useVariableX;
  // se i blocchi sono valori assoluti o relativi
  bool relativeBlock;
  // se i dati iniziano da zero o dal primo utile
  bool zeroOffset;
  double currVal;

  infoScope() : Bkg(RGB(0,0,0)), grid(RGB(0,0,0)), axe(RGB(255,0,0)),
      lineShow1(RGB(255,255,255)), lineShow2(RGB(255,255,0)),
      addrReadyPos(0), totData(0), nBlock(0), nRow(0),
      minVal(0), maxVal(100.0), onlyOneData(false), useVariableX(false),
      relativeBlock(false), zeroOffset(true), currVal(0) { }
};
//----------------------------------------------------------------------------
union uScopeShow
{
  struct dataShow
  {
    DWORD x;
    float v;
  } ds;
  POINT pt;
};
//----------------------------------------------------------------------------
class PVarScope : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    PVarScope(P_BaseBody* owner, uint id);

    virtual ~PVarScope();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    virtual void addReqVar(PVect<P_Bits*>& allBits);

    bool canCacheReq(const PVect<bool>& sPrph);
  protected:
    virtual void performDraw(HDC hdc);
    virtual uint getBaseId() const { return ID_INIT_VAR_SCOPE; }

  private:
    PPanel* Panel;

    pVariable vTotData;
    pVariable vNBlock;
    pVariable vEnableRead;
    pVariable vMinVal;
    pVariable vMaxVal;

    pVariable vAxesX;
    pVariable vCurrX;

    int currPos;

    infoScope IC;

    double* Wave;
    double* stepX;
    LPDWORD Buff;

    void calcGrid();
    void Grid(HDC hdc);
    void restore_point(POINT& point, uint ix);
    void draw_wave(HDC hdc);
    void drawInterp(HDC hdc, int ix);

    void loadColors(int idColors);
    bool readWave(bool needRefresh);
    bool readWaveX(bool needRefresh);
    bool readOnlyOne(bool clear);

    PRect getInflated();
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

