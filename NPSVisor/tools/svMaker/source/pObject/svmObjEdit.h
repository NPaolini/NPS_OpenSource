//------------------ svmObjEdit.h ---------------------------
//-----------------------------------------------------------
#ifndef SVMOBJEDIT_H_
#define SVMOBJEDIT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjEdit : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjEdit(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjEdit(PWin* par, int id, const PRect& r);

    svmObjEdit(svmObject* other, int id);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);

    void makeDef4Cam(uint prf, uint addr);
    virtual Property* allocProperty() { return new Property; }
  protected:
    virtual void DrawObject(HDC hdc);
    virtual smartPointerConstString getTitle() const;
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
};
//-----------------------------------------------------------
#endif
