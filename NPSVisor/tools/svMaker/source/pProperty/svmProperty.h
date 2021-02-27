//---------------- svmProperty.h ----------------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTY_H_
#define SVMPROPERTY_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include "p_Vect.h"
#include "resource.h"
#include "pModDialog.h"
#include "pPanel.h"
#include "p_util.h"
#include "p_file.h"
#include "sizer.h"
#include "p_txt.h"
#include "language_util.h"
#include "svmMoveInfo.h"
//-----------------------------------------------------------
#define DATA_NOT_USED -1
//-----------------------------------------------------------
bool choose_Color(HWND parent, COLORREF& target);
//-----------------------------------------------------------
bool makeBorderStyle(PPanelBorder& border, COLORREF& bkgColor, PWin* parent);
//-----------------------------------------------------------
void addStringToComboBox(HWND hCBx, LPCTSTR str, uint addSize = 0);
void fillCBPerif(HWND hwnd, int select);
void fillCBPerifEx(HWND hwnd, int select, bool addConstPrph = false);
void fillCBTypeVal(HWND hwnd, int select);
void fillCBPswLevel(HWND hwnd, int select, bool full = true);
void setConstValue(HWND hwedit, DWORD value, uint prph, uint type);
void makeConstValue(LPTSTR buff, uint sz, DWORD value, uint prph, uint type);
//-----------------------------------------------------------
#define CLONE(a) a = other.a
//-----------------------------------------------------------
//extern void gSetCurrPage(uint ix);
//-----------------------------------------------------------
struct dataPrf
{
  UINT perif;
  UINT addr;
  UINT typeVal;
  int normaliz;
  int dec;

  dataPrf(UINT perif, UINT addr, UINT typeVal, int normaliz, int dec=0) :
      perif(perif), addr(addr), typeVal(typeVal), normaliz(normaliz), dec(dec) {}
  dataPrf() : perif(0), addr(0), typeVal(0), normaliz(0), dec(0) {}
  dataPrf(const dataPrf& other) { clone(other); }
  const dataPrf& operator =(const dataPrf& other) { clone(other); return *this; }

  private:
    void clone(const dataPrf& other);
};
//-----------------------------------------------------------
void savePrf(P_File& pf, LPTSTR buff, int id, const dataPrf& data);
void loadPrf(uint id, setOfString& set, dataPrf& data);
//-----------------------------------------------------------
class Property
{
  public:
    Property();
    virtual ~Property();

    bool getFullCloneOnCreate();
//    static bool fullCloneOnCreate;
//    static bool useRealCoord;

    COLORREF foreground;
    COLORREF background;
    UINT style;

    UINT idFont;
    HFONT getFont(bool linked = false);
    struct typeFont* forPaste;

    enum align { aCenter, aLeft, aRight, aTop = 4, aMid = 8, aBottom = 16 };

    UINT alignText;

    void setAlign(align which);

    // dipendenti dall'oggetto
    UINT type1;
    UINT type2;

    // quasi tutti gli oggetti sono agganciati a varibili
    UINT perif;
    UINT addr;
    UINT typeVal;
    UINT nDec;
    UINT nBits;
    UINT offset;
    int normaliz;
    bool negative;

    // nuova specifica per password abbinata a variabile (solo per campi di input)
    uint pswLevel;

    // nuove specifche, dati per gestione visibilità
    UINT visPerif;
    UINT visAddr;
    UINT visnBits;
    UINT visOffset;
    bool visNegative;

    // nuova specifica per formato numero
    uint BaseNum;

    PRect Rect;

    wrapMoveInfo MoveInfo;
    // uguali a PPanel
    enum bitStyle {
         NO,              // senza bordo
         BORDER = 1 << 0, // bordo semplice (nero)
         UP = 1 << 1,     // simula un pannello rialzato
         DN = 1 << 2,     // simula un pannello incassato

         FILL = 1 << 3,   // flag di riempimento

         // unione dei vari bordi con il riempimento
         BORDER_FILL = BORDER | FILL,
         UP_FILL = UP | FILL,
         DN_FILL = DN | FILL,

