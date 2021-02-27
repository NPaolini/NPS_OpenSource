//-------------------- svmPropertySlider.h --------------------
//-----------------------------------------------------------
#ifndef svmPropertySlider_H_
#define svmPropertySlider_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmProperty.h"
#include "pGraphBmp.h"
//#include "svmObjBarGraph.h"
//-----------------------------------------------------------
class PropertySlider : public Property
{
  private:
    typedef Property baseClass;
  public:

    PropertySlider();
    virtual ~PropertySlider();

    const PropertySlider& operator=(const PropertySlider& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }


    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    struct varInfo
    {
      UINT perif;
      UINT addr;
      UINT typeVal;
      int normaliz;

      int nBit;
      int offs;

      varInfo() : perif(0), addr(0), typeVal(0), normaliz(0), nBit(0), offs(0) {}
    };

    varInfo vI[2]; // min, max


    TCHAR img[_MAX_PATH];

    uint H_W; // nel caso di fullImage indica quanta parte del bitmap deve comunque essere visualizzata
    bool fullImage; // se false il valore corrente è al centro dell'immagine, se true il valore è al bordo interessato
    bool vert; // ruota sia l'immagine che l'azione di 90° in senso antiorario
    bool mirror; // l'immagine viene specchiata
    bool reverse; // il verso del movimento è al contrario del valore

    bool readOnly;
    bool updateAlways;

  protected:
    virtual void clone(const Property& other);

  private:
    PropertySlider(const PropertySlider& other);

};
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogSlider : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogSlider(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_SLIDER_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogSlider()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void chooseBmp(uint idc);
    void check_const_prph(uint ids);
};
//-----------------------------------------------------------
#endif
