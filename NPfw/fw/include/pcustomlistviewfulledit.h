//-------------- PCustomListViewFullEdit.h --------------------------
//-----------------------------------------------------------
#ifndef PCustomListViewFullEdit_H_
#define PCustomListViewFullEdit_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "PCustomListView.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PCustomListViewFullEdit : public PCustomListView
{
  private:
    typedef PCustomListView baseClass;
  public:
    PCustomListViewFullEdit(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0) : bEditing(false), Dirty(false),
        baseClass(parent, id, x, y, w, h, text, hinst) { Attr.style |= LVS_EDITLABELS; }

    PCustomListViewFullEdit(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0) : bEditing(false), Dirty(false),
        baseClass(parent, id, r, text, hinst) { Attr.style |= LVS_EDITLABELS; }

    PCustomListViewFullEdit(PWin * parent, uint resid, HINSTANCE hinst = 0) :
         bEditing(false), Dirty(false), baseClass(parent, resid, hinst)
         {  }


    ~PCustomListViewFullEdit() { destroy(); }
    virtual bool create();

    // deve essere richiamata dalla window genitrice in risposta al WM_NOTIFY.
    // La genitrice deve tornare result (o chiamare SetWindowLong(hwnd, DWL_MSGRESULT, result); in caso di dialogo)
    // se questa torna successo, far eseguire il normale iter in caso contrario
    virtual bool ProcessNotify(LPARAM lParam, LRESULT& result);

    // l'id dell'edit corrisponde all'IDOK e verrebbe processato come pulsante di ok
    // per evitarlo si può, o mettere un altro pulsante e disabilitare il CmOk()
    // oppure verificare che l'ok non venga dall'edit comfrontandolo con questo
#if 1
    static HWND getEditHandle() { return wEdit ? wEdit->getHandle() : 0; }
    static int getiItem() { return iItem; }
    static int getiSubItem() { return iSubItem; }
    static void setiItem(int i) { iItem = i; }
    static void setiSubItem(int s) { iSubItem = s; }
    static PRect getrcSubItem() { return rcSubItem; }
#else
    static HWND getEditHandle() { return hwndEdit; }
#endif
    virtual bool isDirty() { return Dirty; }
    void resetDirty() { Dirty = false; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void personalizeEdit(int item, int subItem, LPTSTR txt, size_t dim) {}
  virtual bool personalizeEditEnd(int item, int subItem, LPTSTR txt, size_t dim) { return false;  }

#if 1
    static PWin* wEdit;
    virtual PWin* makeWinEdit(HWND hwnd);
#else
    static WNDPROC wpOrigEditProc;
    static HWND hwndEdit;     // handle of hacked edit control
#endif
    static int iItem;    // Currently selected item index
    static int iSubItem;  // Currently selected subitem index
    static PRect rcSubItem;   // RECT  of selected subitem

    bool bEditing;
    // per gestire quale item può essere modificato
    // # non è più necessario, usa il metodo canEditItem() e quindi basta ridefinire quello
    virtual void onNm_Click(NMITEMACTIVATE* pia);

    // per defaul tutte le colonne sono editabili
    virtual bool canEditItem(int item, int subItem) const { return item != -1 && subItem != -1; }

    bool Dirty;
#if 0
  private:
    friend LRESULT APIENTRY EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
