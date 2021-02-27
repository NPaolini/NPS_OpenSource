//----------- alarmEditorDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "alarmEditorDlg.h"
#include "common.h"
//----------------------------------------------------------------------------
static
bool checkFile(LPCTSTR ext, uint id, PVect<uint>& idA)
{
  TCHAR Filename[_MAX_PATH];
  wsprintf(Filename, _T("alarm_%d.%s"), id, ext);
  makeSystemFilename(Filename, Filename);
  if(P_File::P_exist(Filename)) {
    uint n = idA.getElem();
    idA[n] = id;
    return true;
    }
  wsprintf(Filename, _T("#alarm_%d.%s"), id, ext);
  makeSystemFilename(Filename, Filename);
  if(P_File::P_exist(Filename)) {
    uint n = idA.getElem();
    idA[n] = id;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
static void convertOldFile()
{
  TCHAR Filename2[_MAX_PATH] = _T("alarm")PAGE_EXT;
  makeSystemFilename(Filename2, Filename2);
  uint exist = P_File::P_exist(Filename2) ? 1 : 0;

  TCHAR Filename[_MAX_PATH] = _T("#alarm_2")PAGE_EXT;
  makeSystemFilename(Filename, Filename);
  exist |= P_File::P_exist(Filename) ? 2 : 0;

  if(!exist || 3 == exist)
    return;
  if(1 == exist)
    MoveFile(Filename2, Filename);
}
//----------------------------------------------------------------------------
PD_alarmEditorDlg::PD_alarmEditorDlg(PWin* parent, uint resId, HINSTANCE hinstance) :
    PModDialog(parent, resId, hinstance), onJob(jobbingAlarm), currPage(0),
    ImageList(0)

{
}
//----------------------------------------------------------------------------
PD_alarmEditorDlg::~PD_alarmEditorDlg()
{
  clientAlarm.reset();
  clientAssoc.reset();
  destroy();
  if(ImageList)
    ImageList_Destroy(ImageList);
}
//----------------------------------------------------------------------------
template <typename T, typename S>
T* getSibl(PVect<T*>& set, S* sibl)
{
  uint nElem = set.getElem();
  uint id = sibl->getId();
  for(uint i = 0; i < nElem; ++i)
    if(set[i]->getId() == id)
      return set[i];
  return 0;
}
//----------------------------------------------------------------------------
PD_Alarm* PD_alarmEditorDlg::getAlarm(PD_Assoc* client)
{
  return getSibl(clientAlarm, client);
}
//----------------------------------------------------------------------------
PD_Assoc* PD_alarmEditorDlg::getAssoc(PD_Alarm* client)
{
  return getSibl(clientAssoc, client);
}

//----------------------------------------------------------------------------
bool PD_alarmEditorDlg::isDirty(PD_Base* client)
{
  PD_Assoc* t = dynamic_cast<PD_Assoc*>(client);
  if(t)
    return isDirty(t);
  return isDirty((PD_Alarm*)client);
}
//----------------------------------------------------------------------------
bool PD_alarmEditorDlg::isDirty(PD_Alarm* client)
{
  PD_Assoc* t = getAssoc(client);
  return t ? t->isDirty() : false;
}
//----------------------------------------------------------------------------
bool PD_alarmEditorDlg::isDirty(PD_Assoc* client)
{
  PD_Alarm* t = getAlarm(client);
  return t ? t->isDirty() : false;
}
//----------------------------------------------------------------------------
#define CX_IMAGE 21
#define CY_IMAGE 24
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::makeClient()
{
  uint nElem = clientAlarm.getElem();
  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  for(int i = nElem - 1; i >= 0; --i) {
    TabCtrl_DeleteItem(hTab, i);
    delete clientAlarm.remove(i);
    delete clientAssoc.remove(i);
    }
  currPage = 0;

  convertOldFile();

  PVect<uint> idA;
  for(uint i = 0; i < MAX_PERIPH; ++i)
    checkFile(_T("txt"), i + FIRST_PERIPH, idA);

  nElem = idA.getElem();
  if(!nElem) {
    idA[0] = FIRST_PERIPH;
    ++nElem;
    }
  for(uint i = 0; i < nElem; ++i)
    performAddTab(idA[i], i);

  PRect r;
  GetWindowRect(hTab, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
  for(uint i = 0; i < nElem; ++i) {
    SetWindowPos(*clientAlarm[i], 0, 0, r.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
    SetWindowPos(*clientAssoc[i], 0, 0, r.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
    }
  currPage = 0;
  chgPage(currPage);
  TabCtrl_SetCurSel(hTab, currPage);
}
//----------------------------------------------------------------------------
bool PD_alarmEditorDlg::create()
{
  Attr.style |= WS_CLIPCHILDREN;
  if(!baseClass::create())
    return false;
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));

  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  ImageList = ImageList_Create(CX_IMAGE, CY_IMAGE, ILC_COLOR | ILC_MASK, 1, 0);

  PBitmap bmp(IDB_TABS, getHInstance());
  ImageList_AddMasked(ImageList, bmp, RGB(192, 192, 192));

  TabCtrl_SetImageList(hTab, ImageList);
  makeClient();
  ShowWindow(*clientAlarm[0], SW_SHOWNORMAL);

  return true;
}
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::moveChild(PWin* child)
{
  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  PRect r;
  GetWindowRect(hTab, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
  SetWindowPos(*child, 0, 0, r.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
}
//----------------------------------------------------------------------------
bool PD_alarmEditorDlg::isDirty()
{
  uint nElem = clientAlarm.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(clientAlarm[i]->isDirty())
      return true;
    if(clientAssoc[i]->isDirty())
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool PD_alarmEditorDlg::saveDirtyPage(bool req)
{
  uint nElem = clientAlarm.getElem();
  bool dirty = false;
  for(uint i = 0; i < nElem; ++i) {
    clientAlarm[i]->saveCurrData();
    clientAssoc[i]->saveCurrData();
    if(clientAlarm[i]->isDirty() || clientAssoc[i]->isDirty()) {
      dirty = true;
      break;
      }
    }
  if(dirty) {
    int result = IDYES;
    if(req)
      result = MessageBox(*this, _T("Vuoi salvare i dati corrrenti?"), _T("I dati sono cambiati"),
        MB_YESNOCANCEL);
    switch(result) {
      case IDCANCEL:
        return false;
      case IDYES:
        for(uint i = 0; i < nElem; ++i)
          if(!clientAlarm[i]->saveData())
            return false;
        break;
      }
    }
  return true;
}
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//-----------------------------------------------------------------------------
LRESULT PD_alarmEditorDlg::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_NOTIFY:
      if(((NMHDR*)lParam)->code == TCN_SELCHANGE)
        chgPage(TabCtrl_GetCurSel(((NMHDR*)lParam)->hwndFrom));
      break;

    case WM_MOUSEWHEEL:
      if(jobbingAlarm == onJob)
        PostMessage(*clientAlarm[currPage], WM_CLIENT_WHEEL, wParam, lParam);
      else
        PostMessage(*clientAssoc[currPage], WM_CLIENT_WHEEL, wParam, lParam);
      break;

    case WM_CLIENT_REQ:
      switch(LOWORD(wParam)) {
        case CM_SAVE_ALL:
          saveDirtyPage(false);
          break;
        }
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
          return 0;
        case IDC_CLOSE_ALARM:
          if(!saveOnExit())
            return 0;
          CmCancel();
          break;
        case IDC_BUTTON_TOGGLE_VIS:
          toggleVis();
          break;
        case IDC_BUTTON_ADD_TAB:
          addTab();
          break;
        case IDC_BUTTON_REM_TAB:
          remTab();
          break;
        case IDC_BUTTON_PASTE_FROM_CLIP:
          PostMessage(*clientAlarm[currPage], WM_COMMAND, wParam, lParam);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::performAddTab(uint cid, uint pos)
{
  PD_Alarm* alarm = new PD_Alarm(this, cid);
  clientAlarm.insert(alarm, pos);

  PD_Assoc* assoc = new PD_Assoc(this, cid);
  clientAssoc.insert(assoc, pos);
  // la creazione della finestra di alarm deve avvenire dopo la creazione dell'oggetto
  // assoc collegato, perché nella create() di alarm viene caricato il file per entrambi
  // gli oggetti

  alarm->create();
  moveChild(alarm);
  {
  int old = currPage;
  currPage = pos;
  assoc->create();
  currPage = old;
  }
  moveChild(assoc);


  if(currPage >= (int)pos && currPage < (int)clientAlarm.getElem() - 1)
    ++currPage;

  TC_ITEM ti;
  memset(&ti, 0, sizeof(ti));
  ti.mask = TCIF_TEXT | TCIF_IMAGE;

  TCHAR t[20];

  wsprintf(t, _T("Al-%d"), cid);
  ti.pszText = t;
  ti.cchTextMax = _tcslen(ti.pszText);

  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  TabCtrl_InsertItem(hTab, pos, &ti);
  chgPage(pos);
  TabCtrl_SetCurSel(hTab, currPage);

}
//----------------------------------------------------------------------------
static void removeId(PVect<int>&id, int v)
{
  id.remove(v - FIRST_PERIPH);
}
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::addTab()
{
  int nElem = clientAlarm.getElem();
  PVect<int> id;
  id.setDim(MAX_PERIPH);
  for(int i = 0; i < MAX_PERIPH - FIRST_PERIPH; ++i)
    id[i] = i + FIRST_PERIPH;
  for(int i = nElem - 1; i >= 0; --i) {
    int cid = clientAlarm[i]->getId();
    removeId(id, cid);
    }

  HMENU hmenu = CreatePopupMenu();
  if(!hmenu)
    return;
  int nToAdd = id.getElem();

  for(int i = 0; i < nToAdd; ++i) {
    int cid = id[i];
    TCHAR t[20];
    wsprintf(t, _T("Alarm_%d"), cid);
    AppendMenu(hmenu, MF_STRING, cid, t);
    }
  const uint style = TPM_RETURNCMD | TPM_RIGHTALIGN | TPM_TOPALIGN;
  POINT Pt;
  GetCursorPos(&Pt);
  int result = TrackPopupMenu(hmenu, style, Pt.x, Pt.y, 0, *this, 0);
  DestroyMenu(hmenu);
  if(result) {
    int pos = nElem;
    for(int i = 0; i < nElem; ++i) {
      int cid = clientAlarm[i]->getId();
      if(cid >= result) {
        pos = i;
        break;
        }
      }
    performAddTab(result, pos);
    }
}
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::remTab()
{
  if(clientAlarm.getElem() <= 1) {
    MessageBox(*this, _T("Impossibile eliminare.\r\nDeve esistere almeno una pagina"),
      _T("Attenzione!!"), MB_OK | MB_ICONSTOP);
    return;
    }
  if(IDYES == MessageBox(*this, _T("Eliminare il tab corrente?"), _T("Conferma"), MB_YESNO | MB_ICONINFORMATION)) {

    HWND hTab = GetDlgItem(*this, IDC_TAB1);
    TabCtrl_DeleteItem(hTab, currPage);
    delete clientAlarm.remove(currPage);
    delete clientAssoc.remove(currPage);
    if(clientAlarm.getElem() <= (uint)currPage)
      --currPage;
    chgPage(currPage);
    TabCtrl_SetCurSel(hTab, currPage);
    }
}
//----------------------------------------------------------------------------
#define STD_FLG \
  (SWP_NOMOVE | SWP_NOSIZE)
//----------------------------------------------------------------------------
#define STD_FLG_HIDE \
  (STD_FLG | SWP_HIDEWINDOW | SWP_NOZORDER)
//----------------------------------------------------------------------------
#define STD_FLG_SHOW \
  (STD_FLG | SWP_SHOWWINDOW)
//----------------------------------------------------------------------------
#define SHOW_W(a, p) \
  SetWindowPos((a), (p), 0, 0, 0, 0, STD_FLG_SHOW)
//----------------------------------------------------------------------------
#if 1
#define HIDE_W(a) \
  ShowWindow((a), SW_HIDE)
#else
#define HIDE_W(a) \
  SetWindowPos((a), 0, 0, 0, 0, 0, STD_FLG_HIDE)
#endif
//----------------------------------------------------------------------------
#if 1
#define SHOW_HIDE(c, p) \
  {\
  HIDE_W(p);\
  SHOW_W(c, HWND_TOP);\
  }
#else
#define SHOW_HIDE(c, p) \
  {SHOW_W(c, p);\
  HIDE_W(p);}
#endif
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::toggleVis()
{
  commonInfo ci;
  LockWindowUpdate(*this);

  switch(onJob) {
    case jobbingAlarm:
      clientAlarm[currPage]->getInfo(ci);
      clientAssoc[currPage]->setInfo(ci);
      SHOW_HIDE(*clientAssoc[currPage], *clientAlarm[currPage]);

      onJob = jobbingAssoc;
      SetDlgItemText(*this, IDC_BUTTON_TOGGLE_VIS, _T("> Allarmi"));
      setCaption( _T("Editore Allarmi - Associazioni"));
      ShowWindow(GetDlgItem(*this, IDC_BUTTON_PASTE_FROM_CLIP), SW_HIDE);
      break;
    case jobbingAssoc:
      clientAssoc[currPage]->getInfo(ci);
      clientAlarm[currPage]->setInfo(ci);

      SHOW_HIDE(*clientAlarm[currPage], *clientAssoc[currPage]);
      if(clientAssoc[currPage]->isDirty()) {
        clientAlarm[currPage]->loadData();
        }
      onJob = jobbingAlarm;
      SetDlgItemText(*this, IDC_BUTTON_TOGGLE_VIS, _T("> Associazioni"));
      setCaption( _T("Editore Allarmi - Allarmi"));
      ShowWindow(GetDlgItem(*this, IDC_BUTTON_PASTE_FROM_CLIP), SW_SHOW);
      break;
    }
  LockWindowUpdate(0);

}
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::chgPage(int page)
{
  LockWindowUpdate(*this);
  switch(onJob) {
    case jobbingAlarm:
      chgAlarmPage(page);
      break;
    case jobbingAssoc:
      chgAssocPage(page);
      break;
    }
  LockWindowUpdate(0);
}
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::chgAlarmPage(int page)
{
  if((uint)page < clientAlarm.getElem()) {
    SHOW_HIDE(*clientAlarm[page], *clientAlarm[currPage]);
    currPage = page;
    }
}
//----------------------------------------------------------------------------
void PD_alarmEditorDlg::chgAssocPage(int page)
{
  if((uint)page < clientAssoc.getElem()) {
    SHOW_HIDE(*clientAssoc[page], *clientAssoc[currPage]);
    currPage = page;
    }
}
//----------------------------------------------------------------------------
bool PD_alarmEditorDlg::saveOnExit()
{
  uint nElem = clientAlarm.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!clientAlarm[i]->saveOnExit())
      return false;
  return true;
}
//------------------------------------------------------------------------------