         TRANSP = 1 << 4, // memorizza lo sfondo e lo ripristina

         // unione dei vari bordi con il trasparente
         BORDER_TRANSP = BORDER | TRANSP,
         UP_TRANSP = UP | TRANSP,
         DN_TRANSP = DN | TRANSP,
         };
    // serve solo per pannelli e testi, ma se si deve poter impostare su selezione multipla va messo
    // per tutti gli oggetti
    PPanelBorder Border;
    void loadBorder(LPCTSTR p);
    void saveBorder(LPTSTR t, size_t dim);

    const Property& operator=(const Property& other) { clone(other);  return *this; }
    virtual void cloneMinus(const Property& other) { Property::clone(other);  }
  protected:
    virtual void clone(const Property& other);

  private:
    Property(const Property& other);
};
//-----------------------------------------------------------
enum compatibilityFlags
{
  cfPosAndSize    = 1 << 0,
  cfColors        = 1 << 1,
  cfStyle         = 1 << 2,

  cfPerifValues   = 1 << 3, // cfAddr, cfTypeVal, cfNBits, cfOffset,

  cfType          = 1 << 4,

  cfSameObjects   = 1 << 15,
};
//-----------------------------------------------------------
class svmChooseBitmap : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmChooseBitmap(PVect<LPCTSTR>& nameBmp, PWin* parent, uint id = IDD_CHOOSE_BMP, HINSTANCE hInst = 0);
    ~svmChooseBitmap();

    virtual bool create();
  protected:
    PVect<LPCTSTR>& NameBmp;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fill();
    void add();
    void rem();
    void move(bool up);
};
//-----------------------------------------------------------
struct textAndColor
{
  LPTSTR text;
  COLORREF fgColor;
  COLORREF bgColor;
  float value;

  textAndColor(LPCTSTR txt, COLORREF fg, COLORREF bg, float value) :
      text(txt ? str_newdup(txt) : 0), fgColor(fg), bgColor(bg), value(value) {}
  textAndColor(LPCTSTR txt) : text(str_newdup(txt)), fgColor(-1), bgColor(-1), value(0) {}
  textAndColor() : text(0), fgColor(-1), bgColor(-1), value(0) {}
  textAndColor(const textAndColor& other) : text(0) { clone(other); }
  ~textAndColor();

  const textAndColor& operator=(const textAndColor& other) { clone(other); return *this; }

  private:
    void clone(const textAndColor& other);

};
//-----------------------------------------------------------
typedef PVect<textAndColor*> setOfPTextColor;
//-----------------------------------------------------------
struct valueAndColor
{
  COLORREF Color;
  float value;

  valueAndColor(COLORREF color, float value) : Color(color), value(value) {}
  valueAndColor() : Color(-1), value(0) {}
  valueAndColor(const valueAndColor& other)  { clone(other); }

//  ~valueAndColor();

  const valueAndColor& operator=(const valueAndColor& other) { clone(other); return *this; }

  private:
    void clone(const valueAndColor& other) { Color = other.Color; value = other.value; }

};
//-----------------------------------------------------------
typedef PVect<valueAndColor> setOfValueColor;
//-----------------------------------------------------------
struct typeFont
{
  LPCTSTR name;
  WORD h;
  WORD w;
  HFONT hFont;
  DWORD italic : 1;
  DWORD bold   : 1;
  DWORD underl :1;
  DWORD light  : 1;
  typeFont() : name(0), h(0), w(0), italic(0),
               bold(0), underl(0), light(0), hFont(0) { }
  ~typeFont();

  private:
    typeFont(const typeFont& other);
    const typeFont& operator =(const typeFont& other);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
typedef PVect<typeFont*> setOfPFont;
//-----------------------------------------------------------
// il primo font, sempre allocato, è arial, 12, 0, nessuno style
//-----------------------------------------------------------
class svmFont
{
  public:
    svmFont();
    ~svmFont();

    void addFont(typeFont* tpF);
    HFONT getFont(uint id);

    void flush();

