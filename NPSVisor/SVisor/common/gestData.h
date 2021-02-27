//---------- gestdata.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef GEST_DATA__
#define GEST_DATA__
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "manageMemBase.h"
//-------------------------------------------------------------
// in PRF_MEMORY
#define ADDR_CODE_OPER 3
//---------------------------------------------------------------------
#define PREFIX_REPORT   _T("REP")
#define EXT_REPORT      _T(".rep")
#define dReport         dOrdini
#define WM_ADD_REPORT_ROW (WM_POST_ACTIVATE + 1)
//-------------------------------------------------------------
//----------------------------------------------------------
#define STEP_GRP_ALARM_BITS 10
//-------------------------------------------------------------
//---------------------------------------------------------------------
/*
enum causeReport {
  noReport,
  initWork,
  endWork,
  initJob,
  endJob,
  pauseJob,
  restartJob,
  chgShift,
  };
//---------------------------------------------------------------------
#define DIM_PRODUCT_NAME 12
#define DIM_JOB_NAME 12
//---------------------------------------------------------------------
struct reportFile
{
  FILETIME ft;
  BYTE product[DIM_PRODUCT_NAME];
  BYTE job[DIM_JOB_NAME];
  DWORD cause;
  fREALDATA goodWeight;
  fREALDATA badWeight;
};
*/
//-------------------------------------------------------------
class ImplJobData : public ImplJobMemBase
{
  private:
    typedef ImplJobMemBase baseClass;
  public:
    ImplJobData(mainClient *parent);
    ~ImplJobData();

    virtual void init();
    virtual void end();

    virtual void notify();

    virtual void logAlarm(class log_status &log, int which, bool force = false);
    virtual int getNBitAlarm() const;
    virtual int getAddrAlarm() const;

  protected:
    virtual void derivedNotify();

  private:
    class manageStop* Stops;
    class manageOperator* Opers;
    int prfReader;

    void updateFromPlc(gestPerif* plc);
    void SendStop(gestPerif* plc);
    void SendStart(gestPerif* plc);

    void printScreen();
    void saveScreen();

    void checkFilterAlarm();

    void checkPrintDataVar();
    void performPrintDataVar(int id);

    void checkPageAndShutdown();
    void loadDate();
    void checkCauseAlternateInput(gestPerif* plc);

    DWORD onShowError;

    void resetBitAsPing();

};
//---------------------------------------------------------------------
#endif
