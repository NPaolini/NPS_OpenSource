//-------- P_Info.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_Info_H
#define P_Info_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pmoddialog.h"

#include "resource.h"
#include "p_vect.h"
#include "pbitmap.h"
#include "p_manage_ini.h"
#include "pstatic.h"
//----------------------------------------------------------------------------
class myStatic;
class myStaticColor;
//----------------------------------------------------------------------------
class P_Info : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    P_Info(bool& force_refresh, PWin* parent, uint resId = IDD_DIALOG_INFO, HINSTANCE hinstance = 0);
    virtual ~P_Info();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    void BNClickedOk();
    void fillCtrl();
    void checkEnable();
    void chooseColor();
    void chooseFont();
    void selChange();
    void SelectImg(uint idc, LPCTSTR key);

    void addText();
    void modText();
    void delText();

    PVect<PBitmap*> Bmp;
    COLORREF TextColor;
    myStatic* curr;
    myStaticColor* color;
    PVect<uint> modifRow;
    bool& forceRefresh;
};
//----------------------------------------------------------------------------
#endif
