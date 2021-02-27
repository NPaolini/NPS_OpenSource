//------- dturnovr.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DTURNOVR_h_
#define DTURNOVR_h_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef SVDIALOG_H_
#include "svdialog.h"
#endif

#ifndef PLISTBOX_H_
#include "plistbox.h"
#endif

#include "svEdit.h"

#ifndef PCOMBOBOX_H_
#include "pcombobox.h"
#endif

#include "setPack.h"

#define DIM_ID 1
#define DIM_ORA 5
#define DIM_INFO 21
#define MAX_TURN 9
struct lb_data
{
  TCHAR id[DIM_ID + 1];
  TCHAR da[DIM_ORA + 1];
  TCHAR a[DIM_ORA + 1];
  TCHAR info[DIM_INFO + 1];
};

class turnOver
{
  public:
    turnOver();
    const lb_data& get(int numTurn) { return tData[numTurn]; }
    int numTurn() { return NumTurn; }
    void setup(PWin*);
  protected:
    static int NumTurn;
    static lb_data tData[MAX_TURN];
    static bool updated;
};
//----------------------------------------------------------------------------
class TD_TurnOver : public svDialog
{
  public:
    TD_TurnOver(int& n_turn, lb_data *t, PWin* parent, uint resId = IDD_TURN_OVER, HINSTANCE hinst = 0);
    virtual ~TD_TurnOver();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void BNClickedAdd();
    void BNClickedDel();
    void BNClickedMod();
    void LBNDblclk();
    void BNClickedOK();

  private:
    PComboBox* CBX_type;
    svEdit* AOre;
    svEdit* DaOre;
    svEdit* IdTurn;
    PListBox* LB_Turn;

    int& nTurn;
    lb_data *Turn;
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

