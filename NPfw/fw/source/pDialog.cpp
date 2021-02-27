//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pDialog.h"
//----------------------------------------------------------------------------
PDialog::PDialog(PWin* parent, uint id, HINSTANCE hInst) :
  PWin(parent, 0, hInst), oldFocus(0), Result(IDCANCEL), customEraseBkg(false)
{
  Attr.id = id;
  setFlag(pfFromResource);
  setFlag(pfIsDialog);
}
//----------------------------------------------------------------------------
PDialog::~PDialog()
{
  destroy();

  if(oldFocus)
    SetFocus(oldFocus);
}
//----------------------------------------------------------------------------
static bool
registerFails(PWin* win, void*)
{
  return !win->registerClass();
}
//----------------------------------------------------------------------------
void
PDialog::registerChildObjects()
{
  firstThat(registerFails);
}
//----------------------------------------------------------------------------
bool PDialog::create()
{
  if(isFlagSet(pfIsDialog)) {
    registerChildObjects();
    setHandle(doCreate());
    if(getCaption())
      SendMessage(getHandle(), WM_SETTEXT, 0, (LPARAM) getCaption());
    else {
      int len = (int)SendMessage(getHandle(), WM_GETTEXTLENGTH, 0 , 0);
      LPTSTR title = new TCHAR[len + 1];
      SendMessage(getHandle(), WM_GETTEXT, len + 1, (LPARAM) title);
      title[len] = 0;
      setAllocatedTitle(title);
      }
    return createChildren();
    }
  return PWin::create();
}
//----------------------------------------------------------------------------
extern LRESULT CALLBACK atomicWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//----------------------------------------------------------------------------
HWND PDialog::doCreate()
{
  if(getHandle())
    return getHandle();
  HINSTANCE hI = getHInstance();
  HWND hW = 0;
  if(getParent())
    hW = *getParent();
  return ::CreateDialogParam(hI, MAKEINTRESOURCE(Attr.id), hW, (DLGPROC)atomicWindowProc, (UINT_PTR)this);
}
//----------------------------------------------------------------------------
bool PDialog::preProcessMsg(MSG& msg)
{
  if(WM_KEYDOWN == msg.message)
    if(VK_ESCAPE == msg.wParam)
        PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
  return baseClass::preProcessMsg(msg);
}
//----------------------------------------------------------------------------
HWND PDialog::getFirstFocus()
{
#if 1
  return 0;
#else
  PWin* child = getFirstChild();
  return child ? *child : 0;
#endif
}
//----------------------------------------------------------------------------
#define ID_TIMER_LAST 65534
//----------------------------------------------------------------------------
LRESULT PDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ERASEBKGND:
      if(customEraseBkg)
        SetWindowLongPtr(*this, DWLP_MSGRESULT, TRUE);
      break;
/*
    case WM_TIMER:
      if(ID_TIMER_LAST == wParam) {
        KillTimer(*this, ID_TIMER_LAST);
        HWND hw = getFirstFocus();
        if(hw)
          SetFocus(hw);
        }
      break;
*/
    case WM_INITDIALOG:
//      SetTimer(hwnd, ID_TIMER_LAST, 10, 0);
      if(isFlagSet(pfModal))
        oldFocus = GetFocus();
      return 1;
    }

  return 0;
}
//----------------------------------------------------------------------------
int PDialog::modal()
{
  setFlag(pfModal);
  HINSTANCE hI = getHInstance();
  HWND hW = 0;
  if(getParent())
    hW = *getParent();
  return (int)DialogBoxParam(hI, MAKEINTRESOURCE(Attr.id), hW, (DLGPROC)atomicWindowProc, (UINT_PTR)this);
}
//----------------------------------------------------------------------------
int PDialog::smodal(bool disableOther)
{
  HWND oldFocus = GetFocus();
  PWin* pw = getParent();
  PVect<PWin*> toDisable;
  PAppl* pAp = getAppl();
  bool hasApp = true;
  if(!pAp) {
    pAp = new PAppl(getHInstance(), 0);
    hasApp = false;
    }
  int i = 0;
  if(disableOther) {
    while(pw) {
      if(IsWindowEnabled(*pw)) {
        toDisable[i] = pw;
        ++i;
        EnableWindow(*pw, false);
        }
      pw = pw->getParent();
      }
    }
  if(create()) {
    // finché non viene chiusa la window
    while(getHandle()) {
      // aspetta l'arrivo di un messaggio
      ::WaitMessage();
      // lo fa elaborare dall'applicazione principale che provvede al dispatch
      pAp->pumpMessages();
      }
    }
  if(!hasApp)
    delete pAp;
  // riabilita
  for(int j = i - 1; j >= 0; --j)
    EnableWindow(*toDisable[j], true);
  pw = getParent();
  if(pw) {
    while(pw->getParent())
      pw = pw->getParent();
    SetForegroundWindow(*pw);
    }
  if(oldFocus)
    SetFocus(oldFocus);
  return Result;
}
//----------------------------------------------------------------------------
