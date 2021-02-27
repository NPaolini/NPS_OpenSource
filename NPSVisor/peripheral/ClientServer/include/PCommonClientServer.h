//--------- PCommonClientServer.h ------------------------------------------------
//--------------------------------------------------------------------------------
#ifndef PCOMMONCLIENTSERVER_H_
#define PCOMMONCLIENTSERVER_H_
//--------------------------------------------------------------------------------
#include <winsock2.h>
//--------------------------------------------------------------------------------
#include "PCrt_lck.h"
#include "P_Vect.h"
//--------------------------------------------------------------------------------
#include "PCommonData.h"
//--------------------------------------------------------------------------------
#define BASE_PORT 3000
//--------------------------------------------------------------------------------
#define WM_DATA_RECEIVED      (WM_APP + 13)
#define WM_NEED_SEND_CHANGED  (WM_DATA_RECEIVED + 1)
#define WM_NEED_RECONNECT     (WM_NEED_SEND_CHANGED + 1)
//--------------------------------------------------------------------------------
#define SEND_TO_ALL ((DWORD)-1)
//--------------------------------------------------------------------------------
class baseData;
//--------------------------------------------------------------------------------
class dataContainer;
//--------------------------------------------------------------------------------
int hasByte(SOCKET sock);
int canSend(SOCKET sock);
WORD getPort(SOCKET sock);
SOCKET createServerSocket(WORD port, int type);
SOCKET createTalkSocket(WORD port, int type, DWORD addr);
//--------------------------------------------------------------------------------
#include "basequeue.h"
//--------------------------------------------------------------------------------
class dataContainer
{
  public:
    dataContainer() : firstFree(0) {}

    ~dataContainer();

    void flush();

    void add(baseData* data, uint id);
    bool remove(baseData* data);

    bool isPortInUse(WORD port) const;

    bool isPending(uint id) const;

    baseData* get(uint id) const;
    baseData* getByPos(uint pos) const;

    int getPos(baseData* data) const;
    int getPos(uint id) const;

    uint getId(baseData* data) const;
    uint getId(uint pos) const;

    void setId(uint id, uint pos);
    void setId(uint id, baseData* data);

    criticalSect& getCritSect() const { return CS; }

    uint getElem() const;

    enum threadStat {
        sUndef,     // stato inconsistente, quando non è possibile ricavarlo
        sOnRun,     // si sta avviando
        sRunning,   // è in esecuzione
        sOnWaiting, // si sta fermando
        sStopped,   // è fermo
        sWaiting,   // è in attesa
        sNeedWait,  // settato quando si richiede che il thread si fermi in attesa
        sNeedStop   // settato dall'oggetto quando deve terminare
        };

    threadStat getStateTalk(uint id) const;
    threadStat getStateListen(uint id) const;

    bool getState(threadStat& talk, threadStat& listen, uint id) const;

    void setStateTalk(uint id, threadStat st);
    void setStateListen(uint id, threadStat st);

    enum threadType { tTalk, tListen };
    void setState(threadType which, uint id, threadStat st);

    void pushReceived(pMsgBaseData data, uint id, bool atTop = false);
    pMsgBaseData popReceived(uint id);

    void pushToSend(pMsgBaseData data, uint id, bool atTop = false);
    pMsgBaseData popToSend(uint id);

    void toFree(pMsgBaseData data);
    pMsgBaseData getFree();

    struct stat {
      DWORD qSend;
      DWORD qRecv;
      DWORD qFree;
      stat(DWORD send = 0, DWORD recv = 0, DWORD free = 0) :
            qSend(send), qRecv(recv), qFree(free) {}
      };
    void getStat(stat& stats, uint id) const;

    void reset();

  private:
    static criticalSect CS;
    struct elem {
      baseData* base;

      // è formato da due word, la word alta indica la porta di ricezione
      // mentre la parte bassa è l'id vero e proprio.
      // durante la ricerca, nelle funzioni che lo utilizzano, viene confrontata
      // solo la parte bassa, mentre nel settaggio e nella lettura del valore
      // viene considerato l'intero dato
      uint id;

      // mantiene lo stato dei thread
      threadStat statThreadTalk;
      threadStat statThreadListen;

      // coda per messaggi da spedire
      msgBaseQueue qToSend;

      // coda per dati ricevuti, tramite socket, tra client e server
      msgBaseQueue qReceived;

      struct baseStat {
        DWORD qToSend;
        DWORD qReceived;
        baseStat(DWORD send = 0, DWORD recv = 0) :
              qToSend(send), qReceived(recv) {}
        } Stat;

      elem(baseData* base = 0, uint id = 0) : base(base), id(id),
          statThreadTalk(sWaiting),
          statThreadListen(sWaiting) {}
      };

    PVect<elem*> Data;
    pMsgBaseData firstFree;
    DWORD qFree;

    bool isFree(uint pos);
    void flushToFree(msgBaseQueue& queue);

};
//--------------------------------------------------------------------------------
#define TIMER_TICK 500
#define SECOND_BEFORE_DEAD 5
#define MAX_COUNT_DEAD ((UINT)(1000.0 / TIMER_TICK) * SECOND_BEFORE_DEAD)
//--------------------------------------------------------------------------------
#define INIT_STRING_CLIENT "INIT"
#define RECONNECT_STRING   "RECONNECT"
//--------------------------------------------------------------------------------
#define RESULT_NOT_FOUND   "NOT FOUND"
#define RESULT_CONFLICT    "CONFLICT"
#define RESULT_BAD_ID      "BAD ID"
//--------------------------------------------------------------------------------
class baseData
{
  public:
    baseData(HWND hwOwner, dataContainer& container);
    virtual ~baseData();

    bool isValid() const;

    void fillSimpleMsg(pMsgBaseData Buff, DWORD msg, WPARAM wp, LPARAM lp);
    int fillForSendFileToRemote(pMsgBaseData Buff, LPCSTR filename);

    void destroy();

    criticalSect& getCritSect() { return CS; }

    void waitListen(uint id);
    void waitTalk(uint id);

    void addConnection(SOCKET remoteSocket);

  protected:

    virtual void adjuctForSend(pMsgBaseData Buff) = 0;

    bool saveFile(const LPBYTE pData, int len, LPCSTR filename, int offs = 0);

//    virtual bool initThread() = 0;

    UINT msgRegisteredId;

    HWND hwOwner;


    criticalSect CS;

    // oggetti dedicati all'ascolto
    WORD listenPort;
    HANDLE hEventListen;
    SOCKET listenSocket;
    DWORD Received;
    friend unsigned FAR PASCAL listenProc(void*);

    // oggetti dedicati all'invio di dati
    WORD talkPort;
    HANDLE hEventTalk;
    SOCKET talkSocket;
    DWORD Sended;
    friend unsigned FAR PASCAL talkProc(void*);

    dataContainer& Container;

  private:
    bool verifyStat(dataContainer::threadType which, uint id, dataContainer::threadStat stat, HANDLE hEvent);
};
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
inline
baseData::baseData(HWND hwOwner, dataContainer& container) :
    msgRegisteredId(0), hwOwner(hwOwner),
    listenSocket(INVALID_SOCKET), talkSocket(INVALID_SOCKET), Container(container),
    listenPort(0), talkPort(0), Received(0), Sended(0),
    hEventTalk(0), hEventListen(0)

{
  Container.add(this, 0);
}
//--------------------------------------------------------------------------------
inline
bool baseData::isValid() const
{
  return toBool(msgRegisteredId);
}
//--------------------------------------------------------------------------------
// factory, deve essere definita dallo specifico client
extern const dataClientInfo& getInfo();
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
#endif