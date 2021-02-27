//-------------------- svmObjText.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJTEXT_H_
#define SVMOBJTEXT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjText : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjText(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjText(PWin* par, int id, const PRect& r);

    svmObjText(svmObject* other, int id);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual int getIdType() const;
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual LPCTSTR getObjName() const;

    void makeDef4Cam(uint prf, uint addr);
    virtual void fillTips(LPTSTR tips, int size);

    void setCommonProperty(const Property* prop, DWORD bits, LPCTSTR simpleText);
    Property* allocProperty();
    virtual void setOffsetAddr(const infoOffsetReplace& ior);
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    void setOffsetLang(const infoOffsetReplace& ior);
  private:
    void init();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
