//--------- language.cpp -----------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "language.h"
#include "config.h"
#include "set_cfg.h"
#include "p_param_v.h"
#include "p_name.h"
#include "prfdata.h"
#include "p_body.h"
//----------------------------------------------------------------------------
//static setOfString BodyCache;
static setOfString GlobalCache;
//----------------------------------------------------------------------------
static LPCTSTR addToCache(uint id, LPCTSTR p, bool& needDelete, setOfString& cache)
{
  if(needDelete) {
    needDelete = false;
    cache.replaceString(id, (LPTSTR)p, true, true);
    }
  return p;
}
//----------------------------------------------------------------------------
inline
LPCTSTR getFromGlobCache(uint id)
{
  return GlobalCache.getString(id);
}
//----------------------------------------------------------------------------
#if 1
#define getFromBodyCache(id) BodyCache.getString(id)
#else
inline
LPCTSTR getFromBodyCache(uint id)
{
  return BodyCache.getString(id);
}
#endif
//----------------------------------------------------------------------------
LPCTSTR getStringByglobalPage(LPCTSTR p, bool& needDelete)
{
  int v = _ttoi(p + 1);
  if(!v)
    return p;
  LPCTSTR p2 = getGlobalPageString(v);
  if(!p2)
    return p;
  return getStringByLangSimple(p2, needDelete);
}
//----------------------------------------------------------------------------
LPCTSTR getStringByLangSimple(LPCTSTR p, bool& needDelete)
{
  needDelete = false;
  if(!p)
    return 0;

  const TCHAR match1 = _T('|');
  if(match1 == *p)
    return getStringByglobalPage(p, needDelete);

  const TCHAR match2 = _T('§');
  if(match2 != *p)
    return p;

  uint currLang = getCurrLang();
  if(currLang)
    --currLang;

  uint len = extractParam(0, 0, p + 1, currLang, *p);
  while(len <= 1) {
    // se non esiste, torna il primo
    currLang = 0;
    len = extractParam(0, 0, p + 1, currLang, *p);
    if(len <= 1)
      return p;

    break;
    }
  needDelete = true;
  LPTSTR result = new TCHAR[ len + 1];
  len = extractParam(result, len + 1, p + 1, currLang, *p);
  return result;
}
//----------------------------------------------------------------------------
LPCTSTR getStringByLangGlob(uint id, bool& needDelete)
{
  LPCTSTR p = getFromGlobCache(id);
  if(p) {
    needDelete = false;
    return p;
    }
  p = ::getString(id);
  p = getStringByLangSimple(p, needDelete);
  return addToCache(id, p, needDelete, GlobalCache);
}
//----------------------------------------------------------------------------
LPCTSTR getStringOrIdByLangGlob(uint id, bool& needDelete)
{
  LPCTSTR p = getFromGlobCache(id);
  if(p) {
    needDelete = false;
    return p;
    }
  p = getStringByLangSimple(::getString(id), needDelete);
  if(!p) {
    p = str_newdup(getStringOrId(id));
    needDelete = true;
    return addToCache(id, p, needDelete, GlobalCache);
    }
  return addToCache(id, p, needDelete, GlobalCache);
}
//----------------------------------------------------------------------------
void setWindowTextByLangGlob(HWND hwnd, uint id, bool forceId)
{
  bool needDelete;
  LPCTSTR p = 0;
  if(forceId)
    p = getStringOrIdByLangGlob(id, needDelete);
  else
    p = getStringByLangGlob(id, needDelete);
  smartPointerConstString sPS(p, needDelete);
  if(p || forceId)
    SetWindowText(hwnd, &sPS);
}
//----------------------------------------------------------------------------
uint msgBoxByLangGlob(PWin* win, uint idMsg, uint idTitle, uint flag)
{
  bool needDelete;
  LPCTSTR title = getStringOrIdByLangGlob(idTitle, needDelete);
  smartPointerConstString sPSTit(title, needDelete);

  LPCTSTR msg = getStringOrIdByLangGlob(idMsg, needDelete);
  smartPointerConstString sPSMsg(msg, needDelete);

  return svMessageBox(win, &sPSMsg, &sPSTit, flag);
}
//----------------------------------------------------------------------------
smartPointerConstString getStringOrIdByLangGlob(uint id)
{
  bool needDelete;
  LPCTSTR p = getStringOrIdByLangGlob(id, needDelete);
  return smartPointerConstString(p, needDelete);
}
//----------------------------------------------------------------------------
smartPointerConstString getStringByLangGlob(uint id)
{
  bool needDelete;
  LPCTSTR p = getStringByLangGlob(id, needDelete);
  return smartPointerConstString(p, needDelete);
}
//----------------------------------------------------------------------------
smartPointerConstString getStringByLangSimple(LPCTSTR source)
{
  bool needDelete;
  LPCTSTR p = getStringByLangSimple(source, needDelete);
  return smartPointerConstString(p, needDelete);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR getStringByLang(const setOfString& sos, uint id, bool& needDelete)
{
  return  getStringByLangSimple(sos.getString(id), needDelete);
}
//----------------------------------------------------------------------------
smartPointerConstString getStringByLang(const setOfString& sos, uint id)
{
  bool needDelete;
  LPCTSTR p = getStringByLang(sos, id, needDelete);
  return smartPointerConstString(p, needDelete);
}
//----------------------------------------------------------------------------
smartPointerConstString getStringOrIdByLang(const setOfString& sos, uint id)
{
  bool needDelete;
  LPCTSTR p = getStringOrIdByLang(sos, id, needDelete);
  return smartPointerConstString(p, needDelete);
}
//----------------------------------------------------------------------------
void setWindowTextByLang(const setOfString& sos, HWND hwnd, uint id)
{
  bool needDelete;
  LPCTSTR p = getStringOrIdByLang(sos, id, needDelete);
  smartPointerConstString sPS(p, needDelete);
  SetWindowText(hwnd, &sPS);
}
//----------------------------------------------------------------------------
LPCTSTR getStringOrIdByLang(const setOfString& sos, uint id, bool& needDelete)
{
  LPCTSTR p = getStringByLangSimple(sos.getString(id), needDelete);
  if(!p)
    p = sos.getStringOrId(id);
  return p;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void resetCacheLang(bool onlyBody)
{
  P_Body* bd = getCurrBody();
  if(bd)
    bd->resetCache();
  if(onlyBody)
    return;
  GlobalCache.reset();
}
//----------------------------------------------------------------------------
void resetGlobalCacheLang()
{
  GlobalCache.reset();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void P_BaseBody::resetCache()
{
  BodyCache.reset();
}
//----------------------------------------------------------------------------
void P_BaseBody::setWindowTextByLang(HWND hwnd, uint id)
{
  bool needDelete;
  LPCTSTR p = getStringOrIdByLang(id, needDelete);
  smartPointerConstString sPS(p, needDelete);
  SetWindowText(hwnd, &sPS);
}
//----------------------------------------------------------------------------
uint P_BaseBody::msgBoxByLang(PWin* win, uint idMsg, uint idTitle, uint flag)
{
  bool needDelete;
  LPCTSTR title = getStringOrIdByLang(idTitle, needDelete);
  smartPointerConstString sPSTit(title, needDelete);

  LPCTSTR msg = getStringOrIdByLang(idMsg, needDelete);
  smartPointerConstString sPSMsg(msg, needDelete);

  return svMessageBox(win, &sPSMsg, &sPSTit, flag);
}
//----------------------------------------------------------------------------
smartPointerConstString P_BaseBody::getStringOrIdByLang(uint id)
{
  bool needDelete;
  LPCTSTR p = getStringOrIdByLang(id, needDelete);
  return smartPointerConstString(p, needDelete);
}
//----------------------------------------------------------------------------
smartPointerConstString P_BaseBody::getStringByLang(uint id)
{
  bool needDelete;
  LPCTSTR p = getStringByLang(id, needDelete);
  return smartPointerConstString(p, needDelete);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR P_BaseBody::getStringByLang(uint id, bool& needDelete)
{
  LPCTSTR p = getFromBodyCache(id);
  if(p) {
    needDelete = false;
    return p;
    }
  p =  getStringByLangSimple(sStr.getString(id), needDelete);
  return addToCache(id, p, needDelete, BodyCache);
}
//----------------------------------------------------------------------------
LPCTSTR P_BaseBody::getStringOrIdByLang(uint id, bool& needDelete)
{
  LPCTSTR p = getFromBodyCache(id);
  if(p) {
    needDelete = false;
    return p;
    }
  p = getStringByLangSimple(sStr.getString(id), needDelete);
  if(!p) {
    p = str_newdup(sStr.getStringOrId(id));
    needDelete = true;
    return addToCache(id, p, needDelete, BodyCache);
    }
  return addToCache(id, p, needDelete, BodyCache);
}
//----------------------------------------------------------------------------
int getCurrLang()
{
  const cfg& Cfg = config::getAll();
  int currLang = Cfg.LangInUse;
  return currLang;
}
