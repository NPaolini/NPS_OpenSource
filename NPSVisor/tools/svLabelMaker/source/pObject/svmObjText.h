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

    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pf);
    virtual bool load(LPCTSTR row);

    virtual void fillTips(LPTSTR tips, int size);

    Property* allocProperty();
    virtual void copyPropFrom(const Property* other);
    virtual bool endDrag(HDC hdc, const POINT& pt);
    virtual svmObject::typeOfAnchor pointIn(const POINT& pt) const;
    void setCommonProperty(const unionProperty& prop);
    virtual void restoreByHistoryObject(const svmObjHistory& history);
    virtual void setRect(const PRect& rect);
    virtual const PRect& getLinkedRect() const;

    bool setOffsetVar(int offs);
    virtual bool useFont() const { return true; }
    virtual void resolveFont(const typeFont* tf);
  protected:
    virtual void drawRectUI(HDC hdc, const PRect& rtAll);
    virtual void drawBoxes(HDC hdc, const PRect& all);
    virtual void reInsideRect(PRect& test, const PRect& area);
    bool intersec(const PRect& frame);
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    virtual bool performDialogProperty();
    bool calcRectUI(POINT* pPoints, const PRect& rect) const;

    LPCTSTR setBoxedAlign(LPCTSTR row);
    void setAlign(int align);
  private:
    void init();
    PRect trueRect;
    void makeTrueRect();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
