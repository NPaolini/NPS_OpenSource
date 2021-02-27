//-------------------- svmPropertyCam.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYCAM_H_
#define SVMPROPERTYCAM_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjCam.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class PropertyCam : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyCam();
    virtual ~PropertyCam();

    void setText(LPCTSTR txt);

    LPCTSTR getText() const { return Text; }

    const PropertyCam& operator=(const PropertyCam& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

  protected:
    virtual void clone(const Property& other);

  private:
    PropertyCam(const PropertyCam& other);
    LPCTSTR Text;
};
//-----------------------------------------------------------
inline PropertyCam::PropertyCam() :  Text(str_newdup(_T("Cam"))) { }
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogCam : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogCam(svmObject* owner, Property* prop, PWin* parent, DWORD* bits = 0, uint id = IDD_CAM_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), Bits(bits) {  }
    ~svmDialogCam()    {   destroy();   }

    virtual bool create();
  protected:
    DWORD* Bits;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void checkEnable();
};
//-----------------------------------------------------------
#endif
