//-------------------- svmPropertyText.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYTEXT_H_
#define SVMPROPERTYTEXT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjText.h"
//-----------------------------------------------------------
#define simpleVarText type1
#define idInitVarText type2
//-----------------------------------------------------------
class alternateTextSet
{
  public:
    alternateTextSet();
    ~alternateTextSet();
    bool load(setOfString& set, uint id);
    bool save(P_File& pf, uint id);
    LPCTSTR getFileName() const { return Filename; }
    uint getPrph() const { return Prph; }
    uint getAddr() const { return Addr; }
    uint getDim() const { return Dim; }

    void setFileName(LPCTSTR newname) { delete []Filename; Filename = newname ? str_newdup(newname) : 0; }
    void setPrph(uint v) { Prph = v; }
    void setAddr(uint v) { Addr = v; }
    void setDim (uint v) { Dim = v; }
  private:
    LPCTSTR Filename;
    uint Prph;
    uint Addr;
    uint Dim;
    NO_COPY_COSTR_OPER(alternateTextSet)
};
//-----------------------------------------------------------
class PropertyText : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyText();
    virtual ~PropertyText();

    void setText(LPCTSTR txt);

    LPCTSTR getText() const { return Text; }
    setOfPTextColor& getSet() { return TextColor; }
    const setOfPTextColor& getSet() const { return TextColor; }

    const PropertyText& operator=(const PropertyText& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    bool usingColor() const { return useColor; }
    void setUseColor(bool use) { useColor = use; }

    enum typedId { simple, var_text, var_value, var_text_value, var_text_value_text };
    bool isSimpleText() const { return simple == simpleVarText; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    void setBlinkTime(DWORD tick) { Tick = tick; }
    DWORD getBlinkTime() const { return Tick; }
    bool forTitle;

    alternateTextSet& getAltObj() { return altText; }
    const alternateTextSet& getAltObj() const { return altText; }
  protected:
    virtual void clone(const Property& other);

  private:
    PropertyText(const PropertyText& other);
    LPCTSTR Text;

    setOfPTextColor TextColor;
    bool useColor;
    DWORD Tick;
    alternateTextSet altText;
};
//-----------------------------------------------------------
inline PropertyText::PropertyText() :  Text(str_newdup(_T("sample"))), useColor(false), Tick(0), forTitle(false) { }
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogText : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogText(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_TEXT_PROPERTY, HINSTANCE hInst = 0);
    ~svmDialogText()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void checkSimpleOrVar();
    void chooseTextAndColor();
    void checkTypeVar(bool byCheckSimple = false);
    void checkBorder();
    void checkBlink();
    void checkTitle();
    virtual bool useFont() const { return true; }
    void checkBase();
};
//-----------------------------------------------------------
class svmChooseTextAndColor : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmChooseTextAndColor(PropertyText* prop, uint& useColorAndBits, COLORREF* baseColor,
          setOfPTextColor& txtColor, PWin* parent, uint id = IDD_TEXT_AND_COLOR,
          HINSTANCE hInst = 0);
    ~svmChooseTextAndColor();

    virtual bool create();
  protected:
    PropertyText* Prop;
    setOfPTextColor& txtColor;
    COLORREF* baseColor;
    uint& useColorAndBits;
    class svmListBoxVarColorText* LB;
    class POwnBtn* BtnFg;
    class POwnBtn* BtnBg;
    HBRUSH fg;
    HBRUSH bg;
    COLORREF foreground;
    COLORREF background;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void chooseColor(bool fgnd);
    void fill();
    void add();
    void rem();
    void mod();
    void move(bool up);
    void enableColor();
    void replaceText();
    void copyToClip();
    void pasteFromClip();

    void add(textAndColor& info);
    void invalidateColor(DWORD bits);
    textAndColor* unformat(int pos);

    void moveUpCtrls(int first);

    int lastSel;
    bool needSort;
    void checkAlternate();
    void reduceHeight();
};
//-----------------------------------------------------------
#endif
