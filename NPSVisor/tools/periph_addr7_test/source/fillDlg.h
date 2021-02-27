//----------- fillDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef FILLDLG_H_
#define FILLDLG_H_
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
//----------------------------------------------------------------------------
extern void fillCbDataType(HWND hcb, int sel);
extern int getNumAndSelCB(HWND hcb, int& select);
//----------------------------------------------------------------------------
class PD_Fill : public PDialog
{
  public:
    PD_Fill(baseRow cfg[], uint dimCfg, PWin* parent, uint resId = IDD_DIALOG_FILL, HINSTANCE hinstance = 0);
    virtual ~PD_Fill();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    baseRow* Cfg;
    uint dimCfg;
    bool save();
    void load();
    void enableDisable(HWND check, uint idCtrl);
};
//----------------------------------------------------------------------------
struct infoCopy
{
  DWORD sourceAddr;
  DWORD targetAddr;
  DWORD numData;
  DWORD numBlock;
  DWORD offsBlock;
  DWORD flag;
  DWORD dbOffs;
  DWORD addrOffs;
  DWORD sourcePrph;
  int currPrph;

  bool move;
  bool checkNoAction;

  enum { none, eDb = 1 << 0, eAddr = 1 << 1, ePrph = 1 << 2, eNoFirstOffset = 1 << 3, eOffsetProgr = 1 << 4 };

  infoCopy() : sourceAddr(0), targetAddr(0), numData(0), numBlock(0), offsBlock(0), flag(0),
      dbOffs(0), addrOffs(0), sourcePrph(0), currPrph(0), move(false), checkNoAction(true) {}
};
//----------------------------------------------------------------------------
class PD_Copy : public PDialog
{
  public:
    PD_Copy(infoCopy& icopy, PWin* parent, uint resId = IDD_DIALOG_COPY, HINSTANCE hinstance = 0);
    virtual ~PD_Copy();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    infoCopy& iCopy;
    bool save();
    void load();
    void enableDisable(HWND check, uint idCtrl, bool reverse = false);
};
//----------------------------------------------------------------------------
#endif
