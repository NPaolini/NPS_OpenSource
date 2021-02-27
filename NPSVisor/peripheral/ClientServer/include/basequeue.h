//----------- basequeue.h ------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef BASEQUEUE_H_
#define BASEQUEUE_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "p_util.h"
//-----------------------------------------------------------------------------
class msgBaseQueue;
class buffBaseQueue;
//-----------------------------------------------------------------------------
#include "PCommonData.h"
//-----------------------------------------------------------------------------
#include "p_tqueue.h"
#define MSG_BASE_QUEUE_CLASS P_TQueue<pMsgBaseData>
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class msgBaseQueue : public MSG_BASE_QUEUE_CLASS
{
  public:
    msgBaseQueue() : MSG_BASE_QUEUE_CLASS(false) { }

    pMsgBaseData getCurrData() { return getCurr(); }
    pMsgBaseData popData();

  protected:

		typedef pMsgBaseData pDataType;
    // se il dato è già nella coda non viene reinserito
    virtual bool isEqual(const pDataType newData, const pDataType listData);
};
//-----------------------------------------------------------------------------
#endif
