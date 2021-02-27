//--------------- printDataVar.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "printDataVar.h"
#include "p_file.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_date.h"
//----------------------------------------------------------------------------
#define NAME_DATA_PRINT _T("~data_print.tmp")
//----------------------------------------------------------------------------
#define DEF_LEN_ROW 100
//----------------------------------------------------------------------------
printDataVar::printDataVar(PWin* par, svPrinter* printer, LPCTSTR fileTemplate) :
    baseClass(par, printer, NAME_DATA_PRINT), basePath(0),
    currPos(0), lenBuff(0), lenFile(0), buffFile(0), buffRow(0)
{
  buffFile = openFile(fileTemplate, lenFile);
  if(buffFile) {
    lenBuff = DEF_LEN_ROW;
    buffRow = new TCHAR[lenBuff + 2];
    }
  do {
    TCHAR t[_MAX_PATH];
    LPTSTR base = 0;
    setOfString set(fileTemplate);
    // se è nel nuovo formato, carica il path base
    if(set.getString(ID_FILE_DATA) && set.getString(ID_FILE_DATA_VARS)) {
      _tcscpy_s(t, fileTemplate);
      for(int i = _tcslen(t) - 1; i > 0; --i) {
        if(_T('\\') == t[i]) {
          t[i] = 0;
          base = t + i;
          appendPath(t, _T("image"));
          basePath = str_newdup(t);
          break;
          }
        }
      }
    LPCTSTR p = set.getString(ID_FILE_LINKED);
    if(p) {
      if(!base) {
        _tcscpy_s(t, fileTemplate);
        for(int i = _tcslen(t) - 1; i > 0; --i) {
          if(_T('\\') == t[i]) {
            t[i] = 0;
            base = t + i;
            break;
            }
          }
        }
      if(base)
        *base = 0;
      appendPath(t, p);
      uint len2;
      LPTSTR b2 = openFile(t, len2);
      if(b2 && len2) {
        uint len = lenFile + len2 + 2;
        LPTSTR b3 = new TCHAR[len];
        _tcscpy_s(b3, len, b2);
        _tcscpy_s(b3 + len2, len - len2, buffFile);
        delete []b2;
        delete []buffFile;
        buffFile = b3;
        lenFile += len2;
        }
      }
    } while(false);
}
//----------------------------------------------------------------------------
printDataVar::~printDataVar()
{
  delete []buffRow;
  delete []buffFile;
  delete []basePath;
}
//----------------------------------------------------------------------------
bool printDataVar::beginDoc(bool showSetup)
{
  if(!lenFile)
    return false;
  if(!baseClass::beginDoc(showSetup))
    return false;

  return true;
}
//----------------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//----------------------------------------------------------------------------
static LPTSTR getLine(LPTSTR buff, int len)
{
  if(len < 0)
    return 0;
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) <= _T('\r'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;
  if(U_(*p) <= _T('\r') && *p != c) {
    ++p;
    ++i;
    }
  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
bool printDataVar::checkBmp(LPCTSTR p)
{
  if(!basePath)
    return false;
  LPCTSTR p2 = findNextParamTrim(p, 5);
  if(P_File::P_exist(p2))
    return false;

  TCHAR t[_MAX_PATH];
  copyStrZ(t, p, p2 - p);
  _tcscat_s(t, basePath);
  int result;
  for(int i = _tcslen(p) - 1; i > 0; --i) {
    if(_T('\\') == p[i]) {
      appendPath(t, p + i + 1);
      PRINT0(File, t);
      PRINT0(File, _T("\r\n"));
      break;
      }
    }
  return true;
}
//----------------------------------------------------------------------------
int printDataVar::addRow()
{
  LPTSTR p = buffFile + currPos;
  while(p && currPos < (int)lenFile) {
    LPTSTR next = getLine(p, lenFile - currPos);

    long id = _ttol(p);
    int len = next ? next - p : lenFile - currPos;
    currPos += len;
    int result;
    if(id) {
      switch(id) {
        case PRN_BMP:          // X,Y,W,H,nome_file_BMP
          if(checkBmp(p))
            break;
        case PRN_TXT:      // X,Y,testo
        case PRN_LINE:         // X1,Y1,X2,Y2
        case PRN_POLYLINE:     // nlinee, Xiniz,Yiniz,X1,Y1,X2,Y2,..Xn,Yn
        case PRN_BOX:          // X,Y,W,H
        case PRN_SET_FONT:     // H,L,Flag (&1->Italic, &2->Bold, &4->Underlined), nome del font ('*' -> default)
        case PRN_SET_FONT2:     // H,L,angolo,Flag,nome del font ('*' -> default)

        case PRN_SET_ALIGN:    // ALIGN_HORZ,ALIGN_VERT
        case PRN_SET_BKMODE:   // OPAQUE | TRANSPARENT

        case PRN_SET_BRUSH3:    // nome_file_BMP
        case PRN_RECT:          // X,Y,W,H
        case PRN_OVAL:          // X,Y,W,H,filled
        case PRN_TXT_ON_BOX:    // X,Y,W,H,testo
        case PRN_SET_NULL_BRUSH:    // dummy
        case PRN_ROUND_RECT:    // X,Y,W,H,cx,cy
          // il formato è già pronto per la gestione base
          PRINT0(File, p);
          PRINT0(File, _T("\r\n"));
          break;

        case PRN_SET_COLOR_FG: // C
        case PRN_SET_COLOR_BKG:// C
        case PRN_SET_PEN:      // color, width, style
        case PRN_SET_BRUSH1:    // color
        case PRN_SET_BRUSH2:    // color, style
          printColorAndOther(id, p);
          break;

        case pvSimpleVar:
        case pvBoxVar:
        case pvBitmapVar:
          printVar(id, p);
          break;

        case pvFormFeed:
          Y = MAX_Y;
        }
      return 1;
      }
    p = next;
    }
  Y = MAX_Y;
  return -1;
}
//----------------------------------------------------------------------------
bool printDataVar::printVar(int idCode, LPCTSTR p)
{
  int idVar = 0;
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int align = 0;
  p = findNextParam(p, 1);
  switch(idCode) {
    case pvSimpleVar:
      _stscanf_s(p, _T("%d,%d,%d"), &x, &y, &idVar);
      break;

    case pvBoxVar:
      _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &x, &y, &w, &h, &align, &idVar);
      break;

    case pvBitmapVar:
      _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &x, &y, &w, &h, &idVar);
      break;
    }

  for(;;) {
    int needed = getVarText(buffRow, lenBuff, idCode, idVar);
    if(needed > lenBuff) {
      delete  []buffRow;
      buffRow = new TCHAR[needed + 2];
      lenBuff = needed;
      }
    else
      break;
    }
  TCHAR buff[128] = _T("");
  int result;
  switch(idCode) {
    case pvSimpleVar:
      PRINT3_A_S(File, buff, _T("%d,%d,%d,"), PRN_TXT, x, y)
//      wsprintf(buff, _T("%d,%d,%d,"), PRN_TXT, x, y);
      break;
    case pvBoxVar:
      PRINT6_A_S(File, buff, _T("%d,%d,%d,%d,%d,%d,"), PRN_TXT_ON_BOX, x, y, w, h, align)
//      wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,"), PRN_TXT_ON_BOX, x, y, w, h, align);
      break;
    case pvBitmapVar:
      PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%d,"), PRN_BMP, x, y, w, h)
//      wsprintf(buff, _T("%d,%d,%d,%d,%d,"), PRN_BMP, x, y, w, h);
      break;
    }
//  File->P_writeString(buff);
  if(!*buffRow)
    PRINT1_A_S(File, buff, _T("var_%d ???"), idVar)
//    wsprintf(buffRow, _T("var_%d ???"), idVar);
  else
    PRINT0(File, buffRow)
//  File->P_writeString(buffRow);  int result;
  PRINT_CR_NL(File, buff);
  return toBool(result);
}
//----------------------------------------------------------------------------
bool printDataVar::printColorAndOther(int id, LPCTSTR p)
{
  p = findNextParam(p, 1);
  int r = 0;
  int g = 0;
  int b = 0;
  _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
  TCHAR buff[80];

  int result;
  PRINT2_A_S(File, buff, _T("%d,%d"), id, RGB(r, g, b))
//  wsprintf(buff, _T("%d,%d"), id, RGB(r, g, b));
//  File->P_writeString(buff);
  p = findNextParam(p, 3);
  if(p) {
    --p;
    PRINT0(File, p)
    }
//    File->P_writeString(p);
  PRINT_CR_NL(File, buff);
  return toBool(result);
}
//----------------------------------------------------------------------------
