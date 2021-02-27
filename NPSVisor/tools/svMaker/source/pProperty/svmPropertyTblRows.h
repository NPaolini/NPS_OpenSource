//-------------------- svmPropertyTblRows.h ---------------
//-----------------------------------------------------------
#ifndef svmPropertyTblRows_H_
#define svmPropertyTblRows_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmPropertyTableInfo.h"
//-----------------------------------------------------------
class svmDialogTblRows : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmDialogTblRows(svmObject* owner, PropertyTableInfo* prop, PWin* parent, uint id = IDD_TBL_INFO_ROWS, HINSTANCE hInst = 0);
    ~svmDialogTblRows();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    PropertyTableInfo* Prop;
    HBRUSH bg;
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void chooseColor(bool fgnd);
    void chooseFont();
    void setNameFont(const typeFont* tpf);
    void copyFromClipboard();
    void copyToClipboard();
    void fillRowByClipboard(LPTSTR lptstr);
};
//-----------------------------------------------------------
#endif
