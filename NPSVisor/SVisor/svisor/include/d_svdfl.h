//----------- d_svdfl.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_SVDFL_H_
#define D_SVDFL_H_
//----------------------------------------------------------------------------
#include "precHeader.h"

#ifndef SVDIALOG_H_
  #include "svdialog.h"
#endif

#ifndef PSTATIC_H_
  #include "pstatic.h"
#endif
#include "setPack.h"
//----------------------------------------------------------------------------
class TD_SavedFile : public svDialog
{
  public:
    TD_SavedFile(bool success, PWin* parent, uint resId = IDD_SAVED_FILE, HINSTANCE hinst = 0);
    virtual ~TD_SavedFile();
    bool isFinished() { return mustClose; }

    virtual bool create();

  protected:
    PStatic* TxtSaved;
    PStatic* BmpSaved;
    PStatic* BmpNotSaved;
    bool mustClose;

    bool preProcessMsg(MSG& msg);
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    bool Success;
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

