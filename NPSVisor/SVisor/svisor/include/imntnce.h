//-------- imntnce.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef IMNTNCE_H_
#define IMNTNCE_H_

#include "setPack.h"

class IMaint
{
  public:
    virtual ~IMaint() { }

    // ferma il thread (se attivo), ricarica i dati da file e
    // riparte col thread
    virtual void start() = 0;

    // ferma il conteggio (per es. in caso di arresto della macchina)
    virtual void stop() = 0;

    // resetta i soli dati scaduti (prima ferma e poi riavvia)
    virtual void reset() = 0;

    virtual bool isRunning() = 0;

};

extern IMaint *allocMaint();
#include "restorePack.h"

#endif
