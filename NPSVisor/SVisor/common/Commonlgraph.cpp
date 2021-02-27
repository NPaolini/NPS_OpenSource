//----- Commonlgraph.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "1.h"
#include "id_msg_common.h"
#include "Commonlgraph.h"
#include <stdlib.h>
#include <stdio.h>
#include "P_File.h"
//----------------------------------------------------------------------------
enum findRec { frBefore = -1, frFound, frAfter };
//--------------------------------------------------------------------------
findRec cmpDate(P_File& f, DWORD pos, const FILETIME& match, int lenRec, LPBYTE buff, int sizeHeader, int offset)
{
  f.P_seek(pos * lenRec + sizeHeader + offset);
  f.P_read(buff, lenRec);

#if 1
  FILETIME ft = *reinterpret_cast<FILETIME*>(buff);

  return match < ft ? frBefore : match > ft ? frAfter : frFound;
#else
  SYSTEMTIME st;
  FileTimeToSystemTime(reinterpret_cast<FILETIME*>(buff), &st);
  if(match.wYear < st.wYear)
    return frBefore;
  if(match.wYear > st.wYear)
    return frAfter;

  if(match.wMonth < st.wMonth)
    return frBefore;
  if(match.wMonth > st.wMonth)
    return frAfter;

  return match.wDay < st.wDay ? frBefore :
         match.wDay > st.wDay ? frAfter : frFound;
#endif
}
//--------------------------------------------------------------------------
DWORD findFirstPos(P_File& pFile, const FILETIME& match, int lenRec, LPBYTE buff, int sizeHeader, bool prev, int offset)
{
  DWORD records = (DWORD)(pFile.get_len() - sizeHeader) / lenRec;
  findRec cmp = frBefore;
  long upp = records - 1;
  long mid = 0;
  long low = 0;
  while(low <= upp) {
    mid = DWORD(upp + low) >> 1;
    cmp = cmpDate(pFile, mid, match, lenRec, buff, sizeHeader, offset);
    if(cmp < 0)
      upp = mid - 1;
    else if(cmp > 0)
      low = ++mid;
    else
      break;
    }
  if(prev) {
    if(cmp <= 0) {
      while(mid > 0) {
        mid--;
        cmp = cmpDate(pFile, mid, match, lenRec, buff, sizeHeader, offset);
        if(cmp > 0)
          break;
        }
      }
    }
  else {
    if(cmp >= 0) {
      while(mid < (long)(records - 1)) {
        ++mid;
        cmp = cmpDate(pFile, mid, match, lenRec, buff, sizeHeader, offset);
        if(cmp < 0)
          break;
        }
      }
    }

  pFile.P_seek(mid * lenRec + sizeHeader);
  return(mid);
}
//----------------------------------------------------------------------------
bool getNameTrendById(LPTSTR name, uint sz, uint id)
{
  if(!id)
    return false;
  LPCTSTR t = getString(id);
  if(!t)
    return false;
  t = findNextParam(t, OFFS_TRENDNAME);
  _tcscpy_s(name, sz, t);
  return true;
}
//----------------------------------------------------------------------------
uint getIdByNameTrend(LPCTSTR name)
{
  if(!name)
    return 0;
  LPCTSTR p = getString(INIT_TREND_DATA);
  if(!p)
    return 0;
  do {
    uint id = _ttoi(p);
    LPCTSTR t = getString(id);
    if(t) {
      t = findNextParam(t, OFFS_TRENDNAME);
      if(t)
        if(!_tcsicmp(name, t))
          return id;
      }
    p = findNextParam(p, 1);
    } while(p);
  return 0;
}
//----------------------------------------------------------------------------
uint getInitDataByNameTrend(LPCTSTR name, uint& nElem)
{
  uint id = getIdByNameTrend(name);
  while(id) {
    LPCTSTR p = getString(id);
    if(!p)
      break;
    p = findNextParam(p, 1);
    if(!p)
      break;
    uint id2 = _ttoi(p);
    p = getString(id2);
    if(!p)
      break;
    nElem = _ttoi(p);

    p = findNextParam(p, 1);
    if(!p)
      break;
    return _ttoi(p);
    }
  return 0;

}
//----------------------------------------------------------------------------
#define MIN_INIT_STD_MSG_TEXT 800000
//----------------------------------------------------------------------------
LPCTSTR checkNewMode(LPCTSTR p)
{
  if(!p)
    return 0;
  uint len = _tcslen(p);
  for(uint i = 0; i < len; ++i) {
    if(_T(',') == p[i])
      break;
    if(!_istdigit(p[i]))
      return p;
    }
  DWORD id = _ttoi(p);
  if(id >= MIN_INIT_STD_MSG_TEXT) {
    LPCTSTR p2 = getString(id);
    if(p2)
      return p2;
    }
  return p;
}
//----------------------------------------------------------------------------
LPCTSTR checkNewMode(const setOfString& set, LPCTSTR p)
{
  if(!p)
    return 0;
  uint len = _tcslen(p);
  for(uint i = 0; i < len; ++i)
    if(!_istdigit(p[i]))
      return p;
  DWORD id = _ttoi(p);
  if(id >= MIN_INIT_STD_MSG_TEXT) {
    LPCTSTR p2 = set.getString(id);
    if(p2)
      return p2;
    }
  return p;
}
//--------------------------------------------------------------------------
LPCTSTR getDescrDataTrend(uint id, bool& needDelete)
{
  LPCTSTR p = _T(" ");
  if(id) {
    LPCTSTR pt = getString(id);
    pt = findNextParam(pt, 4);
    LPCTSTR p2 = findNextParam(pt, 1);
    p2 = checkNewMode(p2);
    if(!p2)
      p = pt;
    else
      p = p2;
    }
  return getStringByLangSimple(p, needDelete);
}
//--------------------------------------------------------------------------
smartPointerConstString getDescrDataTrend(uint id)
{
  bool needDelete;
  LPCTSTR p = getDescrDataTrend(id, needDelete);
  return smartPointerConstString(p, needDelete);
}
//--------------------------------------------------------------------
TD_Timer::TD_Timer(PWin* parent, LPCTSTR caption, LPCTSTR prompt, uint &time, uint resId)
:
    baseClass(parent, resId), Time(time), Capt(caption), Prompt(prompt)
{
  _Time = new svEdit(this, IDC_EDIT_TIME_4_TIMER, 12);
  txtTime = new PStatic(this, IDC_STATICTEXT_TIME_4_TIMER);
}
//--------------------------------------------------------------------
bool TD_Timer::create()
{
  if(!baseClass::create())
    return false;

  TCHAR t[40];
  _itot_s(Time, t, SIZE_A(t), 10);
  _Time->setText(t);

  setCaption(Capt);
  txtTime->setText(Prompt);
  return true;
}
//--------------------------------------------------------------------
void TD_Timer::CmOk()
{
  // INSERT>> Your code here.
  TCHAR t[40];
  SendMessage(*_Time, WM_GETTEXT, (WPARAM)SIZE_A(t), (LPARAM)t);
  Time = _ttoi(t);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void startSimulDialog(PWin& diag, PWin* owner)
{
  HWND oldFocus = GetFocus();
  PWin* pw = owner;
  PVect<PWin*> toDisable;
  int i = 0;
  do {
    toDisable[i] = pw;
    ++i;
    EnableWindow(*pw, false);
    if(dynamic_cast<mainClient*>(pw))
      break;
    pw = pw->getParent();
    } while(pw);

  if(diag.create()) {
    // finché non viene chiusa la window
    while(diag.getHandle()) {
      // aspetta l'arrivo di un messaggio
      ::WaitMessage();
      // lo fa elaborare dall'applicazione principale che provvede al dispatch
      getAppl()->pumpMessages();
      }
    }
  // riabilita
  for(int j = i - 1; j >= 0; --j)
    EnableWindow(*toDisable[j], true);
  SetForegroundWindow(*owner);
  SetFocus(oldFocus);
}
//------------------------------------------------------------------------
