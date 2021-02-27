//------------ baseDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef baseDlg_H_
#define baseDlg_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#ifndef PDIALOG_H_
#include "pDialog.h"
#endif
//----------------------------------------------------------------------------
#include "pAl_common.h"
//----------------------------------------------------------------------------
class PD_Base : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PD_Base(PWin* parent, uint idPrph, uint resId, HINSTANCE hinstance = 0);
    virtual ~PD_Base();

    bool create();
    void setId(uint id) { idPrph = id; }
    uint getId() { return idPrph; }

    virtual void saveCurrData() = 0;
    virtual void loadData() = 0;

    void getInfo(commonInfo& ci);
    void setInfo(const commonInfo& ci);

    bool isDirty() const { return dirty; }
    void resetDirty() { dirty = false; }
    void setDirty() { dirty = true; }
    virtual bool saveData() = 0;
    virtual bool saveOnExit();

    void ClearAll() { Clear(0, MAX_ADDRESSES - 1); }

  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    uint idPrph;
    bool dirty;

//  private:
    HBRUSH Brush;
    HBRUSH Brush2;
    int currPos;
    int oldStat;

    void evVScrollBar(HWND child, int flags, int pos);

    virtual void Clear(int from, int to) = 0;

    virtual void fill() = 0;
    virtual void searchText() = 0;
    virtual void Copy() = 0;

    void setScrollRange();

    void checkBtn();
    PVect<PBitmap*> Bmp;

    virtual bool isRowEmpty(uint pos) = 0;

    void evMouseWheel(short delta, short x, short y);

};
//----------------------------------------------------------------------------
#endif
