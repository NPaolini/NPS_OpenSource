//----------- P_FreePacketQueue.hpp ------------------------------------------
//----------------------------------------------------------------------------
#define CHECK_VALID
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
P_FreePacketQueue<T, maxPacket>::~P_FreePacketQueue()
{
  while(true) {
    T* t = retrieve();
    if(!t)
      break;
    release(t);
    }
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
void P_FreePacketQueue<T, maxPacket>::release(T* f)
{
#ifdef CHECK_VALID
  if(!f)
    return;
#endif
  criticalLock CL(CSF);
  FreeList.addToFree(f);
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
T* P_FreePacketQueue<T, maxPacket>::get()
{
  T* result = 0;
  do {
    criticalLock CL(CSF);
    result = (T*)FreeList.getFree();
    } while(false);
  ZeroMemory(result, sizeof(*result));
  return result;
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
void P_FreePacketQueue<T, maxPacket>::push(T* f)
{
#ifdef CHECK_VALID
  if(!f)
    return;
#endif
  do {
    criticalLock CL(CSQ);
    if(Queue.Push(f))
      f = 0;
    } while(false);
  if(f)
    release(f);
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
T* P_FreePacketQueue<T, maxPacket>::retrieve()
{
  criticalLock CL(CSQ);
  return Queue.Pop();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
T* P_FreePacketQueue<T, maxPacket>::retrieveNoRemove()
{
  criticalLock CL(CSQ);
  return Queue.getCurr();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
int P_FreePacketQueue<T, maxPacket>::getStored()
{
  criticalLock CL(CSQ);
  return Queue.getStored();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
int P_FreePacketQueue<T, maxPacket>::getFree()
{
  criticalLock CL(CSQ);
  return Queue.getFree();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
int P_FreePacketQueue<T, maxPacket>::copyTo(PVect<T*>& set)
{
  criticalLock CL(CSQ);
  return Queue.copyTo(set);
}
//----------------------------------------------------------------------------
#undef CHECK_VALID
//----------------------------------------------------------------------------
