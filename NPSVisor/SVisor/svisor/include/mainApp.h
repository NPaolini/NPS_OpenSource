//------ mainApp.h -----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MAIN_APP__
#define MAIN_APP__
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "mainClient.h"
#include "systray.h"
//----------------------------------------------------------------------------
#include "setPack.h"
// funzioni specifiche del supervisore da implementare nel target finale

// deve tornare il nome della classe
LPCTSTR getClassName();
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#ifdef _WIN64
  #define NAME_SVBASE _T("npsvBase64.dll")
#else
  #define NAME_SVBASE _T("npsvBase.dll")
#endif
//----------------------------------------------------------------------------
class mainWin : public PMainWin
{
  public:
    mainWin(HINSTANCE hInstance);
    ~mainWin();

    virtual bool create();

    virtual bool preProcessMsg(MSG& msg) { return toBool(IsDialogMessage(getHandle(), &msg)); }

  protected:
    mainClient *client;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LPCTSTR getClassName() const { return ::getClassName(); }
    UINT idTimer;
    UINT idTimerTray;
    void killTimer();

    friend class sysTray;
    sysTray* sysTrayObj;

    manTipsObj* manObjSysTray;
};
//----------------------------------------------------------------------------
// avvia gli applicativi esterni per la comunicazione diretta con le
// periferiche ed instanzia gli oggetti interni per la comunicazione che
// passa poi alla finestra di dialogo principale
void runPerif(class PWin* mainClient);
//-------------------------------------------------------------------
// se attiva (per default) fa avviare una sola instanza dell'applicazione,
// per permettere più applicazioni attive contemporaneamente usare il
// seguente codice
// extern bool ONLY_ONE = false;
// in zona globale

extern bool ONLY_ONE;
//-------------------------------------------------------------------
#include "restorePack.h"
#endif
