//------------------- svmObjRect.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjRect_H_
#define svmObjRect_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjRect : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjRect(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjRect(PWin* par, int id, const PRect& r);

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
inline smartPointerConstString svmObjRect::getTitle() const
{
  return smartPointerConstString(_T("Rect"), 0);
}
//-----------------------------------------------------------
#endif
