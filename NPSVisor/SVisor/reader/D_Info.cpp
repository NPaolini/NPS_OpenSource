//------------- D_Info.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdio.h>
#include "d_info.h"
#include "commonInfo.h"
#include "p_date.h"
#include "printInfo.h"
#include "svPrinter.h"
#include "resource.h"
//----------------------------------------------------------------------------
TD_Info::TD_Info(PWin* parent, const vCodeType& oper,
    const vRangeTime& date, uint resId, HINSTANCE hinst) :
    baseClass(parent, resId, hinst), ImageList(0),
    //Oper(oper),
    Date(date), InfoOper(0), InfoCause(0)

{
  Tree = new PTreeView(this, IDC_TREEVIEW_INFO);
  Oper = oper;
  vCodeType test = oper;
}
//----------------------------------------------------------------------------
TD_Info::~TD_Info()
{
  destroy();
  if(ImageList)
    ImageList_Destroy(ImageList);

  delete InfoOper;
  delete InfoCause;
}
//----------------------------------------------------------------------------
//------------- ---------------------------------------------------------------
void TD_Info::fill_image(HIMAGELIST iml)
{
  int idiIcons[] = { IDI_ARROW, IDI_LOGS, IDI_OPER, IDI_DATE, IDI_ARROW_DN };
  for(int i = 0; i < SIZE_A(idiIcons); ++i) {
    HICON hi = LoadIcon(getHInstance(), MAKEINTRESOURCE(idiIcons[i]));
    ImageList_AddIcon(iml, hi);
    }
}
//----------------------------------------------------------------------------
#define CX_ICON 16
#define CY_ICON 16
//----------------------------------------------------------------------------
bool TD_Info::create()
{
  if(!baseClass::create())
    return false;
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON_NPS));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));

  HFONT f = (HFONT)GetStockObject(ANSI_FIXED_FONT);

  ImageList = ImageList_Create(CX_ICON, CY_ICON, ILC_COLOR4, 5, 0);

  fill_image(ImageList);

  TreeView_SetImageList(*Tree, ImageList, 0);
  Tree->setFont(f);

  InfoOper = new manageInfo(_T("OP"));
  InfoCause = new manageInfo(_T("CA"));

  int nElem = Oper.getElem();
  for(int i = 0; i < nElem; ++i) {
    infoFilterCode filter(Oper[i]);
    InfoOper->addFilterCode(filter);
    }
    // nessun operatore selezionato equivale a tutti
    // occorre ricreare la lista inserendo tutti gli operatori
  if(!nElem) {
//    codeOper allOper;
    nElem = allOper.getElem();
    Oper.setDim(nElem);
    for(int i = 0; i < nElem; ++i)
      Oper[i] = allOper.getCode(i);
    }

  // è necessario aggiungere un filtro nullo alle cause
  // per abilitare il filtro negato sul normal_stop
  do {
    infoFilterCode filter(0);
    InfoCause->addFilterCode(filter);
    } while(0);

  nElem = Date.getElem();
  for(int i = 0; i < nElem; ++i) {
    infoFilterDate filter(Date[i]);
    InfoOper->addFilterDate(filter);
    InfoCause->addFilterDate(filter);
    }

  InfoOper->perform();
  InfoCause->perform();

  fill();
  return true;
}
//----------------------------------------------------------------------------
LRESULT TD_Info::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      do {
        SIZE sz = { LOWORD(lParam), HIWORD(lParam) };
        EvSize(wParam, sz);
        } while(false);
      break;
    case WM_GETMINMAXINFO:
      EvGetMinMaxInfo(*(LPMINMAXINFO)lParam);
      break;
    case WM_NOTIFY:
#if 1
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
            return TVNItemexpanding((LPNM_TREEVIEWW)lParam);;
          }
        }
      break;
#else
      if(((NMHDR*)lParam)->hwndFrom == *Tree) {
        if(((NMHDR*)lParam)->code == TVN_ITEMEXPANDING)
          return TVNItemexpanding((LPNM_TREEVIEWW)lParam);
        }
      break;
