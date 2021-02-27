//----------- winInfoList.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "winInfoList.h"
//----------------------------------------------------------------------------
#ifdef ONLY_FILL
#include <commctrl.h>
#include <stdlib.h>
#include "p_util.h"
#include "p_vect.h"
//----------------------------------------------------------------------------
struct winInfo
{
  HWND hwnd;
  LPCTSTR caption;
  winInfo(HWND h, LPCTSTR c) : hwnd(h), caption(str_newdup(c)) {}
  winInfo() : hwnd(0), caption(0) {}

  const winInfo& operator =(const winInfo& other) { clone(other); return *this; }
  winInfo(const winInfo& other) : caption(0) { clone(other); }

  ~winInfo() { delete []caption; }

  private:
    void clone(const winInfo& other);
};
//----------------------------------------------------------------------------
typedef PVect<winInfo> setOfInfo;
//----------------------------------------------------------------------------
struct winList
{
  HWND owner;
  setOfInfo set;
};
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
void winInfo::clone(const winInfo& other)
{
  if(&other != this) {
    delete []caption;
    caption = str_newdup(other.caption);
    hwnd = other.hwnd;
    }
}
//----------------------------------------------------------------------------
bool existInList(HWND hwnd, const setOfInfo& set)
{
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i)
    if(set[i].hwnd == hwnd)
      return true;
  return false;
}
//----------------------------------------------------------------------------
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  winList* wl = reinterpret_cast<winList*>(lParam);
  if(hwnd != wl->owner && IsWindowVisible(hwnd)) {
#if 1
    HWND parent = GetAncestor(hwnd, GA_ROOT);
    if(!parent)
      parent = hwnd;
#else
    HWND parent = hwnd;
    while(GetParent(parent))
      parent = GetParent(parent);
#endif
    const setOfInfo& set = wl->set;
    if(!existInList(parent, set)) {
      TCHAR caption[500];
      GetWindowText(parent, caption, SIZE_A(caption));
      if(*caption) {
        if(_tcsicmp(_T("Program Manager"), caption)) {
          int nElem = wl->set.getElem();
          wl->set[nElem] = winInfo(parent, caption);
          }
        }
      }
    }
  return TRUE;
}
//----------------------------------------------------------------------------
//static int lastLen;
//----------------------------------------------------------------------------
void addString(HWND hList, LPCTSTR str, bool isLB)
{
  int wid = extent(hList, str);
  int msgGet;
  int msgSet;
  if(isLB) {
    msgGet = LB_GETHORIZONTALEXTENT;
    msgSet = LB_SETHORIZONTALEXTENT;
    }
  else  {
    msgGet = CB_GETDROPPEDWIDTH;
    msgSet = CB_SETDROPPEDWIDTH;
    }

  int awi = SendMessage(hList, msgGet, 0, 0);
  if(wid > awi) {
    SendMessage(hList, msgSet, wid, 0);
    }
  SendMessage(hList, isLB ? LB_ADDSTRING : CB_ADDSTRING, 0, LPARAM(str));
}
//------------------------------------------------------------------------------
bool getListHwnd(winList& WList)
{
  return toBool(EnumWindows(EnumWindowsProc, (LPARAM)&WList));
}
//------------------------------------------------------------------------------
void fillListHwnd(PWin*owner, uint idc, bool isLB)
{
  winList WList;
  WList.owner = *owner;
  if(getListHwnd(WList)) {
    HWND hcb = GetDlgItem(*owner, idc);
    SendMessage(hcb, isLB ? LB_RESETCONTENT : CB_RESETCONTENT, 0, 0);
    const setOfInfo& set = WList.set;
    int nElem = set.getElem();
    TCHAR buff[500];
//    lastLen = 0;
    for(int i = 0; i < nElem; ++i) {
      wsprintf(buff, _T("H%X - %s"), set[i].hwnd, set[i].caption);
      addString(hcb, buff, isLB);
      }
    }
}
//------------------------------------------------------------------------------
/*
  Specifiche ricerca SVisor.

  Crea la lista delle window attive.
  Ricerca nella lista se una window si trova nel path di lavoro (GetWindowModuleFileName)
  Cerca di individuare se si tratta di un SVisor.
  In caso di dubbio propone le alternative.
  Se nessun SVisor è attivo propone di avviarlo.
    Riesegue la ricerca e attiva il SVisor.
*/
//------------------------------------------------------------------------------
//----------------------------------------------------------------------------
/*
class PD_chooseHandle : public PModDialog
{
  public:
    PD_chooseHandle(const setOfInfo& set, PVect<uint>& found, PWin* parent,
        uint resId = IDD_DIALOG_HANDLE, HINSTANCE hinstance = 0);
    virtual ~PD_chooseHandle();

    virtual bool create();
    uint getResult() { return Result; }
  protected:
    virtual void CmOk();
//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    const setOfInfo& Set;
    PVect<uint>& Found;
    uint Result;
    typedef PModDialog baseClass;
};
*/
//----------------------------------------------------------------------------
PD_chooseHandle::PD_chooseHandle(const setOfInfo& set, PVect<uint>& found, PWin* parent,
        uint resId, HINSTANCE hinstance) :
        baseClass(parent, resId, hinstance), Set(set), Found(found), Result(0) {}