    const setOfPFont& getSet() const { return fontSet; }
    const typeFont* getType(uint id) const;
    void remove(uint id);
    void replace(typeFont* tpF, uint id);

    uint getElem() const { return fontSet.getElem(); }
    void allocFont(class setOfString& set);

    bool saveFont(class P_File& pf);

    typeFont* getCopyType(uint id);
    bool getId_add(const typeFont* tf, uint& id);
  private:
    setOfPFont fontSet;
    void init();
};
//-----------------------------------------------------------
extern svmFont& getFontObj();
//-----------------------------------------------------------
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
enum commonChanged {
     CHANGED_FG,
     CHANGED_BG,
     CHANGED_PRF,
     CHANGED_ADDR,
     CHANGED_VALUETYPE,
     CHANGED_NDEC,
     CHANGED_NBITS,
     CHANGED_OFFSET,
     CHANGED_NEGATIVE,
     CHANGED_X,
     CHANGED_Y,
     CHANGED_W,
     CHANGED_H,
     CHANGED_FONT,
     CHANGED_DX,
     CHANGED_DY,
     CHANGED_PSW_LEVEL,

     CHANGED_PRF_V,
     CHANGED_ADDR_V,
     CHANGED_NBITS_V,
     CHANGED_OFFSET_V,
     CHANGED_NEG_V,
     CHANGED_SIMPLE_TEXT,

     CHANGED_NORMALIZ,
     CHANGED_ALIGN,
     CHANGED_ALIGN2,
     CHANGED_BORDER,

     CHANGED_MOVEMENT,

     MAX_CHANGED
     };
//-----------------------------------------------------------
class svmBaseDialogProperty : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmBaseDialogProperty(class svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), Owner(owner), Prop(prop), fg(0), bg(0), tmpProp(0) {  }
    ~svmBaseDialogProperty();

    virtual bool create();
  protected:
    Property* Prop;
    Property* tmpProp;
    class svmObject* Owner;

    // per creare tmpProp
    virtual Property* allocProperty();

    virtual smartPointerConstString getTitle() const;

    HBRUSH fg;
    HBRUSH bg;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void chooseColor(bool fgnd);
    void updateColor(bool fgnd);
    void chooseFont();
    void fillPrf();
    void fillTypeVal();
    void fillPswLevel();
    void getPrf();
    void getTypeVal();
    void getPswLevel();
    virtual void CBChanged(uint idCtrl);
    void chooseNormaliz(uint idCtrl);
    void setNameFont(const typeFont* tpf);
    virtual bool useFont() const { return false; }

    void checkBitOrValueVis();

    void checkEnabledMove();
    void movingData();

};
//-----------------------------------------------------------
class svmCommonDialog : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmCommonDialog(DWORD& bitsChange, Property* prop, LPTSTR& simpleText, PWin* parent, uint id = IDD_COMMON_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(0, prop, parent, id, hInst), bitsChange(bitsChange), simpleText(simpleText)
      {
        wsprintf(title, _T("%d oggetti"), Prop->type1);
      }
    ~svmCommonDialog();

    virtual bool create();
  protected:
    DWORD& bitsChange;
    TCHAR title[50];
    virtual smartPointerConstString getTitle() const;
    virtual Property* allocProperty();
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void checkEnabled(int ctrl = -1);
    void getEnabled();
    LPTSTR& simpleText;
};
//----------------------------------------------------------------------------
enum eBase { ebTen, ebBinary, ebHex };
//-----------------------------------------------------------
inline Property::Property() :
    style(Property::DN_FILL), foreground(0), background(GetSysColor(COLOR_BTNFACE)),
    alignText(aCenter), type1(0), type2(0), perif(0), addr(0), nDec(0),
    typeVal(0), nBits(0), offset(0), negative(false), idFont(0), normaliz(0),
    visPerif(0), visAddr(0), visnBits(0), visOffset(0), visNegative(0),
    pswLevel(0), forPaste(0), BaseNum(ebTen)

{ }
//-----------------------------------------------------------
inline Property::~Property() { delete forPaste; }
//-----------------------------------------------------------
#endif
