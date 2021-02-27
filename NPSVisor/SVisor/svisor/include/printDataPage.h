//-------- printDataPage.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PRINTDATAPAGE_H_
#define PRINTDATAPAGE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "printDataVar.h"
//----------------------------------------------------------------------------
#include "1.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class infoPrintDataPage
{
  public:

    infoPrintDataPage(LPCTSTR pageSet, int idPrf, int idAddr, int idType,
                    int idnDec, int idNorm) :
                    pageSet(str_newdup(pageSet)), idPrf(idPrf), idAddr(idAddr), idType(idType),
                    idnDec(idnDec), idNorm(idNorm) {}

    infoPrintDataPage(const infoPrintDataPage& other) : pageSet(0) { clone(other); }

    ~infoPrintDataPage() { delete []pageSet; }

    const infoPrintDataPage& operator =(const infoPrintDataPage& other) { return clone(other); }

  private:
    LPCTSTR pageSet;
    int idPrf;
    int idAddr;
    int idType;
    int idnDec; // lunghezza se type == testo
    int idNorm;
    const infoPrintDataPage& clone(const infoPrintDataPage& other);

    friend class printDataPage;
};
//----------------------------------------------------------------------------
class printDataPage : public printDataVar
{
  private:
    typedef printDataVar baseClass;
  public:
    printDataPage(PWin* par, svPrinter* printer, LPCTSTR fileTemplate, const infoPrintDataPage& ipdp);
    ~printDataPage();

  protected:

    // deve tornare il numero di caratteri necessari, se maggiore di lenBuff
    // viene allocata maggior memoria e si riprova
    virtual int getVarText(LPTSTR buff, int lenBuff, int idCode, int idVar);
  private:
    setOfString Set;
    infoPrintDataPage ipdp;
    int manageBitText(LPTSTR buffRow, int lenBuff, int idNorm, DWDATA val, int offset);
    int manageWordText(LPTSTR buffRow, int lenBuff, int idNorm, DWDATA val, int offset);
    int copyToResult(LPTSTR buffRow, int lenBuff, LPCTSTR p);
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
