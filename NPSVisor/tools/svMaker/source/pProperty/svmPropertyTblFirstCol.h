//-------------------- svmPropertyTblFirstCol.h ---------------
//-----------------------------------------------------------
#ifndef svmPropertyTblFirstCol_H_
#define svmPropertyTblFirstCol_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmPropertyTableInfo.h"
//-----------------------------------------------------------
class svmDialogTblFirstCol : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmDialogTblFirstCol(svmObject* owner, PropertyTableInfo* prop, PWin* parent, uint id = IDD_TBL_INFO_FIRST, HINSTANCE hInst = 0);
    ~svmDialogTblFirstCol();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void check();
    virtual void CmOk();

    PropertyTableInfo* Prop;
    HBRUSH bgOff;
    HBRUSH bgOn;
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void chooseColor(uint idc);
    void chooseFont();
    void setNameFont(const typeFont* tpf);
    void openBmpName(uint idcEdit);
};
//-----------------------------------------------------------
#endif
