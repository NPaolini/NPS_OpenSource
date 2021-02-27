//-------------------- svmPropertyPlotXY.h --------------------
//-----------------------------------------------------------
#ifndef svmPropertyPlotXY_H_
#define svmPropertyPlotXY_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObject.h"
#include "svmPropertyXScope.h"
//-----------------------------------------------------------
class PropertyPlotXY : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyPlotXY();
    virtual ~PropertyPlotXY();

    xScopeColors& getColors() { return xScopeColor; }
    const xScopeColors& getColors() const { return xScopeColor; }

    const PropertyPlotXY& operator=(const PropertyPlotXY& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    enum idvars {
      // è nel dato base -> eiv_idInitY,
      eiv_idMinValY, eiv_idMaxValY,
      eiv_idMinValX, eiv_idMaxValX,
      eiv_idMaxNumData,
      eiv_idInitX,
      eiv_idCurrPoint,
      eiv_idEnableRead,
//      eiv_idNumBlock,
      eiv_max_vars
      };
    dataPrf DataPrf[eiv_max_vars];

    bool relativeBlock_Y;
    bool relativeBlock_X;

  protected:
    virtual void clone(const Property& other);

  private:
    PropertyPlotXY(const PropertyPlotXY& other);

    xScopeColors xScopeColor;

};
//-----------------------------------------------------------
inline PropertyPlotXY::PropertyPlotXY() : relativeBlock_Y(false), relativeBlock_X(false)  { type1 = 10; type2 = 10; }
//-----------------------------------------------------------
struct ids;
//-----------------------------------------------------------
class svmDialogPlotXY : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogPlotXY(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_XY_PLOT_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), sxs(0) {  }
    ~svmDialogPlotXY()    {   destroy();   }

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
};
//-----------------------------------------------------------
#endif
