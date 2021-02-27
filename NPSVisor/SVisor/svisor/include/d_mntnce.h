//---------- d_mntnce.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_MNTNCE_H_
#define D_MNTNCE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pVarListBox.h"

#include "pvaredit.h"

#ifndef MNTNCE_H_
  #include "mntnce.h"
#endif

#include "p_defbody.h"
#include "setPack.h"
//----------------------------------------------------------------------------
class TD_Maintenance : public P_DefBody {
  public:
    TD_Maintenance(int idPar, PWin* parent, uint resId = IDD_MANUTENZ, HINSTANCE hinst = 0);
    virtual ~TD_Maintenance();

    virtual bool create();
    P_Body* pushedBtn(int idBtn);
    virtual void setReady(bool first);

    // non deve visualizzare alcun errore, i campi di edit sono gestiti internamente
    virtual void ShowErrorData(uint idprf, const prfData& data, prfData::tResultData result) {  }

  protected:
    void evPaint(HDC hdc, const PRect& rect);
    HBRUSH evCtlColor(HDC dc, HWND hWndChild, uint ctlType);
    virtual void refreshBody();

    virtual PVarListBox* allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen = 255, HINSTANCE hinst = 0);

    virtual void getFileStr(LPTSTR path);

    // non vengono più creati ma puntato agli oggetti di base
    PVarListBox* LB; // LBox[0]
    PVarEdit *info;  // Edi[0]
    PVarEdit *ore;   // Edi[1]
//    HFONT fEdit;
//    HFONT Font;
    g_maint fgm[MAX_MAINT];
    gest_maint *G_M;
    int currId;
    int currSel;
    void load();
    void modify();
    void save();
    void threadSet();
    void Reset();

  private:
    void fillLb();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void ENKillfocusOre();

  private:
    typedef P_DefBody baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

