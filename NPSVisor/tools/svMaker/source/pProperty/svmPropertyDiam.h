//-------------------- svmPropertyDiam.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYDIAM_H_
#define SVMPROPERTYDIAM_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjDiam.h"
//-----------------------------------------------------------
struct diamColors
{
    enum colorType {
        cCROWN,
        cPIVOT,
        cAXIS,
        cBKG,
        MAX_COLOR_TYPE
        };

  COLORREF Color[MAX_COLOR_TYPE];

  diamColors();
  diamColors(const diamColors& other)  { clone(other); }
//  diamColors(const valueAndColor& other)  { clone(other); }


  const diamColors& operator=(const diamColors& other) { clone(other); return *this; }

  private:
    void clone(const diamColors& other);

};
//-----------------------------------------------------------
typedef PVect<diamColors> setOfDiamColors;
//-----------------------------------------------------------
class PropertyDiam : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyDiam();
    virtual ~PropertyDiam();

    setOfDiamColors& getSet() { return DiamColor; }
    const setOfDiamColors& getSet() const { return DiamColor; }

    const PropertyDiam& operator=(const PropertyDiam& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    bool usingColor() const { return useColor; }
    void setUseColor(bool use) { useColor = use; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    // per i valori per idCurr usa la classe base, questi sono per idMax, idMin e idColor
    dataPrf DataPrf[3];

    enum diamStyle {
        ONLY_DIAM,
        AXIS = 1,
        TRANSP_PIVOT = 1 << 1,
        DRAW_MAX = 1 << 2,
        INVERT = 1 << 3,
        };

  protected:
    virtual void clone(const Property& other);

  private:
    PropertyDiam(const PropertyDiam& other);

    setOfDiamColors DiamColor;
    bool useColor;
};
//-----------------------------------------------------------
inline PropertyDiam::PropertyDiam() :  useColor(false) { type1 = AXIS | DRAW_MAX; }
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogDiam : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogDiam(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_DIAM_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogDiam()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void chooseColors();
    void fillPrf2();
    void fillTypeVal2();
    void checkTransp();
    void check_const_prph(uint ids);
};
//-----------------------------------------------------------
class svmChooseColor : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmChooseColor(PropertyDiam* prop, PWin* parent,
        uint id = IDD_DIAM_CHOOSE_COLOR, HINSTANCE hInst = 0);
    ~svmChooseColor();

    virtual bool create();
  protected:
    PropertyDiam* Prop;
    class svmListBoxColor* LB;
    HBRUSH hb[diamColors::MAX_COLOR_TYPE];
    diamColors currColors;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void chooseColor(int ix);
    void fill();
    void add();
    void rem();
    void move(bool up);

    void add(const diamColors& info);
    void invalidateColor(DWORD ix);
    diamColors unformat(int pos);

    void calcWidthLB(bool add);
    bool enlarged;
};
//-----------------------------------------------------------
#endif
