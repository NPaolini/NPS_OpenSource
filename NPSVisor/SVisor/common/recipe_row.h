//-------- recipe_row.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef RECIPE_ROW_H_
#define RECIPE_ROW_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_defbody.h"
#include "ptraspbitmap.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// ritorna il testo abbinato alla ricetta per il trefolo.
// Valido solo se il primo campo è in formato testo (nella nuova specifica
// occorre fornire il nome della pagina da cui ricavare le informazioni.
// Veniva richiamato dall'header. Nel nuovo header occorrerà inserire le
// informazioni sulla pagina e sugli indirizzi nel file di testo).
bool getRecipeTextType(LPTSTR target, uint row, LPCTSTR pageFile);
//----------------------------------------------------------------------------
#define MAX_ROW_RECIPE 40
//----------------------------------------------------------------------------
/*
#define INIT_RECIPE_IN_MEM 100
#define CURR_ROW_IN_MEM (INIT_RECIPE_IN_MEM - 1)
#define RECIPE_FILE _T("Recipe_file")
//----------------------------------------------------------------------------
#define INIT_RECIPE_IN_MEM_SPIRA_1 501
//#define CURR_ROW_IN_MEM_SPIRA (INIT_RECIPE_IN_MEM_SPIRA - 1)
#define RECIPE_FILE_SPIRA_1 _T("Recipe_fileSpira1")
//----------------------------------------------------------------------------
#define INIT_RECIPE_IN_MEM_SPIRA_2 901
//#define CURR_ROW_IN_MEM_SPIRA (INIT_RECIPE_IN_MEM_SPIRA - 1)
#define RECIPE_FILE_SPIRA_2 _T("Recipe_fileSpira2")
*/
//----------------------------------------------------------------------------
#define MAX_BUFF_TYPE 30
//----------------------------------------------------------------------------
class TD_RecipeRow : public P_DefBody
{
  public:
    TD_RecipeRow(int idPar, PWin* parent, int resId = IDD_STANDARD, HINSTANCE hinst = 0);
    virtual ~TD_RecipeRow();
    virtual P_Body* pushedBtn(int idBtn);
    virtual void setReady(bool first);
    virtual bool create();

    virtual void ShowErrorData(uint idprf, const prfData& data, prfData::tResultData result);

  protected:
    virtual void getFileStr(LPTSTR path);
//    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    uint idInitMem;
    int currRow;

    int nRow;
    int nCol;
    int firstId;
    int firstIdNum;

    PTraspBitmap* Indicator;

    POINT PtInd;
    int Delta;

    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void evPaint(HDC hdc, const PRect& rect);

    struct color {
      HBRUSH hBrush;
      COLORREF fg;
      COLORREF bkg;
      color() : hBrush(0), fg(0), bkg(0) {}
      ~color() { if(hBrush) DeleteObject(hBrush); }
      };
    color cActive;
    color cUnactive;

    bool Send();

    bool save(bool showResult=true);
    bool load();
    virtual bool customSave(P_File& f);
    virtual bool customLoad(P_File& f);

    void moveUp();
    void moveDown();
    void updateMove(int previousRow);
    void invalidateRow(int row);


    void updateText();
    TCHAR buffText[MAX_ROW_RECIPE][MAX_BUFF_TYPE + 1];

  private:
    typedef P_DefBody baseClass;

    bool useFirstForText;
    void setOffsEdi(bool invalidateAll);
    void drawNum(int delta, HDC hdc, const PRect& rect);
    void refreshNum();

    virtual void  preOffsEdi();
    virtual void  postOffsEdi();

    genericPerif* getPrfMem();

    void setNameActive(bool useCurrRow = false);
    int getActiveRow(int defVal);

    PVect<baseSimple*> Numb;

};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
