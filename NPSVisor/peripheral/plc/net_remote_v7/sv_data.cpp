//-------- sv_data.cpp ---------------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_data.h"
//------------------------------------------------------------------
static FreePacket Free_List(MAX_DIM_BLOCK);
FreePacket& getFreePacket() { return Free_List; }
//------------------------------------------------------------------
static FreePacket Free_List_Buff(MAX_VARS * sizeof(DWORD));
FreePacket& getFreeBuff() { return Free_List_Buff; }
//------------------------------------------------------------------
WORD header::getChkSum(LPBYTE pb, int len)
{
  WORD chk = 0;
  for(int i = 0; i < len; ++i, ++pb) {
    chk <<= 1;
    chk ^= *pb;
    }
  return chk;
}
//------------------------------------------------------------------
void handshake::makePsw(LPSTR target, LPCSTR source, LPBYTE key)
{
  cryptBufferNoZ((LPBYTE)target, (LPCBYTE)source, MAX_PASSWORD, key, DIM_KEY_SEND, 1);
}
//------------------------------------------------------------------
bool handshake::comparePsw(LPCSTR clearPsw)
{
  char t[MAX_PASSWORD];
  makePsw(t, password, key);
  return !strcmp(clearPsw, t);
}
//------------------------------------------------------------------
template <typename T>
T* get_FreeData()
{
  FreePacket& packetFree = getFreePacket();
  T* data = (T*)packetFree.get();
  return data;
}
//------------------------------------------------------------------
template <typename T>
void release_FreeData(T* packet)
{
  if(!packet)
    return;
  FreePacket& packetFree = getFreePacket();
  packetFree.release(packet);
}
//------------------------------------------------------------------
info_modified_data* getFreeData()
{
  return get_FreeData<info_modified_data>();
}
//------------------------------------------------------------------
infocmd* getFreeCmdData()
{
  return get_FreeData<infocmd>();
}
//------------------------------------------------------------------
void releaseFreeData(info_modified_data* packet)
{
  release_FreeData<info_modified_data>(packet);
}
//------------------------------------------------------------------
void releaseCmdData(infocmd* cmd)
{
  release_FreeData<infocmd>(cmd);
}
//------------------------------------------------------------------
template <typename T>
void set_FreeItem(PVect<T*>& set)
{
  FreePacket& packetFree = getFreePacket();
  uint nElem = set.getElem();
  for(uint i = 0; i < nElem; ++i)
    packetFree.release(set[i]);
  set.reset();
}
//------------------------------------------------------------------
void setFreeItem(PVect<infodata*>& set)
{
  set_FreeItem<infodata>(set);
}
//------------------------------------------------------------------
void setFreeItem(PVect<infocmd*>& set)
{
  set_FreeItem<infocmd>(set);
}
//------------------------------------------------------------------
