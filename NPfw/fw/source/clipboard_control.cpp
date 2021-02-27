//--------------- clipboard_control.cpp ---------------------------
//-----------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------
#include "p_util.h"
#include "clipboard_control.h"
//-----------------------------------------------------------------
#ifdef UNICODE
  #define myCF_TEXT CF_UNICODETEXT
#else
  #define myCF_TEXT CF_TEXT
#endif
//-------------------------------------------------------------------
void Clipboard_Control::copyFromControl(infoClipControl& iLb)
{
  int count = (int)getCount(iLb);
  if(!count)
    return;
  if(!OpenClipboard(*getOwner()))
    return;
  EmptyClipboard();

  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (iLb.szBuff * count) * sizeof(TCHAR));
  if(hglbCopy) {
    LPTSTR b = (LPTSTR)GlobalLock(hglbCopy);
    LPTSTR pb = b;
    for(int i = 0; i < count; ++i) {
      getLine(iLb, i);
      pb = prepareForClipboard(pb, iLb);
      }
    GlobalUnlock(hglbCopy);
    SetClipboardData(myCF_TEXT, hglbCopy);
    }
  CloseClipboard();
}
//-------------------------------------------------------------------
void Clipboard_Control::pasteToControl(infoClipControl& iLb)
{
  if(!IsClipboardFormatAvailable(myCF_TEXT))
    return;
  if(!OpenClipboard(*getOwner()))
    return;
  HGLOBAL hglb = GetClipboardData(myCF_TEXT);
  uint ix = 0;
  while(hglb) {
    LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
    if(!lptstr)
      break;
    fillRowByClipboard(lptstr, iLb);
    GlobalUnlock(hglb);
    break;
    }
  CloseClipboard();
}
//-------------------------------------------------------------------
void Clipboard_Control::fillRowByClipboard(LPCTSTR clip, infoClipControl& iLb)
{
  static uint Choose = 0;
  if(!getChoosePaste(Choose))
    return;
  int count = (int)getCount(iLb);
  int sel =  (int)getSel(iLb);
  int toRemove = 0;
  switch(Choose) {
    case eap_replaceAll:
      reset(iLb);
    case eap_append:
      sel = -1;
      break;
    case eap_insert:
      if(sel >= 0)
        break;
    case eap_replase:
      if(sel < 0) {
        MessageBox(*getOwner(), _T("Selezionare una riga della lista"), _T("Attenzione!"), MB_OK | MB_ICONINFORMATION);
        return;
        }
      toRemove = count - sel;
      break;
    }
  LPCTSTR p = clip;
  while(p) {
    p = makeForControl(p, iLb);
    if(toRemove) {
      deleteLine(iLb, sel);
      --toRemove;
      }
    addLine(iLb, sel);
    if(sel >= 0)
      ++sel;
    }
}
//-----------------------------------------------------------------
LRESULT Clipboard_ListBox::getCount(infoClipControl& iLb) {  return SendMessage(iLb.hwnd, LB_GETCOUNT, 0, 0); }
//-----------------------------------------------------------------
void Clipboard_ListBox::getLine(infoClipControl& iLb, int line) {  SendMessage(iLb.hwnd, LB_GETTEXT, line, (LPARAM)iLb.buff); }
//-----------------------------------------------------------------
LRESULT Clipboard_ListBox::getSel(infoClipControl& iLb) {  return SendMessage(iLb.hwnd, LB_GETCURSEL, 0, 0); }
//-----------------------------------------------------------------
void Clipboard_ListBox::reset(infoClipControl& iLb) {  SendMessage(iLb.hwnd, LB_RESETCONTENT, 0, 0); }
//-----------------------------------------------------------------
void Clipboard_ListBox::deleteLine(infoClipControl& iLb, int line) {  SendMessage(iLb.hwnd, LB_DELETESTRING, line, 0); }
//-----------------------------------------------------------------
void Clipboard_ListBox::addLine(infoClipControl& iLb, int line) {  SendMessage(iLb.hwnd, LB_INSERTSTRING, line, (LPARAM)iLb.buff); }
//-----------------------------------------------------------------
LPTSTR Clipboard_ListBox::prepareForClipboard(LPTSTR target, infoClipControl& iLb)
{
  LPTSTR p = target;
  LPCTSTR p2 = iLb.buff;

  for(uint i = 0; i < iLb.itemTot; ++i) {
    copyStrZ(p, p2, iLb.itemSize[i]);
    p[iLb.itemSize[i]] = 0;
    lTrim(trim(p));
    p += _tcslen(p);
    *p++ = _T('\t');
    *p = 0;
    p2 += iLb.itemSize[i] + 1;
    }
  --p;
  *p = 0;
  wsprintf(p, _T("\r\n"));
  p += _tcslen(p);
  return p;
}
//-----------------------------------------------------------------
LPCTSTR Clipboard_ListBox::makeForControl(LPCTSTR p, infoClipControl& iLb)
{
  fillStr(iLb.buff, _T(' '), iLb.szBuff);
  LPTSTR p2 = iLb.buff;

  for(uint i = 0; i < iLb.itemTot; ++i) {
    for(uint j = 0; j < iLb.itemSize[i]; ++j, ++p) {
      if(*p == _T('\t')) {
        ++p;
        break;
        }
      if(!*p || *p == _T('\r') || *p == _T('\n')) {
        while(*p && *p < _T(' '))
          ++p;
        iLb.buff[iLb.itemSize[iLb.itemTot - 1]] = 0;
        return *p ? p : 0;
        }
      p2[j] = *p;
      }
    p2[iLb.itemSize[i]] = _T('\t');
    p2 += iLb.itemSize[i] + 1;
    }
  iLb.buff[iLb.itemSize[iLb.itemTot - 1]] = 0;

  // se siamo arrivati qui vuol dire che ci sono più colonne di quelle richieste

  // cerchiamo il fine riga
  while(*p && *p != _T('\r') && *p != _T('\n'))
    ++p;
  if(!*p)
    return 0;
  // e poi lo saltiamo
  while(*p && (*p == _T('\r') || *p == _T('\n')))
    ++p;

  return *p ? p : 0;
}
//-------------------------------------------------------------------
