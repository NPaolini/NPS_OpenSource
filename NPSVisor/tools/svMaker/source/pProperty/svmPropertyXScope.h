//-------------------- svmPropertyXScope.h --------------------
//-----------------------------------------------------------
#ifndef svmPropertyXScope_H_
#define svmPropertyXScope_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObject.h"
//-----------------------------------------------------------
struct xScopeColors
{
    enum colorType {
        cBkg,
        cGrid,
        cAxe,
        cLineShow1,
        cLineShow2,
        MAX_COLOR_TYPE
        };

  COLORREF Color[MAX_COLOR_TYPE];
  DWORD notUseBit;

  xScopeColors();
  xScopeColors(const xScopeColors& other)  { clone(other); }


  const xScopeColors& operator=(const xScopeColors& other) { clone(other); return *this; }

  private:
    void clone(const xScopeColors& other);

};
//-----------------------------------------------------------
//-----------------------------------------------------------
class sampleXScope : public PStatic
{
  private:
    typedef PStatic baseClass;
  public:
    sampleXScope(PWin* parent, uint id) :
      baseClass(parent, id) {}

    void setColors(xScopeColors& refColors) { RefColors = refColors; }
    void chgBit(DWORD reset, DWORD set);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evPaint(HDC hdc);
    void drawLines(HDC hdc);
    void drawAxis(HDC hdc);
  private:
    xScopeColors RefColors;
};
//-----------------------------------------------------------
class PropertyXScope : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyXScope();
    virtual ~PropertyXScope();

    xScopeColors& getColors() { return xScopeColor; }
    const xScopeColors& getColors() const { return xScopeColor; }

    const PropertyXScope& operator=(const PropertyXScope& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    // idEnbleRead,idInitX,idCurrX,idNumBlock,idMaxNumData,idMinVal,idMaxVal
    dataPrf DataPrf[7];

    bool uniqueData;
    bool useVariableX;
    bool relativeBlock;
    bool zeroOffset;

  protected:
    virtual void clone(const Property& other);

  private:
    PropertyXScope(const PropertyXScope& other);

    xScopeColors xScopeColor;

};
//-----------------------------------------------------------
inline PropertyXScope::PropertyXScope() : uniqueData(false), useVariableX(false),
    relativeBlock(false), zeroOffset(false) { type1 = 10; }
//-----------------------------------------------------------
struct ids
{
  uint idPrph;
  uint idAddr;
  uint idType;
  uint idNorm;
};
//-----------------------------------------------------------
class svmDialogXScope : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogXScope(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_XSCOPE_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), sxs(0) {  }
    ~svmDialogXScope()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();

    sampleXScope* sxs;

    void fillData(const ids& Ids, const dataPrf& DataPrf);
    void fillDataEx(const ids& Ids, const dataPrf& DataPrf);
    bool chooseColor(uint idc);
    void drawColors();
    void unfillData(const ids& Ids, dataPrf& DataPrf);
    void unfillDataEx(const ids& Ids, dataPrf& DataPrf);
    void checkEnableColor();
    void check_const_prph(uint ids);
    void setBitsColor(DWORD idc, bool alsoRadio = true);
    void chgBit(DWORD reset, DWORD set);
    void setInfoReadyOrOffset();
    void checkEnableAxesX();
    void setPrphEqu();
};
//-----------------------------------------------------------
#endif
