//-------------------- svmObjBmp.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJBMP_H_
#define SVMOBJBMP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
#include "svmPropertyBmp.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjBmp : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjBmp(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjBmp(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual int getIdType() const;
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);

    virtual bool canDistorce() const { return false; }
    virtual bool endDrag(HDC hdc, const POINT& pt);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Bitmap di segnalazione"), 0); }

    Property* allocProperty();
    bool useFont() const { return false; }
  protected:
    virtual void drawHandleUI(HDC hdc);
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);

    virtual void DrawTitle(HDC hdc);
    virtual void addInfoAd4Other(manageInfoAd4& set);

  private:
    void init();
    bool saveVarMoveInfo(P_File& pf, uint baseId, const objMoveInfo::varInfo& vI);
    void loadVarMoveInfo(uint id, setOfString& set, objMoveInfo::varInfo& vI);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
