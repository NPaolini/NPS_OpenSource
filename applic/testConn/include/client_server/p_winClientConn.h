//----------- p_winClientConn.h ----------------------------------------------
//----------------------------------------------------------------------------
#ifndef p_winClientConn_H_
#define p_winClientConn_H_

#include "precHeader.h"
#include "connClass.h"
//----------------------------------------------------------------------------
class p_winChildConn;
//----------------------------------------------------------------------------
class p_winClientConn : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    p_winClientConn(PWin* parent, uint id, uint port, LPCTSTR ip, HINSTANCE hInst = 0);
    p_winClientConn(PWin* parent, uint id, uint port, DWORD ip, HINSTANCE hInst = 0);
    ~p_winClientConn();

    bool create();
    virtual bool createChild(bool startConn = true);
    virtual bool deleteChild();
    virtual bool createConn();
    virtual bool deleteConn();

    virtual bool hasClient();
    virtual bool isOnLine();
    virtual void changeIP(DWORD newIP);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    p_winChildConn* Child;
    LPTSTR IP;
    uint Port;
    virtual p_winChildConn* allocChild(PWin* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst = 0) = 0;

    virtual LPCTSTR getClassName() const { return _T("NPwinMainClientConn"); }

};
//----------------------------------------------------------------------------
#endif
