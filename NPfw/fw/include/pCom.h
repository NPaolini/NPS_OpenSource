//-------------------- pcom.h ---------------------------
#ifndef PCOM_H_
#define PCOM_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pcom_bs.h"
#include "PCrt_lck.h"
//------------------------------------------------------
#include "setPack.h"
//------------------------------------------------------
//#define ONE_BYTE
#ifdef ONE_BYTE
//-----------------------------------------------------------
// implementa un vettore circolare, scrive in testa e legge dalla coda
// la lettura/scrittura deve avvenire un byte alla volta
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

    // non dovrebbe essere necessario la tecnica difensiva, vengono
    // letti/scritti byte solo dopo aver controllato la disponibilità
//#define DIFENSIVE
  #ifdef DIFENSIVE
    // controllano validità
    bool putByte(BYTE b);
    bool  getByte(BYTE *b);
  #else
    void putByte(BYTE b);
    void getByte(BYTE *b);
  #endif
    void putBytes(LPBYTE b, int len);
    void getBytes(LPBYTE b, int len);
    void getBytesNoDestroy(LPBYTE b, int len);
    DWORD getDim() const { return Max; }
  private:
    BYTE *Buff;
    DWORD Max;
    DWORD Head;
    DWORD Tail;

    NO_COPY_COSTR_OPER(P_CList)
};
//-----------------------------------------------------------
#else
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
#endif
//------------------------------------------------
#define DIM_BUFF (4096 * 4)
//------------------------------------------------
class PCom : public PComBase
{
  public:
    PCom(HWND hwnd = 0, DWORD dimBuffer = DIM_BUFF);
    virtual ~PCom();

    bool open();
    void close();
    DWORD write_string(const void *buff, DWORD len);
    DWORD read_string(void *buff, DWORD len);
    void setBreak(DWORD ms);
    virtual DWORD has_string();
    virtual void reset();

    void resizeBuffer(DWORD dimBuffer);
    DWORD readStringNoRemove(LPBYTE buff, DWORD len);
  protected:
    unsigned idThread;
    OVERLAPPED oRead;
    OVERLAPPED oWrite;
    criticalSect cS;

    bool connected;
    bool onClosing;


    class P_CList *cL;

    void fillDCB(DCB &dcb) const;
    virtual DWORD readString(bool& onError);
    bool isConnected() { return connected && !onClosing; }
    void resetThread()  { idThread = 0; }


    HANDLE hEventCloseConn;
    friend unsigned FAR PASCAL CommProc(void*);

    NO_COPY_COSTR_OPER(PCom)
};
//------------------------------------------------
#include "restorePack.h"
//------------------------------------------------
#endif
