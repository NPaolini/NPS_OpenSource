//-------------------- PModDialog.H ---------------------------
#ifndef PMODDIALOG_H_
#define PMODDIALOG_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PDIALOG_H_
  #include "PDialog.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PModDialog : public PDialog
{
  public:
    PModDialog(PWin* parent, uint id, HINSTANCE hInst = 0);
    ~PModDialog() { destroy(); }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    virtual void CmCancel();
};
//----------------------------------------------------------------------------
inline
PModDialog::PModDialog(PWin* parent, uint id, HINSTANCE hInst) :
  PDialog(parent, id, hInst) {}
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
