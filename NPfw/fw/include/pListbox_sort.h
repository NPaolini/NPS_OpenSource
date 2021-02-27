//-------------------- pListboxSort.h -----------------------
//-----------------------------------------------------------
#ifndef pListboxSort_H_
#define pListboxSort_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pListbox.h"
#include "p_freeList.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
//----------------------------------------------------------------------------
class lbComp
{
  public:
    lbComp(bool case_sens = false) : caseSens(case_sens) {}
    virtual ~lbComp() {}
    virtual int compare(COMPAREITEMSTRUCT* cis, uint dim) = 0;
  protected:
    int compare_(LPCTSTR p1, LPCTSTR p2, int dim);
    bool caseSens;
};
//----------------------------------------------------------------------------
class pListboxSort : public PListBox
{
  private:
    typedef PListBox baseClass;

  public:
    pListboxSort(PWin * parent, uint id, int x, int y, int w, int h, int len = 255, HINSTANCE hinst = 0);
    pListboxSort(PWin * parent, uint id, const PRect& r, int len = 255, HINSTANCE hinst = 0);
    pListboxSort (PWin* parent, uint id, int len = 255, HINSTANCE hinst = 0) :  baseClass(parent, id, len, hinst), pfl(0), Tot(0) {}
    pListboxSort(HWND hwnd, HINSTANCE hInst) : baseClass(hwnd, hInst), pfl(0), Tot(0) {}

    ~pListboxSort();
    LPTSTR getFree();
    void releaseFree(LPTSTR p);
    void SetTabStop(int ntabs, int *tabs, int *show = 0);

  protected:
    P_freeList* pfl;
    int Tot;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void freeCurrData(HWND hwnd, INT_PTR ix);
    bool retrieveBuff(DRAWITEMSTRUCT FAR& drawInfo);
};
//----------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------
#endif
