//-------------------- PWin.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//#if 1
#ifdef __BORLANDC__
//#include <assert.h>
#define MY_ASSERT(a)
// assert
#else
#include <crtdbg.h>
#define MY_ASSERT _ASSERTE
#endif
//-----------------------------------------------------------
#include "pWin.h"
#include "pAppl.h"
#include "p_util.h"
#include "PCrt_lck.h"
//-----------------------------------------------------------
/**/
void MakeErrorString(DWORD dwErr, LPTSTR buff, size_t dimBuff)
{
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, LANG_SYSTEM_DEFAULT, buff, dimBuff, 0);
}
//-----------------------------------------------------------
void DisplayErrorString(DWORD dwErr)
{
  TCHAR buff[255];
  MakeErrorString(dwErr, buff, SIZE_A(buff));
  MessageBox(0, buff, _T("Descrizione Errore"), MB_OK);
}
/**/
//----------------------------------------------------------------------------
void calcExt(HDC hDC, LPCTSTR str, long& width)
{
  SIZE size;
  long len = _tcslen(str);
  GetTextExtentPoint32(hDC, str, len, &size);
  if(size.cx + len > width)
      width = size.cx + len;
}
//----------------------------------------------------------------------------
void extent(HWND hWnd, LPCTSTR lpString, SIZE& size)
{
    // ottieniamo il DC e il font da usare
    HDC hDC = GetDC(hWnd);
    HFONT hfNew = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);

    // piazziamo il nuovo font
    HFONT hfOld;
    if(hfNew)
      hfOld = (HFONT)SelectObject(hDC, hfNew);

    // otteniamo la dimensione in pixel della stringa
    uint len = _tcslen(lpString);
    GetTextExtentPoint32(hDC, lpString, len, &size);

    // ripristiniamo la situazione di font e DC
    if(hfNew) SelectObject(hDC, hfOld);
    ReleaseDC(hWnd, hDC);

    size.cx += len;
}
//----------------------------------------------------------------------------
int extent(HWND hWnd, LPCTSTR lpString)
{
    SIZE size;
    extent(hWnd, lpString, size);
    return size.cx;
}
//-----------------------------------------------------------
LRESULT CALLBACK atomicWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//-----------------------------------------------------------
PWin::PWin(PWin* parent, LPCTSTR title, HINSTANCE hInst) :
        Parent(parent), Title(title ? str_newdup(title) : 0),
        hInstance(hInst), OldWindowProc(0),
        hWnd(0), capturedMouse(false)
{
  init();
}
//-----------------------------------------------------------
#define STD_STYLE (/*WS_CLIPSIBLINGS | */WS_VISIBLE | WS_CHILD /*| WS_GROUP*/ | WS_TABSTOP)
PWin::PWin(PWin * parent, uint id, int x, int y, int w, int h, LPCTSTR title, HINSTANCE hinst) :
  Attr(0, x, y, w, h, STD_STYLE, 0, id, pfNone),
  Parent(parent), Title(title ? str_newdup(title) : 0), hInstance(hinst),
  OldWindowProc(0), hWnd(0), capturedMouse(false)

{
  init();
}
//-----------------------------------------------------------
PWin::PWin(PWin * parent, uint id, const PRect& r, LPCTSTR title, HINSTANCE hinst) :
  Attr(0, r.left, r.top, r.Width(), r.Height(), STD_STYLE, 0, id, pfNone),
  Parent(parent), Title(title ? str_newdup(title) : 0), hInstance(hinst),
  OldWindowProc(0), hWnd(0), capturedMouse(false)

