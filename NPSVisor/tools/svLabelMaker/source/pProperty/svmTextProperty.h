//--------------- svmTextProperty.h -----------------------------
//-----------------------------------------------------------
#ifndef svmTextProperty_H_
#define svmTextProperty_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmProperty.h"
//-----------------------------------------------------------
// classe per text
// aggiunge le proprietà colore foreground e background, text, font e tipo del
// testo (da variabile o fisso, in linea o in riquadro, multilinea)
class textProperty : public Property
{
  private:
    typedef Property baseClass;
  public:
    enum tAlign { taTop, taLeft = taTop, taCenter, taBottom, taRight = taBottom };
    enum eType { etStatic, etVar, etOnBox };
    textProperty() :
        Text(0), foreground(cBLACK), background(cWHITE), hAlign(taLeft), vAlign(taTop),
        textType(etStatic), Angle(0), idVar(0)
    {}
    virtual ~textProperty() { delete []Text; }

    const textProperty& operator=(const textProperty& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    LPCTSTR Text;
    COLORREF foreground; // text
    COLORREF background;
    uint hAlign;
    uint vAlign;
    enum eTxtType {
      eVariable = 1 << 0,   // zero => semplice, 1 => da variabile
      eboxed = 1 << 1,      // zero => libero, 1 => in box
      eMultiline = 1 << 2   // zero => singola, 1 => multilinea
      };
    uint textType;
    int Angle;
    uint idVar;
    virtual void unionBits(DWORD& capable)
    {
      baseClass::unionBits(capable);
      capable |= BIT_POS(unionProperty::eupFG) | BIT_POS(unionProperty::eupFont) |
        BIT_POS(unionProperty::eupHAlign) | BIT_POS(unionProperty::eupVAlign);
    }
    virtual void initUnion(unionProperty& uProp);
    HFONT getFont(bool linked = false);
    void setText(LPCTSTR newTxt);
    void getVarText(LPTSTR buff, size_t sz);
  protected:
    virtual void clone(const Property& other);
    virtual bool setOtherCommon(const unionProperty& uP);
};
//-----------------------------------------------------------
#endif
