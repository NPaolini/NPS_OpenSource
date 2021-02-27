//--------------- prf_theremino_v7.CPP -------------------
#include "precHeader.h"
//---------------------------------------------------------
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>

#include "pwin.h"

#include "hdrmsg.h"
#include "prfdata.h"
#include "prph_theremino_v7.h"
#include "file_lck.h"
#include "commidt.h"
#include "p_txt.h"
#include "p_util.h"
//---------------------------------------------------------
gestComm *allocGestComm(void* par, uint idPrf)
{
  PWin * w = reinterpret_cast<PWin*>(par);
  if(w) {
    TCHAR buff[100];
    wsprintf(buff, _T("Theremino Device [%d] V7"), idPrf);
    w->setCaption(buff);
    }
  TCHAR name[_MAX_PATH];
  MK_NAME_FILE_DATA(name, idPrf);
  return new theremino_Driver(name, FLAG_PERIF(idPrf), idPrf);
}
//-----------------------------------------------------
//-----------------------------------------------------
theremino_Driver::theremino_Driver(LPCTSTR file, WORD flagCommand, uint idPrf) :
    baseClass(file, flagCommand) {   }
//-----------------------------------------------------
theremino_Driver::~theremino_Driver()
{
}
//-----------------------------------------------------
#define pMM_DEFAULT _T("Theremino1")
#define pMM_SIZE 4080
//-------------------------------------------------------
gestComm::ComResult theremino_Driver::Init()
{
  pMM.P_open(pMM_DEFAULT, pMM_SIZE);
  gestComm::ComResult result = baseClass::Init();

  return result;
}
//-------------------------------------------------------
gestComm::ComResult theremino_Driver::Send(const addrToComm* , Perif_TypeCommand )
{
  return gestComm::NoErr;
}
//-------------------------------------------------------
gestComm::ComResult theremino_Driver::SendData(addrToComm* pAddr)
{
#if 1
  const uint size = sizeof(float);
#else
  uint size = prfData::getNByte((prfData::tData)(pAddr->type));
  if(!size)
    size = getTypeSize();
#endif
  if(!pMM.getDim()) {
    if(!pMM.P_open(pMM_DEFAULT, pMM_SIZE))
      return gestComm::noAction;
    }
#if 1
  memcpy_s((LPBYTE)pMM.getAddr() + pAddr->addr, pMM_SIZE - pAddr->addr, pAddr->buff, pAddr->len * size);
#else
  tWrapMappedMemory<DWORD> wDWmm(pMM);
  LPDWORD pdw = (LPDWORD)pAddr->buff;

  uint pos = pAddr->addr / size;
  memcpy_s(wDWmm + pos, pMM_SIZE - pAddr->addr)
  for(uint i = 0; i < pAddr->len; ++i, ++pos) {
    wDWmm[pos] = pdw[i];
    }
#endif
  return gestComm::NoErr;
}
//-------------------------------------------------------
gestComm::ComResult theremino_Driver::Receive(const addrToComm* pAddr, BDATA* buff)
{
#if 1
  const uint size = sizeof(float);
#else
  uint size = prfData::getNByte((prfData::tData)(pAddr->type));
  if(!size)
    size = getTypeSize();
#endif
  if(!pMM.getDim()) {
    if(!pMM.P_open(pMM_DEFAULT, pMM_SIZE))
      return gestComm::noAction;
    }
#if 1
  memcpy_s(buff, pAddr->len * size, (LPBYTE)pMM.getAddr() + pAddr->addr, pAddr->len * size);
#else
  tWrapMappedMemory<DWORD> wDWmm(pMM);
  LPDWORD pdw = (LPDWORD)buff;

  uint pos = pAddr->addr / size;
  for(uint i = 0; i < pAddr->len; ++i, ++pos) {
    pdw[i] = wDWmm[pos];
    }
#endif
  return gestComm::NoErr;
}
//-----------------------------------------------------

