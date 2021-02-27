//-------- exp_BaseContainer.h --------------------------------------
//-------------------------------------------------------------------
#ifndef exp_BaseContainer_H
#define exp_BaseContainer_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "exp_BaseEdit.h"
//-------------------------------------------------------------------
#include "setPack.h"
//-------------------------------------------------------------------
#define MAX_BODY 6
//-------------------------------------------------------------------
class exp_BaseContainer : public PSplitWin
{
  private:
    typedef PSplitWin baseClass;
  public:
    enum etype { normal, readonly, colorAsReadonly };
    exp_BaseContainer(PWin* parent, uint id, LPCTSTR tit, infoEdit* iEdit, bool readonly = false);
    ~exp_BaseContainer();
    bool create();

    PWin* getEdit() { return Ed; }
    void setText(LPCTSTR buff);
    void getText(LPTSTR buff, int dim);
    LPTSTR getText(uint& dim);
    uint getTextLen();

    const infoEdit& getInfo() const;
    void setInfo(const infoEdit& ed);
    void colorize();
    bool isDirty() const { return Dirty; }
    void resetDirty();

    enum { No, Case, Wholeword };
    bool find(uint offs, LPCTSTR word, DWORD flag, CHARRANGE& cr);
    bool select(LPCTSTR word, DWORD flag, bool next);
    bool replace(LPCTSTR newWord, LPCTSTR word, DWORD flag, bool next);
    bool replaceAll(LPCTSTR newWord, LPCTSTR word, DWORD flag);

    LPTSTR getSel();

    void setReadOnly(bool set);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType);
    void parseMsgFilter(MSGFILTER* msg);
    bool checkRow();
    void colorizeSingle(int ix_line = -1);
    void colorizePaste(CHARRANGE* cr);
    PControl* Tit;
    PControl* Ed;
    PWin* NumberRow;
    bool Dirty;
    int lastRow;
    int lastColor;
    HBRUSH bkgTit;
    void performColorizeLine(int ixLine, int colortype, LPTSTR buff, int nchar);
    void colorizeFunction(CHARRANGE& cr, LPTSTR buff, int lineNum);
    void colorizePart(int offset, int len, const CHARRANGE& cr, richEditStyle ix, int lineNum);
    bool onMySetting;
    int currRow;

    void setUndoAction(uint action);
    IRichEditOle* ireo;
    ITextDocument* itd;
    void disableEvent();
    void restoreEvent();

};
//-------------------------------------------------------------------
//-------------------------------------------------------------------
class PDialogBtn : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PDialogBtn(PWin* parent, uint id, HINSTANCE hInst = 0);
    virtual ~PDialogBtn() { destroy(); }
    HWND getEdit();
    void setText(LPCTSTR buff);
    void getText(LPTSTR buff, int dim);
    LPTSTR getText(uint& dim);
    uint getTextLen();
    bool preProcessMsg(MSG& msg);
    bool create();
    void enableSave(bool set);
    bool isCheckedReadOnly();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//-------------------------------------------------------------------
#include "restorePack.h"
//-------------------------------------------------------------------
#endif
