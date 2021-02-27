//-------------- net_remote.H ----------------------------
//---------------------------------------------------------
#ifndef net_remote_H_
#define net_remote_H_
//---------------------------------------------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include "comgperif.h"
#include "p_avl.h"
#include "PCrt_lck.h"
#include "P_File.h"
#include "P_txt.h"
#include "sv_data.h"
#include "connClass.h"
//-----------------------------------------------------------
/*
  Specifiche driver client in rete:

    I driver usano lo stesso file di configurazione il cui nome è:
    infoServer.cfg

    Il contenuto del file fornisce informazioni sul collegamento con il server.
    Le informazioni necessarie sono:
      id_prph,id_periferica da richiedere,indirizzo IP, porta, password

    Nel codice id_prph + 100 sono contenute informazioni su un eventuale server
    che fa da proxy per ricavare l'ip del sVisor-Server nel caso quest'ultimo
    sia su un collegamento con ip dinamico. Se presente il codice + 100, allora
    l'ip e la porta sono quelli del proxy da chiamare per ottenere l'ip e la porta
    veri.
    Le informazioni necessarie sono:
      id_prph + 100, nome del server da chiamare, username, password

      N.B. i tre parametri non devono superare la dimensione di MAX_LEN_NAME (40)

    Nel codice id_prph + 200 è contenuto l'indirizzo logico su cui verra scritto l'ip
    del server remoto in formato testo ascii nella forma canonica dell'ip es. '192.168.0.1'
    es.

    id_prph,prph,ip,porta,psw
    2,4,192.168.0.12,3000,pswServer

    102,server1,remoto1,123456

    nel caso la periferica coincida con il driver è possibile mettere zero e verrà usata
    la periferica del driver
    es.
    2,0,192.168.0.12,3000,pswServer

    la periferica da richiedere in remoto consente di aggregare in un sVisor remoto
    più sVisor per poterne visualizzare contemporaneamente variabili diverse che
    negli sVisor originali sono nella stessa periferica (es. prph2->plc in due diversi sVisor)

    l'ad4 del driver è compatibile con quello collegato fisicamente alla periferica
    è quindi possibile utilizzare lo stesso ad4

N.B. nel setup del sVisor, nel campo usato per immettere il valore da inviare
     al driver, occorre specificare qual'è il passo di incremento delle variabili.
     Si può lasciare a zero se l'indirizzamento è a byte[1] (es. nel plc siemens),
     mentre è necessario impostare a uno se gli indirizzi sono consecutivi[2] per
     il tipo base (es. lettori laser) (non è ancora implementato l'uso diverso dai
     due casi sopra citati)

     [1] es. word x -> 234, word successiva -> 236
     [2] es. dword x -> 24, dword successiva -> 25

----------------------------------
Modifiche 16-05-2013

nell'id del server va aggiunta, in fondo, la scelta se la periferica è in sola lettura (uno) o anche scrittura (zero o nulla)
segue un flag per indicare il passo di incremento, originariamente messo nel setup del svisor.
Per compatibilità si continua anche ad usare il setup, ma questo, se presente, ha la precedenza.
Lo si è messo qui perché è più facile portarlo da un sup all'altro ed evitare di dimenticarselo.

all'id  1 + (1000 * idInit) ci vanno messi in sequenza, server alternativi, nel caso un server non sia online si passa ad un altro
quindi per prph_2 -> 1 + (1000 * 2) = 2001
visto che va in alternativa al server principale, gli unici dati necessari sono porta e ip
es.
2001,3000,192.168.0.15
2002,3000,192.168.0.16

in caso di ip dinamico non si può usare il multiserver
*/
//----------------------------------------------------------------------------
#define MAX_LEN_NAME  40
//----------------------------------------------------------------------------
#define DIM_HOSTNAME MAX_LEN_NAME
#define OFFSET_DYN_IP 100
//----------------------------------------------------------------------------
struct infoDynamicIP_Req
{
//  BYTE code[SIZE_SERIAL_CODE];
  BYTE hostname[DIM_HOSTNAME];
  BYTE username[MAX_LEN_NAME];
  BYTE password[MAX_LEN_NAME];
};
//---------------------------------------------------------
#define MAX_IP_LEN 16
//---------------------------------------------------------
struct serverAddr
{
  DWORD port;
  TCHAR ip[MAX_IP_LEN];
  serverAddr() : port(0) { ZeroMemory(ip, sizeof(ip)); }
};
//---------------------------------------------------------
struct infoServer
{
  char psw[MAX_PASSWORD];
  DWORD id_prph_req;
  DWORD currServer;
  bool readOnly;
  PVect<serverAddr> Addr;
  infoServer() : id_prph_req(0), currServer(0), readOnly(false) { ZeroMemory(psw, sizeof(psw)); }
};
//---------------------------------------------------------
#define RET_IF_GREAT(t) if(other.t < t) return false
#define RET_IF_LESS(t) if(t < other.t) return true

