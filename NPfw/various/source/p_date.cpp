//------------- p_date.cpp ------------------------------------
#include "precHeader.h"
//--------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "p_date.h"
#include "p_file.h"
//--------------------------------------------------------------
// la data può essere sia gg-mm-aa che gg-mm-aaaa, con un qualsiasi
// separatore non numerico, deve essere una stringa ASCIIZ
//--------------------------------------------------------------
static const int GN = 31;
static const int FB = 28;
static const int MZ = 31;
static const int AP = 30;
static const int MG = 31;
static const int GG = 30;
static const int LG = 31;
static const int AG = 31;
static const int ST = 30;
static const int OT = 31;
static const int NV = 30;
static const int DC = 31;

static const int M_1  = GN;
static const int M_2  = (M_1  + FB);
static const int M_3  = (M_2  + MZ);
static const int M_4  = (M_3  + AP);
static const int M_5  = (M_4  + MG);
static const int M_6  = (M_5  + GG);
static const int M_7  = (M_6  + LG);
static const int M_8  = (M_7  + AG);
static const int M_9  = (M_8  + ST);
static const int M_10 = (M_9  + OT);
static const int M_11 = (M_10 + NV);
static const int M_12 = (M_11 + DC);
//--------------------------------------------------------------
static const int totDay[] =
{   0, M_1, M_2, M_3, M_4, M_5, M_6, M_7, M_8, M_9, M_10, M_11, M_12  };
//--------------------------------------------------------------
static const int gmese[12] =
{   GN, FB, MZ, AP, MG, GG, LG, AG, ST, OT, NV, DC };
//--------------------------------------------------------------
static void div_365(long d,int *a,int *g);
static int sub_bis(int g,int a);
//----------------------------------------------------------------
static void inline div_10(int &v, int &r)
{
  r = v % 10;
  v /= 10;
}
//----------------------------------------------------------------
// verificare se esportare i dati su file in UNICODE
void exportDay(long day, P_File &t, eDataFormat format, LPCTSTR tab)
{
  TCHAR date[24];
  l2d(day, date, format);
  _tcscat_s(date, SIZE_A(date), tab);
  t.P_writeString(date);
}
//--------------------------------------------------------------
void extrYear(LPCTSTR File, LPTSTR buff, size_t lenBuff)
{
  int l = (int)_tcslen(File) - 1;
  while(l > 4) {
    if(_T('.') == File[l])
      break;
    --l;
    }
  if(l > 4) {
    l -= 4;
    _tcsncpy_s(buff, lenBuff, File + l, 4);
    buff[4] = 0;
    }
  else
    _tcsncpy_s(buff, lenBuff, _T("0000"), 5);
}
//--------------------------------------------------------------
int isLeap(int anno)
{
  if(anno & 3)    // non è divisibile per quattro
    return 0;
  if((anno % 100) != 0)  // è divisibile per quattro ma non per cento
    return 1;
  anno /= 100;
  if(anno & 3)  // è divisibile per cento ma non per quattrocento
    return 0;
  return 1;
}
//----------------------------------------------------------------
long getDay(SYSTEMTIME* tm)
{
  SYSTEMTIME stm;
  if(!tm) {
    GetLocalTime(&stm);
    tm = &stm;
    }
  long day = tm->wDay - 1; // zero based
  day += totDay[tm->wMonth-1];
  if(tm->wMonth > 2)
    day += isLeap(tm->wYear);
  return day;
}
//----------------------------------------------------------------
int getLastDay(SYSTEMTIME* tm)
{
  SYSTEMTIME stm;
  if(!tm) {
    GetLocalTime(&stm);
    tm = &stm;
    }
  if(2 == tm->wMonth && isLeap(tm->wYear))
    return FB + 1;
  return gmese[tm->wMonth-1];
}
//----------------------------------------------------------------
long d2l(LPCTSTR date, eDataFormat format)
{
  if(_tcslen(date) < 8)
    return 0;
  LPCTSTR pmonth;
  LPCTSTR pyear;
  LPCTSTR pday;
  switch(format) {
    case European:
      pday = date;
      pmonth = date + 3;
      pyear = date + 6;
      break;
    case American:
      pday = date + 3;
      pmonth = date;
      pyear = date + 6;
      break;
    case Other:
      pday = date + 8;
      pmonth = date + 5;
      pyear = date;
      break;
    default:
      return 0;
    }
  long day = _ttol(pday);
  if(!day)
    return 0;
  int month = _ttoi(pmonth);
  if(!month)
    return 0;
  int year = _ttoi(pyear);
  if(year < 80)
    year += 2000;
  else if(year < 100)
    year += 1900;
  int year_div_cento = year / 100;
  day += totDay[month - 1] + (year >> 2) - year_div_cento +
          (year_div_cento >> 2) + ((long)(year - 1) * 365);
  if(month < 3)
    day -= isLeap(year);
  return(day);
}
//----------------------------------------------------------------
LPTSTR l2d(long d, LPTSTR s, size_t lenBuff, eDataFormat format)
{
  int a;
  int g;
  div_365(d, &a, &g);
  g = sub_bis(g, a);
  while(g < 0) {
    g += 365 + isLeap(a);
    --a;
    }
  int m;
  for(m = 1; m < 12; ++m)
    if(g <= totDay[m])
      break;
  g -= totDay[m - 1];
  if(m <= 2 && isLeap(a)) {
    g++;
    if(m == 1 && g > gmese[m-1]) {
      g = 1;
      m = 2;
      }
    }
  if(!g)  {
    --a;
    m = 12;
    g = 31;
    }
  int t;
  switch(format) {
    case European:
      break;
    case American:
      t = m;
      m = g;
      g = t;
      break;
    case Other:
      t = g;
      g = a;
      a = t;
      break;
    default:
      return 0;
    }
  _stprintf_s(s, lenBuff, _T("%02d/%02d/%02d"), g, m, a);
  return(s);
}
//---------------------------------------------------------------
//----------------------------------------------------------------
void div_365(long d,int *a,int *g)
{
  long t;
#if 1
  t = d % 365;
  d /= 365;
#else
  d /= 365;
  t = _EDX;
#endif
  *g = t;
  *a = d + 1;
}
//----------------------------------------------------------------
int sub_bis(int g,int a)
{
//  g -= (a >> 2) - (a / 100) + (a / 400);
  int a_div_100 = a / 100;
  g -= (a >> 2) - a_div_100 + (a_div_100 >> 2);
  return(g);
}
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
void set_format_data(LPTSTR buff, size_t lenBuff, const SYSTEMTIME& stm, eDataFormat format, LPCTSTR tab)
{
  int year;
  int month;
  int day;
  switch(format) {
    case American:
      day = stm.wMonth;
      month = stm.wDay;
      year = stm.wYear;
      break;
    case Other:
      day = stm.wYear;
      month = stm.wMonth;
      year = stm.wDay;
      break;
    default:
    case European:
      day = stm.wDay;
      month = stm.wMonth;
      year = stm.wYear;
      break;
    }
  _stprintf_s(buff, lenBuff, _T("%02d/%02d/%02d%s%02d:%02d:%02d"),
              day, month, year, tab,
              stm.wHour, stm.wMinute, stm.wSecond);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void set_format_time(LPTSTR buff, size_t lenBuff, const FILETIME& ftm)
{
  FILETIME p = ftm;
  __int64 v = (reinterpret_cast<LARGE_INTEGER*>(&p))->QuadPart;
  v /= 10000000;
  int sec = static_cast<int>(v % 60);
  v /= 60;
  int minute = static_cast<int>(v % 60);
  v /= 60;
  _stprintf_s(buff, lenBuff, _T("%d:%02d:%02d"), int(v), minute, sec);
}
//----------------------------------------------------------------
void set_format_data_full(LPTSTR buff, size_t lenBuff, const SYSTEMTIME& stm, eDataFormat format, LPCTSTR tab)
{
  int year;
  int month;
  int day;
  switch(format) {
    case American:
      day = stm.wMonth;
      month = stm.wDay;
      year = stm.wYear;
      break;
    case Other:
      day = stm.wYear;
      month = stm.wMonth;
      year = stm.wDay;
      break;
    default:
    case European:
      day = stm.wDay;
      month = stm.wMonth;
      year = stm.wYear;
      break;
    }
  _stprintf_s(buff, lenBuff, _T("%02d/%02d/%02d%s%02d:%02d:%02d,%03d"),
              day, month, year, tab,
              stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void set_format_time_full(LPTSTR buff, size_t lenBuff, const FILETIME& ftm)
{
  FILETIME p = ftm;
  __int64 v = (reinterpret_cast<LARGE_INTEGER*>(&p))->QuadPart;
  v /= 10000;
  int msec = static_cast<int>(v % 1000);
  v /= 1000;
  int sec = static_cast<int>(v % 60);
  v /= 60;
  int minute = static_cast<int>(v % 60);
  v /= 60;
  v %= 24;
  _stprintf_s(buff, lenBuff, _T("%d:%02d:%02d,%03d"), int(v), minute, sec, msec);
}
//--------------------------------------------------------------
void makeYear(LPTSTR name, int subYear)
{
  SYSTEMTIME stm;
  GetLocalTime(&stm);
  stm.wYear -= WORD(subYear);
  makeYear(name, stm);
}
//--------------------------------------------------------------
void makeYear(LPTSTR name, const SYSTEMTIME& stm)
{
  TCHAR year[6];
  _itot_s(stm.wYear, year, SIZE_A(year), 10);
  _tcsncpy_s(name, 5, year, 4);
}
//--------------------------------------------------------------
void makeYearAndMonth(LPTSTR name, int subMonth)
{
  SYSTEMTIME stm;
  GetLocalTime(&stm);
  stm.wMonth -= WORD(subMonth);
  while((short)(stm.wMonth) <= 0) {
    stm.wMonth += WORD(12);
    --stm.wYear;
    }
  while((short)(stm.wMonth) > 12) {
    stm.wMonth -= WORD(12);
    ++stm.wYear;
    }
  makeYearAndMonth(name, stm);
}
//--------------------------------------------------------------
void makeYearAndMonth(LPTSTR name, const SYSTEMTIME& stm)
{
  _itot_s(stm.wYear, name, 5, 10);
  name[4] = static_cast<TCHAR>(stm.wMonth / 10 + _T('0'));
  name[5] = static_cast<TCHAR>(stm.wMonth % 10 + _T('0'));
  name[6] = 0;
}
//--------------------------------------------------------------
SYSTEMTIME makeYearAndMonth(LPTSTR name, const SYSTEMTIME& st, int subMonth)
{
  SYSTEMTIME stm = st;
  makeYearAndMonth(name, stm);
  stm.wMonth -= WORD(subMonth);
  while((short)(stm.wMonth) <= 0) {
    stm.wMonth += WORD(12);
    --stm.wYear;
    }
  while((short)(stm.wMonth) > 12) {
    stm.wMonth -= WORD(12);
    ++stm.wYear;
    }
//  makeYearAndMonth(name, stm);
  return stm;
}
//--------------------------------------------------------------
void makeYearMonthAndDay(LPTSTR name)
{
  SYSTEMTIME stm;
  GetLocalTime(&stm);
  makeYearMonthAndDay(name, stm);
}
//--------------------------------------------------------------
void makeYearMonthAndDay(LPTSTR name, const SYSTEMTIME& stm)
{
  makeYearAndMonth(name, stm);
  name[6] = static_cast<TCHAR>(stm.wDay / 10 + _T('0'));
  name[7] = static_cast<TCHAR>(stm.wDay % 10 + _T('0'));
  name[8] = 0;
}
//--------------------------------------------------------------
FILETIME getFileTimeCurr()
{
  SYSTEMTIME st;
  FILETIME ft;
  GetLocalTime(&st);
  SystemTimeToFileTime(&st, &ft);
  return ft;
}
//--------------------------------------------------------------
FILETIME getFileTimeUTC()
{
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  return ft;
}
//----------------------------------------------------------------
static WORD get_mmsec(LPTSTR next)
{
  LPTSTR t = next;
  while(*next && !_istdigit((unsigned)*next))
    ++next;
  WORD wMilliseconds;
  if(next - t <= 2) {
    wMilliseconds = (WORD)_tcstol(next, &next, 10);
    if(wMilliseconds > 999)
      wMilliseconds = 0;
    }
  else
    wMilliseconds = 0;
  return wMilliseconds;
}
//----------------------------------------------------------------
void unformat_data(LPCTSTR buff, SYSTEMTIME& stm, eDataFormat format)
{
  LPTSTR next;

  stm.wDay = (WORD)_tcstol(buff, &next, 10);

  while(*next && !_istdigit((unsigned)*next))
    ++next;
  buff = next;
  stm.wMonth = (WORD)_tcstol(buff, &next, 10);

  while(*next && !_istdigit((unsigned)*next))
    ++next;
  buff = next;
  stm.wYear = (WORD)_tcstol(buff, &next, 10);

  while(*next && !_istdigit((unsigned)*next))
    ++next;
  buff = next;
  stm.wHour = (WORD)_tcstol(buff, &next, 10);

  while(*next && !_istdigit((unsigned)*next))
    ++next;
  buff = next;
  stm.wMinute = (WORD)_tcstol(buff, &next, 10);

  while(*next && !_istdigit((unsigned)*next))
    ++next;
  buff = next;
  stm.wSecond = (WORD)_tcstol(buff, &next, 10);
  stm.wMilliseconds = get_mmsec(next);
  
  switch(format) {
    case American:
      do {
        WORD t = stm.wMonth;
        stm.wMonth = stm.wDay;
        stm.wDay = t;
        } while(false);
      break;
    case Other:
      do {
        WORD t = stm.wYear;
        stm.wYear = stm.wDay;
        stm.wDay = t;
        } while(false);
      break;
    }
}
//--------------------------------------------------------------
//----------------------------------------------------------------
void unformat_time(LPCTSTR buff, SYSTEMTIME& stm)
{
  LPTSTR next;
  FILETIME ft;
  (reinterpret_cast<LARGE_INTEGER*>(&ft))->QuadPart = 0;
  FileTimeToSystemTime(&ft, &stm);

  stm.wHour = (WORD)_tcstol(buff, &next, 10);

  while(stm.wHour >= 24) {
    ++stm.wDay;
    stm.wHour -= 24;
    }

  while(*next && !_istdigit((unsigned)*next))
    ++next;
  buff = next;
  stm.wMinute = (WORD)_tcstol(buff, &next, 10);

  while(*next && !_istdigit((unsigned)*next))
    ++next;
  buff = next;
  stm.wSecond = (WORD)_tcstol(buff, &next, 10);
  stm.wMilliseconds = get_mmsec(next);
}

