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
#include "pModDialog.h"
#include "pSplitter.h"
#include "pTreeview.h"
#include "pListview.h"
#include "p_Vect.h"
#include "checkAddr.h"
#include "allPages.h"
#include "allVars.h"
#include "allVarNorm.h"
#include "warnObj.h"
#include "pageList.h"
#include "imageList.h"
//----------------------------------------------------------------------------
class PDShowVars : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDShowVars(PWin* parent, uint resId = IDD_SHOW, HINSTANCE hinstance = 0);
    virtual ~PDShowVars();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    managePages* MP;
    PD_AllVars* clientVars;
    PD_AllPages* clientPages;
    PD_AllVarNorm* clientNorm;
    PD_WarnObj* clientWarn;
    PD_pageList* clientUsedPage;
    PD_imageList* clientUsedImage;
    HIMAGELIST ImageList;

    PD_AllVarsBysvMaker* forSvMaker;
    LRESULT initObjComm(HWND htree);
    void destroyObjComm();
    void expandObjComm(HTREEITEM htree);

    enum which {
      jobbingVars, jobbingPages, jobbingNorm,
      jobbingPageList, jobbingImageList, jobbingWarn,
      maxWhich,

      newPrj = maxWhich,
      refresh
      };
    which onJob;
    void chgPage(int page);
//    void toggleVis();

    void moveChild(PWin* child);
    void performAddTab(uint cid, uint pos);

    void resize();

    void newCheck();
    void refreshCheck();

    void setHotKey();
    void unsetHotKey();
};
//----------------------------------------------------------------------------
#endif
