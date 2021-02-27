//----------- allPages.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef allPages_H_
#define allPages_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include <stdlib.h>
#include "resource.h"
#include "pModDialog.h"
#include "pSplitter.h"
#include "allVars.h"
#include "pListview.h"
//----------------------------------------------------------------------------
#define OFFS_IMG(a) getIxImageByIdObj(a)
//----------------------------------------------------------------------------
class PD_ListView : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PD_ListView(PWin* parent, uint resId = IDD_LIST, HINSTANCE hinstance = 0);
    virtual ~PD_ListView();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual void fillTitle() {}
    virtual void fill() {}

    virtual void resize();

    virtual bool addItem(PVect<LPTSTR>& field, int ixImage = -1);
    virtual HIMAGELIST makeImageList() = 0;

    PListView* getList() { return List; }
  private:
    PListView* List;
    HIMAGELIST ImageList;
};
//----------------------------------------------------------------------------
class clientListViewPages : public PD_ListView
{
  private:
    typedef PD_ListView baseClass;
  public:
    clientListViewPages(PWin* parent, uint resId = IDD_LIST, HINSTANCE hinstance = 0);
    virtual ~clientListViewPages() { destroy(); }

    virtual void setManPage(managePages* mp) { pMP = mp; clear(); }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void fill();
    virtual void fillTitle();

    virtual HIMAGELIST makeImageList();
    managePages* pMP;

    TCHAR pagename[_MAX_PATH];

    void clear();
};
//----------------------------------------------------------------------------
struct infoRecurse
{
  LPCTSTR page;
  int repeat;
  int level;
};
//----------------------------------------------------------------------------
class clientTreeViewPages : public PD_TreeView
{
  private:
    typedef PD_TreeView baseClass;
  public:
    clientTreeViewPages(PWin* parent, uint resId = IDD_TREE, HINSTANCE hinstance = 0);
    virtual ~clientTreeViewPages() { destroy(); }

    void setManPage(managePages* mp) { pMP = mp; fill(); }
  protected:
    virtual void changedSel(LPNM_TREEVIEWW twn);
    virtual void fill();
    virtual HIMAGELIST makeImageList();

  private:
    managePages* pMP;
    void addTree(HTREEITEM root, const infoPages* rootPages, uint level, PVect<infoRecurse>& setPage);
    void addTreeStdMsgDll(HTREEITEM root, uint pgType);
    void addTreeStdMsg(HTREEITEM root);
    void addTreeNorm(HTREEITEM root);
    void addTreeDll(HTREEITEM root);
    void addTreeAlarmAssoc(HTREEITEM root);

//    PVect<LPCTSTR> SetOpened;
//    bool dejaOpened(const infoPages* ipg);
//    bool isOnParent(const infoPages* ipg, LPCTSTR pagename, PVect<const infoPages*>& visited);
};
//----------------------------------------------------------------------------
class PD_AllPages : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PD_AllPages(PWin* parent, uint resId = IDD_TREE_LIST, HINSTANCE hinstance = 0);
    virtual ~PD_AllPages();

    virtual bool create();
    HWND getTreeView();
    HWND getListView();
    void setManPage(managePages* mp);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    PSplitter* Split;
};
//----------------------------------------------------------------------------
#endif
