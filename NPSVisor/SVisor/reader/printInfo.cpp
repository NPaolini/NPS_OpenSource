//--------------- printInfo.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "printInfo.h"
//----------------------------------------------------------------------------
#include "p_file.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_date.h"
#include "commonInfo.h"
//----------------------------------------------------------------------------
#define NAME_INFO_PRINT _T("~info_print.tmp")
//----------------------------------------------------------------------------
printTree::printTree(PWin* par, svPrinter* printer, LPCTSTR fileRows) :
    baseClass(par, printer, NAME_INFO_PRINT), pfRows(fileRows, P_READ_ONLY),
    heightFont(16), heightRow(60), firstOper(true), firstRow(true)
{
  pfRows.P_open();
}
//----------------------------------------------------------------------------
#define TOP_Y 200
#define MAX_Y (2970 - 200)
//----------------------------------------------------------------------------
bool printTree::beginDoc(bool showSetup)
{
  if(!baseClass::beginDoc(showSetup))
    return false;
  return true;
}
//----------------------------------------------------------------------------
bool printTree::makeFooter()
{
  if(!baseClass::makeFooter())
    return false;
  return true;
}
//----------------------------------------------------------------------------
LPCTSTR printTree::getTitle()
{
  static TCHAR title[] = _T("Master sVisor");
  return title;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define X_TOT   200
#define X_OPER 1000
//#define X_TIME_OPER 1800
#define X_CAUSE 200

//#define X_TIME 700
//#define X_PERC 900
#define X_TIME 900
#define X_PERC (X_TIME+200)

#define X_TIME_TOT 900

#define X_RIGHT 1900
//----------------------------------------------------------------------------
void fillPerc(LPTSTR buff, const FILETIME& ftTot, const FILETIME& ft)
{
#if 1
  double v = (double)cMK_I64(ft);
  double vT = (double)cMK_I64(ftTot);
  v *= 100;
  v /= vT;
  v += 0.5;
  wsprintf(buff, _T("%02d%%"), (int)v);
#else
  __int64 v = cMK_I64(ft);
  __int64 vT = cMK_I64(ftTot);
  v *= 100;
  v /= vT;
  wsprintf(buff, _T("%02d%%"), (int)v);
#endif
}
//----------------------------------------------------------------------------
void fillTime(LPTSTR buff, const FILETIME& ft)
{
  int day;
  int hour;
  int minute;
  int second;
  extractTime(ft, day, hour, minute, second);
  if(second > 30) {
    if(++minute >= 60) {
      minute = 0;
      if(++hour >= 24) {
        hour = 0;
        ++day;
        }
      }
    }
#ifdef USE_DAY
  wsprintf(buff, _T("%d + %d:%02d"), day, hour, minute);
#else
  hour += day * 24;
  wsprintf(buff, _T("%d:%02d"), hour, minute);
#endif
}
//----------------------------------------------------------------------------
void printTree::fillMain(LPTSTR buff, const row4print& rp)
{
  Tot = rp.ft;
  wsprintf(buff, _T("Tempo di arresto:\r\n%d,%d,%d,"), PRN_TXT, X_TIME, Y);
  fillTime(buff + _tcslen(buff), rp.ft);
}
//----------------------------------------------------------------------------
void printTree::fillOper(LPTSTR buff, const row4print& rp)
{
  Oper = rp.ft;
  TCHAR tmp[100] = _T("");
  TD_Info* di = dynamic_cast<TD_Info*>(Parent);
  if(di)
    di->makeNameOper(tmp, SIZE_A(tmp), rp.code);
#if 1
  wsprintf(buff, _T("Operatore: %s"), tmp);
#else
  wsprintf(buff, _T("Operatore: %s\r\n%d,%d,%d,"), tmp, PRN_TXT, X_TIME_OPER, Y -20);
  fillTime(buff + _tcslen(buff), rp.ft);
#endif
}
//----------------------------------------------------------------------------
void printTree::fillCause(LPTSTR buff, const row4print& rp)
{
  TCHAR tmp[100] = _T("");
  TD_Info* di = dynamic_cast<TD_Info*>(Parent);
  if(di)
    di->makeNameCause(tmp, SIZE_A(tmp), rp.code);

  wsprintf(buff, _T("%04d-%s\r\n%d,%d,%d,"), rp.code, tmp, PRN_TXT, X_TIME, Y);
  fillTime(buff + _tcslen(buff), rp.ft);
  wsprintf(buff + _tcslen(buff), _T("\r\n%d,%d,%d,"),PRN_TXT, X_PERC, Y);
  fillPerc(buff + _tcslen(buff), Tot, rp.ft);
}
//----------------------------------------------------------------------------
void printTree::drawLines(LPTSTR buff, int nLine)
{
  const int deltaLine = 4;
  int result;
  for(int i = 0; i < nLine; ++i) {
#if 1
    PRINT5(File, buff, _T("%d,%d,%d,%d,%d\r\n"), PRN_LINE, X_CAUSE, Y, X_RIGHT, Y);
#else
    wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"), PRN_LINE, X_CAUSE, Y, X_RIGHT, Y);
    File->PRINT_STR(buff);
#endif
    Y += deltaLine;
    }
}
//----------------------------------------------------------------------------
bool printTree::makeHeader()
{
  if(!baseClass::makeHeader())
    return false;

  int result;
  TCHAR buff[200];
#if 1
    PRINT5(File, buff, _T("%d,%d,%d,%d,%s\r\n"), PRN_SET_FONT, 20, 0, 0, _T("Times New Roman"));
#else
  wsprintf(buff, _T("%d,%d,%d,%d,%s\r\n"), PRN_SET_FONT, 20, 0, 0, _T("Times New Roman"));
  File->PRINT_STR(buff);
#endif
  // cosa scrivere come linea?
//  TCHAR linea[] = "Linea filo";
  LPCTSTR linea = getString(ID_MAIN_TITLE);
  if(!linea)
    linea = _T("???");

#if 1
  PRINT5(File, buff, _T("%d,%d,%d,%s %s\r\n"), PRN_TXT, X_TOT, Y, _T("Linea:"), linea);
#else
  wsprintf(buff, _T("%d,%d,%d,%s %s\r\n"), PRN_TXT, X_TOT, Y, "Linea:", linea);
  File->PRINT_STR(buff);
#endif
  Y += 80;
  headerRow4print hr;
  if(!pfRows.P_read(&hr, sizeof(hr)))
    return false;

  TCHAR tDate[40];
  set_format_data(tDate, SIZE_A(tDate), hr.ftFrom, whichData(), _T(" - "));
#if 1
  PRINT4(File, buff, _T("%d,%d,%d,Periodo da: %s\r\n"), PRN_TXT, X_TOT, Y, tDate);
#else
  wsprintf(buff, _T("%d,%d,%d,Periodo da: %s\r\n"), PRN_TXT, X_TOT, Y, tDate);
  File->PRINT_STR(buff);
#endif
  set_format_data(tDate, SIZE_A(tDate), hr.ftTo, whichData(), _T(" - "));
#if 1
  PRINT4(File, buff, _T("%d,%d,%d,Periodo a: %s\r\n"), PRN_TXT, X_OPER, Y, tDate);
#else
  wsprintf(buff, _T("%d,%d,%d,Periodo a: %s\r\n"), PRN_TXT, X_OPER, Y, tDate);
  File->PRINT_STR(buff);
#endif
  Y += 60;
#if 1
  PRINT3(File, buff, _T("%d,%d,%d,Diametro filo ______________\r\n"), PRN_TXT, X_TOT, Y);
  PRINT3(File, buff, _T("%d,%d,%d,Velocità linea ______________\r\n"), PRN_TXT, X_OPER, Y);
#else
  wsprintf(buff, _T("%d,%d,%d,Diametro filo ______________\r\n"), PRN_TXT, X_TOT, Y);
  File->PRINT_STR(buff);
  wsprintf(buff, _T("%d,%d,%d,Velocità linea ______________\r\n"), PRN_TXT, X_OPER, Y);
  File->PRINT_STR(buff);
#endif
  Y += 80;

  drawLines(buff, 2);
  Y += 10;
  hr.ftTo -= hr.ftFrom;
  fillTime(tDate, hr.ftTo);

#if 1
  PRINT3(File, buff, _T("%d,%d,%d,Tempo a disposizione\r\n"), PRN_TXT, X_TOT, Y);
  PRINT4(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, X_TIME_TOT, Y, tDate);
#else
  wsprintf(buff, _T("%d,%d,%d,Tempo a disposizione\r\n"), PRN_TXT, X_TOT, Y);
  File->PRINT_STR(buff);
  wsprintf(buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, X_TIME_TOT, Y, tDate);
  File->PRINT_STR(buff);
#endif
  Y += 60;

  row4print rp;
  if(!pfRows.P_read(&rp, sizeof(rp)))
    return false;
  pfRows.P_seek(-(long)sizeof(rp), SEEK_CUR_);

  hr.ftFrom = hr.ftTo - rp.ft;
  fillTime(tDate, hr.ftFrom);
  TCHAR perc[10];
  fillPerc(perc, hr.ftTo, hr.ftFrom);

#if 1
  PRINT3(File, buff, _T("%d,%d,%d,Tempo di marcia\r\n"), PRN_TXT, X_TOT, Y);
  PRINT5(File, buff, _T("%d,%d,%d,%s %s\r\n"), PRN_TXT, X_TIME_TOT, Y, tDate, perc);
#else
  wsprintf(buff, _T("%d,%d,%d,Tempo di marcia\r\n"), PRN_TXT, X_TOT, Y);
  File->PRINT_STR(buff);

  wsprintf(buff, _T("%d,%d,%d,%s %s\r\n"), PRN_TXT, X_TIME_TOT, Y, tDate, perc);
  File->PRINT_STR(buff);
#endif
  Y += 60;

  fillTime(tDate, rp.ft);
  fillPerc(perc, hr.ftTo, rp.ft);

#if 1
  PRINT3(File, buff, _T("%d,%d,%d,Tempo d'arresto\r\n"), PRN_TXT, X_TOT, Y);
  PRINT5(File, buff, _T("%d,%d,%d,%s %s\r\n"), PRN_TXT, X_TIME_TOT, Y, tDate, perc);
#else
  wsprintf(buff, _T("%d,%d,%d,Tempo d'arresto\r\n"), PRN_TXT, X_TOT, Y);
  File->PRINT_STR(buff);

  wsprintf(buff, _T("%d,%d,%d,%s %s\r\n"), PRN_TXT, X_TIME_TOT, Y, tDate, perc);
  File->PRINT_STR(buff);
#endif
  Y += 80;
  drawLines(buff, 2);
  Y += 10;

#if 1
  PRINT4(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_SET_FONT, 20, 0, _T("arial"));
  PRINT3(File, buff, _T("%d,%d,%d,Cause d'arresto\r\n"), PRN_TXT, X_TOT, Y);
#else
  wsprintf(buff, _T("%d,%d,%d,%s\r\n"), PRN_SET_FONT, 20, 0, _T("arial"));
  File->PRINT_STR(buff);

  wsprintf(buff, _T("%d,%d,%d,Cause d'arresto\r\n"), PRN_TXT, X_TOT, Y);
  File->PRINT_STR(buff);
#endif
  Y += 80;
  firstRow = true;
  return true;
}
//----------------------------------------------------------------------------
int printTree::addRow()
{
  row4print rp;
  if(!pfRows.P_read(&rp, sizeof(rp))) {
    if(!firstRow)
      Y = MAX_Y;
    return -1;
    }
  firstRow = false;
  int result;
  TCHAR buff[500];
  switch(rp.id) {
    case 1:
#if 1
      PRINT3(File, buff, _T("%d,%d,%d,"), PRN_TXT, X_TOT, Y);
      fillMain(buff, rp);
      PRINT0(File, buff);
      PRINT0(File, _T("\r\n"));
#else
      wsprintf(buff, _T("%d,%d,%d,"), PRN_TXT, X_TOT, Y);
      File->PRINT_STR(buff);
      fillMain(buff, rp);
      File->PRINT_STR(buff);
      File->PRINT_STR(_T("\r\n"));
#endif
      Y += 80;
      firstOper = true;
      break;
    case 2:
      if(!firstOper) {
        firstOper = true;
        Y = MAX_Y;
//        pfRows.P_seek(-(long)sizeof(rp), SEEK_CUR_);
        }
      else {
        firstOper = false;
#if 1
        PRINT3(File, buff, _T("%d,%d,%d,"), PRN_TXT, X_TOT, Y);
        fillOper(buff, rp);
        PRINT0(File, buff);
        PRINT0(File, _T("\r\n"));
#else
        wsprintf(buff, _T("%d,%d,%d,"), PRN_TXT, X_OPER, TOP_Y);
        File->PRINT_STR(buff);
        fillOper(buff, rp);
        File->PRINT_STR(buff);
        File->PRINT_STR(_T("\r\n"));
#endif
        }
      break;
    case 3:
#if 1
      PRINT3(File, buff, _T("%d,%d,%d,"), PRN_TXT, X_CAUSE, Y);
      fillCause(buff, rp);
      PRINT0(File, buff);
      PRINT0(File, _T("\r\n"));
#else
      wsprintf(buff, _T("%d,%d,%d,"), PRN_TXT, X_CAUSE, Y);
      File->PRINT_STR(buff);
      fillCause(buff, rp);
      File->PRINT_STR(buff);
      File->PRINT_STR(_T("\r\n"));
#endif
      Y += 60;
      drawLines(buff);
      Y += 10;
      break;
    }
  return 1;
}
//----------------------------------------------------------------------------
printTree* allocPrintTree(PWin* par, svPrinter* printer, LPCTSTR filename)
{
  return new printTree(par, printer, filename);
}
//----------------------------------------------------------------------------
