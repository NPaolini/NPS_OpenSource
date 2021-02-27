//--------------- s7_libnodave_common.h -------------------
#ifndef s7_libnodave_common_H_
#define s7_libnodave_common_H_
//---------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------
#include "comgperif.h"
#include "connClass.h"
//---------------------------------------------------------
#include "setPack.h"
//---------------------------------------------------------
enum lanCommand_libnodave {
  DB_ERROR_LIBNODAVE,
  DB_READ_LIBNODAVE,
  DB_READED_LIBNODAVE,
  DB_WRITE_LIBNODAVE,
  DB_WRITED_LIBNODAVE,

  DB_LOGIN_LIBNODAVE,
  DB_LOGGED_LIBNODAVE,
  DB_LOGOUT_LIBNODAVE
  };
//-----------------------------------------------------------
struct dataSend
{
  int id;
  int cmd;
  int db;
  int start;
  int len; // usato anche come codice di errore
  BYTE buffer[MAX_DIM_PERIF];
};
//----------------------------------------------------------
#define MAX_ID 999
//----------------------------------------------------------
#define SZ_STRU_MIN (sizeof(int) * 5)
#define SZ_STRU_MAX(l) (SZ_STRU_MIN + l)
//-----------------------------------------------------------
void makeServerMutexName(LPTSTR mx, LPCTSTR ns);
void makeClientMutexName(LPTSTR mx, LPCTSTR ns);
//-----------------------------------------------------------
#include "restorePack.h"
//---------------------------------------------------------
#endif
