//-------------------- PWin.H ---------------------------
#ifndef PWIN_H_
#define PWIN_H_
//-----------------------------------------------------------
#include <windows.h>
//-----------------------------------------------------------
#ifndef PDEF_H_
  #include "pDef.h"
#endif
#include "p_vect.h"
//-----------------------------------------------------------
class PWin;
//-----------------------------------------------------------
#ifndef PAPPL_H_
  #include "pAppl.h"
#endif

#ifndef PRECT_H_
  #include "pRect.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
typedef void (*PActionFunc)(PWin* win, void* param);
typedef bool (*PCondFunc)(PWin* win, void* param);
//-----------------------------------------------------------
typedef void (PWin::*PActionMemFunc)(PWin* win, void* param) const;
typedef bool (PWin::*PCondMemFunc)(PWin* win, void* param) const;
//-----------------------------------------------------------
typedef LONG_PTR PWinProc;
//typedef LRESULT (CALLBACK *PWinProc)(HWND, UINT, WPARAM, LPARAM);
//-----------------------------------------------------------
enum PWinFlag {
  pfNone        = 0,
  pfModal       = 1 << 0,
  pfWantIdle    = 1 << 1,
  pfSubClassed  = 1 << 2,
  pfMainWindow  = 1 << 3,
  pfFromResource= 1 << 4,
  pfPredefinite = 1 << 5,
  pfIsDialog    = 1 << 6,
  pfDialogToWin = 1 << 7,
  pfWrapped4HWND= 1 << 8,
  };
//-----------------------------------------------------------
#ifndef IDM_MAIN
#define IDM_MAIN 1
#endif
//-----------------------------------------------------------
void DisplayErrorString(DWORD dwErr);
void MakeErrorString(DWORD dwErr, LPTSTR buff, size_t dimBuff);
//-----------------------------------------------------------
void extent(HWND hWnd, LPCTSTR lpString, SIZE& size);
int extent(HWND hWnd, LPCTSTR lpString);
void calcExt(HDC hDC, LPCTSTR str, long& width);
//-----------------------------------------------------------
class IMPORT_EXPORT PWin
{
  public:
    PWin(PWin* parent, LPCTSTR title = 0, HINSTANCE hInst = 0);
    PWin(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0);
    PWin(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0);
    PWin(HWND hwnd, HINSTANCE hInst);

    virtual ~PWin();

    virtual bool create();

    // viene richiamato in risposta al messaggio WM_DESTROY,
    // dopo aver settato a zero l'handle per evitare loop (richiama
    // DestroyWindow()), e nel dtor.
    // N.B. Se occorresse effettuare pulizie varie, va richiamato
    // anche dal dtor della classe erede per poter gestire il messaggio
    // WM_DESTROY di chiusura (in caso di chiusura automatica, senza
    // intervento utente), altrimenti l'oggetto erede sarebbe già
    // stato distrutto, eliminata la sua vtable e quindi non più
    // accessibile
    void destroy();


    HWND getHandle() const;
    void setHandle(HWND hw);

    operator HWND() const { return getHandle(); }

    void setCaption(LPCTSTR text);
    LPCTSTR getCaption();

    HINSTANCE getHInstance() const;

    PWin* Next();
    PWin* Previous();
    void setNext(PWin* next);
    PWin* getFirstChild();
    PWin* getLastChild();
    void setFlag(UINT mask);
    void clearFlag(UINT mask);
    bool isFlagSet(UINT mask) const;

    virtual bool idle(DWORD count);

    static void performZOrder(const PVect<PWin*>& set);

    class attrib {
      public:
      attrib(LPTSTR hm = 0,
        int x = CW_USEDEFAULT, int y = CW_USEDEFAULT,
        int w = CW_USEDEFAULT, int h = CW_USEDEFAULT,
        UINT style = WS_VISIBLE, UINT styleEx = 0,
        UINT id = 0, PWinFlag flag = pfNone) :
            hmenu(hm),
            x(x), y(y), w(w), h(h), style(style),
            exStyle(styleEx), id(id), flag(flag) {  }
      int x;
      int y;
      int w;
      int h;
      UINT id;
      UINT style;
      UINT exStyle;
      LPTSTR hmenu;
      UINT flag;
      } Attr;
    virtual PRect getRect();
    PWin* firstThat(PCondFunc test, void* paramList = 0) const;
    void  forEach(PActionFunc action, void* paramList = 0) const;

    PWin* firstThat(PCondMemFunc test, void* paramList = 0) const;
    void  forEach(PActionMemFunc action, void* paramList = 0) const;

    const PWin* getParent() const;
    PWin* getParent();
    virtual bool registerClass();

    static PWin* getWindowPtr(HWND hwnd);

    bool setWindowPos(HWND hWndInsertAfter, const PRect& r, UINT uFlags);
    virtual bool preProcessMsg(MSG& msg);

    bool isCaptured();
    bool isEnabledCapture();
    bool enableCapture(bool set = true);
    virtual bool isInWindow(const POINT& pt);
    virtual bool isInClient(const POINT& pt);

    void addChild(PWin* child);
    void removeChild(PWin* child);
    void setParent(PWin* parent);
  protected:

    virtual LPCTSTR getClassName() const;
    virtual void getWindowClass(WNDCLASS& wcl);
    // torna false se non riesce a creare tutti i figli
    bool createChildren();

    virtual HWND performCreate();

    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
//#define STATIC_MEMBER
#ifdef STATIC_MEMBER
    static LRESULT CALLBACK atomicWindowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
#else
    friend LRESULT CALLBACK atomicWindowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
#endif
    virtual void mouseEnter(const POINT& pt, uint flag);
    virtual void mouseMove(const POINT& pt, uint flag);
    virtual void mouseLeave(const POINT& pt, uint flag);

