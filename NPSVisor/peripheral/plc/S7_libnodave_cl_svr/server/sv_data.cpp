//-------- sv_data.cpp ---------------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_data.h"
//------------------------------------------------------------------
static FreePacket Free_List(MAX_DIM_BLOCK);
FreePacket& getFreePacket() { return Free_List; }
//------------------------------------------------------------------
dataSend* getFreeData()
{
  FreePacket& packetFree = getFreePacket();
  dataSend* data = (dataSend*)packetFree.get();
  return data;
}
//------------------------------------------------------------------
void releaseFreeData(dataSend* packet)
{
  if(!packet)
    return;
  FreePacket& packetFree = getFreePacket();
  packetFree.release(packet);
}
//------------------------------------------------------------------
