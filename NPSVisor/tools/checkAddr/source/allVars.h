//----------- allVars.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef allVars_H_
#define allVars_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include <stdlib.h>
#include "resource.h"
#include "pModDialog.h"
#include "pTreeview.h"
#include "p_Vect.h"
#include "checkAddr.h"
#include "p_mappedfile.h"
#include "common.h"
//----------------------------------------------------------------------------
extern  PWin* getMain();
//----------------------------------------------------------------------------
#define CX_IMAGE_ADDR 32
#define CY_IMAGE_ADDR 24
//----------------------------------------------------------------------------
#define CX_IMAGE_PAGE 32
#define CY_IMAGE_PAGE 24
//----------------------------------------------------------------------------
class PD_TreeView : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PD_TreeView(PWin* parent, uint resId = IDD_TREE, HINSTANCE hinstance = 0);
    virtual ~PD_TreeView();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual void fill() {}
    virtual void fill_sub(LPNM_TREEVIEWW twn) {}
    virtual void endExpand(LPNM_TREEVIEWW twn) {}
    virtual void changedSel(LPNM_TREEVIEWW twn) {}
//    virtual bool onExpanding(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result) { return false; }
    HTREEITEM add_Child(LPTSTR Buff, const infoPrph* ipp, HTREEITEM parent, HTREEITEM child, uint ixImage);

    virtual void resize();
    virtual HIMAGELIST makeImageList() = 0;
    PTreeView* getTree() { return Tree; }
    virtual HWND getHTree() { return *Tree; }

    virtual void my_TreeView_DeleteAllItems() { TreeView_DeleteAllItems(getHTree()); }
    virtual HTREEITEM my_TreeView_GetRoot() { return TreeView_GetRoot(getHTree()); }
    virtual HTREEITEM my_TreeView_InsertItem(TV_INSERTSTRUCT* is) { return TreeView_InsertItem(getHTree(), is); }
    virtual void my_TreeView_Expand(HTREEITEM parent, UINT flag) { TreeView_Expand(getHTree(), parent, flag); }
    virtual HTREEITEM my_TreeView_GetChild(HTREEITEM parent) { return TreeView_GetChild(getHTree(), parent); }
    virtual BOOL my_TreeView_SetItem(TV_ITEM* item)  { return TreeView_SetItem(getHTree(), item); }
    virtual BOOL my_TreeView_GetItem(TV_ITEM* item)  { return TreeView_GetItem(getHTree(), item); }
    virtual BOOL my_TreeView_DeleteItem(HTREEITEM item)  { return TreeView_DeleteItem(getHTree(), item); }
    virtual HTREEITEM my_TreeView_GetParent(HTREEITEM item)  { return TreeView_GetParent(getHTree(), item); }
    virtual void my_refresh() {}
  private:
    PTreeView* Tree;
    HIMAGELIST ImageList;
};
//----------------------------------------------------------------------------
class PD_AllVars : public PD_TreeView
{
  private:
    typedef PD_TreeView baseClass;
  public:
    PD_AllVars(PWin* parent, uint resId = IDD_TREE, HINSTANCE hinstance = 0);
    virtual ~PD_AllVars();

    void setManPage(managePages* mp) { pMP = mp; fill(); }
  protected:
    virtual void fill();
    virtual void fill_sub(LPNM_TREEVIEWW twn);
    virtual void endExpand(LPNM_TREEVIEWW twn);
//    virtual bool onExpanding(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result);

    void fillSub1(HTREEITEM parent);
    void fillSub2(HTREEITEM parent);
    void fillSub3(HTREEITEM parent);

    virtual HIMAGELIST makeImageList();

//  private:
    managePages* pMP;
    virtual bool isValidPrph(uint prph) { return true; }
    virtual bool isValidAddr(uint prph, uint addr, uint type, uint nbit, uint offs) { return true; }
};
//----------------------------------------------------------------------------
extern uint registeredMsg;
#define CM_INIT_COMM      101
#define CM_START_COMM     102
#define CM_END_COMM       103
#define CM_EXPAND         104
#define CM_MARSHALL_COMM  105
//----------------------------------------------------------------------------
#define DEF_FILE_FOR_SVMAKER _T("npsvMaker4checkAddr_fm")
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
// non viene creata la window, ma solo l'oggetto che poi risponderà alle richieste
// del svMaker.
// N.B. non va chiamata né la modal(), né la create()

class PD_AllVarsBysvMaker : public PD_AllVars
{
  private:
    typedef PD_AllVars baseClass;
  public:
    PD_AllVarsBysvMaker(PWin* parent, HWND hremote);
    ~PD_AllVarsBysvMaker();

    void actionExpand(HTREEITEM parent);
  protected:
    virtual bool isValidPrph(uint prph);
    virtual bool isValidAddr(uint prph, uint addr, uint type, uint nbit, uint offs);
    PVect<varsFilter> vFlt;

    virtual void my_TreeView_DeleteAllItems();
    virtual HTREEITEM my_TreeView_GetRoot();
    virtual HTREEITEM my_TreeView_InsertItem(TV_INSERTSTRUCT* is);
    virtual void my_TreeView_Expand(HTREEITEM parent, UINT flag);
    virtual HTREEITEM my_TreeView_GetChild(HTREEITEM parent);
    virtual BOOL my_TreeView_GetItem(TV_ITEM* item);
    virtual BOOL my_TreeView_SetItem(TV_ITEM* item);
    virtual BOOL my_TreeView_DeleteItem(HTREEITEM item);
    virtual HTREEITEM my_TreeView_GetParent(HTREEITEM item);
    virtual void my_refresh();

    p_MappedFile* mp;
    HWND hRemote;
    DWORD_PTR msgResult;
};
//----------------------------------------------------------------------------
#endif
