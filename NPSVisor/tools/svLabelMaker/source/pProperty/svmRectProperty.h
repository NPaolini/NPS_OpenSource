//--------------- svmRectProperty.h -----------------------------
//-----------------------------------------------------------
#ifndef svmRectProperty_H_
#define svmRectProperty_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmDrawProperty.h"
//-----------------------------------------------------------
// classe per rettangoli
// aggiunge due valori per bordi arrotondati
class rectProperty : public drawProperty
{
  private:
    typedef drawProperty baseClass;
  public:
    rectProperty() : cX(0), cY(0) {}
    virtual ~rectProperty() {}

    const rectProperty& operator=(const rectProperty& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    uint cX;
    uint cY;
    virtual void unionBits(DWORD& capable)
    {
      baseClass::unionBits(capable);
      capable |= BIT_POS(unionProperty::eupCX) | BIT_POS(unionProperty::eupCY);
    }
    virtual void initUnion(unionProperty& uProp);
  protected:
    virtual void clone(const Property& other);
    virtual bool setOtherCommon(const unionProperty& uP);
};
//-----------------------------------------------------------
#endif
