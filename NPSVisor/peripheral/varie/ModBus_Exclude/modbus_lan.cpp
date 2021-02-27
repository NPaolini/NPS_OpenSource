//------------- modBusRTU_Lan.cpp -------------------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "modBus_lan.h"
#include <stdlib.h>
//-----------------------------------------------------------------------------
#define DIM_READ  6
#define DIM_WRITE_RESULT 6
// dimensione minima del pacchetto di ritorno con (errore o primo dato)
// in realtà la dimensione minima è tre, ma per mantenere il codice equivalente
// alla versione con crc, si lascia a cinque, ma vengono comunque letti prima
// tre byte, verificato che non ci sia errore e poi gli altri due.
#define MIN_RESULT 5
#define OFFS_DATA_READ 3

#define MIN_RESULT_LAN 3
//-----------------------------------------------------------------------------
#define OFFS_DATA_WRITE_S 4
#define OFFS_DATA_WRITE 7
//-----------------------------------------------------------------------------
// dimensione aggiuntiva per la connessione lan
#define OFFS_BY_LAN 6
#define DIM_FULL_READ (DIM_READ + OFFS_BY_LAN)
//-----------------------------------------------------------------------------
/*
struttura pacchetto lettura
BYTE 1 -> device
BYTE 2 -> function
BYTE 3 -> MSB addr
BYTE 4 -> LSB addr
BYTE 5 -> MSB number of words
BYTE 6 -> LSB number of words
-----------------------------
struttura pacchetto responso lettura
BYTE 1 -> device
BYTE 2 -> function
BYTE 3 -> number of bytes read
BYTE 4 -> MSB first word
BYTE 5 -> LSB first word
....
BYTE ? -> MSB last word
BYTE ? -> LSB last word
-----------------------------
-----------------------------
struttura pacchetto scrittura
BYTE 1 -> device
BYTE 2 -> function
BYTE 3 -> MSB addr
BYTE 4 -> LSB addr
BYTE 5 -> MSB number of words
BYTE 6 -> LSB number of words
BYTE 7 -> number of data bytes
BYTE 8 -> MSB first word
BYTE 9 -> LSB first word
....
BYTE ? -> MSB last word
BYTE ? -> LSB last word
-----------------------------
struttura pacchetto responso scrittura
BYTE 1 -> device
BYTE 2 -> function
BYTE 3 -> MSB addr
BYTE 4 -> LSB addr
BYTE 5 -> MSB number of words
BYTE 6 -> LSB number of words
-----------------------------
-----------------------------
struttura pacchetto responso errore
BYTE 1 -> device
BYTE 2 -> function with most significant bit set to 1
BYTE 3 -> error code
-----------------------------
error code
02 -> illegal data address
03 -> illegal data value
-----------------------------
pacchetto intestazione per tcp/ip

BYTE 1 -> MSB ID_COUNTER (identificativo transazione del pacchetto)
BYTE 2 -> LSB ID_COUNTER
BYTE 3 -> MSB Protocollo (zero per modbus)
BYTE 4 -> LSB Protocollo
BYTE 5 -> MSB len dati che seguono
BYTE 6 -> LSB len dati che seguono

Il Byte 7 corrisponde al device che deve essere posto a zero se il
server MB è su lan (va invece impostato sul numero del device se c'è
un adattatore seriale-lan di mezzo).
-----------------------------
Il numero di dati che è possibile inviare-ricevere in una volta è:

253 byte + 7 byte header (6 lan + 1 device)
*/
//-----------------------------------------------------------------------------
class transactCounter
{
  public:
    static WORD getCurrent() { return transact; }
    static WORD getIncrCurrent() { return ++transact; }
    static void reset() { transact = 0; }
  private:
    static WORD transact;
};
//-----------------------------------------------------------------------------
WORD transactCounter::transact;
//-----------------------------------------------------------------------------
gModBusLan::gModBusLan(DWORD ip, DWORD port)
{
  Conn = new PConnBase(ip, port, 0, false);
}
//-----------------------------------------------------------------------------
inline
static void fillCommand(BYTE* command, const sModBus& data, BYTE cmd, bool forRead)
{
  WORD transact = transactCounter::getIncrCurrent();
  command[0] = (BYTE)(transact >> 8);
  command[1] = (BYTE)(transact & 0xff);
  command[2] = 0;
  command[3] = 0;
  WORD len;
  if(forRead)
    len = DIM_READ;
  else
    len = data.len * sizeof(WORD) + OFFS_DATA_WRITE;
  command[4] = (BYTE)(len >> 8);
  command[5] = (BYTE)(len & 0xff);

#if 1
  command[6] = data.device ? data.device : 0xff;
#else
  command[6] = data.device;
#endif
  command[7] = cmd;
  command[8] = (BYTE)(data.addr >> 8);
  command[9] = (BYTE)(data.addr & 0xff);
  command[10] = (BYTE)(data.len >> 8);
  command[11] = (BYTE)(data.len & 0xff);
}
//-----------------------------------------------------------------------------
bool gModBusLan::hasErrorLan(LPBYTE command) const
{
  LPDWORD pComm = (LPDWORD)command;
  LPDWORD pBuff = (LPDWORD)Buff;
  return *pComm != * pBuff;
}
//-----------------------------------------------------------------------------
// MIN_RESULT è la dimensione minima che comunque riceve in risposta.
// Se ci sono errori si trovano negli ultimi [due] byte, altrimenti questi
// indicano i primi dati letti (corrispondenti a MIN_RESULT - OFFS_DATA_READ).
//-----------------------------------------------------------------------------
bool gModBusLan::read(const sModBus& data, BYTE* target) const
{
  BYTE command[DIM_READ + 2 + OFFS_BY_LAN];
  fillCommand(command, data, COMMAND_READ, true);

  if(DIM_FULL_READ != Conn->write_string(command, DIM_FULL_READ))
    return false;

  SleepEx(50, 0);
  int n = Conn->read_string(Buff, OFFS_BY_LAN);
  if(OFFS_BY_LAN != n)
    return false;
  if(hasErrorLan(command))
    return false;
  n = Conn->read_string(Buff, MIN_RESULT_LAN);
  if(MIN_RESULT_LAN != n)
    return false;

  if(hasError(data))
    return false;

  Conn->read_string(Buff + MIN_RESULT_LAN, 2);

  uint toRead = data.len * sizeof(WORD) - (MIN_RESULT - OFFS_DATA_READ);

  if(toRead != Conn->read_string(Buff + MIN_RESULT, toRead))
    return false;

  LPDWORD t = (LPDWORD)target;
  uint toCopy = data.len;

  if(!fullSwap || sizeof(WORD) == data.dimType)
    _swab((char*)Buff + OFFS_DATA_READ, (char*)target, data.len * sizeof(WORD));
  else
    mySwab((char*)Buff + OFFS_DATA_READ, (char*)target, data.len * sizeof(WORD));

  return true;
}
//-----------------------------------------------------------------------------
bool gModBusLan::writeSingle(const sModBus& data) const
{
  BYTE command[DIM_READ + 2 + OFFS_BY_LAN];
  fillCommand(command, data, COMMAND_READ, true);
  command[7] = COMMAND_SING_WRITE;
  if(DIM_FULL_READ != Conn->write_string(command, DIM_FULL_READ))
    return false;

  SleepEx(50, 0);
  int n = Conn->read_string(Buff, OFFS_BY_LAN);
  if(OFFS_BY_LAN != n)
    return false;
  if(hasErrorLan(command))
    return false;
  n = Conn->read_string(Buff, MIN_RESULT_LAN);
  if(MIN_RESULT_LAN != n)
    return false;

  if(memcmp(command + OFFS_BY_LAN, Buff, n))
    return false;

  uint toRead = DIM_READ - MIN_RESULT_LAN;

  if(toRead != Conn->read_string(Buff + MIN_RESULT_LAN, toRead))
    return false;

  WORD val = (WORD)(((WORD)Buff[5] << 8) | Buff[6]);
  return val == data.len;
}
//-----------------------------------------------------------------------------
bool gModBusLan::write(const sModBus& data, const BYTE* source) const
{
  if(data.writeSingle) {
    sModBus d = data;
    uint repeat = data.len;
    LPWORD pW = (LPWORD)source;
    bool success = true;
    for(uint i = 0; i < repeat; ++i, ++d.addr, ++pW) {
      _swab((char*)pW, (char*)&d.len, sizeof(WORD));
      success &= writeSingle(d);
      }
    return success;
    }

  fillCommand(Buff, data, COMMAND_WRITE, false);
  Buff[OFFS_DATA_WRITE + OFFS_BY_LAN - 1] = data.len * sizeof WORD;

  WORD* pS = (WORD*)source;
  WORD nCoil = data.len;

  WORD* pB = (WORD*)(Buff + OFFS_DATA_WRITE + OFFS_BY_LAN);

  LPDWORD s = (LPDWORD)((char*)source);

  if(!fullSwap || sizeof(WORD) == data.dimType)
    _swab((char*)pS, (char*)pB, nCoil * sizeof(WORD));
  else
    mySwab((char*)pS, (char*)pB, nCoil * sizeof(WORD));

  uint lenPacket = OFFS_DATA_WRITE + OFFS_BY_LAN + nCoil * sizeof(WORD);

  if(lenPacket != Conn->write_string(Buff, lenPacket))
    return false;

  SleepEx(50, 0);

  return verifyMultipleWrite(data, Buff);
}
//-----------------------------------------------------------------------------
#define RESULT_ERR 3
#define END_RESULT_WRITE (DIM_WRITE_RESULT - RESULT_ERR)
//-----------------------------------------------------------------------------
bool gModBusLan::verifyMultipleWrite(const sModBus& data, LPBYTE command) const
{
  BYTE tmp[OFFS_BY_LAN];

  if(OFFS_BY_LAN != Conn->read_string(tmp, OFFS_BY_LAN))
    return false;
  if(hasErrorLan(tmp))
    return false;

  if(RESULT_ERR != Conn->read_string(Buff, RESULT_ERR))
    return false;

  if(Buff[1] & 0x80)
    return false;

  if(END_RESULT_WRITE != Conn->read_string(Buff + RESULT_ERR, END_RESULT_WRITE))
    return false;

  if(hasError(data))
    return false;

  return true;
}
//-----------------------------------------------------------------------------
