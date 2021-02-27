//----------- imageList.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef imageList_H_
#define imageList_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include <stdlib.h>
#include "resource.h"
#include "pModDialog.h"
#include "pTreeview.h"
#include "p_Vect.h"
#include "allVars.h"
#include "pSplitter.h"
//----------------------------------------------------------------------------
class PD_imageList : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PD_imageList(PWin* parent, uint resId = IDD_TREE_LIST_IMG, HINSTANCE hinstance = 0);
    virtual ~PD_imageList();

    virtual bool create();
    void setManPage(managePages* mp);
    virtual bool preProcessMsg(MSG& msg);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    PSplitter* Split;
};
//----------------------------------------------------------------------------
#endif