#endif
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
bool TD_Info::TVNItemexpanding(LPNM_TREEVIEWW twn)
{
  fill_sub(twn);
  return false;
}
//----------------------------------------------------------------------------
void TD_Info::fillSubAlarm(HTREEITEM /*parent*/)
{
}
//----------------------------------------------------------------------------
void TD_Info::fill_sub(LPNM_TREEVIEWW twn)
{
  if(twn->action == TVE_EXPAND) {
    LockWindowUpdate(*Tree);
    do {
      HTREEITEM parent = twn->itemNew.hItem;
      if(!parent)
        break;
      HTREEITEM child = TreeView_GetChild(*Tree, parent);
      if(!child)
        break;
      TCHAR buff[200];
      TV_ITEM tvi;
      tvi.mask = TVIF_TEXT;
      tvi.hItem = child;
      tvi.pszText = buff;
      tvi.cchTextMax = SIZE_A(buff);
      if(!TreeView_GetItem(*Tree, &tvi))
        break;

      if(*buff == _T('*')) {
        TreeView_DeleteItem(*Tree, child);
        if(_T('*') == buff[1])
          fillSub2(parent);
        else if(_T('#') == buff[1])
          fillSubAlarm(parent);
        else
          fillSub1(parent);
        }
      } while(false);
    LockWindowUpdate(0);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MIN_DIM_X 400
#define MIN_DIM_Y 200
#define LEFT_PR   rect.Width()/2-r.Width()-5
#define RIGHT_PR  rect.Width()/2+5

#define LEFT_NO_PR  (rect.Width()-r.Width())/2

#define DELTA_Y_BTN 20
//----------------------------------------------------------------------------
void TD_Info::EvGetMinMaxInfo(MINMAXINFO far& minmaxinfo)
{
  minmaxinfo.ptMinTrackSize.x = MIN_DIM_X;
  minmaxinfo.ptMinTrackSize.y = MIN_DIM_Y;
}
//----------------------------------------------------------------------------
void TD_Info::EvSize(uint sizeType, SIZE& size)
{
  #define OFFS_H_TREE 5
  #define OFFS_V_TREE 5
  PRect rect;
  GetClientRect(*this, rect);
  rect.Inflate(-OFFS_H_TREE, -OFFS_V_TREE);
  Tree->setWindowPos(0, rect, SWP_NOZORDER);
}
//--------------------------------------------------------------------------
static smartPointerConstString getFormatString(uint id, LPCTSTR def)
{
  smartPointerConstString format(getString(id), false);
  if(!format)
    format = smartPointerConstString(def, false);
  else
    format = getStringByLangSimple(format);
  return format;
}
//--------------------------------------------------------------------------
void TD_Info::fill(manageInfo* master, manageInfo* slave, int idIcon)
{
  HTREEITEM root_t = TreeView_GetRoot(*Tree);
  TV_INSERTSTRUCT is;
  is.hParent = root_t;
  is.hInsertAfter = TVI_FIRST;
  if(root_t)
    is.item.mask = TVIF_HANDLE;
  else
    is.item.mask = 0;
  is.item.hItem = root_t;

  HTREEITEM root = TreeView_InsertItem(*Tree, &is);
  TCHAR buff[500];
  FILETIME totTime;
  MK_I64(totTime) = 0;
  int day;
  int hour;
  int minute;
  int second;

  smartPointerConstString format = getFormatString(ID_TREE_INFO_NAME_AND_TIME, _T("[%3d][%s][Giorni %d + Ore %02d:%02d:%02d]"));

  const infoList& listMaster = master->getInfo();
  const infoList& listSlave = slave->getInfo();

  if(!listMaster.setFirst())
    return;

  int count = 0;
  HTREEITEM parent = TVI_FIRST;
  do {

    FILETIME ft = calcTime(listMaster, listSlave);
    extractTime(ft, day, hour, minute, second);
    totTime += ft;
    codeType code = listMaster.getCurrCode();
    TCHAR name[64];
    makeNameFromCodeMaster(name, SIZE_A(name), code);
    code = useCodeOrCountMaster(code, ++count);
    wsprintf(buff, format, code, name, day, hour, minute, second);
    is.hParent = root;
    is.hInsertAfter = parent;
    is.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    is.item.hItem = 0;
    is.item.pszText = buff;
    is.item.iImage = idIcon;
    is.item.iSelectedImage = idIcon;

    parent = TreeView_InsertItem(*Tree, &is);
    _tcscpy_s(buff, SIZE_A(buff), _T("*"));

    is.hParent = parent;
    is.hInsertAfter = TVI_FIRST;
    is.item.mask = TVIF_TEXT;
    TreeView_InsertItem(*Tree, &is);

    } while(listMaster.setNext());

  extractTime(totTime, day, hour, minute, second);

  format = getFormatString(ID_TREE_INFO_TOT, _T("[Totale Elementi: %d][Totale Giorni %d + Ore %02d:%02d:%02d]"));

  _stprintf_s(buff, SIZE_A(buff), format, count, day, hour, minute, second);
  is.item.hItem = root;
  is.item.mask = TVIF_HANDLE | TVIF_TEXT;
  TreeView_SetItem(*Tree, &is.item);
  TreeView_Expand(*Tree, root, TVE_EXPAND);

}
//----------------------------------------------------------------------------
void TD_Info::fillSub1(HTREEITEM parent, const infoList& listMaster, const infoList& listSlave, int idIcon)
{
  if(!listSlave.setFirst())
    return;
  int day;
  int hour;
  int minute;
  int second;
  smartPointerConstString format = getFormatString(ID_TREE_INFO_NAME_AND_TIME, _T("[%3d][%s][Giorni %d + Ore %02d:%02d:%02d]"));

  TCHAR Buff[500];
  int count = 0;
  HTREEITEM child = TVI_FIRST;
  do {
    FILETIME ft = calcTimeMasterBySlave(listMaster, listSlave);

    extractTime(ft, day, hour, minute, second);
    codeType code = listSlave.getCurrCode();
    TCHAR name[64];
    makeNameFromCodeSlave(name, SIZE_A(name), code);
    code = useCodeOrCountSlave(code, ++count);

    wsprintf(Buff, format, code, name, day, hour, minute, second);

    TV_INSERTSTRUCT is;
    is.hParent = parent;
    is.hInsertAfter = child;
    is.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    is.item.hItem = 0;
    is.item.pszText = Buff;
    is.item.iImage = idIcon;
    is.item.iSelectedImage = idIcon;

    child = TreeView_InsertItem(*Tree, &is);
    _tcscpy_s(Buff, SIZE_A(Buff), _T("**"));

    is.hParent = child;
    is.hInsertAfter = TVI_FIRST;
    is.item.mask = TVIF_TEXT;
    TreeView_InsertItem(*Tree, &is);
    } while(listSlave.setNext());
}
//----------------------------------------------------------------------------
void TD_Info::fillSub2(HTREEITEM parent, manageInfo* infoMaster, manageInfo* infoSlave)
{
  codeType master;
  codeType slave;
  getCodeMasterAndSlave(parent, master, slave);
  if(!slave)
    return;

  smartPointerConstString title = getFormatString(ID_TREE_INFO_TIME_TITLE, _T("[         Da        ][         A         ][Durata]"));

  TCHAR Buff[500];
  _tcscpy_s(Buff, SIZE_A(Buff), title);
  TV_INSERTSTRUCT is;
  is.hParent = parent;
  is.hInsertAfter = TVI_FIRST;
  is.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
  is.item.hItem = 0;
  is.item.pszText = Buff;
  is.item.iImage = I_DN;
  is.item.iSelectedImage = I_DN;
  HTREEITEM child = TreeView_InsertItem(*Tree, &is);

  const infoList& listSlave = infoSlave->getInfo();
  if(!listSlave.find(slave))
    return;

  const infoList& listMaster = infoMaster->getInfo();
  if(!listMaster.find(master))
    return;

  LPCTSTR formatTime = getString(ID_TREE_INFO_TIME);
  if(!formatTime)
    formatTime = _T("[%s][%s][%s]");

  const vRangeTime& vRangeMaster = listMaster.getCurrRange();
  const vRangeTime& vRangeSlave = listSlave.getCurrRange();

  int nElem = vRangeMaster.getElem();
  int nRange = vRangeSlave.getElem();
  for(int i = 0; i < nElem; ++i) {

    const rangeTime& rt = vRangeMaster[i];
    int j = findFirstTime(rt.from, vRangeSlave);
    if(j)
      --j;
    for(; j < nRange; ++j) {
      if(overTime(rt, vRangeSlave[j]))
        break;
      FILETIME from = maxTime(rt.from, vRangeSlave[j].from);
      FILETIME to =  minTime(rt.to, vRangeSlave[j].to);
      if(to <= from)
        continue;
      addNodeTime(Buff, from, to, formatTime);

      is.hInsertAfter = child;
      is.item.mask = TVIF_TEXT;
      is.item.pszText = Buff;
      child = TreeView_InsertItem(*Tree, &is);
      }
    }

}
//----------------------------------------------------------------------------
FILETIME TD_Info::calcTime(const infoList& listMaster, const infoList& listSlave)
{
  FILETIME ft;
  MK_I64(ft) = 0;

  const vRangeTime& vRangeMaster = listMaster.getCurrRange();

  if(!listSlave.setFirst())
    return ft;

  int nElem = vRangeMaster.getElem();
  for(int i = 0; i < nElem; ++i) {
    listSlave.setFirst();
    do {
      const vRangeTime& vRangeSlave = listSlave.getCurrRange();
      ft += addTime(vRangeMaster[i], vRangeSlave);
      } while(listSlave.setNext());
    }
  return ft;
}
//--------------------------------------------------------------------------
FILETIME TD_Info::calcTimeMasterBySlave(const infoList& listMaster, const infoList& listSlave)
{
  FILETIME ft;
  MK_I64(ft) = 0;
  const vRangeTime& vRangeSlave = listSlave.getCurrRange();

  const vRangeTime& vRangeMaster = listMaster.getCurrRange();

  int nElem = vRangeMaster.getElem();
  for(int i = 0; i < nElem; ++i)
    ft += addTime(vRangeMaster[i], vRangeSlave);

  return ft;
}
//--------------------------------------------------------------------------
void TD_Info::makeNameOper(LPTSTR target, size_t len, const codeType& code)
{
  copyStrZ(target, allOper.getName(code), len);
}
//--------------------------------------------------------------------------
void TD_Info::makeNameCause(LPTSTR target, size_t len, const codeType& code)
{
#if 1
  int id = code >= FIRST_CODE_DEFAULT ? (FIRST_ID_DEFAULT + FIRST_CODE_DEFAULT - code) : (code + ID_CAUSE_TITLE_INIT - 1);
  LPCTSTR name = getString(id);
  if(!name)
    _tcscpy_s(target, len, getStringOrId(id));
  else {
    smartPointerConstString tmp = getStringByLangSimple(name);
    _tcscpy_s(target, len, tmp);
    }
#else
  LPCTSTR name;
  if(code >= FIRST_CODE_DEFAULT)
    name = getStringOrId(FIRST_ID_DEFAULT + FIRST_CODE_DEFAULT - code);
  else
    name = getStringOrId(code + ID_CAUSE_TITLE_INIT - 1);
  return name;
#endif
}
//--------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TD_Info::fill4print(manageInfo* master, manageInfo* slave, bool alsoSub)
{
  FILETIME totTime;
  MK_I64(totTime) = 0;
  const infoList& listMaster = master->getInfo();
  const infoList& listSlave = slave->getInfo();

  if(!listMaster.setFirst())
    return false;
  if(!listSlave.setFirst())
    return false;

  P_File pf(FILE_TMP_PREP, P_CREAT);
  if(!pf.P_open())
    return false;

  headerRow4print hr;
  hr.ftFrom = Date[0].from;
  hr.ftTo = Date[0].to;
  pf.P_write(&hr, sizeof(hr));

  row4print rp;
  memset(&rp, 0, sizeof(rp));
  if(!alsoSub)
    pf.P_write(&rp, sizeof(rp));

  bool first = true;
  do {
    FILETIME ft = calcTime(listMaster, listSlave);
    totTime += ft;
    codeType code = listMaster.getCurrCode();

    if(alsoSub) {
      if(!first) {
        rp.id = 2;
        rp.ft = ft;
        rp.code = code;
        pf.P_write(&rp, sizeof(rp));
        pf.P_write(&hr, sizeof(hr));
        rp.id = 1;
        rp.ft = totTime;
        rp.code = 0;
        pf.P_write(&rp, sizeof(rp));
        rp.id = 2;
        rp.ft = ft;
        rp.code = code;
        pf.P_write(&rp, sizeof(rp));
        }
      else {
        rp.id = 1;
        rp.ft = ft;
        rp.code = 0;
        pf.P_write(&rp, sizeof(rp));
        }

      if(listSlave.setFirst()) {
        do {
          FILETIME fft = calcTimeMasterBySlave(listMaster, listSlave);
          codeType ccode = listSlave.getCurrCode();
          rp.id = 3;
          rp.ft = fft;
          rp.code = ccode;
          pf.P_write(&rp, sizeof(rp));
          } while(listSlave.setNext());
        }
      rp.id = 2;
      }
    else
      rp.id = 3;
    rp.ft = ft;
    rp.code = code;
    if(first || !alsoSub)
      pf.P_write(&rp, sizeof(rp));
    first = false;
    } while(listMaster.setNext());

  if(!alsoSub) {
    pf.P_seek(sizeof(hr));
    rp.id = 1;
    rp.ft = totTime;
    rp.code = 0;
    pf.P_write(&rp, sizeof(rp));
    }
  return true;
}
//----------------------------------------------------------------------------
void TD_Info::print(bool preview, bool setup)
{
  InfoOper = new manageInfo(_T("OP"));
  InfoCause = new manageInfo(_T("CA"));

  int nElem = Oper.getElem();
  for(int i = 0; i < nElem; ++i) {
    infoFilterCode filter(Oper[i]);
    InfoOper->addFilterCode(filter);
    }
    // nessun operatore selezionato equivale a tutti
    // occorre ricreare la lista inserendo tutti gli operatori
  if(!nElem) {
    return;
//    codeOper allOper;
//    nElem = allOper.getElem();
//    Oper.setDim(nElem);
//    for(int i = 0; i < nElem; ++i)
//      Oper[i] = allOper.getCode(i);
    }

  // è necessario aggiungere un filtro nullo alle cause
  // per abilitare il filtro negato sul normal_stop
  do {
    infoFilterCode filter(0);
    InfoCause->addFilterCode(filter);
    } while(0);

  nElem = Date.getElem();
  for(int i = 0; i < nElem; ++i) {
    infoFilterDate filter(Date[i]);
    InfoOper->addFilterDate(filter);
    InfoCause->addFilterDate(filter);
    }

  InfoOper->perform();
  InfoCause->perform();

  if(!fill4print())
    return;

//  printInfo* pi = allocPrintInfo(GetApplication()->GetMainWindow(), "~prn.tmp");
  static svPrinter Printer(this);
  printTree* pi = allocPrintTree(this, &Printer, FILE_TMP_PREP);
//  const bool setup = false;
  if(preview)
    pi->Preview(setup);
  else
    pi->Print(setup);
  delete pi;
}
//----------------------------------------------------------------------------
