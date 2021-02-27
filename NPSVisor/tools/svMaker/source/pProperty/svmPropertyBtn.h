//-------------------- svmPropertyBtn.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYBTN_H_
#define SVMPROPERTYBTN_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjButton.h"
#include "POwnBtn.h"
//-----------------------------------------------------------
struct fullPrph
{
  UINT prph;
  UINT addr;
  UINT typeVal;
//  UINT nDec;
  UINT nBits;
  UINT offset;
  int normaliz;
  fullPrph() : prph(0), addr(0), typeVal(0), /*nDec(0),*/ nBits(0), offset(0), normaliz(0) {}
};
//-----------------------------------------------------------
void saveFullPrph(P_File& pf, LPTSTR buff, int id, const fullPrph& data);
void loadFullPrph(uint id, setOfString& set, fullPrph& data);
//-----------------------------------------------------------
class PropertyBtn : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyBtn();
    virtual ~PropertyBtn();

    const PropertyBtn& operator=(const PropertyBtn& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);
    const PVect<LPCTSTR>& getNames() const { return nameBmp; }
    PVect<LPCTSTR>& getNames() { return nameBmp; }
  void fillNameBmp();
    COLORREF fgPress;
    COLORREF bgPress;
    LPCTSTR functionLink;
    LPCTSTR normalText;
    LPCTSTR pressedText;

    LPCTSTR modelessName;
//---- nuovi --------------
    PVect<LPCTSTR> allText; // ci vengono messi anche normal e pressed come primi due valori
    PVect<double> textVal;

    PVect<COLORREF> otherFg;
    PVect<COLORREF> otherBg;
    PVect<double> colorVal;
    PVect<double> bmpVal;

    union {
      struct {
        DWORD theme : 1;
        DWORD flat : 1;
        DWORD fixedBmpDim : 1; // non più usato, lasciato per compatibilità
        DWORD pos   : 4;
        // i valori seguenti possono essere: 0 = non usa var, 1 = usa valore, 2 = usa indice, 3 = valore esatto (solo per i colori)
        DWORD colorByVar : 3;
        DWORD textByVar : 3;
        DWORD bitmapByVar : 3;
        DWORD noBorder : 1;
        DWORD styleBmpDim : 2; // 0 -> fisso, 1 -> scalato, 2 -> riempie tutto il pulsante (valido solo per testo/bmp centrato)
        };
      DWORD flag;
      } Flags;

    // 0-> colore, 1 -> testo, 2 -> bmp
    fullPrph DataPrf[3];
//-------------------------
  protected:
    virtual void clone(const Property& other);

  private:
    PropertyBtn(const PropertyBtn& other);
    PVect<LPCTSTR> nameBmp;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
enum btnStyleShow {
  btnS_OnlyBmp,
  btnS_BmpAndText,
  btnS_StdAndBmp,
  btnS_NewStdAndBmp,
  };
//-----------------------------------------------------------
enum btnStyleAction {
  btnAction,
  btnOnOff,
  btnFirstGroup,
  btnNextGroup,
  btnPressing,
  btnHide,
  btnModeless,
  btnOpenPageByBit,
  };
//-----------------------------------------------------------
class svmDialogBtn : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogBtn(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_BTN_PROPERTY, HINSTANCE hInst = 0);
    ~svmDialogBtn()    {   destroy();  delete Bmp; }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fillType();
    void chooseBmp();
    bool checkZeroAction();
    void checkEditLink();
    bool checkNormalAction();
    bool checkhide();
    void checkEnabled(bool all);
    void enableModeless(int enable);
    void findModelessName();
    virtual bool useFont() const { return true; }

    void personalize();
    void allocBmp();
//    void checkHeight();

    PBitmap* Bmp;
    bool onLink;

    struct colorBtn
    {
        POwnBtn* Btn;
        COLORREF color;

        colorBtn() : Btn(0), color(0) {}
    };
    colorBtn ColorBtn[4];
    void chooseColor(int ix);
    void invalidateColor(int ix);

    void chooseAction();

    int lastAction;
};
//-----------------------------------------------------------
#endif
