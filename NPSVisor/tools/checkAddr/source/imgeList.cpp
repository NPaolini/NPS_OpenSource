//----------- imageList.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "imageList.h"
#include "HeaderMsg.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "pModDialog.h"
//----------------------------------------------------------------------------
class PD_childImageList : public PD_TreeView
{
  private:
    typedef PD_TreeView baseClass;
  public:
    PD_childImageList(PWin* parent, uint resId = IDD_TREE, HINSTANCE hinstance = 0);
    virtual ~PD_childImageList();

    void setManPage(managePages* mp) { pMP = mp; fill(); }
    const PVect<myStr>& getNotUsed() { return notUsed; }
  protected:
    virtual void fill();
    virtual void fill_sub(LPNM_TREEVIEWW twn);

    virtual HIMAGELIST makeImageList();
  private:
    managePages* pMP;
    PVect<myStr> notUsed;
    void resetNotUsed();
};
//----------------------------------------------------------------------------
class PD_MoveImage : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PD_MoveImage(PWin* parent, uint resId = IDD_MOVE_BMP, HINSTANCE hinstance = 0);
    virtual ~PD_MoveImage() { destroy(); }

    void setNotUsed(const PVect<myStr>& set);
    void setManPage(managePages* mp) { pMP = mp; }
    virtual bool preProcessMsg(MSG& msg);
    virtual bool create();
    void idleTimer();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void findfolder();
    virtual void move();
    bool checkSamePath(LPCTSTR t);

  private:
    managePages* pMP;
    PVect<myStr> Set;
};
//----------------------------------------------------------------------------
PD_MoveImage::PD_MoveImage(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), pMP(0)
{
  int idBmp[] = { IDB_MOVE, IDB_FOLDER };
  int idBtn[] = { IDC_BUTTON_RUN, IDC_BUTTON_SEARCH_FILE };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true );
    }
}
//----------------------------------------------------------------------------
bool PD_MoveImage::preProcessMsg(MSG& msg)
{
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
#define ID_IDLE_TIMER 1524
//----------------------------------------------------------------------------
bool PD_MoveImage::create()
{
  if(!baseClass::create())
    return false;
  ENABLE(IDC_BUTTON_RUN, false);
  SetTimer(*this, ID_IDLE_TIMER, 500, 0);
  return true;
}
//----------------------------------------------------------------------------
static bool isFullPath(LPCTSTR t)
{
  if(_istalpha(*t) && _T(':') == t[1])
    return true;
  if(_T('\\') == *t || _T('/') == *t)
//    if(_T('\\') == t[1] || _T('/') == t[1])
      return true;
  return false;
}
//----------------------------------------------------------------------------
static void makeBaseProjectPath(LPTSTR t, LPCTSTR p)
{
  _tcscpy_s(t, MAX_PATH, p);
  int l = _tcslen(t);
  for(int i = l - 1; i > 0; --i) {
    if(_T('\\') == t[i] || _T('/') == t[i]) {
      if(!_tcsicmp(_T("system"), t + i + 1))
        t[i] = 0;
      break;
      }
    }
}
//----------------------------------------------------------------------------
static void makeImageProjectPath(LPTSTR t, LPCTSTR p, LPCTSTR base)
{
  if(isFullPath(p))
    _tcscpy_s(t, _MAX_PATH, p);
  else {
    _tcscpy_s(t, _MAX_PATH, base);
    appendPath(t, p);
    }
  int l = _tcslen(t) - 1;
  while(l > 0 && (_T('\\') == t[l] || _T('/') == t[l])) {
    t[l] = 0;
    --l;
    }
}
//----------------------------------------------------------------------------
bool PD_MoveImage::checkSamePath(LPCTSTR t)
{
  if(!pMP)
    return false;
  TCHAR p1[_MAX_PATH];
  makeBaseProjectPath(p1, pMP->getPath());

  TCHAR p2[_MAX_PATH];
  makeImageProjectPath(p2, t, p1);
  appendPath(p1, _T("image"));
  return toBool(_tcsicmp(p1, p2));
}
//----------------------------------------------------------------------------
void PD_MoveImage::idleTimer()
{
  bool enable = Set.getElem() > 0;
  if(enable) {
    TCHAR t[_MAX_PATH] = _T("");
    GET_TEXT(IDC_EDIT_PATH, t);
    trim(lTrim(t));
    enable = toBool(*t);
    if(enable)
      enable = checkSamePath(t);
    }
  ENABLE(IDC_BUTTON_RUN, enable);
}
//----------------------------------------------------------------------------
LRESULT PD_MoveImage::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      KillTimer(hwnd, ID_IDLE_TIMER);
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_RUN:
          move();
          break;
        case IDC_BUTTON_SEARCH_FILE:
          findfolder();
          break;
        }
      break;
    case WM_TIMER:
      if(ID_IDLE_TIMER == wParam)
        idleTimer();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_MoveImage::findfolder()
{
  static TCHAR path[_MAX_PATH];
  if(PChooseFolder(*this, path)) {
    SET_TEXT(IDC_EDIT_PATH, path);
//    bool enable = Set.getElem() > 0;
//    ENABLE(IDC_BUTTON_RUN, enable);
    }
}
//----------------------------------------------------------------------------
static LPCTSTR removePath(LPCTSTR p)
{
  int l = _tcslen(p);
  for(int i = l - 1; i > 0; --i)
    if(_T('\\') == p[i] || _T('/') == p[i])
      return p + i + 1;
  return p;
}
//----------------------------------------------------------------------------
void PD_MoveImage::move()
{
  TCHAR s[_MAX_PATH];
  makeBaseProjectPath(s, pMP->getPath());

  TCHAR tt[_MAX_PATH] = _T("");
  GET_TEXT(IDC_EDIT_PATH, tt);
  trim(lTrim(tt));
  TCHAR t[_MAX_PATH];
  makeImageProjectPath(t, tt, s);
  createDirectoryRecurse(t);

  appendPath(s, _T("image"));
  LPTSTR ps = s + _tcslen(s);

  LPTSTR pt = t + _tcslen(t);
  uint nElem = Set.getElem();
  for(uint i = 0; i < nElem; ++i) {
    *ps = 0;
    appendPath(s, Set[i].P);
    *pt = 0;
    LPCTSTR onlyName = removePath(Set[i].P);
    appendPath(t, onlyName);
    MoveFile(s, t);
    }
  PModDialog* m = getParentWin<PModDialog>(this);
  if(m)
    PostMessage(*m, WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
}
//----------------------------------------------------------------------------
void PD_MoveImage::setNotUsed(const PVect<myStr>& set)
{
  Set.reset();
  Set = set;
/*
  bool enable = Set.getElem() > 0;
  if(enable) {
    TCHAR t[_MAX_PATH] = _T("");
    GET_TEXT(IDC_EDIT_PATH, t);
    trim(lTrim(t));
    enable = toBool(*t);
    }
  ENABLE(IDC_BUTTON_RUN, enable);
*/
}
//----------------------------------------------------------------------------
PD_childImageList::PD_childImageList(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), pMP(0)
{}
//----------------------------------------------------------------------------
PD_childImageList::~PD_childImageList()
{
  destroy();
  resetNotUsed();
}
//----------------------------------------------------------------------------
void PD_childImageList::resetNotUsed()
{
  uint nElem = notUsed.getElem();
  for(uint i = 0; i < nElem; ++i)
    delete []notUsed[i].P;
  notUsed.reset();
}
//----------------------------------------------------------------------------
#define TV_BKG_COLOR RGB(0xe1, 0xdf, 0xbe)
#define TV_TXT_COLOR RGB(0, 0, 0xa3)
//----------------------------------------------------------------------------
HIMAGELIST PD_childImageList::makeImageList()
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
static
bool isImage(LPCTSTR pFile)
{
  int len = _tcslen(pFile);
  if(len < 5)
    return false;

  LPCTSTR extImage[] = {
    _T(".bmp"),
    _T(".jpg"),
    _T(".gif"),
    _T(".wmf"),
    _T(".emf"),
    _T(".tif"),
    _T(".png"),
    };
  LPCTSTR p = pFile + len - 4;

  for(uint i = 0; i < SIZE_A(extImage); ++i)
    if(!_tcsicmp(extImage[i], p))
      return true;

  return false;
}
//----------------------------------------------------------------------------
//#define ALLOC_NOT_FOUND
static
void addFile(LPCTSTR pFile, PVect<myStr>& notUsed, PVect<myStrImage>& notFound)
{
//  if(used.exist(pFile))
//    return;
//  myStr str(str_newdup(pFile));
//  notUsed.insert(str);

  uint pos;
  if(!notFound.find(myStrImage(pFile), pos)) {
    myStr str(str_newdup(pFile));
    notUsed.insert(str);
    return;
    }
  myStrImage str(notFound.remove(pos));
#ifdef ALLOC_NOT_FOUND
  delete []notFound[i].P;
  delete []notFound[i].Page;
#endif
}
//----------------------------------------------------------------------------
static
void recurseFillSet(LPCTSTR basePath, LPCTSTR subPath, PVect<myStr>& notUsed, PVect<myStrImage>& notFound)
{
  TCHAR oldDir[_MAX_PATH];
  GetCurrentDirectory(SIZE_A(oldDir), oldDir);

  TCHAR currDir[_MAX_PATH] = _T("");
  if(*basePath)
    _tcscpy(currDir, basePath);

  if(subPath && *subPath)
    appendPath(currDir, subPath);
  if(*currDir)
    if(!SetCurrentDirectory(currDir))
      return;
  WIN32_FIND_DATA FindFileData;
  TCHAR files[] = _T("*.*");
  HANDLE hf = FindFirstFile(files, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if(!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)) {
        LPCTSTR pFile = FindFileData.cFileName;
        if(!isImage(pFile))
          continue;
        TCHAR tmp[_MAX_PATH] = _T("");
        if(subPath && *subPath) {
          _tcscpy(tmp, subPath);
          appendPath(tmp, pFile);
          }
        else
          _tcscpy(tmp, pFile);
        addFile(tmp, notUsed, notFound);
        }
      else if(_T('.') != FindFileData.cFileName[0]) {
        TCHAR tmp[_MAX_PATH] = _T("");
        if(subPath && *subPath) {
          _tcscpy(tmp, subPath);
          appendPath(tmp, FindFileData.cFileName);
          }
        else
          _tcscpy(tmp, FindFileData.cFileName);
        recurseFillSet(basePath, tmp, notUsed, notFound);
        }
      } while(FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }

  SetCurrentDirectory(oldDir);
}
//----------------------------------------------------------------------------
void PD_childImageList::fill()
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

  TCHAR buff[500];
  HTREEITEM parent = TVI_FIRST;

  const setOfImages& nSet = MP.getSetImage();

  uint nElem = nSet.getElem();
  resetNotUsed();
  PVect<myStrImage> notFound;
  notFound.setDim(nElem);
  for(uint i = 0; i < nElem; ++i) {
#ifdef ALLOC_NOT_FOUND
    myStrImage str(str_newdup(nSet.getImage(i).P, str_newdup(nSet.getImage(i).Page));
#else
    myStrImage str(nSet.getImage(i).P, nSet.getImage(i).Page);
#endif

#if 1
    // sono già ordinate, non vale la pena di inserirle con la funzione
    // di ordinamento
    notFound[i] = str;
#else
    notFound.insert(str);
#endif
    }
  TCHAR path[_MAX_PATH];
  ::getPath(path);
  appendPath(path, _T("image"));

//  PVect<myStr> notUsed;
  recurseFillSet(path, 0, notUsed, notFound);

  for(uint i = 0; i < nElem; ++i) {
    nSet.getImage(i).fillBuff(buff);
    is.hParent = root;
    is.hInsertAfter = parent;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    is.item.hItem = 0;
    is.item.pszText = buff;
    int ixImage = 2;

    is.item.iImage = ixImage;
    is.item.iSelectedImage = ixImage;

    parent = TreeView_InsertItem(*getTree(), &is);
    }

  is.item.iImage = IMG_OK_TYPE;
  is.item.iSelectedImage = IMG_OK_TYPE;
  wsprintf(buff, _T("%s - [%d]"), _T("Images on use"), nElem);
  is.item.hItem = root;
  is.item.pszText = buff;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  TreeView_SetItem(*getTree(), &is.item);
  TreeView_Expand(*getTree(), root, TVE_EXPAND);

  nElem = notFound.getElem();
  if(nElem) {
    is.hParent = root_t;
    is.hInsertAfter = root;
    if(root_t)
      is.item.mask = TVIF_HANDLE;
    else
      is.item.mask = 0;
    is.item.hItem = root_t;

    root = TreeView_InsertItem(*getTree(), &is);
    parent = TVI_FIRST;

    for(uint i = 0; i < nElem; ++i) {
      notFound[i].fillBuff(buff);
#ifdef ALLOC_NOT_FOUND
      delete []notFound[i].P;
      delete []notFound[i].Page;
#endif
      is.hParent = root;
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
    wsprintf(buff, _T("%s - [%d]"), _T("Images requested but not found"), nElem);
    is.item.hItem = root;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    TreeView_SetItem(*getTree(), &is.item);
    }

  nElem = notUsed.getElem();
  if(nElem) {
    is.hParent = root_t;
    is.hInsertAfter = root;
    if(root_t)
      is.item.mask = TVIF_HANDLE;
    else
      is.item.mask = 0;
    is.item.hItem = root_t;

    root = TreeView_InsertItem(*getTree(), &is);
    parent = TVI_FIRST;

    for(uint i = 0; i < nElem; ++i) {
      _tcscpy(buff, notUsed[i].P);
//      delete []notUsed[i].P;
      is.hParent = root;
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
    wsprintf(buff, _T("%s - [%d]"), _T("Images not used"), nElem);
    is.item.hItem = root;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    TreeView_SetItem(*getTree(), &is.item);
    }
}
//----------------------------------------------------------------------------
void PD_childImageList::fill_sub(LPNM_TREEVIEWW twn)
{
}
//----------------------------------------------------------------------------
PD_imageList::PD_imageList(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), Split(0)
{}
//----------------------------------------------------------------------------
PD_imageList::~PD_imageList()
{
  destroy();
  delete Split;
}
//----------------------------------------------------------------------------
bool PD_imageList::create()
{
  PWin* w1 = new PD_MoveImage(this);
  PWin* w2 = new PD_childImageList(this);
  Split = new PSplitter(this, w1, w2, 300, 5, PSplitter::PSW_VERT);
  if(!baseClass::create())
    return false;
  PRect r;
  GetWindowRect(*w1, r);
  Split->setDim1(r.Height());
  Split->setLock(PSplitter::PSL_FIRST);
  return Split->create();
}
//----------------------------------------------------------------------------
bool PD_imageList::preProcessMsg(MSG& msg)
{
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
LRESULT PD_imageList::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(Split)
    Split->windowProcSplit(hwnd, message, wParam, lParam);
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_imageList::setManPage(managePages* mp)
{
  PWin* w = Split->getW2();

  PD_childImageList* iml = (PD_childImageList*)w;
  if(iml) {
    iml->setManPage(mp);
    w = Split->getW1();
    PD_MoveImage* mi = (PD_MoveImage*)w;
    if(mi) {
      mi->setManPage(mp);
      mi->setNotUsed(iml->getNotUsed());
      }
    }
}
//----------------------------------------------------------------------------
