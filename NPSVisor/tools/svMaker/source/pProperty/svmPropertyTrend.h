//-------------------- svmPropertyTrend.h --------------------
//-----------------------------------------------------------
#ifndef svmPropertyTrend_H_
#define svmPropertyTrend_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObject.h"
//-----------------------------------------------------------
#include "1.h"
extern void makeStdMsgSet(setOfString& set);
//-----------------------------------------------------------
struct infoTrendName
{
  LPCTSTR name;
  LPCTSTR match;
  uint nField;
  bool history;
  infoTrendName() : name(0), match(0), nField(0), history(false) {}
  ~infoTrendName() { delete []name; delete []match; }
  private:
    infoTrendName(const infoTrendName&);
    const infoTrendName& operator=(const infoTrendName&);
};
//-----------------------------------------------------------
void getAllTrendName(PVect<infoTrendName*>& trendName, setOfString& sosStdMsg);
//-----------------------------------------------------------
struct colorLine
{
  int tickness;
  COLORREF Color;
  colorLine() : tickness(1), Color((rand() ^ (DWORD(rand()) << 16)) & 0xffffff) {}
};
//-----------------------------------------------------------
struct oTrendColors
{
    enum colorType {
        cBkg,
        cGrid,
        cLineShow,
        MAX_BASE_COLOR
        };

  COLORREF baseColor[MAX_BASE_COLOR];

  PVect<colorLine> linesColor;
  int gridType;

  oTrendColors();
  oTrendColors(const oTrendColors& other)  { clone(other); }


  const oTrendColors& operator=(const oTrendColors& other) { clone(other); return *this; }

  private:
    void clone(const oTrendColors& other);

};
//-----------------------------------------------------------
class sampleTrend;
//-----------------------------------------------------------
class PropertyTrend : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyTrend();
    virtual ~PropertyTrend();

    oTrendColors& getColors() { return oTrendColor; }
    const oTrendColors& getColors() const { return oTrendColor; }

    const PropertyTrend& operator=(const PropertyTrend& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    // idBitShow,idBitActive,idBitCtrl,idDateTime
    dataPrf DataPrf[4];
    LPCTSTR trendName;
    LPCTSTR trendMinMaxFile;
    bool autoStepTen;
    bool autoRows;
  protected:
    virtual void clone(const Property& other);

  private:
    PropertyTrend(const PropertyTrend& other);

    oTrendColors oTrendColor;

};
//-----------------------------------------------------------
// usa type1 -> numero_colonne, type2 -> numero_righe, style -> stile_bkg
inline PropertyTrend::PropertyTrend() : trendName(0), trendMinMaxFile(0), autoStepTen(false), autoRows(false) { type1 = 5; type2 = 4; }
//-----------------------------------------------------------
void paintSampleTrend(HDC hdc, const PRect& rect, const oTrendColors& info, uint nRow, uint nCol, bool inside = false);
//-----------------------------------------------------------
struct ids;
//-----------------------------------------------------------
class svmDialogTrend : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogTrend(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_TREND_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), sxs(0), bkgText(0) {  }
    ~svmDialogTrend()    {   destroy();  if(bkgText) DeleteObject(bkgText); }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();
    virtual bool useFont() const { return true; }

    sampleTrend* sxs;

    void fillData(const ids& Ids, const dataPrf& DataPrf);
    bool chooseColor(uint idc);
    void drawColors();
    void unfillData(const ids& Ids, dataPrf& DataPrf);
    void checkEnableColor();
    void setBitsColor(DWORD idc, bool alsoRadio = true);
    void chgBit(DWORD reset, DWORD set);
    void setInfoReadyOrOffset();

    HBRUSH bkgText;

    void fillDataShort(const ids& Ids, const dataPrf& DataPrf);
    void unfillDataShort(const ids& Ids, dataPrf& DataPrf);
    void fillCbType(int sel);
    void firstCheckTrend();
    void chooseTrend();
    bool chooseLineColor();
    void chooseTextColor();
    void checkUseMinMax();
    void checkAutoStep();
};
//-----------------------------------------------------------
/*
class svmChooseTrendColor : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmChooseTrendColor(PropertyTrend* prop, PWin* parent,
        uint id = IDD_XSCOPE_CHOOSE_COLOR, HINSTANCE hInst = 0);
    ~svmChooseColor();

    virtual bool create();
  protected:
    PropertyTrend* Prop;
    class svmListBoxColor* LB;
    HBRUSH hb[oTrendColors::MAX_COLOR_TYPE];
    oTrendColors currColors;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void chooseColor(int ix);
    void fill();
    void add();
    void rem();
    void move(bool up);

    void add(const oTrendColors& info);
    void invalidateColor(DWORD ix);
    oTrendColors unformat(int pos);

    void calcWidthLB(bool add);
    bool enlarged;
};
*/
//-----------------------------------------------------------
#endif
