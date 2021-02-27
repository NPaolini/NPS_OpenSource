//----------- winInfoList.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef winInfoList_H_
#define winInfoList_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "PModDialog.h"
#include "resource.h"
//----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL  (WINAPI *prtEnumProcesses)(
    DWORD * lpidProcess,
    DWORD   cb,
    DWORD * cbNeeded
    );

typedef BOOL (WINAPI *prtEnumProcessModules)(
    HANDLE hProcess,
    HMODULE *lphModule,
    DWORD cb,
    LPDWORD lpcbNeeded
    );

typedef DWORD (WINAPI *prtGetModuleFileNameExW)(
    HANDLE hProcess,
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    );

#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------
//#define ONLY_FILL
//----------------------------------------------------------------------------
#ifdef ONLY_FILL
//----------------------------------------------------------------------------
void fillListHwnd(Pwin* owner, uint idc, bool isLB);
//----------------------------------------------------------------------------
#else
#include <commctrl.h>
#include <stdlib.h>
#include "p_util.h"
#include "p_vect.h"
//----------------------------------------------------------------------------
void fillListHwnd(PWin* owner, uint idc, bool isLB);
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
bool getListHwnd(winList& WList);
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
class PD_chooseHandle : public PModDialog
{
  public:
    PD_chooseHandle(const setOfInfo& set, PVect<uint>& found, PWin* parent,
        uint resId = IDD_DIALOG_HANDLE, HINSTANCE hinstance = 0);
    virtual ~PD_chooseHandle() { destroy(); }

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
//----------------------------------------------------------------------------
class PD_chooseFile : public PModDialog
{
  public:
    PD_chooseFile(LPTSTR path, PWin* parent,
        uint resId = IDD_DIALOG_HANDLE, HINSTANCE hinstance = 0);
    virtual ~PD_chooseFile() { destroy(); }

    virtual bool create();
  protected:
    virtual void CmOk();
//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    LPTSTR Path;
    typedef PModDialog baseClass;
};
//----------------------------------------------------------------------------
#endif