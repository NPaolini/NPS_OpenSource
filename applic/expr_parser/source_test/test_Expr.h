//----------- test_Expr.h -------------------------------------------------
//----------------------------------------------------------------------------
#ifndef test_Expr_H_
#define test_Expr_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "exp_BaseContainer.h"
#include "p_Util.h"
#include "p_file.h"
#include "PBitmap.h"
#include "testManVars.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class test_Expr : public PMainWin
{
  private:
    typedef PMainWin baseClass;
  public:
    test_Expr(LPCTSTR title, HINSTANCE hinst = 0);
    virtual ~test_Expr();

    virtual bool create();
    bool preProcessMsg(MSG& msg);
    HWND getEditCtrlResult();
    HWND getEditCtrlInit();
    HWND getEditCtrlTest();
    HWND getEditCtrlCalc();
    HWND getEditCtrlFunct();
    void deleteSearchReplace();
    bool findFunct(LPCTSTR functName, CHARRANGE& cr);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC hdc, HWND hwChild, UINT ctlType);

  private:
    PSplitter* Split;
    exp_BaseContainer* bodyCalc;
    exp_BaseContainer* bodyFunct;
    exp_BaseContainer* bodyInit;
    exp_BaseContainer* bodyVars;
    exp_BaseContainer* bodyTest;
    exp_BaseContainer* bodyResult;
    PDialogBtn* topBtn;

    class PDialogSearchReplace* objSearchReplace;
    exp_BaseContainer* forSearchRepace[4];

    void run();
    bool save(bool dialog);
    void getFile();

    bool saveFile(LPCTSTR file);
    bool checkSave(bool reset = true);
    void resetDirty();
    void checkReadOnly();

    void savePerc();
    void setup();
    void searchReplace();

    enum state { sUndef = -1, sInit, sVars = sInit, sCalc, sInitialize, sMaxState };
    int currState;

    void findAndSetblock(LPTSTR buff, LPCTSTR marker, exp_BaseContainer* body);
    void loadPart(LPCTSTR file);

    void setTitle(LPCTSTR tit);
    void check_init(exp_BaseContainer* body, testManVars& tmv);
};
//----------------------------------------------------------------------------
#endif

