//--------------- MainWin.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "commidt.h"
#include "gperifwn.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class mem_Window : public TgPerif_Window
{
  public:
    mem_Window(LPCTSTR title = 0, HINSTANCE hInst = 0) : TgPerif_Window(title, hInst)
    {
      commIdentity *cI = getIdentity();
      cI->setUseMappedFile(false);

    }

    // numero di cicli chiusi massimi che effettua se non ci sono errori,
    // né messaggi nella coda del thread, e ci sono azioni nella coda della classe
//    virtual uint getMoreCycleCount() const { return 0; } // default 5

    // se un driver ha necessità diverse può variare la frequenza con cui viene
    // richiamata la routine di gestione
//    virtual uint getTimerTick() const { return 300; }    // default 200
};
//----------------------------------------------------------------------------
gestCommWindow* commIdentity::getMainWindow(LPCTSTR title, HINSTANCE hInst)
{
  return new mem_Window(title, hInst);
}


