//----------- winServerConn.h --------------------------------------------------
//----------------------------------------------------------------------------
#ifndef winServerConn_H_
#define winServerConn_H_

#include "precHeader.h"
#include "winClientConn.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class winServerConn : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    winServerConn(PWin* parent, uint id, LPCTSTR name = 0, HINSTANCE hInst = 0);
    ~winServerConn();

    bool create();

    criticalSect& getCritSect() { return csMsg; }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // lista di client attivi
    PVect<winClientConn*> Clients;

    // lista di client in attesa di attivazione
    PVect<winClientConn*> Waiting;

    PConnBase* serverConn;
    criticalSect csMsg;

    virtual bool createConn() = 0;

    void addNewClient(HANDLE_CONN hc);
    void removeClient(winClientConn* client);
    void addClient(winClientConn* client);

    void removeAllClient();

    virtual LPCTSTR getClassName() const { return _T("svPwinServerConn"); }

};
//----------------------------------------------------------------------------
#endif
