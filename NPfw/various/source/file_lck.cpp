//////////////// P_FILE_L.CPP /////////////////////////////////////
#include "precHeader.h"
#include <stdlib.h>

#include "p_util.h"
#include "p_file.h"
#include "file_lck.h"

#include "pAppl.h"

#define DELAY_LOCK 3000

#define SIZE_I(a) (int(sizeof(a)))
//---------------------------------------------------------
CRITICAL_SECTION _wait_lock::cs;
DWORD _wait_lock::refCount = 0;
//---------------------------------------------------------
//---------------------------------------------------------
bool message_yes_no(LPCTSTR title, LPCTSTR msg);
bool message_lock(LPCTSTR title, LPCTSTR file);
//---------------------------------------------------------
bool _wait_lock::wait()
{
  EnterCriticalSection(&cs);
  bool ret = true; // inizializza a successo
  DWORD tick = GetTickCount();
  for(;;) {
    if(action()) // successo
      break;
    if((GetTickCount() - tick) >= DELAY_LOCK) {
#if 1
      if(request())  // richiesta di continuare o smettere
        tick = GetTickCount();
      else
#endif
        {
        ret = ret_err(); // ritorna errore timeout (deve essere false)
        break;
        }
      }
#if 1
    PAppl* appl = getAppl();
    if(appl)
      appl->pumpMessages();
#else
    MSG Msg;
    while(PeekMessage (&Msg, 0, 0, 0, PM_REMOVE) != 0) {
      if(!IsDialogMessage(Msg.hwnd, &Msg)) {
        TranslateMessage (&Msg);
        DispatchMessage (&Msg);
        }
      }
#endif
    }
  LeaveCriticalSection(&cs);
  return ret;
}
//---------------------------------------------------------
//---------------------------------------------------------
criticalSect _wait_lock_named::cs;
PVect<_wait_lock_named::dual*> _wait_lock_named::Dual;
//---------------------------------------------------------
//---------------------------------------------------------
void _wait_lock_named::init(LPCTSTR name)
{
  criticalLock cl(cs);

  uint nElem = Dual.getElem();
  currIx = -1;
  for(uint i = 0; i < nElem; ++i) {
    if(!_tcsicmp(Dual[i]->name, name)) {
      currIx = i;
      ++Dual[i]->refCount;
      break;
      }
    }
  if(currIx < 0) {
    Dual[nElem] = new dual(name);
    currIx = nElem;
    }
}
//---------------------------------------------------------
void _wait_lock_named::end()
{
  criticalLock cl(cs);
  if(currIx >= 0)
    if(--Dual[currIx]->refCount <= 0)
      delete Dual.remove(currIx);
}
//---------------------------------------------------------
bool _wait_lock_named::wait()
{
  if(currIx < 0)
    return false;
  criticalLock cl(Dual[currIx]->cs);
  bool ret = true; // inizializza a successo
  DWORD tick = GetTickCount();
  for(;;) {
    if(action()) // successo
      break;
    if((GetTickCount() - tick) >= DELAY_LOCK) {
      if(request())  // richiesta di continuare o smettere
        tick = GetTickCount();
      else
        {
        ret = ret_err(); // ritorna errore timeout (deve essere false)
        break;
        }
      }
    PAppl* appl = getAppl();
    if(appl)
      appl->pumpMessages();
    else {
      MSG Msg;
      while(PeekMessage (&Msg, 0, 0, 0, PM_REMOVE) != 0) {
        if(!IsDialogMessage(Msg.hwnd, &Msg)) {
          TranslateMessage (&Msg);
          DispatchMessage (&Msg);
          }
        }
      }
    }
  return ret;
}
//---------------------------------------------------------
class _wait_lock_read_write : public _wait_lock_named
{
  public:
    _wait_lock_read_write(P_File *f, LPCTSTR tit, int err, lUDimF &res, tFILE *buff,lUDimF len) :
      _wait_lock_named(f->get_name()), F(f), title(str_newdup(tit)), Err(err), Res(res), Buff(buff), Len(len) { }
    ~_wait_lock_read_write()  { delete []title; }
  protected:
    P_File *F;
    int Err;
    LPTSTR title;
    lUDimF &Res;
    tFILE *Buff;
    lUDimF Len;
    virtual bool request() { return toBool(message_lock(title, F->get_name())); }
    virtual bool ret_err() { return toBool(F->Error(Err)); }
};
//---------------------------------------------------------
//---------------------------------------------------------
class _wait_lock_read : public _wait_lock_read_write
{
  public:
    _wait_lock_read(P_File *f, LPCTSTR tit, int err, lUDimF &res, tFILE *buff, lUDimF len) :
       _wait_lock_read_write(f, tit, err, res, buff, len) { }
  protected:
    bool action()
    {
      Res = F->P_read(Buff, Len);
      // se il file è stato chiuso, potrebbe essere terminata l'applicazione,
      // meglio tornare successo
      return Res > 0 || !F->get_len() || INVALID_HANDLE_VALUE == F->get_handle();
    }
};
//---------------------------------------------------------
//---------------------------------------------------------
class _wait_lock_write : public _wait_lock_read_write
{
  public:
    _wait_lock_write(P_File *f, LPCTSTR tit,int err, lUDimF &res, const tFILE *buff, lUDimF len) :
       _wait_lock_read_write(f, tit, err, res, const_cast<tFILE*>(buff), len) { }
  protected:
    bool action()
    {
      Res = F->P_write(Buff, Len);
      // se il file file è stato chiuso, potrebbe essere terminata l'applicazione,
      // meglio tornare successo
      return Res > 0 || INVALID_HANDLE_VALUE == F->get_handle();
    }
};
//---------------------------------------------------------
//---------------------------------------------------------
class _wait_lock_fix_lock : public _wait_lock_named
{
  public:
    _wait_lock_fix_lock(P_File *f) :   _wait_lock_named(f->get_name()), F(f) { }
  protected:
    P_File *F;
    virtual bool request() {
        return message_yes_no(_T("Impossible to lock the file, Retry?"),
//        return message_yes_no(_T("Impossibile bloccare il file, Riprovo?"),
              F->get_name()); }
    virtual bool ret_err() { return false; }
    bool action()
    {
      return INVALID_HANDLE_VALUE !=  F->get_handle() &&
              LockFile(F->get_handle(),0L,0L,0x7fffffff,0L);
    }
};
//---------------------------------------------------------
//---------------------------------------------------------
bool message_yes_no(LPCTSTR title, LPCTSTR msg)
{
  HWND hw = ::GetForegroundWindow();
#if 1
  for(;;) {
    HWND par = GetParent(hw);
    if(par)
      hw = par;
    else
      break;
    }
#endif
  return ::MessageBox(hw, msg, title, MB_YESNO | MB_ICONWARNING) == IDYES;
}
//---------------------------------------------------------
bool message_lock(LPCTSTR title, LPCTSTR file)
{
  TCHAR buff[500];
  wsprintf(buff, _T("Impossible agree to the file %s\nMust I wait the unblocking?"), file);
//  wsprintf(buff,_T("Non posso accedere al file %s\n Attendo che si sblocchi?)", file);
  return message_yes_no(title, buff);
}

