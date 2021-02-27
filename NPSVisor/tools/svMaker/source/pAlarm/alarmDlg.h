//------------ alarmDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef alarmDlg_H_
#define alarmDlg_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#include "baseDlg.h"
//----------------------------------------------------------------------------
#include "pAl_common.h"
//----------------------------------------------------------------------------
class PD_Alarm : public PD_Base
{
  private:
    typedef PD_Base baseClass;
  public:
    PD_Alarm(PWin* parent, uint idPrph, uint resId = IDD_CL_ALARM, HINSTANCE hinstance = 0);
    virtual ~PD_Alarm();

    virtual bool create();

    const baseRowAlarm* getRows() { return Cfg; }

    void saveCurrData();
    void loadData();

    bool saveData();
    virtual bool saveOnExit();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void Clear(int from, int to);
    void Copy();
    void searchText();
    void fill();
    void copyFromClipboard();
    void fillRowByClipboard(LPTSTR lptstr);
  private:
    TCHAR Filename[_MAX_PATH];
    baseRowAlarm Cfg[MAX_ADDRESSES];
    class PRow_Alarm* Rows[MAX_GROUP];

    bool loadFile();

    bool isRowEmpty(uint pos);

    void setFileName();

    void loadAssocFile(long id, setOfString& Set, setOfString& Set2, baseRowAssoc* aRows, LPCTSTR p);
};
//----------------------------------------------------------------------------
#endif
