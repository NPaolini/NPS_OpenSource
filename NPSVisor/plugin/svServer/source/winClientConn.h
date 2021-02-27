//----------- winClientConn.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef winClientConn_H_
#define winClientConn_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "wm_custom.h"
#include "connClass.h"
#include "p_util.h"
#include "p_vect.h"
#include "paramconn.h"
#include "PCrt_lck.h"
#include "sv_data.h"
#include "sv_make_dll.h"
//----------------------------------------------------------------------------
class winServerConn;
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
    // lParam = winClientConn*
  #define CM_REMOVE_CLIENT  2
    // lParam = winClientConn*

  #define CM_MSG_FROM_CLIENT 3
    // messaggio generico, HIWORD(wParam) e lParam dipendono dal messaggio

  #define CM_KILL_TIMER  4
    // è arrivato l'ack, occorre rimuovere il timer del timeout
//----------------------------------------------------------------------------
  // vale sia per il server che per il client
  #define CM_END_THREAD     20
//----------------------------------------------------------------------------
class winClientConn : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    winClientConn(winServerConn* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst = 0);
    ~winClientConn();

    bool create();

    PConnBase* getHConn() { return Conn; }

    void setEventSend() { SetEvent(hEventSendChar); }

    bool sendModifiedData(LPDWORD buff);

    virtual DWORD getIdPrph() const { return idPrph; }
    virtual void writeCommand();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    DWORD  idPrph;
    criticalSect CS;
    HANDLE hEventHasChar;
    HANDLE hEventSendChar;
    HANDLE hEventCloseConn;

    PConnBase* Conn;

    bool threadActive;
    bool readOnly;

    void stopThread();

    void setThread() { threadActive = true; }
    void resetThread() { threadActive = false; }
    friend unsigned FAR PASCAL ReaderClientProc(void*);


    virtual bool readConn();
    virtual bool fillToWrite(info_cmd_data* packet);

    virtual bool writeConn();

    bool flushConn();
    bool flushConn(int size);

    manageReadingData mrData;
    PVect<infocmd*> CmdSet;

    BYTE keySend[DIM_KEY_SEND];
    LPCBYTE getKeySend() const { return keySend; }

    uint idTimer;
    virtual LPCTSTR getClassName() const { return _T("svPwinClientConn"); }
};
//----------------------------------------------------------------------------
#endif
