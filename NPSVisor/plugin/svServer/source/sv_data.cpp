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
void handshake::makePsw(LPSTR target, LPCSTR source, LPCBYTE key)
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
static
infodata* getFreeItem(DWORD addr, DWORD value)
{
  FreePacket& packetFree = getFreePacket();
  infodata* item = (infodata*)packetFree.get();
  item->addr = addr;
  item->numdata = 1;
  item->data[0] = value;
  return item;
}
//------------------------------------------------------------------
info_modified_data* getFreeData()
{
  FreePacket& packetFree = getFreePacket();
  info_modified_data* data = (info_modified_data*)packetFree.get();
  return data;
}
//------------------------------------------------------------------
void releaseFreeData(info_modified_data* packet)
{
  if(!packet)
    return;
  FreePacket& packetFree = getFreePacket();
  packetFree.release(packet);
}
//------------------------------------------------------------------
static
void initializeData(info_modified_data* data)
{
  data->Head = header(header::hVarChg);
  data->numinfodata = 0;
}
//------------------------------------------------------------------
static
void finalizeData(info_modified_data* data, uint dim)
{
  data->Head.Len = dim - sizeof(data->Head);
  data->Head.makeChkSum();
}
//------------------------------------------------------------------
// non è necessario eseguire un inserimento ordinato, i dati arrivano
// già ordinati ed è sufficiente aggiungerli in coda
// e conviene già eseguire il controllo di consecutività
static
void addToSend(uint addr, DWORD value, PVect<infodata*>& set)
{
  uint nElem = set.getElem();
  if(nElem) {
    infodata* item = set[nElem - 1];
    if(item->addr + item->numdata == addr) {
      if(item->numdata < MAX_ITEM) {
        item->data[item->numdata] = value;
        ++item->numdata;
        return;
        }
      }
    }
  infodata* item = getFreeItem(addr, value);
  set[nElem] = item;

}
//------------------------------------------------------------------
extern DWORD cryptDWord(DWORD s, DWORD key, uint ix);
//------------------------------------------------------------------
static
bool addInfo(info_modified_data* data, const infodata* toAdd, LPBYTE& lastPtr, LPCBYTE key)
{
  infodata* curr = (infodata*)lastPtr;
  uint onAdd = sizeof(*toAdd) + sizeof(DWORD) * (toAdd->numdata - 1);
  if(((LPBYTE)curr - (LPBYTE)data) + onAdd >= MAX_BUFF_DATA)
    return false;
  *curr = *toAdd;
  cryptBufferNoZ((LPBYTE)curr->data, (LPCBYTE)(toAdd->data), toAdd->numdata * sizeof(DWORD), key, DIM_KEY_SEND, 1);
  lastPtr = (LPBYTE)curr + sizeof(*toAdd) + sizeof(DWORD) * (toAdd->numdata - 1);
/*
  curr->data[0] = cryptDWord(curr->data[0], key, 0);
  lastPtr = (LPBYTE)curr + sizeof(*toAdd);
  for(uint i = 1; i < toAdd->numdata; ++i) {
    curr->data[i] = cryptDWord(toAdd->data[i], key, i);
    lastPtr += sizeof(DWORD);
    }
*/
  ++data->numinfodata;
  return true;
}
//------------------------------------------------------------------
static
uint fillData(info_modified_data* data, PVect<infodata*>& set, uint lastpos, LPBYTE& lastPtr, LPCBYTE key)
{
  uint i = lastpos;
  uint nElem = set.getElem();
  for(; i < nElem; ++i) {
    if(!addInfo(data, set[i], lastPtr, key))
      break;
    }
  return i;
}
//------------------------------------------------------------------
static
void setFreeData(info_modified_data* data)
{
  FreePacket& packetFree = getFreePacket();
  packetFree.release(data);
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
manageReadingData::manageReadingData() : firstCmp(1)
{
  ZeroMemory(oldBuff, sizeof(oldBuff));
}
//------------------------------------------------------------------
manageReadingData::~manageReadingData()
{
  setFreeItem(Set);
}
//------------------------------------------------------------------
bool manageReadingData::sendModifiedData(LPDWORD buff)
{
  for(uint i = 0; i < MAX_VARS; ++i) {
    if(firstCmp > 0 || buff[i] != oldBuff[i]) {
      FreePacket& fp = getFreeBuff();
      LPDWORD ib = (LPDWORD)fp.get();
      memcpy(ib, buff, sizeof(oldBuff));
      if(!buffSet.Push(ib))
        fp.release(ib);
      return true;
      }
    }
  return false;
}
//------------------------------------------------------------------
void manageReadingData::sendModifiedData(PConnBase* conn, LPCBYTE key)
{
  if(!buffSet.getStored())
    return;
  if(firstCmp)
    firstCmp = -1;
  FreePacket& fp = getFreeBuff();
  info_modified_data* data = getFreeData();
  while(true) {
    LPDWORD buff = buffSet.Pop();
    if(!buff)
      break;
    for(uint i = 0; i < MAX_VARS; ++i) {
      if(firstCmp || buff[i] != oldBuff[i]) {
        oldBuff[i] = buff[i];
        addToSend(i, buff[i], Set);
        }
      }
    fp.release(buff);
    uint nElem = Set.getElem();
    if(!nElem)
      continue;

    uint lastpos = 0;
    while(lastpos < nElem) {
      LPBYTE lastPtr = (LPBYTE)data->data;
      initializeData(data);
      lastpos = fillData(data, Set, lastpos, lastPtr, key);
      uint dimBlock = lastPtr - (LPBYTE)data;
      finalizeData(data, dimBlock);
      conn->write_string((LPBYTE)data, dimBlock);
      }
    setFreeItem(Set);
    }
  setFreeData(data);
  firstCmp = 0;
}
//------------------------------------------------------------------
