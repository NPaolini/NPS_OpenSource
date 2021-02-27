//------------- modBus.cpp ----------------------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "connClass.h"
#include "modBus.h"
#include <stdlib.h>
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
gModBus::gModBus() : Conn(0), fullSwap(true) {  }
//-----------------------------------------------------------------------------
gModBus::~gModBus()
{
  delete Conn;
}
//-----------------------------------------------------
WORD gModBus::calc_crc(BYTE* buff, int len) const
{
  WORD crc = 0xffff;
  for(int i = 0; i < len; ++i) {
    crc ^= buff[i];
    for(int j = 0; j < 8; ++j) {
      bool carry = crc & 1;
      crc >>= 1;
      if(carry)
        crc ^= 0xA001;
      }
    }
//#define SWAP_CRC
#ifdef SWAP_CRC
  buff[len] = (BYTE)(crc >> 8);
  buff[len + 1] = (BYTE)(crc & 0xff);
#else
  buff[len] = (BYTE)(crc & 0xff);
  buff[len + 1] = (BYTE)(crc >> 8);
#endif
  return crc;
}
//-----------------------------------------------------------------------------
bool gModBus::hasError(const sModBus& data) const
{
  if(Buff[1] & 0x80)
    return true;
  switch(Buff[1]) {

    case COMMAND_READ:
//      return false;
      return Buff[2] != (BYTE)(data.len * sizeof(WORD));

    case COMMAND_WRITE:
      do {
        WORD addr = (WORD)(((WORD)Buff[2] << 8) | Buff[3]);
        return addr != data.addr;
         } while(false);
      break;

    }
  return true;
}
//-----------------------------------------------------
// azzera sempre dopo lo shift
// non dovrebbe essere necessario se valori unsigned
  #define SWAB__S(v) ( (((v) >> 24) & 0xff)     | \
                      (((v) << 24) & 0xff000000)  | \
                      (((v) >> 8) & 0xff00)  | \
                      (((v) << 8) & 0xff0000) \
                    )
// se valori unsigned
  #define SWAB__(v) ( ((v) >> 24)     | \
                      ((v) << 24)  | \
                      (((v) >> 8) & 0xff00)  | \
                      (((v) << 8) & 0xff0000) \
                    )
//-----------------------------------------------------
void mySwab(void *from, void *to, int n)
{
  n >>= 2;
  DWORD *t = reinterpret_cast<DWORD*>(to);
  DWORD *s = reinterpret_cast<DWORD*>(from);
  for(int i = 0; i < n; ++i)
    t[i] = SWAB__(s[i]);
}
//-----------------------------------------------------------------------------
