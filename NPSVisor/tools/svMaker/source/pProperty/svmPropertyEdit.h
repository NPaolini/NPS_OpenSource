//-------------------- svmPropertyEdit.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYEDIT_H_
#define SVMPROPERTYEDIT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjEdit.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogEdit : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogEdit(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_EDIT_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogEdit()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    virtual void CBChanged(uint idCtrl);
    virtual bool useFont() const { return true; }
    void checkOldStyle();
    void checkBase();
};
//-----------------------------------------------------------
#endif
