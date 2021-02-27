//--------------- svmDrawProperty.h -----------------------------
//-----------------------------------------------------------
#ifndef svmDrawProperty_H_
#define svmDrawProperty_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmLineProperty.h"
//-----------------------------------------------------------
// classe base per rettangoli, ellissi
// aggiunge la proprietà di brush
class drawProperty : public lineProperty
{
  private:
    typedef lineProperty baseClass;
  public:
    drawProperty() : idBrush(1) {}
    virtual ~drawProperty() {}

    const drawProperty& operator=(const drawProperty& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    uint idBrush;
    virtual void unionBits(DWORD& capable)
    {
      baseClass::unionBits(capable);
      capable |= BIT_POS(unionProperty::eupBrush);
    }
    virtual void initUnion(unionProperty& uProp);
  protected:
    virtual void clone(const Property& other);
    virtual bool setOtherCommon(const unionProperty& uP);
};
//-----------------------------------------------------------
#endif
