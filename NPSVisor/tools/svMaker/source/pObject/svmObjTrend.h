//-------------------- svmObjTrend.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjTrend_H_
#define svmObjTrend_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjTrend : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjTrend(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjTrend(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Trend"), 0); }

    Property* allocProperty();
    virtual void setOffsetAddr(const infoOffsetReplace& ior);

  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    virtual void addInfoAd4Other(manageInfoAd4& set);
  private:
    void init();
    void drawFilled(HDC hdc);
    void drawAxis(HDC hdc);
    void drawLines(HDC hdc);

    int getStyle();
    void setStyle(int style);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
