//-------------------- mainMaker.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commctrl.h>
//-----------------------------------------------------------
#include "svmMainClient.h"
#include "svmRule.h"
#include "PSplitWin.h"
#include "PscrollContainer.h"
#include "common.h"
#include "resource.h"
//-----------------------------------------------------------
static
LPCTSTR get_ClassName()
{
  return _T("svLabelMaker_ClassWin");
}
//-----------------------------------------------------------
#define ID_CONTAINER 100
#define ID_RULE_X    101
#define ID_RULE_Y    102
#define ID_SPLIT1    103
#define ID_SPLIT2    104
#define ID_EMPTY     105
//-----------------------------------------------------------
#define SIZE_RULE    20
//-----------------------------------------------------------
bool loadRect(PRect& rect)
{
  return readKey(_T("MainCoords"), MAIN_PATH, (LPBYTE)(LPRECT)rect, sizeof(PRect), false);
}
bool writeKey(LPCTSTR name, LPCTSTR keyBlock, LPCBYTE value, int len);
bool readKey(LPCTSTR name, LPCTSTR keyBlock, LPBYTE value, int len, bool writeIfNotFound);

//-----------------------------------------------------------
void saveRect(HWND hwnd)
{
  PRect r;
  GetWindowRect(hwnd, r);
  writeKey(_T("MainCoords"), MAIN_PATH, (LPBYTE)(LPRECT)r, sizeof(r));
}
//-----------------------------------------------------------
class makerWin : public PMainWin
{
  private:
    typedef PMainWin baseClass;

  public:
    makerWin(LPCTSTR title, HINSTANCE hInstance);
    ~makerWin() { destroy(); delete Splitter; }

