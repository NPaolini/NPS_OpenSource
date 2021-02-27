//-------- sv_data.h -----------------------------------------------
//------------------------------------------------------------------
#ifndef sv_data_H_
#define sv_data_H_
//------------------------------------------------------------------
#include "precHeader.h"
#include "defgPerif.h"
#include "sv_baseServer.h"
#include "PCrt_lck.h"
#include "p_vect.h"
#include "p_util.h"
#include "P_FreePacketQueue.h"
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <pshpack2.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(push, 2)

#else
  #error Occorre definire l'allineamento a 2 byte per le strutture seguenti
#endif
//------------------------------------------------------------------
#define MAX_VARS MAX_DWORD_PERIF
#define SV_MAGIC 'S.PN'
#define DIM_KEY_SEND 8
//------------------------------------------------------------------
struct header
{
  enum { hNone, hPsw, hVarChg, hVarToWrite, hPingRenew, hDynamicIP_Request };
  DWORD Magic;
  WORD Code;
  WORD Len; // dimensione in byte di quello che segue la struttura
  WORD ChkSum;
  header() : Magic(SV_MAGIC), Code(0), Len(0), ChkSum(0) {}
  header(WORD code, WORD len = 0, WORD chksum = 0) : Magic(SV_MAGIC), Code(code), Len(len), ChkSum(chksum) {}

  // presuppone che la struttura faccia parte di un buffer allocato
  // comprendente anche i dati
  void makeChkSum() {  ChkSum = getChkSum(this);  }

  WORD getChkSum(const header* pH) { LPBYTE pb = (LPBYTE)pH; return getChkSum(pb + sizeof(*pH), pH->Len); }

  static WORD getChkSum(LPBYTE pb, int len);
};
//----------------------------------------------------------------------------
#define MAX_LEN_NAME  40
//----------------------------------------------------------------------------
#define DIM_HOSTNAME MAX_LEN_NAME
//----------------------------------------------------------------------------
struct infoDynamicIP_Renew
{
  BYTE code[MAX_PASSWORD];
  DWORD port;
  BYTE hostname[DIM_HOSTNAME];
  infoDynamicIP_Renew() : port(0) { code[0] = 0; hostname[0] = 0; }
};
//------------------------------------------------------------------
//struttura ping per invio ip dinamico
struct ping4dynamicIP
{
  header Head;
  infoDynamicIP_Renew Info;
  ping4dynamicIP() : Head(header::hPingRenew, sizeof(Info)) {}
};
//------------------------------------------------------------------
//struttura handshake iniziale
struct handshake
{
  header Head;
  DWORD  prph; // periferica per cui si richiedono i dati
  BYTE   key[DIM_KEY_SEND];  // chiave di cifratura per la password, usata anche per cifrare i dati
  char   password[MAX_PASSWORD]; // viene inviata cifrata con la key

  static void makePsw(LPSTR target, LPCSTR source, LPCBYTE key);

  bool comparePsw(LPCSTR clearPsw);
  void makePsw(LPCSTR clearPsw) { makePsw(password, clearPsw, key); }
};
//------------------------------------------------------------------
// struttura pacchetto dati in lettura (modificati)
struct infodata
{
  DWORD  addr;
  DWORD  numdata;
  DWORD  data[1]; // i successivi allocati in runtime
};
//------------------------------------------------------------------
// struttura pacchetto dati in scrittura
struct infocmd
{
  DWORD  addr;
  DWORD  typevar;
  DWORD  numdata;
  DWORD  data[1]; // i successivi allocati in runtime
};
//------------------------------------------------------------------
// la dimensione totale non può superare MAX_DIM_BLOCK
struct info_modified_data
{
  header Head;
  DWORD  numinfodata;
  // i successivi non sono consecutivi in memoria, ma si deve tener conto della dimensione reale di
  // ciascuna struttura
  infodata data[1];
};
//------------------------------------------------------------------
// la dimensione totale non può superare MAX_DIM_BLOCK
struct info_cmd_data
{
  header Head;
  DWORD  numinfodata;
  // i successivi non sono consecutivi in memoria, ma si deve tener conto della dimensione reale di
  // ciascuna struttura
  infocmd data[1];
};
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <poppack.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(pop)

#endif
//------------------------------------------------------------------
#define MAX_DIM_BLOCK     (4096 / 2)
#define MAX_ITEM        ((MAX_DIM_BLOCK - sizeof(info_cmd_data) * 2) / sizeof(DWORD))
#define MAX_BUFF_DATA     MAX_DIM_BLOCK
//------------------------------------------------------------------
class my_freeList_cs : public P_freeList
{
  private:
    typedef P_freeList baseClass;
  public:
    my_freeList_cs(size_t size) : baseClass(size) { }
    LPVOID getFree() { criticalLock crtLck(cs); return baseClass::getFree(); }
    void addToFree(LPVOID data) { criticalLock crtLck(cs);  baseClass::addToFree(data); }

    LPVOID get() { return getFree(); }
    void release(LPVOID data) { addToFree(data); }
  private:
    criticalSect cs;
};
//------------------------------------------------------------------
typedef my_freeList_cs FreePacket;
//------------------------------------------------------------------
extern FreePacket& getFreePacket();
//------------------------------------------------------------------
extern FreePacket& getFreeBuff();
//------------------------------------------------------------------
extern info_modified_data* getFreeData();
extern void releaseFreeData(info_modified_data* packet);
extern void setFreeItem(PVect<infodata*>& set);
extern void setFreeItem(PVect<infocmd*>& set);
//------------------------------------------------------------------
#define getFreeInfoCmdData (info_cmd_data*)getFreeData
#define releaseInfoCmdData(a)  releaseFreeData((info_modified_data*)(a))
//------------------------------------------------------------------
#define MAX_QUEUE 50
//------------------------------------------------------------------
class my_Queue_cs : public P_TQueueSimple<LPDWORD, MAX_QUEUE>
{
  private:
    typedef P_TQueueSimple<LPDWORD, MAX_QUEUE> baseClass;
  public:
    bool Push(LPDWORD d) { criticalLock crtLck(cs);  return baseClass::Push(d); }

    LPDWORD Pop() { criticalLock crtLck(cs); return baseClass::Pop(); }

    // ritorna il dato corrente senza rimuoverlo
    LPDWORD getCurr() { criticalLock crtLck(cs); return baseClass::getCurr(); }

    int getStored() { criticalLock crtLck(cs); return baseClass::getStored(); }
    int getFree() { criticalLock crtLck(cs); return baseClass::getFree(); }

    // copia (appende) tutta la coda in set, torna il numero di elementi aggiunti
    int copyTo(PVect<LPDWORD>& set) { criticalLock crtLck(cs); return baseClass::copyTo(set); }
  private:
    criticalSect cs;
};
//------------------------------------------------------------------
class manageReadingData
{
  public:
    manageReadingData();
    ~manageReadingData();

    // torna false se non ci sono dati modificati
    bool sendModifiedData(LPDWORD buff);

    void sendModifiedData(PConnBase* conn, LPCBYTE key);
  private:
    DWORD oldBuff[MAX_VARS];
    PVect<infodata*> Set;
    my_Queue_cs buffSet;
    int firstCmp;
};
//------------------------------------------------------------------
#endif
