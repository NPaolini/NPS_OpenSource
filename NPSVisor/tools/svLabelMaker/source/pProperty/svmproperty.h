//--------------- svmProperty.h -----------------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTY_H_
#define SVMPROPERTY_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include "p_Vect.h"
#include "resource.h"
#include "pModDialog.h"
#include "p_util.h"
#include "p_file.h"
#include "p_txt.h"
#include "common.h"
#include "macro_utils.h"
//-----------------------------------------------------------
#define DATA_NOT_USED 2000000000
//-----------------------------------------------------------
bool choose_Color(HWND parent, COLORREF& target);
void chooseColor(HWND hwnd, uint idc, HBRUSH& hb, COLORREF& color);
//-----------------------------------------------------------
void addStringToComboBox(HWND hCBx, LPCTSTR str, uint addSize = 0);
//-----------------------------------------------------------
void fillCB_PenType(HWND hCBx, bool alsoNoLine);
void fillCB_BrushType(HWND hCBx);
//-----------------------------------------------------------
void invalidateWorkArea(PWin* child);
//-----------------------------------------------------------
void clearDual();
//-----------------------------------------------------------
#define CLONE(a) a = other.a
//-----------------------------------------------------------
// struttura contenente l'unione di tutte le proprietà, utilizzata per la modifica
// di proprietà su selezioni multiple, poi ogni oggetto usa solo quello che gli serve
struct unionProperty
{
  DWORD flagBits; // bit per identificazione proprietà modificate, usato anche come flag per proprietà da cambiare
  PRect Rect;  // right e bottom sono usati come width e height
  int dX; // per offset
  int dY;

  uint idPen; // forms
  uint idBrush;

  COLORREF foreground; // text
  uint idFont;

  int cX; // roundRect
  int cY;

  UINT hAlign;
  UINT vAlign;

  uint totObject;

  unionProperty() : flagBits(0), dX(0), dY(0), idPen(0), idBrush(0), foreground(RGB(0,0,0)), idFont(0),
                    cX(0), cY(0), hAlign(0), vAlign(0), totObject(0) {}

  enum unionPropBits {
    eupX, eupY, eupW, eupH, eupDX, eupDY, eupCX, eupCY,
    eupFG, eupPen, eupBrush, eupFont,
    eupHAlign, eupVAlign,
    eupMAX_VAL
    };
};
#define BIT_POS(v) (1 << (v))
#define PROP_BIT_POS(v) BIT_POS(unionProperty::v)
#define BASE_BIT_POS (PROP_BIT_POS(eupX) | PROP_BIT_POS(eupY) | PROP_BIT_POS(eupW) | PROP_BIT_POS(eupH) | PROP_BIT_POS(eupDX) | PROP_BIT_POS(eupDY))
//-----------------------------------------------------------
class Property
{
  public:
    Property() : idFont(0), forPaste(0) {}
    virtual ~Property();
    Property(const Property& other) : forPaste(0) { clone(other); }
    const Property& operator=(const Property& other) { forPaste = 0; clone(other);  return *this; }
    virtual void cloneMinus(const Property& other) { Property::clone(other);  }

    PRect Rect;

    UINT idFont;
    HFONT getFont(bool linked = false) { return 0; }
    struct typeFont* forPaste;

    bool setCommon(const unionProperty& uP);
    virtual void unionBits(DWORD& capable) { capable |= BASE_BIT_POS; }
    virtual void initUnion(unionProperty& uProp);

  protected:
    virtual void clone(const Property& other) { Rect = other.Rect; idFont = other.idFont; }
    virtual bool setOtherCommon(const unionProperty& uP) = 0;

  private:
//    Property(const Property& other);
};
//-----------------------------------------------------------
#define DYN_CAST(CLASS)   const CLASS& other = *dynamic_cast<const CLASS*>(&other_)
//-----------------------------------------------------------
#define SET_MODIFIED(b, v) \
    if((uP.flagBits & PROP_BIT_POS(b)) == PROP_BIT_POS(b)) { \
      if(v != uP.v) { v = uP.v;  modified = true; } }
//-----------------------------------------------------------
#define SET_INIT_UNION(b, v) \
    if(!(uProp.flagBits & PROP_BIT_POS(b))) { \
      uProp.v = v; }
//-----------------------------------------------------------
extern void modifyPageFont(PWin* parent);
//-----------------------------------------------------------
struct typeFont
{
  LPCTSTR name;
  WORD h;
  WORD w;
  HFONT hFont;
  WORD italic : 1;
  WORD bold   : 1;
  WORD underl :1;
  WORD light  : 1;
  short Angle;
  typeFont() : name(0), h(0), w(0), italic(0), Angle(0),
               bold(0), underl(0), light(0), hFont(0) { }
  ~typeFont();

  void setFlag(uint flag);
  uint getFlag() const;
  bool operator <(const typeFont& other);
  private:
    typeFont(const typeFont& other);
    const typeFont& operator =(const typeFont& other);
};
//-----------------------------------------------------------
typedef PVect<typeFont*> setOfPFont;
//-----------------------------------------------------------
// il primo font, sempre allocato, è arial, 16, 0, nessuno style
//-----------------------------------------------------------
class svmFont
{
  public:
    svmFont();
    ~svmFont();

