//------------ commwn.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef COMMWN_H_
#define COMMWN_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------

#include "gestComm.h"
#include "defgPerif.h"
#include "pButton.h"
//----------------------------------------------------------------------------
class gestCommWindow : public PMainWin
{
  public:
    gestCommWindow(LPCTSTR title = 0, HINSTANCE hInst = 0);
    virtual ~gestCommWindow();

    virtual bool create();
    virtual void Paint(HDC hdc);
    void Destroy(HWND hwnd);

  protected:
    gestComm *gComm;
    gestComm::ComResult LastErr;
    UINT idTimer;
    PButton *Reset;
    bool Started;
    bool toggled;
    int MustClose;
    int countRec;
    int countSend;
    int countTot;

    uint repeatWaitOnInitError;
    int consecutiveError;

//    bool canResetOnMaxError;

    bool w_active;
    bool firstShow;

// nuovo flag per far riconoscere al sVisor di essere a 16kVar
#ifdef USE_NEW_DIM
  #define NEW_DIM 8
#else
  #define NEW_DIM 0
#endif
#define SEND_INFO_FLAGS ((LPARAM)(\
                         (forceDirtyOnReceive ? 1 : 0) | \
                         (canManageBit ? 2 : 0) | \
                         (getIdentity()->useMappedFile() ? 4 : 0) | \
                         NEW_DIM | \
                         (gComm->hasRequestVar() ? 0 : 16) | \
                         ((2 & canManageBit) ? 32 : 0) \
                         ))
    bool forceDirtyOnReceive;
    // per default è attivo, quindi gestisce il messaggio e per default legge prima
    // la word dalla periferica, attiva/disattiva il bit e poi rispedisce la word.
    // Se qualche periferica usa l'invio diretto del bit si può inibire il metodo
    // prepareSendBits() nella classe gestCommgPerif che esegue le azioni di cui sopra.
    // Se la periferica non usa affatto la gestione a bit si può disabilitare il flag
    // Aggiunta la possibilità di usare il sendbit a 16bit, quindi ora il valore diventa
    // col doppio significato, se vale 1 è il send bit norale, se vale 2 è a 16bit
    DWORD canManageBit;

    bool init_gComm();
    void toggle(UINT id);
    void CloseApp(int);
    void processCommEvent();

    // numero di cicli chiusi massimi che effettua se non ci sono errori,
    // né messaggi nella coda del thread, e ci sono azioni nella coda della classe
    virtual uint getMoreCycleCount() const { if(gComm) return gComm->getMoreCycleCount(); return 5; }

    // se un driver ha necessità diverse può variare la frequenza con cui viene
    // richiamata la routine di gestione

    // modifica del 29-11-2005. Legge il timer dal file .adr (tramite gComm). Se non presente usa 200msec
    virtual uint getTimerTick() const;
//    virtual uint getTimerTick() const { return 200; }

    // gestisce il ritorno di ogni routine di gestComm
    gestComm::ComResult action(gestComm::ComResult last, bool setCount = false);

    // funzione segnaposto da ridefinire per elaborare messaggi provenienti dal
    // supervisore
    virtual LRESULT processSVisorMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
      { return PMainWin::windowProc(hwnd, message, wParam, lParam); }

    // funzioni segnaposto per una eventuale esecuzione prima e dopo
    // il run (indipendentemente dall'init)
    virtual gestComm::ComResult beforeRun(){ return gestComm::noAction; }
    virtual gestComm::ComResult afterRun(gestComm::ComResult last){ return last; }

    // funzioni segnaposto per spedizione messaggio di changed_data
    // il messaggio viene inviato dopo la afterRun() se i dati sono cambiati
    // e se non ci sono stati errori
    virtual WORD makeHiWordTParam1() { return 0; }
    virtual DWORD makeTParam2() { return 0; }

    // da ridefinire se si usa la gestione ad eventi anziché col timer
    virtual bool useTimer() const { return true; }

    // da ridefinire se si usa sia la gestione ad eventi che col timer
    virtual bool useDualMode() const { return false; }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR getClassName() const;
    virtual void getWindowClass(WNDCLASS& wcl);

//    void EvTimer(UINT timerId);
};

#endif  // commwn_h sentry.
