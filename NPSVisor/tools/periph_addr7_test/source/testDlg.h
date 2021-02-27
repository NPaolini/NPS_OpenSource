//----------- testDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef TESTDLG_H_
#define TESTDLG_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#ifndef PDIALOG_H_
#include "pDialog.h"
#endif
//----------------------------------------------------------------------------
#include "common.h"
//----------------------------------------------------------------------------
class PD_Test : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PD_Test(PWin* parent, uint idAdr, uint resId = IDD_CL_SVTEST, HINSTANCE hinstance = 0);
    virtual ~PD_Test();

    virtual bool create();

    void setId(uint id) { idAdr = id; }
    uint getId() { return idAdr; }
    void loadByIdAdr();

    bool isHex();
    void loadData();
    void saveCurrData();

    void getInfo(commonInfo& ci);
    void setInfo(const commonInfo& ci);
    uint getNByte() { return nByteBase; }
  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    uint idAdr;

    int idFocus;
    void checkRowWithFocus(uint id);
  private:
    HBRUSH Brush;
    HBRUSH Brush2;
    HBRUSH Brush3;

    UINT WM_PRPH;

    class PRowTest* Rows[MAX_GROUP];
    int currPos;
    int oldStat;
    uint nByteBase;

    DWORD Values[MAX_ADDRESSES];

    uint baseAddr;
    void setBaseAddr(uint idCtrl);

    bool saveData();

    void saveFile();
    void loadFile();

    void evVScrollBar(HWND child, int flags, int pos);

    void changeBase();

    DWORD maxShow;
    void changeNumRow();
    void checkNumRow(DWORD nrow);
    void setScrollRange();

    class PBitmap* Bmp[5];

    bool orderSet(class orderedPAddr& set);

    bool isRowEmpty(uint pos);
    bool hasValidZeroValue(HWND hWndChild);

    bool autoRefresh;

    void changeValue();
    bool noConvert;
    void adviseSVisor();

    void evMouseWheel(short delta, short x, short y);

    void searchSVisor(bool req = 0, LPCTSTR running = 0);
    void performSearchSVisor(bool req, LPCTSTR running);

    void checkSVisor();
    void searchText();

};
//----------------------------------------------------------------------------
#endif
