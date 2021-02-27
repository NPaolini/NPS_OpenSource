//------------------- svmObjEllipse.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjEllipse_H_
#define svmObjEllipse_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjEllipse : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjEllipse(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjEllipse(PWin* par, int id, const PRect& r);

    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pf);
    virtual bool load(LPCTSTR row);

    virtual void fillTips(LPTSTR tips, int size);
    virtual Property* allocProperty();
    void setCommonProperty(const unionProperty& prop);
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    virtual bool performDialogProperty();
  private:
    void init();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline smartPointerConstString svmObjEllipse::getTitle() const
{
  return smartPointerConstString(_T("Ellipse"), 0);
}
//-----------------------------------------------------------
#endif
