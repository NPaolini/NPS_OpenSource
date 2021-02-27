//--------------- mr_88_command.cpp -------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "mr_88_command.h"
//-----------------------------------------------------------
#define MAX_REPEAT   20
#define WAIT_REPEAT 100
//-----------------------------------------------------------
static bool waitByte(PConnBase* com)
{
  for(uint i = 0; i < MAX_REPEAT; ++i) {
    if(com->has_string())
      return true;
    Sleep(WAIT_REPEAT);
    }
  return false;
}
//-----------------------------------------------------------
static bool waitFor(PConnBase* com, BYTE byte)
{
  BYTE t;
  for(;;) {
    if(!waitByte(com))
      return false;
    com->read_string(&t, 1);
    if(t == byte)
      return true;
    }
  return false;
}
//------------------------------------------------------------------------
// è impostato per un solo mixer, quindi indirizzo uno
#define ADDR 1

#define STX 0x7E
#define ETX 0x7D
#define RES 0x7F

#define REPLACE(v) ((v) | 0xf0)
#define REMOVE(v) ((v) & 0x7f)

#define RETURN_CODE(a) ((a) | 0x80)
//------------------------------------------------------------------------
bool info_packet::receive(PConnBase* Conn)
{
  if(!waitFor(Conn, STX))
    return false;
  if(!waitByte(Conn))
    return false;
  BYTE addr;
  Conn->read_string(&addr, 1);
  Conn->read_string(&code, 1);
  switch(code) {
    case RES:
      Conn->read_string(&code, 1);
      code = REMOVE(code);
      break;
    }
  for(uint i = 0; i < SIZE_A(data); ++i) {
    if(!waitByte(Conn))
      return false;
    Conn->read_string(data + i, 1);
    switch(data[i]) {
      case RES:
        Conn->read_string(data + i, 1);
        data[i] = REMOVE(data[i]);
        break;
      case ETX:
        num_data = i;
        BYTE chksum = calcChecksum();
        return chksum == getChecksum();
      }
    }
  return false;
}
//------------------------------------------------------------------------
bool info_packet::send(PConnBase* Conn)
{
  BYTE t[] = { STX, ADDR, code, 0 };
  int len = SIZE_A(t) - 1;
  switch(code) {
    case STX:
    case ETX:
    case RES:
      t[SIZE_A(t) - 2] = RES;
      t[SIZE_A(t) - 1] = REPLACE(code);
      len = SIZE_A(t);
      break;
    }
  if(!Conn->write_string(t, len))
    return false;
  for(int i = 0; i < num_data; ++i) {
    switch(data[i]) {
      case STX:
      case ETX:
      case RES:
        t[0] = RES;
        t[1] = REPLACE(data[i]);
        if(!Conn->write_string(t, 2))
          return false;
        break;
      default:
        if(!Conn->write_string(data + i, 1))
          return false;
        break;
      }
    }
  t[0] = ETX;
  if(!Conn->write_string(t, 1))
    return false;
  return true;
}
//------------------------------------------------------------------------
BYTE info_packet::calcChecksum()
{
  const BYTE addr = 1;
  BYTE sum = code + addr;
  for(int i = 0; i < num_data - 1; ++i)
    sum += data[i];
  return ~sum;
}
//------------------------------------------------------------------------
BYTE info_packet::getChecksum()
{
  if(num_data > 0 && num_data < SIZE_A(data) - 1)
    return data[num_data - 1];
  return 0;
}
//------------------------------------------------------------------------
void info_packet::setChecksum(BYTE checksum)
{
  if(num_data > 0 && num_data < SIZE_A(data) - 1)
    data[num_data - 1] = checksum;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
bool command::run(PConnBase* Conn, LPDWORD buff)
{
  if(!fillCommandSend(buff))
    return false;
  if(!ip.send(Conn))
    return false;
#ifndef TEST_RECEIVE
  if(!afterSend())
    return false;
  if(!ip.receive(Conn))
    return false;
  if(!elabReceived(buff))
    return false;
#endif
  return true;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
bool command_receive::fillCommandSend(LPDWORD buff)
{
  ip.code = Cmd;
  ip.num_data = 1; // solo chksum
  ip.setChecksum(ip.calcChecksum()); // equivale a ~code
  return true;
}
//------------------------------------------------------------------------
bool command_receive::elabReceived(LPDWORD buff)
{
  int len = min(LenData, ip.num_data -1);
  for(int i = 0; i < len; ++i)
    buff[i] = (DWORD)(long)char(ip.data[i]);
  for(int i = len; i < LenData; ++i)
    buff[i] = 0;
  return ip.code == RETURN_CODE(Cmd);
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
bool command_send::elabReceived(LPDWORD buff)
{
  return ip.code == RETURN_CODE(Cmd);
}
//------------------------------------------------------------------------
bool command_send::fillCommandSend(LPDWORD buff)
{
  ip.code = Cmd;
  ip.num_data = LenData + 1;
  for(int i = 0; i < LenData; ++i)
    ip.data[i] = (BYTE)buff[i];//(BYTE)buff[Addr + i];

  ip.setChecksum(ip.calcChecksum());
  return true;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#define GATE_LEVEL 8
#define MUTE_GATE_LEVEL 0x23
#define OFFS_GATE_LEVEL 6
//------------------------------------------------------------------------
bool command_receive_input_channel::elabReceived(LPDWORD buff)
{
#if false
  return baseClass::elabReceived(buff);
#else
  if(baseClass::elabReceived(buff)) {
    if(MUTE_GATE_LEVEL == buff[GATE_LEVEL])
      buff[GATE_LEVEL] = 0;
    else
      buff[GATE_LEVEL] += OFFS_GATE_LEVEL;
    return true;
    }
  return false;
#endif
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
bool command_send_input_channel::fillCommandSend(LPDWORD buff)
{
#if false
  return baseClass::fillCommandSend(buff);
#else
  if(baseClass::fillCommandSend(buff)) {
    if(ip.data[GATE_LEVEL] < OFFS_GATE_LEVEL)
      ip.data[GATE_LEVEL] = MUTE_GATE_LEVEL;
    else
      ip.data[GATE_LEVEL] -= OFFS_GATE_LEVEL;

    ip.setChecksum(ip.calcChecksum());
    return true;
    }
  return false;
#endif
}
//------------------------------------------------------------------------
