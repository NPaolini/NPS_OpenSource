//----------- mainDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MAINDLG_H_
#define MAINDLG_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include <stdlib.h>
#include "resource.h"

#ifndef PMODDIALOG_H_
#include "pModDialog.h"
#endif
#include "p_Vect.h"
#include "adrDlg.h"
#include "testDlg.h"
//----------------------------------------------------------------------------
class PD_Address : public PModDialog
{
  public:
    PD_Address(PWin* parent, uint resId = IDD_CLIENT, HINSTANCE hinstance = 0);
    virtual ~PD_Address();

    virtual bool create();
    baseRow* getRows() { return clientAdr[currPage]->getRows(); }

    HWND getTestHandle();
    HWND getAdrHandle();

    HWND getSvHandle() { return SvHandle; }
    void setSvHandle(HWND svh, LPCTSTR name);
    LPCTSTR getSvName() { return SvName; }

    bool isDirty();
    void fillBit(DWORD& flag);
    baseRow* getRows(uint id);
    void setDirty(uint prph, bool reload = true);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    PVect<PD_Adr*> clientAdr;
    PVect<PD_Test*> clientsvTest;
    HWND SvHandle;
    TCHAR SvName[_MAX_PATH];
    int currPage;

    uint idTimer;
    void checkExternalChange();

    HIMAGELIST ImageList;
    void chgPage(int page);
    void chgAdrPage(int page);
    void chgTestPage(int page);

    enum which { jobbingAdr, jobbingSvTest };
    which onJob;
    void toggleVis();
    bool saveOnExit();

    void remTab();
    void addTab();

    void moveChild(PWin* child);

//    void loadByExist(PVect<uint>& ids);
    void makeClient();
    void performAddTab(uint cid, uint pos);

    bool saveDirtyPage(bool req);

};
//----------------------------------------------------------------------------
#endif
