//------------------- svmObjLine.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjLine_H_
#define svmObjLine_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
#include "lineEquation.h"
//-----------------------------------------------------------
class svmObjLine : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjLine(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjLine(PWin* par, int id, const PRect& r);

    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pf);
    virtual bool load(LPCTSTR row);

    virtual void fillTips(LPTSTR tips, int size);
    virtual Property* allocProperty();
    virtual void copyPropFrom(const Property* other);
    virtual bool endDrag(HDC hdc, const POINT& pt);
    virtual svmObject::typeOfAnchor pointIn(const POINT& pt) const;
    virtual bool intersec(const PRect& frame);
    void setCommonProperty(const unionProperty& prop);
    virtual void restoreByHistoryObject(const svmObjHistory& history);
    virtual void setRect(const PRect& rect);
  protected:
    virtual void drawRectUI(HDC hdc, const PRect& rtAll);
    virtual void drawBoxes(HDC hdc, const PRect& all);
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    virtual void reInsideRect(PRect& test, const PRect& area);
    virtual bool performDialogProperty();
  private:
    void init();
    void toggleDir();
    lineEquation lEq;
    void makeEquation();

};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline smartPointerConstString svmObjLine::getTitle() const
{
  return smartPointerConstString(_T("Line"), 0);
}
//-----------------------------------------------------------
#endif
