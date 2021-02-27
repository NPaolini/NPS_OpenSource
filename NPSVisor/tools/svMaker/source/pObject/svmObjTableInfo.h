//-------------------- svmObjTableInfo.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjTableInfo_H_
#define svmObjTableInfo_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjTableInfo : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjTableInfo(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED);
    svmObjTableInfo(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Table info"), 0); }
    virtual bool canDistorce() const { return false; }

    virtual typeOfAnchor pointIn(const POINT& pt) const;
    virtual typeOfAnchor getAnchor() const { if(Extern != whichPoint) return Inside; return Extern; }
    virtual Property* allocProperty();
    virtual const PRect& getRect() const;
    virtual void setRect(const PRect& rect);
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
  private:
    void init();
    PRect calcRect(int x, int y) const;
    uint saveBasePanel(P_File& pf);
    uint saveHeadPanel(P_File& pf);
    uint saveHeadText(P_File& pfClear, P_File& pfCrypt);
    uint saveRowPanel(P_File& pf);
    uint saveFirstText(P_File& pfCrypt, P_File& pfClear);
    uint saveInfoText(P_File& pfClear, P_File& pfCrypt, PVect<LPCTSTR>& list);

    void loadBasePanel(uint id, setOfString& set);
    void loadHeadPanel(uint id, setOfString& set);
    void loadHeadText(uint id, setOfString& set);
    void loadRowPanel(uint id, setOfString& set);
    void loadFirstText(uint id, setOfString& set);
    void loadInfoText(uint id, PVect<LPCTSTR>& list, setOfString& set);

};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
