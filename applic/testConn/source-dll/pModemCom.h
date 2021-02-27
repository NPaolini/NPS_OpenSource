//------------------ PModemCom.h ----------------------------
#ifndef PMODEMCOM_H_
#define PMODEMCOM_H_
//-----------------------------------------------------------
#include "precompHeader.h"
//-----------------------------------------------------------
#include "pcom.h"
//-----------------------------------------------------------
// modifiche al WM_CUSTOM_MSG_CONNECTION
// WPARAM == numero di Byte, se zero indica un cambiamento nella connessione
// LPARAM == idCom, se WPARAM == zero, zero indica disconnessione, uno indica connessione
//-----------------------------------------------------------
/*
ATA   -> diretto
ATO0  -> passa in linea
AT&C0 -> DCD sempre on
*/
//-----------------------------------------------------------
class PModemCom : public PCom
{
  private:
    typedef PCom baseClass;
  public:
    PModemCom(HWND hwnd = 0);
    virtual ~PModemCom();

//    bool open();
    bool isOnLine() const { return onLine; }
    virtual DWORD write_string(const void *buff, DWORD len);

    // se address == NULL attende la chiamata, altrimenti compone il numero
    virtual bool startModem(LPCTSTR address = 0, LPCTSTR initString = 0, int useTone = 1, int sleepTime = 200);
    void closeLine();
    void close();
  protected:
    virtual DWORD readString(bool& onError);
    virtual bool sendInitString(LPCTSTR initString, int sleepTime);
    void setOnLine(bool set) { onLine = set; }
    bool volatile onLine;
    bool needCheck;
    virtual void notify(int msg, WPARAM wp, LPARAM lp = 0);

  private:
    void checkConn(DWORD len);
//    void close();
    int volatile waitingClose;
    enum { errCheck, cNumber, cString };
    bool checkOK(DWORD check);
    DWORD lastTick;
    bool support_RLSD;
};
//-----------------------------------------------------------
#endif
