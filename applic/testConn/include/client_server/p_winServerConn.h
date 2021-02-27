//----------- p_winServerConn.h ----------------------------------------------
//----------------------------------------------------------------------------
#ifndef p_winServerConn_H_
#define p_winServerConn_H_

#include "precHeader.h"
#include "connClass.h"
#include "PCrt_lck.h"
//----------------------------------------------------------------------------
class p_winChildConn;
//----------------------------------------------------------------------------
class p_winServerConn : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    p_winServerConn(PWin* parent, uint id, uint port, HINSTANCE hInst = 0);
    p_winServerConn(PWin* parent, uint id, const PVect<WORD>& port, HINSTANCE hInst = 0);
    ~p_winServerConn();

    bool create();

    criticalSect& getCritSect() { return csMsg; }
    virtual uint getConnStatus();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // lista di client attivi
    PVect<p_winChildConn*> Clients;

    // lista di client in attesa di attivazione
    PVect<p_winChildConn*> Waiting;

    PVect<PConnBase*> serverConn;
    criticalSect csMsg;
    PVect<WORD> listenPort;

    virtual bool createConn();
    virtual p_winChildConn* allocChild(PWin* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst = 0) = 0;

    void addNewClient(HANDLE_CONN hc);
    void removeClient(p_winChildConn* client);
    void addClient(p_winChildConn* client);

    void removeAllClient();

    virtual LPCTSTR getClassName() const { return _T("NPwinServerConn"); }

};
//----------------------------------------------------------------------------
#endif
