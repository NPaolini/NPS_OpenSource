//--------- TD_Choose_PrintTree.h -------------------------------------------
//----------------------------------------------------------------------------
#ifndef TD_CHOOSE_PRINTTREE_H_
#define TD_CHOOSE_PRINTTREE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "svDialog.h"
//----------------------------------------------------------------------------
#include "infoTree.h"
#include "idMsgInfoTree.h"
//----------------------------------------------------------------------------
class TD_Choose_PrintTree : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    TD_Choose_PrintTree(PWin* parent, vCodeType& oper, vRangeTime& date,
      uint resId = IDD_CHOOSE_PRINT, HINSTANCE hinst = 0);
    virtual ~TD_Choose_PrintTree();

    virtual bool create();

  protected:
    void CmOk();

    void LBNDblclkAvailable();
    void checkEnable();

  private:
    vCodeType& Oper;
    vRangeTime& Date;

    bool stringToTime(LPCTSTR date, LPCTSTR time, FILETIME& ft, bool from);
    bool saveTime();
    void saveOper(bool filter);

    void fillLbOper();
    static TCHAR bDate[2][20];
    static TCHAR bTime[2][20];
};
//----------------------------------------------------------------------------
#endif
