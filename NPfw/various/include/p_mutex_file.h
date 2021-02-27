//-------------- p_mutex_file.h -----------------------------------------------
//-----------------------------------------------------------------------
#ifndef p_mutex_file_H_
#define p_mutex_file_H_
//-----------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------
#include "defin.h"

#include "setPack.h"
//-----------------------------------------------------------------------
// il file può esistere e non viene modificato
class mutex_file
{
  public:
    mutex_file(LPCTSTR name);
    ~mutex_file();
    bool lock();
    bool unlock();
  private:
    HANDLE hFile;
    LPCTSTR Name;
    NO_COPY_COSTR_OPER(mutex_file)
};
//----------------------------------------------------------------------------
// il file non deve esistere, viene cancellato al rilascio
class mutex_file_del
{
  public:
    mutex_file_del(LPCTSTR name);
    ~mutex_file_del();
    bool lock();
    bool unlock();
  private:
    HANDLE hFile;
    LPCTSTR Name;
    NO_COPY_COSTR_OPER(mutex_file_del)
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
