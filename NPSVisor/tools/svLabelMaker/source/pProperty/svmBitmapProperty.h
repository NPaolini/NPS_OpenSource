//--------------- svmBitmapProperty.h -----------------------------
//-----------------------------------------------------------
#ifndef svmBitmapProperty_H_
#define svmBitmapProperty_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmProperty.h"
//-----------------------------------------------------------
// classe base per rettangoli, ellissi
// aggiunge la proprietà di brush
class bitmapProperty : public Property
{
  private:
    typedef Property baseClass;
  public:
    bitmapProperty() : idBmp(0) {}
    virtual ~bitmapProperty() {}

    const bitmapProperty& operator=(const bitmapProperty& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    uint idBmp;
    virtual void unionBits(DWORD& capable)
    {
      baseClass::unionBits(capable);
    }
  protected:
    virtual void clone(const Property& other);
    virtual bool setOtherCommon(const unionProperty& uP);
};
//-----------------------------------------------------------
#endif