    bool create();

#if 1
    virtual bool preProcessMsg(MSG& msg)
    {
      if(TranslateAccelerator(getHandle(), haccel, &msg))
        return true;
//      return IsDialogMessage(getHandle(), &msg);
      return false;
    }
#else
    // non ci sono controlli attivi, e se si preme un CTRL+tasto va in loop
    virtual bool preProcessMsg(MSG& msg)
    {
      return IsDialogMessage(getHandle(), &msg);
    }
#endif
    void setZoom(svmManZoom::zoomX zoom, bool force) { RuleX->setZoom(zoom, force); RuleY->setZoom(zoom, force); }
    void setStartX(long v) { RuleX->setStartPx(v); }
    void setStartY(long v) { RuleY->setStartPx(v); }
  protected:
    HACCEL haccel;
    virtual LPCTSTR getClassName() const { return ::get_ClassName(); }
    virtual void getWindowClass(WNDCLASS& wcl);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    PscrollContainer* Child;
    PRule* RuleX;
    PRule* RuleY;
    PSplitter* Splitter;
};
//-----------------------------------------------------------
void g_setZoom(PWin* child, svmManZoom::zoomX zoom, bool force)
{
  makerWin* main = getParentWin<makerWin>(child);
  if(main)
    main->setZoom(zoom, force);
}
//-----------------------------------------------------------
void g_setStartX(PWin* child, long v)
{
  makerWin* main = getParentWin<makerWin>(child);
  if(main)
    main->setStartX(v);
}
//-----------------------------------------------------------
void g_setStartY(PWin* child, long v)
{
  makerWin* main = getParentWin<makerWin>(child);
  if(main)
    main->setStartY(v);
}
//-----------------------------------------------------------
#define SPLIT_W 1
//-----------------------------------------------------------
makerWin::makerWin(LPCTSTR title, HINSTANCE hInstance) :
      baseClass(title, hInstance)
{
  PSplitWin* split1 = new PSplitWin(this, ID_SPLIT1);
  PSplitWin* split2 = new PSplitWin(this, ID_SPLIT2);

  split1->Attr.style |= WS_CLIPCHILDREN;
  split2->Attr.style |= WS_CLIPCHILDREN;

  RuleY = new PRuleY(split1, ID_RULE_Y, hInstance);
  PWin* empty = new PWin(split1, ID_EMPTY, PRect(), 0, hInstance);
  split1->setWChild(empty, RuleY);

  RuleX = new PRuleX(split2, ID_RULE_X, hInstance);
  Child = new childScrollContainer(split2, ID_CONTAINER, hInstance);
  split2->setWChild(RuleX, Child);

  PSplitter* t = split1->getSplitter();
  t->setStyle(PSplitter::PSW_VERT, false);
  t->setDim1(SIZE_RULE, false);
  t->setSplitterWidth(SPLIT_W, false);
  t->setLock(PSplitter::PSL_FIRST);

  t = split2->getSplitter();
  t->setStyle(PSplitter::PSW_VERT, false);
  t->setDim1(SIZE_RULE, false);
  t->setSplitterWidth(SPLIT_W, false);
  t->setLock(PSplitter::PSL_FIRST);

  Splitter = new PSplitter(this, split1, split2);
  Splitter->setDim1(SIZE_RULE, false);
  Splitter->setSplitterWidth(SPLIT_W, false);
  Splitter->setLock(PSplitter::PSL_FIRST);
  Attr.style |= WS_CLIPCHILDREN;
}
//-----------------------------------------------------------
bool makerWin::create()
{
  PRect r;
  if(loadRect(r)) {
    Attr.x = r.left;
    Attr.y = r.top;
    Attr.w = r.Width();
    Attr.h = r.Height();
    }
  if(!baseClass::create())
    return false;

  haccel = LoadAccelerators(getHInstance(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));
  return Splitter->create();
}
//-----------------------------------------------------------
LRESULT makerWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      saveRect(hwnd);
      break;
    case WM_ERASEBKGND:
      return 1;
    case WM_COMMAND:
      PostMessage(*Child->getFirstChild(), message, wParam, lParam);
      break;
    }
  if(Splitter)
    Splitter->windowProcSplit(hwnd, message, wParam, lParam);
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void makerWin::getWindowClass(WNDCLASS& wcl)
{
  baseClass::getWindowClass(wcl);
  wcl.hbrBackground  = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
}
//-----------------------------------------------------------
class makerApp : public PAppl
{
  public:
    makerApp(HINSTANCE hInstance, int nCmdShow) :
      PAppl(hInstance, nCmdShow) { }
  protected:
    virtual PWin* initMainWindow(LPCTSTR title, HINSTANCE hInstance)
    {
      return new makerWin(title, hInstance);
    }
};
//-----------------------------------------------------------
#define TIT_NOT_ZOOM _T("Attenzione!! OS non pienamente supportato")
#define NOT_ZOOM _T("Lo Zoom non è supportato da questo OS\r\ne potrebbero esserci altri problemi legati all'obsolescenza\r\nVuoi continuare lo stesso?")
//-----------------------------------------------------------
#define SECOND_BEFORE_ABORT 10000
#define TIME_BETWEEN_REPEAT 200
#define REPEAT_BEFORE_ABORT (SECOND_BEFORE_ABORT / TIME_BETWEEN_REPEAT)
//----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
  if(!isWinNT_Based()) {
    if(IDYES != MessageBox(0, NOT_ZOOM, TIT_NOT_ZOOM, MB_YESNO | MB_ICONSTOP))
      return 0;
    }
  HANDLE hMutex = CreateMutex(0, true, get_ClassName());
  if(hMutex && ERROR_ALREADY_EXISTS == GetLastError()) {
    int i;
    for(i = 0; i < REPEAT_BEFORE_ABORT; ++i) {
      HWND hWnd = ::FindWindow(get_ClassName(), NULL);
      if(hWnd) {
        SetForegroundWindow(hWnd);
        break;
        }
      SleepEx(TIME_BETWEEN_REPEAT, false);
      }
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return 0;
    }

  InitCommonControls();
  do {
    int res = GetSystemMetrics(SM_CXFULLSCREEN);
    infoAdjusct adj;
    if(res > 1500) {
      adj.V = 1.2;
      adj.H = 1.2;
      }
    adj = loadAdjuct(adj);
    svmManZoom::setAdjuct(adj);
    } while(false);

  int result = makerApp(hInstance, nCmdShow).run(_T("SvLabel"));

  if(hMutex) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    }
  return result;
}
//-------------------------------------------------
