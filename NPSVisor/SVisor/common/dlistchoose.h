//------- dlistchoose.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DLISTCHOOSE_H_
#define DLISTCHOOSE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "svdialog.h"
#include "plistbox.h"
#include "p_txt.h"
//--------------------------------------------------------------------------

class TD_ListChoose : public svDialog {
  public:
    TD_ListChoose(PWin* parent, uint resId = IDD_EXPORT);
    virtual ~TD_ListChoose();
    int getChoose() { return choose; }
  protected:
    PListBox* LB;
    static int choose;
  public:
    virtual bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void LBNDblclk();
    void CmOk();

  private:
    typedef svDialog baseClass;
};

#endif

