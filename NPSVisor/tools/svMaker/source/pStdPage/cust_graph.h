//------ cust_graph.h -----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef cust_graph_H_
#define cust_graph_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "pModDialog.h"
#include "pEdit.h"
#include "pStatic.h"
#include "svmMainClient.h"
#include "common_color.h"
//----------------------------------------------------------------------------
#define MAX_GROUP 10
//----------------------------------------------------------------------------
class PSimpleColorRow
{
  public:
    enum checkFocus { LOSS = -1, NOCHANGE, GET };

    PSimpleColorRow(PWin* parent, uint first_id);
    virtual ~PSimpleColorRow() {}

    // passando 0 come HWND si forza il settaggio con il ridisegno
    virtual checkFocus setFocus(uint firstIdSB, HWND hwnd = 0);
    void setColor(COLORREF color);
    void setColorBySelf(uint firstIdSB);
//    void moveTo(PSimpleColorRow& other);
    COLORREF getColor();
    PColorBox* getBox() { return cbx; }
    bool hasFocus() { return Focus; }

    virtual void setAll(LPCTSTR p);
  protected:
    PEdit* rgb[3];
    PColorBox* cbx;
    uint firstId;
    bool Focus;
};
//----------------------------------------------------------------------------
class PColorRow : public PSimpleColorRow
{
  private:
    typedef PSimpleColorRow baseClass;
  public:

    PColorRow(PWin* parent, uint first_id);

    // passando 0 come HWND si forza il settaggio con il ridisegno
    checkFocus setFocus(uint firstIdSB, HWND hwnd = 0);

    WORD getPtScr();
    void setPtScr(WORD val);
    WORD getPtPrn();
    void setPtPrn(WORD val);

    void setAll(LPCTSTR p);
  protected:
    PEdit* ptScr;
    PEdit* ptPrn;
};
//----------------------------------------------------------------------------
//class PBitmap;
class cust_graph : public PModDialog
{
  public:
    cust_graph(PWin* parent, uint resId = IDD_DIALOG_DEFAULT_GRAPH, HINSTANCE hinstance = 0);
    virtual ~cust_graph();

    virtual bool create();
    void setCurrColor(COLORREF c) { CurrColor->set(c); }
  protected:
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk() {}
  private:
    PColorRow* Rows[MAX_GROUP];
    PColorBox* BaseColor[3];
    PColorBox* CurrColor;

    PSimpleColorRow* axText;

    HBRUSH Brush;
    HBRUSH Brush2;
    HBRUSH Brush3;

    void saveData();
    void loadData();
    void fillLB(setOfString& set);
    void modifyLB();
    void evHScrollBar(HWND child, int flags, int pos);
    void checkEnableScaleX();
    void checkEnabletypeRow();
    void changedCBTrendName();

};
//-------------------------------------------------------------------
#endif
