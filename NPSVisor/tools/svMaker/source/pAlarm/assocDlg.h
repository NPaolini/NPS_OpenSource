//------------ assocDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef assocDlg_H_
#define assocDlg_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#include "baseDlg.h"
//----------------------------------------------------------------------------
#include "pAl_common.h"
//----------------------------------------------------------------------------
class PD_Assoc : public PD_Base
{
  private:
    typedef PD_Base baseClass;
  public:
    PD_Assoc(PWin* parent, uint idPrph, uint resId = IDD_CL_ASSOC, HINSTANCE hinstance = 0);
    virtual ~PD_Assoc();

    virtual bool create();

    baseRowAssoc* getRows() { return Cfg; }
    void saveCurrData();
    void loadData();

    bool saveData();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void Clear(int from, int to);
    void fill();
    void Copy();
    void searchText();
    bool isRowEmpty(uint pos);
    void openAssocFile();
  private:
    baseRowAssoc Cfg[MAX_ADDRESSES];
    PRow_Assoc* Rows[MAX_GROUP];

    void manageText(uint idcBtn);

};
//----------------------------------------------------------------------------
#endif
