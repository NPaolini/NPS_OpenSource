//-------------- file_lck.h --------------------------------------------
//----------------------------------------------------------------------
#ifndef FILE_LCK_H_
#define FILE_LCK_H_
//----------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------
#include "defin.h"
#include "PCrt_lck.h"
#include "setPack.h"

typedef void tFILE;
//---------------------------------------------------------
class _wait_lock
{
  public:
    _wait_lock()  {
      if(!refCount)
        InitializeCriticalSection(&cs);
      ++refCount;
      }
    virtual ~_wait_lock() {
      --refCount;
      if(!refCount)
        DeleteCriticalSection(&cs);
      }

    // imposta un ciclo di attesa con timeout finché action() non torna true
    // al timeout richiama request() e riparte se questa torna true,
    // abbandona e torna ret_err() se request() torna false
    bool wait();
  protected:
    // azione che potrebbe essere bloccata, da ritentare più volte
    virtual bool action()=0;
    // richiesta di aspettare ancora o abbandonare
    virtual bool request()=0;
    // ritorno (false) con scelta di un eventuale messaggio per l'utente
    virtual bool ret_err()=0;
  private:
    static CRITICAL_SECTION cs;
    static DWORD refCount;
    NO_COPY_COSTR_OPER(_wait_lock)
};
//---------------------------------------------------------
// ora la classe file sotto usa questa
class _wait_lock_named
{
  public:
    _wait_lock_named(LPCTSTR name) : currIx(-1) {  init(name);  }
    virtual ~_wait_lock_named() { end(); }

    // imposta un ciclo di attesa con timeout finché action() non torna true
    // al timeout richiama request() e riparte se questa torna true,
    // abbandona e torna ret_err() se request() torna false
    bool wait();
  protected:
    // azione che potrebbe essere bloccata, da ritentare più volte
    virtual bool action()=0;
    // richiesta di aspettare ancora o abbandonare
    virtual bool request()=0;
    // ritorno (false) con scelta di un eventuale messaggio per l'utente
    virtual bool ret_err()=0;
  private:
    void init(LPCTSTR name);
    void end();
    struct dual {
      criticalSect cs;
      TCHAR name[_MAX_PATH];
      DWORD refCount;
      dual(LPCTSTR lockname = 0) : refCount(1) {
        if(lockname)
          _tcscpy_s(name, lockname);
        else
          ZeroMemory(name, sizeof(name));
        }
      ~dual() { }
      };
    static PVect<dual*> Dual;
    static criticalSect cs;
    int currIx;

    NO_COPY_COSTR_OPER(_wait_lock_named)
};
//---------------------------------------------------------
bool message_yes_no(LPCTSTR title, LPCTSTR msg);
bool message_lock(LPCTSTR title, LPCTSTR file);
//----------------------------------------------------------------------
class P_File_Lock // gestione file bloccato già in apertura
{
  public:
    P_File_Lock(LPCTSTR n);
    virtual ~P_File_Lock();
  // le funzioni sotto verificano se F è un puntatore valido
  // le funzioni di scrittura e lettura implementano una wait per attendere
   // che le zone si liberino o per rinunciare
    lUDimF P_write(const tFILE *b, lUDimF len);
    lUDimF P_read(tFILE *b, lUDimF len);
    bool P_seek(DimF ofs, int fw = SEEK_SET_);
    UDimF P_get_len();

    bool flush();

    bool isLocked() { return Locked; }

  protected:
    class P_File *F; // classe di accesso fisico ai file
    bool Locked;
  // bloccaggio e sbloccaggio del file
    bool Lock(); // blocca tutto il file, viene richiamato nel ctor
    bool UnLock(); // viene richiamato nel dtor

    NO_COPY_COSTR_OPER(P_File_Lock)
};
//----------------------------------------------------------------------
#include "restorePack.h"
#endif