#define RET_IF(t) RET_IF_GREAT(t); RET_IF_LESS(t)
//---------------------------------------------------------
#define USE_VECT_DUAL
//#define USE_AVL_DUAL
//---------------------------------------------------------
#ifdef USE_AVL_DUAL
class dual : public genericSet
#elif defined USE_VECT_DUAL
class dual
#endif
{
  public:
    uint logicAddr;

    uint ipAddr;
    uint port;
    uint db;
    uint addr;

    dual(uint addr = 0,  uint ipAddr = 0, uint port = 0, uint db = 0, uint logicAddr = 0) :
        logicAddr(logicAddr), ipAddr(ipAddr), port(port), db(db), addr(addr) {}

    dual(const dual& other) : logicAddr(other.logicAddr), ipAddr(other.ipAddr),
          port(other.port), db(other.db), addr(other.addr) {}

    bool operator <(const dual& other) const {
      RET_IF(db);
      RET_IF(ipAddr);
      RET_IF(port);
      RET_IF(addr);
      return false;
      }
};
//-----------------------------------------------------------------------------
#ifdef USE_AVL_DUAL
  typedef genericAvl<dual> orderedDual;
#elif defined USE_VECT_DUAL
  typedef PVect<dual> orderedDual;
#endif
//---------------------------------------------------------
enum netConnState { ncs_none, ncs_Actived, ncs_Down };
//-------------------------------------------------------
#define MAX_DWORD_IP 4
//---------------------------------------------------------
class net_remote : public gestCommgPerif
{
  private:
    typedef gestCommgPerif baseClass;
  public:
    net_remote(PWin* owner, LPCTSTR file, WORD flagCommand = 0);
    virtual ~net_remote();
    virtual ComResult Init();
    virtual bool manageCounter(int& countSend, int& countRec, int& countTot, int& consErr);

    void copyRemoteIp(LPCTSTR ip);
  protected:
    virtual void verifySet(class setOfString& set);

    virtual ComResult Send(const addrToComm* pAddr, Perif_TypeCommand type);
    virtual ComResult SendData(addrToComm* pAddr);
    virtual ComResult Receive(const addrToComm* pAddr, BDATA* buff);

  private:
    criticalSect criticalSectionCount;
    criticalSect criticalSectionBuff;
    criticalSect criticalSectionSend;

    void thSetErrThread();
    void thResetErrThread();

    DWORD errorByThread;
    int realErrorReceive;
    int realErrorSend;
    int realTotCount;

    int Addr4IpRemote;

    BYTE keySend[DIM_KEY_SEND];
    LPCBYTE getKeySend() const { return keySend; }

    DWORD idThread;
    HANDLE hEventClose;
    HANDLE hEventSendChar;

    DWORD Buff_Thread[MAX_DWORD_PERIF];
    friend unsigned FAR PASCAL ReaderClientProc(void*);
    friend class makeTrueConn;

    PWin* Owner;
    infoServer iServer;
    infoDynamicIP_Req iDynIp;

    void makeDual(class setOfString& set);

    uint Step;
    orderedDual dualSet;
    bool getDualAddr(const addrToComm* pAddr, DWORD& laddr, DWORD offset);

    // dati ricevuti dal sVisor da inviare al server
    PVect<infocmd*> CmdSet;

    bool thReadConn(PConnBase* Conn);
    bool thWriteConn(PConnBase* Conn);
    void thSendAck(PConnBase* Conn);
    void thFillChanged(info_modified_data* packet);
    void thChangeConn(info_cmd_chgserver* packet, PConnBase* Conn);

    uint connState;
    void resetRemoteIp();
    void showRemoteIp(PConnBase* Conn);
    virtual int getStep(uint type) { return 1 == Step ? Step :  prfData::getNByte((prfData::tData)type); }

};
//---------------------------------------------------------
#endif
