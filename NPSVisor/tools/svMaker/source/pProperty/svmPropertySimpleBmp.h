//-------------------- svmPropertySimpleBmp.h ---------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYSIMPLEBMP_H_
#define SVMPROPERTYSIMPLEBMP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
//#include "svmObjBmp.h"
//-----------------------------------------------------------
#include "svmObjSimpleBmp.h"
//-----------------------------------------------------------
// type1 della classe base viene usato per il flag
//-----------------------------------------------------------
class PropertySimpleBmp : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertySimpleBmp();
    virtual ~PropertySimpleBmp();

    const PropertySimpleBmp& operator=(const PropertySimpleBmp& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);
    LPCTSTR filename;
    SIZE szBmp;
  protected:
    virtual void clone(const Property& other);

  private:
    PropertySimpleBmp(const PropertySimpleBmp& other);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogSimpleBmp : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogSimpleBmp(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_SIMPLE_BMP_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), Bmp(0) {  }
    ~svmDialogSimpleBmp()    {   destroy();  delete Bmp; }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void checkBlink();
    void chooseBmp();

    void allocBmp();
    void checkHeight();

    PBitmap* Bmp;
};
//-----------------------------------------------------------
#endif
