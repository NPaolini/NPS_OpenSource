//--------------- svmLineProperty.h -----------------------------
//-----------------------------------------------------------
#ifndef svmLineProperty_H_
#define svmLineProperty_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmProperty.h"
//-----------------------------------------------------------
// classe base per primitive di base, linee, rettangoli, ellissi
// aggiunge le proprietà di pen e un valore per direzione della
// linea (per non usare dimensioni negative nel rect)
class lineProperty : public Property
{
  private:
    typedef Property baseClass;
  public:
    enum elDir { edTopLeft, edTopRight /*, edBottomLeft, edBottomRight */ };
    lineProperty() : dir(edTopLeft), idPen(1) {}
    virtual ~lineProperty() {}

    const lineProperty& operator=(const lineProperty& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    virtual void unionBits(DWORD& capable)
    {
      baseClass::unionBits(capable);
      capable |= BIT_POS(unionProperty::eupPen);
    }
    virtual void initUnion(unionProperty& uProp);
    uint dir;
    uint idPen;
   protected:
    virtual void clone(const Property& other);
    virtual bool setOtherCommon(const unionProperty& uP);
};
//-----------------------------------------------------------
#endif
