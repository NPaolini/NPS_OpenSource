//----------- PDiagCheckaddr.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "PDiagCheckaddr.h"
//----------------------------------------------------------------------------
uint registeredMsg = RegisterWindowMessage(_T("NP_CHECK_ADDR_TO_SV_MAKER"));
//----------------------------------------------------------------------------
#define DEF_FILE_FOR_CHECKADDR _T("npsvMaker4checkAddr_fm")
//----------------------------------------------------------------------------
extern void saveRegCheckAddr(LPCTSTR buff);
extern void loadRegCheckAddr(LPTSTR buff, size_t dim);
//----------------------------------------------------------------------------
PDiagCheckaddr::PDiagCheckaddr(PWin* parent, HWND remote, PVect<varsFilter>& vFlt, bool refresh, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), Tree(0), ImageList(0), vFlt(vFlt), mp(0), hRemote(remote), refresh(refresh)
{
  Tree = new PTreeView(this, IDC_TREE1);
}
//----------------------------------------------------------------------------
PDiagCheckaddr::~PDiagCheckaddr()
{
  SendMessage(hRemote, registeredMsg, MAKEWPARAM(CM_END_COMM, 0), 0);
  destroy();
  if(ImageList)
    ImageList_Destroy(ImageList);
  if(mp) {
    TCHAR t[_MAX_PATH];
    _tcscpy_s(t, mp->get_name());
    delete mp;
    DeleteFile(t);
    }
}
//----------------------------------------------------------------------------
bool PDiagCheckaddr::create()
{
  if(!baseClass::create())
    return false;

  ImageList = makeImageList();

  TCHAR buff[256];
  loadRegCheckAddr(buff, SIZE_A(buff));
  int maximized = 0;
  if(*buff) {
    int x;
    int y;
    int w;
    int h;
    _stscanf_s(buff, _T("%d,%d,%d,%d,%d"), &x, &y, &w, &h, &maximized);

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    if(x < 0)
      x = 0;
    if(y < 0)
      y = 0;
    if(x + w > width)
      x = width - w;
    if(y + h > height) {
      y = height - h;
      if(y < 0)
        y = 0;
      }
    SetWindowPos(*this, 0, x, y, w, h, SWP_NOZORDER);
    if(maximized)
      ShowWindow(*this, SW_MAXIMIZE);
    }
  resize();
  fill();

  return true;
}
//----------------------------------------------------------------------------
#define TV_BKG_COLOR RGB(0xf8, 0xe4, 0xb3)
#define TV_TXT_COLOR RGB(0, 0, 0xb3)
//----------------------------------------------------------------------------
#define CX_IMAGE_ADDR 32
#define CY_IMAGE_ADDR 24
//----------------------------------------------------------------------------
HIMAGELIST PDiagCheckaddr::makeImageList()
{
  Tree->setFont(D_FONT(16, 0, 0, _T("courier new")), true);
  HIMAGELIST imageList = ImageList_Create(CX_IMAGE_ADDR, CY_IMAGE_ADDR, ILC_COLOR | ILC_MASK, 20, 0);

  PBitmap bmp(IDB_CHECK_ADDR, getHInstance());
  ImageList_AddMasked(imageList, bmp, RGB(192, 192, 192));

  TreeView_SetImageList(*Tree, imageList, 0);
  TreeView_SetBkColor(*Tree, TV_BKG_COLOR);
  TreeView_SetTextColor(*Tree, TV_TXT_COLOR);

  return imageList;
}
//----------------------------------------------------------------------------
void PDiagCheckaddr::resize()
{
  PRect r;
  GetClientRect(*this, r);
  Tree->setWindowPos(0, r, SWP_NOZORDER);
}
//----------------------------------------------------------------------------
void PDiagCheckaddr::fill()
{
  DWORD nElem = vFlt.getElem();
  TCHAR path[_MAX_PATH];
  GetTempPath(SIZE_A(path), path);
  appendPath(path, DEF_FILE_FOR_CHECKADDR);
  mp = new p_MappedFile(path);
  if(!mp->P_open(4096)) {
    delete mp;
    mp = 0;
    return;
    }
  LPDWORD wrap = (LPDWORD)mp->getAddr();
  *wrap++ = nElem;
  for(uint i = 0; i < nElem; ++i) {
    *wrap++ = vFlt[i].prph;
    *wrap++ = vFlt[i].addr;
    *wrap++ = vFlt[i].type;
    *wrap++ = vFlt[i].nbit;
    *wrap++ = vFlt[i].offs;
    }
  mp->flush();
  SendMessage(hRemote, registeredMsg, MAKEWPARAM(CM_START_COMM, refresh ? 1 : 0), (LPARAM)(HWND)*this);
}
//----------------------------------------------------------------------------
void PDiagCheckaddr::fill_sub(LPNM_TREEVIEWW twn)
{
  if(twn->action == TVE_EXPAND) {
    HTREEITEM parent = twn->itemNew.hItem;
    if(parent)
      SendMessage(hRemote, registeredMsg, MAKEWPARAM(CM_EXPAND, 0), (LPARAM)parent);
    }
}
//---------------------------------------------------------
enum eIdCodeTreeview { ectNone, ectDeleteAll, ectGetRoot, ectInsert, ectExpand, ectGetChild, ectGetItem, ectSetItem, ectDeleteItem, ectGetParent, ectRefresh, ectMax };
//----------------------------------------------------------------------------
static void saveCurrCoords(HWND hwnd)
{
  TCHAR buff[512];
  WINDOWPLACEMENT wp;
  ZeroMemory(&wp, sizeof(wp));
  wp.length = sizeof(wp);
  GetWindowPlacement(hwnd, &wp);
  int maximized = SW_MAXIMIZE == (wp.showCmd & SW_MAXIMIZE);
  PRect r(wp.rcNormalPosition);
  wsprintf(buff, _T("%d,%d,%d,%d,%d"), r.left, r.top, r.Width(), r.Height(), maximized);
  saveRegCheckAddr(buff);
}
//----------------------------------------------------------------------------
LRESULT PDiagCheckaddr::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(registeredMsg == message) {
    switch(LOWORD(wParam)) {
      case CM_INIT_COMM:
      case CM_START_COMM:
      case CM_END_COMM:
      case CM_EXPAND:
        return 0;
      case CM_MARSHALL_COMM:
        switch(HIWORD(wParam)) {
          case ectDeleteAll:
            TreeView_DeleteAllItems(*Tree);
            break;
          case ectGetRoot:
            tv_getRoot();
            break;
          case ectInsert:
            tv_InsertItem();
            break;
          case ectExpand:
            tv_Expand();
            break;
          case ectGetChild:
            tv_getChild();
            break;
          case ectGetItem:
            tv_getItem();
            break;
          case ectSetItem:
            tv_setItem();
            break;
          case ectDeleteItem:
            tv_DeleteItem();
            break;
          case ectGetParent:
            tv_GetParent();
            break;
          case ectRefresh:
            InvalidateRect(*Tree, 0, 0);
            break;
          default:
            tv_setError();
            break;
          }
        break;
      }
    return 1;
    }
  switch(message) {
    case WM_DESTROY:
      saveCurrCoords(hwnd);
      break;
    case WM_SIZE:
      resize();
      break;

    case WM_NOTIFY:
      if(((NMHDR*)lParam)->hwndFrom == *Tree) {
        switch(((NMHDR*)lParam)->code) {
          case TVN_ITEMEXPANDINGA:
          case TVN_ITEMEXPANDINGW:
            fill_sub((LPNM_TREEVIEWW)lParam);
            return 0;
          }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//---------------------------------------------------------
void PDiagCheckaddr::tv_setError()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  *pdw |= 1 << 16;
}
//---------------------------------------------------------
void PDiagCheckaddr::tv_getRoot()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  if(ectGetRoot != *pdw)
    return;
  HTREEITEM root = TreeView_GetRoot(*Tree);
  ++pdw;
  *(HTREEITEM*)pdw = root;
}
//---------------------------------------------------------
void PDiagCheckaddr::tv_InsertItem()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  if(*pdw != ectInsert)
    return;
  TV_INSERTSTRUCT* pis = (TV_INSERTSTRUCT*)(pdw + 1);
  LPTSTR ptxt = (LPTSTR)(pis + 1);
  pis->item.pszText = ptxt;
  HTREEITEM item = TreeView_InsertItem(*Tree, pis);
  if(!item)
    tv_setError();
  else {
    ++pdw;
    *(HTREEITEM*)pdw = item;
    }
}
//---------------------------------------------------------
void PDiagCheckaddr::tv_Expand()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  if(pdw[0] != ectExpand)
    return;
  TreeView_Expand(*Tree, *(HTREEITEM*)(pdw + 2), pdw[1]);
}
//---------------------------------------------------------
void PDiagCheckaddr::tv_getChild()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  if(pdw[0] != ectGetChild)
    return;
  HTREEITEM item = TreeView_GetChild(*Tree, *(HTREEITEM*)(pdw + 1));
  if(!item)
    tv_setError();
  else {
    ++pdw;
    *(HTREEITEM*)pdw = item;
    }
}
//---------------------------------------------------------
void PDiagCheckaddr::tv_getItem()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  if(*pdw != ectGetItem)
    return;

  TV_ITEM* pi = (TV_ITEM*)(pdw + 1);
  LPTSTR ptxt = (LPTSTR)(pi + 1);
  pi->pszText = ptxt;

  if(!TreeView_GetItem(*Tree, pi))
    tv_setError();
}
//---------------------------------------------------------
void PDiagCheckaddr::tv_setItem()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  if(*pdw != ectSetItem)
    return;

  TV_ITEM* pi = (TV_ITEM*)(pdw + 1);
  LPTSTR ptxt = (LPTSTR)(pi + 1);
  pi->pszText = ptxt;

  if(!TreeView_SetItem(*Tree, pi))
    tv_setError();
}
//---------------------------------------------------------
void PDiagCheckaddr::tv_DeleteItem()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  if(pdw[0] != ectDeleteItem)
    return;
  if(!TreeView_DeleteItem(*Tree, *(HTREEITEM*)(pdw + 1)))
    tv_setError();
}
//---------------------------------------------------------
void PDiagCheckaddr::tv_GetParent()
{
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  if(*pdw != ectGetParent)
    return;

  HTREEITEM item = TreeView_GetParent(*Tree, *(HTREEITEM*)(pdw + 1));
  if(!item)
    tv_setError();
  else {
    ++pdw;
    *(HTREEITEM*)pdw = item;
    }
}
//---------------------------------------------------------
