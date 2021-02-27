//------------------ manageSetupPrph.cpp --------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "p_txt.h"
#include "manageSetupPrph.h"
//-----------------------------------------------------------------------------
void var4Setup::copyFrom(const alternAddress& aa, uint l_addr)
{
  lAddr = l_addr;
  ipAddr = aa.ipAddr;
  port = aa.port;
  db = aa.db;
  addr = aa.addr;
  dataType = aa.dataType;
}
//-----------------------------------------------------------------------------
#define F_ACT(a,v) if(sizeof(v) != pf.P_##a(&v, sizeof(v))) return false
//-----------------------------------------------------------------------------
#define COMMON_CODE(a) \
    F_ACT(a,ipAddr);\
    F_ACT(a,port);\
    F_ACT(a,db);\
    F_ACT(a,addr);\
    F_ACT(a,dataType);\
    F_ACT(a,dwVal);\
    return true
//-----------------------------------------------------------------------------
bool var4Setup::read(P_File& pf)
{
  COMMON_CODE(read);
}
//-----------------------------------------------------------------------------
bool var4Setup::write(P_File& pf)
{
  COMMON_CODE(write);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
manageSetupPrph::manageSetupPrph(gestCommgPerif* owner, uint id) :
    Owner(owner), Id(id), readCount(0), commitCount(-1), oVS(true),
    needSave(false), countDelaySave(0)

{}
//-----------------------------------------------------------------------------
manageSetupPrph::~manageSetupPrph()
{
  if(needSave)
    saveFile();
}
//-----------------------------------------------------------------------------
bool manageSetupPrph::loadSet(setOfString& set)
{
  if(set.setFirst()) {

    alternAddress tmpAddr;

    const int nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;

    do {
      uint ix = set.getCurrId();
      if(ix >= nElem)
        continue;
      LPCTSTR p = set.getCurrString();
      stringToAddr(1, 1, p, &tmpAddr);
      if(tmpAddr.action & gestCommgPerif::VAR_4_SETUP) {
        var4Setup v4s;
        v4s.copyFrom(tmpAddr, ix);
        oVS.Add(new var4Setup(v4s));
        ++readCount;
        }
      } while(set.setNext());
    }
  set.setFirst();
  return oVS.setFirst();
}
//-----------------------------------------------------------------------------
static void makeFilename(LPTSTR file, uint id)
{
  wsprintf(file, _T("prph_%d_setup.dat"), id);
}
//-----------------------------------------------------------------------------
bool manageSetupPrph::loadFile()
{
  commitCount = 0;
  if(!oVS.setFirst())
    return false;
  TCHAR file[_MAX_PATH];
  makeFilename(file, Id);
  P_File pf(file, P_READ_ONLY);
  if(!pf.P_open()) {
  needSave = true;
    return false;
  }
  while(true) {
    var4Setup v4s;
    if(!v4s.read(pf))
      break;
    if(oVS.Search(&v4s)) {
      var4Setup* pv4s = oVS.getCurrData();
      if(pv4s->dwVal != v4s.dwVal) {
        pv4s->dwVal = v4s.dwVal;
        pv4s->committed = false;
        ++commitCount;
        }
      }
    }
  return true;
}
//-----------------------------------------------------------------------------
bool manageSetupPrph::saveFile()
{
  countDelaySave = 0;
  needSave = false;
  if(!oVS.setFirst())
    return false;
  TCHAR file[_MAX_PATH];
  makeFilename(file, Id);
  do {
    P_File pf(file);
    pf.reback();
    } while(false);

  P_File pf(file, P_CREAT);
  if(!pf.P_open()) {
    pf.unback();
    return false;
    }
  do {
    var4Setup* pv4s = oVS.getCurrData();
    if(!pv4s->write(pf))
      break;
    } while(oVS.setNext());
  return true;
}
//-----------------------------------------------------------------------------
bool manageSetupPrph::getValToSend(PVect<transf4Send>& set)
{
  if(!oVS.setFirst())
    return false;
  uint nEl = set.getElem();
  do {
    var4Setup* pv4s = oVS.getCurrData();
    if(!pv4s->committed) {
      --commitCount;
      pv4s->committed = true;
      set[nEl].addr = pv4s->lAddr - 1;
      set[nEl].value = pv4s->dwVal;
      ++nEl;
      }
    } while(commitCount && oVS.setNext());
  saveFile();
  return true;
}
//-----------------------------------------------------------------------------
#define MAX_COUNT_DELAY_SAVE 100
//-----------------------------------------------------------------------------
void manageSetupPrph::checkReadedVar(LPBYTE buff, uint size, const addrToComm* atc, uint offs)
{
  var4Setup v4s(*atc);
  v4s.addr += offs;
  if(oVS.Search(&v4s)) {
    var4Setup* pv4s = oVS.getCurrData();
    if(!pv4s->readed) {
      memcpy(&pv4s->dwVal, buff, size);
      --readCount;
      pv4s->readed = true;
      }
    }
  if(needSave) {
    if(++countDelaySave >= MAX_COUNT_DELAY_SAVE)
      saveFile();
    }
}
//-----------------------------------------------------------------------------
void manageSetupPrph::refreshWrite(addrToComm* pAddr)
{
  var4Setup v4s(*pAddr);
  uint size = GET_NBYTE(pAddr->type);
  LPBYTE buff = pAddr->buff;
  bool changed = false;
  for(uint i = 0; i < pAddr->len; ++i, ++v4s.addr, buff += size) {
    if(oVS.Search(&v4s)) {
      var4Setup* pv4s = oVS.getCurrData();
      if(memcmp(&pv4s->dwVal, buff, size)) {
        memcpy(&pv4s->dwVal, buff, size);
        changed = true;
        }
      }
    }
  if(changed) {
    needSave = true;
    if(++countDelaySave >= MAX_COUNT_DELAY_SAVE)
      saveFile();
    }
}
