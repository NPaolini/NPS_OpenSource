//------------------ modBus_ser.H -------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef MODBUS_SER_H_
#define MODBUS_SER_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "modbus.h"
//-----------------------------------------------------------------------------
class gModBusSer : public gModBus
{
  public:
    // param può essere l'ip o i parametri per la seriale, dipende da port
    gModBusSer(paramConn pc);
//    ~gModBusSer();

    // tornano false se si verifica un qualsiasi errore.
    // Se occorresse sapere il tipo di errore si può aggiungere
    // una variabile per contenerne lo stato ed un metodo per
    // prelevarlo
    bool read(const sModBus& data, BYTE* buff) const;
    bool write(const sModBus& data, const BYTE* buff) const;

  protected:
    bool writeSingle(const sModBus& data) const;
    bool verifyMultipleWrite(const sModBus& data) const;

};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif
