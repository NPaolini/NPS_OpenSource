//-------- PVarTrend.h -------------------------------------------------------
#ifndef PVarTrend_H_
#define PVarTrend_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "gestdata.h"
#include "pstatic.h"
#include "pBitmap.h"
#include "mainclient.h"
#include "pUtil_Packet.h"
#include "lgraph2-zoom.h"
#include "P_FreePacketQueue.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
struct infoTrend
{
  COLORREF Bkg;
  UINT BorderType;
  COLORREF Grid;
  COLORREF Text;
  COLORREF LineShow;
  UINT GridType;
  uint nRow;
  uint nCol;
  HFONT hFont;
  PRect Inside;
  SIZE defStr; // spazio occupato da una stringa di default, per calcoli vari

//  DWORD bitShowVert; // serie di bit per visualizzazione testi
//  DWORD bitShowHorz;
  PVect<int> gridPosX;
  PVect<int> gridPosY;
  DWORD totVar;
  TCHAR trendName[_MAX_PATH];

  infoTrend() : Bkg(0), BorderType(0), Grid(0), GridType(0), nRow(0), nCol(0),
      hFont(0), Text(0), LineShow(0), totVar(0)//, bitShowVert(0), bitShowHorz(0)
  {
    trendName[0] = 0;
    defStr.cx = 0;
    defStr.cy = 0;
  }
  enum eGridType { eNoGrid, eSolid, eDot, eDashed };
};
//----------------------------------------------------------------------------
#define myMAXFLOAT (1000000000.0)
#define myMINFLOAT (-1000000000.0)
//----------------------------------------------------------------------------
struct infoRangeTrend
{
  float vMin;
  float vMax;
  infoRangeTrend() : vMin(myMAXFLOAT), vMax(myMINFLOAT) {}
  void calc(float v) { if(v < vMin) vMin = v * 0.9f; if(v > vMax) vMax = v * 1.1f; }
};
//----------------------------------------------------------------------------
struct infoX_Coord
{
  // entrambi i valori sono relativi all'inizio del grafico
  int X;  // in pixel
  __int64 Time;
  infoX_Coord(int x = 0, __int64 time = 0) : X(x), Time(time) {}
//  infoX_Coord(int x) : X(x), Time(0) {}
  infoX_Coord(__int64 time) : X(0), Time(time) {}
};
//----------------------------------------------------------------------------
struct infoY_Coord
{
  int Y; // va ricalcolato al cambiamento di scala
  float Value; // caricato direttamente dal file
  infoY_Coord(int y = 0, float value = 0) : Y(y), Value(value) {}
  infoY_Coord(int y) : Y(y), Value(0) {}
  infoY_Coord(float value) : Y(0), Value(value) {}
};
//----------------------------------------------------------------------------
typedef PVect<infoY_Coord> ySet;
//----------------------------------------------------------------------------
struct infoY
{
  public:
    ySet Set;
    COLORREF color;
    uint tickness;
    infoY(COLORREF color = 0, uint tickness = 0) : color(color), tickness(tickness), pPT(0),
        nPoint(0), Visible(true) {}
    ~infoY() { delete []pPT; }

    void recalcPix(const PRect& rect);
    void getAutoRange(infoRangeTrend& range);
    void append(float val, const PRect& rect);
    void append(const infoY_Coord& val, const PRect& rect);

    void simpleAppend(float val);

    void paint(HDC hdc, const PVect<infoX_Coord>& XSet, int minPix);

    const infoRangeTrend& getRange() const { return Range; }
    void setRange(const infoRangeTrend& r) { Range = r; }

    void resetY() { Set.reset(); }

    bool isVisible() const { return Visible; }
    void setVisible(bool set) { Visible = set; }
    void toggleVisible() {  Visible = !Visible; }
  private:
    LPPOINT pPT;
    DWORD nPoint;
    infoRangeTrend Range;
    bool Visible;
};
//----------------------------------------------------------------------------
struct infoVar
{
  smartPointerConstString name;
  uint nDec;
  infoVar(smartPointerConstString name, uint nDec = 0) : name(name), nDec(nDec) {}
  infoVar() : nDec(nDec) {}
};
//----------------------------------------------------------------------------
#define MAX_ITEM 32
//----------------------------------------------------------------------------
struct PacketInfo
{
  enum whichSide { epi_left, epi_right };
  whichSide Side;
  // il primo valore in input contiene il valore della x (pixel) da cui ricavare gli altri dati
  float Value[MAX_ITEM];
  FILETIME Ft;
  PacketInfo() { clear(); }
  void clear() { ZeroMemory(this, sizeof(*this)); }
};
//----------------------------------------------------------------------------
inline
void FzClearPacket(PacketInfo* packet, size_t tsize)
{
  packet->clear();
}
//----------------------------------------------------------------------------
typedef P_SimpleFreePacketClear<PacketInfo, FzClearPacket> freeListPacket;
//----------------------------------------------------------------------------
extern freeListPacket& getFreePacket();
//----------------------------------------------------------------------------
struct infoVarTrend
{
  bool onRefresh; // se attivo si è nel time corrente

