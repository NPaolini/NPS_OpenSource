//------------ gestprf.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef GESTPRF_H_
#define GESTPRF_H_
//----------------------------------------------------------------------------
#include "prfData.h"
#include "hdrmsg.h"
#include "defin.h"
#include "p_exit.h"
#include "p_tqueue_simple.h"
#include "PCrt_lck.h"
//----------------------------------------------------------------------------
#include "setPack.h"

// classe principale di gestione ciclo
class mainClient;
//----------------------------------------------------------------------------
// visualizza una MessageBox
//void showMessage(LPCTSTR msg, LPCTSTR title);
//----------------------------------------------------------------------------
// necessaria perché nel setOnPressing non si può disabilitare il pulsante
// altrimenti non risponde al rilascio.
bool setDisableWindowByTranfer(bool disable);
//----------------------------------------------------------------------------
#define MAX_SENT 50
typedef P_TQueueSimple<DWORD, MAX_SENT> queueId;
//----------------------------------------------------------------------------
// usata quando servono solo i due metodi, non fa distinzione tra periferiche
// reali e dati in memoria
class genericPerif
{
  public:
    genericPerif(uint id_msg);
    virtual ~genericPerif() {  }

    virtual prfData::tResultData get(prfData& target) const = 0;
    // se non derivata torna errore
    virtual prfData::tResultData set(const prfData& /*data*/, bool noCommit = false)
      { return prfData::invalidPerif; }
    virtual prfData::tResultData getset(const prfData& /*data*/, bool noCommit = false)
      { return prfData::invalidPerif; }
    uint getId() const { return idMsg; }

    virtual prfData::tData getDefaultType() const  { return (prfData::tData)typeAddress; }

    bool isReadOnly() const { return readOnly; }
    virtual bool isReady() const { return true; }

    virtual void dontSaveNow() {}
    virtual void saveNow() {}

    virtual void getBlockData(LPDWORD target, uint ndw, uint startAddr) = 0;
    virtual void copyBlockData(LPDWORD source, uint ndw, uint startAddr) = 0;
    virtual void fillBlockData(DWORD value, uint ndw, uint startAddr) = 0;

    virtual void sendResetAlarm() { }
    virtual void commitAlarm() {  }
    virtual int getNBitAlarm() const = 0;
    virtual int getAddrAlarm() const = 0;

    enum howSend {
                   NOT_DEFINED,

                   NO_CHECK = 1 << 0,

                   // verifica se la periferica sia pronta
                   CHECK_READY = 1 << 1,

                   // invia un SendMessage ed attende la risposta
                   WAIT_RESULT = 1 << 2,

                   // somma i due controlli precedenti
                   CHECK_ALL = CHECK_READY | WAIT_RESULT,

                   // invia un PostMessage, poi rimane in attesa
                   // di un messaggio da parte della periferica
                   // senza bloccare l'elaborazione dei messaggi
                   WAIT_ASYNC = 1 << 3,

                   CHECK_ALL_ASYNC = CHECK_READY | WAIT_ASYNC,

                   // visualizza o meno una window di attesa
                   SHOW_WAIT = 1 << 4,  // valido solo per il WAIT_ASYNC

                   ALL_AND_SHOW = CHECK_ALL_ASYNC | SHOW_WAIT,

                   };

    howSend getAutoShow() { return onAutoComm;  }
    howSend setAutoShow(howSend newStatus) {
      howSend old = onAutoComm;
      onAutoComm = newStatus;
      return old;
      }
  protected:
    uint idMsg;
    // tipo di indirizzamento, deve essere caricato all'avvio dal file
    // dei testi standard nelle classi derivate.
    uint typeAddress;
    // letto da std_msg.txt, se sola lettura non esegue la set()
    bool readOnly;
    // se le classi derivate richiamano la send/receive nei metodi set/get
    // questa variabile indica se deve visualizzare la finestra di attesa o meno
    mutable howSend onAutoComm;
};
//----------------------------------------------------------------------------
class gestPerif : public genericPerif
{
  public:
    gestPerif(uint id_msg, mainClient* par);

      // dovrebbe già essere stato inviato il messaggio di chiusura
      // ma per sicurezza....
    virtual ~gestPerif();// {  closeApp(); }

//    uint getId() const { return idMsg; }

    virtual bool acceptMsgId(uint msg) const { return getId() == msg; }

    // deve tornare true se accetta il messaggio, false se non la riguarda
    // per messaggi inviati dai programmi esterni di comunicazione diretta
    // o per messaggi tra le periferiche
    virtual bool acceptMsg(uint msg, WPARAM wParam, LPARAM& lParam) = 0;

