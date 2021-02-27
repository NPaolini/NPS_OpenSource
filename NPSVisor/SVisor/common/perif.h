//--------- perif.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PERIF__
#define PERIF__
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "defgPerif.h"
#include "gestPrf.h"
#include "p_mappedfile.h"
#include "PCrt_lck.h"
//-------------------------------------------------------------
// classe di gestione periferica comune a tutte le periferiche.
// Alcune proprietà/metodi sono validi solo per la periferica 2 (deve essere sempre il PLC)
// nelle altre periferiche vengono ignorati o tornano un valore anonimo
//-------------------------------------------------------------
#define WM_PLC 2
//----------------------------------------------------------------------------
//#define SPLASH_UNICODE
//-------------------------------------------------------------
// nuove specifiche
// per compatibilità viene lasciato il #define WM_PLC, ma indica solo il numero
// della prima periferica. Per ottenere il vero gestore del plc occorre richiamare
// questo valore che va impostato (per default come WM_PLC)
extern uint WM_REAL_PLC;
//-------------------------------------------------------------
class perif : public gestPerif
{
  public:
    perif(mainClient* par, uint id);
    ~perif();

    virtual void refresh();
    virtual void commit();
    virtual bool acceptMsg(uint msg, WPARAM wParam, LPARAM& lParam);

    virtual prfData::tResultData get(prfData& target) const;
    virtual prfData::tResultData set(const prfData& data, bool noCommit = false);
    virtual prfData::tResultData getset(prfData& data, bool noCommit = false);
    virtual prfData::tResultData multiSet(const prfData* data, int num, bool show = true,
            sorted_multi_set sms = sms_noSort);

    // carica in target il dato contenuto in data, torna la dimensione del dato
    int getFromData(__int64& target, const prfData& data);

    virtual bool isOnEmerg() const { return false; }

    virtual bool acceptMsgId(uint msg) const { return idMsgPerif == msg; }

    virtual statAlarm hasAlarm() const;
    virtual void sendResetAlarm();
    virtual void logAlarm(class log_status &log, int wich, bool force = false);

    // per inviare al plc la segnalazione di un errore
    enum eSetting { sSet = 1, sReset };
    bool sendErrBit(eSetting set, int bit);

    virtual prfData::tResultData getErr(prfData& target) const;

    int getNBitAlarm() const;
    int getAddrAlarm() const { return addrAlarm; }

    virtual void getBlockData(LPDWORD target, uint ndw, uint startAddr);
    virtual void copyBlockData(LPDWORD source, uint ndw, uint startAddr);
    virtual void fillBlockData(DWORD value, uint ndw, uint startAddr);
  protected:

    BDATA oldBuff[MAX_BDATA_PERIF];
    // modifica: se viene usato un filemapped currBuff punta al file
    // altrimenti a memoria allocata
//    BDATA currBuff[MAX_BDATA_PERIF];
    BDATA* currBuff;
    // se la periferica invia il flag opportuno è aggiornata alla versione con 16kVar
    // altrimenti si usa la vecchia dimensione 4kVar
    DWORD trueDimBuff; // in BDATA

    p_MappedFile* pMF;
    virtual void openMapping(bool useFileMapped);

    // nella nuova gestione l'id della classe base è l'id di periferica e questo è invece
    // l'id tornato da RegisterWindowMessage().
    uint idMsgPerif;

    // poiché l'indirizzamento è generico occorre usare questa per
    // recuparare il dato. Si torna DWDATA poiché dovrebbe essere il
    // typo più capiente
    DWDATA getData(const BDATA* buff, int addr, uint type) const;

    int countRefresh;

    bool makeDataToSend(const void *buff, int len, uint id);
    void unmakeDataToSend(bool failedWrite, uint id);

    void log_Alarm(log_status &log, int which, bool force, int offs, int maxWord, int base, int step);

    int fillToSend(const prfData& data, struct datasend& result);

    int forceDirtyOnReceive;

    DWDATA addrAlarm;
    DWDATA numWordAlarm;

    DWDATA offsEvent;
    DWDATA numWordEvent;

    bool useFlagAlarm;
    DWDATA maskAlarm;
    DWDATA maskEvent;
    DWDATA wordStat;

    DWDATA baseTypeAlarm;

    DWDATA wordResetAlarm;
    // se valore negativo non viene usato
    int bitReset;

    // attivato da un messaggio della periferica, indica se gestisce la scrittura
    // del singolo bit. Usato con i nuovi driver.
    // N.B. vale solo per invio singolo, non per multiSet()
    int useSendBit;
    prfData::tResultData setBits(const prfData& data);
    prfData::tResultData setBits16(const prfData& data);
    bool canSendBits(prfData* data);

    void setErrPrph();
    mutable criticalSect cSectSet;
  private:
    typedef gestPerif baseClass;
};
//-----------------------------------------------------------------
// il PLC è una periferica speciale e conviene trattarlo a parte, deve
// rispondere a più esigenze
//-------------------------------------------------------------
class perifPLC : public perif
{
  public:
    perifPLC(mainClient* par);
    ~perifPLC();

    virtual statMachine isRunning() const;

    virtual void commit();
//    virtual statAlarm hasAlarm() const;
//    virtual void sendResetAlarm();
//    virtual void logAlarm(class log_status &log, int wich, bool force = false);

    // per inviare al plc la segnalazione di un errore
//    enum eSetting { sSet = 1, sReset };
//    bool sendErrBit(eSetting set, int bit);

//    virtual prfData::tResultData getErr(prfData& target) const;

    bool isOnEmerg() const;

  protected:
  private:

//    DWDATA addrAlarm;
//    DWDATA numWordAlarm;

//    DWDATA offsEvent;
//    DWDATA numWordEvent;

//    bool useFlagAlarm;
//    DWDATA maskAlarm;
//    DWDATA maskEvent;
//    DWDATA wordStat;


    enum howCheckRunning {
        alwaysFalse, // non verifica e torna false
        alwaysTrue,  // non verifica e torna true
        useBits,     // setta i vari bit ed usa la mask, è sufficiente un solo bit attivo
        useVal       // usa valori numerici, in tal caso è necessario caricare un array
        };
    howCheckRunning useRunning;
    DWDATA wordRunning;
    DWDATA maskRunning;
    DWDATA offsRunning;

#define MAX_VAL_STATUS 6
    DWDATA pVal[MAX_VAL_STATUS];

//    DWDATA wordResetAlarm;
    // se valore negativo non viene usato
//    int bitReset;

  private:
    typedef perif baseClass;

};
//-----------------------------------------------------------------
#endif

