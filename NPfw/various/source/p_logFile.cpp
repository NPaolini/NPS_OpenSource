//----------------- p_logFile.cpp --------------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include "p_logFile.h"
#include "p_util.h"
//----------------------------------------------------------
#include <stdio.h>
//-----------------------------------------------------
void p_logfile::init(uint id, LPCTSTR filename, bool append)
{
  delete pf;
  TCHAR n[_MAX_PATH];
  wsprintf(n, filename, id);
  if(append)
    pf = new P_File(n);
  else
    pf = new P_File(n, P_CREAT);
  count = 0;
  if(!pf->P_open()) {
    delete pf;
    pf = 0;
    count = maxCount + 1;
    }
  else {
    if(append) {
      pf->P_seek(0, SEEK_END_);
      pf->P_writeString(_T("\r\n-------------------------------------------\r\n"));
      }
    pf->P_writeString(_T("Avvio log\r\n"));
    }
}
//-----------------------------------------------------
void p_logfile::log(LPCTSTR row, bool newline, bool initBlk)
{
  if(count > maxCount)
    return;
  if(initBlk) {
    if(++count > maxCount) {
      delete pf;
      pf = 0;
      return;
      }
    TCHAR t[64];
    wsprintf(t, _T("N. [%2d] --------------------------------------\r\n"),  count);
    pf->P_writeString(t);
    }
  pf->P_writeString(row);
  if(newline)
    pf->P_writeString(_T("\r\n"));
}
//-----------------------------------------------------
void p_logfile::log(uint v, bool newline)
{
  if(count > maxCount)
    return;
  TCHAR t[64];
  wsprintf(t, _T(" %d"), v);
  log(t, newline);
}
//-----------------------------------------------------
//----------------------------------------------------------
p_logFile::p_logFile(LPCTSTR filename, bool append) : pf(0)
{
  if(append)
    pf = new P_File(filename);
  else
    pf = new P_File(filename, P_CREAT);
  if(pf->P_open()) {
    if(append)
      pf->P_seek(0, SEEK_END_);
    }
  else
    safeDeleteP(pf);
}
//----------------------------------------------------------
p_logFile::~p_logFile()
{
  delete pf;
}
//----------------------------------------------------------
DWORD p_logFile::get_len()
{
  return pf ? DWORD(pf->get_len()) : 0;
}
//----------------------------------------------------------
void p_logFile::addLog(LPCTSTR format, ...)
{
  if(!pf)
    return;
  TCHAR buff[4096 * 2];
  va_list  args;
  va_start( args, format );
  _vstprintf_p(buff, SIZE_A(buff) - 1, format, args);
  pf->P_writeString(buff);
  va_end(args);
}
//----------------------------------------------------------
