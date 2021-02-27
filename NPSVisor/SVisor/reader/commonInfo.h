//------- commonInfo.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef COMMONINFO_H_
#define COMMONINFO_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "d_info.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
enum findRec { frBefore = -1, frFound, frAfter };
//--------------------------------------------------------------------------
findRec cmpDay(const vRangeTime& vRangeCause, int pos, int day);
//----------------------------------------------------------------------------
int findFirstTime(const FILETIME& first, const vRangeTime& vRange);
//----------------------------------------------------------------------------
inline
FILETIME minTime(const FILETIME& ft1, const FILETIME& ft2)
{
  return cMK_I64(ft1) > cMK_I64(ft2) ? ft2 : ft1;
}
//----------------------------------------------------------------------------
inline
FILETIME maxTime(const FILETIME& ft1, const FILETIME& ft2)
{
  return cMK_I64(ft1) < cMK_I64(ft2) ? ft2 : ft1;
}
//----------------------------------------------------------------------------
inline
FILETIME intersec(const rangeTime& rt, const rangeTime& range)
{
  return minTime(rt.to, range.to) - maxTime(rt.from, range.from);
}
//----------------------------------------------------------------------------
FILETIME addTime(const rangeTime& rt, const vRangeTime& vRangeCause);
//----------------------------------------------------------------------------
void extractTime(const FILETIME& ft, int &day, int& hour, int& minute, int& second);
//----------------------------------------------------------------------------
codeType getCodeOperByBuff(LPCTSTR Buff);
//----------------------------------------------------------------------------
codeType getCodeCauseByBuff(LPCTSTR Buff);
//----------------------------------------------------------------------------
void extractString(LPTSTR result, LPCTSTR Buff, uint nBrack);
//----------------------------------------------------------------------------
void addNodeTime(LPTSTR target, const FILETIME& from, const FILETIME& to, LPCTSTR formatTime);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
bool overTime(const rangeTime& based, const rangeTime& test)
{
  return based.to < test.from;
}
//----------------------------------------------------------------------------
#endif
