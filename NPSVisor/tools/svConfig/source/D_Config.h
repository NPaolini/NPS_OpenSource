//--------- d_config.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_CONFIG_H_
#define D_CONFIG_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pcombobox.h"
#include "pedit.h"
#include "pradiobtn.h"
#include "pcheckbox.h"
#include "pmodDialog.h"
#include "resource.h"
//----------------------------------------------------------------------------
#include "p_txt.h"
//----------------------------------------------------------------------------
#define CFG_PERIF 8
//----------------------------------------------------------------------------
class TD_Config : public PModDialog
{
  public:
    TD_Config(LPCTSTR normfile, struct cfg *Cfg, PWin* parent, int resId = IDD_CONFIG, HINSTANCE hinst = 0);
    virtual ~TD_Config();

    virtual bool create();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void BNClickedParam(int prf);
    void BNClickedParam_Perif1();
    void BNClickedParam_Perif2();
    void BNClickedParam_Perif3();
    void BNClickedParam_Perif4();
    void BNClickedParam_Perif5();
    void BNClickedParam_Perif6();
    void BNClickedParam_Perif7();
    void BNClickedParam_Perif8();
    void BNClickedNormaliz();
    void BNClickedUseMouse();
    void BNClickedTurni();
    void BNClickedOK();
    void fillLanguage();
    void BNClickedChgPsw();
    void BNClickedSavePsw();
    void BNClickedShowPsw();

    void tranferLanguage(LPCTSTR source);

    PCheckBox* disableDLL;
    PCheckBox* UseMouse;
    PCheckBox* ShutDown;
    PCheckBox* DisablePoweroff;
    PCheckBox* numLock;
    PCheckBox* useCard;

    PCheckBox* taskBar;
    PCheckBox* AltKey;
    PCheckBox* CtrlKey;
    PCheckBox* WinKey;
    PCheckBox* hideShowDriver;

    PCheckBox* CB_Perif[CFG_PERIF];
    PComboBox* CBX_Perif[CFG_PERIF];
    PComboBox* Prg_Perif[CFG_PERIF];
    PEdit* OffsPrf[CFG_PERIF];

    struct cfg *Cfg;
    LPCTSTR normFile;
    setOfString StrsSet;
  private:
    typedef PModDialog baseClass;
};
//----------------------------------------------------------------------------
#endif

