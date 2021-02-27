//----------- allVarNorm.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef allVarNorm_H_
#define allVarNorm_H_
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
//----------------------------------------------------------------------------
class PD_AllVarNorm : public PD_TreeView
{
  private:
    typedef PD_TreeView baseClass;
  public:
    PD_AllVarNorm(PWin* parent, uint resId = IDD_TREE, HINSTANCE hinstance = 0);
    virtual ~PD_AllVarNorm();

    void setManPage(managePages* mp) { pMP = mp; fill(); }
  protected:
    virtual void fill();
    virtual void fill_sub(LPNM_TREEVIEWW twn);

    void fillSub1(HTREEITEM parent);
    void fillSub2(HTREEITEM parent);
    void fillSub3(HTREEITEM parent);

    virtual HIMAGELIST makeImageList();
  private:
    managePages* pMP;
};
//----------------------------------------------------------------------------
#endif
