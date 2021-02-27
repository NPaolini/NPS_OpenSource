//-------- sv_data.h -----------------------------------------------
//------------------------------------------------------------------
#ifndef sv_data_H_
#define sv_data_H_
//------------------------------------------------------------------
#include "precHeader.h"
#include "defgPerif.h"
#include "sv_baseServer.h"
#include "PCrt_lck.h"
#include "p_vect.h"
#include "p_util.h"
#include "P_FreePacketQueue.h"
//---------------------------------------------------------
#include "s7_libnodave_common.h"
//------------------------------------------------------------------
#define MAX_DIM_BLOCK     sizeof(dataSend)
#define MAX_BUFF_DATA     MAX_DIM_BLOCK
//------------------------------------------------------------------
class my_freeList_cs : public P_freeList
{
  private:
    typedef P_freeList baseClass;
  public:
    my_freeList_cs(size_t size) : baseClass(size) { }
    LPVOID getFree() { criticalLock crtLck(cs); return baseClass::getFree(); }
    void addToFree(LPVOID data) { criticalLock crtLck(cs);  baseClass::addToFree(data); }

    LPVOID get() { return getFree(); }
    void release(LPVOID data) { addToFree(data); }
  private:
    criticalSect cs;
};
//------------------------------------------------------------------
typedef my_freeList_cs FreePacket;
//------------------------------------------------------------------
extern FreePacket& getFreePacket();
//------------------------------------------------------------------
extern dataSend* getFreeData();
extern void releaseFreeData(dataSend* packet);
//------------------------------------------------------------------
#endif
