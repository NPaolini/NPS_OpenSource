//----------- mainDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "mainDlg.h"
#include "fillDlg.h"
#include "verifyDlg.h"
//----------------------------------------------------------------------------
static
bool checkFile(LPCTSTR ext, uint id, PVect<uint>& idA)
{
  TCHAR Filename[_MAX_PATH];
  wsprintf(Filename, _T("prph_%d_data.%s"), id, ext);
  if(P_File::P_exist(Filename)) {
    uint n = idA.getElem();
    idA[n] = id;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PD_Address::PD_Address(PWin* parent, uint resId, HINSTANCE hinstance) :
    PModDialog(parent, resId, hinstance), onJob(jobbingAdr), currPage(0),
    ImageList(0), SvHandle(0), idTimer(0)

{
  checkBasePage();
  setFolderSVisor();
  SvName[0] = 0;
}
//----------------------------------------------------------------------------
PD_Address::~PD_Address()
{
  clientAdr.reset();
  clientsvTest.reset();
  destroy();
  if(ImageList)
    ImageList_Destroy(ImageList);
}
//----------------------------------------------------------------------------
void PD_Address::setSvHandle(HWND svh, LPCTSTR name)
{
  if(-1 == (LONG)svh || !svh) {
    SvName[0] = 0;
    SvHandle = (HWND)-1;
    return;
    }
  HWND parent = svh;
  while(GetParent(parent))
    parent = GetParent(parent);

  SvHandle = GetWindow(svh, GW_CHILD);
  if(name)
    _tcscpy(SvName, name);
  else
    SvName[0] = 0;
}
//----------------------------------------------------------------------------
#define CX_IMAGE 25
#define CY_IMAGE 16
//----------------------------------------------------------------------------
#define MAX_PERIPH 20
//----------------------------------------------------------------------------
void PD_Address::makeClient()
{
  TCHAR tit[] = _T("Gestione file ADDR7 - SVisor - Peripheral");
  TCHAR buff[_MAX_PATH + SIZE_A_c(tit)];
  _tcscpy(buff, tit);
  GetCurrentDirectory(_MAX_PATH, buff + SIZE_A_c(tit) - 1);

  SetWindowText(*this, buff);

  uint nElem = clientAdr.getElem();
  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  for(int i = nElem - 1; i >= 0; --i) {
    TabCtrl_DeleteItem(hTab, i);
    delete clientAdr.remove(i);
    delete clientsvTest.remove(i);
    }
  currPage = 0;

  PVect<uint> idA;
  idA[0] = 1;
  for(uint i = 0; i < MAX_PERIPH; ++i) {
    if(checkFile(_T("ad7"), i + 2, idA))
      continue;
//    if(checkFile(_T("adr"), i + 2, idA))
//      continue;
    if(checkFile(_T("dat"), i + 2, idA))
      continue;
    }
  nElem = idA.getElem();
  for(uint i = 0; i < nElem; ++i)
    performAddTab(idA[i], i);

  PRect r;
  GetWindowRect(hTab, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
  for(uint i = 0; i < nElem; ++i) {
    SetWindowPos(*clientAdr[i], 0, 0, r.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
    SetWindowPos(*clientsvTest[i], 0, 0, r.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
    }
  currPage = 0;
  chgPage(currPage);
  TabCtrl_SetCurSel(hTab, currPage);

}
//----------------------------------------------------------------------------
bool PD_Address::create()
{
  Attr.style |= WS_CLIPCHILDREN;
  if(!PModDialog::create())
    return false;
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));

  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  ImageList = ImageList_Create(CX_IMAGE, CY_IMAGE, ILC_COLOR | ILC_MASK, 2, 0);

  PBitmap bmp(IDB_TABS, getHInstance());
  ImageList_AddMasked(ImageList, bmp, RGB(192, 192, 192));

  TabCtrl_SetImageList(hTab, ImageList);
  makeClient();
  ShowWindow(*clientAdr[0], SW_SHOWNORMAL);

  idTimer = SetTimer(*this, 1357, 5 * 1000, 0);
  return true;
}
//----------------------------------------------------------------------------
void PD_Address::moveChild(PWin* child)
{
  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  PRect r;
  GetWindowRect(hTab, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);
  SetWindowPos(*child, 0, 0, r.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
}
//----------------------------------------------------------------------------
void PD_Address::fillBit(DWORD& flag)
{
  flag = 0;
  uint nElem = clientAdr.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(i != currPage)
      flag |= 1 << (clientAdr[i]->getId() - 1);
}
//----------------------------------------------------------------------------
baseRow* PD_Address::getRows(uint id)
{
  uint nElem = clientAdr.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(id == clientAdr[i]->getId())
      return clientAdr[i]->getRows();
  return 0;
}
//----------------------------------------------------------------------------
void PD_Address::setDirty(uint id, bool reload)
{
  uint nElem = clientAdr.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(id == clientAdr[i]->getId()) {
      clientAdr[i]->set_Dirty(reload);
      break;
      }
}
//----------------------------------------------------------------------------
HWND PD_Address::getTestHandle()
{
  return *clientsvTest[currPage];
}
//----------------------------------------------------------------------------
HWND PD_Address::getAdrHandle()
{
  return *clientAdr[currPage];
}
//----------------------------------------------------------------------------
bool PD_Address::isDirty()
{
  uint nElem = clientAdr.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(clientAdr[i]->isDirty())
      return true;
  return false;
}
//----------------------------------------------------------------------------
bool PD_Address::saveDirtyPage(bool req)
{
  uint nElem = clientAdr.getElem();
  bool dirty = false;
  for(uint i = 0; i < nElem; ++i)
    if(clientAdr[i]->isDirty()) {
      dirty = true;
      break;
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
          if(!clientAdr[i]->saveData())
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
LRESULT PD_Address::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_NOTIFY:
      if(((NMHDR*)lParam)->code == TCN_SELCHANGE)
        chgPage(TabCtrl_GetCurSel(((NMHDR*)lParam)->hwndFrom));
      break;

    case WM_MOUSEWHEEL:
      if(jobbingAdr == onJob)
        PostMessage(*clientAdr[currPage], WM_CLIENT_WHEEL, wParam, lParam);
      else
        PostMessage(*clientsvTest[currPage], WM_CLIENT_WHEEL, wParam, lParam);
      break;

    case WM_CLIENT_REQ:
      switch(LOWORD(wParam)) {
        case CM_SAVE_ALL:
          saveDirtyPage(false);
          break;

        case CM_NEW_PATH:
          if(!saveDirtyPage(true))
            break;
          if(chooseFolderSVisor())
            makeClient();
          break;
        }
      break;

    case WM_DESTROY:
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      break;
    case WM_ACTIVATEAPP:
      if(wParam)
        checkExternalChange();
      break;
    case WM_TIMER:
      if(idTimer == wParam)
        checkExternalChange();
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          return 0;
        case ID_EXIT_APP:
          if(!saveOnExit())
            return 0;
          CmCancel();
          break;
        case IDCANCEL:
          return 0;
        case IDC_BUTTON_TOGGLE_VIS:
          toggleVis();
          break;
        case IDC_BUTTON_ADD_TAB:
          addTab();
          break;
        case IDC_BUTTON_REM_TAB:
          remTab();
          break;
        }
      break;
    }
  return PModDialog::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_Address::checkExternalChange()
{
  uint nElem = clientAdr.getElem();
  for(uint i = 0; i < nElem; ++i)
    clientAdr[i]->checkExternalChange();
}
//----------------------------------------------------------------------------
void PD_Address::performAddTab(uint cid, uint pos)
{
  PD_Adr* adr = new PD_Adr(this, cid);
  clientAdr.insert(adr, pos);
  adr->create();
  moveChild(adr);

  PD_Test* test = new PD_Test(this, cid);
  clientsvTest.insert(test, pos);
  {
  // nella create carica i dati da adr che viene abbinata tramite la currPage
  // allora forza la currPage per il tempo necessario
  int old = currPage;
  currPage = pos;
  test->create();
  currPage = old;
  }
  moveChild(test);


  if(currPage >= (int)pos && currPage < (int)clientAdr.getElem() - 1)
    ++currPage;

  TC_ITEM ti;
  memset(&ti, 0, sizeof(ti));
  ti.mask = TCIF_TEXT | TCIF_IMAGE;

  TCHAR t[20];

  if(1 == cid) {
    ti.pszText = _T("Mem");
    ti.iImage = 0;
    }
  else if(cid <= 9) {
    ti.iImage = 1;
    wsprintf(t, _T("P-%d"), cid);
    ti.pszText = t;
    }
  else {
    ti.iImage = 0;
    wsprintf(t, _T("M-%d"), cid);
    ti.pszText = t;
    }
  ti.cchTextMax = _tcslen(ti.pszText);

  HWND hTab = GetDlgItem(*this, IDC_TAB1);
  TabCtrl_InsertItem(hTab, pos, &ti);
  chgPage(pos);
  TabCtrl_SetCurSel(hTab, currPage);

}
//----------------------------------------------------------------------------
void PD_Address::addTab()
{
  int nElem = clientAdr.getElem();
  PVect<int> id;
  id.setDim(20);
  for(int i = 0; i < 20; ++i)
    id[i] = i + 1;
  for(int i = nElem - 1; i >= 0; --i) {
    int cid = clientAdr[i]->getId() - 1;
    id.remove(cid);
    }

  HMENU hmenu = CreatePopupMenu();
  if(!hmenu)
    return;
  int nToAdd = id.getElem();

  bool extMem = false;
  for(int i = 0; i < nToAdd; ++i) {
    int cid = id[i];
    if(1 == cid) {
      AppendMenu(hmenu, MF_STRING, 1, _T("Mem"));
      AppendMenu(hmenu, MF_SEPARATOR, 0, 0);
      }
    else if(cid <= 9) {
      TCHAR t[20];
      wsprintf(t, _T("Prph_%d"), cid);
      AppendMenu(hmenu, MF_STRING, cid, t);
      }
    else {
      if(!extMem) {
        extMem = true;
        AppendMenu(hmenu, MF_SEPARATOR, 0, 0);
        }
      TCHAR t[20];
      wsprintf(t, _T("extMem_%d"), cid);
      AppendMenu(hmenu, MF_STRING, cid, t);
      }
    }
  const uint style = TPM_RETURNCMD | TPM_RIGHTALIGN | TPM_TOPALIGN;
  POINT Pt;
  GetCursorPos(&Pt);
  int result = TrackPopupMenu(hmenu, style, Pt.x, Pt.y, 0, *this, 0);
  DestroyMenu(hmenu);
  if(result) {
    int pos = nElem;
    for(int i = 0; i < nElem; ++i) {
      int cid = clientAdr[i]->getId();
      if(cid >= result) {
        pos = i;
        break;
        }
      }
    performAddTab(result, pos);
    }
}
//----------------------------------------------------------------------------
void PD_Address::remTab()
{
  if(clientAdr.getElem() <= 1) {
    MessageBox(*this, _T("Impossibile eliminare.\r\nDeve esistere almeno una pagina"),
      _T("Attenzione!!"), MB_OK | MB_ICONSTOP);
    return;
    }
  if(IDYES == MessageBox(*this, _T("Eliminare il tab corrente?"), _T("Conferma"), MB_YESNO | MB_ICONINFORMATION)) {
    bool alsoFile = IDYES == MessageBox(*this, _T("Eliminare anche il file abbinato?"), _T("Conferma"), MB_YESNO | MB_ICONINFORMATION);

    HWND hTab = GetDlgItem(*this, IDC_TAB1);
    TabCtrl_DeleteItem(hTab, currPage);
    if(alsoFile) {
      LPCTSTR file = clientAdr[currPage]->getFilename();
      if(*file) {
        DeleteFile(file);
        TCHAR datFile[_MAX_PATH];
        _tcscpy_s(datFile, file);
        P_File::chgExt(datFile, _T(".dat"));
        DeleteFile(datFile);
        }
      }
    delete clientAdr.remove(currPage);
    delete clientsvTest.remove(currPage);
    if(clientAdr.getElem() <= (uint)currPage)
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
void PD_Address::toggleVis()
{
  commonInfo ci;
  LockWindowUpdate(*this);

  switch(onJob) {
    case jobbingAdr:
      clientAdr[currPage]->getInfo(ci);
      clientsvTest[currPage]->setInfo(ci);
      SHOW_HIDE(*clientsvTest[currPage], *clientAdr[currPage]);

      onJob = jobbingSvTest;
      SetDlgItemText(*this, IDC_BUTTON_TOGGLE_VIS, _T("> ADDR"));
      break;
    case jobbingSvTest:
      clientsvTest[currPage]->getInfo(ci);
      clientAdr[currPage]->setInfo(ci);

      SHOW_HIDE(*clientAdr[currPage], *clientsvTest[currPage]);

      onJob = jobbingAdr;
      SetDlgItemText(*this, IDC_BUTTON_TOGGLE_VIS, _T("> sVisor Test"));
      break;
    }
  LockWindowUpdate(0);

}
//----------------------------------------------------------------------------
void PD_Address::chgPage(int page)
{
  LockWindowUpdate(*this);
  switch(onJob) {
    case jobbingAdr:
      chgAdrPage(page);
      break;
    case jobbingSvTest:
      chgTestPage(page);
      break;
    }
  clientAdr[currPage]->setTitle();
  LockWindowUpdate(0);
}
//----------------------------------------------------------------------------
void PD_Address::chgAdrPage(int page)
{
  if((uint)page < clientAdr.getElem()) {
    SHOW_HIDE(*clientAdr[page], *clientAdr[currPage]);
    currPage = page;
    }
}
//----------------------------------------------------------------------------
void PD_Address::chgTestPage(int page)
{
  if((uint)page < clientsvTest.getElem()) {
    SHOW_HIDE(*clientsvTest[page], *clientsvTest[currPage]);
    currPage = page;
    }
}
//----------------------------------------------------------------------------
bool PD_Address::saveOnExit()
{
  uint nElem = clientAdr.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(clientAdr[i]->isDirty())
      if(!clientAdr[i]->saveOnExit())
        return false;
  return true;
}
//------------------------------------------------------------------------------