    // gestione allarmi della periferica, la posizione indica la priorità
    enum statAlarm { sNoAlarm, sEvent, sAlarm };
    virtual statAlarm hasAlarm()  const { return sNoAlarm; }

    // verificare se la classe log_status può servire per tutti gli
    // usi e quindi se va inserita nella lib
    virtual void logAlarm(class log_status& /*log*/, int /*wich*/, bool force = false) {  }

    enum statMachine {
        sNoCheck = 0,       // non gestisce il controllo di run
        sOnRun = 1,         // si è avviata
        sDejaRun = 1 << 1,  // era già in moto
        sOnStop = 1 << 2,   // si è fermata
        sDejaStop = 1 << 3, // era già ferma
        sRunning = sOnRun | sDejaRun,  // comunque in moto
        sStopped = sOnStop | sDejaStop // comunque ferma
        };
     // solo una periferica dovrebbe gestire lo stato globale del sistema
    virtual statMachine isRunning() const { return sNoCheck; }

    // esegue un aggiornamento dei dati
    virtual void refresh() { }

    // valido solo dopo una chiamata a refresh()
    bool isDirty() const { return Dirty; }

    bool isReady() const { return Ready; }

    // per resettare lo stato di dirty.
    virtual void commit() = 0;

    // viene avvisata della chiusura dell'applicazione
    virtual void closeApp();

    // ritorna il dato immediatemente letto dalla periferica
    DWDATA getData() const { return Data; }

    enum sorted_multi_set {
          sms_noSort,           // nessuna riorganizzazione (per compatibilità)
          sms_needSort,         // riorganizzazione completa
          sms_dejaSortButCheck, // i dati sono ordinati ma possono esserci buchi
          sms_sortedOneBlock,   // i dati sono ordinati e senza buchi, va spedito un solo blocco
          };

    // se non derivata torna errore. Per inviare una serie di dati in una volta
    virtual prfData::tResultData multiSet(const prfData* /*data*/, int /*num*/, bool /*show*/ = true,
                          sorted_multi_set /*sms*/ = sms_noSort)
      { return prfData::invalidPerif; }

    // se gli errori usano lo stesso buffer dei dati, punta
    // all'offset degli errori, altrimenti all'inizio del buffer
    virtual prfData::tResultData getErr(prfData& /*target*/) const { return prfData::invalidPerif; }

    // per spedizione messaggi personalizzati. howSend gestisce le modalità.
    // Se si aspetta un valore di ritorno, esso viene ritornato in p2.
    // Il metodo torna false se la periferica non è pronta o non ha l'HANDLE della
    // window del driver
    virtual bool SendCustom(WORD msg, howSend how, WORD hiP1, DWORD &p2, WORD desiredResponce = NO) const;

    virtual bool Send(const void *buff, int len, bool showWait = true);

    // gli indirizzi vengono fusi in lParam, il flag viene unito
    // al messaggio MSG_READ_CUSTOM, per informazioni supplementari.
    // Per default segnala cosa si vuol ricevere in caso di successo
    // (comunque dipende dalla periferica se usare o no questo flag,
    // l'importante è che ritorni OK_READ in caso di successo)
    virtual bool Receive(int addr_init, int addr_end, WORD flag = OK_READ) const;

    // usate per spedire/ricevere dati senza far apparire la finestra di
    // attesa e senza verificarne l'esito.
    // In caso di periferica di uso generale che non prende iniziative di
    // lettura/scrittura, ma che dipende esclusivamente dal supervisore.
    // Tornano false in caso di errore
    virtual bool SendHide(const void *buff, int len);
    virtual bool ReceiveHide(int addr_init, int addr_end) const;

    // attivo durante un invio, può servire per evitare la richiesta
    // automatica di lettura durante una trasmissione
    bool isSending() const { return inExec; }

    WORD LtoP_Init(int addr) const;
    WORD LtoP_End(int addr) const;
    uint getSizeType() const { return LtoP_Init(1); }

    void forceRefresh() { needRefresh = true; refresh(); }
    HWND getHwndDriver() { return hWnd; }
    virtual bool hasRequestVar() { return requestVarPresent; }

  protected:
    bool requestVarPresent;

    HWND hWnd;
    // viene attivato/disattivato dal driver, per comunicare
    // se puo' ricevere comandi
    bool Ready;

    mainClient* Par;

    // dovrebbe cambiare in base ad un messaggio tornato dalla periferica fisica
    mutable WORD Responce;
    // se oltre al messaggio tornano dei dati
    mutable DWORD Data;

    void setDirty(bool set = true) { Dirty = set; }

    // flag usato per forzare la lettura (da file)
    mutable bool needRefresh;

