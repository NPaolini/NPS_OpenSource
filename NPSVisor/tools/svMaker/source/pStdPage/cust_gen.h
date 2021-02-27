//--------- cust_gen.h ----------------------------------------------
//-------------------------------------------------------------------
#ifndef cust_gen_H_
#define cust_gen_H_
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
class cust_Row
{
  public:

    cust_Row(PWin* parent, uint first_id);
    virtual ~cust_Row() {}

    void getDescr(LPTSTR buff) const;
    void setDescr(LPCTSTR buff) const;
    DWORD getID();
    void setID(DWORD db);

  protected:
    PEdit* ID;
    PEdit* Descr;
    uint firstId;

};
//----------------------------------------------------------------------------
#define MAX_GROUP 8
//----------------------------------------------------------------------------
#define MAX_TEXT 1000
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
#define CYAN_COLOR  RGB(0xcf,0xcf,0xff)
#define bkgColor2 CYAN_COLOR
//----------------------------------------------------------------------------
#define LABEL_COLOR  RGB(218,236,215)
#define bkgColor3 LABEL_COLOR
//----------------------------------------------------------------------------
struct cust_baseRow
{
  DWORD id;
  LPTSTR text;

  cust_baseRow() : id(0), text(new TCHAR[MAX_TEXT]) { text[0] = 0; }
  cust_baseRow(DWORD id, LPCTSTR txt) : id(id), text(new TCHAR[MAX_TEXT]) { _tcscpy_s(text, MAX_TEXT, txt); }
  ~cust_baseRow() { delete []text; }

  private:
    const cust_baseRow& operator=(const cust_baseRow& other);
    cust_baseRow(const cust_baseRow& other);
};
//-------------------------------------------------------------------
#define MAX_ROWS 50
//-------------------------------------------------------------------
class cust_gen : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    cust_gen(PWin* parent, uint resId = IDD_DIALOG_DEFAULT_STD, HINSTANCE hinstance = 0);
    ~cust_gen();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();// {}

  private:
    cust_baseRow Saved[MAX_ROWS];
    cust_Row* Rows[MAX_GROUP];
    int currPos;

    HBRUSH Brush;
    HBRUSH Brush2;
    HBRUSH Brush3;

    void saveCurrData();
    bool saveData();
    void loadData();

    void evVScrollBar(HWND child, int flags, int pos);
};
//-------------------------------------------------------------------
#endif
