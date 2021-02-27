//--------- cust_reciperow.h ----------------------------------------------
//-------------------------------------------------------------------
#ifndef cust_recipe_H_
#define cust_recipe_H_
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pedit.h"
#include "p_Vect.h"
#include "p_Txt.h"
#include "p_Util.h"
#include "pModDialog.h"
#include "svmMainClient.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
#define CYAN_COLOR  RGB(0xcf,0xcf,0xff)
#define bkgColor2 CYAN_COLOR
//----------------------------------------------------------------------------
#define LABEL_COLOR  RGB(218,236,215)
#define bkgColor3 LABEL_COLOR
//-------------------------------------------------------------------
class cust_recipeRow : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    cust_recipeRow(PWin* parent, uint resId = IDD_DIALOG_DEFAULT_RECIPEROW, HINSTANCE hinstance = 0);
    ~cust_recipeRow();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();

  private:

    bool hasFirstText;

    uint nCol;

    HBRUSH Brush;
    HBRUSH Brush2;
    HBRUSH Brush3;

    bool saveData();
    void loadData();

    int FirstNum;
    int NumRow;
    int NumCol;
    int FirstEdit;
    int InitOnMem;

};
//-------------------------------------------------------------------
//-------------------------------------------------------------------
#endif
