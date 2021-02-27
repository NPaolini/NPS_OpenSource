//------- d_choose.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_CHOOSE_H_
#define D_CHOOSE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pstatic.h"
#include "plistbox.h"
#include "p_txt.h"
#include "svDialog.h"
//----------------------------------------------------------------------------
#define MAX_CHOOSE 10
#define MAX_BTN_CHOOSE 6
//----------------------------------------------------------------------------
/*
enum initCodChoose {
        INIT_SPEED = 1,
        INIT_POWER = INIT_SPEED + STAT_MAX_SPEED,
        INIT_TEMP  = INIT_POWER + STAT_MAX_POWER,
        INIT_PASSO = INIT_TEMP  + STAT_MAX_OTHER_VAL,
        INIT_TIRO  = INIT_PASSO + STAT_MAX_OTHER_VAL,
        INIT_DATI  = INIT_TIRO + STAT_MAX_OTHER_VAL,
        };
enum endCodChoose {
        END_SPEED = INIT_POWER,
        END_POWER = INIT_TEMP,
        END_TEMP  = INIT_PASSO,
        END_PASSO = INIT_TIRO,
        END_TIRO  = INIT_DATI,
        END_DATI  = INIT_DATI + STAT_MAX_OTHER_VAL,
        };
*/
class TD_Choose : public svDialog {
  public:
    TD_Choose(PWin* parent, setOfString& sos, int *last_choose, uint resId = IDD_CHOOSE_GRAPH);
    virtual ~TD_Choose();

    virtual bool create();

  protected:
    void BNClicked_F1();
    void BNClicked_F2();
    void BNClicked_F3();
    void BNClicked_F4();
    void BNClicked_F5();
    void BNClicked_F6();
    void LB_SelChange();
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    void CmOk() {}
//    void CmCancel() {}


  private:
    PStatic* Info[MAX_BTN_CHOOSE];
    class PListBox* LB_Choose;
    int *lastChoose;
    void fill_LB();
    void enableBtn(int id, bool enable);
    bool isEnableBtn(int id);
    bool addToLB(int code);
    setOfString& sStr;
  private:
    typedef svDialog baseClass;
};

#endif
