//----------- allPages.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "allPages.h"
#include "HeaderMsg.h"
//----------------------------------------------------------------------------
PD_ListView::PD_ListView(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), List(0), ImageList(0)
{
  List = new PListView(this, IDC_LIST1);
}
//----------------------------------------------------------------------------
PD_ListView::~PD_ListView()
{
  destroy();
  if(ImageList)
    ImageList_Destroy(ImageList);
}
//----------------------------------------------------------------------------
bool PD_ListView::create()
{
  if(!baseClass::create())
    return false;

  ImageList = makeImageList();
  fillTitle();

  return true;
}
//-----------------------------------------------------------
bool PD_ListView::addItem(PVect<LPTSTR>& field, int ixImage)
{
  LVITEM lvi;
  ZeroMemory(&lvi, sizeof(lvi));
  lvi.mask = LVIF_TEXT | LVIF_IMAGE;
  lvi.iImage = ixImage;
  lvi.iItem = 65535;
  lvi.iSubItem = 0;
  lvi.pszText = field[0];

  int pos = ListView_InsertItem(*List, &lvi);
  if(-1 == pos)
    return false;

  lvi.iItem = pos;
  uint nElem = field.getElem();
  for(uint i = 1; i < nElem; ++i) {
    lvi.iSubItem = i;
    lvi.pszText = field[i];
    if(!ListView_SetItem(*List, &lvi))
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
void PD_ListView::resize()
{
  PRect r;
  GetClientRect(*this, r);
  List->setWindowPos(0, r, SWP_NOZORDER);
}
//----------------------------------------------------------------------------
LRESULT PD_ListView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      resize();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
struct infoColumn
{
  LPCTSTR title;
  int sz;
};
//----------------------------------------------------------------------------
static
infoColumn titleList[] = {
  { _T("Obj"), 100 },
  { _T("X"), 60 },
  { _T("Y"), 60 },
  { _T("Prph"), 50 },
  { _T("Addr"), 60 },
  { _T("Type"), 90 },
  { _T("Norm"), 50 },
  { _T("nBits"), 50 },
  { _T("Offs"),  50 },
  { _T("Action"), 350 }
};
//----------------------------------------------------------------------------
#define TOT_COLUMN SIZE_A(titleList)
//-----------------------------------------------------------
//-----------------------------------------------------------
clientListViewPages::clientListViewPages(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), pMP(0)
{
  pagename[0] = 0;
}
//----------------------------------------------------------------------------
HIMAGELIST clientListViewPages::makeImageList()
{
  getList()->setFont(D_FONT(16, 0, fBOLD, _T("courier new")), true);
  HIMAGELIST imageList = ImageList_Create(CX_IMAGE_ADDR, CY_IMAGE_ADDR, ILC_COLOR | ILC_MASK, 20/*13*/, 0);

  PBitmap bmp(IDB_FULL, getHInstance());
  ImageList_AddMasked(imageList, bmp, RGB(192, 192, 192));

  ListView_SetImageList(*getList(), imageList, LVSIL_SMALL);

  return imageList;
}
//-----------------------------------------------------------
void clientListViewPages::fillTitle()
{
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

  for(uint i = 0; i < TOT_COLUMN; ++i) {
    lvc.iSubItem = i;
    lvc.pszText = (LPTSTR)titleList[i].title;
    lvc.cx = titleList[i].sz;
    if(i != TOT_COLUMN - 1)
      lvc.fmt = LVCFMT_CENTER;
    else
      lvc.fmt = LVCFMT_LEFT;
    if(-1 == ListView_InsertColumn(*getList(), i, &lvc))
      return;
    }
  DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;// | LVS_EX_SUBITEMIMAGES;
  ListView_SetExtendedListViewStyle(*getList(), dwExStyle);

#define LV_TXT_COLOR RGB(0, 0, 0xb3)
#define LV_BKG_COLOR RGB(0xf8, 0xe4, 0xb3)

  ListView_SetBkColor(*getList(), LV_BKG_COLOR);
  ListView_SetTextBkColor(*getList(), LV_BKG_COLOR);

  ListView_SetTextColor(*getList(), LV_TXT_COLOR);
}
//----------------------------------------------------------------------------
#define WM_CUSTOM_VIEW (WM_FW_FIRST_FREE + 1)
#define CM_CHG_PAGE 1
//----------------------------------------------------------------------------
LRESULT clientListViewPages::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CUSTOM_VIEW:
      switch(LOWORD(wParam)) {
        case CM_CHG_PAGE:
          if(_tcsicmp((LPCTSTR)lParam, pagename)) {
            _tcscpy(pagename, (LPCTSTR)lParam);
            fill();
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static LPCTSTR PgType[] = {
  _T("noPg"), _T("Body"), _T("Header"), _T("Modeless"), _T("Modal"), _T("Link1"), _T("Link2"),
  _T("Dll"),
  _T("base_data"),
  _T("Footer"),
  _T("Normal"),
  _T("Alarm Assoc"),
  };
//----------------------------------------------------------------------------
LPCTSTR getStrPageType(uint ix)
{
  if(ix >= SIZE_A(PgType))
    ix = 0;
  return PgType[ix];
}
//----------------------------------------------------------------------------
static LPCTSTR ObjType[] =
{
  _T("noObj"),
  _T("Btn"),
  _T("Text"),
  _T("Edit"),
  _T("Panel"),
  _T("Bmp"),
  _T("BarGraph"),
  _T("Led"),
  _T("Listbox"),
  _T("Diam"),
  _T("Choose"),
  _T("Curve"),
  _T("PlotXY"),
  _T("Cam"),
  _T("xMeter"),
  _T("xScope"),
  _T("Spin"),
  _T("Slider"),
  _T("objAlarm"),
  _T("TableInfo"),
  _T("Trend"),
  _T("Print"),
  _T("Alarm"),
  _T("Stop PLC"),
  _T("Funct"),
  _T("Code Oper Addr"),
  _T("ListBox Cause Addr"),
  _T("Unknown")
};
//----------------------------------------------------------------------------
LPCTSTR getStrObject(uint ix)
{
  if(ix >= SIZE_A(ObjType))
    ix = SIZE_A(ObjType) - 1;
  return ObjType[ix];
}
//----------------------------------------------------------------------------
static LPCTSTR Action[] =  {
  _T("noAct"),
  _T("OpenBody"),
  _T("OpenTrend"),
  _T("OpenRecipe"),
  _T("OpenRecipeRow"),
  _T("OpenMaint"),
  _T("OpenModal"),
  _T("OpenModeless"),
  _T("OpenLink1"),
  _T("OpenLink2"),
  _T("Visibility"),
  _T("SendOnOpen"),
  _T("SendOnClose"),
  _T("Spin [nBit-> 1=up, 0=down] [Offset => idSpin]"),
  _T("Movement"),
  _T("Unknown")
};
//----------------------------------------------------------------------------
LPCTSTR getStrAction(uint ix)
{
  if(ix >= SIZE_A(Action))
    ix = SIZE_A(Action) - 1;
  return Action[ix];
}
//----------------------------------------------------------------------------
static LPCTSTR dataType[] =
{
  _T(" 0 - null"),
  _T(" 1 - bit"),
  _T(" 2 - byte"),
  _T(" 3 - word"),
  _T(" 4 - dword"),
  _T(" 5 - float"),
  _T(" 6 - int64"),
  _T(" 7 - real"),
  _T(" 8 - DateTime"),
  _T(" 9 - Date"),
  _T("10 - Time"),
  _T("11 - char"),
  _T("12 - short"),
  _T("13 - long"),
  _T("14 - string"),
  _T("15-Unknown")
};
//----------------------------------------------------------------------------
LPCTSTR getStrDataType(uint ix)
{
  if(ix >= SIZE_A(dataType))
    ix = SIZE_A(dataType) - 1;
  return dataType[ix];
}
//----------------------------------------------------------------------------
void clientListViewPages::clear()
{
  *pagename = 0;
  ListView_DeleteAllItems(*getList());
}
//----------------------------------------------------------------------------
void getTextAction(const infoPrph* ipp, LPTSTR act)
{

  uint ix = GET_pV(ipp, action);
  if(ix >= SIZE_A(Action)) {
    ix = SIZE_A(Action) - 1;
    wsprintf(act, _T("%s"), Action[ix]);
    }
  else {
    if(GET_pV(ipp, openPage))
      wsprintf(act, _T("%s ->[ %s ]"), Action[ix], GET_pV(ipp, openPage));
    else
      wsprintf(act, _T("%s"), Action[ix]);
    }

}
//----------------------------------------------------------------------------
void clientListViewPages::fill()
{
  if(!pMP)
    return;
  managePages& MP = *pMP;

  PVect<LPTSTR> t;
  TCHAR buff[SIZE_A_c(titleList) - 1][64];
  ZeroMemory(buff, sizeof(buff));
  TCHAR act[255];
  for(uint i = 0; i < SIZE_A_c(titleList) - 1; ++i)
    t[i] = buff[i];

  t[SIZE_A_c(titleList) - 1] = act;
  MP.setOrder(managePages::byPages);

  ListView_DeleteAllItems(*getList());

  TCHAR tName[_MAX_PATH];
  _tcscpy(tName, pagename);
  LPTSTR p = tName;
  int offs = -1;
  while(*p) {
    if(_T('{') == *p) {
      *p = 0;
      p += 2;
      int len = _tcslen(PgType[infoPrph::eLink1]);
      if(!cmpStr(PgType[infoPrph::eLink1], p, len)) {
        p += len;
        offs = _ttoi(p + 1);
        }
      break;
      }
    ++p;
    }

  do {
    infoPrph prphPage;
    SET_rO(prphPage, Page, tName);
    MP.find(&prphPage);
    } while(false);

  const infoPrph* ipp;
  while(MP.getCurr(ipp)) {
    if(_tcsicmp(GET_pV(ipp, Page), tName) < 0) {
      if(!MP.setNextPrph())
        break;
      }
    else
      break;
    }

  do {
    if(!MP.getCurr(ipp))
      break;

    if(_tcsicmp(GET_pV(ipp, Page), tName))
      break;

    if(offs >= 0 && ipp->getOffsLink() != offs)
      continue;
    int X = GET_pV(ipp, X);
    int Y = GET_pV(ipp, Y);
    *buff[1] = 0;
    *buff[2] = 0;
    bool isFileAssoc = false;
    if(COORD_BY_ASSOC_FILENAME == X) {
      isFileAssoc = true;
      wsprintf(buff[2], _T("Alarm_%d" PAGE_EXT), Y);
      uint pAss = GET_pV(ipp, action);
      if(pAss) {
        LPCTSTR fileAssoc = (LPCTSTR)pAss;
        const setOfPages& soPage = MP.getSetPage();
        const infoPages* ip = soPage.getPageInfo(fileAssoc);
        TCHAR t = (TCHAR)ip->getLevel();
        wsprintf(act, _T("[%c] %s"), t, fileAssoc);
        }
      }
    else if(COORD_BY_FUNCT == X) {
      uint pgType = GET_pV(ipp, pageType);
      switch(pgType) {
        case infoPrph::eAlarmAssoc:
          wsprintf(buff[2], _T("Alarm_%d" PAGE_EXT), Y);
          break;
        case infoPrph::eNorm:
          switch(Y) {
            case COORD_BY_NORM_LINK_VAR:
              _tcscpy(buff[2], _T("Norm By Var"));
              break;
            case COORD_BY_NORM_LINK_BIT:
              _tcscpy(buff[2], _T("Norm By Bit"));
              break;
            }
          break;
        case infoPrph::eStdMsg:
          switch(Y) {
            case COORD_BY_TIMER_TREND:
              _tcscpy(buff[2], _T("Timer"));
              break;
            case COORD_BY_ENABLE_TREND:
              _tcscpy(buff[2], _T("Enable"));
              break;
            case COORD_BY_PRINT_VARS:
            case COORD_BY_PRINT_SCREEN:
            case COORD_BY_SAVE_SCREEN:
            case COORD_BY_FILTER_ALARM:
            case COORD_BY_MAN_STOP:
            case COORD_BY_MAN_STOP_OPER:
            case COORD_BY_CURR_DATETIME:
            case COORD_BY_INITIALIZED:
              break;
            default:
              wsprintf(buff[2], _T("%d"), Y);
              break;
            }
          break;
        case infoPrph::eDll:
          switch(Y) {
            case COORD_BY_FUNCT:
              break;
            case COORD_BY_DLL_RELOAD:
              _tcscpy(buff[2], _T("reload"));
              break;
            default:
              do {
                LPCTSTR p = MP.getVarNameScript(Y);
                if(p)
                  wsprintf(buff[2], _T("$%s"), p);
                else
                  wsprintf(buff[2], _T("$%d"), Y);
                } while(false);
              break;
            }
          break;
        default:
          switch(Y) {
            case COORD_BY_FUNCT:
              break;
            case COORD_BY_RECIPE_PAGE:
              _tcscpy(buff[2], _T("for Recipe"));
              break;
            case COORD_BY_RECIPE_ROW_PAGE:
              _tcscpy(buff[2], _T("for RecipeRow"));
              break;
            default:
              if(Y < 20)
                wsprintf(buff[2], _T("F%d"), Y + 1);
              else
                wsprintf(buff[2], _T("S%d"), Y + 1 - 20);
              break;
            }
          break;
        }
      }
    else {
      wsprintf(buff[1], _T("%d"), GET_pV(ipp, X));
      wsprintf(buff[2], _T("%d"), GET_pV(ipp, Y));
      }
    wsprintf(buff[3], _T("%d"), GET_pV(ipp, Prph));
    DWORD addr = GET_pV(ipp, Addr);
    uint ix = GET_pV(ipp, dataType);
    if(1000 == GET_pV(ipp, Prph)) {
      _tcscpy_s(buff[3], _T("Const"));
      if(5 == ix) {
        float t = *(float*)&addr;
        _stprintf_s(buff[4], _T("V=%f"), t);
        zeroTrim(buff[4]);
        }
      else
        wsprintf(buff[4], _T("V=%d"), GET_pV(ipp, Addr));
      }
    else {
      wsprintf(buff[3], _T("%d"), GET_pV(ipp, Prph));
      wsprintf(buff[4], _T("%d"), GET_pV(ipp, Addr));
      }
    wsprintf(buff[5], _T("%s"), getStrDataType(ix));

    wsprintf(buff[6], _T("%d"), GET_pV(ipp, Norm));
    wsprintf(buff[7], _T("%d"), GET_pV(ipp, nBit));
    wsprintf(buff[8], _T("%d"), GET_pV(ipp, Offs));

    if(!isFileAssoc)
      getTextAction(ipp, act);
    ix = GET_pV(ipp, objType);

    wsprintf(buff[0], _T("%s"), getStrObject(ix));

    if(infoPrph::eManStopAddrOper == ix || infoPrph::eManStopAddrListBox == ix)
      ix = infoPrph::eManStop;
    else if(infoPrph::eMaxObj <= ix)
      ix = infoPrph::eManStop;

    if(!addItem(t, OFFS_IMG(ix)))
      break;
    } while(MP.setNextPrph());
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
clientTreeViewPages::clientTreeViewPages(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), pMP(0) {}
//----------------------------------------------------------------------------
HIMAGELIST clientTreeViewPages::makeImageList()
{
  getTree()->setFont(D_FONT(16, 0, fBOLD, _T("courier new")), true);
  HIMAGELIST imageList = ImageList_Create(CX_IMAGE_PAGE, CY_IMAGE_PAGE, ILC_COLOR | ILC_MASK, 20, 0);

  PBitmap bmp(IDB_PAGES, getHInstance());
  ImageList_AddMasked(imageList, bmp, RGB(192, 192, 192));

  TreeView_SetImageList(*getTree(), imageList, 0);
  TreeView_SetBkColor(*getTree(), RGB(0xff, 0xff, 0xd0));

#define TV_TXT_COLOR RGB(0, 0, 0xb3)
  TreeView_SetTextColor(*getTree(), TV_TXT_COLOR);

  return imageList;
}
//----------------------------------------------------------------------------
void clientTreeViewPages::changedSel(LPNM_TREEVIEWW twn)
{
  HTREEITEM node = twn->itemNew.hItem;
  if(!node)
    return;

  TCHAR buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = node;
  tvi.pszText = buff;
  tvi.cchTextMax = SIZE_A(buff);
  if(!TreeView_GetItem(*getTree(), &tvi))
    return;

  HWND hList = ((PD_AllPages*)getParent())->getListView();
  if(hList)
    SendMessage(hList, WM_CUSTOM_VIEW, MAKEWPARAM(CM_CHG_PAGE, 0), (LPARAM)buff);
}
//----------------------------------------------------------------------------
void clientTreeViewPages::fill()
{
  if(!pMP)
    return;
  managePages& MP = *pMP;

  TreeView_DeleteAllItems(*getTree());
  const infoPages* rootPages = MP.getRoot();
  if(!rootPages)
    return;

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
#if 1
  addTreeStdMsg(root);
  is.item.iImage = infoPrph::eStdMsg - 1;
  is.item.iSelectedImage = infoPrph::eStdMsg - 1;
  is.item.pszText = _T("base_data");
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  TreeView_SetItem(*getTree(), &is.item);
  TreeView_Expand(*getTree(), root, TVE_EXPAND);

  is.hInsertAfter = root;
  root = TreeView_InsertItem(*getTree(), &is);

  addTreeNorm(root);
  is.item.iImage = infoPrph::eStdMsg - 1;
  is.item.iSelectedImage = infoPrph::eStdMsg - 1;
  is.item.pszText = _T("Normal");
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  TreeView_SetItem(*getTree(), &is.item);
  TreeView_Expand(*getTree(), root, TVE_EXPAND);

  is.hInsertAfter = root;
  root = TreeView_InsertItem(*getTree(), &is);
  addTreeDll(root);

  is.item.iImage = infoPrph::eDll - 1;
  is.item.iSelectedImage = infoPrph::eDll - 1;
  is.item.pszText = _T("DLL");
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  TreeView_SetItem(*getTree(), &is.item);
  TreeView_Expand(*getTree(), root, TVE_EXPAND);

  is.hInsertAfter = root;
  root = TreeView_InsertItem(*getTree(), &is);
  addTreeAlarmAssoc(root);

  is.item.iImage = infoPrph::eAlarmAssoc - 1;
  is.item.iSelectedImage = infoPrph::eAlarmAssoc - 1;
  is.item.pszText = _T("Alarm Assoc");
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  TreeView_SetItem(*getTree(), &is.item);
  TreeView_Expand(*getTree(), root, TVE_EXPAND);

  is.hInsertAfter = root;
  root = TreeView_InsertItem(*getTree(), &is);

  const infoPages* headerPages = MP.getHeader();
  if(headerPages) {
    PVect<infoRecurse> setPage;
    addTree(root, headerPages, 0, setPage);
    is.item.iImage = infoPrph::eHeader - 1;
    is.item.iSelectedImage = infoPrph::eHeader - 1;
    is.item.pszText = (LPTSTR)headerPages->getPageCurr();
    is.item.hItem = root;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    TreeView_SetItem(*getTree(), &is.item);

    TreeView_Expand(*getTree(), root, TVE_EXPAND);

    is.hInsertAfter = root;
    root = TreeView_InsertItem(*getTree(), &is);
    }
#endif

//  SetOpened.reset();
  PVect<infoRecurse> setPage;
  addTree(root, rootPages, 0, setPage);

  is.item.iImage = infoPrph::eBody - 1;
  is.item.iSelectedImage = infoPrph::eBody - 1;
  is.item.pszText = (LPTSTR)rootPages->getPageCurr();
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  TreeView_SetItem(*getTree(), &is.item);

  TreeView_Expand(*getTree(), root, TVE_EXPAND);
}
//----------------------------------------------------------------------------
static bool isSameInfo(const infoPrph* ipp1, const infoPrph* ipp2)
{
  LPCTSTR t1 = GET_pV(ipp1, Page);
  if(!t1)
    t1 = _T("\0");
  LPCTSTR t2 = GET_pV(ipp2, Page);
  if(!t2)
    t2 = _T("\0");
  return !_tcsicmp(t1, t2);
}
//----------------------------------------------------------------------------
void clientTreeViewPages::addTreeStdMsgDll(HTREEITEM root, uint pgType)
{
  if(!pMP)
    return;
  managePages& MP = *pMP;

  MP.setOrder(managePages::byOther);
  do {
    infoPrph Ipp;
    SET_rO(Ipp, pageType, pgType);
    MP.find(&Ipp);
    const infoPrph* ipp;
    while(MP.getCurr(ipp)) {
      if(GET_pV(ipp, pageType) != pgType) {
        if(!MP.setNextPrph())
          break;
        }
      else
        break;
      }
    } while(false);

  LPTSTR buff = new TCHAR[500];
  TV_INSERTSTRUCT is;

  HTREEITEM node = TVI_FIRST;

  const infoPrph* oldIpp = 0;
  do {
    const infoPrph* ipp;
    if(!MP.getCurr(ipp))
      break;
    if(GET_pV(ipp, pageType) != pgType)
      break;

    if(!oldIpp || !isSameInfo(oldIpp, ipp)) {
      oldIpp = ipp;
      LPCTSTR t = GET_pV(ipp, Page);
      if(!t)
        t = _T("");
      uint obj = GET_pV(ipp, objType);
      wsprintf(buff, _T("%s{ %s }"), t, getStrObject(obj));

      if(infoPrph::eManStopAddrOper == obj || infoPrph::eManStopAddrListBox == obj)
        obj = infoPrph::eManStop;
      else if(infoPrph::eMaxObj <= obj)
        obj = infoPrph::eManStop;
      int ixImage = obj - infoPrph::eTrend + 8;
      is.hParent = root;
      is.hInsertAfter = node;
      is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
      is.item.hItem = 0;
      is.item.pszText = buff;
      is.item.iImage = ixImage;
      is.item.iSelectedImage = ixImage;

      node = TreeView_InsertItem(*getTree(), &is);
      }
    } while(MP.setNextPrph());
  delete []buff;
}
//----------------------------------------------------------------------------
void clientTreeViewPages::addTreeStdMsg(HTREEITEM root)
{
  addTreeStdMsgDll(root, infoPrph::eStdMsg);
}
//----------------------------------------------------------------------------
void clientTreeViewPages::addTreeNorm(HTREEITEM root)
{
  addTreeStdMsgDll(root, infoPrph::eNorm);
}
//----------------------------------------------------------------------------
void clientTreeViewPages::addTreeDll(HTREEITEM root)
{
  addTreeStdMsgDll(root, infoPrph::eDll);
}
//----------------------------------------------------------------------------
void clientTreeViewPages::addTreeAlarmAssoc(HTREEITEM root)
{
  addTreeStdMsgDll(root, infoPrph::eAlarmAssoc);
}
//----------------------------------------------------------------------------
static bool isOnSet(PVect<infoRecurse>& setPage, infoPages* ip, int level)
{
  uint nElem = setPage.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!_tcsicmp(ip->getPageCurr(), setPage[i].page) &&  setPage[i].level <= level) {
      if(setPage[i].repeat)
        return true;
      ++setPage[i].repeat;
      return false;
      }
    }
  setPage[nElem].page = ip->getPageCurr();
  setPage[nElem].repeat = 0;
  setPage[nElem].level = level;
  return false;
}
//----------------------------------------------------------------------------
void clientTreeViewPages::addTree(HTREEITEM root, const infoPages* rootPages, uint level, PVect<infoRecurse>& setPage)
{
  if(level >= MAX_LEVEL)
    return;
  LPTSTR buff = new TCHAR[500];
  TV_INSERTSTRUCT is;

  HTREEITEM node = TVI_FIRST;

  const PVect<infoPages*>& child = rootPages->getChild();
  uint nElem = child.getElem();

  for(uint i = 0; i < nElem; ++i) {
    _tcscpy(buff, child[i]->getPageCurr());

    int ixImage = 0;
    uint pgt = child[i]->getPageType();
    if(pgt) {
      ixImage = pgt - 1;
      _tcscat(buff, _T("{ "));
      _tcscat(buff, PgType[pgt]);
      if(infoPrph::eLink1 == pgt) {
        const infoPrph* ip = child[i]->getInfoOpenedBy();
        if(ip) {
          LPTSTR p = buff + _tcslen(buff);
          wsprintf(p, _T("-%d"), ip->getOffsLink());
          }
        }
      _tcscat(buff, _T(" }"));
      }
    is.hParent = root;
    is.hInsertAfter = node;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    is.item.hItem = 0;
    is.item.pszText = buff;
    is.item.iImage = ixImage;
    is.item.iSelectedImage = ixImage;

    node = TreeView_InsertItem(*getTree(), &is);
    if(child[i]->getLevel() >= rootPages->getLevel() && !isOnSet(setPage, child[i], level))
      addTree(node, child[i], level + 1, setPage);
    }
  delete []buff;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PD_AllPages::PD_AllPages(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), Split(0)
{}
//----------------------------------------------------------------------------
PD_AllPages::~PD_AllPages()
{
  destroy();
  delete Split;
}
//----------------------------------------------------------------------------
bool PD_AllPages::create()
{
  PWin* w1 = new clientTreeViewPages(this, IDD_TREE);
  PWin* w2 = new clientListViewPages(this, IDD_LIST);
  Split = new PSplitter(this, w1, w2, 300, 5, PSplitter::PSW_HORZ);
  if(!baseClass::create())
    return false;
  return Split->create();
}
//----------------------------------------------------------------------------
HWND PD_AllPages::getTreeView()
{
  return *Split->getW1();
}
//----------------------------------------------------------------------------
HWND PD_AllPages::getListView()
{
  return *Split->getW2();
}
//----------------------------------------------------------------------------
LRESULT PD_AllPages::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(Split)
    Split->windowProcSplit(hwnd, message, wParam, lParam);
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_AllPages::setManPage(managePages* mp)
{
  PWin* w = PWin::getWindowPtr(getListView());

  clientListViewPages* clvp = (clientListViewPages*)w;
  if(clvp)
    clvp->setManPage(mp);


  w = PWin::getWindowPtr(getTreeView());

  clientTreeViewPages* ctvp = (clientTreeViewPages*)w;
  if(ctvp)
    ctvp->setManPage(mp);
}
//----------------------------------------------------------------------------
