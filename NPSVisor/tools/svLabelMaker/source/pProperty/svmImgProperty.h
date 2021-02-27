//--------------- svmImgProperty.h -----------------------------
//-----------------------------------------------------------
#ifndef svmImgProperty_H_
#define svmImgProperty_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmProperty.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
// classe per img
// aggiunge le proprietà idImg, tipo di manipolazione (scalata o distorta)
class imgProperty : public Property
{
  private:
    typedef Property baseClass;
  public:
    enum eType { eStretched, eScaled };
    imgProperty() : idBmp(0), imgType(eStretched) {}
    virtual ~imgProperty() {}

    uint idBmp;
    uint imgType;
  protected:
    virtual void clone(const Property& other);
    virtual bool setOtherCommon(const unionProperty& uP);
};
//-----------------------------------------------------------
#endif
