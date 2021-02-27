//-------------------- svmPropertySpin.h --------------------
//-----------------------------------------------------------
#ifndef svmPropertySpin_H_
#define svmPropertySpin_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjSpin.h"
//-----------------------------------------------------------
#define DEF_EDGE_BMP 32
//-----------------------------------------------------------
class svmDialogSpin : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogSpin(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_SPIN_PROPERTY, HINSTANCE hInst = 0);
    ~svmDialogSpin();

    virtual bool create();
  protected:
//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
