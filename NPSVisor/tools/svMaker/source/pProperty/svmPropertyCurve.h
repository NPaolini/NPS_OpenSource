//-------------------- svmPropertyCurve.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYCURVE_H_
#define SVMPROPERTYCURVE_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjCurve.h"
//-----------------------------------------------------------
struct curveColors
{
    enum colorType {
        cBALL,
        cGRID,
        cAXE,
        cLINE_SHOW,
        MAX_COLOR_TYPE
        };

  COLORREF Color[MAX_COLOR_TYPE];

  curveColors();
  curveColors(const curveColors& other)  { clone(other); }


  const curveColors& operator=(const curveColors& other) { clone(other); return *this; }

  private:
    void clone(const curveColors& other);

};
//-----------------------------------------------------------
//typedef PVect<curveColors> setOfCurveColors;
//-----------------------------------------------------------
class PropertyCurve : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyCurve();
    virtual ~PropertyCurve();

    curveColors& getSet() { return CurveColor; }
    const curveColors& getSet() const { return CurveColor; }

    const PropertyCurve& operator=(const PropertyCurve& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    // uint nPoints; // usa nDec
    uint nRow;
    DWORD maxX;
    TCHAR fileShow[_MAX_PATH];
    bool useFileShow;
    bool hideBall;
  protected:
    virtual void clone(const Property& other);

  private:
    PropertyCurve(const PropertyCurve& other);

    curveColors CurveColor;

};
//-----------------------------------------------------------
// il valore negative della classe base viene usato per readOnly,
// type1 per perc1stSpace, type2 per nPoint1stSpace
inline PropertyCurve::PropertyCurve() : nRow(10), maxX(0), useFileShow(false), hideBall(false)
 { nDec = 10; fileShow[0] = 0; }
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogCurve : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogCurve(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_CURVE_PROPERTY, HINSTANCE hInst = 0);
    ~svmDialogCurve();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();
    HBRUSH otherBrush[curveColors::MAX_COLOR_TYPE];

    void chooseCustomColor(int ix);
    void invalidateColor(DWORD ix);

    void checkEnable();
    void checkEnableBall();
//    void chooseColors();
//    void fillPrf2();
//    void fillTypeVal2();
//    void checkTransp();

};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
