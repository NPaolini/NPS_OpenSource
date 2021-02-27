//---------- password.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PASSWORD_H_
#define PASSWORD_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
class PWin;
//--------------------------------------------------------------
#include "setPack.h"
//--------------------------------------------------------------
// livello usato quando necessita solo di una conferma
#define ONLY_CONFIRM 1000
//----------------------------------------------------------------------------
// tempo di abilitazione password in secondi di default
#define MAX_TIME_PSW (30 * 1000)
//----------------------------------------------------------------------------
class PassWord
{
  public:
    PassWord() : Changed(false) {}
    // ad ogni richiesta con successo viene riavviato il time d'uso
    int getPsW(int requested, PWin* w, bool retUsed = false);

    // riavvia il timer, riattivando il livello di accesso (riazzerato dal timer)
    // da richiamare all'uscita dall'accesso alla zona protetta
    // solo se la getPsw() ha avuto successo
    void restartTime(int lev);

    // cambia le password di levello 1 o 2, la password base è fissa.
    // Non può essere richiamata dal livello 1
    bool changePsw(int reqLev, PWin* w);

    bool isChanged() { bool ret = Changed; Changed = false; return ret; }

    void restorePsw(PWin* w);
  protected:
  private:
    static int level;
    static DWORD seconds;
    static bool inExec;
    static DWORD maxTimePsw;
    static uint prph;
    static uint addr;
    static uint maxLevelByPrph;
    static int levelByPrph;
    static int prevLevelByPrph;
    static bool waitForMessageResult;
    void verifyPsW(int requested, PWin* w);
    bool checkLevelFromPeriph(PWin* w);
    void verifyValidityTime(PWin* w, DWORD tickCount);
    static void calcPrph();

    void savePswToPrph(PWin* w, int newpsw);
    void setPsw(PWin* w);

    bool Changed;
};

#include "restorePack.h"

#endif


