//------------- modBus.cpp ----------------------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "connClass.h"
#include "modBus_ser.h"
#include <stdlib.h>
//-----------------------------------------------------------------------------
#define DIM_READ  8
#define DIM_WRITE_RESULT 8
// dimensione del pacchetto di ritorno in caso di errore
#define MIN_RESULT 5
#define OFFS_DATA_READ 3
//-----------------------------------------------------------------------------
#define OFFS_DATA_WRITE 7
//-----------------------------------------------------------------------------
// su un manuale il crc usa il little endian e su un altro usa
// il big endian (come i dati). Occorre verificare e commentare
// o meno il #define.
// ?? Sul manuale che usa il big endian per il crc usa invece
// il little endian sul codice di esempio del calcolo.

//#define SWAP_CRC
//-----------------------------------------------------------------------------
/*
struttura pacchetto lettura
BYTE 1 -> device
BYTE 2 -> function
BYTE 3 -> MSB addr
BYTE 4 -> LSB addr
BYTE 5 -> MSB number of words
BYTE 6 -> LSB number of words

BYTE 7 -> LSB CRC
BYTE 8 -> MSB CRC
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

BYTE ? -> LSB CRC
BYTE ? -> MSB CRC
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

BYTE ? -> LSB CRC
BYTE ? -> MSB CRC
-----------------------------
struttura pacchetto responso scrittura
BYTE 1 -> device
BYTE 2 -> function
BYTE 3 -> MSB addr
BYTE 4 -> LSB addr
BYTE 5 -> MSB number of words
BYTE 6 -> LSB number of words

BYTE 7 -> LSB CRC
BYTE 8 -> MSB CRC
-----------------------------
-----------------------------
struttura pacchetto scrittura singola, il responso è lo stesso identico pacchetto
BYTE 1 -> device
BYTE 2 -> function
BYTE 3 -> MSB addr
BYTE 4 -> LSB addr
BYTE 5 -> MSB value
BYTE 6 -> LSB value

BYTE 7 -> LSB CRC
BYTE 8 -> MSB CRC
-----------------------------
struttura pacchetto responso errore
BYTE 1 -> device
BYTE 2 -> function with most significant bit set to 1
BYTE 3 -> error code

BYTE 4 -> LSB CRC
BYTE 5 -> MSB CRC
-----------------------------
error code
02 -> illegal data address
03 -> illegal data value
*/
//-----------------------------------------------------------------------------
gModBusSer::gModBusSer(paramConn pc)
{
  Conn = new PConnBase(pc, 0);
}
//-----------------------------------------------------------------------------
inline
static void fillCommand(BYTE* command, BYTE funct, const sModBus& data)
{
  command[0] = data.device;
  command[1] = funct;
  command[2] = (BYTE)(data.addr >> 8);
  command[3] = (BYTE)(data.addr & 0xff);
  command[4] = (BYTE)(data.len >> 8);
  command[5] = (BYTE)(data.len & 0xff);
}
//-----------------------------------------------------------------------------
inline
WORD get_crc(const BYTE* buff, int len)
{
#ifdef SWAP_CRC
  WORD crc = (WORD)(((WORD)buff[len] << 8) | buff[len + 1]);
#else
  WORD crc = (WORD)(((WORD)buff[len + 1] << 8) | buff[len]);
#endif
  return crc;
}
//-----------------------------------------------------------
#define VERIFY_COM_HAS_STRING
//-----------------------------------------------------------
#ifdef VERIFY_COM_HAS_STRING
  #define MAX_REPEAT 160
  #define DELAY_COM_HAS_STRING 50
bool verifyCom(PConnBase* com)
{
  for(int i = 0; i < MAX_REPEAT; ++i) {
    Sleep(DELAY_COM_HAS_STRING);
    if(com->has_string())
      return true;
    }
  return false;
}
  #define CHECK_COM(a) if(!verifyCom(a)) return false;
#else
  #define CHECK_COM(a)
