//--------- PServerDriver.h ------------------------------------------------------
//--------------------------------------------------------------------------------
#ifndef PSERVERDRIVER_H_
#define PSERVERDRIVER_H_
//--------------------------------------------------------------------------------
#include "PCommonClientServer.h"
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// pacchetto scambiato in COPYDATASTRUCT tra driver e server
struct dataForServer
{
  // parametri standard, come se fosse in contatto diretto
  dataMsg Msg;
  // identificativo per il thread. Serve per il distingo se si usano più pc remoti
  DWORD id;
};
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
class dataServer : public baseData
{
  public:
    dataServer(HWND hwOwner, dataContainer& container, SOCKET sock);
    ~dataServer();

    LRESULT acceptMsg(HWND hwnd, DWORD msg, WPARAM wp, LPARAM lp);

    void evPaint(PVect<char>& text);

    void sendEndingToClient(uint id);

    static uint getIncrementedCounter() { return ++Counter; }
    static uint getCounter() { return Counter; }
    static void setCounter(uint c) { Counter = c; }

    bool isConnected();
  protected:
    virtual bool initThread();

    virtual void adjuctForSend(pMsgBaseData Buff);

  private:

    int elab(pMsgBaseData pMsgData);
    void adjuctRange(pMsgBaseData Buff);
    int handleNeedReconnect(dataContainer::threadType type);
    int reconnectTalk();

    // nome della classe di window per trovare l'applicazione driver
    LPCSTR className;

    // nome del messaggio per RegisterWindowMessage() e suo ritorno
    LPCSTR msgName;

    // nome del file di comandi
    LPCSTR commandFileName;

    // nome del file di dati
    LPCSTR dataFileName;

    HANDLE hEvent;

    UINT remoteMsgId;
    UINT localSVisorMsgId;

    HWND hwDriver;

    BYTE fileBuff[MAX_BUFF_SOCK - OFFS_DATA_INIT];

    bool needSendChanged;

    DWORD remoteAddr;

    static DWORD Counter;
    DWORD Id;
};
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
inline bool dataServer::initThread()
{
  return true;
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
class mainServer
{
  public:
    mainServer(HWND hwndOwner);
    ~mainServer();

    bool windowMessage(HWND hwnd, DWORD msg, WPARAM wParam, LPARAM lParam);
    void evPaint(HDC hdc, PRect& rect);
    void sendEndingToClient();

    uint getElem() { return Container.getElem(); }

    void checkClosed();

  private:
    dataContainer Container;
    HWND hwndOwner;
    DWORD idThread;
    SOCKET Socket;

    void addConnection(SOCKET sock);
    friend unsigned FAR PASCAL mainProc(void*);
};
//--------------------------------------------------------------------------------
#endif


