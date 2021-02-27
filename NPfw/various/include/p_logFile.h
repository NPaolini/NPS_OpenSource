//----------------- p_logFile.h ----------------------------
#ifndef p_logFile_H_
#define p_logFile_H_
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include "pDef.h"
#include "p_file.h"
//----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------
#define MAX_LOG_COUNT_DEF 50
//-----------------------------------------------------
#define FOREVER_LOG_COUNT (uint(-1))
//-----------------------------------------------------
class p_logfile
{
  public:
    p_logfile(uint nLog = 0) : pf(0), count(0), maxCount(nLog ? nLog : MAX_LOG_COUNT_DEF) {}
    ~p_logfile() { delete pf; }
    void init(uint id, LPCTSTR filename, bool append = false); // es. _T("Prph_%d_data.log")
    void log(LPCTSTR row, bool newline = false, bool initBlk = false);
    void logInitBlock(LPCTSTR row) { log(row, true, true); }
    void logNL(LPCTSTR row) { log(row, true); }
    void log(uint v, bool newline = false);
  private:
    class P_File* pf;
    uint count;
    const uint maxCount;
};
//----------------------------------------------------------
class p_logFile
{
  public:
    p_logFile(LPCTSTR filename, bool append = false);
    ~p_logFile();
    void addLog(LPCTSTR format, ...);
    bool isValid() { return toBool(pf); }
    DWORD get_len();
  private:
    class P_File* pf;
};
//----------------------------------------------------------
#include "restorePack.h"

#endif
