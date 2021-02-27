//-------- printDataVar.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PRINTDATAVAR_H_
#define PRINTDATAVAR_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "printGeneric.h"
//----------------------------------------------------------------------------
#include "1.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
// N.B. copiati dall'editor
// Il file non può essere lo stesso del template di stampa
#define ID_FILE_DATA      995000
#define ID_FILE_DATA_VARS 995001
#define ID_FILE_PAGE_DIM  995002
#define ID_FILE_LINKED    995003
//----------------------------------------------------------------------------
// gli altri codici sono equivalenti a quelli in 'pPrintPage.h'
enum printVarOther
{
  pvSimpleVar = 51,
  pvBoxVar,
  pvBitmapVar,

  pvFormFeed = 100
};
//----------------------------------------------------------------------------
class printDataVar : public printGeneric
{
  private:
    typedef printGeneric baseClass;
  public:
    printDataVar(PWin* par, svPrinter* printer, LPCTSTR fileTemplate);
    ~printDataVar();

  protected:
    virtual bool beginDoc(bool showSetup);
    virtual int addRow();

    // deve tornare il numero di caratteri necessari, se maggiore di lenBuff
    // viene allocata maggior memoria e si riprova
    virtual int getVarText(LPTSTR buff, int lenBuff, int idCode, int idVar) = 0;
    LPCTSTR getBasePath() const { return basePath; }
  private:
    int currPos;
    int lenBuff;
    uint lenFile;
    LPTSTR buffFile;
    LPTSTR buffRow;
    bool printVar(int idCode, LPCTSTR p);
    bool printColorAndOther(int id, LPCTSTR p);
    LPCTSTR basePath;
    virtual bool checkBmp(LPCTSTR p);
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
