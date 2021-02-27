//------------------ pOpenedCom.h ---------------------------
#ifndef POPENEDCOM_H_
#define POPENEDCOM_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pcom.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class POpenedCom : public PCom
{
  private:
    typedef PCom baseClass;
  public:
    POpenedCom(HANDLE hCom, HWND hwnd = 0) : baseClass(hwnd) { setIdCom(hCom); }
    virtual ~POpenedCom() { close(); }

    bool open();
  private:
    // non deve essere richiamata esplicitamente, ma solo dal distruttore.
    // l'HANDLE non viene chiuso dalla classe, ma deve essere fatto dal
    // chiamante
//    void close();

};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
