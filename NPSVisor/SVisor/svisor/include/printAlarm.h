//-------- printAlarm.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PRINTALARM_H_
#define PRINTALARM_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "printGeneric.h"
//----------------------------------------------------------------------------
#include "1.h"
//----------------------------------------------------------------------------
#define MAX_TITLE_PRINT 60
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
struct rowAlarmFileHeader
{
  DWORD type;  // 1 -> allarmi, 2 -> report
  TCHAR title[MAX_TITLE_PRINT];
};
//----------------------------------------------------------------------------
#define D_DESCR_ALARM 200
//----------------------------------------------------------------------------
struct rowAlarmFile
{
#if 1
  int alarm;
  int prph;
  TCHAR descr[D_DESCR_ALARM];
#else
  int id;
#endif
  int stat;
  FILETIME ft;
};
//----------------------------------------------------------------------------
class printAlarm : public printGeneric
{
  private:
    typedef printGeneric baseClass;
  public:
    printAlarm(PWin* par, svPrinter* printer, LPCTSTR fileRows);

  protected:
    P_File pfRows;
    virtual bool beginDoc(bool showSetup);
    virtual bool makeHeader();
    virtual bool makeFooter();
    virtual int addRow();
    PVect<int> posX;
    PVect<int> align;
    int heightFont;
    int heightRow;
  private:
    virtual LPCTSTR getTitle();
    rowAlarmFileHeader Header;
    virtual void run(bool preview, bool setup, LPCTSTR alternateBitmapOrID4Tool);
    int idInitAlarm;

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
