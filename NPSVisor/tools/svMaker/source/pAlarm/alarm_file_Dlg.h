//------------ alarm_file_Dlg.h ----------------------------------------------
//----------------------------------------------------------------------------
#ifndef alarm_file_Dlg_H_
#define alarm_file_Dlg_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#include "baseDlg.h"
//----------------------------------------------------------------------------
#include "pAl_common.h"
//----------------------------------------------------------------------------
#define ALARM_ASSOC_EXT _T(".txa")
#define ALL_ALARM_ASSOC_EXT _T("*") ALARM_ASSOC_EXT
//----------------------------------------------------------------------------
class PD_alarm_file_Dlg : public  PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PD_alarm_file_Dlg(PWin* parent, LPTSTR file = 0, uint resId = IDD_CL_ASSOC_FILE, HINSTANCE hinstance = 0);
    virtual ~PD_alarm_file_Dlg();

    virtual bool create();

    void saveCurrData();
    void loadData();

    bool saveData();
    virtual bool saveOnExit();

    void ClearAll() { Clear(0, MAX_ADDRESSES - 1); }

    bool isDirty() const { return dirty; }
    void resetDirty() { dirty = false; }
    void setDirty() { dirty = true; }
  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void CmOk();
    bool dirty;

    HBRUSH Brush;
    HBRUSH Brush2;
    int currPos;
    int oldStat;

    void evVScrollBar(HWND child, int flags, int pos);

    void setScrollRange();

    void checkBtn();
    PVect<PBitmap*> Bmp;

    void evMouseWheel(short delta, short x, short y);

  private:
    LPTSTR Target;
    TCHAR Filename[_MAX_PATH];
    baseRowAssocFile Cfg[MAX_ADDRESSES];
    class PRow_Assoc_File* Rows[MAX_GROUP];

    bool loadFile();
    void openAssocFile();

    void Clear(int from, int to);

    void fill();

    bool isRowEmpty(uint pos);

    void setFileName(LPCTSTR file);

    static bool retFile;
};
//----------------------------------------------------------------------------
#endif
