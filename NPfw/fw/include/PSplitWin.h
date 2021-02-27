//-------------------- PSplitWin.h --------------------------
//-----------------------------------------------------------
#ifndef PSPLITWIN_H_
#define PSPLITWIN_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
#include "PSplitter.h"
//-----------------------------------------------------------
class PSplitWin : public PWin
{
  public:
    PSplitWin(PWin* parent, uint id, uint perc = 500, uint width = 3, uint style = PSplitter::PSW_HORZ, HINSTANCE hInst = 0);

    virtual ~PSplitWin();

    void setW1(PWin* w) { Split->setW1(w); }
    void setW2(PWin* w) { Split->setW2(w); }
    void setWChild(PWin* w1, PWin* w2) { Split->setW1(w1); Split->setW2(w2); }

    virtual bool create();
    PSplitter* getSplitter() { return Split; }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PSplitter* getSplit() { return Split; }
    void replaceSplit(PSplitter* other, bool preserveData = true);
  private:
    typedef PWin baseClass;
    PSplitter* Split;
};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
