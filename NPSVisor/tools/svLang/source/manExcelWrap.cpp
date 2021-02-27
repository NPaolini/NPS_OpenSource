//----------- manExcelWrap.cpp ----------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "manExcel.h"
#include "P_ModEdit.h"
#include "p_util.h"
#include "BasicExcelVC6.hpp"
using namespace YExcel;
//---------------------------------------------------------------------
uint GetCellString(BasicExcelCell* cell, LPTSTR target, size_t maxLen)
{
  size_t len = cell->GetStringLength();
  len = min(len, maxLen);

  LPCSTR str = cell->GetString();
  if(str) {
#ifdef UNICODE
    MultiByteToWideChar(CP_ACP, 0, str, len, target, len);
#else
    copyStr(target, str, len);
#endif
    target[len] = 0;
    return len;
    }

  const wchar_t* wstr = cell->GetWString();
  if(wstr) {
    copyStr(target, wstr, len);
    target[len] = 0;
    return len;
    }
  return 0;
}
//---------------------------------------------------------------------
#define MAX_LANG 10
#define LANG_SEP _T("§")
//---------------------------------------------------------------------
bool writeLanguage(BasicExcelWorksheet* sheet, const PVect<LPTSTR>& lang)
{
  uint nElem = lang.getElem();
  BasicExcelCell* cell;
  cell = sheet->Cell(0, 0);
  cell->Set(getStringOrDef(8, _T("Codice")));
  uint i;
  for(i = 0; i < nElem; ++i) {
    cell = sheet->Cell(0, i + 1);
    cell->Set(lang[i]);
    }
  for(; i < MAX_LANG; ++i) {
    cell = sheet->Cell(0, i + 1);
    TCHAR buff[100];
    wsprintf(buff, getStringOrDef(45, _T("Lang %d")), i + 1);
    cell->Set(buff);
    }
  return true;
}
//---------------------------------------------------------------------
bool writePageRow(BasicExcelWorksheet* sheet, uint row, int code, LPCTSTR str, bool noSpace)
{
  BasicExcelCell* cell;
  cell = sheet->Cell(row, 0);
  cell->Set(code);
  freeList& fL = getSetFree();
  LPTSTR t = fL.get();
  if(*LANG_SEP == *str) {
    pvvChar target;
    uint nElem = splitParam(target, str + 1, *LANG_SEP);
    BasicExcelCell* cell;
    for(uint i = 0; i < nElem; ++i) {
      if(*target[i].getVect()) {
        cell = sheet->Cell(row, i + 1);
        translateFromCRNL(t, target[i].getVect());
        if(noSpace)
          trim(t);
        cell->Set(t);
        }
      }
    }
  else {
    cell = sheet->Cell(row, 1);
    translateFromCRNL(t, str);
    if(noSpace)
      trim(t);
    cell->Set(t);
    }
  fL.release(t);
  return true;
}
//---------------------------------------------------------------------
bool writePage(BasicExcelWorksheet* sheet, setOfString& set, const PVect<LPTSTR>& lang, bool noSpace)
{
  if(!set.setFirst())
    return true;
  if(!writeLanguage(sheet, lang))
    return false;
  uint row = 1;
  do {
    LPCTSTR p = set.getCurrString();
    if(!writePageRow(sheet, row, set.getCurrId(), p, noSpace))
      return false;
    ++row;
    } while(set.setNext());
  return true;
}
//---------------------------------------------------------------------
bool readLanguage(BasicExcelWorksheet* sheet, PVect<LPCTSTR>& lang)
{
  BasicExcelCell* cell;
  for(uint i = 0; i < MAX_LANG + 1; ++i) {
    cell = sheet->Cell(0, i);
    TCHAR buff[100] = _T("");
    GetCellString(cell, buff, SIZE_A(buff) - 1);
    lang[lang.getElem()] = str_newdup(buff);
    }
  return true;
}
//---------------------------------------------------------------------
uint readPageRow(BasicExcelWorksheet* sheet, uint row, LPTSTR& str)
{
  BasicExcelCell* cell;
  cell = sheet->Cell(row, 0);
  uint code = cell->GetInteger();
  if(!code)
    return 0;
  PVect<LPCTSTR> txt;
  freeList& fl = getSetFree();
  uint len = 1;
  uint nEl = 0;
  for(uint i = 1; i < MAX_LANG + 1; ++i) {
    LPTSTR buff = fl.get();
    buff[0] = 0;
    cell = sheet->Cell(row, i);
    len += GetCellString(cell, buff, SIZE_SINGLE_BUFF - 1) + 1;
    if(*buff)
      ++nEl;
    txt[txt.getElem()] = buff;
    }
  if(1 >= nEl) {
    nEl = txt.getElem();
    uint i;
    for(i = 0; i < nEl; ++i) {
      if(txt[i][0] != 0) {
        str = str_newdup(txt[i]);
        break;
        }
      fl.release(txt[i]);
      }
    for(; i < nEl; ++i)
      fl.release(txt[i]);
    }
  else {
    ++len;
    str = new TCHAR[len];
    str[0] = 0;
    uint nElem = txt.getElem();
    for(uint i = 0; i < nElem; ++i) {
      _tcscat_s(str, len, LANG_SEP);
      _tcscat_s(str, len, txt[i]);
      fl.release(txt[i]);
      }
    }
  return code;
}
//---------------------------------------------------------------------
bool readPage(BasicExcelWorksheet* sheet, setOfString& set)
{
  int row = 1;
  int maxRow = sheet->GetTotalRows();
  while(row < maxRow) {
    LPTSTR str = 0;
    uint code = readPageRow(sheet, row, str);
    if(code)
      if(str)
        set.replaceString(code, translateToCRNL(str, str), true);
      else
        set.replaceString(code, str_newdup(str), true);
    ++row;
    }
  return true;
}
//---------------------------------------------------------------------
