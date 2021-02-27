//-------------------- svmPropertyListBox.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYLISTBOX_H_
#define SVMPROPERTYLISTBOX_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjListBox.h"
#include "POwnBtn.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogListBox : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogListBox(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_LISTBOX_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogListBox()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    virtual bool useFont() const { return true; }
    void checkEnable();

    struct colorItem
    {
        POwnBtn* Btn;
        COLORREF color;

        colorItem() : Btn(0), color(0) {}
    };
    colorItem ColorItem[4];
    void chooseColor(int ix);
    void invalidateColor(int ix);
};
//-----------------------------------------------------------
#endif