{
  init();
}
//-----------------------------------------------------------
PWin::PWin(HWND hwnd, HINSTANCE hInst) :
  Parent(0), Title(0), hInstance(hInst),
  OldWindowProc(0), hWnd(hwnd), capturedMouse(false)
{
  PRect r;
  GetWindowRect(hWnd, r);
  Attr.x = r.left;
  Attr.y = r.top;
  Attr.w = r.Width();
  Attr.h = r.Height();
  setFlag(pfWrapped4HWND);
  subClass();
  addProp(hWnd, this);

}
//-----------------------------------------------------------
static void shutDown(PWin* win, void*)
{
//  win->destroy();
  delete win;
}
//-----------------------------------------------------------
PWin::~PWin()
{
  if(isFlagSet(pfWrapped4HWND)) {
    unsubClass();
    PWin::remProp(hWnd);
    setHandle(0);
    }
  else {
    forEach(shutDown);
    destroy();
    if(Parent)
      Parent->removeChild(this);
    }
  delete[] Title;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void PWin::destroy()
{
  if(isFlagSet(pfWrapped4HWND))
    return;
  if(getHandle()) {
    // solo se esiste un parent, se è ancora attivo e se appartiene
    // allo stesso thread, gli riassegna il focus
#if 1
    if(getHandle() == GetFocus() && Parent && Parent->getHandle() &&
          GetWindowThreadProcessId(*Parent, 0) == GetCurrentThreadId())
      SetFocus(*Parent);
#else
// non restituisce il focus al controllo che lo possedeva,
// meglio farlo fare quando effettivamente serve
    if(Parent && Parent->getHandle()) {
      DWORD parentId = GetWindowThreadProcessId(*Parent, 0);
      DWORD currId = GetCurrentThreadId();
      if(parentId == currId)
        SetFocus(*Parent);
      else {
        AttachThreadInput(currId, parentId, true);
        SetFocus(*Parent);
        AttachThreadInput(currId, parentId, false);
        }
      }
#endif
#define VERIFY_DESTROYWINDOW
#ifdef VERIFY_DESTROYWINDOW
    if(IsWindow(getHandle()))
      if(!::DestroyWindow(getHandle()))
        DisplayErrorString(GetLastError());
#else
    if(IsWindow(getHandle())
      DestroyWindow(getHandle());
#endif
    setHandle(0);
    }
}
//-----------------------------------------------------------
void PWin::init()
{
  Child = 0;
  Attr.hmenu = 0;

  if (Parent)
    Parent->addChild(this);
  else
    Sibling = 0;
}
//-----------------------------------------------------------
void PWin::addChild(PWin* child)
{
  if (child)
    if (Child) {
      child->Sibling = Child->Sibling;
      Child->Sibling = child;
      Child = child;
      }
    else {
      Child = child;
      child->Sibling = child;
      }
}
//-----------------------------------------------------------
#define TRACE(a)  MessageBox(0, _T(a), " ", MB_OK)

bool PWin::registerClass()
{
  WNDCLASS wcl;
  // verifica se si tratta di una classe standard
  bool gc = GetClassInfo(0, getClassName(), &wcl) != 0;
  HINSTANCE hI = getHInstance();

  if(!gc) {
    if(!GetClassInfo(hI, getClassName(), &wcl)) {
      getWindowClass(wcl);
      WNDCLASS dummy;
      if(!GetClassInfo(hI, getClassName(), &dummy))
        return toBool(RegisterClass(&wcl));
      }
    return true;
    }
  setFlag(pfPredefinite);
  return true;
}
//-----------------------------------------------------------
void PWin::getWindowClass(WNDCLASS& wcl)
{
  HINSTANCE hI = getHInstance();
  wcl.hInstance      = hI;
  wcl.lpszClassName  = getClassName();
  wcl.lpfnWndProc    = atomicWindowProc;
//  wcl.style          = CS_DBLCLKS;
  wcl.style          = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
  wcl.hIcon          = 0;
  wcl.hCursor        = LoadCursor(0, IDC_ARROW);
  wcl.lpszMenuName   = Attr.hmenu;
  wcl.cbClsExtra     = 0;
  if(isFlagSet(pfIsDialog))
    wcl.cbWndExtra     = DLGWINDOWEXTRA;
  else
    wcl.cbWndExtra     = sizeof(this);
  wcl.hbrBackground  = (HBRUSH) (COLOR_BTNFACE + 1); // GetStockObject(LTGRAY_BRUSH);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
static TCHAR nameProp[] = _T("NPWinBase");
//-----------------------------------------------------------
void PWin::addProp(HWND hwnd, LPVOID win)
{
  SetProp(hwnd, nameProp, (HANDLE)win);
}
//-----------------------------------------------------------
void PWin::remProp(HWND hwnd)
{
  RemoveProp(hwnd, nameProp);
 }
//-----------------------------------------------------------
inline PWin* getProp(HWND hwnd)
{
  return (PWin*)GetProp(hwnd, nameProp);
}
//-----------------------------------------------------------
HWND PWin::performCreate()
{
  HWND hPar = HWND_DESKTOP;
  if(Parent)
    hPar = *Parent;

  HINSTANCE hI = getHInstance();

  HMENU menuOrId;
  if (Attr.hmenu)
    menuOrId = LoadMenu(hI, Attr.hmenu);
  else
    menuOrId = (HMENU)Attr.id;

  LPCTSTR className = getClassName();
  return CreateWindowEx(Attr.exStyle, className, Title,
                  Attr.style, Attr.x, Attr.y, Attr.w, Attr.h,
                  hPar, menuOrId, hI, this);
}
//-----------------------------------------------------------
bool PWin::create()
{
  if(isFlagSet(pfWrapped4HWND))
    return true;
  if (isFlagSet(pfFromResource)) {
    if(!getHandle())
      setHandle(Parent ? GetDlgItem(*Parent, Attr.id) : 0);
    delete []Title;
    int len = (int)SendMessage(getHandle(), WM_GETTEXTLENGTH, 0 , 0);
    Title = new TCHAR[len + 1];
    SendMessage(getHandle(), WM_GETTEXT, len + 1, (LPARAM) Title);
    Title[len] = 0;
    }
#if 0
  if(!getHandle()) {
#else
  else {
#endif
    if(!registerClass())
      return false;

    hWnd = performCreate();
    if(!hWnd) {
      DisplayErrorString(GetLastError());
      return false;
      }
    }

  if(isFlagSet(pfPredefinite)) {
    subClass();
    addProp(hWnd, this);
    }

//  SetClassLong(hWnd, GCL_STYLE, GetClassLong(*this, GCL_STYLE) & ~CS_SAVEBITS);

  return createChildren();
}
//-----------------------------------------------------------
static bool cantCreate(PWin* win, void*)
{
  if(*win) {
    return false;
    }

  if (!win->create())
    return true;

  if (IsIconic(*win)) {
    int   textLen = GetWindowTextLength(*win);
    TCHAR* text = new TCHAR[textLen + 1];

    GetWindowText(*win, text, textLen + 1);
    SetWindowText(*win, text);
    delete[] text;
    }
  return false;
}
//-----------------------------------------------------------
void PWin::performZOrder(const PVect<PWin*>& set)
{
  uint nElem = set.getElem();
  if(!nElem)
    return;

  HWND above = HWND_TOP;
  for (int i = nElem - 1; i; --i) {
    SetWindowPos(*set[i], above, 0,0,0,0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE | SWP_DEFERERASE | SWP_NOREDRAW);
    above = *set[i];
    }
}
//-----------------------------------------------------------
bool PWin::createChildren()
{
  PWin* childFailed = firstThat(cantCreate);
  if (childFailed)
    return false;
/*
  HWND above = HWND_TOP;
  for (int top = numChildren(); top; top--) {
    PWin* wnd = firstThat(&PWin::orderIsI, &top);
    if (wnd) {
      SetWindowPos(*wnd, above, 0,0,0,0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE | SWP_DEFERERASE | SWP_NOREDRAW);
      above = *wnd;
      }
    }
*/
  return true;
}
//-----------------------------------------------------------
void PWin::subClass()
{
  MY_ASSERT(getHandle());

  DWORD which;
//  if(isFlagSet(pfModal))
  if(isFlagSet(pfIsDialog))
    which = DWLP_DLGPROC;
  else
    which = GWLP_WNDPROC;
  PWinProc old = (PWinProc)GetWindowLongPtr(getHandle(), which);

  // se la procedura non è quella della nostra classe, subclasse e salva
  if((PWinProc)atomicWindowProc != old) {
    old = (PWinProc) SetWindowLongPtr(getHandle(), which, (LONG_PTR) atomicWindowProc);
    storeOldProc(old);
    setFlag(pfSubClassed);
    }
}
//-----------------------------------------------------------
void PWin::unsubClass()
{
  MY_ASSERT(getHandle());
  // Se è stata subclassata, recupera la procedura corrente.
  // Se è quella giusta recupera dalla classe erede la procedura
  // originaria e la ripristina
  if(isFlagSet(pfSubClassed)) {
    DWORD which;
//    if(isFlagSet(pfModal))
    if(isFlagSet(pfIsDialog))
      which = DWLP_DLGPROC;
    else
      which = GWLP_WNDPROC;
    PWinProc curr = (PWinProc)GetWindowLongPtr(getHandle(), which);
    if((PWinProc)atomicWindowProc == curr) {
      PWinProc old = getOldProc();
      if(old) {
        SetWindowLongPtr(getHandle(), which, (LONG_PTR) old);
        storeOldProc(0);
        }
      }
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#ifdef STATIC_MEMBER
LRESULT CALLBACK PWin::atomicWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
#else
LRESULT CALLBACK atomicWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
#endif
{
  if(WM_CREATE == message) {
    PWin::addProp(hwnd, ((LPCREATESTRUCT) lParam)->lpCreateParams);
#if 0
    // forse meglio permettere la gestione anche alle classi derivate
    return 0;
#endif
    }
  else if(WM_INITDIALOG == message) {
    PWin *pThis = (PWin*)(LPVOID)(lParam);
    PWin::addProp(hwnd, pThis);
    if(pThis->isFlagSet(pfModal) && !pThis->isFlagSet(pfDialogToWin)) {
      // se è modale occorre assegnare l'handle qui
      pThis->setHandle(hwnd);
      // richiama la creazione normale, ma avendo già acquisito
      // l'handle esegue solo la creazione dei child
      pThis->create();
      }
//    addProp(hwnd, pThis);
#if 0
    // come sopra
    return 1;
#endif
    }

  // l'inizializzazione è separata dal resto così è possibile acquisire
  // il puntatore una sola volta per tutti i casi
  PWin *pThis = getProp(hwnd);

  if(!pThis)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch(message) {
    case WM_SYSCOMMAND:
      switch(wParam & 0xFFF0) {
        case SC_CLOSE:
          if(!pThis->canClose())
            break;
          if(pThis->isFlagSet(pfIsDialog) || pThis->isFlagSet(pfDialogToWin)) {
            if(pThis->isFlagSet(pfModal) && !pThis->isFlagSet(pfDialogToWin))
              EndDialog(hwnd, IDCANCEL);
            else
              DestroyWindow(pThis->getHandle());
//              delete pThis;
            return 0;
            }
          break;
        }
      break;

/*
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
//      if(!pThis->isCaptured())
        PostMessage(hwnd, WM_MOUSEMOVE, wParam, lParam);
      break;
*/
    case WM_CAPTURECHANGED:
/*
      if(!pThis->isCaptured()) {
        LRESULT result;
        if(pThis->performCapture(hwnd, message, wParam, lParam, result))
          return result;
        }
*/
      break;

    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:

    case WM_MOUSEMOVE:
      do {
        LRESULT result;
        if(pThis->performCapture(hwnd, message, wParam, lParam, result))
          return result;
        } while(false);
      break;


    case WM_DESTROY:
      pThis->unsubClass();
      pThis->setHandle(0);
      PWin::remProp(hwnd);
      // se è la finestra principale chiude il programma
      if(pThis->isFlagSet(pfMainWindow)) {
        PostQuitMessage (0);
//      return 0;
        }
      // rendiamo disponibile il messaggio anche alle classi eredi
      // per rimozione oggetti che necessitano dell'handle (es. timer)
      // return 0;
      break;
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
      if(wParam != pThis->Attr.id && wParam) {
        HWND hChild = GetDlgItem(hwnd, (int)wParam);
        if(hChild && getProp(hChild)) {
          return SendMessage(hChild, message, wParam, lParam);
//          return true;
          }
        }
      break;
    }
  return pThis->windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------
//-------------------------------------------------
LRESULT PWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if 1
  if(WM_INITDIALOG == message)
    return 1;
  else if(WM_CREATE == message)
    return 0;

  WNDPROC old = (WNDPROC)getOldProc();
  if(old)
    return CallWindowProc(old, hwnd, message, wParam, lParam);

  if(isFlagSet(pfIsDialog))
    return DefDlgProc(hwnd, message, wParam, lParam);

  return DefWindowProc(hwnd, message, wParam, lParam);


#else
  if(isFlagSet(pfIsDialog))
    return FALSE;

  WNDPROC old = (WNDPROC)getOldProc();
  if(old)
    return CallWindowProc(old, hwnd, message, wParam, lParam);
  return DefWindowProc(hwnd, message, wParam, lParam);
#endif
}
//-----------------------------------------------------------
void PWin::setParent(PWin* parent)
{
  Parent = parent;
  SetParent(*this, *parent);
}
//-----------------------------------------------------------
void PWin::removeChild(PWin* child)
{
  if (child && Child) {
    PWin*  lastChild = Child;
    PWin*  nextChild = lastChild;

    while (nextChild->Sibling != lastChild &&
           nextChild->Sibling != child) {
      nextChild = nextChild->Sibling;
    }

    if (nextChild->Sibling == child) {
      if (nextChild->Sibling == nextChild)
        Child = 0;

      else {
        if (nextChild->Sibling == Child)
          Child = nextChild;

        nextChild->Sibling = nextChild->Sibling->Sibling;
      }
    }
  }
}
//-----------------------------------------------------------
bool PWin::idle(DWORD idleCount)
{
  bool wantMore = false;
  PWin* win = getFirstChild();
  if (win) {
    do {
      if(idleCount == 0 || win->isFlagSet(pfWantIdle)) {
        if(win->idle(idleCount)) {
          win->setFlag(pfWantIdle);
          wantMore = true;
          }
        else {
          win->clearFlag(pfWantIdle);
          }
      }
      win = win->Next();
    } while (win && win != getFirstChild());
  }
  return wantMore;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PWin* PWin::firstThat(PCondFunc test, void* paramList) const
{
  if (Child) {
    PWin*  nextChild = Child->Next();
    PWin*  curChild;

    do {
      curChild = nextChild;
      nextChild = nextChild->Next();

      if (test(curChild, paramList))
        return curChild;
    } while (curChild != Child && Child);
  }
  return 0;
}
//-----------------------------------------------------------
void PWin::forEach(PActionFunc action, void* paramList) const
{
  if (Child) {
    PWin*  curChild;
    PWin*  nextChild = Child->Next();

    do {
      curChild = nextChild;
      nextChild = nextChild->Next();
      action(curChild, paramList);
    } while (curChild != Child && Child);
  }
}
//-----------------------------------------------------------
PWin* PWin::firstThat(PCondMemFunc test, void* paramList) const
{
  if (Child) {
    PWin*  nextChild = Child->Next();
    PWin*  curChild;

    do {
      curChild = nextChild;
      nextChild = nextChild->Next();

      if ((this->*test)(curChild, paramList))
        return curChild;
    } while (curChild != Child && Child);
  }
  return 0;
}
//-----------------------------------------------------------
void PWin::forEach(PActionMemFunc action, void* paramList) const
{
  if (Child) {
    PWin*  nextChild = Child->Next();
    PWin*  curChild;

    do {
      curChild = nextChild;
      nextChild = nextChild->Next();
      (this->*action)(curChild, paramList);
    } while (curChild != Child && Child);
  }
}
//-----------------------------------------------------------
/*
//-----------------------------------------------------------
struct posWin
{
  int position;
  PWin* child;
};
//-----------------------------------------------------------
static bool isItThisChild1(PWin* win, void* child)
{
  posWin* posW = (posWin*)child;
  ++posW->position;
  return win == posW->child;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int PWin::indexOf(PWin* child) const
{
  posWin pw = { -1, child };
  return firstThat(isItThisChild1, &pw) ? pw.position : -1;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
unsigned PWin::numChildren() const
{
  return indexOf(Child) + 1;
}
*/
//-----------------------------------------------------------
PWin* PWin::Previous()
{
  if (!Sibling) {
    return 0;
  }
  else {
    PWin*  CurrentIndex = this;

    while (CurrentIndex->Next() != this)
      CurrentIndex = CurrentIndex->Next();

    return CurrentIndex;
  }
}
//-----------------------------------------------------------
//----------------------------------------------------------------------------
PWin* PWin::getWindowPtr(HWND hwnd)
{
   return getProp(hwnd);
}
//-----------------------------------------------------------
void PWin::setCaption(LPCTSTR text)
{
  delete []Title;
  Title = text ? str_newdup(text) : 0;
  if(getHandle())
    SendMessage(getHandle(), WM_SETTEXT, 0 , (LPARAM) Title);
}
//-----------------------------------------------------------
void PWin::setAllocatedTitle(LPTSTR title)
{
  delete []Title;
  Title = title;
}
//-----------------------------------------------------------
bool PWin::performCapture(HWND /*hwnd*/, UINT message, WPARAM wParam,
                        LPARAM lParam, LRESULT& result)
{
  if(!isEnabledCapture())
    return false;
  POINT pt = { LOWORD(lParam), HIWORD(lParam) };
  bool isIn = isInClient(pt);
  if(isCaptured()) {
    bool needRelease = !isIn;
    if(!needRelease) {
      POINT pt2 = pt;
      ClientToScreen(*this, &pt2);
      needRelease = WindowFromPoint(pt2) != *this;
      }
    if(needRelease)
      needRelease = !toBool(wParam & MK_LBUTTON);

    if(needRelease) {
      if(GetCapture() == *this)
        ReleaseCapture();
      resetCapture();
      mouseLeave(pt, (uint)wParam);
      }
    else {
      if(GetCapture() != *this)
        SetCapture(*this);
      mouseMove(pt, (uint)wParam);
      }
    }
  else {
    if(isIn) {
      setCapture();
      SetCapture(*this);
      mouseEnter(pt, (uint)wParam);
      }
    }
//        char buff[100];
//        wsprintf(buff, "%3d x %3d", pt.x, pt.y);
//        SetWindowText(*pThis, buff);
  switch(message) {
    case WM_CAPTURECHANGED:
      result = 0;
      return true;
    }
  return false;
}
//-----------------------------------------------------------
bool PWin::enableCapture(bool set)
{
  if(set == isEnabledCapture())
    return set;
  if(set)
    capturedMouse |= ENABLE_CAPTURE;
  else {
    if(isCaptured()) {
      ReleaseCapture();
      resetCapture();
      POINT pt = { 0, 0 };
      mouseLeave(pt, 0);
      }
    capturedMouse &= ~ENABLE_CAPTURE;
    }
  return !set;
}
//-----------------------------------------------------------
HINSTANCE PWin::getHInstance() const
{
  if(!hInstance)
    hInstance = PAppl::getHinstance();

  if(!hInstance) {
    const PWin* parent = getParent();
    do {
      if(!parent)
        break;
      hInstance = parent->getHInstance();
      parent = parent->getParent();
      } while(!hInstance);
    }
  return hInstance;
}
//-----------------------------------------------------------
PRect PWin::getRect()
{
  PRect rect;
  if(getHandle()) {
    GetWindowRect(getHandle(), rect);
    const PWin* parent = getParent();
    if(parent)
      MapWindowPoints(HWND_DESKTOP, *parent, (LPPOINT)(LPRECT)rect, 2);
    }
  else if(!isFlagSet(pfFromResource)) {
    rect.left = Attr.x;
    rect.right = Attr.x + Attr.w;
    rect.top = Attr.y;
    rect.bottom = Attr.y + Attr.h;
    }
  return rect;
}
