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
//----------------------------------------------------------------------------
class winServerConn;
struct serializeConn;
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

    virtual DWORD getIdPlc() const { return idPlc; }

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    DWORD  idPlc;
    HANDLE hEventHasChar;
    HANDLE hEventCloseConn;

    HANDLE hEventDaveConn;
    void getDaveConn(serializeConn& srzC);
    void releaseDaveConn(serializeConn& srzC);

    PConnBase* Conn;

    bool threadActive;

    void stopThread();

    void setThread() { threadActive = true; }
    void resetThread() { threadActive = false; }
    friend unsigned FAR PASCAL ReaderClientProc(void*);


    virtual bool readConn();

    bool fillReadToSend(dataSend* packet);
    bool Send(dataSend* packet);

    uint idTimer;
    virtual LPCTSTR getClassName() const { return _T("svPwinClientConnLibnodave"); }
};
//----------------------------------------------------------------------------
#endif
