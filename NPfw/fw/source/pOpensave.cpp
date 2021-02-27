//------ pOpensave.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include<commdlg.h>
#include<stdlib.h>
#include <shlobj.h>
#include "pOpensave.h"
#include "simple_helper.h"
#include "p_util.h"
//----------------------------------------------------------------------------
//#define TEST_FILTER
// se definito occorre includere anche la shlwapi.lib

#ifdef TEST_FILTER
#ifndef OFN_ENABLEINCLUDENOTIFY
  #define OFN_ENABLEINCLUDENOTIFY      0x00400000
  #define CDN_INCLUDEITEM         (CDN_FIRST - 0x0007)
#endif
//----------------------------------------------------------------------------
UINT CALLBACK
OpenFileCallbackProc(HWND hwnd, UINT uMsg, WPARAM lp, LPARAM pData);
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
#define MAX_DIM_BUFF_OPEN_M 1000000
//----------------------------------------------------------------------------
void POpenSave::setInitialFile(infoOpenSave& info)
{
  if(info.getFile())
    _tcscpy_s(filename, info.getFile());
  else
    filename[0] = 0;
}
//----------------------------------------------------------------------------
bool POpenSave::run(infoOpenSave& info)
{
  TCHAR currPath[_MAX_PATH];

  GetCurrentDirectory(_MAX_PATH, currPath);

  setPathFromHistory();

  setInitialFile(info);

  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = Owner;
  ofn.lCustData = (LPARAM)this;
  ofn.lpstrInitialDir = _T(".");
#ifdef TEST_FILTER
  ofn.Flags = OFN_LONGNAMES | OFN_EXPLORER | OFN_ENABLEINCLUDENOTIFY | OFN_ENABLEHOOK;
  ofn.lpfnHook = OpenFileCallbackProc;
#else
  ofn.Flags = OFN_LONGNAMES | OFN_EXPLORER;
#endif
  if(infoOpenSave::OPEN_F_MULTIPLE == info.getAction()) {
    if(!pMultipleFiles)
      pMultipleFiles = new TCHAR[MAX_DIM_BUFF_OPEN_M];
    pMultipleFiles[0] = 0;
    ofn.lpstrFile = pMultipleFiles;
    ofn.nMaxFile = MAX_DIM_BUFF_OPEN_M;
    }
  else {
    delete []pMultipleFiles;
    pMultipleFiles = 0;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = _MAX_PATH;
    }
  ofn.lpstrFilter = info.getFilter();

  bool success = false;
  switch(info.getAction()) {
    case infoOpenSave::OPEN_F_MULTIPLE:
      ofn.Flags |= OFN_ALLOWMULTISELECT;
      // fall through

    case infoOpenSave::OPEN_F:
      ofn.Flags |= OFN_FILEMUSTEXIST;
    case infoOpenSave::OPEN_NO_EXIST:
      success = toBool(GetOpenFileName(&ofn));
      break;
    case infoOpenSave::SAVE_AS:
      ofn.Flags |= OFN_OVERWRITEPROMPT;
      ofn.nFilterIndex = info.ixFilter + 1;
      success = toBool(GetSaveFileName(&ofn));
      break;
    }
  if(success) {
    if(infoOpenSave::OPEN_F_MULTIPLE == info.getAction()) {
      if(ofn.nFileOffset)
        pMultipleFiles[ofn.nFileOffset - 1] = 0;
      }
    else {
      TCHAR Ext[_MAX_PATH];
      _tsplitpath_s(filename, 0, 0, 0, 0, 0, 0, Ext, SIZE_A(Ext));
      if(!*Ext) {
        if(infoOpenSave::OPEN_F == info.getAction())
          success = false;
        else {
          if(ofn.nFilterIndex > 0)
            info.ixFilter = ofn.nFilterIndex - 1;
          LPCTSTR vExt = info.getExt()[info.ixFilter];
          // visto che, es. per le ricette, usiamo la doppia estensione (.rcp.txt)
          // in caso non sia stata immessa, occorre copiarcela tutta
          while(*vExt && *vExt != _T('.'))
            ++vExt;
          if(vExt)
            _tcscat_s(filename, SIZE_A(filename), vExt);
          }
        }
      else {
        // se è stata immessa una estensione, dobbiamo usare lo stesso metodo per estrarla dalla lista
        LPCTSTR* vExt = info.getExt();
        TCHAR ext[_MAX_PATH];
        for(int i = 0; vExt[i]; ++i) {
          _tsplitpath_s(vExt[i], 0, 0, 0, 0, 0, 0, ext, SIZE_A(ext));
          if(!_tcsicmp(Ext, ext)) {
            info.ixFilter = i;
            break;
            }
          }
        }
      }
    }

  savePathToHistory();

  SetCurrentDirectory(currPath);
  return success;
}
//----------------------------------------------------------------------------
#ifdef TEST_FILTER
// Structure used for all object based OpenFileName notifications
typedef struct _OFNOTIFYEXA
{
        NMHDR           hdr;
        LPOPENFILENAMEA lpOFN;
        LPVOID          psf;
        LPVOID          pidl;          // May be NULL
} OFNOTIFYEXA, FAR *LPOFNOTIFYEXA;
// Structure used for all object based OpenFileName notifications
typedef struct _OFNOTIFYEXW
{
        NMHDR           hdr;
        LPOPENFILENAMEW lpOFN;
        LPVOID          psf;
        LPVOID          pidl;          // May be NULL
} OFNOTIFYEXW, FAR *LPOFNOTIFYEXW;
#ifdef UNICODE
typedef OFNOTIFYEXW OFNOTIFYEX;
typedef LPOFNOTIFYEXW LPOFNOTIFYEX;
#else
typedef OFNOTIFYEXA OFNOTIFYEX;
typedef LPOFNOTIFYEXA LPOFNOTIFYEX;
#endif // UNICODE
//----------------------------------------------------------------------------
#ifndef WINSHLWAPI
#if !defined(_SHLWAPI_)
#define LWSTDAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define LWSTDAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#define LWSTDAPIV         EXTERN_C DECLSPEC_IMPORT HRESULT STDAPIVCALLTYPE
#define LWSTDAPIV_(type)  EXTERN_C DECLSPEC_IMPORT type STDAPIVCALLTYPE
#else
#define LWSTDAPI          STDAPI
#define LWSTDAPI_(type)   STDAPI_(type)
#define LWSTDAPIV         STDAPIV
#define LWSTDAPIV_(type)  STDAPIV_(type)
#endif
#endif // WINSHLWAPI
//----------------------------------------------------------------------------
LWSTDAPI StrRetToBufA(struct _STRRET *pstr, const UNALIGNED struct _ITEMIDLIST *pidl, LPSTR pszBuf, UINT cchBuf);
LWSTDAPI StrRetToBufW(struct _STRRET *pstr, const UNALIGNED struct _ITEMIDLIST *pidl, LPWSTR pszBuf, UINT cchBuf);
#ifdef UNICODE
#define StrRetToBuf  StrRetToBufW
#else
#define StrRetToBuf  StrRetToBufA
#endif // !UNICODE
//----------------------------------------------------------------------------
static
bool manageNotify(LPOFNOTIFYEX pData, int& result)
{
  bool needDefault = true;
  switch(((NMHDR *)pData)->code) {
    case CDN_INCLUDEITEM:
      needDefault = false;
      do {
        STRRET str;
        ZeroMemory(&str, sizeof(str));
        str.uType = STRRET_CSTR;
        IShellFolder* isf = (IShellFolder*)(pData->psf);
        LPCITEMIDLIST pidl = (LPCITEMIDLIST)(pData->pidl);
        result = 1;
        if(NOERROR == isf->GetDisplayNameOf(pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, &str)) {
          POpenSave* owner = (POpenSave*)(pData->lpOFN->lCustData);
          TCHAR buff[_MAX_PATH];
          if(S_OK == StrRetToBuf(&str, pidl, buff, SIZE_A(buff))) {
#if 1
            if(*buff == 'B' || *buff == 'b')
//              SFGAO_FILESYSTEM
              result = 0;
#else
            result = owner->acceptItem(buff);
#endif
            }
          }
        } while(false);
      break;
    }
  return !needDefault;
}
//----------------------------------------------------------------------------
UINT CALLBACK
OpenFileCallbackProc(HWND hwnd, UINT uMsg, WPARAM lp, LPARAM pData)
{
  switch(uMsg) {
    case WM_NOTIFY:
      do {
        int result = 0;
        if(manageNotify((LPOFNOTIFYEX)pData, result))
          return result;
        } while(false);

    default:
      break;
    }
  return 0;
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int CALLBACK
BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
  TCHAR szDir[_MAX_PATH];

  switch(uMsg) {
    case BFFM_INITIALIZED:
      if(GetCurrentDirectory(sizeof(szDir)/sizeof(TCHAR), szDir)) {
                  // WParam is TRUE since you are passing a path.
                  // It would be FALSE if you were passing a pidl.
        if(pData)
          appendPath(szDir, (LPCTSTR)pData);

        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szDir);
        SendMessage(hwnd, BFFM_SETEXPANDED, TRUE, (LPARAM)szDir);
        }
      break;

    case BFFM_SELCHANGED:
               // Set the status window to the currently selected path.
      if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
        SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
      break;

    default:
      break;
    }
  return 0;
}
//----------------------------------------------------------------------------
//-----------------------------------------------------------
LPITEMIDLIST getITEMIDLIST(LPTSTR path)
{
  LPITEMIDLIST pidl = 0;
  simple_com_ptr<IShellFolder> iShellFolder;
  HRESULT hres =  CoCreateInstance(CLSID_ShellDesktop, NULL, CLSCTX_INPROC,
            IID_IShellFolder, iShellFolder);
  if(SUCCEEDED(hres)) {

    TCHAR full[_MAX_PATH];
    _tfullpath(full, path, _MAX_PATH);
    simple_BSTR bstrFile(full);
    ULONG len;
    hres = iShellFolder->ParseDisplayName(0, 0, bstrFile, &len, &pidl, 0);
    }
  return SUCCEEDED(hres) ? pidl : 0;
}
//----------------------------------------------------------------------------
static
bool performPChooseFolder_old(LPTSTR path, LPCTSTR title, bool onlySub, LPCTSTR relPath, HWND owner)
{
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if(FAILED(hr)) {
    return false;
    }
  LPMALLOC  lpMalloc;
  hr = SHGetMalloc(&lpMalloc);
  if(FAILED(hr)) {
    CoUninitialize();
    return false;
    }

  TCHAR currPath[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, currPath);

  if(*path)
    SetCurrentDirectory(path);

  BROWSEINFO bi;
  memset(&bi, 0, sizeof(bi));
  bi.hwndOwner = owner;
  LPITEMIDLIST lpidl = 0;
  if(onlySub && *path) {
    lpidl = getITEMIDLIST(path);
    }
  bi.pidlRoot = lpidl;
  bi.pszDisplayName = path;
  bi.lpszTitle = title;
  if(!title)
    bi.lpszTitle = _T("Find Working Folder");
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS /*| BIF_STATUSTEXT*/ | BIF_USENEWUI;

  bi.lpfn = BrowseCallbackProc;
  bi.lParam = (LPARAM)relPath;
  LPITEMIDLIST lpi = SHBrowseForFolder(&bi);

  bool success = lpi != 0;
  if(success) {
    SHGetPathFromIDList(lpi, path);
    lpMalloc->Free(lpi);
    }
  if(lpidl)
    lpMalloc->Free(lpidl);
  lpMalloc->Release();
  SetCurrentDirectory(currPath);
  CoUninitialize();
  return success;
}
//----------------------------------------------------------------------------
BOOL IsWindowsVistaOrGreater ()
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;
   int op=VER_GREATER_EQUAL;

   // Initialize the OSVERSIONINFOEX structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 6;
   osvi.dwMinorVersion = 0;
   osvi.wServicePackMajor = 2;
   osvi.wServicePackMinor = 0;

   // Initialize the condition mask.

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR, op );

   // Perform the test.

   return VerifyVersionInfo(
      &osvi,
      VER_MAJORVERSION | VER_MINORVERSION |
      VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
      dwlConditionMask);
}
//----------------------------------------------------------------------------
static
bool performPChooseFolder(LPTSTR path, LPCTSTR title, bool onlySub, LPCTSTR relPath, HWND owner)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    bool success = false;
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
          DWORD dwOptions;
          if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions)))
          {
              pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
          }
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem *pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    LPWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                      copyStrZ(path, pszFilePath);
//                        MessageBox(NULL, pszFilePath, _T("File Path"), MB_OK);
                        CoTaskMemFree(pszFilePath);
                        success = true;
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    return success;
}
//----------------------------------------------------------------------------
bool PChooseFolder(LPTSTR path, LPCTSTR title, bool onlySub, LPCTSTR relPath, HWND owner)
{
  if(IsWindowsVistaOrGreater())
    return performPChooseFolder(path, title, onlySub, relPath, owner);
  return performPChooseFolder_old(path, title, onlySub, relPath, owner);
}
//----------------------------------------------------------------------------
bool PChooseFolder(HWND owner, LPTSTR path, LPCTSTR title, bool onlySub, LPCTSTR relPath)
{
  if(IsWindowsVistaOrGreater())
    return performPChooseFolder(path, title, onlySub, relPath, owner);
  return performPChooseFolder_old(path, title, onlySub, relPath, owner);
}
//----------------------------------------------------------------------------