//---------------------------------------------------------
//---------------------------------------------------------
P_File_Lock::P_File_Lock(LPCTSTR n) : Locked(false)
{
  TCHAR full_name[_MAX_PATH];
  _tfullpath(full_name, n, _MAX_PATH);
  F = new P_File(full_name);  // crea
  if(F) {
    if(!F->P_open()) {
      delete F;
      F = NULL;
      }
    else
      Locked = Lock();
   }
}
//---------------------------------------------------------
P_File_Lock::~P_File_Lock()
{
  if(F) {
    UnLock();
    F->P_close();
    delete F;
    }
}
//---------------------------------------------------------
bool P_File_Lock::P_seek(DimF ofs, int fw)
{
  return F ? F->P_seek(ofs,fw) : 0;
}
//---------------------------------------------------------
UDimF P_File_Lock::P_get_len()
{
  return F ? F->get_len() : 0;
}
//---------------------------------------------------------
lUDimF P_File_Lock::P_write(const tFILE *b, lUDimF len)
{
  if(!F)
    return 0;
  TCHAR *tit = _T("Error while writing");

  lUDimF TNum=0;
  _wait_lock_write lk(F, tit, NOT_WRITE, TNum, b, len);
  if(lk.wait())
    return TNum;
  return 0;
}
//---------------------------------------------------------
lUDimF P_File_Lock::P_read(tFILE *b, lUDimF len)
{
  if(!F)
    return 0;
  TCHAR *tit = _T("Error while reading");
  lUDimF TNum=0;
  _wait_lock_read lk(F, tit, NOT_READ, TNum, b, len);
  if(lk.wait())
    return TNum;
  return 0;
}
//---------------------------------------------------------
bool P_File_Lock::Lock()
{
  if(!F)
    return 0;
  _wait_lock_fix_lock lk(F);
  return lk.wait();
}
//---------------------------------------------------------
bool P_File_Lock::UnLock()
{
  return toBool(UnlockFile(F->get_handle(),0L,0L,0x7fffffff,0L));
}
//---------------------------------------------------------
bool P_File_Lock::flush()
{
  return F ? F->flush() : false;
}
//---------------------------------------------------------


