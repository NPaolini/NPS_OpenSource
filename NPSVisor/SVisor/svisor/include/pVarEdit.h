//-------- PVarEdit.h --------------------------------------------------------
#ifndef PVAREDIT_H_
#define PVAREDIT_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "svEdit.h"
#include "svVisualTextEdit.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
#define WAIT_UPDATE_CHAR_EDIT _T("---")
//----------------------------------------------------------------------------
class PVarEdit : public svEdit, virtual public baseActive
{
  public:
    PVarEdit(P_BaseBody* owner, uint id);
    virtual ~PVarEdit();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    bool evCtlColor(HBRUSH& result, HDC hdc, HWND hwnd, uint message);

    bool setFocus(pAround::around where);
    virtual void set_Text(LPCTSTR txt) { SetWindowText(getHwnd(), txt); }


    bool getData(prfData& target, int prph);
    void getColors(COLORREF& fg, COLORREF& bkg) { fg = Fg; bkg = Bkg; }
    virtual void setVisibility(bool set);
    virtual void setCustomPaste(bool set) { customPasteAndUndo = set; }
    void resetClip() { clipped = false; }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual uint getBaseId() const { return ID_INIT_VAR_EDI; }
    virtual HWND getHwnd() { return getHandle(); }
    virtual bool evKeyDown(WPARAM& key);
    virtual bool evChar(WPARAM& key);

    HWND evSetAroundFocus(UINT key);

    void sendCurrData();
    pVisualTextEdit* vTextEdit;
  private:
    HBRUSH Brush;
    COLORREF Fg;
    COLORREF Bkg;
    static int useSeparator;
    bool clipped;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

