//-------------------- svmPropertyXMeter.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYXMETER_H_
#define SVMPROPERTYXMETER_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjXMeter.h"
//-----------------------------------------------------------
class PropertyXMeter : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyXMeter();
    virtual ~PropertyXMeter();

    const PropertyXMeter& operator=(const PropertyXMeter& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    // per i valori per idCurr usa la classe base, questi sono per idMin e idMax
    dataPrf DataPrf[2];

    int offsX;
    int offsY;
    uint granularity;
    bool active;
    bool updateAlways;
    bool center;
    bool moveVert;
    LPCTSTR getName() const { return nameBmp; }
    void setName(LPCTSTR name_bmp) { delete []nameBmp; nameBmp = str_newdup(name_bmp); }

  protected:
    virtual void clone(const Property& other);

  private:
    PropertyXMeter(const PropertyXMeter& other);
    LPCTSTR nameBmp;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogXMeter : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogXMeter(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_XMETER_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogXMeter()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fillPrf2();
    void fillTypeVal2();
    void check_const_prph(uint ids);
    void checkActive();
    void checkCenter();
};
//-----------------------------------------------------------
#endif
