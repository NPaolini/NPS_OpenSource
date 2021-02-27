//-------------------- pVisualTextEdit.cpp ------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pVisualTextEdit.h"
#include "smartPs.h"
#include "p_util.h"
//-------------------------------------
void pVisualTextEdit::setFocus()
{
  onFocus = true;
}
//-------------------------------------
void pVisualTextEdit::killFocus()
{
  onFocus = false;
}
//-------------------------------------
//-------------------------------------
void pVisualTextEdit::getData(LPTSTR buff, size_t dim)
{
  ZeroMemory(buff, dim);
  GetWindowText(*Owner, buff, (int)dim);
}
//-------------------------------------
void pVisualTextEdit::setData(LPCTSTR buff)
{
  SetWindowText(*Owner, buff);
  SendMessage(*Owner, EM_SETMODIFY, 0, 0);
}
//-------------------------------------
//-------------------------------------
void pVisualTextEditSep::setFocus()
{
  TCHAR t[512];
  GetWindowText(*Owner, t, SIZE_A(t));
  removeSep(t, Sep);
  SetWindowText(*Owner, t);
  baseClass::setFocus();
}
//-------------------------------------
void pVisualTextEditSep::killFocus()
{
  baseClass::killFocus();
  if(*Owner) {
    TCHAR s[512];
    TCHAR t[512 + 16];
    GetWindowText(*Owner, s, SIZE_A(s));
    if(DecSep)
      addSep(t, s, Sep, DecSep, Step, Sign);
    else
      addSep(t, s, Sep, Step, Sign);
    SetWindowText(*Owner, t);
    }
}
//-------------------------------------
void pVisualTextEditSep::getData(LPTSTR buff, size_t dim)
{
  baseClass::getData(buff, dim);
  if(!onFocus)
    removeSep(buff, Sep);
}
//-------------------------------------
void pVisualTextEditSep::setData(LPCTSTR buff)
{
  if(onFocus)
    baseClass::setData(buff);
  else {
    TCHAR t[512];
    if(DecSep)
      addSep(t, buff, Sep, DecSep, Step, Sign);
    else
      addSep(t, buff, Sep, Step, Sign);
    baseClass::setData(t);
    }
}
//-------------------------------------
void pVisualTextEditSep::removeSep(LPTSTR buff, TCHAR sep)
{
  LPTSTR p = buff;
  while(*buff) {
    if(sep != *buff)
      *p++ = *buff++;
    else
      ++buff;
    }
  *p = 0;
}
//-------------------------------------
void pVisualTextEditSep::addSep(LPTSTR target, LPCTSTR source, TCHAR sep, uint step, bool sign)
{
  if(sign) {
    if(_T('-') == *source || _T('+') == *source)
      *target++ = *source++;
    }
  uint len = (uint)_tcslen(source);
  if(len > step) {
    uint offset = len % step;
    if(offset) {
      while(offset-- > 0)
        *target++ = * source++;
      *target++ = sep;
      }
    }
  uint count = 0;
  while(*source) {
    *target++ = *source++;
    if(!(++count % step) && *source)
      *target++ = sep;
    }
  *target = 0;
}
//-------------------------------------
void pVisualTextEditSep::addSep(LPTSTR target, LPCTSTR source, TCHAR sep, TCHAR decSep, uint step, bool sign)
{
  if(sign) {
    if(_T('-') == *source || _T('+') == *source)
      *target++ = *source++;
    }
  uint len = (uint)_tcslen(source);
  smartPointerString tmp(new TCHAR[1024], true);
  LPTSTR t = &tmp;
  for(uint i = 0; i < len; ++i) {
    t[i] = source[i];
    if(decSep == source[i]) {
      t[i] = 0;
      addSep(target, t, sep, step);
      LPTSTR p = target + _tcslen(target);
      copyStrZ(p, source + i, len - i + 1);
      return;
      }
    }
  // non ci sono i decimali, esegue quello normale
  addSep(target, source, sep, step);
}