    HFONT getFont(uint id);
    // font per lo schermo, va eliminato dal richiedente
    HFONT getFont2(uint id, short angle);
    // come sopra, ma arrotonda per eccesso, per il calcolo del rettangolo di selezione
    HFONT getFont3(uint id, short angle);

    void flush();

    const setOfPFont& getSet() const { return fontSet; }
    const typeFont* getType(uint id) const;
    void remove(uint id);
    void replace(typeFont* tpF, uint id);

    uint getElem() const { return fontSet.getElem(); }
//    int allocFont(LPCTSTR row);
    int addFont(typeFont* tpF);

//    bool saveFont(class P_File& pf, uint id);

    bool getId_add(const typeFont* tf, uint& id);
    typeFont* getCopyType(uint id);
  private:
    setOfPFont fontSet;
    void init();
};
//-----------------------------------------------------------
extern svmFont& getFontObj();
extern svmFont& getFontObjLinked();
//-----------------------------------------------------------
void setNameFont(PWin* owner, uint idc, const typeFont* tpf, bool usefont);
//-----------------------------------------------------------
class svmChooseFont : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmChooseFont(uint& idFont, PWin* parent, uint id = IDD_CHOOSE_FONT, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), idFont(idFont) {  }
    ~svmChooseFont()  { destroy(); }

    virtual bool create();
  protected:
    class svmListFont* LB;
    uint& idFont;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void chooseFont(bool replace);
    void addToLb(const typeFont* tpf, int pos);
    void remFont();

};
//-----------------------------------------------------------
inline Property::~Property() { delete forPaste; }
//-----------------------------------------------------------
template <typename T, int idInit, int preserveFirst>
class dualT
{
  public:
    dualT() : currCount(idInit) {}
    virtual ~dualT() {}

    virtual int addAndGetId(T var);

    virtual int findId(T var) const;
    virtual T getIt(int id) const;
    virtual void reset();

  protected:
    int currCount;
    struct link
    {
      int id;
      T var;
    };

    virtual T clone(T var) = 0;
    virtual int cmp(T v1, T v2) const = 0;
    virtual void freeVar(T v) = 0;
    PVect<link> Lnk;
    void releaseAll();
};
//-----------------------------------------------------------
typedef dualT<LPCTSTR, 1, 0> dual;
dual* getDualBmp();
//-----------------------------------------------------------
#define cNULL_COLOR ((DWORD)-1)
//-----------------------------------------------------------
struct infoPen // spessore, colore, stile, HPEN
{
  int tickness;
  COLORREF color;
  uint style;
  HPEN hpen;
  infoPen() : tickness(0), color(cBLACK), style(0), hpen(0) {}
  infoPen(const infoPen& other);
  const infoPen& operator =(const infoPen& other);
  ~infoPen() { if(hpen) DeleteObject(hpen); }
};
//-----------------------------------------------------------
typedef infoPen* pInfoPen;
typedef dualT<pInfoPen, 1, 1> dualPen;
//-----------------------------------------------------------
dualPen* getDualPen();
//-----------------------------------------------------------
struct infoBrush // colore, tipo (semplice = -1, tratteggio),
{
  COLORREF color;
  int style;
  HBRUSH hbrush;
  infoBrush() : color(cNULL_COLOR), style(0), hbrush(0) {}
  infoBrush(const infoBrush& other);
  const infoBrush& operator =(const infoBrush& other);
  ~infoBrush() { if(hbrush) DeleteObject(hbrush); }
};
//-----------------------------------------------------------
typedef infoBrush* pInfoBrush;
typedef dualT<pInfoBrush, 1, 1> dualBrush;
dualBrush* getDualBrush();
//-----------------------------------------------------------
class svmObject;
//-----------------------------------------------------------
class lastPropertyInUse
{
  public:
    lastPropertyInUse() {}
    ~lastPropertyInUse() { reset(); }
    Property* getLast(uint idType);
    void replaceLast(svmObject* obj);
    void reset();
  private:
    struct info
    {
      int idType;
      Property* prop;
      info() : idType(0), prop(0) {}
    };

    PVect<info> set;
};
//-----------------------------------------------------------
lastPropertyInUse* getLastPropertyInUse();
//-----------------------------------------------------------
#define INIT_COLOR RGB(0xff, 0xff, 0xff)
//-----------------------------------------------------------
class svmCommonDialog : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmCommonDialog(unionProperty& prop, PWin* parent, uint id = IDD_DIALOG_UNION_PROP, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), Prop(prop), bitsChange(prop.flagBits), idFont(0)
      {
        Prop.flagBits = 0;
      }
    ~svmCommonDialog();

    virtual bool create();
  protected:
    unionProperty& Prop;
    DWORD bitsChange;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    HBRUSH evCtlColor(HDC dc, HWND hWndChild);
    void checkEnabled(int ctrl = -1);
    void chooseFont();
    void getEnabled();
    uint idFont;
    struct infoColor
    {
      HBRUSH hb;
      COLORREF col;
      infoColor() : col(INIT_COLOR), hb(CreateSolidBrush(INIT_COLOR)) {}
      ~infoColor() { if(hb) DeleteObject(hb); }
    } iColor[3];

};
//-----------------------------------------------------------
#endif
