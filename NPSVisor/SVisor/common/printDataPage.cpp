//--------------- printDataPage.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "printDataPage.h"
#include "p_file.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_date.h"
#include "newNormal.h"
//----------------------------------------------------------------------------
#define CLONE(a) a = other.a
//----------------------------------------------------------------------------
const infoPrintDataPage& infoPrintDataPage::clone(const infoPrintDataPage& other)
{
  if(&other != this) {
    delete []pageSet;
    pageSet = str_newdup(other.pageSet);
    CLONE(idPrf);
    CLONE(idAddr);
    CLONE(idType);
    CLONE(idnDec);
    CLONE(idNorm);
    }
  return *this;
}
//----------------------------------------------------------------------------
static void copyStrV(PVect<TCHAR>& t, PVect<TCHAR>& s)
{
  uint nElem = s.getElem();
  t.reset();
  for(uint i = 0; i < nElem; ++i)
    t[i] = s[i];
}
//----------------------------------------------------------------------------
static void unionSet(setOfString& target, setOfString& toAdd, uint id[])
{
  const uint N_ROW = 5;
  pvvChar t[N_ROW];
  pvvChar s[N_ROW];
  uint nElemT = splitParam(t[0], target.getString(id[0]));
  uint nElemS = splitParam(s[0], toAdd.getString(id[0]));
  uint nElem = min(nElemT, nElemS);
  for(uint i = 1; i < N_ROW; ++i) {
    splitParam(t[i], target.getString(id[i]));
    splitParam(s[i], toAdd.getString(id[i]));
    }
  for(uint i = 0; i < nElem; ++i) {
    TCHAR p[64];
    copyStrZ(p, &t[0][i], t[0][i].getElem());
    if(!_ttoi(p)) {
      for(uint j = 0; j < N_ROW; ++j)
        copyStrV(t[j][i], s[j][i]);
      }
    }
  for(uint i = nElem; i < nElemS; ++i) {
    for(uint j = 0; j < N_ROW; ++j)
      copyStrV(t[j][i], s[j][i]);
    }
  for(uint i = 0; i < N_ROW; ++i) {
    pvChar targ;
    unionParam(targ, t[i]);
    LPTSTR repl = str_newdup(&targ);
    target.replaceString(id[i], repl, true, true);
    }
}
//----------------------------------------------------------------------------
printDataPage::printDataPage(PWin* par, svPrinter* printer, LPCTSTR fileTemplate, const infoPrintDataPage& ipdp) :
    baseClass(par, printer, fileTemplate), ipdp(ipdp)
{
  Set.add(ipdp.pageSet);
  do {
    TCHAR p[_MAX_PATH] = { 0 };
    {
      setOfString set(fileTemplate);
      LPCTSTR p2 = set.getString(ID_FILE_LINKED);
      if(p2)
        _tcscpy_s(p, p2);
    }
    if(*p) {
      TCHAR t[_MAX_PATH];
      LPTSTR base = 0;
      _tcscpy_s(t, getBasePath());
      for(int i = _tcslen(t) - 1; i > 0; --i) {
        if(_T('\\') == t[i]) {
          t[i] = 0;
          base = t + i;
          appendPath(t, p);
          break;
          }
        }

      setOfString set(t);
      LPCTSTR p2 = set.getString(ID_FILE_DATA);
      if(p2) {
        if(base)
          *base = 0;
        appendPath(t, p2);
        setOfString toAdd(t);
        uint id[] = { ipdp.idPrf, ipdp.idAddr, ipdp.idType, ipdp.idnDec, ipdp.idNorm };
        unionSet(Set, toAdd, id);
        }
      }
    } while(false);
}
//----------------------------------------------------------------------------
printDataPage::~printDataPage()
{
}
//----------------------------------------------------------------------------
int printDataPage::copyToResult(LPTSTR buffRow, int lenBuff, LPCTSTR p)
{
  int len = _tcslen(p);
  int minLen = min(len, lenBuff);
  copyStr(buffRow, p, minLen);
  buffRow[minLen] = 0;
  trim(buffRow);
  return len;
}
//----------------------------------------------------------------------------
int printDataPage::manageBitText(LPTSTR buffRow, int lenBuff, int idNorm, DWDATA val, int offset)
{
  // se è >= 0 indica un solo bit, se è > zero usa testo indicato da idNorm
  if(idNorm >= 0) {
    int bit = ((val & (1 << offset)) ? 1 : 0);
    if(idNorm > 0) {
      LPCTSTR p = Set.getString(idNorm);
      if(p) {
        LPCTSTR p2 = findNextParam(p, 1);
        if(bit && p2)
          return copyToResult(buffRow, lenBuff, p2);

        TCHAR buff[200];
        int len = p2 ? p2 - p - 1 : _tcslen(p);
        copyStr(buff, p, len);
        buff[len] = 0;
        return copyToResult(buffRow, lenBuff, buff);
        }
      }
    buffRow[0] = _T('0') + bit;
    buffRow[1] = 0;
    return 1;
    }

  // minore di 0 indica una serie di bit
  idNorm = -idNorm;
  LPCTSTR p = Set.getString(idNorm);
  while(p) {
    int nBit = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    val >>= offset;
    DWDATA mask = 0xffffffff;
    mask >>= 32 - nBit;
    if(val & mask) {
      for(int i = 0; i < nBit; ++i, val >>= 1) {
        if(val & 1) {
          LPCTSTR p2 = Set.getString(idNorm + 1 + i);
          if(!p2) {
            val = 0;
            break;
            }
          return copyToResult(buffRow, lenBuff, p2);
          }
        }
      }

    if(!(val & mask))
      return copyToResult(buffRow, lenBuff, p);
    break;
    }
  *buffRow = 0;
  return 0;
}
//----------------------------------------------------------------------------
int printDataPage::manageWordText(LPTSTR buffRow, int lenBuff, int idNorm, DWDATA val, int offset)
{
  LPCTSTR p = Set.getString(idNorm);
  while(p) {
    int nBit = _ttoi(p);
    val >>= offset;
    DWDATA mask = 0xffffffff;
    if(nBit)
      mask >>= 32 - nBit;
    val &= mask;
    p = Set.getString(idNorm + 1 + val);
    if(!p)
      break;
    return copyToResult(buffRow, lenBuff, p);
    }
  *buffRow = 0;
  return 0;
}
//----------------------------------------------------------------------------
int printDataPage::getVarText(LPTSTR buffRow, int lenBuff, int idCode, int idVar)
{
  switch(idCode) {
    case pvSimpleVar:
    case pvBoxVar:
      break;
    case pvBitmapVar:
    default:
      *buffRow = 0;
      return 0;
    }

  LPCTSTR p = Set.getString(ipdp.idPrf);
  int idPrf = 0;
  if(p) {
    p = findNextParam(p, idVar);
    if(p)
      idPrf = _ttoi(p);
    }

  p = Set.getString(ipdp.idAddr);
  int Addr = 0;
  if(p) {
    p = findNextParam(p, idVar);
    if(p)
      Addr = _ttoi(p);
    }

  p = Set.getString(ipdp.idType);
  int Type = 0;
  if(p) {
    p = findNextParam(p, idVar);
    if(p)
      Type = _ttoi(p);
    }

  p = Set.getString(ipdp.idnDec);
  int nDec = 0;
  if(p) {
    p = findNextParam(p, idVar);
    if(p)
      nDec = _ttoi(p);
    }

  p = Set.getString(ipdp.idNorm);
  int idNorm = 0;
  if(p) {
    p = findNextParam(p, idVar);
    if(p)
      idNorm = _ttoi(p);
    }

  if(!idPrf || !Type) {
    *buffRow = 0;
    return 0;
    }


  prfData data;

  if(prfData::tStrData == Type)
    data.setStr(nDec);
  else if(prfData::tBitData == Type)
    data.typeVar = prfData::tDWData;
  else
    data.typeVar = Type;
  data.lAddr = Addr;

  mainClient* Par = getMain();

  genericPerif* prf = Par->getGenPerif(idPrf);

  prf->get(data);

  if(prfData::tBitData == Type)
    return manageBitText(buffRow, lenBuff, idNorm, data.U.dw, nDec);

  if(prfData::tStrData == Type) {
    int minLen = min(nDec, lenBuff);
    copyStr(buffRow, data.U.str.buff, minLen);
    buffRow[minLen] = 0;
    trim(buffRow);
    if(!*buffRow) { // per evitare di tornare come errore
      buffRow[0] = _T(' ');
      buffRow[1] = 0;
      }
    return nDec;
    }

  if(idNorm >= 0) {
    switch(Type) {
      case prfData::tDateHour:
        set_format_data(buffRow, lenBuff, data.U.ft, whichData(), _T(" - "));
        break;
      case prfData::tDate:
        set_format_data(buffRow, lenBuff, data.U.ft, whichData(), _T("|"));
        buffRow[10] = 0;
        break;
      case prfData::tHour:
        set_format_time(buffRow, lenBuff, data.U.ft);
        break;

      default:
        do {
          REALDATA val = 0;
          getNormalizedData(val, data, getNorm(idNorm));
          // non dovrebbe essere necessario il controllo, si alloca inizialmente
          // un buffer abbastanza capiente per i numeri
          makeFixedString(val, buffRow, lenBuff, nDec);
          } while(false);
        break;
      }
    return _tcslen(buffRow);
    }

  return manageWordText(buffRow, lenBuff, -idNorm, data.U.dw, nDec);
}
//----------------------------------------------------------------------------
