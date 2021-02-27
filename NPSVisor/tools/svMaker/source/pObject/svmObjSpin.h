//-------------------- svmObjSpin.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjSpin_H_
#define svmObjSpin_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjSpin : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjSpin(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED);
    svmObjSpin(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Spin"), 0); }
    virtual bool canDistorce() const { return false; }

    virtual typeOfAnchor pointIn(const POINT& pt) const;
    virtual typeOfAnchor getAnchor() const { if(Extern != whichPoint) return Inside; return Extern; }
    virtual Property* allocProperty();
    virtual const PRect& getRect() const;
    virtual void setRect(const PRect& rect);
    bool useFont() const { return false; }
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
