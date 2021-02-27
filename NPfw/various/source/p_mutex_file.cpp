//-------------- p_mutex_file.cpp ---------------------------------------
//-----------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------
#include "p_mutex_file.h"
#include "p_util.h"
//-----------------------------------------------------------------------
#define F_READ  GENERIC_READ
#define F_WRITE (GENERIC_WRITE | F_READ)

#define S_4_WRITE  FILE_SHARE_READ
#define S_4_READ  (FILE_SHARE_WRITE | S_4_WRITE)

#define SHARE_(a)  ((a) & GENERIC_WRITE ? S_4_WRITE : (a) & GENERIC_READ ? S_4_READ : 0)

#define OPEN_(a,b,how) CreateFile((a), (b), SHARE_(b), 0, (how), FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, 0)

#define OPEN_FILE_MUTEX(a,how) OPEN_((a),F_WRITE,(how))

#define OPEN_CREAT_(a) OPEN_FILE_MUTEX((a), OPEN_ALWAYS)
#define NEW_CREAT_(a) OPEN_FILE_MUTEX((a), CREATE_NEW)
//----------------------------------------------------------------------------
#define VALID_HANDLE(h) (INVALID_HANDLE_VALUE != (h) && (h))
//----------------------------------------------------------------------------
static HANDLE lock_file(LPCTSTR name, UINT how)
{
  for(uint i = 0; i < 300; ++i) {
    HANDLE hFile = OPEN_FILE_MUTEX(name, how);
    if(INVALID_HANDLE_VALUE != hFile) {
      if(LockFile(hFile, 0L, 0L, 0x7fffffff, 0L))
        return hFile;
      CloseHandle(hFile);
      }
    Sleep(300);
    }
  return INVALID_HANDLE_VALUE;
}
//----------------------------------------------------------------------------
static HANDLE unlock_file(HANDLE hFile)
{
  if(VALID_HANDLE(hFile)) {
    UnlockFile(hFile, 0L, 0L, 0x7fffffff, 0L);
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    }
  return hFile;
}
//----------------------------------------------------------------------------
mutex_file::mutex_file(LPCTSTR name) : Name(str_newdup(name)), hFile(INVALID_HANDLE_VALUE) {}
//----------------------------------------------------------------------------
mutex_file::~mutex_file()
{
  unlock();
  delete []Name;
}
//----------------------------------------------------------------------------
bool mutex_file::lock()
{
  if(VALID_HANDLE(hFile))
    return false;
  hFile = lock_file(Name, OPEN_ALWAYS);
  return VALID_HANDLE(hFile);
}
//----------------------------------------------------------------------------
bool mutex_file::unlock()
{
  hFile = unlock_file(hFile);
  return !VALID_HANDLE(hFile);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
mutex_file_del::mutex_file_del(LPCTSTR name) : Name(str_newdup(name)), hFile(INVALID_HANDLE_VALUE) {}
//----------------------------------------------------------------------------
mutex_file_del::~mutex_file_del()
{
  unlock();
  delete []Name;
}
//----------------------------------------------------------------------------
bool mutex_file_del::lock()
{
  if(VALID_HANDLE(hFile))
    return false;
  hFile = lock_file(Name, CREATE_NEW);
  return hFile && INVALID_HANDLE_VALUE != hFile;
}
//----------------------------------------------------------------------------
bool mutex_file_del::unlock()
{
  hFile = unlock_file(hFile);
  bool success = !VALID_HANDLE(hFile);
  if(success)
    DeleteFile(Name);
  return success;
}
