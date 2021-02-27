//------------ adrDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef ADRDLG_H_
#define ADRDLG_H_
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
class PD_Adr : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PD_Adr(PWin* parent, uint idAdr, uint resId = IDD_CL_ADR, HINSTANCE hinstance = 0);
    virtual ~PD_Adr();

    virtual bool create();

    void setId(uint id) { idAdr = id; }
    uint getId() { return idAdr; }

    void setTitle();

    baseRow* getRows() { return Cfg; }

    bool isHex();
    void saveCurrData();
    void loadData();

    void getInfo(commonInfo& ci);
    void setInfo(const commonInfo& ci);

    bool isDirty() const { return dirty; }
    bool saveData();
    bool saveOnExit();
    uint getNByte();
    LPCTSTR getFilename() const { return Filename; }
    void checkExternalChange();

    void set_Dirty(bool reload = true) { setDirty(true); if(reload) loadData(); }
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
    TCHAR Filename[_MAX_PATH];
    baseRow Cfg[MAX_ADDRESSES];
    class PRow* Rows[MAX_GROUP];
    int currPos;
    int oldStat;
    int currPrphCopyFrom;
    bool enablePrphCopyFrom;

    FILETIME lastModified;
    P_File* lockFile;

    bool isCrypted();
    void setCrypted(bool set);


    uint baseAddr;
    void setBaseAddr(uint idCtrl);

    void openFile();
    bool loadFile();
    void evVScrollBar(HWND child, int flags, int pos);

    bool dirty;
    void setDirty(bool set);

    void Clear();
    void Clear(int from, int to);

    void fill();
    void copy();
    void copyFromClipboard();
    void fillRowByClipboard(LPTSTR lptstr);

    void changeBase();

    bool noConvert;
    void changeValue();
    DWORD maxShow;
    void changeNumRow();
    void setScrollRange();

    void checkBtn();

    void verifyAll();
    bool orderSet(class orderedPAddr& set);

    bool isRowEmpty(uint pos);
    bool hasValidZeroValue(HWND hWndChild);

    void setFileName();
    void setPrevFileName();

    void evMouseWheel(short delta, short x, short y);
    bool onOpen;
    void searchText();
};
//----------------------------------------------------------------------------
#endif
