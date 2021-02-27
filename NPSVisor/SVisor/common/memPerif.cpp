//-------- memPerif.cpp ---------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "memPerif.h"
#include "log_stat.h"
#include "file_lck.h"
#include "p_base.h"
#include "mainclient.h"
#include "p_util.h"
#include "def_dir.h"
#include "p_freeList.h"

#include "1.h"
//-------------------------------------------------------------
typedef PVect<int> pVectInt;
//-----------------------------------------------------------
extern void fillSetWithRange(pVectInt& v, LPCTSTR p);
//-------------------------------------------------------------
static P_freeList tmpBuff(MAX_BDATA_PERIF);
//---------------------------------------------------------------------
extern bool useMappedFilePrph1();
//-----------------------------------------------------------------
#define WAIT_RELOAD 2
//-------------------------------------------------------------
memPerif::memPerif(mainClient* par, uint id) : baseClass(par, id + 1000),
    BitsNoCommit(MAX_BDATA_PERIF), errFileMapped(false), onLocalUseAll(false), needReload(WAIT_RELOAD),
    needSave(false)
{
  useSendBit = 0;
  Ready = true;
  typeAddress = prfData::tDWData;
  trueDimBuff = MAX_BDATA_PERIF;
  if(isReadOnly()) {
    LPCTSTR p = getString(ID_ADDR_ALWAYS_LOCALPRPH_MEM + id - WM_FIRST_EXT_ONMEM);
    if(p) {
      if(-1 == _ttoi(p))
        onLocalUseAll = true;
      else
        fillSetWithRange(vNoReload, p);
      }
    }
  openMapping(!isReadOnly() && useMappedFilePrph1());
}
//-----------------------------------------------------------------
memPerif::~memPerif()
{
}
//-----------------------------------------------------------------
void memPerif::makePath(LPTSTR path, uint sz, bool local)
{
  if(local) {
    _tcscpy_s(path, sz, getLocalBaseDir());
    if(*path) {
      appendPath(path, 0);
      LPTSTR p = path + _tcslen(path);
      MK_NAME_FILE_DATA(p, getId());
      }
    else
      MK_NAME_FILE_DATA(path, getId());
    }
  else {
    MK_NAME_FILE_DATA(path, getId());
    getBasePath(path);
    }
}
//-----------------------------------------------------------------
bool memPerif::loadFile(LPTSTR path, LPBYTE buff)
{
  P_File f(path, P_READ_ONLY);
  for(uint i = 0; i < 2; ++i) {
    bool success = false;
    if(f.P_open()&& f.get_len() > 0) {
      f.P_read(buff, trueDimBuff);
      return true;
      }
    Sleep(100);
    }
  return false;
}
//-----------------------------------------------------------------
void memPerif::refresh()
{
  if(!isReadOnly()) {
    baseClass::refresh();
    needRefresh = false;
    return;
    }
  criticalLock cl(cSectSet);
  if(++needReload < WAIT_RELOAD || needSave)
    return;
    // se non c'è stato un set() da qualche pagina
  needReload = 0;

  LPBYTE tmp = (LPBYTE)tmpBuff.getFree();
  bool online = false;
  do {
    TCHAR path[_MAX_PATH];
    makePath(path, SIZE_A(path), onLocalUseAll);
    online = loadFile(path, tmp);
    } while(false);

  if(online) {
    if(!onLocalUseAll) {
      int nElem = vNoReload.getElem();
      if(nElem) {
        LPBYTE tmp2 = (LPBYTE)tmpBuff.getFree();
        do {
          TCHAR path[_MAX_PATH];
          makePath(path, SIZE_A(path), true);
          online = loadFile(path, tmp2);
          } while(false);
        if(online) {
          LPDWORD pdw = (LPDWORD)tmp;
          LPDWORD pcurr = (LPDWORD)tmp2;
          for(int i = 0; i < nElem; ++i) {
            int ix = vNoReload[i];
            pdw[ix] = pcurr[ix];
            }
          }
        tmpBuff.addToFree(tmp2);
        }
      }
    memcpy(currBuff, tmp, trueDimBuff);
    }
  tmpBuff.addToFree(tmp);
}
//-----------------------------------------------------------------
void memPerif::openMapping(bool useFileMapped)
{
  if(useFileMapped) {
    if(!pMF) {
      TCHAR name[_MAX_PATH];
      MK_NAME_FILE_DATA(name, getId());
      getBasePath(name);
      pMF = new p_MappedFile(name);
      if(!pMF->P_open(trueDimBuff)) {
        errFileMapped = true;
        delete []currBuff;
        currBuff = 0;
        openMapping(false);
        return;
        }
      delete []currBuff;
      currBuff = (BDATA*)pMF->getAddr();
      }
    // non dovrebbe verificarsi, se si chiude il driver viene chiuso anche il mapping
    else { // è necessario riaprirlo?
      delete []currBuff;
      currBuff = 0;
      delete pMF;
      pMF = 0;
      openMapping(useFileMapped);
      }
    }
  else { // !useFileMapped
    if(pMF) {
      delete pMF;
      pMF = 0;
      currBuff = new BDATA[trueDimBuff];
      needRefresh = true;
      refresh();
      }
    }
}
//-----------------------------------------------------------
#define NBITS_DWORD (8 * sizeof(DWORD))
//-----------------------------------------------------
void memPerif::commit()
{
  criticalLock cl(cSectSet);
  if(!pMF && (needRefresh || memcmp(currBuff, oldBuff, trueDimBuff))) {
    TCHAR path[_MAX_PATH];
    makePath(path, SIZE_A(path), isReadOnly());

    P_File fl(path, P_CREAT);
    if(fl.P_open())
      fl.P_write(currBuff, trueDimBuff);
    needSave = false;
    }

  const uint szBuff = sizeof(oldBuff);

  LPDWORD tmp = (LPDWORD)tmpBuff.getFree();

  if(BitsNoCommit.hasSet())
    memcpy(tmp, oldBuff, szBuff);

  baseClass::commit();
  if(BitsNoCommit.hasSet()) {
    DWORD ndw = BitsNoCommit.getNumDWORD();
    LPDWORD pOldDw = (LPDWORD)oldBuff;
    for(uint i = 0; i < ndw; ++i) {
      if(BitsNoCommit.isDWordSet(i)) {
        DWORD ix = i * NBITS_DWORD;
        for(int j = 0; j < NBITS_DWORD; ++j, ++ix) {
          if(BitsNoCommit.isSet(ix))
            pOldDw[ix] = tmp[ix];
          }
        }
      }
    BitsNoCommit.clearAll();
    }
  tmpBuff.addToFree(tmp);
}
//---------------------------------------------------------------------
prfData::tResultData memPerif::getset(prfData& data, bool noCommit)
{
  criticalLock cl(cSectSet);
  prfData data2 = data;
  get(data);
  prfData::tResultData result = set(data2, noCommit);
  return result;
}
//---------------------------------------------------------------------
prfData::tResultData memPerif::set(const prfData& data, bool noCommit)
{
  criticalLock cl(cSectSet);
  prfData::tResultData result = prfData::invalidType;
  needSave = true;

  prfData::tData type = (prfData::tData)(data.typeVar & ~prfData::tNegative);
  DWDATA* dwBuff = (DWDATA*)currBuff;
  DWDATA lowTarget = dwBuff[data.lAddr];
  DWDATA highTarget = dwBuff[data.lAddr + 1];

  switch(type) {
    case prfData::tBitData:
      do {
        WORD offAndBit = LOWORD(data.U.dw);
        DWORD val = HIWORD(data.U.dw);
        BYTE nBit = LOBYTE(offAndBit);
        BYTE offs = HIBYTE(offAndBit);

        DWDATA mask = 0xffffffff >> (32 - nBit);
        mask <<= offs;

        lowTarget = dwBuff[data.lAddr];
        if(data.typeVar & prfData::tNegative)
          val = ~val;
        val <<= offs;
        val &= mask;
        mask = ~mask;
        lowTarget &= mask;
        lowTarget |= val;
        result = prfData::okData;
        } while(false);
      break;

    case prfData::tBData:
    case prfData::tBsData:
      lowTarget &= ~0xff;
      lowTarget |= data.U.b;
      result = prfData::okData;
      break;

    case prfData::tWData:
    case prfData::tWsData:
      lowTarget &= ~0xffff;
      lowTarget |= data.U.w;
      result = prfData::okData;
      break;

    case prfData::tFRData:
    case prfData::tDWData:
    case prfData::tDWsData:
      lowTarget = data.U.dw;
      result = prfData::okData;
      break;

    case prfData::tRData:   // 64 bit float
    case prfData::ti64Data: // 64 bit
    case prfData::tDateHour:// 64 bit per data + ora
    case prfData::tDate:    // 64 bit per data
    case prfData::tHour:    // 64 bit per ora
      lowTarget = data.U.li.u.LowPart;
      highTarget = data.U.li.u.HighPart;
      result = prfData::okData;
      break;

    case prfData::tStrData:
      do {
        int len = data.U.str.len  + data.lAddr * sizeof(dwBuff[0]);
        if(len > MAX_BDATA_PERIF)
          break;

        DWDATA* curr = dwBuff + data.lAddr;
        memcpy(curr, data.U.str.buff, data.U.str.len);
        if(noCommit) {
          len = data.U.str.len / sizeof(DWDATA);
          if(data.U.str.len % sizeof(DWDATA))
            ++len;
          for(int i = 0; i < len; ++i)
            BitsNoCommit.set(data.lAddr + i);
          }
        return prfData::okData;
        } while(false);
      break;

    default:
      break;
    }
  if(prfData::invalidType != result) {
    dwBuff[data.lAddr] = lowTarget;
    dwBuff[data.lAddr + 1] = highTarget;
    if(noCommit) {
      BitsNoCommit.set(data.lAddr);
      switch(type) {
        case prfData::tRData:   // 64 bit float
        case prfData::ti64Data: // 64 bit
        case prfData::tDateHour:// 64 bit per data + ora
        case prfData::tDate:    // 64 bit per data
        case prfData::tHour:    // 64 bit per ora
          BitsNoCommit.set(data.lAddr + 1);
          break;
        }
      }
    }
  return result;
}
//---------------------------------------------------------------------
prfData::tResultData memPerif::multiSet(const prfData* data, int num, bool, sorted_multi_set)
{
  prfData::tResultData result = prfData::okData;
  for(int i = 0; i < num; ++i) {
    prfData::tResultData res = set(data[i]);
    if(prfData::okData != res)
      result = res;
    }
  return result;
}
//----------------------------------------------------------------------------
