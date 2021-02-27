//-------------------- svmObjCurve.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJCURVE_H_
#define SVMOBJCURVE_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjCurve : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjCurve(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjCurve(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Curve"), 0); }
//    virtual bool canDistorce() const { return false; }
		void addInfoAd4Other(manageInfoAd4& set);
    Property* allocProperty();
    bool useFont() const { return false; }
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
  private:
    void init();
//    void drawWithCycle(HDC hdc);
//    void drawFilled(HDC hdc);
//    void drawMaxVal(HDC hdc);
//    void drawAxis(HDC hdc);

//    int getStyle1();
//    void setStyle1(int style);
//    int getStyle2();
//    void setStyle2(int style);

};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
