//---------- connClass.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef connClass_H_
#define connClass_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "wm_custom.h"
#include "conn_dll.h"
#include "paramconn.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PConnBase
{
  public:
    // connessione già avviata
    PConnBase(HANDLE_CONN client);

    // server lan
    PConnBase(DWORD port, HWND hwnd, bool udp);

    // client lan
    PConnBase(DWORD addr, DWORD port, HWND hwnd, bool udp);
    PConnBase(LPCTSTR addrOrName, DWORD port, HWND hwnd, bool udp);

    // modem
    PConnBase(HWND hwnd, paramConn pc, LPCTSTR number, LPCTSTR init, DWORD modemType);

    // seriale diretta
    PConnBase(paramConn pc, HWND hwnd, bool on485 = false);
    ~PConnBase();

    bool open();
    void close();
    void closeAndDestroy();

    bool isConnected();

    DWORD has_string();
    DWORD read_string(LPBYTE buff, DWORD len);
    DWORD read_stringNoRemove(LPBYTE buff, DWORD len);
    DWORD write_string(const BYTE* buff, DWORD len);
    bool reqNotify(bool req);

    void removeClient(HANDLE_CONN conn);

    bool iAm(LPARAM lParam);
    void setCallBack(pInfoCallBack pICB);

    LRESULT passThrough(uint codeMsg, LPDWORD p1);

    // chiude la connessione corrente e la riapre sui nuovi ip e port passati, non verifica che sia valida
    // o che i parametri siano diversi dai precedenti
    bool chgLanConn(DWORD addr, DWORD port);
    bool chgLanConn(LPCTSTR addrOrName, DWORD port);

    const ConnInfo& getInfo() { return CI; }

    HANDLE_CONN replaceHandle(HANDLE_CONN r) { HANDLE_CONN t = Conn; Conn = r; return t; }
    HANDLE_CONN detach() { return replaceHandle(0); }

    HWND changeOwner(HWND newOwner);
  private:
    HANDLE_CONN Conn;
    ConnInfo CI;

    long delayFor485;
    NO_COPY_COSTR_OPER(PConnBase)

};
//----------------------------------------------------------------------------
class customServerLanData
{
  public:
    customServerLanData(HWND owner, bool udp) : Owner(owner), Conn(0), Udp(udp) {}
    ~customServerLanData();

    bool run(DWORD port);
    void remove(HANDLE_CONN client);

  private:
    HWND Owner;
    PConnBase* Conn;
    bool Udp;

    // è solo come esempio, in pratica fa quello che farebbe il server in mancanza
    // della callback
    friend LRESULT callBackConn(uint codeMsg, LPVOID msg, LPVOID custom);

    NO_COPY_COSTR_OPER(customServerLanData)
};
//----------------------------------------------------------------------------
#endif
