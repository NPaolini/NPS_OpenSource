//-------- mntnce.h ----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MNTNCE_H_
#define MNTNCE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PDEF_H_
  #include "pDef.h"
#endif

#ifndef PCRT_LCK_H_
  #include "PCrt_lck.h"
#endif

#ifndef IMNTNCE_H_
  #include "imntnce.h"
#endif
#include "setPack.h"
//----------------------------------------------------------------------------
#define MAX_MAINT 80
#define FILENAME_MAINT _T("Mainten.dat")

#define SECOND_TO_I64 10000000i64
#define HOUR_TO_I64 (MINUTE_TO_I64 * 60)
#define MINUTE_TO_I64 (SECOND_TO_I64 * 60)
//---------------------------------------------------------------------
struct g_maint
{
  FILETIME ft; // time di lavoro effettuato
  uint hours;  // numero di ore di lavoro prima della manutenzione
  g_maint(const g_maint& m) : ft(m.ft), hours(m.hours) {  }
  // dovendo usare array di g_maint che verranno letti da file, l'azzeramento
  // di default dei dati è una perdita di tempo inutile
  g_maint()  {  }
  // ctor specializzato per inizializzazione
  g_maint(int) : hours(0) {
      ft.dwLowDateTime = 0;
      ft.dwHighDateTime = 0;
      }
};
//---------------------------------------------------------------------
//#define MK_I64(ft) ((reinterpret_cast<LARGE_INTEGER*>(&ft))->QuadPart)
//#define cMK_I64(ft) ((reinterpret_cast<const LARGE_INTEGER*>(&ft))->QuadPart)
struct g_mem_maint
{
  uint id;     // codice manutenzione
  g_maint maint;
  g_mem_maint() { }
  g_mem_maint(int) : id(0), maint(0) {  }
  g_mem_maint(int id, const g_maint &m) : maint(m), id(id) {  }
  __int64 remain() const;

  // occorre verificare prima se il valore sia o meno negativo e quindi
  // escluderlo dalla lista.
  // se il valore è negativo deve essere considerato più grande (per non
  // aggiungerlo alla lista di quelli da tenere sotto controllo per una
  // scadenza vicina), in pratica significa che è già scaduto, ma non
  // ancora resettato
  bool operator >(const g_mem_maint &a) { return remain() > a.remain();  }
  bool operator <(const g_mem_maint &a) { return remain() < a.remain();  }
  bool operator ==(const g_mem_maint &a)  { return(remain() == a.remain()); }

};

// il file contenente i dati di manutenzione è formato da un array fisso
// di MAX_MAINT strutture g_maint, in cui la posizione indica il codice.
// In memoria viene caricato solo quello a più breve scadenza, insieme
// a quelli coincidenti.
// Vengono mostrati a video solo quelli presenti nel file dei testi.
// Ad ogni modifica (nella apposita pagina), viene riscritto il file e
// reinizializzata la lista

// Per non effettuare un polling implementa un thread con attesa dell'evento
// di ricarica/chiusura con timeout equivalente al refresh dei dati su disco.
// Alla scadenza apre una finestra con la lista delle manutenzioni da fare
class gest_maint : public IMaint
{
  public:
    gest_maint();
    ~gest_maint();

    // ferma il thread (se attivo), ricarica i dati da file e
    // riparte col thread
    void start();

    // ferma il conteggio (per es. in caso di arresto della macchina)
    void stop();

    // resetta i soli dati scaduti (prima ferma il thread e poi lo riavvia)
    void reset();

    // aggiornano i dati, la prima aggiorna solo la possizione passata,
    // la seconda tutti i dati corrispondenti
    // fermano il thread e lo riavviano
    void setHours(uint pos, uint val);
    void setHours(g_maint* all);

    void setLeft(uint pos, __int64 val);
    void setLeft(g_maint* all);

    void setAll(uint pos, uint vHours, __int64 vLeft);
    void setAll(g_maint* all);

    bool isRunning();

    bool isChanged() { bool changed = Dirty; Dirty = false; return changed; }

    // carica in buff tutti i dati, buff deve essere grande abbastanza
    bool getAll(g_maint* buff);

    DWORD getMainThreadId() { return mainThreadId; }
  private:

    // riempie la lista con scadenze di manutenzione più a breve,
    // composta da codici con tempi uguali di rimanenza (a meno di una
    // tolleranza)
    bool fillList(class setOfMaint &l, bool& fullExpired, bool force = false);

    HANDLE hEvent;
    DWORD idThread;
    DWORD mainThreadId;
    bool goingOut;
    bool Dirty;
    criticalSect CS;

    void init();
    void resetThread();

    friend DWORD WINAPI ScadManutProc(LPVOID);
/*
    typedef unsigned ( __stdcall *pfThread )( void * );
    static unsigned __stdcall ScadManutProc(void*);
*/
//    static DWORD FAR PASCAL ScadManutProc(LPSTR);

    enum tset { _hours, _left, _all };
    void setGen(g_maint* all, tset type);
    void setGen(uint pos, uint vHours, __int64 vLeft, tset type);

};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
inline void gest_maint::setHours(uint pos, uint val)
  { setGen(pos, val, 0, _hours);  }
//-------------------------------------------------------------------------
inline void gest_maint::setLeft(uint pos, __int64 val)
  { setGen(pos, 0, val, _left); }
//-------------------------------------------------------------------------
inline void gest_maint::setAll(uint pos, uint vHours, __int64 vLeft)
  { setGen(pos, vHours, vLeft, _all); }
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
inline void gest_maint::setHours(g_maint* all)  { setGen(all, _hours);  }
//-------------------------------------------------------------------------
inline void gest_maint::setLeft(g_maint* all) { setGen(all, _left); }
//-------------------------------------------------------------------------
inline void gest_maint::setAll(g_maint* all)  { setGen(all, _all);  }
//-------------------------------------------------------------------------
#include "restorePack.h"

#endif
