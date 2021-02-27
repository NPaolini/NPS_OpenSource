//----------- queueLog.h ------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef QUEUELOG_H_
#define QUEUELOG_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "p_file.h"
#include "p_util.h"
#include "PCrt_lck.h"
#include "p_freeList.h"
#include "file_simplelck.h"
//----------------------------------------------------------------------------
#define INI_FILE _T("wrap_squid.ini")

#define INI_FILE_BLK _T("listen_port")
#define INI_FILE_KEY _T("port")
#define KEY_INI_LOG _T("log_access")
// il blocco da usare viene passato dall'helper
#define KEY_INI_PROGR _T("progr")
#define KEY_INI_PARAM _T("param")
#define KEY_INI_OBFUSCATED _T("obfuscated")
//-----------------------------------------------------------------------------
#define BUFF_SIZE (4096 * 4)
//-----------------------------------------------------------------------------
struct dataSend
{
  WORD type; // 1 -> receive, 2 -> send
  char buff[BUFF_SIZE];
  dataSend(WORD type) : type(type) { buff[0] = 0; }
  void clear() { type = 0; buff[0] = 0; }
};
//-----------------------------------------------------------------------------
typedef dataSend* pDataSend;
#define WM_FROM_QUEUE (WM_FROM_CLIENT + 100)
//-----------------------------------------------------------------------------
class logQueue
{
  public:
    logQueue(PWin* owner = 0);
    virtual ~logQueue();

    void set_Owner(PWin* owner) { Owner = owner; }
    virtual void Add(pDataSend d);

    // preleva il primo dato libero dalla lista libera o alloca memoria
    // se la lista è vuota
    pDataSend getFree();
    void  setFree(pDataSend plog);

  private:
    PWin* Owner;
    criticalSect CS;
    P_freeList FreeList;
    bool canLog;
};
//-----------------------------------------------------------------------------
extern logQueue& getLogQueue();
//-----------------------------------------------------------------------------
#endif
