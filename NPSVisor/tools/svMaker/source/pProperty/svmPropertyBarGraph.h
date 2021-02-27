//-------------------- svmPropertyBarGraph.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYBARGRAPH_H_
#define SVMPROPERTYBARGRAPH_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmProperty.h"
#include "pGraphBmp.h"
//#include "svmObjBarGraph.h"
//-----------------------------------------------------------
class PropertyBarGraph : public Property
{
  private:
    typedef Property baseClass;
  public:

    PropertyBarGraph();
    virtual ~PropertyBarGraph();

    const setOfValueColor& getValueColors() const { return ValueColor; }
    setOfValueColor& getValueColors() { return ValueColor; }

    const PropertyBarGraph& operator=(const PropertyBarGraph& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }


    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);
    void copyVarsToBase();
    void copyVarsFromBase();

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

    varInfo vI[3];

    bool negValue;

    struct infoBmp
    {
      DWORD tiled : 1;
      DWORD rotate : 1;
      DWORD mirror : 1;
      TCHAR img[_MAX_PATH];
      infoBmp() : tiled(0), rotate(0), mirror(0) { ZeroMemory(img, sizeof(img)); }
      int getFlag() const;
      void setFlag(int flag);
    };

    infoBmp iBmp[2];

    int getFlag(const infoBmp& ib) { return ib.getFlag(); }
    void setFlag(int flag, infoBmp& ib) { ib.setFlag(flag); }

    int centerZero;
    // valori di test per il disegno con lo zero centrato, min(0), max(1), se il minimo non è minore di zero
    // disegna normalmente
    long valueTest[2];
  protected:
    virtual void clone(const Property& other);

  private:
    PropertyBarGraph(const PropertyBarGraph& other);

    setOfValueColor ValueColor;

};
//-----------------------------------------------------------
inline PropertyBarGraph::PropertyBarGraph() : negValue(false), centerZero(0)
{
  valueTest[0] = valueTest[1] = 0;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogBarGraph : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogBarGraph(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_BARGRAPH_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogBarGraph()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void chooseValueAndColor();
    void chooseBmp(uint idc);
    void checkEnable(bool isbar);
    void check_const_prph(uint ids);
};
//-----------------------------------------------------------
#endif
