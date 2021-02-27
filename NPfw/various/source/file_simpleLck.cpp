//-------- file_simpleLck.cpp ---------------------------------
#include "precHeader.h"
#include <stdlib.h>
//---------------------------------------------------------
#include "p_util.h"
#include "p_file.h"
#include "file_simplelck.h"
//---------------------------------------------------------
P_File_SimpleLock::P_File_SimpleLock(LPCTSTR n) : Locked(false)
{
  TCHAR full_name[_MAX_PATH];
  _tfullpath(full_name, n, _MAX_PATH);
  F = new P_File(full_name);  // crea
  if(F) {
    if(!F->P_open()) {
      delete F;
      F = NULL;
      }
   }
}
//---------------------------------------------------------
P_File_SimpleLock::~P_File_SimpleLock()
{
  if(F) {
    UnLock();
    F->P_close();
    delete F;
    }
}
//---------------------------------------------------------
bool P_File_SimpleLock::P_seek(DimF ofs, int fw)
{
  return F ? F->P_seek(ofs,fw) : 0;
}
//---------------------------------------------------------
UDimF P_File_SimpleLock::P_get_len()
{
  return F ? F->get_len() : 0;
}
//---------------------------------------------------------
lUDimF P_File_SimpleLock::P_write(const tFILE *b, lUDimF len)
{
  if(!F)
    return 0;
  return F->P_write(b, len);
}
//---------------------------------------------------------
lUDimF P_File_SimpleLock::P_read(tFILE *b, lUDimF len)
{
  if(!F)
    return 0;
  return F->P_read(b, len);
}
//---------------------------------------------------------
bool P_File_SimpleLock::Lock()
{
  if(!F || Locked)
    return false;
  return Locked = INVALID_HANDLE_VALUE !=  F->get_handle() &&
              LockFile(F->get_handle(), 0L, 0L, 0x7fffffff, 0L);
}
//---------------------------------------------------------
bool P_File_SimpleLock::UnLock()
{
  if(!F || !Locked)
    return false;
  Locked = false;
  return toBool(UnlockFile(F->get_handle(),0L,0L,0x7fffffff,0L));
}
//---------------------------------------------------------
bool P_File_SimpleLock::flush()
{
  return F ? F->flush() : false;
}
//---------------------------------------------------------


