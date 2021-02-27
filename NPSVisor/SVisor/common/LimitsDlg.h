//----------- LimitsDlg.h ----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef LIMITSDLG_H_
#define LIMITSDLG_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#ifndef SVDIALOG_H_
#include "svDialog.h"
#endif
//#include "lgraph2.h"
#include "Commonlgraph.h"
//----------------------------------------------------------------------------
#define MAX_TEXT MAX_DESCR
//----------------------------------------------------------------------------
#define MAX_GROUP 10
#define MAX_ADDRESSES MAX_SET
//----------------------------------------------------------------------------
struct baseRowLimit
{
  fREALDATA vMin;
  fREALDATA vMax;
  TCHAR text[MAX_TEXT + 1];
  baseRowLimit() : vMin(0), vMax(0) { ZeroMemory(text, sizeof(text)); }
};
//----------------------------------------------------------------------------
#define wDIALOG svDialog
class PD_Limits : public wDIALOG
{
  public:
    PD_Limits(LPCTSTR filename, setOfString& sos, PWin* parent, uint resId = IDD_LGRAPH2_LIMITS, HINSTANCE hinstance = 0);
    virtual ~PD_Limits();

    virtual bool create();

  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    HBRUSH Brush;
    HBRUSH Brush2;
    rangeSet rSet;

    bool scrollByEdit;

    baseRowLimit Cfg[MAX_ADDRESSES];
    class PRow_ld* Rows[MAX_GROUP];
    int currPos;

    int maxShow;

    bool saveData();
    void saveCurrData();
    void loadData();
    void evVScrollBar(HWND child, int flags, int pos);

    void setTitle();
    bool dirty;

    bool saveOnExit();

    void setScrollRange();

    void checkBtn();
    class PBitmap* Bmp[2];

    LPCTSTR FileName;
    setOfString& SOS;

    typedef wDIALOG baseClass;
};
//----------------------------------------------------------------------------
#endif
