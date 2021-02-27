//-------------------- svmObjLed.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJLED_H_
#define SVMOBJLED_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
#include "svmPropertyLed.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjLed : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjLed(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjLed(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual int getIdType() const;
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("LED"), 0); }

    Property* allocProperty();
    bool useFont() const { return false; }
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);

//    virtual void DrawTitle(HDC hdc);
  private:
    void init();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
