//------------------- svmObjListBox.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJLISTBOX_H_
#define SVMOBJLISTBOX_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjListBox : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjListBox(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjListBox(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Listbox"), 0); }

    virtual Property* allocProperty() { return new Property; }
  protected:
    virtual void DrawObject(HDC hdc);
    virtual void DrawTitle(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
  private:
    void init();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
