//--------------- commidt.h --------------------------------------------------
//----------------------------------------------------------------------------
#ifndef COMMIDT_H_
#define COMMIDT_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "defgPerif.h"
#include "hdrmsg.h"
#ifndef GESTCOMM_H_
  #include "gestComm.h"
#endif
#include "p_vect.h"
//----------------------------------------------------------------------------
class commIdentity
{
  public:
    commIdentity();
    virtual ~commIdentity() { }

    int setHWND(HWND h, DWORD id = 0);
    void removeId(DWORD id);
    void addId(DWORD id);

    void setComm(unsigned pcom);

    virtual bool isOkArg(unsigned ok);

    const HWND getHSuperVisor();
    unsigned getPCom();

    virtual void parseArg(LPCTSTR arg);

    virtual gestComm *getGestComm(class PWin* w = 0);

    // per default richiama, in genMainWin.cpp, la creazione di TgPerif_Window
    virtual class gestCommWindow* getMainWindow(
              LPCTSTR title = 0,
              HINSTANCE hInst = 0);

     // va richiamata in caso di elaborazioni pesanti
     // per non bloccare l'applicazione
    void flushQueueMsg();

    LRESULT sendMessage(HWND hwnd, DWORD registeredMsg, WPARAM wp, LPARAM lp, DWORD id, bool post);

    UINT get_WM() const;

    uint getNum() const;
    DWORD getData() const;
    LPCTSTR getClassName() const;

    bool useMappedFile() const;

    bool needHide() const;
    void setNeddHide(bool set);

    void forceMappedFile(bool use) { UseMappedFile = use; }
  protected:
    void addOk(unsigned ok);
    unsigned getOk();

  private:
    HWND HSVisor;
    // un solo server per tutti i svisor remoti
    HWND HWServer;
    // identificativi dei svisor remoti
    PVect<DWORD> idReq;

    // messaggio registrato dipendente dal numero della periferica
    UINT WM_PRF;

    // identificativo periferica per gestione file e messaggi
    uint numPerif;

    // valore generico inviato dal svisor
    DWORD genData;

    bool UseMappedFile;

    bool Hided;

    unsigned pCom;
    unsigned okArg;
    void sendToServer(HWND hwnd, DWORD Msg, WPARAM wp, LPARAM lp, DWORD id);
};
//----------------------------------------------------------------------------
//---- factory method --------------------------------------------------------
extern commIdentity *getIdentity();
//----------------------------------------------------------------------------
#define OK_ARG (1 | (1 << 1) | (1 << 2) | (1 << 3))
//----------------------------------------------------------------------------
inline void commIdentity::setComm(unsigned pcom) { pCom = pcom; }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline bool commIdentity::isOkArg(unsigned ok) { return OK_ARG == (ok | okArg); }
//----------------------------------------------------------------------------
inline const HWND commIdentity::getHSuperVisor() { return HSVisor; }
//----------------------------------------------------------------------------
inline unsigned commIdentity::getPCom() { return pCom; }
//----------------------------------------------------------------------------
inline void commIdentity::addOk(unsigned ok) { okArg |= ok; }
//----------------------------------------------------------------------------
inline unsigned commIdentity::getOk() { return okArg; }
//----------------------------------------------------------------------------
inline UINT commIdentity::get_WM() const { return WM_PRF; }
//----------------------------------------------------------------------------
inline uint commIdentity::getNum() const { return numPerif; }
//----------------------------------------------------------------------------
inline DWORD commIdentity::getData() const { return genData; }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline gestComm* commIdentity::getGestComm(class PWin* w)
{
  return allocGestComm(w, getNum());
}
//----------------------------------------------------------------------------
inline bool commIdentity::useMappedFile() const { return UseMappedFile; }
//----------------------------------------------------------------------------
inline bool commIdentity::needHide() const { return Hided; }
//----------------------------------------------------------------------------
inline void commIdentity::setNeddHide(bool set) { Hided = set; }
//----------------------------------------------------------------------------

#endif
