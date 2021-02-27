//--------------- printAlarm.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "printAlarm.h"
#include "p_file.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_date.h"
//----------------------------------------------------------------------------
#define NAME_ALARM_PRINT _T("~alarm_print.tmp")
//----------------------------------------------------------------------------
#define DIM_LOCAL_BUFF 512
//----------------------------------------------------------------------------
printAlarm::printAlarm(PWin* par, svPrinter* printer, LPCTSTR fileRows) :
    baseClass(par, printer, NAME_ALARM_PRINT), pfRows(fileRows, P_READ_ONLY),
    heightFont(16), heightRow(60), idInitAlarm(ID_INIT_LABEL_PRINT_ALARM)

{
  ZeroMemory(&Header, sizeof(Header));
  if(pfRows.P_open())
    pfRows.P_read(&Header, sizeof(Header));
  if(2 == Header.type)
    idInitAlarm = ID_INIT_LABEL_PRINT_REPORT;
  LPCTSTR p = getString(idInitAlarm);
  if(p && *p) {
    int n = _ttoi(p);
    p = findNextParam(p, 1);
    if(p)
      heightFont = _ttoi(p);
    p = findNextParam(p, 1);
    if(p)
      heightRow = _ttoi(p);

    posX.setDim(n);
    int ix = 0;
    for(int i = 0; i < n; ++i) {
      p = getString(idInitAlarm + 1 + i);
      if(p) {
        posX[ix] = _ttoi(p);
        p = findNextParam(p, 1);
        align[ix] = _ttoi(p);
        ++ix;
        }
      }
    }
}
//----------------------------------------------------------------------------
#define HEIGHT_FONT_HEADER 20
#define HEIGHT_FONT_ROWS   heightFont
//----------------------------------------------------------------------------
#define OFFS_Y_ROWS heightRow
//----------------------------------------------------------------------------
#define FORM_CENTER ((pageForm.right + pageForm.left) / 2)
//----------------------------------------------------------------------------
static
void makeHorzLines(P_File* pf, LPTSTR buff, int x1, int x2, int y, int nLine, int offs)
{
  int result;
  for(int i = 0; i < nLine; ++i) {
    PRINT5_S(pf, buff, DIM_LOCAL_BUFF, _T("%d,%d,%d,%d,%d\r\n"), PRN_LINE, x1, y, x2, y);
    y += offs;
    }
}
//----------------------------------------------------------------------------
#define TWO_LINE(y) \
    makeHorzLines(File, buff, pageForm.left, pageForm.right, y, 2, 5)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static
LPCTSTR readMargin(LPCTSTR p, int& m)
{
  if(p) {
    int v = _ttoi(p);
    if(v >= 0)
      m = v;
    p = findNextParam(p, 1);
    }
  return p;
}
//----------------------------------------------------------------------------
bool printAlarm::beginDoc(bool showSetup)
{
  if(!baseClass::beginDoc(showSetup))
    return false;
  LPCTSTR p = getString(idInitAlarm);
  if(p) {
    PRect r = getMargin();

    p = findNextParam(p, 3);

    p = readMargin(p, r.left);
    p = readMargin(p, r.top);
    p = readMargin(p, r.right);
    p = readMargin(p, r.bottom);
    setMargin(r);
    }
  return true;
}
//----------------------------------------------------------------------------
extern LPCTSTR checkNewMode(LPCTSTR p);
//----------------------------------------------------------------------------
bool printAlarm::makeHeader()
{
  if(!baseClass::makeHeader())
    return false;

  TCHAR buff[DIM_LOCAL_BUFF];
  int result;
  PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_CENTER, TA_TOP);

  PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%s\r\n"), PRN_SET_FONT, HEIGHT_FONT_HEADER, 0, 0, _T("arial"));

  PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, FORM_CENTER, Y, Header.title);

  LPCTSTR p = getString(idInitAlarm);
  p = findNextParam(p, 8);
  if(!p)
    p = _T("arial");
  PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%s\r\n"), PRN_SET_FONT, HEIGHT_FONT_ROWS, 0, 0, p);

  Y += 60;
  TWO_LINE(Y);
  Y += 50;
