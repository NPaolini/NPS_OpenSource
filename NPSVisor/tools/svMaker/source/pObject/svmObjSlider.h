//-------------------- svmObjSlider.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjSlider_H_
#define svmObjSlider_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmPropertySlider.h"
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjSlider : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjSlider(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjSlider(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Slider"), 0); }
    void addInfoAd4Other(manageInfoAd4& set);
    Property* allocProperty();
    bool useFont() const { return false; }
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    bool saveVarInfo(P_File& pf, uint firstAdd, uint secondAdd, const PropertySlider::varInfo& vI);
    void loadVarInfo(uint id, setOfString& set, PropertySlider::varInfo& vI);
  private:
    void init();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
