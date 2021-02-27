//-------- recipeDan.h ----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef RECIPEDAN_H_
#define RECIPEDAN_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "recipe.h"
#include "ptraspbitmap.h"
//----------------------------------------------------------------------------
#define ID_INIT_RECIPE_LB 1001300
//----------------------------------------------------------------------------
class TD_RecipeDan : public TD_Recipe
{
  public:
    TD_RecipeDan(int idPar, PWin* parent, int resId = IDD_STANDARD, HINSTANCE hinst = 0);
    virtual ~TD_RecipeDan();
    virtual P_Body* pushedBtn(int idBtn);
    virtual void setReady(bool first);
    virtual bool create();

  protected:
//    virtual void setDefaultValue();
    PVarListBox* allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen, HINSTANCE hinst);
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    bool lastChoose;
    void loadFromLb();

    void addToLb(LPCTSTR name, LPBYTE record);
    void loadAll(bool filter);
    bool isValid(LPBYTE fBuff);
    void addRow(LPCTSTR filename, bool force);

    void handleLBDblClick(HWND hwnd);

    pVectInt vOffset;
    pVectInt vFilter;
    pVectInt vLenField;
    pVectInt vLenColumn;
    pVectInt vTypes;
    pVectInt vDec;
    int lenRow;

    void calcLenAndOffset();

    typedef TD_Recipe baseClass;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
