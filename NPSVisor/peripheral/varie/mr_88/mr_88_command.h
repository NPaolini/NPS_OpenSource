//--------------- mr_88_command.h ----------------------------------------
//------------------------------------------------------------------------
#ifndef mr_88_command_H_
#define mr_88_command_H_
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#include <stdlib.h>
#include "p_file.h"
#include "hdrmsg.h"
#include "commidt.h"
#include "prfData.h"
#include "ConnClass.h"

#include "p_util.h"
//------------------------------------------------------------------------
// dimensione massima di pachetto ... esagerata
#define MAX_DATA_REC  32
//------------------------------------------------------------------------
//#define TEST_RECEIVE
//------------------------------------------------------------------------
struct info_packet
{
  BYTE code;
  int num_data;
  BYTE data[MAX_DATA_REC];

  info_packet(BYTE code = 0, int num_data = 0) : code(code), num_data(num_data) { ZeroMemory(data, sizeof(data)); }

  bool receive(PConnBase* Conn); // gestisce i byte speciali, calcola il checksum e torna false se non corrisponde

  bool send(PConnBase* Conn); // gestisce i byte speciali


  BYTE calcChecksum();
  BYTE getChecksum();
  void setChecksum(BYTE checksum);
};
//------------------------------------------------------------------------
class command
{
  public:
    command(BYTE cmd, int addr, int lenData) : Cmd(cmd), Addr(addr), LenData(lenData), Updated(false) { ip.code = cmd; }
    virtual ~command() {}
    virtual bool run(PConnBase* Conn, LPDWORD buff);
    int get_init_addr() { return Addr; }
    int get_len_data() { return LenData; }
    bool isUpdated() { return Updated; }
    void setUpdated() { Updated = true; }
  protected:
    // buff punta all'inizio dei dati
    virtual bool fillCommandSend(LPDWORD buff) = 0;
    virtual bool afterSend() { return true; }
    // buff è già indirizzato e allineato
    virtual bool elabReceived(LPDWORD buff) = 0;
    info_packet ip;
    BYTE Cmd;
    int Addr;
    int LenData;
    PConnBase* Conn;
    bool Updated;
};
//------------------------------------------------------------------------
class command_receive : public command
{
  private:
    typedef command baseClass;
  public:
    command_receive(BYTE cmd, int addr, int lenData) : baseClass(cmd, addr, lenData) {}
    virtual ~command_receive() {}
  protected:
    virtual bool fillCommandSend(LPDWORD buff);
    virtual bool elabReceived(LPDWORD buff);
};
//------------------------------------------------------------------------
class command_send : public command
{
  private:
    typedef command baseClass;
  public:
    command_send(BYTE cmd, int addr, int lenData) : baseClass(cmd, addr, lenData) {}
    virtual ~command_send() {}
    virtual void replace_addr(int new_addr) { Addr = new_addr; }
  protected:
    virtual bool fillCommandSend(LPDWORD buff);
    virtual bool elabReceived(LPDWORD buff);
};
//------------------------------------------------------------------------
class command_receive_input_channel : public command_receive
{
  private:
    typedef command_receive baseClass;
  public:
    command_receive_input_channel(BYTE cmd, int addr, int lenData) : baseClass(cmd, addr, lenData) {}
    virtual ~command_receive_input_channel() {}
  protected:
    virtual bool elabReceived(LPDWORD buff);
};
//------------------------------------------------------------------------
class command_send_input_channel : public command_send
{
  private:
    typedef command_send baseClass;
  public:
    command_send_input_channel(BYTE cmd, int addr, int lenData) : baseClass(cmd, addr, lenData) {}
    virtual ~command_send_input_channel() {}
  protected:
    virtual bool fillCommandSend(LPDWORD buff);
};
//------------------------------------------------------------------------
#endif