#endif
//-----------------------------------------------------------------------------
static void resetComm(PConnBase* conn)
{
  conn->passThrough(cReset, 0);
}
//-----------------------------------------------------------------------------
bool gModBusSer::read(const sModBus& data, BYTE* target) const
{
  BYTE command[DIM_READ + 2];
  fillCommand(command, COMMAND_READ, data);

  calc_crc(command, DIM_READ - 2);

  resetComm(Conn);
  if(DIM_READ != Conn->write_string(command, DIM_READ))
    return false;

  CHECK_COM(Conn);

  if(MIN_RESULT != Conn->read_string(Buff, MIN_RESULT))
    return false;

  if(hasError(data))
    return false;

  uint toRead = data.len * sizeof(WORD) - (MIN_RESULT - OFFS_DATA_READ) + 2;

  if(toRead != Conn->read_string(Buff + MIN_RESULT, toRead))
    return false;

  // preleva quello letto
  WORD crc = get_crc(Buff, toRead + MIN_RESULT - 2);

  // ricalcola
  WORD crc2 = calc_crc(Buff, toRead + MIN_RESULT - 2);

  if(crc != crc2)
    return false;

  if(!fullSwap || sizeof(WORD) == data.dimType)
    _swab((char*)Buff + OFFS_DATA_READ, (char*)target, data.len * sizeof(WORD));
  else
    mySwab((char*)Buff + OFFS_DATA_READ, (char*)target, data.len * sizeof(WORD));
  return true;
}
//-----------------------------------------------------------------------------
bool gModBusSer::writeSingle(const sModBus& data) const
{
// si usa lo stesso pacchetto della lettura con al posto della len c'è il valore da inviare
// il ritorno è lo stesso pacchetto
  BYTE command[DIM_READ + 2];
  fillCommand(command, COMMAND_READ, data);
  command[1] = COMMAND_SING_WRITE;

  calc_crc(command, DIM_READ - 2);

  resetComm(Conn);
  if(DIM_READ != Conn->write_string(command, DIM_READ))
    return false;

  CHECK_COM(Conn);

  if(MIN_RESULT != Conn->read_string(Buff, MIN_RESULT))
    return false;

  if(memcmp(command, Buff, MIN_RESULT))
    return false;

  uint toRead = DIM_READ - MIN_RESULT;

  if(toRead != Conn->read_string(Buff + MIN_RESULT, toRead))
    return false;

  WORD val = (WORD)(((WORD)Buff[5] << 8) | Buff[6]);
  return val == data.len;
}
//-----------------------------------------------------------------------------
bool gModBusSer::write(const sModBus& data, const BYTE* source) const
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
  WORD* pS = (WORD*)source;
  WORD nCoil = data.len;

  Buff[0] = data.device;
  Buff[1] = (BYTE)COMMAND_WRITE;
  Buff[2] = (BYTE)(data.addr >> 8);
  Buff[3] = (BYTE)(data.addr & 0xff);

  Buff[4] = (BYTE)(nCoil >> 8);
  Buff[5] = (BYTE)(nCoil & 0xff);
  Buff[6] = (BYTE)(nCoil * sizeof(WORD));

  WORD* pB = (WORD*)(Buff + OFFS_DATA_WRITE);

  if(!fullSwap || sizeof(WORD) == data.dimType)
    _swab((char*)pS, (char*)pB, nCoil * sizeof(WORD));
  else
    mySwab((char*)pS, (char*)pB, nCoil * sizeof(WORD));

  uint lenPacket = OFFS_DATA_WRITE + nCoil * sizeof(WORD);
  calc_crc(Buff, lenPacket);

  lenPacket += 2;

  resetComm(Conn);
  if(lenPacket != Conn->write_string(Buff, lenPacket))
    return false;

  return verifyMultipleWrite(data);
}
//-----------------------------------------------------------------------------
bool gModBusSer::verifyMultipleWrite(const sModBus& data)  const
{
  CHECK_COM(Conn);
  if(MIN_RESULT != Conn->read_string(Buff, MIN_RESULT))
    return false;

  if(hasError(data))
    return false;

  uint toRead = DIM_WRITE_RESULT - MIN_RESULT;

  if(toRead != Conn->read_string(Buff + MIN_RESULT, toRead))
    return false;

  WORD crc = get_crc(Buff, DIM_WRITE_RESULT - 2);
  WORD crc2 = calc_crc(Buff, DIM_WRITE_RESULT - 2);

  if(crc != crc2)
    return false;
  return true;
}
//-----------------------------------------------------------------------------
