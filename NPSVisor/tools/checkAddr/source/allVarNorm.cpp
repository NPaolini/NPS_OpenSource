//----------- allVarNorm.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "allVarNorm.h"
#include "HeaderMsg.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PD_AllVarNorm::PD_AllVarNorm(PWin* parent, uint resId, HINSTANCE hinstance) :
  baseClass(parent, resId, hinstance), pMP(0)
{}
//----------------------------------------------------------------------------
PD_AllVarNorm::~PD_AllVarNorm()
{
  destroy();
}
//----------------------------------------------------------------------------
#define TV_BKG_COLOR RGB(0xe1, 0xdf, 0xbe)
#define TV_TXT_COLOR RGB(0, 0, 0xa3)
//----------------------------------------------------------------------------
HIMAGELIST PD_AllVarNorm::makeImageList()
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
extern LPCTSTR getFirstDig(LPCTSTR p);
//----------------------------------------------------------------------------
extern LPCTSTR getNextDig(LPCTSTR p);
//----------------------------------------------------------------------------
static void unformatNorm(LPCTSTR buff, int& norm)
{
  norm = -1;
  buff = getFirstDig(buff);
  if(!buff)
    return;
  norm = _ttoi(buff);
}
//----------------------------------------------------------------------------
struct usedNorm
{
  uint id;
  LPCTSTR text;
};
//----------------------------------------------------------------------------
extern void makeSetNorm(setOfString& sos);
//----------------------------------------------------------------------------
static void makeUsedList(PVect<usedNorm>& nSet, const bool* lNorm, bool used)
{
  setOfString set;
  makeSetNorm(set);

  if(!set.setFirst())
    return;
  uint nEl = 0;
  do {
    uint id = set.getCurrId();
    if(id > 2 && id < MAX_NORMAL) {
      if(!(used ^ lNorm[id])) {
        nSet[nEl].id = id;
        nSet[nEl].text = str_newdup(set.getCurrString());
        ++nEl;
        }
      }
    } while(set.setNext());
}
//----------------------------------------------------------------------------
static void formatNorm(LPTSTR buff, const usedNorm& nun)
{
  wsprintf(buff, _T("[%3d] - %s"), nun.id, nun.text);
}
//----------------------------------------------------------------------------
void PD_AllVarNorm::fill()
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

  const bool* lNorm = MP.getListNorm();
  PVect<usedNorm> nSet;
  makeUsedList(nSet, lNorm, true);
  uint nElem = nSet.getElem();
  for(uint i = 0; i < nElem; ++i) {
    formatNorm(buff, nSet[i]);
    delete []nSet[i].text;
    is.hParent = root;
    is.hInsertAfter = parent;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    is.item.hItem = 0;
    is.item.pszText = buff;
    int ixImage = 2;

    is.item.iImage = ixImage;
    is.item.iSelectedImage = ixImage;

    parent = TreeView_InsertItem(*getTree(), &is);
    _tcscpy(buff, _T("*"));

    is.hParent = parent;
    is.hInsertAfter = TVI_FIRST;
    is.item.mask = TVIF_TEXT;
    TreeView_InsertItem(*getTree(), &is);

    }

  is.item.iImage = IMG_OK_TYPE;
  is.item.iSelectedImage = IMG_OK_TYPE;
  wsprintf(buff, _T("%s - [%d]"), _T("Normalizer active"), nElem);
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  is.item.pszText = buff;
  TreeView_SetItem(*getTree(), &is.item);
  TreeView_Expand(*getTree(), root, TVE_EXPAND);


  nSet.reset();
  makeUsedList(nSet, lNorm, false);
  nElem = nSet.getElem();
  if(nElem) {
    is.hParent = root_t;
    is.hInsertAfter = root;
    if(root_t)
      is.item.mask = TVIF_HANDLE;
    else
      is.item.mask = 0;
    is.item.hItem = root_t;

    HTREEITEM root2 = TreeView_InsertItem(*getTree(), &is);
    parent = TVI_FIRST;

    for(uint i = 0; i < nElem; ++i) {
      formatNorm(buff, nSet[i]);
      delete []nSet[i].text;

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
    wsprintf(buff, _T("%s - [%d]"), _T("Normalizer not used"), nElem);
    is.item.hItem = root2;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    TreeView_SetItem(*getTree(), &is.item);
    }
}
//----------------------------------------------------------------------------
void PD_AllVarNorm::fill_sub(LPNM_TREEVIEWW twn)
{
  if(twn->action == TVE_EXPAND) {
    do {
      HTREEITEM parent = twn->itemNew.hItem;
      if(!parent)
        break;
      HTREEITEM child = TreeView_GetChild(*getTree(), parent);
      if(!child)
        break;
      TCHAR buff[200];
      TV_ITEM tvi;
      tvi.mask = TVIF_TEXT;
      tvi.hItem = child;
      tvi.pszText = buff;
      tvi.cchTextMax = SIZE_A(buff);
      if(!TreeView_GetItem(*getTree(), &tvi))
        break;

      if(*buff == _T('*')) {
        TreeView_DeleteItem(*getTree(), child);
        if(_T('*') == buff[1])
          fillSub2(parent);
        else if(_T('#') == buff[1])
          fillSub3(parent);
        else
          fillSub1(parent);
        }
      } while(false);
    }
}
//----------------------------------------------------------------------------
extern LPCTSTR getStrDataType(uint ix);
//----------------------------------------------------------------------------
static
bool isSame1(const infoPrph* ipp1, const infoPrph* ipp2)
{
  if(GET_pV(ipp1, Prph) != GET_pV(ipp2, Prph))
    return false;
  return true;
}
//----------------------------------------------------------------------------
static
bool isSame2(const infoPrph* ipp1, const infoPrph* ipp2)
{
  if(GET_pV(ipp1, Prph) != GET_pV(ipp2, Prph))
    return false;
  if(GET_pV(ipp1, Addr) != GET_pV(ipp2, Addr))
    return false;
  if(GET_pV(ipp1, dataType) != GET_pV(ipp2, dataType))
    return false;
  if(GET_pV(ipp1, nBit) != GET_pV(ipp2, nBit))
    return false;
  if(GET_pV(ipp1, Offs) != GET_pV(ipp2, Offs))
    return false;
  return true;
}
//----------------------------------------------------------------------------
void formatRowPrph(LPTSTR buff, const infoPrph* ipp)
{
  LPCTSTR format = _T("Addr[%4d] - Type[%-11s] - nBit[%2d] - Offs[%2d]");
  uint ix = GET_pV(ipp, dataType);
  wsprintf(buff, format, GET_pV(ipp, Addr), getStrDataType(ix), GET_pV(ipp, nBit), GET_pV(ipp, Offs));
}
//----------------------------------------------------------------------------
void unformatRowPrph(LPCTSTR buff, infoPrph* ipp)
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
void formatRowOnlyPrph(LPTSTR buff, const infoPrph* ipp, DWORD count)
{
  LPCTSTR format = _T("Prph[%2d] -> %d");
  wsprintf(buff, format, GET_pV(ipp, Prph), count);
}
//----------------------------------------------------------------------------
void unformatRowOnlyPrph(LPCTSTR buff, infoPrph* ipp)
{
  LPCTSTR p = getFirstDig(buff);
  if(p) {
    uint v = _ttoi(p);
    SET_pO(ipp, Prph, v);
    }
}
//----------------------------------------------------------------------------
void PD_AllVarNorm::fillSub1(HTREEITEM parent)
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
  if(!TreeView_GetItem(*getTree(), &tvi))
    return;

  int ixNorm;
  unformatNorm(Buff, ixNorm);
  if(-1 == ixNorm)
    return;

  MP.setOrder(managePages::byNorm);
  do {
    infoPrph prph;
    SET_rO(prph, Norm, ixNorm);
    MP.find(&prph);
    const infoPrph* ipp;
    while(MP.getCurr(ipp)) {
      if(GET_pV(ipp, Norm) != ixNorm) {
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
    if(GET_pV(ipp, Norm) != ixNorm)
      break;

    if(!isSame1(&oldIpp, ipp)) {
      formatRowOnlyPrph(Buff, &oldIpp, count);
      int ix = GET_rV(oldIpp, Prph);
      if(ix > 1 && ix <= 9)
        ix = 1;
      else
        ix = 0;
      child = add_Child(Buff, &oldIpp, parent, child, ix);
      oldIpp = *ipp;
      count = 0;
      }
    ++count;
    } while(MP.setNextPrph());
  if(count) {
    formatRowOnlyPrph(Buff, &oldIpp, count);
      int ix = GET_rV(oldIpp, Prph);
      if(ix > 1 && ix <= 9)
        ix = 1;
      else
        ix = 0;
    add_Child(Buff, &oldIpp, parent, child, ix);
    }
}
//----------------------------------------------------------------------------
extern int formatRowObj(managePages& MP, LPTSTR buff, const infoPrph* ipp);
//----------------------------------------------------------------------------
void PD_AllVarNorm::fillSub2(HTREEITEM parent)
{
  if(!pMP)
    return;
  managePages& MP = *pMP;

  HTREEITEM parOfParent = TreeView_GetParent(*getTree(), parent);
  if(!parOfParent)
    return;

  TCHAR Buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = parOfParent;
  tvi.pszText = Buff;
  tvi.cchTextMax = SIZE_A(Buff);
  if(!TreeView_GetItem(*getTree(), &tvi))
    return;

  int ixNorm;
  unformatNorm(Buff, ixNorm);
  if(-1 == ixNorm)
    return;

  tvi.hItem = parent;
  if(!TreeView_GetItem(*getTree(), &tvi))
    return;

  infoPrph Ipp;

  unformatRowOnlyPrph(Buff, &Ipp);
  SET_rO(Ipp, Norm, ixNorm);

  HTREEITEM child = TVI_FIRST;
  MP.setOrder(managePages::byNorm);
  const infoPrph* oldIpp = 0;
  do {
    MP.find(&Ipp);
    const infoPrph* ipp;
    while(MP.getCurr(ipp)) {
      if(GET_pV(ipp, Norm) != ixNorm || GET_pV(ipp, Prph) != GET_rV(Ipp, Prph)) {
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
    if(GET_pV(ipp, Norm) != ixNorm || GET_pV(ipp, Prph) != GET_rV(Ipp, Prph))
      break;

    if(!oldIpp || !isSame2(oldIpp, ipp)) {
      oldIpp = ipp;
      formatRowPrph(Buff, ipp);

      TV_INSERTSTRUCT is;
      is.hParent = parent;
      is.hInsertAfter = child;
      is.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
      is.item.hItem = 0;
      is.item.pszText = Buff;
      is.item.iImage = 2;
      is.item.iSelectedImage = 2;

      child = TreeView_InsertItem(*getTree(), &is);

      _tcscpy(Buff, _T("*#"));

      is.hParent = child;
      is.hInsertAfter = TVI_FIRST;
      is.item.mask = TVIF_TEXT;
      TreeView_InsertItem(*getTree(), &is);
      }
    } while(MP.setNextPrph());

}
//----------------------------------------------------------------------------
void PD_AllVarNorm::fillSub3(HTREEITEM parent)
{
  if(!pMP)
    return;
  managePages& MP = *pMP;

  HTREEITEM parOfParent = TreeView_GetParent(*getTree(), parent);
  if(!parOfParent)
    return;

  HTREEITEM grandParOfParent = TreeView_GetParent(*getTree(), parOfParent);
  if(!grandParOfParent)
    return;
  TCHAR Buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = grandParOfParent;
  tvi.pszText = Buff;
  tvi.cchTextMax = SIZE_A(Buff);
  if(!TreeView_GetItem(*getTree(), &tvi))
    return;

  int ixNorm;
  unformatNorm(Buff, ixNorm);
  if(-1 == ixNorm)
    return;

  tvi.hItem = parOfParent;
  if(!TreeView_GetItem(*getTree(), &tvi))
    return;

  infoPrph Ipp;
  unformatRowOnlyPrph(Buff, &Ipp);

  tvi.hItem = parent;
  if(!TreeView_GetItem(*getTree(), &tvi))
    return;

  unformatRowPrph(Buff, &Ipp);
  SET_rO(Ipp, Norm, ixNorm);

  HTREEITEM child = TVI_FIRST;
  MP.setOrder(managePages::byNorm);
  do {
    MP.find(&Ipp);
    const infoPrph* ipp;
    while(MP.getCurr(ipp)) {
      if(GET_pV(ipp, Norm) != ixNorm || GET_pV(ipp, Prph) != GET_rV(Ipp, Prph)) {
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
    if(GET_pV(ipp, Norm) != ixNorm || GET_pV(ipp, Prph) != GET_rV(Ipp, Prph))
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

      child = TreeView_InsertItem(*getTree(), &is);
      }
    } while(MP.setNextPrph());

}
//----------------------------------------------------------------------------