//  if(1 == Header.type) {
    int nElem = posX.getElem();
    int ix = 0;
    int oldAlign = -1;
    for(int i = 0; i < nElem; ++i) {
      LPCTSTR p = getString(idInitAlarm + 1 + i);
      if(p) {
        p = findNextParam(p, 2);
        p = checkNewMode(p);

        if(p) {
          smartPointerConstString sp = getStringByLangSimple(p);

          if(oldAlign != align[ix]) {
            oldAlign = align[ix];
            int al = oldAlign == 0 ? TA_LEFT : oldAlign == 1 ? TA_CENTER : TA_RIGHT;
            PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, al, TA_TOP);
            }
          PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, pageForm.left + posX[ix], Y, &sp);
          ++ix;
          }
        }
      }
    Y += OFFS_Y_ROWS;
    TWO_LINE(Y);
    Y += 20;
//    }
  return true;
}
//----------------------------------------------------------------------------
int printAlarm::addRow()
{
  rowAlarmFile raf;
  if(sizeof(raf) != pfRows.P_read(&raf, sizeof(raf)))
    return 0;
  LPTSTR onOff;
  if(1 == Header.type) {
    onOff = _T("ON");
    if(0 == raf.stat)
      onOff = _T("OFF");
    else if(2 == raf.stat)
      onOff = _T("ACK");
    }
  else {
    onOff = _T("START");
    if(0 == raf.stat)
      onOff = _T("STOP");
    else if(2 == raf.stat)
      onOff = _T("ACK");
    }
  TCHAR code[20];
  _stprintf_s(code, SIZE_A(code), _T("%d-%d"), raf.prph, raf.alarm);
/*
  uint c;
  int idPrf = getPrphFromAlarm(raf.id, c);

  _stprintf_s(code, SIZE_A(code), _T("%d-%d"), idPrf, c);
  LPCTSTR descr = getStringOrIdAlarm(raf.id);
  TCHAR tD[D_DESCR_ALARM];
  clearBuff(tD, descr);
*/
  TCHAR time[30];
  set_format_data(time, SIZE_A(time), raf.ft, whichData(), _T(" - "));
  LPCTSTR all[] = {
//    onOff, code, time, tD
    onOff, code, time, raf.descr
    };

  TCHAR buff[DIM_LOCAL_BUFF];
  int result;
  int nElem = posX.getElem();
  int oldAlign = -1;
  for(int i = 0; i < nElem; ++i) {
    if(oldAlign != align[i]) {
      oldAlign = align[i];
      int al = oldAlign == 0 ? TA_LEFT : oldAlign == 1 ? TA_CENTER : TA_RIGHT;
      PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, al, TA_TOP);
      }
    PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, pageForm.left + posX[i], Y, all[i]);
    }
  Y += OFFS_Y_ROWS;
  if(Y + OFFS_Y_ROWS >= MAX_Y)
    Y = MAX_Y;
  result = pfRows.get_pos() < pfRows.get_len() ? 1 : -1;
  if(result < 0)
    Y = MAX_Y;
  return result;
}
//----------------------------------------------------------------------------
bool printAlarm::makeFooter()
{
  TCHAR buff[DIM_LOCAL_BUFF];
  TWO_LINE(Y);
  return baseClass::makeFooter();
}
//----------------------------------------------------------------------------
void printAlarm::run(bool preview, bool setup, LPCTSTR alternateBitmapOrID4Tool)
{
  LPCTSTR p = getString(idInitAlarm);
  p = findNextParam(p, 7);
  if(p) {
    int flag = _ttoi(p);
    preview = toBool(flag & 1);
    setup = toBool(flag & 2);
    }
//  baseClass::run(preview, setup, (LPCTSTR)IDB_TOOLBAR_PREVIEW2);
  baseClass::run(preview, setup, alternateBitmapOrID4Tool);
//  baseClass::run(preview, setup, _T("image\\tbarpreview3.bmp"));
}
//----------------------------------------------------------------------------
LPCTSTR printAlarm::getTitle()
{
  return Header.title;
}
