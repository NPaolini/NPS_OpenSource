//------- D_InfoStop.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_INFOSTOP_H_
#define D_INFOSTOP_H_
//----------------------------------------------------------------------------
#include "d_info.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class TD_Info_Stop : public TD_Info
{
  private:
    typedef TD_Info baseClass;
  public:
    TD_Info_Stop(PWin* parent, const vCodeType& oper, const vRangeTime& date,
        uint resId = IDD_INFO, HINSTANCE hinst = 0)
      : baseClass(parent, oper, date, resId, hinst) { }
  protected:
    virtual void fill();
    virtual bool fill4print();
    virtual void fillSub1(HTREEITEM parent);
    virtual void fillSub2(HTREEITEM parent);

    virtual void makeNameFromCodeMaster(LPTSTR target, size_t len, const codeType& code);
    virtual codeType useCodeOrCountMaster(const codeType& code, int count);

    virtual void makeNameFromCodeSlave(LPTSTR target, size_t len, const codeType& code);
    virtual codeType useCodeOrCountSlave(const codeType& code, int count);

    virtual void getCodeMasterAndSlave(HTREEITEM parent, codeType& master, codeType& slave);

};
//----------------------------------------------------------------------------
#endif


