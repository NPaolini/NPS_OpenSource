//-------- lgraph2-b.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef LGRAPH2_B_H_
#define LGRAPH2_B_H_
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "Commonlgraph.h"
#include "p_defbody.h"
#include "p_txt.h"
#include "PSimpleText.h"
#include "pgraph.h"
#include "pedit.h"
#include "lgraph2-zoom.h"
#include "pUtil_Packet.h"
//----------------------------------------------------------------------------
class TD_LGraph2b : public P_DefBody {
  public:
    TD_LGraph2b(int idPar, PWin* parent, int resId = IDD_STANDARD, HINSTANCE hinst = 0);
    virtual ~TD_LGraph2b();
    virtual P_Body* pushedBtn(int idBtn);
    virtual bool create();
    virtual void setReady(bool first);

    // non deve visualizzare alcun errore, i campi di edit sono gestiti internamente
    virtual void ShowErrorData(uint idprf, const prfData& data, prfData::tResultData result) {  }

    virtual void postSendEdit(PVarEdit* edi);

    virtual void refreshBody();

  protected:
    void evPaint(HDC hdc, const PRect& Rect);
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual PVarListBox* allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen = 255, HINSTANCE hinst = 0);

    virtual void getFileStr(LPTSTR path);

    enum fixType { fByHour, fByMinute, fByFiletime };
    fixType fxType;

    bool uniqueRange;

    bool usePrph_0;
    PSimpleText* InfoFile;

  private:
    class linear *lGraph[MAX_LINEAR_GRAPH];
    PVect<lgCoord> yGraph[MAX_LINEAR_GRAPH];
    PVect<lgCoord> XCoord;
    int maxChoose;

    COLORREF textColor;

    static bool showOver;

    void drawYInfo(HDC hdc);

//    void reload();
    void fillLB();

    void chooseGraph();
    void setTimeForTimer();
    void refreshGraph();

    void invalidateGraph();
    void makeLinear();
//    void setText(int id, LPCTSTR txt);
    void setText(int id, fREALDATA v);
//    void setText(int id, int v);
    void getText(int id, LPTSTR txt, int len);
    __int64 calcTime(int ctrlDate, int ctrlTime, bool useCurrTime = true);
    __int64 calcVal(int ctrl);

    __int64 calcInitEnd(__int64& Init);

    struct rangeSet *RangeSet;
    struct rangeSet *RangeSetOnUse;
    uint idTimerRefresh;
    double lastTotVal;
    void getFileRange(LPTSTR path);

    void saveSet(int idEdit);

    bool canUpdate;
    bool firstEnter;
    bool firstInvalidate;

    bool insideTime;
    void checkInside();
    void makeInside();

    void saveSet();
    void showSet();
    void loadSet();
    void saveDefSet();


    void makeAllGraph();
    int  makeVect();
    int makeVectStep(DWORD nTotRec, DWORD ixInit);
    int makeVectFull(DWORD nTotRec, DWORD ixInit);


    pListFile List;

    bool isInsideDate(LPCTSTR filename, const FILETIME& ftInit, const FILETIME& ftEnd, bool history);
    void insertInListFile(LPCTSTR filename, bool history);
    bool findMatchTrend();
    void replaceNameInfo();
    DWORD calcInitIx(DWORD& ixInit);
    void calcInputDatefromFile();


    lgCoord calcY(fREALDATA val, int type);

    lgCoord calcX(const LPBYTE sft);
    int  addToVect(const LPBYTE stf, int curr);
    void recalcXCoord(LPCBYTE buffStat);
    void addLastToVect();

    int codeToType(int code);

    void setDateTime(__int64 val, int ctrlDate, int ctrlTime);
    void setRangeVal(__int64 iVal, int ctrl);

    PRect getCoordGraph();
    int getDec(int id);

    struct rangeInFile
    {
      int offsStatFile;
      int dec;
      TCHAR descr[MAX_DESCR + 1];
    };

    PVect<rangeInFile*> infoStat;

    DWDATA offsStat;
    DWDATA sizeStat;
    DWDATA sizeHeader;
    int    val4X;
    int    fixedScale;
    LPBYTE BuffStat;

    TCHAR fileName[_MAX_PATH];
    FILETIME lastUpdate;

    bool makePathTrend(LPTSTR path, LPCTSTR file, bool history = false);
    bool makePathTrend(LPTSTR path, bool history = false);

    bool findLastTrend(LPTSTR file);
    fREALDATA getXVal(LPCBYTE buff);

    bool chooseFile();
    void setLimits();

    bool reCalcInit(LPCBYTE buffStat, __int64& Init);
    bool reCalcEnd(LPCBYTE buffStat, __int64& End);
    bool reCalcEnd(P_File& pf, __int64& End);
    void calcLastTot();
    void reCalcInitEnd(P_File& pf, __int64& Init, __int64& End, LPCBYTE buffStat);
    __int64 getToValue();


    void calcFixTime();
    void setFixedTime();

    void addTime(bool prev);

    void print_Graph();

    int count4refresh;
    int maxWaitBeforeRefresh;
    bool onlyRefresh;

    void exportTrend();
    bool byChooseFile;

    manage_coord* ManCoord;
    void showZoom(PRect& rect);
    bool dontRedraw;

    long maxRecord2Show;

    freeListFile FreeFile;
    WORD currDay;

    double lastCurrVal;

    void showCurrValue(POINT pt);

  private:
    typedef P_DefBody baseClass;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
