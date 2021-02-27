//-------------- prph_modbus.h -----------------------------------------
#ifndef PRPH_MODBUS_H_
#define PRPH_MODBUS_H_
//----------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------
#include "comgperif.h"
#include "modBus.h"
//----------------------------------------------------------------------
class prph_modbus : public gestCommgPerif
{
  private:
    typedef gestCommgPerif baseClass;
  public:
    prph_modbus(LPCTSTR file, WORD flagCommand = 0);
    virtual ~prph_modbus();
    virtual gestComm::ComResult Init();
  protected:

    virtual gestComm::ComResult Send(const addrToComm* pAddr, Perif_TypeCommand type);
    virtual gestComm::ComResult SendData(addrToComm* pAddr);
    virtual gestComm::ComResult Receive(const addrToComm* pAddr, BDATA* buff);

    virtual gestComm::ComResult ReceiveError(const addrToComm* pAddr, bool set);

    void verifySet(setOfString& set);

  private:
    DWORD maxLenData;
    struct modBusInfo {
      gModBus* mBus;
      // usati per identificare l'oggetto con cui comunicare,
      // se sono a zero indicano che è una connessione seriale,
      // altrimenti è una lan
      DWORD IP;
      DWORD Port;
      
      modBusInfo(DWORD IP = 0, DWORD Port = 0) : IP(IP), Port(Port), mBus(0) {}
      };

    PVect<modBusInfo> modBusSet;
    gModBus* getModBus(const addrToComm* pAddr);
    
    void addToData(const alternAddress& tAddr);

};
//----------------------------------------------------------------------
#endif

