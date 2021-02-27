//-------------------- svmObjCam.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJCAM_H_
#define SVMOBJCAM_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjCam : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjCam(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjCam(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual int getIdType() const;
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual LPCTSTR getObjName() const;

    Property* allocProperty();
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
  private:
    void init();
    void makeBtn(DWORD Bits);

};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
