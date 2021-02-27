//------ systray.cpp ---------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "systray.h"
#include "mainapp.h"
#include "mainClient.h"
//----------------------------------------------------------------------------
tipsObj::infoActive tipsObj::needAdd(mainClient* mc, bool force)
{
  if(!prph)
    return none;
  genericPerif* pPrph = mc->getGenPerif(prph);
  if(!pPrph)
    return none;
  prfData data;
  data.typeVar = type;
  data.lAddr = addr;
  if(prfData::tBitData == type)
    data.U.dw = MAKELONG(nbit, offs);

  prfData::tResultData result = pPrph->get(data);
  uint v = data.U.dw;
  if(prfData::tBitData != type && nbit) {
    v >>= offs;
    v &= (1 << nbit) - 1;
    }
  if(force) {
    oldVal = v;
    return v ? changedActive : changedRemoved;
    }
  bool changed = v != oldVal;
  oldVal = v;
  if(changed)
    return v ? changedActive : changedRemoved;
  return v ? noChangedActive : none;
}
//-------------------------------------------------------------------
LPCTSTR unSet(LPCTSTR buff, uint& v)
{
  if(!buff)
    return 0;
  v = _ttoi(buff);
  return findNextParamTrim(buff);
}
//----------------------------------------------------------------------------
void tipsObj::load(LPCTSTR row)
{
  if(!row)
    return;
  LPCTSTR p = row;
  p = unSet(p, prph);
  p = unSet(p, addr);
  p = unSet(p, type);
  p = unSet(p, nbit);
  p = unSet(p, offs);
  p = unSet(p, Level);
  if(p) {
    uint id = _ttoi(p);
    smartPointerConstString text = getStringByLangGlob(id);
    if(text)
      copyStrZ(Text, &text, SIZE_A(Text) - 1);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
manTipsObj::manTipsObj(mainClient* owner) : Owner(owner)
{
  ZeroMemory(normalString, sizeof(normalString));

  LPCTSTR p = getString(INIT_TRAY_DATA);
  while(p)  {
    uint nElem = _ttoi(p);
    if(!nElem)
      break;
    p = findNextParamTrim(p);
    if(!p)
      break;
    uint id = _ttoi(p);
    smartPointerConstString text = getStringByLangGlob(id);
    if(text)
      copyStrZ(normalString, &text, SIZE_A(normalString) - 1);
    p = findNextParamTrim(p);
    if(!p)
      break;
    id = _ttoi(p);
    text = getStringByLangGlob(id);
    if(text)
      copyStrZ(title, &text, SIZE_A(title) - 1);
    tObj.setDim(nElem);
    for(uint i = 0; i < nElem; ++i) {
      p = getString(INIT_TRAY_DATA + 1 + i);
      if(!p)
        continue;
      tObj[i] = tipsObj(p);
      }
    break;
    }
}
//----------------------------------------------------------------------------
bool manTipsObj::checkChanged(bool force)
{
  uint nElem = tObj.getElem();
  for(uint i = 0; i < nElem; ++i) {
    tipsObj::infoActive iact = tObj[i].needAdd(Owner, force);
    if(iact > tipsObj::noChangedActive)
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
int manTipsObj::makeString(LPTSTR text, uint dim, bool force)
{
  text[0] = 0;
  uint nElem = tObj.getElem();
  PVect<tipsObj::infoActive> iact;
  int level = 0;
  for(uint i = 0; i < nElem; ++i) {
    iact[i] = tObj[i].needAdd(Owner, force);
    if(iact[i] > tipsObj::changedRemoved)
      if(level < tObj[i].getLevel())
        level = tObj[i].getLevel();
    }
  // nessun allarme
  if(!level) {
    copyStrZ(text, normalString, dim - 1);
    return level;
    }
  int len = dim;
  for(uint i = 0; i < nElem && len > 0; ++i) {
    if(iact[i] > tipsObj::changedRemoved) {
      int t = _tcslen(tObj[i].getText());
      if(t >= len - 3)
        return level;
      copyStrZ(text, tObj[i].getText(), t);
      text += t;
      copyStrZ(text, _T("\r\n"), 3);
      text += 2;
      len -= t;
      }
    }
  for(uint i = 0; i < dim; ++i, --text) {
    if(*text > _T(' ')) {
      text[1] = 0;
      break;
      }
    }
  return level;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_LEVEL_TRAY 4
//----------------------------------------------------------------------------
#define IDI_ICON_SYSTRAY_4 IDI_ICON_SYSTRAY_3
//----------------------------------------------------------------------------
sysTray::sysTray(mainWin* owner, manTipsObj& m_obj) : Owner(owner), mObj(m_obj)
{
  gotoSysTray(1);
}
//----------------------------------------------------------------------------
static void ShowContextMenu(HWND hWnd)
{
  POINT pt;
  GetCursorPos(&pt);
  HMENU hMenu = CreatePopupMenu();
  if(hMenu)  {
    InsertMenu(hMenu, -1, MF_BYPOSITION, WM_REMOVE_FROM_TRAY, _T("Restore"));
    SetForegroundWindow(hWnd);

    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL );

    DestroyMenu(hMenu);
  }
}
//----------------------------------------------------------------------------
static const UINT wmTaskbarRestart = RegisterWindowMessage(_T("TaskbarCreated"));
//----------------------------------------------------------------------------
bool sysTray::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
  switch(message) {
    case SWM_TRAYMSG:
      switch(lParam) {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_CONTEXTMENU:
          ShowContextMenu(hwnd);
          break;
        case NIN_BALLOONTIMEOUT:
          setTips();
          break;
        case NIN_BALLOONUSERCLICK:
          PostMessage(*Owner, WM_COMMAND, MAKEWPARAM(WM_REMOVE_FROM_TRAY, 0), 0);
          break;
        }
      break;
    case WM_TIMER:
      if(ID_TIMER_CHECK_TRAY == wParam)
        check();
      break;
    default:
      if(wmTaskbarRestart == message)
        gotoSysTray(-1);
      break;

    }
  return false;
}
//----------------------------------------------------------------------------
void sysTray::check()
{
  bool changed = mObj.checkChanged(false);
  if(changed)
    setBaloon();
}
//----------------------------------------------------------------------------
void sysTray::setBaloon()
{
  uint idI;
  uint ico;
  TCHAR text[256];
  int level = mObj.makeString(text, SIZE_A(text) - 1, true);

  switch(level) {
    case 0:
      ico = NIIF_INFO;
      idI = IDI_ICON_SYSTRAY_1;
      break;
    case 1:
      ico = NIIF_WARNING;
      idI = IDI_ICON_SYSTRAY_2;
      break;
    case 2:
    case 3:
    default:
      ico = NIIF_ERROR;
      idI = IDI_ICON_SYSTRAY_3;
      break;
    }

  niData.dwInfoFlags = ico;
  _tcscpy_s(niData.szInfoTitle, mObj.getTitle());
  _tcscpy_s(niData.szInfo, text);
  niData.uFlags = NIF_ICON | NIF_INFO;

  niData.hIcon = (HICON)LoadImage(Owner->getHInstance(), MAKEINTRESOURCE(idI),
                IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
                GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);

  Shell_NotifyIcon(NIM_MODIFY, &niData);

  if(niData.hIcon && DestroyIcon(niData.hIcon))
    niData.hIcon = NULL;
}
//----------------------------------------------------------------------------
void sysTray::setTips()
{
  uint idI;
  TCHAR text[256];
  _tcscpy_s(text, mObj.getTitle());
  _tcscat_s(text, _T("\r\n"));
  int len = _tcslen(text);
  int level = mObj.makeString(text + len, SIZE_A(text) - 1 - len, true);
  switch(level) {
    case 0:
      idI = IDI_ICON_SYSTRAY_1;
      break;
    case 1:
      idI = IDI_ICON_SYSTRAY_2;
      break;
    case 2:
    case 3:
    default:
      idI = IDI_ICON_SYSTRAY_3;
      break;
    }

  _tcscpy_s(niData.szTip, text);

  niData.uFlags = NIF_ICON | NIF_TIP;
  niData.hIcon = (HICON)LoadImage(Owner->getHInstance(), MAKEINTRESOURCE(idI),
                IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
                GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);

  Shell_NotifyIcon(NIM_MODIFY, &niData);

  if(niData.hIcon && DestroyIcon(niData.hIcon))
    niData.hIcon = NULL;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void sysTray::gotoSysTray(int level)
{
  if(-1 != level) {
    SendMessage(*Owner->client, WM_SHOW_HIDE_PRPH, 0, 0);
    ShowWindow(*Owner, SW_HIDE);
    }
  ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

  niData.cbSize = sizeof(NOTIFYICONDATA);
  niData.uID = TRAYICONID;
  niData.uFlags = NIF_MESSAGE;
  niData.hWnd = *Owner;
  niData.uCallbackMessage = SWM_TRAYMSG;

  // Add icon to tray
  Shell_NotifyIcon(NIM_ADD, &niData);

  niData.uFlags = NIF_INFO;
  niData.uVersion = NOTIFYICON_VERSION;
  Shell_NotifyIcon(NIM_SETVERSION, &niData);
#if 0
  if(-1 != level)
    setBaloon();
  else
#endif
    setTips();
}
//----------------------------------------------------------------------------
void sysTray::removeSysTray()
{
  if(niData.uFlags) {
    niData.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &niData);
    }
  SendMessage(*Owner->client, WM_SHOW_HIDE_PRPH, 1, 0);
  ShowWindow(*Owner, SW_SHOWNORMAL);
}
//----------------------------------------------------------------------------
