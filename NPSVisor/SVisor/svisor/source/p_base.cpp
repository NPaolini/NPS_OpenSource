//------------- P_Base.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_base.h"
#include "mainClient.h"
#include "hdrmsg.h"
//----------------------------------------------------------------------------
static void makeModulePath(LPTSTR target, size_t dim,  LPCTSTR filename)
{
  LPCTSTR pf = filename + _tcslen(filename) - 1;
  while(pf > filename) {
    if(_T('\\') == *pf || _T('/') == *pf) {
      ++pf;
      break;
      }
    --pf;
    }
  GetModuleDirName(dim, target);
  appendPath(target, pf);
}
//----------------------------------------------------------------------------
myManageIni::myManageIni(LPCTSTR filename) : baseClass(filename)
{
  TCHAR t[_MAX_PATH];
  makeModulePath(t, SIZE_A(t), filename);
  delete []Filename;
  Filename = str_newdup(t);
}
//----------------------------------------------------------------------------
P_Base::P_Base(PWin* parent, uint resId, HINSTANCE hInstance)  :
    baseClass(parent, resId, -500, -500, 100, 100, 0, hInstance),
//    inherited(parent, resId, hInstance),
      needRefresh(1),
      Par(0), isBody(true)
{
  PWin *p = parent;
  while(p) {
    Par = dynamic_cast<mainClient*>(p);
    if(Par)
      break;
    p = p->getParent();
    }
}
//----------------------------------------------------------------------------
P_Base::~P_Base()
{
  destroy();
}
//----------------------------------------------------------------------------
bool P_Base::create()
{
  Attr.style |= WS_CHILD | WS_GROUP;
//  Attr.style |= WS_CHILD | WS_TABSTOP;
  if(!baseClass::create())
    return false;
//  SetClassLong(*this, GCL_STYLE, GetClassLong(*this, GCL_STYLE) & ~CS_SAVEBITS);
  return true;
}
//----------------------------------------------------------------------------
LPCTSTR P_Base::getClassName() const
{
#if 0
  return baseClass::getClassName();
#else
  return _T("BaseChild_Class");
#endif
}
//----------------------------------------------------------------------------
void P_Base::getWindowClass(WNDCLASS& wcl)
{
  baseClass::getWindowClass(wcl);
  wcl.style |= CS_PARENTDC;
//  wcl.hbrBackground = 0;
}
//----------------------------------------------------------------------------
bool P_Base::preProcessMsg(MSG& msg)
{
//  return inherited::preProcessMsg(msg) ;
  return toBool(IsDialogMessage(getHandle(), &msg));
}
//-----------------------------------------------------------
LRESULT P_Base::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SET_DIRTY_BODY:
      if(dBody == wParam)
        setDirty();
      else if(dData == wParam) {
        gestJobData* job = Par->getData();
        job->setDirty();
        }

      break;

    case WM_CLOSE:
      return 0;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void P_Base::postDirty(int which)
{
  PostMessage(*this, WM_SET_DIRTY_BODY, which, 0);
}
//-----------------------------------------------------------
int strcpyWithoutSpace(LPSTR dest, LPCSTR source)
{
  LPSTR d = dest;
  LPCSTR s = source;

  // cerca il primo carattere non blank
  while(*s <= _T(' '))
    ++s;
  LPCSTR ends = s;

  // cerca la fine della stringa
  while(*ends)
    ++ends;

  // se la stringa non era vuota o con solo spazi
  if(s != ends) {
    --ends;
    // finché trova blank e non torna all'inizio
    while(*ends <= _T(' ') && ends >= s)
      --ends;
    while(*s && s <= ends)
      *d++ = *s++;
    }
  *d = 0;
  return d - dest;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
