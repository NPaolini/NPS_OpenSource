//----------- allVars.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "allVars.h"
#include "HeaderMsg.h"
//----------------------------------------------------------------------------
PD_TreeView::PD_TreeView(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), Tree(0), ImageList(0)
{
  Tree = new PTreeView(this, IDC_TREE1);
}
//----------------------------------------------------------------------------
PD_TreeView::~PD_TreeView()
{
  destroy();
  if(ImageList)
    ImageList_Destroy(ImageList);
}
//----------------------------------------------------------------------------
bool PD_TreeView::create()
{
  if(!baseClass::create())
    return false;

  ImageList = makeImageList();

  fill();

  return true;
}
//----------------------------------------------------------------------------
void PD_TreeView::resize()
{
  PRect r;
  GetClientRect(*this, r);
  Tree->setWindowPos(0, r, SWP_NOZORDER);
}
//----------------------------------------------------------------------------
LRESULT PD_TreeView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      resize();
      break;

    case WM_NOTIFY:
      if(((NMHDR*)lParam)->hwndFrom == *Tree) {
        switch(((NMHDR*)lParam)->code) {
/*
  sembra che anche in unicode legga solo il code ???A
*/
#if 0
          case TVN_ITEMEXPANDING:
#else
          case TVN_ITEMEXPANDINGA:
          case TVN_ITEMEXPANDINGW:
#endif
            fill_sub((LPNM_TREEVIEWW)lParam);
            return 0;
#if 0
          case TVN_ITEMEXPANDED:
#else
          case TVN_ITEMEXPANDEDA:
          case TVN_ITEMEXPANDEDW:
#endif
            endExpand((LPNM_TREEVIEWW)lParam);
            return 0;
#if 0
          case TVN_SELCHANGED:
#else
          case TVN_SELCHANGEDA:
          case TVN_SELCHANGEDW:
#endif
            changedSel((LPNM_TREEVIEWW)lParam);
            return 0;
          }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HTREEITEM PD_TreeView::add_Child(LPTSTR Buff, const infoPrph* ipp, HTREEITEM parent, HTREEITEM child, uint ixImage)
{
  TV_INSERTSTRUCT is;
  is.hParent = parent;
  is.hInsertAfter = child;
  is.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  is.item.hItem = 0;
  is.item.pszText = Buff;
  is.item.iImage = ixImage;
  is.item.iSelectedImage = ixImage;

  child = my_TreeView_InsertItem(&is);
  _tcscpy(Buff, _T("**"));

  is.hParent = child;
  is.hInsertAfter = TVI_FIRST;
  is.item.mask = TVIF_TEXT;
  my_TreeView_InsertItem(&is);
  return child;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PD_AllVars::PD_AllVars(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), pMP(0)
{}
//----------------------------------------------------------------------------
PD_AllVars::~PD_AllVars()
{
  destroy();
}
//----------------------------------------------------------------------------
#define TV_BKG_COLOR RGB(0xf8, 0xe4, 0xb3)
#define TV_TXT_COLOR RGB(0, 0, 0xb3)
//----------------------------------------------------------------------------
HIMAGELIST PD_AllVars::makeImageList()
{
  getTree()->setFont(D_FONT(16, 0, 0, _T("courier new")), true);
  HIMAGELIST imageList = ImageList_Create(CX_IMAGE_ADDR, CY_IMAGE_ADDR, ILC_COLOR | ILC_MASK, 20, 0);

  PBitmap bmp(IDB_FULL, getHInstance());
  ImageList_AddMasked(imageList, bmp, RGB(192, 192, 192));

  TreeView_SetImageList(*getTree(), imageList, 0);
  TreeView_SetBkColor(*getTree(), TV_BKG_COLOR);
  TreeView_SetTextColor(*getTree(), TV_TXT_COLOR);

  return imageList;
}
//----------------------------------------------------------------------------
static void formatPrph(LPTSTR buff, uint prph)
{
  wsprintf(buff, _T("Prph [%d]"), prph);
}
//----------------------------------------------------------------------------
static void unformatPrph(LPTSTR buff, int& prph)
{
  prph = -1;
  if(!buff)
    return;
  buff += 6;
  prph = _ttoi(buff);
}
//----------------------------------------------------------------------------
void PD_AllVars::fill()
{
  my_TreeView_DeleteAllItems();
  if(!pMP)
    return;
  managePages& MP = *pMP;
  HTREEITEM root_t = my_TreeView_GetRoot();
  TV_INSERTSTRUCT is;
  is.hParent = root_t;
  is.hInsertAfter = TVI_FIRST;
  if(root_t)
    is.item.mask = TVIF_HANDLE;
  else
    is.item.mask = 0;
  is.item.hItem = root_t;

  HTREEITEM root = my_TreeView_InsertItem(&is);
/*
  if(!root) {
    DisplayErrorString(GetLastError());
    return;
    }
*/
  TCHAR buff[500];
  HTREEITEM parent = TVI_FIRST;

  const bool* lPrph = MP.getListPrph();
  for(uint i = 1; i < MAX_PRPH; ++i) {
    if(!lPrph[i] || !isValidPrph(i))
      continue;
    formatPrph(buff, i);
    is.hParent = root;
    is.hInsertAfter = parent;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    is.item.hItem = 0;
    is.item.pszText = buff;
    int ixImage = 2;
    if(2 <= i && i <= 9)
      ixImage = 1;
    else
      ixImage = 0;
    is.item.iImage = ixImage;
    is.item.iSelectedImage = ixImage;

    parent = my_TreeView_InsertItem(&is);
    _tcscpy(buff, _T("*"));

    is.hParent = parent;
    is.hInsertAfter = TVI_FIRST;
    is.item.mask = TVIF_TEXT;
    my_TreeView_InsertItem(&is);

    }

  #define ROOT_IMAGE 2
  is.item.iImage = ROOT_IMAGE;
  is.item.iSelectedImage = ROOT_IMAGE;
  _tcscpy(buff, _T("Peripheral"));
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  is.item.pszText = buff;
  my_TreeView_SetItem(&is.item);
  my_TreeView_Expand(root, TVE_EXPAND);
  my_refresh();
}
//----------------------------------------------------------------------------
void PD_AllVars::fill_sub(LPNM_TREEVIEWW twn)
{
  if(twn->action == TVE_EXPAND) {
    do {
      HTREEITEM parent = twn->itemNew.hItem;
      if(!parent)
        break;
      HTREEITEM child =  my_TreeView_GetChild(parent);
      if(!child)
        break;
      TCHAR buff[200];
      TV_ITEM tvi;
      tvi.mask = TVIF_TEXT;
      tvi.hItem = child;
      tvi.pszText = buff;
      tvi.cchTextMax = SIZE_A(buff);
      if(!my_TreeView_GetItem(&tvi))
        break;

      if(*buff == _T('*')) {
        my_TreeView_DeleteItem(child);
        if(_T('*') == buff[1])
          fillSub2(parent);
        else if(_T('#') == buff[1])
          fillSub3(parent);
        else
          fillSub1(parent);
        }
      } while(false);
    }
  my_refresh();
}
//----------------------------------------------------------------------------
void PD_AllVars::endExpand(LPNM_TREEVIEWW twn)
{
}
//----------------------------------------------------------------------------
extern LPCTSTR getStrPageType(uint ix);
extern LPCTSTR getStrObject(uint ix);
extern LPCTSTR getStrAction(uint ix);
extern LPCTSTR getStrDataType(uint ix);
//----------------------------------------------------------------------------
bool isSame1(const infoPrph* ipp1, const infoPrph* ipp2)
{
  if(GET_pV(ipp1, Addr) != GET_pV(ipp2, Addr))
    return false;
  return true;
}
//----------------------------------------------------------------------------
bool isSame2(const infoPrph* ipp1, const infoPrph* ipp2)
{
  if(GET_pV(ipp1, Addr) != GET_pV(ipp2, Addr))
    return false;
  if(GET_pV(ipp1, dataType) != GET_pV(ipp2, dataType))
    return false;
  if(GET_pV(ipp1, nBit) != GET_pV(ipp2, nBit))
    return false;
  if(GET_pV(ipp1, Offs) != GET_pV(ipp2, Offs))
    return false;
  if(GET_pV(ipp1, Norm) != GET_pV(ipp2, Norm))
    return false;
  return true;
}
//----------------------------------------------------------------------------
void formatRowAddr(LPTSTR buff, const infoPrph* ipp)
{
  LPCTSTR format = _T("Addr[%4d] - Type[%-11s] - Norm[%2d] - nBit[%2d] - Offs[%2d]");
  uint ix = GET_pV(ipp, dataType);
  wsprintf(buff, format, GET_pV(ipp, Addr), getStrDataType(ix), GET_pV(ipp, Norm), GET_pV(ipp, nBit), GET_pV(ipp, Offs));
}
//----------------------------------------------------------------------------
LPCTSTR getFirstDig(LPCTSTR p)
{
  if(!p)
    return 0;
  while(*p && !_istdigit((unsigned)*p))
    ++p;
  return p;
}
//----------------------------------------------------------------------------
LPCTSTR getNextDig(LPCTSTR p)
{
  if(!p)
    return 0;
  while(_istdigit((unsigned)*p))
    ++p;
  return getFirstDig(p);
}
//----------------------------------------------------------------------------
void unformatRowAddr(LPCTSTR buff, infoPrph* ipp)
{
  LPCTSTR p = getFirstDig(buff);
  while(p) {
    uint v = _ttoi(p);
    SET_pO(ipp, Addr, v);

    p = getNextDig(p);
    if(!p)
      break;
    v = _ttoi(p);
    SET_pO(ipp, dataType, v);

    p = getNextDig(p);
    if(!p)
      break;
    v = _ttoi(p);
    SET_pO(ipp, Norm, v);

    p = getNextDig(p);
    if(!p)
      break;
    v = _ttoi(p);
    SET_pO(ipp, nBit, v);

    p = getNextDig(p);
    if(!p)
      break;
    v = _ttoi(p);
    SET_pO(ipp, Offs, v);
    break;
    }
}
//----------------------------------------------------------------------------
void formatRowOnlyAddr(LPTSTR buff, const infoPrph* ipp, DWORD count)
{
  LPCTSTR format = _T("Addr[%4d] -> %d");
  wsprintf(buff, format, GET_pV(ipp, Addr), count);
}
//----------------------------------------------------------------------------
void unformatRowOnlyAddr(LPCTSTR buff, infoPrph* ipp)
{
  LPCTSTR p = getFirstDig(buff);
  if(p) {
    uint v = _ttoi(p);
    SET_pO(ipp, Addr, v);
    }
}
//----------------------------------------------------------------------------
void PD_AllVars::fillSub1(HTREEITEM parent)
{
  if(!pMP)
    return;
  managePages& MP = *pMP;
  TCHAR Buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = parent;
  tvi.pszText = Buff;
  tvi.cchTextMax = SIZE_A(Buff);
  if(!my_TreeView_GetItem(&tvi))
    return;

  int ixPrph;
  unformatPrph(Buff, ixPrph);
  if(-1 == ixPrph)
    return;

  MP.setOrder(managePages::byPrph);
  do {
    infoPrph prph;
    SET_rO(prph, Prph, ixPrph);
    MP.find(&prph);
    const infoPrph* ipp;
    while(MP.getCurr(ipp)) {
      if(GET_pV(ipp, Prph) != ixPrph) {
        if(!MP.setNextPrph())
          break;
        }
      else
        break;
      }
    } while(false);

  HTREEITEM child = TVI_FIRST;
  infoPrph oldIpp;
  const infoPrph* ipp;
  MP.getCurr(ipp);
  if(!ipp)
    return;
  oldIpp = *ipp;
  DWORD count = 0;
  do {
    const infoPrph* ipp;
    if(!MP.getCurr(ipp))
      break;
    if(GET_pV(ipp, Prph) != ixPrph)
      break;
    if(isValidAddr(ixPrph, GET_pV(ipp, Addr), GET_pV(ipp, dataType), GET_pV(ipp, nBit), GET_pV(ipp, Offs))) {
      if(!isSame1(&oldIpp, ipp)) {
        if(count) {
          formatRowOnlyAddr(Buff, &oldIpp, count);
          child = add_Child(Buff, &oldIpp, parent, child, 2);
          }
        oldIpp = *ipp;
        count = 0;
        }
      ++count;
      }
    } while(MP.setNextPrph());
  if(count) {
    formatRowOnlyAddr(Buff, &oldIpp, count);
    add_Child(Buff, &oldIpp, parent, child, 2);
    }
}
//----------------------------------------------------------------------------
int formatRowObj(managePages& MP, LPTSTR buff, const infoPrph* ipp)
{
  LPCTSTR format[] = {
    _T("%-8s - X[%4d] - Y[%4d] - Page[%s] - Action[%s]"), // 0
    _T("%-8s - Funct [F%d] - Page[%s] - Action[%s]"),     // 1
    _T("%-8s - Page[%s] - Action[%s]"),                   // 2

    _T("%-8s - Name[%s]"), //dll                          // 3

    _T("%-8s - Timer addr - [%s]"), // timer trend        // 4
    _T("%-8s - Enable addr - [%s]"), // enable trend      // 5
    _T("%-8s - Var[%d] - [%s]"), // vars trend            // 6

    _T("%-8s - [%s]"), // printscreen/vars - filter alarm // 7

    _T("%-8s - Funct [S%d] - Page[%s] - Action[%s]"),     // 8 (1bis)

    _T("%s - Alarm_%d" PAGE_EXT), // associazione allarmi           // 9

    _T("%s - Alarm_%d") PAGE_EXT _T(" - %s"), // associazione allarmi con file associato // 10

    };
  uint ixObj = GET_pV(ipp, objType);
  int X = GET_pV(ipp, X);
  int Y = GET_pV(ipp, Y);

  int ixForm = 0;
  uint pgType = 0;
  if(COORD_BY_ASSOC_FILENAME == X)
    ixForm = 10;
  else if(COORD_BY_FUNCT == X) {
    ixForm = 1;
    pgType = GET_pV(ipp, pageType);
    switch(pgType) {
      case infoPrph::eAlarmAssoc:
        ixForm = 9;
        break;
      case infoPrph::eNorm:
        ixForm = 7;
        break;

      case infoPrph::eDll:
        ixForm = 3;
        break;
      case infoPrph::eStdMsg:
        switch(Y) {
          case COORD_BY_TIMER_TREND:
            ixForm = 4;
            break;
          case COORD_BY_ENABLE_TREND:
            ixForm = 5;
            break;
          case COORD_BY_PRINT_SCREEN:
          case COORD_BY_SAVE_SCREEN:
          case COORD_BY_PRINT_VARS:
          case COORD_BY_FILTER_ALARM:
          case COORD_BY_MAN_STOP:
          case COORD_BY_MAN_STOP_OPER:
          case COORD_BY_CURR_DATETIME:
          case COORD_BY_INITIALIZED:
            ixForm = 7;
            break;
          default:
            if(COORD_BY_FUNCT != Y)
              ixForm = 6;
            break;
          }
        break;

      default:
        switch(Y) {
          case COORD_BY_FUNCT:
            ixForm = 2;
            break;
          case COORD_BY_RECIPE_PAGE:
          case COORD_BY_RECIPE_ROW_PAGE:
            ixForm = 7;
            break;
          }
        break;
      }
    }

  LPCTSTR page = GET_pV(ipp, Page);
  if(!page)
    page = _T("");

  TCHAR act[200];

  int ix = GET_pV(ipp, action);
  if(ix > infoPrph::eMaxAction) {
    ix = infoPrph::eMaxAction;
    wsprintf(act, _T("%s"), getStrAction(ix));
    }
  else {
    if(GET_pV(ipp, openPage))
      wsprintf(act, _T("%s ->[ %s ]"), getStrAction(ix), GET_pV(ipp, openPage));
    else
      wsprintf(act, _T("%s"), getStrAction(ix));
    }

  switch(ixForm) {
    case 0:
      wsprintf(buff, format[ixForm], getStrObject(ixObj), X, Y, page, act);
      break;
    case 1:
      if(Y < 20)
        wsprintf(buff, format[ixForm], getStrObject(ixObj), Y + 1, page, act);
      else
        wsprintf(buff, format[8], getStrObject(ixObj), Y + 1 - 20, page, act);
      break;
    case 2:
      wsprintf(buff, format[ixForm], getStrObject(ixObj), page, act);
      break;
    case 3:
      wsprintf(buff, format[ixForm], getStrObject(ixObj), page);
      break;
    case 4:
      wsprintf(buff, format[ixForm], getStrObject(ixObj), page);
      break;
    case 5:
      wsprintf(buff, format[ixForm], getStrObject(ixObj), page);
      break;
    case 6:
      wsprintf(buff, format[ixForm], getStrObject(ixObj), Y, page);
      break;
    case 7:
      wsprintf(buff, format[ixForm], getStrObject(ixObj), page);
      break;
    case 10:
      {
        uint pAss = GET_pV(ipp, action);
        if(pAss) {
          LPCTSTR fileAssoc = (LPCTSTR)pAss;
          const setOfPages& soPage = MP.getSetPage();
          const infoPages* ip = soPage.getPageInfo(fileAssoc);
          TCHAR t = (TCHAR)ip->getLevel();
          wsprintf(act, _T("[%c] %s"), t, fileAssoc);
          wsprintf(buff, format[ixForm], page, Y, act);
          break;
          }
        }
      // fall through
    case 9:
      wsprintf(buff, format[ixForm], page, Y);
      break;
    default:
      _tcscpy(buff, _T("Error"));
      break;
    }
  return getIxImageByIdObj(ixObj);
}
//----------------------------------------------------------------------------
int getIxImageByIdObj(int ixObj)
{
  if(ixObj >= infoPrph::ePlotXY)
    --ixObj;
  return ixObj + 2;
}
//----------------------------------------------------------------------------
void PD_AllVars::fillSub2(HTREEITEM parent)
{
  if(!pMP)
    return;
  managePages& MP = *pMP;

  HTREEITEM parOfParent = my_TreeView_GetParent(parent);
  if(!parOfParent)
    return;

  TCHAR Buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = parOfParent;
  tvi.pszText = Buff;
  tvi.cchTextMax = SIZE_A(Buff);
  if(!my_TreeView_GetItem(&tvi))
    return;

  int ixPrph;
  unformatPrph(Buff, ixPrph);
  if(-1 == ixPrph)
    return;

  tvi.hItem = parent;
  if(!my_TreeView_GetItem(&tvi))
    return;

  infoPrph Ipp;

  unformatRowOnlyAddr(Buff, &Ipp);
  SET_rO(Ipp, Prph, ixPrph);

  HTREEITEM child = TVI_FIRST;
  MP.setOrder(managePages::byPrph);
  const infoPrph* oldIpp = 0;
  do {
    MP.find(&Ipp);
    const infoPrph* ipp;
    while(MP.getCurr(ipp)) {
      if(GET_pV(ipp, Prph) != ixPrph || GET_pV(ipp, Addr) != GET_rV(Ipp, Addr)) {
        if(!MP.setNextPrph())
          break;
        }
      else
        break;
      }
    } while(false);

  do {
    const infoPrph* ipp;
    if(!MP.getCurr(ipp))
      break;
    if(GET_pV(ipp, Prph) != ixPrph || GET_pV(ipp, Addr) != GET_rV(Ipp, Addr))
      break;

    if(isValidAddr(ixPrph, GET_pV(ipp, Addr), GET_pV(ipp, dataType), GET_pV(ipp, nBit), GET_pV(ipp, Offs))) {

      if(!oldIpp || !isSame2(oldIpp, ipp)) {
        oldIpp = ipp;
        formatRowAddr(Buff, ipp);

        TV_INSERTSTRUCT is;
        is.hParent = parent;
        is.hInsertAfter = child;
        is.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
        is.item.hItem = 0;
        is.item.pszText = Buff;
        is.item.iImage = 2;
        is.item.iSelectedImage = 2;

        child = my_TreeView_InsertItem(&is);

        _tcscpy(Buff, _T("*#"));

        is.hParent = child;
        is.hInsertAfter = TVI_FIRST;
        is.item.mask = TVIF_TEXT;
        my_TreeView_InsertItem(&is);
        }
      }
    } while(MP.setNextPrph());
}
//----------------------------------------------------------------------------
void PD_AllVars::fillSub3(HTREEITEM parent)
{
  if(!pMP)
    return;
  managePages& MP = *pMP;

  HTREEITEM parOfParent = my_TreeView_GetParent(parent);
  if(!parOfParent)
    return;

  HTREEITEM grandParOfParent = my_TreeView_GetParent(parOfParent);
  if(!grandParOfParent)
    return;
  TCHAR Buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = grandParOfParent;
  tvi.pszText = Buff;
  tvi.cchTextMax = SIZE_A(Buff);
  if(!my_TreeView_GetItem(&tvi))
    return;

  int ixPrph;
  unformatPrph(Buff, ixPrph);
  if(-1 == ixPrph)
    return;

  tvi.hItem = parent;
  if(!my_TreeView_GetItem(&tvi))
    return;

  infoPrph Ipp;

  unformatRowAddr(Buff, &Ipp);
  SET_rO(Ipp, Prph, ixPrph);

  HTREEITEM child = TVI_FIRST;
  MP.setOrder(managePages::byPrph);
  do {
    MP.find(&Ipp);
    const infoPrph* ipp;
    while(MP.getCurr(ipp)) {
      if(GET_pV(ipp, Prph) != ixPrph || GET_pV(ipp, Addr) != GET_rV(Ipp, Addr)) {
        if(!MP.setNextPrph())
          break;
        }
      else
        break;
      }
    } while(false);

  do {
    const infoPrph* ipp;
    if(!MP.getCurr(ipp))
      break;
    if(GET_pV(ipp, Prph) != ixPrph || GET_pV(ipp, Addr) != GET_rV(Ipp, Addr))
      break;

    if(isSame2(&Ipp, ipp)) {

      int ixImage = formatRowObj(MP, Buff, ipp);

      TV_INSERTSTRUCT is;
      is.hParent = parent;
      is.hInsertAfter = child;
      is.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
      is.item.hItem = 0;
      is.item.pszText = Buff;
      is.item.iImage = ixImage;
      is.item.iSelectedImage = ixImage;

      child = my_TreeView_InsertItem(&is);
      }
    } while(MP.setNextPrph());
}
//----------------------------------------------------------------------------
#define RET_IF(f) { t = v.f - ins.f; if(t) return t; }
//----------------------------------------------------------------------------
int fzCmpFilter(const varsFilter& v, const varsFilter& ins)
{
  int t;
  RET_IF(prph)
  RET_IF(addr)
  RET_IF(type)
  RET_IF(nbit)
  RET_IF(offs)
  return 0;
}
//----------------------------------------------------------------------------
int fzCmpFilterOnlyPrph(const varsFilter& v, const varsFilter& ins)
{
  return v.prph - ins.prph;
}
//----------------------------------------------------------------------------
//#define SND_MSG_REMOTE(wParam) SendMessage(HWND_BROADCAST, registeredMsg, (wParam), 0)
#define SND_MSG_REMOTE(wParam) SendMessageTimeout(hRemote, registeredMsg, (wParam), 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &msgResult)
//----------------------------------------------------------------------------
PD_AllVarsBysvMaker::PD_AllVarsBysvMaker(PWin* parent, HWND hRemote) :
      baseClass(parent), mp(0), hRemote(hRemote)
{
  TCHAR path[_MAX_PATH];
  GetTempPath(SIZE_A(path), path);
  appendPath(path, DEF_FILE_FOR_SVMAKER);
  mp = new p_MappedFile(path);
  if(!mp->P_open()) {
    delete mp;
    mp = 0;
    }
  else {
    LPDWORD wrap = (LPDWORD)mp->getAddr();

    DWORD nElem = *wrap++;
    vFlt.setDim(nElem);
    for(uint i = 0; i < nElem; ++i) {
      varsFilter f;
      f.prph = *wrap++;
      f.addr = *wrap++;
      f.type = *wrap++;
      f.nbit = *wrap++;
      f.offs = *wrap++;
      vFlt.insertEx(f, fzCmpFilter);
      }
    }
}
//----------------------------------------------------------------------------
PD_AllVarsBysvMaker::~PD_AllVarsBysvMaker()
{
  destroy();
  delete mp;
}
//----------------------------------------------------------------------------
void PD_AllVarsBysvMaker::actionExpand(HTREEITEM parent)
{
  NM_TREEVIEW tree;
  ZeroMemory(&tree, sizeof(tree));
  tree.itemNew.hItem = parent;
  tree.action = TVE_EXPAND;
  fill_sub(&tree);
}
//----------------------------------------------------------------------------
bool PD_AllVarsBysvMaker::isValidPrph(uint prph)
{
  uint pos;
  varsFilter vf(prph, 0);
  return vFlt.find(vf, pos, fzCmpFilterOnlyPrph);
}
//----------------------------------------------------------------------------
bool PD_AllVarsBysvMaker::isValidAddr(uint prph, uint addr, uint type, uint nbit, uint offs)
{
  uint pos;
  varsFilter vf(prph, addr, type, nbit, offs);
  return vFlt.find(vf, pos, fzCmpFilter);
}
//---------------------------------------------------------
enum eIdCodeTreeview { ectNone, ectDeleteAll, ectGetRoot, ectInsert, ectExpand, ectGetChild, ectGetItem, ectSetItem, ectDeleteItem, ectGetParent, ectRefresh, ectMax };
//---------------------------------------------------------
void PD_AllVarsBysvMaker::my_refresh()
{
  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectRefresh));
}
//---------------------------------------------------------
// qui non ci sono dati da immettere nel file mappato
void PD_AllVarsBysvMaker::my_TreeView_DeleteAllItems()
{
  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectDeleteAll));
}
//---------------------------------------------------------
HTREEITEM PD_AllVarsBysvMaker::my_TreeView_GetRoot()
{
  if(!mp)
    return 0;
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  *pdw = ectGetRoot;
  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectGetRoot));
  if(ectGetRoot != *pdw)
    return 0;
  ++pdw;
  HTREEITEM root = *(HTREEITEM*)pdw;
  return root;
}
//---------------------------------------------------------
HTREEITEM PD_AllVarsBysvMaker::my_TreeView_InsertItem(TV_INSERTSTRUCT* is)
{
  if(!mp)
    return 0;
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  *pdw = ectInsert;
  TV_INSERTSTRUCT* pis = (TV_INSERTSTRUCT*)(pdw + 1);
  *pis = *is;
  if(is->item.mask & TVIF_TEXT) {
    LPTSTR ptxt = (LPTSTR)(pis + 1);
    uint len = _tcslen(is->item.pszText);
    copyStrZ(ptxt, is->item.pszText, len);
    }
  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectInsert));
  if(ectInsert != *pdw)
    return 0;
  ++pdw;
  HTREEITEM item = *(HTREEITEM*)pdw;
  return item;
}
//---------------------------------------------------------
void PD_AllVarsBysvMaker::my_TreeView_Expand(HTREEITEM parent, UINT flag)
{
  if(!mp)
    return;
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  pdw[0] = ectExpand;
  pdw[1] = flag;
  *(HTREEITEM*)(pdw + 2) = parent;
  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectExpand));
}
//---------------------------------------------------------
HTREEITEM PD_AllVarsBysvMaker::my_TreeView_GetChild(HTREEITEM parent)
{
  if(!mp)
    return 0;
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  pdw[0] = ectGetChild;
  *(HTREEITEM*)(pdw + 1) = parent;

  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectGetChild));
  if(ectGetChild != *pdw)
    return 0;
  ++pdw;
  HTREEITEM item = *(HTREEITEM*)pdw;
  return item;
}
//---------------------------------------------------------
BOOL PD_AllVarsBysvMaker::my_TreeView_GetItem(TV_ITEM* item)
{
  if(!mp)
    return false;
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  *pdw = ectGetItem;
  TV_ITEM* pi = (TV_ITEM*)(pdw + 1);
  *pi = *item;

  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectGetItem));
  if(ectGetItem != *pdw)
    return false;
  if(item->mask & TVIF_TEXT) {
    LPTSTR ptxt = (LPTSTR)(pi + 1);
    uint len = _tcslen(ptxt);
    copyStrZ(item->pszText, ptxt, len);
    }
  return true;
}
//---------------------------------------------------------
BOOL PD_AllVarsBysvMaker::my_TreeView_SetItem(TV_ITEM* item)
{
  if(!mp)
    return false;
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  *pdw = ectSetItem;
  TV_ITEM* pi = (TV_ITEM*)(pdw + 1);
  *pi = *item;
  if(item->mask & TVIF_TEXT) {
    LPTSTR ptxt = (LPTSTR)(pi + 1);
    pi->pszText = ptxt;
    uint len = _tcslen(item->pszText);
    copyStrZ(pi->pszText, item->pszText, len);
    }
  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectSetItem));
  if(ectSetItem != *pdw)
    return false;
  return true;
}
//---------------------------------------------------------
BOOL PD_AllVarsBysvMaker::my_TreeView_DeleteItem(HTREEITEM item)
{
  if(!mp)
    return false;
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  pdw[0] = ectDeleteItem;
  *(HTREEITEM*)(pdw + 1) = item;

  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectDeleteItem));
  return ectDeleteItem == *pdw;
}
//---------------------------------------------------------
HTREEITEM PD_AllVarsBysvMaker::my_TreeView_GetParent(HTREEITEM item)
{
  if(!mp)
    return 0;
  LPDWORD pdw = (LPDWORD)mp->getAddr();
  *pdw = ectGetParent;
  *(HTREEITEM*)(pdw + 1) = item;

  SND_MSG_REMOTE(MAKEWPARAM(CM_MARSHALL_COMM, ectGetParent));
  if(ectGetParent != *pdw)
    return 0;
  ++pdw;
  HTREEITEM parent = *(HTREEITEM*)pdw;
  return parent;
}
//---------------------------------------------------------

