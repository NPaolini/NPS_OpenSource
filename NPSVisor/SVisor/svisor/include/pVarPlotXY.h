//-------- PVarPlotXY.h --------------------------------------------------------
#ifndef PVarPlotXY_H_
#define PVarPlotXY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

#include "pstatic.h"
#include "setPack.h"
//----------------------------------------------------------------------------
struct infoPlotXY
{
  COLORREF Bkg;
  COLORREF grid;
  COLORREF axe;
  // linea in costruzione
  COLORREF lineShow1;
  // linea precedente (alla dx di quella in costruzione)
  COLORREF lineShow2;

  // memorizzati in variabili
  // numero totale di dati, deve essere un multiplo del numero dei blocchi
  uint totData;

  double minVal_X;
  double maxVal_X;
  double minVal_Y;
  double maxVal_Y;

  // numero di colonne per griglia
  uint nCol;
  // numero di righe per griglia
  uint nRow;

  // se i valori sono assoluti o relativi
  bool relative_X;
  bool relative_Y;

  // messi come appoggio per evitare di ricalcolarli  per ogni punto
  double ratioX;
  double ratioY;
  PRect R;

  infoPlotXY() : Bkg(RGB(0,0,0)), grid(RGB(0,0,0)), axe(RGB(255,0,0)),
      lineShow1(RGB(255,255,255)), lineShow2(RGB(255,255,0)),
      totData(0), nCol(0), nRow(0),
      minVal_X(0), maxVal_X(100.0), minVal_Y(0), maxVal_Y(100.0),
      relative_X(false), relative_Y(false), ratioX(1), ratioY(1)
      { }
};
//----------------------------------------------------------------------------
union uPlotXYShow
{
  struct dataShow
  {
    DWORD x;
    float v;
  } ds;
  POINT pt;
};
//----------------------------------------------------------------------------
class PVarPlotXY : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    PVarPlotXY(P_BaseBody* owner, uint id);

    virtual ~PVarPlotXY();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    virtual void addReqVar(PVect<P_Bits*>& allBits);

    bool canCacheReq(const PVect<bool>& sPrph);
  protected:
    virtual void performDraw(HDC hdc);
    virtual uint getBaseId() const { return ID_INIT_VAR_PLOT_XY; }

  private:
    PPanel* Panel;

    pVariable vTotData;
    pVariable vEnableRead;

    pVariable vInitX;
    pVariable vMinVal_X;
    pVariable vMaxVal_X;

    // initY == base
    pVariable vMinVal_Y;
    pVariable vMaxVal_Y;

    pVariable vCurr;

    int currPos;

    infoPlotXY IC;

    double* Wave_Y;
    double* Wave_X;
    LPDWORD Buff;

    void Grid(HDC hdc);
    void restore_point(POINT& point, uint ix);
    void draw_wave(HDC hdc);

    void loadColors(int idColors);
    bool readWaveY(bool needRefresh);
    bool readWaveX(bool needRefresh);
    void fillWave(double* target, double minV, double maxV);
    bool readWave(double* target, pVariable& var, bool relative, bool needRefresh);

    bool checkMinMax(bool& force, pVariable& var, double& val);

    PRect getInflated();
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

