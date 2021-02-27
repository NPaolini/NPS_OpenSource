//---------- saveData.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef SAVEDATA_H_
#define SAVEDATA_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PLISTBOX_H_
  #include "plistbox.h"
#endif

#include "p_defbody.h"
//----------------------------------------------------------------------------
class saveData : public P_DefBody {
  public:
    saveData(int idPar, PWin* parent, uint resId = IDD_STANDARD, HINSTANCE hinst = 0);
    virtual ~saveData();

    virtual void setReady(bool first);

    virtual bool create();
    P_Body* pushedBtn(int idBtn);

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void getFileStr(LPTSTR path);
    virtual PVarListBox* allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen = 255, HINSTANCE hinst = 0);

  private:
    void fillLb();
    void fillLb(LPCTSTR file);

    void addRow(LPTSTR row);
    void actionByListboxSave();
    void actionByListboxUnsave();
    void handleLBDblClick(HWND hwnd);
    void handleSelChange(HWND hwnd);

    bool chooseFile();
    LPTSTR addData(LPTSTR buff, uint ix);
    LPTSTR remData(LPTSTR buff, uint ix);
    void saveAll();

  private:
    typedef P_DefBody baseClass;
};
//----------------------------------------------------------------------------
#endif

