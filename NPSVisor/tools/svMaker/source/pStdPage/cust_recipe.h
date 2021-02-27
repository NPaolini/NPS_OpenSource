//--------- cust_recipe.h ----------------------------------------------
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
class cust_recipe : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    cust_recipe(PWin* parent, bool hasListbox, uint resId = IDD_DIALOG_DEFAULT_RECIPE, HINSTANCE hinstance = 0);
    ~cust_recipe();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();

  private:

    bool hasListbox;

    HBRUSH Brush;
    HBRUSH Brush2;
    HBRUSH Brush3;

    bool saveData();
    void loadData();

    void evVScrollBar(HWND child, int flags, int pos);

    void fillLBText(setOfString& set);
    void fillLBSend(setOfString& set);
    void fillLBDef(setOfString& set);
    void fillLBLB(setOfString& set);

    void modifyLB();
    void checkLabelDef();
    void checkCurrSelSend();

    void addSendCopy(bool append = true);
    void addDef(bool append = true);
    void addLbLb(bool append = true);

    bool saveLbSend(P_File* pf);
    bool saveLbDef(P_File* pf);
    bool saveLbLb(P_File* pf);

    void unFillSend();
    void unFillDef();
    void unFillLbLb();
    void modSendCopy();
    void modDef();
    void modLbLb();

    void copyFromLb();
    void pasteToLb();
    void fillRowByClipboard(LPCTSTR buff);

};
//-------------------------------------------------------------------
//-------------------------------------------------------------------
#endif
