//----------- P_FreePacketQueue_2.hpp ------------------------------------------
//----------------------------------------------------------------------------
#define CHECK_VALID
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
void P_FreePacketQueue_2<T, maxPacket>::release(T* f)
{
  FreeList.addToFree(f);
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
T* P_FreePacketQueue_2<T, maxPacket>::get()
{
  return (T*)FreeList.getFree();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
void P_FreePacketQueue_2<T, maxPacket>::push(T* f)
{
#ifdef CHECK_VALID
  if(!f)
    return;
#endif
  if(!Queue.Push(f))
    release(f);
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
T* P_FreePacketQueue_2<T, maxPacket>::retrieve()
{
  return Queue.Pop();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
T* P_FreePacketQueue_2<T, maxPacket>::retrieveNoRemove()
{
  return Queue.getCurr();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
int P_FreePacketQueue_2<T, maxPacket>::getStored()
{
  return Queue.getStored();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
int P_FreePacketQueue_2<T, maxPacket>::getFree()
{
  return Queue.getFree();
}
//----------------------------------------------------------------------------
template <typename T, int maxPacket>
int P_FreePacketQueue_2<T, maxPacket>::copyTo(PVect<T*>& set)
{
  return Queue.copyTo(set);
}
//----------------------------------------------------------------------------
#undef CHECK_VALID
//----------------------------------------------------------------------------
