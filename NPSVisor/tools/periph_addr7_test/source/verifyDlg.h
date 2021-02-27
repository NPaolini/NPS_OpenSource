//----------- verifyDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef VERIFYDLG_H_
#define VERIFYDLG_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#ifndef PDIALOG_H_
#include "pDialog.h"
#endif
//----------------------------------------------------------------------------
#include "mainDlg.h"
#include "p_avl.h"
#include "headerMsg.h"
//----------------------------------------------------------------------------
#define WM_ENABLE_VERIFY (WM_FW_FIRST_FREE + 1)
//----------------------------------------------------------------------------
#define CHECK_MIN_SUP(a) if(a > other.a) return true; if(a < other.a) return false
//----------------------------------------------------------------------------
class addrConv : public genericSet
{
  public:
    addrConv(uint ipAddr = 0, uint port = 0, uint db = 0, uint pAddr = 0, uint lAddr = 0, uint type = 0, uint action = 0) :
          ipAddr(ipAddr), port(port), db(db), pAddr(pAddr), lAddr(lAddr), type(type), action(action) {}
    uint ipAddr;
    uint port;
    uint db;
    uint pAddr;
    uint lAddr;
    uint type;
    uint action;
    bool operator >(const addrConv& other) {
      CHECK_MIN_SUP(ipAddr);
      CHECK_MIN_SUP(port);
      CHECK_MIN_SUP(db);
      CHECK_MIN_SUP(pAddr);
      return false;
//      return (ipAddr > other.ipAddr) || (db > other.db) || ((db == other.db) && (pAddr > other.pAddr));
      }
};
//----------------------------------------------------------------------------
class orderedPAddr : public P_Avl
{
  public:
    orderedPAddr() : P_Avl(true) { }
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
};
//----------------------------------------------------------------------------
class PD_Verify : public PModDialog
{
  public:
    PD_Verify(orderedPAddr& set, int type_sz, int type_show, bool step1,  PWin* parent, uint resId = IDD_DIALOG_VERIFY, HINSTANCE hinstance = 0);
    virtual ~PD_Verify();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    orderedPAddr& Set;
    int TypeSz;
    int typeShow;
    bool Step1;

    PRect fullRect;

    PWin* Par;
    int getTypeSize() const { return TypeSz; }
    int addRow(HWND  hwnd, addrConv* p);
    void load();
    PVect<int> Db;
    typedef PModDialog baseClass;
};
//----------------------------------------------------------------------------
#endif
