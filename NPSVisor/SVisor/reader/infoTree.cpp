//------- infoTree.cpp -------------------------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include <stdlib.h>
#include "infoTree.h"
#include "p_date.h"
#include "def_dir.h"
//----------------------------------------------------------
//----------------------------------------------------------
int infoFilterCode::validate(const fileCheck& check) const
{
  const codeType& filter = getData();

  // *** se il codice è di normale chiusura non viene inserito.
  // Verificare che non ci sia mai un codice operatore uguale
  // a questo codice
  if(filter && filter != check.code || CODE_NORMAL_STOP == check.code)
    return 0;

  return 1;
}
//----------------------------------------------------------
//----------------------------------------------------------
int infoFilterDate::validate(const fileCheck& check) const
{
  const rangeTime& filter = getData();

  if(cMK_I64(filter.from) && cMK_I64(filter.from) > cMK_I64(check.end))
    return 0;

  if(cMK_I64(filter.to) && cMK_I64(filter.to) < cMK_I64(check.init))
    return -1;

  return 1;
}
//----------------------------------------------------------
//----------------------------------------------------------
inline bool isEquRangeTime(const rangeTime& a, const rangeTime& b)
{
    return a.from == b.from || a.to == b.to;
}
//----------------------------------------------------------
bool infoList::addData(const fileCheck& check)
{
  rangeTime rt(check.init, check.end);
  if(find(check.code)) {
    vRangeTime& range = getRange();
    uint nElem = range.getElem();
    if(nElem) {
      const rangeTime& rt2 = range[nElem - 1];
      if(isEquRangeTime(rt2, rt))
        return true;
      }
    range[nElem] = rt;
    return true;
    }
  dataMainList* data = new dataMainList(check.code);
  data->range[0] = rt;
  return baseClass::Add(data);
}
//----------------------------------------------------------
//----------------------------------------------------------
/*
bool infoList::addCodeIf(vCodeType& code, const rangeTime& range,const vRangeTime& v) const
{
  int nElem = v.getElem();
  for(int i = 0; i < nElem; ++i) {
    const rangeTime& rt = v[i];
    if(rt & range) {
      code[code.getElem()] = getCurrCode();
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------
bool infoList::getCode(vCodeType& code, const rangeTime& range) const
{
  if(!setFirst())
    return false;
  bool added = false;
  do {
    added |= addCodeIf(code, range, getCurrRange());
    } while(setNext());
  return added;
}
*/
//----------------------------------------------------------
//----------------------------------------------------------
rangeTime manageInfo::initRangeTime()
{
  rangeTime rT;
  rT.from = getFileTimeCurr();
  int nElem = FilterDate.getElem();
  if(nElem) {
    for(int i = 0; i < nElem; ++i) {
      const rangeTime& curr = FilterDate[i].getData();
#if 1
      if(curr.from < rT.from)
        rT.from = curr.from;
      if(curr.to > rT.to)
        rT.to = curr.to;
#else
      if(cMK_I64(curr.from) < cMK_I64(rT.from))
        MK_I64(rT.from) = cMK_I64(curr.from);
      if(cMK_I64(curr.end) > cMK_I64(rT.end))
        MK_I64(rT.end) = cMK_I64(curr.end);
#endif

      }
    }
  else {
    rT.to = rT.from;
    SYSTEMTIME st;
    FileTimeToSystemTime(&rT.from, &st);
    st.wMonth = 1;
    st.wDay = 1;
    st.wHour = 0;
    st.wMinute = 0;
    st.wSecond = 0;
    SystemTimeToFileTime(&st, &rT.from);
    }
  return rT;
}
//----------------------------------------------------------
static int howManyMonth(const rangeTime& rT)
{
  SYSTEMTIME st1;
  FileTimeToSystemTime(&rT.from, &st1);

  SYSTEMTIME st2;
  FileTimeToSystemTime(&rT.to, &st2);

  int year = st2.wYear - st1.wYear;
  int month = st2.wMonth - st1.wMonth;

  return year * 12 + month + 1;
}
//----------------------------------------------------------
static bool existFile(LPCTSTR path)
{
  WIN32_FIND_DATA fd;
  HANDLE hf = FindFirstFile(path, &fd);
  if(INVALID_HANDLE_VALUE != hf) {
    FindClose(hf);
    return true;
    }
  return false;
}
//----------------------------------------------------------
int manageInfo::makeFileList(PVect<LPTSTR>& fileList, const rangeTime& rT)
{
  TCHAR path[_MAX_PATH] = {0};
  FILETIME ft = rT.from;
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  int added = 0;
  int nMonth = howManyMonth(rT);
  for(int i = 0; i < nMonth; ++i) {
    fillStr(path, 0, SIZE_A(path));
    _tcscpy_s(path, SIZE_A(path), prefixName);
    st = makeYearAndMonth(path + 2, st, -1);
    makePath(path, dExRep, false);
    if(existFile(path)) {
      fileList[added] = str_newdup(path);
      ++added;
      }
    }
  fillStr(path, 0, SIZE_A(path));
  _tcscpy_s(path, SIZE_A(path), prefixName);
  _tcscat_s(path, SIZE_A(path), SUFFIX_NAME_TEMP);
  makePath(path, dExRep, false);
  if(existFile(path)) {
    fileList[added] = str_newdup(path);
    ++added;
    }

  return added;
}
//----------------------------------------------------------
int manageInfo::accept(const fileCheck& fCk)
{
  if(!fCk.code)
    return 0;
  int nElem = FilterDate.getElem();
  bool outOfDate = nElem ? true : false;
  bool isOk = nElem ? false : true;

  for(int i = 0; i < nElem && !isOk; ++i) {
    int result = FilterDate[i].validate(fCk);
    isOk |= result > 0;
    outOfDate &= result < 0;
    }
  if(!isOk)
    return outOfDate ? -1 : 0;

#if 0
  return 1;
#else
  nElem = FilterCode.getElem();
  isOk = nElem ? false : true;

  for(int i = 0; i < nElem && !isOk; ++i)
    isOk |= FilterCode[i].validate(fCk) > 0;

  return isOk ? 1 : 0;
#endif
}
//----------------------------------------------------------
bool intersec(fileCheck& fCk, const rangeTime& rT)
{
  if(fCk.init < rT.from)
    fCk.init = rT.from;

  if(fCk.end > rT.to)
    fCk.end  = rT.to;
  return fCk.init < fCk.end;
}
//----------------------------------------------------------
bool manageInfo::perform()
{
  rangeTime rT = initRangeTime();
  PVect<LPTSTR> fileList;
  int nFile = makeFileList(fileList, rT);
  if(!nFile)
    return false;
  for(int i = 0; i < nFile; ++i) {
    P_File f(fileList[i], P_READ_ONLY);
    if(!f.P_open())
      continue;
    // eventualmente da implementare per migliorare la performance
    // in caso di inizio lettura da file non all'inizio del file
    // setFirstPos(f);
    fileCheck fCk;
    do {
      if(sizeof(fCk) != f.P_read(&fCk, sizeof(fCk)))
        break;

      int result = accept(fCk);

      if(result > 0) {
        if(intersec(fCk, rT))
          if(!Info.addData(fCk))
            break;
        }
     // il filtro ha verificato che è inutile continuare
     // la data è ormai fuori range
      else if(result < 0)
        break;
      } while(true);
    }
  for(int i = 0; i < nFile; ++i)
    delete []fileList[i];
  return true;
}
//----------------------------------------------------------
//----------------------------------------------------------
