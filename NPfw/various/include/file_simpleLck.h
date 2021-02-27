//-------------- file_simpleLck.h --------------------------------------
//----------------------------------------------------------------------
#ifndef file_simpleLck_H_
#define file_simpleLck_H_
//----------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------
#include "defin.h"
//----------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------
typedef void tFILE;
//---------------------------------------------------------
class P_File_SimpleLock // gestione file bloccato già in apertura
{
  public:
    P_File_SimpleLock(LPCTSTR n);
    virtual ~P_File_SimpleLock();

  // bloccaggio e sbloccaggio del file
    virtual bool Lock(); 
    virtual bool UnLock();

  // le funzioni sotto verificano se F è un puntatore valido
    lUDimF P_write(const tFILE *b, lUDimF len);
    lUDimF P_read(tFILE *b, lUDimF len);
    bool P_seek(DimF ofs, int fw = SEEK_SET_);
    UDimF P_get_len();

    bool flush();

    bool isLocked() { return Locked; }

  protected:
    class P_File *F; // classe di accesso fisico ai file
    bool Locked;

    NO_COPY_COSTR_OPER(P_File_SimpleLock)
};
//----------------------------------------------------------------------
#include "restorePack.h"
#endif
