//------- dsettime.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DSETTIME_H_
#define DSETTIME_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef SVDIALOG_H_
#include "svDialog.h"
#endif

#include "svEdit.h"

#ifndef PSTATIC_H_
#include "pStatic.h"
#endif

#include "setPack.h"
//----------------------------------------------------------------------------
class TD_SetTime : public svDialog
{
  public:
    TD_SetTime(PWin* parent, int &hour, int& minute, int id_caption = 0, uint resId = IDD_SET_MAINT_TIME, HINSTANCE hinstance = 0);
    virtual ~TD_SetTime();
  private:
    int& Hour;
    int& Minute;
    int idCaption;
  public:
    virtual bool create();
    void CmOk();

  protected:
    svEdit* minute_;
    svEdit* hour_;
    PStatic* txtMinute;
    PStatic* txtHour;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

