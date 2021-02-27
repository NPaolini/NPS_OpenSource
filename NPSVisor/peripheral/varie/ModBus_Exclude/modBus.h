//------------------ modBus.H -------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef MODBUS_H_
#define MODBUS_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "ConnClass.h"
//-----------------------------------------------------------------------------
#define COMMAND_READ   3
#define COMMAND_SING_WRITE 6
#define COMMAND_WRITE 16
//-----------------------------------------------------------------------------
struct sModBus
{
  WORD addr;    // indirizzo di inizio
  WORD len;     // numero di word da leggere/scrivere (valore se scrittura singola)
  BYTE device;  // strumento destinatario
  BYTE dimType; // dimensione del tipo
  bool writeSingle; // se attivo necessità della scrittura con codice 6
};
//-----------------------------------------------------------------------------
void mySwab(void *from, void *to, int n);
//-----------------------------------------------------------------------------
class gModBus
{
  public:
    gModBus();
    virtual ~gModBus();

    // tornano false se si verifica un qualsiasi errore.
    // Se occorresse sapere il tipo di errore si può aggiungere
    // una variabile per contenerne lo stato ed un metodo per
    // prelevarlo
    virtual bool read(const sModBus& data, BYTE* buff) const = 0;
    virtual bool write(const sModBus& data, const BYTE* buff) const = 0;

    virtual bool init() { return Conn->open(); }
    void setFullSwap(bool set) { fullSwap = set; }

  protected:
    mutable BYTE Buff[4096];
    PConnBase* Conn;
    WORD calc_crc(BYTE* buff, int len) const;
    bool hasError(const sModBus& data) const;
    bool fullSwap;
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif
