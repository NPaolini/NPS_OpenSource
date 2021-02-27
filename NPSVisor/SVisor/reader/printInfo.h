//-------- printTree.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PRINTINFO_H_
#define PRINTINFO_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "printGeneric.h"
#include "d_info.h"
//----------------------------------------------------------------------------
//#include "1.h"
//----------------------------------------------------------------------------
class printTree : public printGeneric
{
  private:
    typedef printGeneric baseClass;

  public:
    printTree(PWin* par, svPrinter* printer, LPCTSTR filename);

  private:
    P_File pfRows;
    virtual bool beginDoc(bool showSetup);
    virtual bool makeHeader();
    virtual bool makeFooter();
    virtual int addRow();

    PVect<int> posX;
    PVect<int> align;
    int heightFont;
    int heightRow;
    bool firstRow;

    virtual LPCTSTR getTitle();

    int firstOper;
    FILETIME Tot;
    FILETIME Oper;
    void fillMain(LPTSTR buff, const row4print& rp);
    void fillOper(LPTSTR buff, const row4print& rp);
    void fillCause(LPTSTR buff, const row4print& rp);
    void drawLines(LPTSTR buff, int nLine = 1);

};
//----------------------------------------------------------------------------
printTree* allocPrintTree(PWin* par, svPrinter* printer, LPCTSTR filename);
//----------------------------------------------------------------------------
#endif