    DWORD maxDelay;

    void makefilenamecommand(LPTSTR path, uint id);
  private:
    mutable bool Dirty;
    mutable bool inExec;

    // routine di attesa usata in WAIT_ASYNC,
    // può aprire una finestra di attesa della risposta [r]
//    enum eResponce { NO, OK_READ, OK_WRITE, FAILED };
    bool waitFor(eResponce r, bool showWait) const;

    // prepara i dati [su file] prima di spedire il messaggio,
    // richiamata da Send()
    // ** poiché non tutte le periferiche hanno necessità di ricevere dati
    //    dal supervisore, non viene usata pura
    virtual bool makeDataToSend(const void *buff, int len, uint id) { return false; }

    // richiamata se errore (true -> in trasmissione, false -> in ricezione)
    virtual void unmakeDataToSend(bool /*failedWrite*/, uint id) { }

    bool performSendCustom(WORD msg, howSend how, WORD hiP1, DWORD& p2, WORD desiredResponce) const;

    queueId Id2Send;
    queueId IdSent;
  criticalSect CS;
    WORD idLast;
    friend unsigned FAR PASCAL DriverProc(void*);
    HANDLE hThread;
    HANDLE HEvSend;
    HANDLE HEvClose;
    WORD getNewId();


    void makefilenamecommandQueue(LPTSTR path);
    bool thPerformSend(DWORD id) const;
    void thEvent();
    void thTimeout();
    void thStart();
    void thEnd();

};
//-----------------------------------------------------
// set di periferiche,
class perifsSet
{
  public:
    perifsSet();
    ~perifsSet();
    void add(gestPerif *p, bool autodelete = true);

    // forward a tutte le periferiche
    void refresh();
    bool isDirty() const;
    bool isReady() const;
    // invia sia il commit() che il commitAlarm()
    void commit();
    void closeApp();
    gestPerif::statAlarm hasAlarm() const;
    void logAlarm(class log_status &log, int which, bool force = false);
    void resetAlarm();

    bool isDirty(uint id) const;
    // torna la periferica avente l'id passato o zero se non esiste
    gestPerif* get(uint id) const;
    gestPerif* getByMsg(uint msg) const;
    // torna la periferica corrente, da usare dopo una setFirst()
    // ed in seguito ad una setNext()
    gestPerif* getCurr() const;
    bool setFirst();
    bool setNext();

    void sendHideShow(bool hide);

    void removeAll();
  private:
    struct listPerif {
      gestPerif *perif;
      listPerif* next;
      bool autodelete;
      } *Head, *Curr;
    // funzione ricorsiva che elimina le periferiche a partire dall'ultima

    void remove(listPerif* curr);

    perifsSet operator *(const perifsSet &set);
    perifsSet operator *(const perifsSet set);
};
//-----------------------------------------------------
// classe di implementazione dei dati.
class ImplJobData;

// classe di interfaccia per gestione manutenzioni
class IMaint;

// class gestione dati vari
class gestJobData : public genericPerif
{
  public:
    gestJobData(mainClient *parent);
    ~gestJobData();
    // occorre separare l'inizializzazione dalla costruzione dell'oggetto
    // non tutti gli oggetti esterni necessari potrebbero essere già stati
    // inizializzati o costruiti
    void init();
    // per lo stesso motivo dell'init(), potrebbero essere stati distrutti
    // oggetti necessari al logout
    void end();

    void save();
    void load();

    void notify();

    // notifica per effettuare un salvataggio di tutti i dati di log
    // esclusi quelli standard
    void notifySaveLog();

    // salva lo stato interno
    void commit();
    ImplJobData* get() const { return Data; }

    // setta lo stato interno come modificato
    void setDirty();

    // qualche pagina ha modificato il timer per il salvataggio dei trend
    // occorre reinizializzarlo
    void checkTimerTrend();

    virtual bool isReady() const;
 //    IMaint *getMaint();

    prfData::tResultData get(prfData& target) const;
    prfData::tResultData set(const prfData& data, bool noCommit = false);
    prfData::tResultData getset(prfData& data, bool noCommit = false);
    virtual void dontSaveNow();
    virtual void saveNow();
    virtual void getBlockData(LPDWORD target, uint ndw, uint startAddr);
    virtual void copyBlockData(LPDWORD source, uint ndw, uint startAddr);
    virtual void fillBlockData(DWORD value, uint ndw, uint startAddr);
    virtual void logAlarm(class log_status &log, int wich, bool force = false);
    virtual int getNBitAlarm() const;
    virtual int getAddrAlarm() const;

  private:
    ImplJobData* Data;
};
//-----------------------------------------------------
#include "restorePack.h"

#endif
