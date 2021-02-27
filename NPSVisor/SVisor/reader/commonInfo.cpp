//------- commonInfo.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "manage.h"
#include "commonInfo.h"
#include "p_date.h"
#include "d_infoOper.h"
#include "d_infoStop.h"
//--------------------------------------------------------------------------
static
findRec cmpTime(const vRangeTime& vRangeCause, int pos, const FILETIME& match)
{
  FILETIME ft = vRangeCause[pos].from;
  return match < ft ? frBefore :
         match > ft ? frAfter : frFound;
}
//----------------------------------------------------------------------------
int findFirstTime(const FILETIME& first, const vRangeTime& vRange)
{
  long upp = vRange.getElem() - 1;
  long mid = 0;
  long low = 0;
  findRec cmp = frBefore;
  while(low <= upp) {
    mid = DWORD(upp + low) >> 1;
    cmp = cmpTime(vRange, mid, first);
    if(cmp < 0)
      upp = mid - 1;
    else if(cmp > 0)
      low = ++mid;
    else
      break;
    }
  if(cmp <= 0) {
    while(mid > 0) {
      mid--;
      cmp = cmpTime(vRange, mid, first);
      if(cmp > 0)
        break;
      }
    }
  return(mid);
}
//----------------------------------------------------------------------------
/*
FILETIME addTime(const rangeTime& rt, const vRangeTime& vRangeCause)
{
  FILETIME ft;
  MK_I64(ft) = 0;
  int nElem = vRangeCause.getElem();
//  int i = findFirstTime(rt.from, vRangeCause);
//  if(i)
//    --i;
  for(int i = 0; i < nElem; ++i) {
//    if(overTime(rt, vRangeCause[i]))
//      break;
#if 1
    ft += intersec(vRangeCause[i], vRangeCause[i]);
#else
    FILETIME t = intersec(rt, vRangeCause[i]);
    if(MK_I64(t) > 0)
      ft += t;
#endif
    }
  return ft;
}
*/
//----------------------------------------------------------------------------
FILETIME addTime(const rangeTime& rt, const vRangeTime& vRangeCause)
{
  FILETIME ft;
  MK_I64(ft) = 0;
  int nElem = vRangeCause.getElem();
  int i = findFirstTime(rt.from, vRangeCause);
  if(i)
    --i;
  for(; i < nElem; ++i) {
    if(overTime(rt, vRangeCause[i]))
      break;
    FILETIME t = intersec(rt, vRangeCause[i]);
    if(MK_I64(t) > 0)
      ft += t;
    }
  return ft;
}
//----------------------------------------------------------------------------
void extractTime(const FILETIME& ft, int &day, int& hour, int& minute, int& second)
{
  __int64 v = cMK_I64(ft);
  v /= 10000000;
  second = static_cast<int>(v % 60);
  v /= 60;
  minute = static_cast<int>(v % 60);
  v /= 60;
  hour = static_cast<int>(v % 24);
  v /= 24;
  day = static_cast<int>(v);
}
//----------------------------------------------------------------------------
void extractString(LPTSTR result, LPCTSTR Buff, uint nBrack)
{
  int len = _tcslen(Buff);
  int j = 0;
  for(uint i = 0; i < nBrack; ++i, ++j)
    for(; j < len; ++j)
      if(_T('[') == Buff[j])
        break;
  if(j < len) {
    for(int i = 0; j < len; ++j, ++i) {
      if(_T(']') == Buff[j]) {
        result[i] = 0;
        break;
        }
      result[i] = Buff[j];
      }
    }
  len = _tcslen(result);
  for(int i = len - 1; i > 0; --i) {
    if(result[i] > _T(' '))
      break;
    result[i] = 0;
    }
}
//----------------------------------------------------------------------------
codeType getCodeOperByBuff(LPCTSTR Buff)
{
  TCHAR buff[200];
  extractString(buff, Buff, 2);
  codeOper allOper;
  return allOper.getCode(buff);
}
//----------------------------------------------------------------------------
codeType getCodeCauseByBuff(LPCTSTR Buff)
{
  TCHAR buff[40];
  extractString(buff, Buff, 1);
  return (codeType)_ttoi(buff);
}
//----------------------------------------------------------------------------
void addNodeTime(LPTSTR target, const FILETIME& from, const FILETIME& to, LPCTSTR formatTime)
{
  TCHAR time1[32];
  TCHAR time2[32];
  TCHAR time3[32];
  set_format_data(time1, SIZE_A(time1), from, whichData(), _T("-"));
  set_format_data(time2, SIZE_A(time2), to, whichData(), _T("-"));
#if 1
  FILETIME ft = to - from;
  int day;
  int hour;
  int minute;
  int second;
  extractTime(ft, day, hour, minute, second);
  wsprintf(time3, _T("%02d:%02d:%02d"), day * 24 + hour, minute, second);
#else
  set_format_time(time3, to - from);
#endif
  wsprintf(target, formatTime, time1, time2, time3);
}
//----------------------------------------------------------------------------
void runTree(int type, PWin* parent, const vCodeType& oper, const vRangeTime& date)
{
  if(0 == type)
    TD_Info_Oper(parent, oper, date).modal();
  else
    TD_Info_Stop(parent, oper, date).modal();
}
//----------------------------------------------------------------------------
void runPrint(int type, PWin* parent, const vCodeType& oper, const vRangeTime& date, bool preview, bool setup)
{
  if(0 == type)
    TD_Info_Oper(parent, oper, date).print(preview, setup);
  else
    TD_Info_Stop(parent, oper, date).print(preview, setup);
}
//----------------------------------------------------------------------------
