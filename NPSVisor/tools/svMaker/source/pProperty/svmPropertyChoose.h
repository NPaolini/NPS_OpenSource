//-------------------- svmPropertyChoose.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYCHOOSE_H_
#define SVMPROPERTYCHOOSE_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjChoose.h"
//-----------------------------------------------------------
#include "svmPropertyText.h"
//-----------------------------------------------------------
#define MAX_LB_COLORS 4
//-----------------------------------------------------------
class chooseText
{
  public:
    chooseText() : text(0) {  }
    chooseText(LPCTSTR init) : text(0) { setText(init); }
    chooseText(const chooseText& other) : text(0) { clone(other); }
    ~chooseText() { delete []text; }

    LPCTSTR getText() const { return text; }
    void setText(LPCTSTR txt);
    const chooseText& operator =(const chooseText& other) { clone(other); return *this; }
    operator LPCTSTR() const { return text; }
  private:
    void clone(const chooseText& other);
    LPCTSTR text;
};
//-----------------------------------------------------------
typedef PVect<chooseText*> setOfPText;
//-----------------------------------------------------------
class PropertyChoose : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyChoose();
    virtual ~PropertyChoose() {  flushPV(TextColor); flushPV(Text); }

    LPCTSTR getText() const { return *Text[0]; }
    setOfPText& getSet() { return Text; }
    const setOfPText& getSet() const { return Text; }

    setOfPTextColor& getSetColor() { return TextColor; }
    const setOfPTextColor& getSetColor() const { return TextColor; }

    const PropertyChoose& operator=(const PropertyChoose& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    COLORREF lbColors[MAX_LB_COLORS];
    bool usingColor() const { return useColor; }
    void setUseColor(bool use) { useColor = use; }

    bool usingCustomSend() const { return useCustomSend; }
    void setUseCustomSend(bool use) { useCustomSend = use; }

    alternateTextSet& getAltObj() { return altText; }
    const alternateTextSet& getAltObj() const { return altText; }
  protected:
    virtual void clone(const Property& other);

  private:
    PropertyChoose(const PropertyChoose& other);

    setOfPText Text;

    setOfPTextColor TextColor;
    bool useColor;
    bool useCustomSend;
    alternateTextSet altText;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogChoose : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogChoose(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_CHOOSE_PROPERTY, HINSTANCE hInst = 0);
    ~svmDialogChoose()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void chooseText();
    void chooseTextAndColor();
    void checkEnable();
    virtual bool useFont() const { return true; }
};
//-----------------------------------------------------------
class svmChooseText : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmChooseText(PropertyChoose* prop, PWin* parent, uint id = IDD_CHOOSE_TEXT,
          HINSTANCE hInst = 0);
    ~svmChooseText();

    virtual bool create();
  protected:
    PropertyChoose* Prop;
    class PListBox* LB;
    bool enlarged;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fill();
    void add();
    void rem();
    void mod();
    void move(bool up);
    void copyToClip();
    void pasteFromClip();

    void add(const chooseText& info, int sel = -1);
    void chooseColor(int ix);
    int selectedForMod;
    void enableForMod(bool enable);
    void checkAlternate();
};
//-----------------------------------------------------------
#endif
