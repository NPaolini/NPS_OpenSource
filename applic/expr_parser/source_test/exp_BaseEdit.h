//-------- exp_BaseEdit.h --------------------------------------
//-------------------------------------------------------------------
#ifndef exp_BaseEdit_H
#define exp_BaseEdit_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "resource.h"
#include "PSplitWin.h"
#include "PDialog.h"
#include "PStatic.h"
#include "PEdit.h"
#include <richedit.h>
//-------------------------------------------------------------------
#include <Tom.h>
#include <Richole.h>
#define DEFINE_GUIDXXX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID CDECL name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
EXTERN_C extern const GUID CDECL IID_ITextDocument;
//-------------------------------------------------------------------
#include "setPack.h"
//-------------------------------------------------------------------
#define IDC_EDIT_VARS   300
#define IDC_EDIT_INIT   301
#define IDC_EDIT_CALC   302
#define IDC_EDIT_TEST   303
#define IDC_EDIT_RESULT 304
#define IDC_EDIT_FUNCT  305
//-------------------------------------------------------------------
#define ID_STATIC 1
#define ID_EDIT   2
#define ID_NUMBER 3
#define FIRST_EDIT (IDC_EDIT_VARS + ID_EDIT)
//-------------------------------------------------------------------
#define WM_MY_CUSTOM (WM_FW_FIRST_FREE + 5)
  #define CM_COLORIZE 1
  #define CM_NEWLINE  2
  #define CM_INVALIDATE 3
  #define CM_FOCUS      4
//-------------------------------------------------------------------
enum richEditStyle { eRichBase, eRichGoto, eRichLabel, eRichComment, eRichMax };
#define MAX_INFO_COLOR eRichMax
//-------------------------------------------------------------------
struct infoEdit
{
  COLORREF fg[MAX_INFO_COLOR];
  COLORREF bg;
  int fontHeight;
  int fontStyle;
  TCHAR fontName[256];
  infoEdit() : bg(0), fontHeight(0), fontStyle(0)
    {
      ZeroMemory(fg, sizeof(fg));
      ZeroMemory(fontName, sizeof(fontName));
    }
  bool operator ==(const infoEdit& other);
  bool operator !=(const infoEdit& other) { return !(*this == other); }
};
//-------------------------------------------------------------------
extern LPCTSTR getClassName_Rich();
extern bool useRichEdit2();
//-------------------------------------------------------------------
class setRedraw
{
  public:
    static void on(HWND ed);
    static void off(HWND ed);
  private:
    static int count;
};
//-------------------------------------------------------------------
struct menuPopup
{
  uint flag;
  uint id;
  LPCTSTR text;
};
//-------------------------------------------------------------------
int popupMenu(HWND hwnd, menuPopup* item, int num, POINT* pt = 0);
//-------------------------------------------------------------------
//-------------------------------------------------------------------
class PNumberRow;
#define NOT_VALID_Y (-6553500)
//-------------------------------------------------------------------
class InsideEdit : public PControl
{
  private:
    typedef PControl baseClass;
  public:
    InsideEdit(PWin* parent, uint id,  infoEdit* iedit, bool readonly);
    ~InsideEdit() { destroy(); }

    void setOnFocus(bool set) { onFocus = set; }
    void setManRow(PNumberRow* nr) { NumbRow = nr; }

    const infoEdit& getInfo() const { return iEdit; }
    void setInfo(const infoEdit& ed);
    bool create();
    PNumberRow* getNumbRow() { return NumbRow; }
    bool hasFocus() { return onFocus; }
    void setReadOnly(bool set);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void mouseEnter(const POINT& pt, uint flag);
    bool evPaste();

    virtual LPCTSTR getClassName() const { return getClassName_Rich(); }
    bool onFocus;
    // la gestione è stata spostata tutta nella classe derivata (prima era solo per il primo edit, ora vale per tutti)
//    int verifyKey();
    virtual bool evChar(UINT& key);
    bool evKeyDown(UINT& key);

    bool readOnly;
    infoEdit iEdit;
    void setFormat();

    int calcHeight();
    void onMenupopup();
    bool checkVertPos();

    int lastY;

    friend class PNumberRow;
    PNumberRow* NumbRow;
};
//-------------------------------------------------------------------
class InsideEditFirst : public InsideEdit
{
  private:
    typedef InsideEdit baseClass;
  public:
    InsideEditFirst(PWin* parent, uint id,  infoEdit* iedit, bool readonly);
    ~InsideEditFirst() { destroy(); }
    void reloadCurr();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    int verifyKey();
    virtual bool evKeyDown(UINT& key);
    virtual bool evKeyUp(UINT& key);
    CHARRANGE crSave;
    POINT ptSave;
    bool onTabSpace;
    uint idTimer;
    friend class PNumberRow;
};
//-----------------------------------------------------------
class PNumberRow : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PNumberRow(PWin* parent, uint id, InsideEdit* ed) : baseClass(parent, id, PRect(0, 0, 10, 10)), Ed(ed),
    hBmpWork(0), mdcWork(0), oldObj(0), letter_height(0), refreshTimer(0), lastY(NOT_VALID_Y), timerCount(0)
    {
      Attr.style |= WS_CHILD;
      Attr.style &= ~WS_TABSTOP;
    }
    ~PNumberRow() { destroy(); freeBkg(); }
    PRect getRectLine(int currLine);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void makeBkg();
    void freeBkg();

    HBITMAP hBmpWork;
    HDC mdcWork;
    HGDIOBJ oldObj;
    PRect numberRect;
    void evPaint(HDC hdc);
    void drawNumber();
  private:
    InsideEdit* Ed;
    int letter_height;
    uint refreshTimer;
    int lastY;
    uint timerCount;
    friend class InsideEdit;
};
//-------------------------------------------------------------------
#include "restorePack.h"
//-------------------------------------------------------------------
#endif