    void setAllocatedTitle(LPTSTR title);

    virtual bool canClose() { return true; }
  private:
    HWND hWnd;
    //  genitore, figli e fratelli.
    // Il genitore è unico,
    // i figli sono implementati come una lista linkata semplice,
    // i fratelli come una lista circolare semplice in cui il primo nodo è se stesso,
    // a meno che non abbia genitore, allora non deve avere nemmeno fratelli
    PWin *Parent;
    PWin *Child;
    PWin *Sibling;

    friend class PDialog;
    mutable HINSTANCE hInstance;

    enum { NOT_CAPTURED = 1, CAPTURED = 1 << 1, ENABLE_CAPTURE = 1 << 2 };
    DWORD capturedMouse;
    void setCapture();
    void resetCapture();

    LPTSTR Title;

    void init();

    PWinProc OldWindowProc;
    void subClass();
    void unsubClass();

    void storeOldProc(PWinProc  oldProc);
    PWinProc getOldProc() const;

    bool performCapture(HWND hwnd, UINT message, WPARAM wParam,
                        LPARAM lParam, LRESULT& result);

    static void addProp(HWND hwnd, LPVOID win);
    static void remProp(HWND hwnd);

//    friend class PAppl;
    NO_COPY_COSTR_OPER(PWin)
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class IMPORT_EXPORT PMainWin : public PWin
{
  public:
    PMainWin(LPCTSTR title, HINSTANCE hInst = 0) :
      PWin(0, title, hInst)
    {
      setFlag(pfMainWindow);
      Attr.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
      Attr.exStyle |= WS_EX_APPWINDOW;
    }
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <class T>
T* getParentWin(PWin* win)
{
  T* main;
  do {
    main = dynamic_cast<T*>(win);
    if(main)
      break;
    win = win->getParent();
    } while(win);
  return main;
}
//----------------------------------------------------------------------------
// PWin inlines
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline LPCTSTR PWin::getClassName() const { return _T("PWin"); }
//----------------------------------------------------------------------------
inline PWin* PWin::Next()
{
  return Sibling;
}
//----------------------------------------------------------------------------
inline void PWin::setNext(PWin* next)
{
  Sibling = next;
}
//----------------------------------------------------------------------------
inline PWin* PWin::getFirstChild()
{
  return Child ? Child->Sibling : 0;
}
//----------------------------------------------------------------------------
inline PWin* PWin::getLastChild()
{
  return Child;
}
//----------------------------------------------------------------------------
inline void PWin::setFlag(UINT mask)
{
  Attr.flag |= mask;
}
//----------------------------------------------------------------------------
inline void PWin::clearFlag(UINT mask)
{
  Attr.flag &= ~mask;
}
//----------------------------------------------------------------------------
inline bool PWin::isFlagSet(UINT mask) const
{
  return (Attr.flag & mask) ? true : false;
}
//----------------------------------------------------------------------------
inline void PWin::storeOldProc(PWinProc  oldProc)
{
  OldWindowProc = oldProc;
}
//----------------------------------------------------------------------------
inline PWinProc PWin::getOldProc() const
{
  return OldWindowProc;
}
//----------------------------------------------------------------------------
inline bool PWin::preProcessMsg(MSG& msg)
{
  return false;
}
//----------------------------------------------------------------------------
inline const PWin* PWin::getParent() const
{
  return Parent;
}
//----------------------------------------------------------------------------
inline PWin* PWin::getParent()
{
  return Parent;
}
//----------------------------------------------------------------------------
/*
inline HINSTANCE PWin::getHInstance() const
{
  return hInstance ? hInstance : PAppl::getHinstance();
}
*/
//----------------------------------------------------------------------------
inline HWND PWin::getHandle() const { return hWnd; }
//----------------------------------------------------------------------------
inline void PWin::setHandle(HWND hw) { hWnd = hw; }
//----------------------------------------------------------------------------
inline bool PWin::setWindowPos(HWND hWndInsertAfter, const PRect& r, UINT uFlags)
{
  return toBool(::SetWindowPos(*this, hWndInsertAfter, r.left, r.top, r.Width(), r.Height(), uFlags));
}
//----------------------------------------------------------------------------
inline LPCTSTR PWin::getCaption()
{
  return Title;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline void PWin::mouseEnter(const POINT& /*pt*/, uint /*flag*/) { }
//----------------------------------------------------------------------------
inline void PWin::mouseMove(const POINT& /*pt*/, uint /*flag*/) { }
//----------------------------------------------------------------------------
inline void PWin::mouseLeave(const POINT& /*pt*/, uint /*flag*/) { }
//----------------------------------------------------------------------------
inline bool PWin::isInClient(const POINT& pt)
{
  PRect r;
  GetClientRect(*this, r);
  return r.IsIn(pt);
}
//----------------------------------------------------------------------------
inline bool PWin::isInWindow(const POINT& pt)
{
#if 0
  PRect r(Attr.x, Attr.y, Attr.x + Attr.w, Attr.y + Attr.h);
#else
  PRect r;
  GetWindowRect(*this, r);
#endif
  return r.IsIn(pt);
}
//----------------------------------------------------------------------------
inline bool PWin::isCaptured() { return toBool(CAPTURED & capturedMouse); }
//----------------------------------------------------------------------------
inline bool PWin::isEnabledCapture() { return toBool(ENABLE_CAPTURE & capturedMouse); }
//----------------------------------------------------------------------------
inline void PWin::setCapture() { capturedMouse |= CAPTURED; }
//----------------------------------------------------------------------------
inline void PWin::resetCapture() { capturedMouse &= ~CAPTURED; }
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

