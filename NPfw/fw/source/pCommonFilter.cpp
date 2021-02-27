//---------------- pCommonFilter.cpp ------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pCommonFilter.h"
//-----------------------------------------------------------
bool PFilter::isCommonKey(WPARAM& key, HWND /*ctrl*/)
{
  switch(key) {
//    case VK_DELETE:
    case VK_BACK:
    case VK_RETURN:
    case VK_TAB:
      return true;
    }
  return false;
}
//-----------------------------------------------------------
bool PFilter::getSel(HWND ctrl, int& init, int& end)
{
  LRESULT sel = SendMessage(ctrl, EM_GETSEL, 0, 0);
  if(-1 == sel)
    return false;

  init = LOWORD(sel);
  end  = HIWORD(sel);
  return true;
}
//-----------------------------------------------------------
void PFilter::addString(HWND ctrl, LPCTSTR str, int initPos, int endPos)
{
  SendMessage(ctrl, EM_SETSEL, (WPARAM)initPos, (LPARAM)endPos);
  SendMessage(ctrl, EM_REPLACESEL, true, (LPARAM)str);
}
//-----------------------------------------------------------
bool PNumbFilter::accept(WPARAM& key, HWND ctrl)
{
  if(isCommonKey(key, ctrl))
    return true;

  if(_istdigit(key))
    return true;
  if(_T('-') == key || _T('+') == key)
    return true;
  if(_T('.') == key || _T(',') == key) {
    return true;
    }
  return false;
}
//-----------------------------------------------------------
PNumbFilterComma::PNumbFilterComma() : comma(_T(','))
{
  TCHAR buff[8];
  int res = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, buff, SIZE_A(buff));
  if(res)
    comma = *buff;
}
//-----------------------------------------------------------
bool PNumbFilterComma::accept(WPARAM& key, HWND ctrl)
{
  if(!baseClass::accept(key, ctrl))
    return false;

  if(_T('.') == key || _T(',') == key)
    if(key != comma)
      key = comma;
  return true;
}
//-----------------------------------------------------------
void PDateTimeFilter::clone(const PFilter& other)
{
  if(this != &other) {
    const PDateTimeFilter* dtf = dynamic_cast<const PDateTimeFilter*>(&other);
    if(dtf)
      clone(*dtf);
    else
      baseClass::clone(other);
    }
}
//-----------------------------------------------------------
void PDateTimeFilter::initCtrl(HWND ctrl)
{
  SendMessage(ctrl, EM_SETLIMITTEXT, getLimitLen(), 0);
}
//-----------------------------------------------------------
bool PDateTimeFilter::accept(WPARAM& key, HWND ctrl)
{
  if(isCommonKey(key, ctrl))
    return true;

  bool isOk = toBool(_istdigit(key));

  if(!isOk)
    isOk = getSep() == key;

  if(!isOk)
    return false;

  int init;
  int end;
  if(!getSel(ctrl, init, end))
    return false;
  int pos1 = 1;
  int pos2 = 4;
  if(!isStandard()) {
    pos1 += 2;
    pos2 += 2;
    }
  if(pos1 == init || pos2 == init) {
    TCHAR buff[3];
    buff[0] = key;
    buff[1] = getSep();
    buff[2] = 0;
    addString(ctrl, buff, init, end);
    // torna false perché è già stato caricato sia il carattere
    // che il separatore
    return false;
    }
  return true;
}
//-----------------------------------------------------------
void PDateFilter::onKillFocus(HWND ctrl)
{
  // se non è standard ... l'anno deve metterlo per forza con quattro cifre
  if(!isStandard())
    return;
  TCHAR t[MAX_DATE_LEN + 2] = { 0 };
  GetWindowText(ctrl, t, MAX_DATE_LEN + 1);
  uint len = (uint)_tcslen(t);
  if(len < MAX_DATE_LEN) {
    // gestiamo solo questo caso
    if(8 == len) { // non sono state immesse le quattro cifre per l'anno, ma solo due
      uint year = _ttoi(t + 6) + 2000;
      wsprintf(t + 6, _T("%d"), year);
      SetWindowText(ctrl, t);
      }
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool PHexFilter::accept(WPARAM& key, HWND ctrl)
{
  if(isCommonKey(key, ctrl))
    return true;

  if(_istdigit(key))
    return true;

  if(key >= _T('A') && key <= _T('F'))
    return true;

  if(key >= _T('a') && key <= _T('f'))
    return true;

  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool PBinaryFilter::accept(WPARAM& key, HWND ctrl)
{
  if(isCommonKey(key, ctrl))
    return true;

  if(_T('0') == key || _T('1') == key)
    return true;

  return false;
}
//-----------------------------------------------------------
void PDateAndTimeFilter::clone(const PFilter& other)
{
  if(this != &other) {
    const PDateAndTimeFilter* dtf = dynamic_cast<const PDateAndTimeFilter*>(&other);
    if(dtf)
      clone(*dtf);
    else
      baseClass::clone(other);
    }
}
//-----------------------------------------------------------
void PDateAndTimeFilter::initCtrl(HWND ctrl)
{
  SendMessage(ctrl, EM_SETLIMITTEXT, getLimitLen(), 0);
}
//-----------------------------------------------------------
bool PDateAndTimeFilter::accept(WPARAM& key, HWND ctrl)
{
  if(isCommonKey(key, ctrl))
    return true;

  bool isOk = toBool(_istdigit(key));

  bool isSep = false;
  if(!isOk) {
    switch(key) {
      case _T('/'):
      case _T(' '):
      case _T(':'):
      case _T('-'):
        isOk = true;
        isSep = true;
        break;
      }
    }
//    isOk = getSep() == key;

  if(!isOk)
    return false;

  int init;
  int end;
  if(!getSel(ctrl, init, end))
    return false;
  int pos1 = 1;
  int pos2 = 4;
  // dd/mm/yyyy - hh:mm:ss
  int pos3 = 9;
  int pos4 = 14;
  int pos5 = 17;
  if(!isStandard()) {
    pos1 += 2;
    pos2 += 2;
    }
  LPCTSTR sep = 0;
  if(pos1 == init || pos2 == init)
    sep = _T("/");
  else if(pos3 == init)
    sep = _T(" - ");
  else if(pos4 == init || pos5 == init)
    sep = _T(":");
  if(sep) {
    // non possiamo avere un separatore nel carattere prima del separatore
    if(isSep)
      return false;
    TCHAR buff[8];
    buff[0] = key;
    _tcscpy_s(buff + 1, SIZE_A(buff) - 1, sep);
//    buff[1] = sep;
//    buff[2] = 0;
    addString(ctrl, buff, init, end);
    // torna false perché è già stato caricato sia il carattere
    // che il separatore
    return false;
    }
  return true;
}
//-----------------------------------------------------------
