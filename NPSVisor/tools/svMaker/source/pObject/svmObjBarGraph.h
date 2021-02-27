//------------------- svmObjBarGraph.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJBARGRAPH_H_
#define SVMOBJBARGRAPH_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
#include "svmPropertyBarGraph.h"
//-----------------------------------------------------------
class svmObjBarGraph : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjBarGraph(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjBarGraph(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual uint getStyle() const;
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("BarGraph"), 0); }

    virtual void fillTips(LPTSTR tips, int size);
    virtual Property* allocProperty() { return new PropertyBarGraph; }
    virtual void setOffsetAddr(const infoOffsetReplace& ior);
    void setCommonProperty(const Property* prop, DWORD bits, LPCTSTR simpleText);
    bool useFont() const { return false; }

  protected:
    virtual void DrawObject(HDC hdc);
    virtual void DrawTitle(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    virtual void addInfoAd4Vis(manageInfoAd4& set) {}
//    virtual LPCTSTR getTextInfoVis() { return _T("Visibilità"); }
    virtual LPCTSTR getTextInfoBase() { return _T("Visibilità"); }
    virtual void addInfoAd4Other(manageInfoAd4& set);

  private:
    void init();
    void loadVarInfo(uint id, setOfString& set, PropertyBarGraph::varInfo& vI);
    bool saveVarInfo(P_File& pf, uint firstAdd, uint secondAdd, const PropertyBarGraph::varInfo& vI);
    void loadColors(uint id, setOfString& set, setOfValueColor& c);
    bool saveColors(P_File& pf, uint id, const setOfValueColor& c);
    void setStyle(uint style);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
