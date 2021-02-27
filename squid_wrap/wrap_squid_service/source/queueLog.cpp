//----------- queueLog.cpp ----------------------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Richedit.h>
//-----------------------------------------------------------------------------
#include "queueLog.h"
#include "p_manage_ini.h"
#include "p_winChildConn.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
logQueue::logQueue(PWin* owner) : Owner(owner), FreeList(sizeof(dataSend)), canLog(false)
{
  TCHAR tmp[_MAX_PATH];
  GetModuleDirName(SIZE_A(tmp), tmp);
  appendPath(tmp, INI_FILE);
  manageIni ini(tmp);
  if(ini.parse()) {
    LPCTSTR p = ini.getValue(KEY_INI_LOG, INI_FILE_BLK);
    if(p)
      canLog = toBool(_ttoi(p));
    }
}
//-----------------------------------------------------------------------------
logQueue::~logQueue()
{
}
//-----------------------------------------------------------------------------
void logQueue::Add(pDataSend d)
{
  if(canLog && Owner)
    PostMessage(*Owner, WM_FROM_QUEUE, 2, (LPARAM)d);
  else
    setFree(d);
}
//-----------------------------------------------------------------------------
pDataSend logQueue::getFree()
{
  pDataSend result = 0;
  do {
    criticalLock CL(CS);
    result = (pDataSend)FreeList.getFree();
    } while(false);
  if(result)
    result->clear();
  return result;
}
//-----------------------------------------------------------------------------
void logQueue::setFree(pDataSend plog)
{
  criticalLock CL(CS);
  FreeList.addToFree(plog);
}
//-----------------------------------------------------------------------------
