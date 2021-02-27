//--------- d_psw.h ----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_PSW_H_
#define D_PSW_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef SVDIALOG_H_
  #include "svdialog.h"
#endif
#include "svEdit.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
#define MAX_LEN_PSW 12
//----------------------------------------------------------------------------
// viene memorizzato il puntatore a [buff].
// [buff] deve avere dimensione di almeno MAX_LEN_PSW caratteri
class TD_Psw : public svDialog {
  public:
    TD_Psw(char *buff, PWin* parent, uint resId = IDD_PSW, HINSTANCE hinst = 0);
    virtual ~TD_Psw();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
  private:
    svEdit* E_Psw;
    char *Buff;
    void BNClickedOk();
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

