//------ dinputdt.h ----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DINPUTDT_H_
#define DINPUTDT_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PSTATIC_H_
  #include "pstatic.h"
#endif

#include "svEdit.h"

#ifndef SVDIALOG_H_
  #include "svdialog.h"
#endif
#include "setPack.h"
//----------------------------------------------------------------------------
class TD_InputDate : public svDialog
{
  public:
    TD_InputDate(FILETIME &ft1, FILETIME &ft2, PWin* parent, uint resId = IDD_INPUT_DATE, HINSTANCE hinst = 0);
    virtual ~TD_InputDate();

    virtual bool create();

  protected:
    svEdit* From;
    svEdit* To;
    svEdit* YearFrom;
    svEdit* YearTo;
    PStatic* TxtTo;
    PStatic* TxtFrom;
    svEdit* timeFrom;
    svEdit* timeTo;

    HBRUSH evCtlColor(HDC dc, HWND hWndChild, uint ctlType);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//  private:
    FILETIME &fT1;
    FILETIME &fT2;
  private:
    typedef svDialog baseClass;

};
//----------------------------------------------------------------------------
class TD_InputDateOnlyDay : public TD_InputDate
{
  public:
    TD_InputDateOnlyDay(FILETIME &ft1, FILETIME &ft2, PWin* parent, uint resId = IDD_INPUT_DATE, HINSTANCE hinst = 0);
    virtual ~TD_InputDateOnlyDay();

    virtual bool create();
  protected:
    virtual void BNClickedF1();
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    typedef TD_InputDate baseClass;
};
//----------------------------------------------------------------------------
class TD_InputTime : public svDialog
{
  public:
    TD_InputTime(FILETIME &ft1, FILETIME &ft2, PWin* parent, uint resId = IDD_INPUT_TIME, HINSTANCE hinst = 0);
    virtual ~TD_InputTime();

    virtual bool create();

  protected:
    PStatic* TxtTo;
    PStatic* TxtFrom;
    PStatic* dateInfo;
    svEdit* timeFrom;
    svEdit* timeTo;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//  private:
    FILETIME &fT1;
    FILETIME &fT2;
  private:
    typedef svDialog baseClass;

};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

