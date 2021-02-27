//------- PDiagCheckaddr.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PDiagCheckaddr_H_
#define PDiagCheckaddr_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include "pModDialog.h"
#include "resource.h"
#include "macro_utils.h"
#include "svmObject.h"
#include "ptreeview.h"
#include "1.h"
#include "p_mappedfile.h"
//----------------------------------------------------------------------------
extern uint registeredMsg;
#define CM_INIT_COMM      101
#define CM_START_COMM     102
#define CM_END_COMM       103
#define CM_EXPAND         104
#define CM_MARSHALL_COMM  105
//----------------------------------------------------------------------------
struct varsFilter
{
  int prph;
  int addr;
  int type;
  int nbit;
  int offs;
  varsFilter(int prph = 0, int addr = 0, int type = 0, int nbit = 0, int offs = 0) : prph(prph), addr(addr), type(type), nbit(nbit), offs(offs) {}
};
//----------------------------------------------------------------------------
class PDiagCheckaddr : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDiagCheckaddr(PWin* parent, HWND remote, PVect<varsFilter>& vFlt, bool refresh, uint resId = IDD_DIALOG_CHECKADDR, HINSTANCE hinstance = 0);
    virtual ~PDiagCheckaddr();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void resize();
    virtual HIMAGELIST makeImageList();
    void fill();
    void fill_sub(LPNM_TREEVIEWW twn);
  private:
    PTreeView* Tree;
    HIMAGELIST ImageList;
    PVect<varsFilter>& vFlt;
    p_MappedFile* mp;
    bool refresh;
    void tv_getRoot();
    void tv_InsertItem();
    void tv_Expand();
    void tv_getChild();
    void tv_getItem();
    void tv_setItem();
    void tv_DeleteItem();
    void tv_GetParent();
    void tv_setError();
    HWND hRemote;
};
//----------------------------------------------------------------------------
#endif
