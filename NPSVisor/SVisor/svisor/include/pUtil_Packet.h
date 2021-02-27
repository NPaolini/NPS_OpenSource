//-------- pUtil_Packet.h ----------------------------------------------------
#ifndef pUtil_Packet_H_
#define pUtil_Packet_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "P_SimpleFreePacket.h"
#include "setPack.h"
//----------------------------------------------------------------------------
struct listFile
{
  TCHAR name[_MAX_PATH];
};
//----------------------------------------------------------------------------
typedef PVect<listFile*> pListFile;
//----------------------------------------------------------------------------
#if 1
inline void fz_clearFirstChar(listFile* s, size_t sz)
{
  s->name[0] = 0;
}
//----------------------------------------------------------------------------
typedef P_SimpleFreePacketClear<listFile, fz_clearFirstChar > freeListFile;
#else
template <typename T>
void fz_clearFirstChar(T* s, size_t sz)
{
  s->name[0] = 0;
}
//----------------------------------------------------------------------------
typedef P_SimpleFreePacketClear<listFile, fz_clearFirstChar<listFile> > freeListFile;
#endif
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <typename F, typename L>
void flushPV_toFree(F& FreeList, L& fList)
{
  uint nElem = fList.getElem();
  for(uint i = 0; i < nElem; ++i)
    FreeList.release(fList[i]);
  fList.reset();
}
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

