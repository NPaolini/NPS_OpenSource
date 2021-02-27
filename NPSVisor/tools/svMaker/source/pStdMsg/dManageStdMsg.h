//------- dManageStdMsg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef dManageStdMsg_H_
#define dManageStdMsg_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#ifndef PMODDIALOG_H_
#include "pModDialog.h"
#endif

#ifndef PEDIT_H_
#include "pEdit.h"
#endif

#ifndef PSTATIC_H_
#include "pStatic.h"
#endif

#include "resource.h"
#include "p_Txt.h"
#include "svmMainClient.h"
#include "macro_utils.h"
#include "p_Vect.h"
#include "language_util.h"

#include "1.h"
//----------------------------------------------------------------------------
#define MIN_INIT_STD_MSG_TEXT 800000
//----------------------------------------------------------------------------
DWORD getGlobalStdMsgText();
//----------------------------------------------------------------------------
struct infoLabelValue
{
  int id;
  LPCTSTR label;

  int idCtrlLabel;
  int idCtrlSet;
};
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
#define CYAN_COLOR  RGB(0xcf,0xcf,0xff)
#define bkgColor2 CYAN_COLOR
//----------------------------------------------------------------------------
#define LABEL_COLOR  RGB(218,236,215)
#define bkgColor3 LABEL_COLOR
//----------------------------------------------------------------------------
class basePage : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    basePage(const setOfString& set, PWin* parent, uint id, HINSTANCE hInst = 0);
    virtual ~basePage();
    virtual bool save(P_File& /*pfCrypt*/, P_File& /*pfClear*/) { return true; }
    // deve tornare 0 | IDYES | IDNO | IDCANCEL, con zero continua il ciclo, con gli altri valori
    // ferma il ciclo con l'azione corrispondente
    virtual uint needSaveBeforeClose() { return 0; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType) { return 0; }

    const setOfString& Set;
    HBRUSH Brush;
    HBRUSH Brush2;
    HBRUSH Brush3;
    uint Dirty;
    virtual uint reqSave();
};
//----------------------------------------------------------------------------
#define MAX_TAB_PAGES 10
//----------------------------------------------------------------------------
class dManageStdMsg : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    dManageStdMsg(PWin* parent, uint resId = IDD_STD_MSG, HINSTANCE hinstance = 0);
    virtual ~dManageStdMsg();

    virtual bool create();
    virtual void Cm_Ok();
    void changedResolution() { changedRes = true; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk() {}
    virtual void CmCancel();

  private:
    TCHAR Path[_MAX_PATH];

    basePage* Page[MAX_TAB_PAGES];
    setOfString Set;
    int currPage;
    int maxPage;
    HIMAGELIST ImageList;

//    void setTitle();

    void chgPage(int page);
    class PBitmap* Bmp[2];

    HBRUSH bkgBrush;

    bool changedRes;

};
//----------------------------------------------------------------------------
bool getUseExtKeyb();
//----------------------------------------------------------------------------
#endif