  PVect<infoX_Coord> X_Set;
  PVect<infoY> Y_Set;

  PVect<infoVar> varName;

  infoVarTrend() : onRefresh(false) {}

  // usate per aggiungere un singolo dato in modalità refresh
  void append(uint ix, float val, const PRect& rect);
  bool append(__int64 curr, __int64 start, __int64 width, const PRect& rect);


  // usate per aggiungere dati in lettura da file, è meglio un recalc solo alla fine
  void simpleAppend(uint ix, float val);
  void recalcPix(uint ix, const PRect& rect);

  void simpleAppend(__int64 curr);
  void recalcPix(__int64 start, __int64 width, const PRect& rect);

  bool getRange(uint ix, infoRangeTrend& r) const;
  void setRange(uint ix, const infoRangeTrend& r);

  void paint(HDC hdc, int minPix);

  bool fillPacket(struct PacketInfo* packet, int x);

  void resetXY();
  void resetRange();

  void getAutoRange(uint ix, infoRangeTrend& range);

};
//----------------------------------------------------------------------------
class PVarTrend : public PWin, virtual public baseActive
{
  private:
    typedef baseActive baseClass;
    typedef PWin baseWinClass;
  public:
    PVarTrend(P_BaseBody* owner, uint id);

    virtual ~PVarTrend();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    bool create();
    void fillPacket(PacketInfo* packet);
    virtual void setVisibility(bool set);

    virtual void addReqVar(PVect<P_Bits*>& allBits);

  protected:
    virtual void performDraw(HDC hdc) {  }
    virtual uint getBaseId() const { return ID_INIT_VAR_TREND; }
    virtual LPCTSTR getFileMinMax();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HWND getHwnd() { return getHandle(); }

    HWND evSetAroundFocus(UINT key);

    void sendCurrData();
    bool reload(bool force = false);
    bool reloadAll();

  private:

    const ImplJobMemBase::save_trend* pTrend;

    DWORD count4CheckTrend;
    bool allocObj2();

    infoVarTrend iVarTrend;
    infoTrend iTrend;

    // memorizzata nella variabile base standard
//    pVariable initRange;

    pVariable hideShowBit;
    pVariable activeBit;
    pVariable dateStart;
    pVariable hourStart;
    pVariable dateHourLen;
    pVariable variousBit;

    HBRUSH Brush;

    __int64 fLastSeek;

    FILETIME ftFrom;
    FILETIME ftTo;
    bool insideTime;
    LPBYTE BuffStat;
    bool step24h;
    bool noAutoCenter;
    bool autoStep;
    bool autoRows;

    void evPaint(HDC hdc);

    void drawBorder(HDC hdc, PRect r);
    void drawGrid(HDC hdc);
    void drawText(HDC hdc);
    void drawLeftScaleText(HDC hdc, uint ix, PRect r);
    void drawAutoRows(HDC hdc, uint ix, int x1, int x2);

    DWORD getTotVar();
    bool findTrendByName();
    void sendMsg();
    LPCTSTR addColorInfo(LPCTSTR p);

    freeListFile FreeList;
    pListFile ListFile;

    bool isInsideDate(LPCTSTR file, const FILETIME& ftStart, const FILETIME& ftEnd);
    void insertInListFile(LPCTSTR filename);

    void saveTime();
    void makeInside();
    void checkInside();

    DWORD calcInitIx(DWORD& ixInit);
    bool inExec;

    long getNumField() const { return (long)iTrend.totVar; } // ((pTrend->getLenRec() - sizeof(FILETIME)) / sizeof(float)); }

    bool makeVect();
    bool makeVectStep(DWORD nTotRec, DWORD ixInit, DWORD maxRecord2Show);
    void recalcNewRange(int nField);
    void setCurrRange(int nField);
    void saveCurrRange();
    void recalcCurrRange();
    bool appendToVect(LPBYTE buff, int nField);
    bool addToVect(LPBYTE buff, int nField);
    bool makeVectFull(DWORD nTotRec, DWORD ixInit);
    bool changedRange();

    int checkDate(prfData& data, bool check = true);

    void setVar(pVariable& var, prfData& data, bool noCommit = false);

    HBITMAP hBmpWork;
    HDC mdcWork;
    HGDIOBJ oldObjWork;

    bool allocBmpWork(HDC hdc);
    void removeBmpWork();

    void moveTime(int where);
    bool checkChangedVariousBit(bool force);
    void setTimeForTimer();

    manage_coord* manCoord;
    void showZoom(PRect& rect);
//    void resetOnZoomBit();
    int dontRedraw;
    bool onZoom;
    void removeZoom(bool send = false);

    int Left;
    int Right;
    void runShowValue();

    void exportTrend();
    void print_Graph();
    void printerSetup();
    lgCoord trasform4Comp(uint ix);

    bool showRange();

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

