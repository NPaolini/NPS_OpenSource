//-------------------- pCom_bs.H ---------------------------
#ifndef PCOM_BS_H_
#define PCOM_BS_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PARAMCOM_H_
  #include "paramcom.h"
#endif
//------------------------------------------------------
#include "setPack.h"
//------------------------------------------------------
// messaggio inviato dalla classe alla finestra per indicare
// che ci sono dati nel buffer
#define ID_MSG_HAS_CHAR (WM_USER + 200)
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
    void setParam(paramCom pc) { pCom = pc; }
    paramCom getParam() const { return pCom; }
    bool reqNotify(bool set) { bool old = needNotify; needNotify = set; return old; }
    struct time_out { // time out per lettura
      time_out(DWORD _ms = 20, DWORD rep = 100) : ms(_ms), repCount(rep) {  }
      DWORD ms;       // tempo di attesa tra un tentativo e l'altro
      DWORD repCount; // numero di tentativi
      };
    void setTimeOut(const time_out& to) { timeOut = to; }
    const HANDLE getIdCom() const { return idComDev; }

    HWND setNewHWND(HWND hwnd) { HWND old = HWnd; HWnd = hwnd; return old; }

    virtual DWORD readStringNoRemove(LPBYTE buff, DWORD len) = 0;

  protected:
    const HWND getHWin() { return HWnd; }
    void notify(int msg, WPARAM wp, LPARAM lp = 0);
    void setIdCom(HANDLE newH) { idComDev = newH; }
    DWORD getTOms() { return timeOut.ms; }
    DWORD getTOcount() { return timeOut.repCount; }

  private:
    HWND HWnd;
    paramCom pCom;
    HANDLE idComDev;
    bool needNotify;
    time_out timeOut;
};
//------------------------------------------------------
inline
void PComBase::notify(int msg, WPARAM wp, LPARAM lp)
{
  if(HWnd && needNotify)
    ::PostMessage(HWnd, msg, wp, lp);
}
//------------------------------------------------------
extern PComBase *allocCom(HWND w = 0);
//------------------------------------------------------
#include "restorePack.h"
//------------------------------------------------
#endif
