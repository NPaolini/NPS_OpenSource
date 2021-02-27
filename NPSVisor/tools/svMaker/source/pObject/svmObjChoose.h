//-------------------- svmObjChoose.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJCHOOSE_H_
#define SVMOBJCHOOSE_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjChoose : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjChoose(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjChoose(PWin* par, int id, const PRect& r);
    svmObjChoose(svmObject* other, int id);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);

    Property* allocProperty();
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
  private:
    void init();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