//----------------------------------------------------------------------------
bool PD_chooseHandle::create()
{
  if(!baseClass::create())
    return false;
  SetWindowText(*this, _T("Scelta SVisor"));

  HWND hlist = GetDlgItem(*this, IDC_LIST_SV);

  SendMessage(hlist, LB_RESETCONTENT, 0, 0);
  int nElem = Found.getElem();
  TCHAR buff[500];
//  lastLen = 0;
  for(int i = 0; i < nElem; ++i) {
    uint ix = Found[i];
    wsprintf(buff, _T("H%X - %s"), Set[ix].hwnd, Set[ix].caption);
    addString(hlist, buff, true);
    }
  return true;
}
//----------------------------------------------------------------------------
void PD_chooseHandle::CmOk()
{
  HWND hlist = GetDlgItem(*this, IDC_LIST_SV);

  Result = SendMessage(hlist, LB_GETCURSEL, 0, 0);
  if(Result >= 0)
    baseClass::CmOk();
}
//----------------------------------------------------------------------------
/*
//----------------------------------------------------------------------------
class PD_chooseFile : public PModDialog
{
  public:
    PD_chooseHandle(LPTSTR path, PWin* parent,
        uint resId = IDD_DIALOG_HANDLE, HINSTANCE hinstance = 0);
    virtual ~PD_chooseHandle();

    virtual bool create();
  protected:
    virtual void CmOk();
//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    LPTSTR Path;
    typedef PModDialog baseClass;
};
*/
//----------------------------------------------------------------------------
PD_chooseFile::PD_chooseFile(LPTSTR path, PWin* parent,
        uint resId, HINSTANCE hinstance) :
        baseClass(parent, resId, hinstance), Path(path) {}
//----------------------------------------------------------------------------
bool PD_chooseFile::create()
{
  if(!baseClass::create())
    return false;
  SetWindowText(*this, _T("Scelta SVisor da avviare"));

  HWND hlist = GetDlgItem(*this, IDC_LIST_SV);

  SendMessage(hlist, LB_RESETCONTENT, 0, 0);
  TCHAR path[_MAX_PATH];
  _tcscpy(path, Path);
  appendPath(path, _T("NPS_*.exe"));

  WIN32_FIND_DATA FindFileData;
  HANDLE hf = FindFirstFile(path, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if(FILE_ATTRIBUTE_DIRECTORY != FindFileData.dwFileAttributes) {
        _tcscpy(path, FindFileData.cFileName);
        addString(hlist, path, true);
        }
      } while(FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }
  return true;
}
//----------------------------------------------------------------------------
void PD_chooseFile::CmOk()
{
  HWND hlist = GetDlgItem(*this, IDC_LIST_SV);

  int sel = SendMessage(hlist, LB_GETCURSEL, 0, 0);
  if(sel >= 0) {
    TCHAR path[_MAX_PATH];
    SendMessage(hlist, LB_GETTEXT, sel, (LPARAM)path);
    appendPath(Path, path);
    baseClass::CmOk();
    }
}
//----------------------------------------------------------------------------
