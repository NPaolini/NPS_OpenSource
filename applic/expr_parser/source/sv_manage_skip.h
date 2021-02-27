//----------- sv_manage_skip.h -------------------------------------
#ifndef sv_manage_skip_H_
#define sv_manage_skip_H_
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_manage_express.h"
//------------------------------------------------------------------
class svManSkip
{
  public:
    svManSkip( manExpressionVars* manV) : manV(manV), onError(false), buffElab(0) {}
    ~svManSkip() { flush(); }

    LPTSTR run(LPTSTR buff, uint& len);

  private:
    struct infoCalc
    {
      int row;
      LPCTSTR skipLabel;
      infoCalc(int row = 0, LPCTSTR skipLabel = 0) : row(row), skipLabel(skipLabel) {}
    };
    typedef PVect<infoCalc> vInfoCalc;

    // righe di calcolo in cui non è stata sostituita la label
    vInfoCalc notFound;

    // righe di riferimento
    vInfoCalc Label;

    LPTSTR buffElab;

    PVect<LPCTSTR> Rows;

    void manageCalc(LPTSTR p);
    void replaceLabel(uint ix, const infoCalc& iLabel);
    bool manageLabel(LPTSTR p);
    bool manageLine(LPTSTR p);
    LPTSTR finish(uint& len);

    void flush();
    manExpressionVars* manV;
    bool onError;
};
//------------------------------------------------------------------
#endif
