//----------- log_stat.h -------------------------------
#ifndef __LOG_STATUS__
#define __LOG_STATUS__

#ifndef P_FILE_H_
#include "p_file.h"
#endif

// dimensione massima dei nomi dei file (usa lo stile dos, 8+3)
#define MAXLEN_NAME_FILE_DAT 8

// il primo per il tempo totale di run della macchina
// il secondo per il tempo totale di arresto per allarme
// gli altri per le categorie di allarmi
#define MAX_GRP_ALARM (1 + 1 + 6)

// dimensione dati salvati dalla classe base stat_gen
#define SZ_BASE_STAT (sizeof(FILETIME) * (MAX_GRP_ALARM + 1))

// dimensione dati salvati dalla classe base + classe base val_gen
#define SZ_BASE (SZ_BASE_STAT + sizeof(int))

#define ID_TIME_RUN  0
#define ID_TIME_STOP 1
#define ID_INIT_CATG 2

//--------------------------------------------------------------
#include "setPack.h"
//--------------------------------------------------------------
class val_gen;
class stat_gen;
//--------------------------------------------------------------
class log_status
{
  public:
    enum eRepType { NO_REP, ONLY_ALARMS, ALL_EVENTS };
    log_status(eRepType rep_all = NO_REP);
    ~log_status();

      // tipo di oggetto da richiedere
    enum eType { tOper, tTurn, tOrder, tGlob };

      // eventi gestibili
    enum event { chgOper = 1, chgTurno, chgOrder,
                 onPause, onRun, onAlarm, endAlarm,  setRepAll, onInit,
                 refreshStat, onAckAlarm
               };
      // errori riportati dal gestore eventi
    enum errEvent { EvNoErr, EvErrUnknow, EvErrOper, EvErrTurn, EvErrOrd,
                EvErrAlarm1, EvErrAlarm2, EvErrAlarm3 };

      // l'oggetto puntato da [v] varia a seconda dell'evento
    errEvent setEvent(event type, const val_gen* v = 0);
    const stat_gen* getObj(eType obj) const;
    stat_gen* getObj(eType obj);
  private:
    // ereditano da stat_gen
    stat_gen  *oper;    // gestione statistiche per operatore
    stat_gen  *turno;   // gestione statistiche per turno
    stat_gen  *ordine;  // gestione statistiche per ordine di lavoro
    class stat_alarm      *global;  // gestione statistiche globali della macchina

    eRepType repAll;  // flag di attivazione del report di tutti gli allarmi
    class rep_alarm       *allAlarm;// gestione particolareggiata degli allarmi

//    errEvent chgOperTurn(stat_oper_turn *obj, const val_gen* val, BYTE cod);
    errEvent chgAlarm(const val_gen* val, stat_gen *stat[], log_status::event type);
    bool readOnly;

};
//---------------------------------------------------------------
//--------------------------------------------------------------
// struttura dati base per passaggio parametri
class val_gen
{
  public:
    val_gen() : id(0), val(0) {}
    virtual ~val_gen() {}
    int id;
    DWORD val;
    virtual UDimF write(P_File &f);
    virtual UDimF read(P_File &f);
    virtual bool exportData(P_File &) { return true; }
    virtual bool exportHeader(P_File &) { return true; }
    const val_gen& operator=(const val_gen& other) { clone(other); return *this; }
    virtual void clone(const val_gen& other) { id = other.id; val = other.val; }
};
//--------------------------------------------------------------
//--------------------------------------------------------------
class stat_gen
{
  public:
    stat_gen();
    virtual ~stat_gen();
    void start_alarm(int al);
    void stop_alarm(int al);
    void reset_alarm(int al);
    virtual int save() = 0;
    virtual int init();
    virtual bool exportData(P_File &f);
    virtual bool exportHeader(P_File &f);

    virtual log_status::errEvent setEvent(const val_gen* = 0)
        { return log_status::EvNoErr; }

    // caricano il tempo totale di permanenza nello stato di allarme
    // dell'oggetto[id] della classe status_alarm
    // tornano false se [id] è fuori range
    bool getTime(FILETIME *ft, uint id) const;
    bool getTime(SYSTEMTIME *st, uint id) const;

  protected:
    virtual UDimF write(P_File &f);
    virtual UDimF read(P_File &f);

    class status_alarm *alarm[MAX_GRP_ALARM];
};
//--------------------------------------------------------------
class stat_alarm : public stat_gen
{
  public:
    stat_alarm();
    virtual int save() {  return action(false); }
    virtual int init() {  return action(true); }
    virtual bool exportData(LPCTSTR file, P_File &f);
    void fillTotalTime(LPCTSTR file, FILETIME *ft, int fromDay, int toDay);
  private:
    int action(bool init);
    // rende inaccessibile la export di base, occorre passargli
    // anche il nome del file
    virtual bool exportData(P_File &f) { return stat_gen::exportData(f); }
};
//--------------------------------------------------------------
//--------------------------------------------------------------
struct infoAssocAlarm;
//--------------------------------------------------------------
class rep_alarm
{
  public:
    void startTime(int n, const infoAssocAlarm& iaa) { setTime(n, true, iaa); }
    void stopTime(int n, const infoAssocAlarm& iaa) { setTime(n, false, iaa); }
    virtual bool exportData(LPCTSTR file, P_File &f);
  private:
    void setTime(int n, bool start, const infoAssocAlarm& iaa);
};
//--------------------------------------------------------------
void check_old_rep();
//--------------------------------------------------------------
#include "restorePack.h"
//--------------------------------------------------------------
#endif

