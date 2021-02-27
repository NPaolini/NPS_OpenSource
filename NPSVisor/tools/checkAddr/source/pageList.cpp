//----------- pageList.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pageList.h"
#include "HeaderMsg.h"
#include "smartPS.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PD_pageList::PD_pageList(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), pMP(0)
{}
//----------------------------------------------------------------------------
PD_pageList::~PD_pageList()
{
  destroy();
}
//----------------------------------------------------------------------------
#define TV_BKG_COLOR RGB(0xe1, 0xdf, 0xbe)
#define TV_TXT_COLOR RGB(0, 0, 0xa3)
//----------------------------------------------------------------------------
HIMAGELIST PD_pageList::makeImageList()
{
  getTree()->setFont(D_FONT(16, 0, 0, _T("courier new")), true);
  HIMAGELIST imageList = ImageList_Create(CX_IMAGE_ADDR, CY_IMAGE_ADDR, ILC_COLOR | ILC_MASK, 24, 0);

  PBitmap bmp(IDB_FULL, getHInstance());
  ImageList_AddMasked(imageList, bmp, RGB(192, 192, 192));

  TreeView_SetImageList(*getTree(), imageList, 0);
  TreeView_SetBkColor(*getTree(), TV_BKG_COLOR);
  TreeView_SetTextColor(*getTree(), TV_TXT_COLOR);

  return imageList;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void formatPageRecurse(LPTSTR buff, const infoPages* ip, bool first)
{
  if(!first)
    wsprintf(buff + _tcslen(buff), _T(" <- %s"), ip->getPageCurr());
  uint nElem = ip->getPageParent().getElem();
  if(!nElem)
    return;
  for(uint i = 0; i < nElem; ++i) {
    if(ip->getLevel() > ip->getPageParent()[i]->getLevel()) {
      formatPageRecurse(buff, ip->getPageParent()[i], false);
      break;
      }
    }
}
//----------------------------------------------------------------------------
static void formatPage(LPTSTR buff, const infoPages* ip)
{
  int level = ip->getLevel();
  if(level >= MAX_LEVEL)
    wsprintf(buff, _T("[------] - %s"), ip->getPageCurr());
  else
    wsprintf(buff, _T("[lev.%2d] - %s"), level, ip->getPageCurr());
  formatPageRecurse(buff, ip, true);
}
//----------------------------------------------------------------------------
static
bool isTruePage(const infoPages* ip)
{
  LPCTSTR p = ip->getPageCurr();
  int len = _tcslen(p);
  if(len < 5)
    return false;
  return !_tcsicmp(PAGE_EXT, p + len - 4);
}
//----------------------------------------------------------------------------
static
bool isFileReserved(LPCTSTR pFile, LPCTSTR pMatch, bool single)
{
  size_t len = _tcslen(pMatch);

  if(!_tcsnicmp(pMatch, pFile, len)) {
    LPCTSTR t = pFile + len;
    if(_istdigit((unsigned)t[0]) && !_tcsnicmp(PAGE_EXT, t + 1, 4))
      return true;
    if(single)
      return false;
    // c'è la doppia identificazione, filename_prph_language, quindi salta 'prph_'
    t += 2;
    // la lingua potrebbe essere 10, quindi controlla se inizia con un numero e poi se
    // segue un .txt subito dopo o al massimo un carattere dopo
    if(_istdigit((unsigned)t[0]) && (!_tcsnicmp(PAGE_EXT, t + 1, 4) || !_tcsnicmp(PAGE_EXT, t + 2, 4)))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
static
bool isReserved(LPCTSTR pFile)
{
  LPCTSTR noInclude[] = {
    _T("config_9x") PAGE_EXT,
    _T("config_nt")PAGE_EXT,
    _T("config")PAGE_EXT,
    _T("infoAlarm")PAGE_EXT,
    _T("alarm")PAGE_EXT,
    STD_MSG_NAME,
    _T("globalPageText")PAGE_EXT,
    };

  for(uint i = 0; i < SIZE_A(noInclude); ++i)
    if(!_tcsicmp(noInclude[i], pFile))
      return true;

  if(isFileReserved(pFile, _T("alarm_"), true))
    return true;

  if(isFileReserved(pFile, _T("infoAlarm_"), false))
    return true;

  return false;
}
//----------------------------------------------------------------------------
static
void addFile(LPCTSTR pFile, PVect<myStr>& notUsed, const setOfPages& used, const PVect<myStr>& nStdPage)
{
  if(used.existPage(pFile))
    return;
  myStr str(pFile);
  uint pos;
  if(nStdPage.find(str, pos))
    return;

  str.P = str_newdup(pFile);
  if(!notUsed.insert(str))
    delete []str.P;
}
//----------------------------------------------------------------------------
static
void recurseFillSet(LPCTSTR basePath, LPCTSTR subPath, PVect<myStr>& notUsed, const setOfPages& used, const PVect<myStr>& nStdPage)
{
  smartPointerString oldDir(new TCHAR[_MAX_PATH], true);
  GetCurrentDirectory(_MAX_PATH, oldDir);

  smartPointerString currDir(new TCHAR[_MAX_PATH], true);

  if(*basePath)
    _tcscpy_s(currDir, _MAX_PATH, basePath);

  if(subPath && *subPath)
    appendPath(currDir, subPath);
  if(*currDir)
    if(!SetCurrentDirectory(currDir))
      return;
  WIN32_FIND_DATA FindFileData;
  TCHAR files[] = _T("*" PAGE_EXT);
  HANDLE hf = FindFirstFile(files, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if(!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)) {
        LPCTSTR pFile = _T('#') == *FindFileData.cFileName ? FindFileData.cFileName + 1 : FindFileData.cFileName;
        if(isReserved(pFile))
          continue;
        smartPointerString tmp(new TCHAR[_MAX_PATH], true);
        if(subPath && *subPath) {
          _tcscpy_s(tmp, _MAX_PATH, subPath);
          appendPath(tmp, pFile);
          }
        else
          _tcscpy_s(tmp, _MAX_PATH, pFile);
        addFile(tmp, notUsed, used, nStdPage);
        }
      } while(FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }

  TCHAR dirs[] = _T("*.*");
  hf = FindFirstFile(dirs, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if((FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) && _T('.') != FindFileData.cFileName[0]) {
        smartPointerString tmp(new TCHAR[_MAX_PATH], true);
        if(subPath && *subPath) {
          _tcscpy_s(tmp, _MAX_PATH, subPath);
          appendPath(tmp, FindFileData.cFileName);
          }
        else
          _tcscpy_s(tmp, _MAX_PATH, FindFileData.cFileName);
        recurseFillSet(basePath, tmp, notUsed, used, nStdPage);
        }
      } while(FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }

  SetCurrentDirectory(oldDir);
}
//----------------------------------------------------------------------------
void PD_pageList::fill()
{
  TreeView_DeleteAllItems(*getTree());
  if(!pMP)
    return;
  managePages& MP = *pMP;
  HTREEITEM root_t = TreeView_GetRoot(*getTree());
  TV_INSERTSTRUCT is;
  is.hParent = root_t;
  is.hInsertAfter = TVI_FIRST;
  if(root_t)
    is.item.mask = TVIF_HANDLE;
  else
    is.item.mask = 0;
  is.item.hItem = root_t;

  HTREEITEM root = TreeView_InsertItem(*getTree(), &is);

  TCHAR buff[5000];
  HTREEITEM parent = TVI_FIRST;

  const setOfPages& nSet = MP.getSetPage();

  uint nElem = nSet.getElem();
  uint realPage = 0;
  for(uint i = 0; i < nElem; ++i) {
    if(!isTruePage(nSet.getPageInfo(i)))
      continue;
    formatPage(buff, nSet.getPageInfo(i));
    is.hParent = root;
    is.hInsertAfter = parent;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    is.item.hItem = 0;
    is.item.pszText = buff;
    int ixImage = 2;

    is.item.iImage = ixImage;
    is.item.iSelectedImage = ixImage;

    parent = TreeView_InsertItem(*getTree(), &is);
    ++realPage;
    }
  is.item.iImage = IMG_OK_TYPE;
  is.item.iSelectedImage = IMG_OK_TYPE;
  wsprintf(buff, _T("%s - [%d]"), _T("Pages on use"), realPage);
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  TreeView_SetItem(*getTree(), &is.item);
  TreeView_Expand(*getTree(), root, TVE_EXPAND);


  const PVect<myStr>& nStdPage = MP.getStdPage();
  nElem = nStdPage.getElem();
  HTREEITEM root2 = root;
  if(nElem) {
    is.hParent = root_t;
    is.hInsertAfter = root;
    if(root_t)
      is.item.mask = TVIF_HANDLE;
    else
      is.item.mask = 0;
    is.item.hItem = root_t;

    root2 = TreeView_InsertItem(*getTree(), &is);
    parent = TVI_FIRST;

    for(uint i = 0; i < nElem; ++i) {
      _tcscpy(buff, nStdPage[i].P);
      is.hParent = root2;
      is.hInsertAfter = parent;
      is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
      is.item.hItem = 0;
      is.item.pszText = buff;
      int ixImage = IMG_OK_TYPE;

      is.item.iImage = ixImage;
      is.item.iSelectedImage = ixImage;

      parent = TreeView_InsertItem(*getTree(), &is);
      }
    is.item.iImage = IMG_OK_TYPE;
    is.item.iSelectedImage = IMG_OK_TYPE;
    wsprintf(buff, _T("%s - [%d]"), _T("Pages on base_data"), nElem);
    is.item.hItem = root2;
    is.item.pszText = buff;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    TreeView_SetItem(*getTree(), &is.item);
    }


  TCHAR path[_MAX_PATH];
  ::getPath(path);
  appendPath(path, _T("system"));

  PVect<myStr> notUsed;
//  recurseFillSet(path, 0, notUsed, nSet);
  recurseFillSet(path, 0, notUsed, nSet, nStdPage);
  nElem = notUsed.getElem();
  if(nElem) {
    is.hParent = root_t;
    is.hInsertAfter = root2;
    if(root_t)
      is.item.mask = TVIF_HANDLE;
    else
      is.item.mask = 0;
    is.item.hItem = root_t;

    HTREEITEM root2 = TreeView_InsertItem(*getTree(), &is);
    parent = TVI_FIRST;

    for(uint i = 0; i < nElem; ++i) {
      _tcscpy(buff, notUsed[i].P);
      delete []notUsed[i].P;
      is.hParent = root2;
      is.hInsertAfter = parent;
      is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
      is.item.hItem = 0;
      is.item.pszText = buff;
      int ixImage = IMG_WARN_TYPE;

      is.item.iImage = ixImage;
      is.item.iSelectedImage = ixImage;

      parent = TreeView_InsertItem(*getTree(), &is);
      }
    is.item.iImage = IMG_WARN_TYPE;
    is.item.iSelectedImage = IMG_WARN_TYPE;
    wsprintf(buff, _T("%s - [%d]"), _T("Pages not used"), nElem);
    is.item.hItem = root2;
    is.item.pszText = buff;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    TreeView_SetItem(*getTree(), &is.item);
    }

}
//----------------------------------------------------------------------------
void PD_pageList::fill_sub(LPNM_TREEVIEWW twn)
{
}
//----------------------------------------------------------------------------
