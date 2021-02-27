//-------- pBaseListAlarm.h -----------------------------------------------------
#ifndef pBaseListAlarm_H_
#define pBaseListAlarm_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "pVarListAlarm.h"
#include "currAlrm.h"
//----------------------------------------------------------------------------
struct fullInfoAlarm;
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
#define MAX_COLUMN_ALARM 20
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
enum whichShowCol {
  esIdAlarm = 1 << 0,
  esIdStat  = 1 << 1,
  esDate    = 1 << 2,
  esTime    = 1 << 3,
  esFilter  = 1 << 4,
  esGroup   = 1 << 5,
  esGrpDesc = 1 << 6,
  esPrph    = 1 << 7,
  esDescr   = 1 << 8,
  esMAX_VAL = 1 << 9,
  esUseListBox = esMAX_VAL,
  };
//----------------------------------------------------------------------------
enum whichShowColSeq {
  esqIdAlarm = 0,
  esqIdStat,
  esqDate,
  esqTime,
  esqFilter,
  esqGroup,
  esqGrpDesc,
  esqPrph,
  esqDescr,
  esqMAX_VAL,
  };
//----------------------------------------------------------------------------
#define MAX_SHOW_AL esqMAX_VAL
//----------------------------------------------------------------------------
#define ID_ALARM_BIT_INIT  31
//----------------------------------------------------------------------------
class pBaseListAlarm
{
  public:
    pBaseListAlarm(P_BaseBody* owner, uint id, pAround& Around);
    virtual ~pBaseListAlarm();

    virtual bool create() = 0;

    LPCTSTR loadColumnWidth(LPCTSTR p);
    void loadAllColor(LPCTSTR p);
    void calcPercWidth(LPCTSTR p);

    virtual int getCount() = 0;
    virtual int getCurrSel() = 0;
    virtual void setCurrSel(int sel) = 0;
    virtual void removeAllItem() = 0;

    DWORD getShow() const { return currShow; }
    void setShow(DWORD v) { currShow = v; }

    virtual void formatAlarm(const fullInfoAlarm& fia, const SYSTEMTIME &st, const currAlarm::alarmStat& stat) = 0;
    virtual void formatHeaderAlarm(uint idHeadTitle) = 0;
    virtual HWND getHwnd() = 0;
    virtual bool setFocus(pAround::around where);
    virtual void calcWidth() {}

  protected:
    P_BaseBody* Owner;
    DWORD currShow;
    pAround& Around;

    int percWidth[MAX_COLUMN_ALARM];
    int columnPos[MAX_COLUMN_ALARM];

    virtual SIZE getSizeFont() = 0;
    LPCTSTR getFilterText() { return FilterText; }

    void formatAlarm(PVect<LPCTSTR>& items, const fullInfoAlarm& fia, const SYSTEMTIME &st, const currAlarm::alarmStat& stat);
    void formatHeaderAlarm(PVect<LPCTSTR>& txt, int width[], uint idHeadTitle);

    struct infoColorLB
    {
      COLORREF fgN;
      COLORREF bgN;
      COLORREF fgS;
      COLORREF bgS;
      infoColorLB() : fgN(0), bgN(RGB(0xff, 0xff, 0xff)), fgS(0), bgS(RGB(0xff, 0xff, 0xff)) {}
    };

    infoColorLB cBase;
    infoColorLB cEvent;
    infoColorLB cActive;
    infoColorLB cAcknoledged;
    infoColorLB cAcknoledgedEvent;
    LPCTSTR loadColor(infoColorLB& ic, LPCTSTR p);

    bool isFirstRow();
    bool isLastRow();
    bool evSetAroundFocus(UINT key);


    static void reverseTrueOrder(int target[], int source[]);
    static void verifyTrueOrder(int set[]);
    static void makeTrueOrder(int target[], int source[], int set[]);
    static void makeSet(DWORD var, int set[]);
  private:
    TCHAR FilterText[256];
    friend class pVarListAlarm;

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

