//----------- log_ord.h -------------------------------
#ifndef __LOG_ORD__
#define __LOG_ORD__

#include "log_stat.h"

//--------------------------------------------------------------
// stato/evento di gestione ordine
enum ordStat {
    oUndef,       // stato inconsistente
    oSaveNoActive,// salva i dati passati senza attivare l'ordine
    oLoad,        // inizializza dai dati passati (in pratica legge solo il nome
                  // del file da aprire)
    oSave,        // salva l'ordine correntemente attivo
    oSuspended,   // sospende l'ordine, per chiusura programma o a richiesta
    oActive,      // carica nell'oggetto passato i dati correnti
    oEnded,       // finito l'ordine, salva e lo sposta nello storico
    };
// la classe di implementazione di gestione dati usa lo stesso enumeratore
// per notificare i cambiamenti nell'ordine attivo e riempire la classe
// val_ordine (vedere ImplJobData::chgOrd() )
// Le azioni e i flag gestiti sono:
//    oSave,        // riempie la struttura, richiamata nel salvataggio automatico
//                  // circa ogni minuto
//    oSuspended,
//    oEnded,

// la classe val_order gestisce i dati
class val_order;

//--------------------------------------------------------------
// classe di uso generale per gestione ordini
// l'implementazione deve instanziare un oggetto per i dati
class stat_order : public stat_gen
{
  public:
    enum whichDir { eCurrent, eHistory };
    stat_order();
    ~stat_order();

    void set_data(const val_gen *val);
    const val_order& get_data();

    virtual void get_data(val_gen& buff);
    virtual int save() { return action(false); }
    virtual int init() { return action(true); }

    virtual bool exportData(P_File &f);
    void setDir(whichDir which = eHistory) { Which = which; }

    log_status::errEvent setEvent(const val_gen* val);

  protected:

    virtual UDimF write(P_File &f);
    virtual UDimF read(P_File &f);
    val_order *data;

  private:
    int action(bool init);
    whichDir Which;
};
//--------------------------------------------------------------
//--------------------------------------------------------------
stat_gen *allocOrder();
//--------------------------------------------------------------
//--------------------------------------------------------------
#endif

