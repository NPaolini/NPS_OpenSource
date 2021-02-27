//-------- P_Setup.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_Setup_H
#define P_Setup_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pdialog.h"

#include "resource.h"
#include "p_vect.h"
#include "pbitmap.h"
#include "p_manage_ini.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class P_Setup : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    P_Setup(PWin* parent, uint resId = IDD_DIALOG_SETUP, HINSTANCE hinstance = 0);
    virtual ~P_Setup();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    void BNClickedOk();
    void fillCtrl();
    void SelectSoundFolder(uint idc, LPCTSTR key);
    void SelectImgFolder();
    bool compareAndSave(LPCTSTR keyname, uint idc, uint typePath);
    PVect<PBitmap*> Bmp;
};
//----------------------------------------------------------------------------
#endif
