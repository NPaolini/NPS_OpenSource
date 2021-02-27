//-------------------- svmPropertyTblHeader.h ---------------
//-----------------------------------------------------------
#ifndef svmPropertyTblHeader_H_
#define svmPropertyTblHeader_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmPropertyTableInfo.h"
//-----------------------------------------------------------
class svmDialogTblHeader : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmDialogTblHeader(svmObject* owner, PropertyTableInfo* prop, PWin* parent, uint id = IDD_TBL_INFO_HEAD, HINSTANCE hInst = 0);
    ~svmDialogTblHeader();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PropertyTableInfo* Prop;
    virtual void CmOk();

    HBRUSH bg;
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void chooseColor(bool fgnd);
    void chooseFont();
    void setNameFont(const typeFont* tpf);
};
//-----------------------------------------------------------
#endif
