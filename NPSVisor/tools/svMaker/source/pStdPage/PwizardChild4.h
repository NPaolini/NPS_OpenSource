//----------- pWizardChild4.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PWIZARDCHILD4_H_
#define PWIZARDCHILD4_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pWizardChild.h"
//----------------------------------------------------------------------------
struct infoRow;
//----------------------------------------------------------------------------
class pWizardChild4 : public pWizardChild
{
  private:
    typedef pWizardChild baseClass;
  public:
    pWizardChild4(PWin* parent, uint resId = IDD_WZ_RECIPE, HINSTANCE hinstance = 0);
    virtual ~pWizardChild4();

    virtual bool create();
    virtual bool save(setOfString& Set);
    virtual void setItem(int ix);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void getTitle(LPTSTR append, size_t lenBuff);

  private:
    void addRow(bool append);
    void remRow();
    void modRow();
    void moveRow(bool up);

    void unfillRow(LPTSTR row, infoRow& ir);

    bool savePrph(P_File* pf, LPTSTR buff, uint id, PVect<infoRow*>& vIr);
    bool saveAddr(P_File* pf, LPTSTR buff, uint id, PVect<infoRow*>& vIr);
    bool saveType(P_File* pf, LPTSTR buff, uint id, PVect<infoRow*>& vIr);
    bool saveDim(P_File* pf, LPTSTR buff, uint id, PVect<infoRow*>& vIr);

    void refreshAddr();

    bool performSave(setOfString& Set, P_File* pf, P_File* pfClear);

};
//----------------------------------------------------------------------------
#endif
