//-------------------- pCom_bs.H ---------------------------
#ifndef PCOM_BS_H_
#define PCOM_BS_H_
//-----------------------------------------------------------
#include "precompHeader.h"
//-----------------------------------------------------------
#ifndef PARAMCONN_H_
  #include "paramconn.h"
#endif
//------------------------------------------------------
// messaggio inviato dalla classe alla finestra per indicare
// che ci sono dati nel buffer
//WM_CUSTOM_MSG_CONNECTION
// WPARAM == numero di Byte
// LPARAM == idCom
//------------------------------------------------------
class PComBase
{
  public:
    PComBase(HWND w = 0) : idComDev(INVALID_HANDLE_VALUE), HWnd(w),
          needNotify(true) { }
    virtual ~PComBase() { }

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual DWORD write_string(const void *buff, DWORD len) = 0;
    virtual DWORD read_string(void *buff, DWORD len) = 0;
    virtual DWORD has_string() = 0;
    virtual void setBreak(DWORD ms) = 0;

    virtual void reset() {  }
    void setParam(paramConn pc) { pCom = pc; }
    paramConn getParam() const { return pCom; }
    bool reqNotify(bool set) { bool old = needNotify; needNotify = set; return old; }
    struct time_out { // time out per lettura
      time_out(DWORD _ms = 20, DWORD rep = 100) : ms(_ms), repCount(rep) {  }
      DWORD ms;       // tempo di attesa tra un tentativo e l'altro
      DWORD repCount; // numero di tentativi
      };
    void setTimeOut(const time_out& to) { timeOut = to; }
    const HANDLE getIdCom() const { return idComDev; }

    HWND setNewHWND(HWND hwnd) { HWND old = HWnd; HWnd = hwnd; return old; }
  protected:
    const HWND getHWin() { return HWnd; }
    virtual void notify(int msg, WPARAM wp, LPARAM lp = 0);
    void setIdCom(HANDLE newH) { idComDev = newH; }
    DWORD getTOms() { return timeOut.ms; }
    DWORD getTOcount() { return timeOut.repCount; }

  private:
    HWND volatile HWnd;
    paramConn pCom;
    HANDLE volatile idComDev;
    bool needNotify;
    time_out timeOut;
};
//------------------------------------------------------
inline
void PComBase::notify(int msg, WPARAM wp, LPARAM lp)
{
  if(HWnd && needNotify && IsWindow(HWnd))
    ::PostMessage(HWnd, msg, wp, lp);
}
//------------------------------------------------------
extern PComBase *allocCom(HWND w = 0);
//------------------------------------------------------
//------------------------------------------------
#endif
