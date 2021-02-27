//--------- TD_Choose_FilterTree.h -------------------------------------------
//----------------------------------------------------------------------------
#ifndef TD_CHOOSE_FILTERTREE_H_
#define TD_CHOOSE_FILTERTREE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "svDialog.h"
//----------------------------------------------------------------------------
#include "infoTree.h"
#include "idMsgInfoTree.h"
#include "plistbox.h"
//----------------------------------------------------------------------------
class ListBoxKey : public PListBox
{
  private:
    typedef PListBox baseClass;
  public:
    ListBoxKey (PWin* parent, int id, int len = 255, HINSTANCE hinst = 0):
      baseClass(parent, id, len, hinst)
      {}

    void changeColor(bool focus);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void EvKillFocus(HWND hWndGetFocus);
    void EvSetFocus(HWND hWndLostFocus);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class TD_Choose_FilterTree : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    TD_Choose_FilterTree(PWin* parent, vCodeType& oper, vRangeTime& date,
      uint resId = IDD_CHOOSE_INFO_TREE, HINSTANCE hinst = 0);
    virtual ~TD_Choose_FilterTree();

    virtual bool create();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();

    void BNClickedAddDate();
    void BNClickedRemDate();
    void BNClickedAddOper();
    void BNClickedAddAllOper();
    void BNClickedRemOper();
    void BNClickedRemAllOper();

    void LBNDblclkAvailable();
    void LBNDblclkChoosed();
    void LBNDblclkDate();

  private:
    vCodeType& Oper;
    vRangeTime& Date;
    class ListBoxKey* LB_Date;

    bool addToLBDate(const rangeTime& range);
    bool stringToTime(LPCTSTR buff, FILETIME& ft, bool from);
    bool stringToRange(LPTSTR buff, rangeTime& range);
    void saveTime();
    void saveOper();

    void fillLbOper();

    void SelectAll(HWND lb);
    void BNClickedMoveOper(HWND lbTarget, HWND lbSource);
};
//----------------------------------------------------------------------------
#endif
