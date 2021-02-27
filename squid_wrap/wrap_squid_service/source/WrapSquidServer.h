//------ WrapSquidServer.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef WrapSquidServer_h__
#define WrapSquidServer_h__
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_winChildConn.h"
#include "p_winServerConn.h"
#include "p_tqueue_simpleCS.h"
#include "queueLog.h"
//----------------------------------------------------------------------------
class myChild : public p_winChildConn
{
  private:
    typedef p_winChildConn baseClass;
  public:
    myChild(PWin* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst = 0);
    ~myChild();

    bool create();
  protected:
    virtual bool th_readConn();
    virtual bool th_writeConn();
    virtual LPCTSTR getClassName() const { return _T("WrapSquidServer_ChildConn"); }

  private:
    HANDLE hChildStd_IN_Rd;
    HANDLE hChildStd_IN_Wr;
    HANDLE hChildStd_OUT_Rd;
    HANDLE hChildStd_OUT_Wr;
    LPCSTR progr_type;
    bool closing;
    bool init_send_ok(LPSTR buff, DWORD len);
    bool perform_wrap(LPSTR buff, DWORD len);
    bool createProcessChild(LPTSTR filename);
    bool Initializated;
    bool Obfuscated;


    bool perform_write(LPSTR buff, DWORD len);
    bool perform_read(LPSTR buff, DWORD szBuff, DWORD& dwRead);
    void addInfoType(LPSTR buff, size_t dim);
};
//-------------------------------------------------------------------
class myServerConn : public p_winServerConn
{
  private:
    typedef p_winServerConn baseClass;
  public:
    myServerConn(PWin* parent, uint id, uint port, HINSTANCE hInst = 0);
    myServerConn(PWin* parent, uint id, const PVect<WORD>& port, HINSTANCE hInst = 0);
    ~myServerConn();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual p_winChildConn* allocChild(PWin* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst = 0)
      { return new myChild(parent, id, hc, hInst); }
    virtual LPCTSTR getClassName() const { return _T("WrapSquidServer_ServerConn"); }
    void log(pDataSend pds);
    P_File* pfLog;

};
//-------------------------------------------------------------------
#endif
