//----------- language.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef LANGUAGE_H_
#define LANGUAGE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_txt.h"
#include "smartPS.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
LPCTSTR getStringOrIdByLangGlob(uint id, bool& needDelete);
LPCTSTR getStringByLangGlob(uint id, bool& needDelete);
//----------------------------------------------------------------------------
smartPointerConstString getStringOrIdByLangGlob(uint id);
smartPointerConstString getStringByLangGlob(uint id);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR getStringOrIdByLang(const setOfString& sos, uint id, bool& needDelete);
LPCTSTR getStringByLang(const setOfString& sos, uint id, bool& needDelete);
//----------------------------------------------------------------------------
smartPointerConstString getStringOrIdByLang(const setOfString& sos, uint id);
smartPointerConstString getStringByLang(const setOfString& sos, uint id);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR getStringByLangSimple(LPCTSTR source, bool& needDelete);
//----------------------------------------------------------------------------
smartPointerConstString getStringByLangSimple(LPCTSTR source);
//----------------------------------------------------------------------------
void setWindowTextByLang(const setOfString& sos, HWND hwnd, uint id);
//----------------------------------------------------------------------------
void setWindowTextByLangGlob(HWND hwnd, uint id, bool forceId = true);
//uint msgBoxByLang(const setOfString& sos, PWin* win, uint idMsg, uint idTitle, uint flag);
uint msgBoxByLangGlob(PWin* win, uint idMsg, uint idTitle, uint flag);
//----------------------------------------------------------------------------
void resetCacheLang(bool onlyBody);
//----------------------------------------------------------------------------
int getCurrLang();
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
