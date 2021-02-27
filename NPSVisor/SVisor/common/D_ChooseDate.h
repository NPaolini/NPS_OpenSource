//------ d_choosedate.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_CHOOSEDATA_H_
#define D_CHOOSEDATA_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "plistbox.h"
#include "pstatic.h"
#include "svEdit.h"
#include "p_Vect.h"
#include "svDialog.h"
//----------------------------------------------------------------------------
#define MAX_BTN_CHOOSE 4
//----------------------------------------------------------------------------
//----------------------------------------------------------
struct rangeTime
{
  FILETIME from;
  FILETIME to;

  rangeTime(FILETIME from, FILETIME to) :
    from(from), to(to) {}
  rangeTime() {}
};
//----------------------------------------------------------------------------
class TD_ChooseDate : public svDialog
{
  public:
    TD_ChooseDate(PWin* parent, PVect<rangeTime>& range, uint resId = IDD_CHOOSE_DATE_EXPORT);
    virtual ~TD_ChooseDate();

  public:
    virtual bool create();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void BNClicked_F1();
    void BNClicked_F2();
    void BNClicked_F3();
    void BNClicked_F4();
    void LB_SelChange();

  private:
    PStatic* Info[MAX_BTN_CHOOSE];
    class PListBox* LB_Choose;
    svEdit* From;
    svEdit* To;

    void enableBtn(int id, bool enable);
    bool isEnableBtn(int id);
    bool addToLB(const rangeTime& range);
    bool stringToTime(LPCTSTR buff, FILETIME& ft, bool from);
    bool stringToRange(LPTSTR buff, rangeTime& range);

    PVect<rangeTime>& Range;
    typedef svDialog baseClass;
};

#endif

