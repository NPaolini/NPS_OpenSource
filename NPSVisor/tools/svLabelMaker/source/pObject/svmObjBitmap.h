//------------------- svmObjBitmap.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjBitmap_H_
#define svmObjBitmap_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjBitmap : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjBitmap(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjBitmap(PWin* par, int id, const PRect& r);

    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pf);
    virtual bool load(LPCTSTR row);

    virtual void fillTips(LPTSTR tips, int size);
    virtual Property* allocProperty();
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
inline smartPointerConstString svmObjBitmap::getTitle() const
{
  return smartPointerConstString(_T("Bitmap"), 0);
}
//-----------------------------------------------------------
#endif
