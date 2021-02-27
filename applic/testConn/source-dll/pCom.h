//-------------------- pcom.h -------------------------------
//-----------------------------------------------------------
#ifndef PCOM_H_
#define PCOM_H_
//-----------------------------------------------------------
#include "precompHeader.h"
//-----------------------------------------------------------
#include "pcom_bs.h"
#include "PCrt_lck.h"
//-----------------------------------------------------------
class P_CList
{
  public:
    explicit P_CList(DWORD n_el) : Buff(new BYTE[n_el]), Max(n_el),
        Head(0), Tail(0)    {   }

    ~P_CList()    {     delete []Buff;    }

    // numero di byte caricati
    DWORD stored() { int t = Head - Tail; return t < 0 ? Max + t : t; }
    // numero di byte liberi, uno in meno per evitare che
    // Head e Tail si sovrappongano
    DWORD free() { int t = Tail - Head; return t <= 0 ? Max + t - 1 : t - 1; }

    void reset() { Head = 0; Tail = 0; }

    void putBytes(LPBYTE b, int len);
    void getBytes(LPBYTE b, int len);
    void getBytesNoDestroy(LPBYTE b, int len);

    DWORD getDim() const { return Max; }
  private:
    P_CList();
    P_CList operator =(P_CList&);
    LPBYTE Buff;
    DWORD Max;
    DWORD Head;
    DWORD Tail;
};
//-----------------------------------------------------------
#define DIM_BUFF (512 * 256)
//-----------------------------------------------------------
class PCom : public PComBase
{
  public:
    PCom(HWND hwnd = 0, DWORD dimBuffer = DIM_BUFF);
    virtual ~PCom();

    bool open();
    virtual void close();
    DWORD write_string(const void *buff, DWORD len);
    DWORD read_string(void *buff, DWORD len);
    void setBreak(DWORD ms);
    virtual DWORD has_string();
    virtual void reset();

    void resizeBuffer(DWORD dimBuffer);

    virtual DWORD appendToBuffer(LPBYTE buff, DWORD len);
    DWORD readStringNoRemove(LPBYTE buff, DWORD len);
  protected:
    unsigned volatile idThread;
    OVERLAPPED oRead;
    OVERLAPPED oWrite;
    criticalSect cS;
//    criticalSect cSOneWay;

    bool volatile connected;
    bool onClosing;

    class P_CList *cL;

    void fillDCB(DCB &dcb) const;
    virtual DWORD readString(bool& onError);
    bool isConnected() { return connected; }
    void resetThread()  { idThread = 0; }

    HANDLE hEventCloseConn;

    friend unsigned FAR PASCAL CommProc(void*);

};
//-----------------------------------------------------------
#endif
