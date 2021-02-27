//------- D_InfoOper.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_INFOOPER_H_
#define D_INFOOPER_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "d_info.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class TD_Info_Oper : public TD_Info
{
  private:
    typedef TD_Info baseClass;
  public:
    TD_Info_Oper(PWin* parent, const vCodeType& oper, const vRangeTime& date,
        uint resId = IDD_INFO, HINSTANCE hinst = 0)
      : baseClass(parent, oper, date, resId, hinst) { }
  protected:
    virtual void fill();
    virtual bool fill4print();
    void fillSub1(HTREEITEM parent);
    void fillSub2(HTREEITEM parent);

    virtual void makeNameFromCodeMaster(LPTSTR target, size_t len, const codeType& code);
    virtual codeType useCodeOrCountMaster(const codeType& code, int count);

    virtual void makeNameFromCodeSlave(LPTSTR target, size_t len, const codeType& code);
    virtual codeType useCodeOrCountSlave(const codeType& code, int count);

    virtual void getCodeMasterAndSlave(HTREEITEM parent, codeType& master, codeType& slave);
};
//----------------------------------------------------------------------------
#endif


