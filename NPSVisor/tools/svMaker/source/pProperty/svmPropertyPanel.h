//-------------------- svmPropertyPanel.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYPANEL_H_
#define SVMPROPERTYPANEL_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjPanel.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogPanel : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogPanel(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_PANEL_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogPanel()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void check();
    void checkBlink();
    virtual void CmOk();
};
//-----------------------------------------------------------
#endif
