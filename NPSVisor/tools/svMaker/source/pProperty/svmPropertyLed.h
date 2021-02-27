//-------------------- svmPropertyLed.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYLED_H_
#define SVMPROPERTYLED_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjLed.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class PropertyLed : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyLed();
    virtual ~PropertyLed();

    const PropertyLed& operator=(const PropertyLed& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);
    const PVect<LPCTSTR>& getNames() const { return nameBmp; }
    PVect<LPCTSTR>& getNames() { return nameBmp; }

    // usa nBits
    // int nDigit;
    bool showZero;

  protected:
    virtual void clone(const Property& other);

  private:
    PropertyLed(const PropertyLed& other);
    PVect<LPCTSTR> nameBmp;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogLed : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogLed(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_LED_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogLed()    {   destroy();  }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void chooseBmp();
    void checkDigit();
};
//-----------------------------------------------------------
#endif
