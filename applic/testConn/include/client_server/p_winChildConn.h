//----------- p_winChildConn.h -----------------------------------------------
//----------------------------------------------------------------------------
#ifndef p_winChildConn_H_
#define p_winChildConn_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "connClass.h"
#include"p_util.h"
//----------------------------------------------------------------------------
#define WM__FIRST_FREE  (WM_APP  + 100)
//----------------------------------------------------------------------------
#define WM_FROM_SERVER  WM__FIRST_FREE
  // LOWORD(wParam)
  #define CM_MSG_FROM_SERVER 3
    // messaggio generico, HIWORD(wParam) e lParam dipendono dal messaggio,
    // viene inviato solo ai client attivi
//----------------------------------------------------------------------------
#define WM_FROM_CLIENT  (WM_FROM_SERVER + 1)
  // LOWORD(wParam)
  #define CM_OK_CLIENT      1
    // lParam = p_winChildConn*
  #define CM_REMOVE_CLIENT  2
    // lParam = p_winChildConn*

  #define CM_MSG_FROM_CLIENT 3
    // messaggio generico, HIWORD(wParam) e lParam dipendono dal messaggio

  #define CM_KILL_TIMER  4
    // è arrivato l'ack, occorre rimuovere il timer del timeout
//----------------------------------------------------------------------------
  // vale sia per il server che per il client
  #define CM_END_THREAD     20
//----------------------------------------------------------------------------
// la win parent deve derivare da p_winServerConn o comunque gestire il messaggio CM_REMOVE_CLIENT
// per distruggere l'oggetto
// i metodi che iniziano con 'th_' sono eseguiti nel thread 'ReaderClientProc'
//----------------------------------------------------------------------------
class p_winChildConn : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    p_winChildConn(PWin* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst = 0);
    ~p_winChildConn();

    bool create();

    PConnBase* getHConn() { return Conn; }
    void setEventSend() { SetEvent(hEventSendChar); }
    void deleteConn();
    void addConn(HANDLE_CONN hc);
    bool isOnLine();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HANDLE hEventHasChar;
    HANDLE hEventSendChar;
    HANDLE hEventCloseConn;

    PConnBase* Conn;
    uint idTimer;
    bool threadActive;

    void stopThread();
    virtual bool th_readConn() = 0;
    virtual bool th_writeConn() = 0;
    virtual LPCTSTR getClassName() const { return _T("NPwinClientConn"); }
// se il client non riceve dati entro questo tempo invia il messaggio CM_REMOVE_CLIENT al genitore
// la classe che eredita deve inviare, a se stessa, il messaggio CM_KILL_TIMER se riceve dati e può quindi continuare
    virtual uint getTimeoutAck() { return 10 * 1000; }

  private:
    void th_setThread() { threadActive = true; }
    void th_resetThread() { threadActive = false; }
    friend unsigned FAR PASCAL ReaderClientProc(void*);

};
//----------------------------------------------------------------------------
#endif
