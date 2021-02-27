//----------- warnObj.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef warnObj_H_
#define warnObj_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "allPages.h"
//----------------------------------------------------------------------------
class PD_WarnObj : public clientListViewPages
{
  private:
    typedef clientListViewPages baseClass;
  public:
    PD_WarnObj(PWin* parent, uint resId = IDD_LIST, HINSTANCE hinstance = 0);
    virtual ~PD_WarnObj() { destroy(); }

    virtual void setManPage(managePages* mp) { pMP = mp; clear(); customFill(); }
  protected:
    virtual void fill() {};
    void customFill();
    virtual void fillTitle();
};
//----------------------------------------------------------------------------
#endif
