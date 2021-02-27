//--------------- p_tqueue_simpleCS.hpp -------------------------------
//---------------------------------------------------------------------
template <typename T, int Sz>
bool P_TQueueSimpleCS<T, Sz>::Push(T f)
{
  criticalLock CL(CSQ);
  return Queue.Push(f);
}
//----------------------------------------------------------------------
template <typename T, int Sz>
T P_TQueueSimpleCS<T, Sz>::Pop()
{
  criticalLock CL(CSQ);
  return Queue.Pop();
}
//----------------------------------------------------------------------
template <typename T, int Sz>
T P_TQueueSimpleCS<T, Sz>::getCurr()
{
  criticalLock CL(CSQ);
  return Queue.getCurr();
}
//----------------------------------------------------------------------
template <typename T, int Sz>
int P_TQueueSimpleCS<T, Sz>::getStored()
{
  criticalLock CL(CSQ);
  return Queue.getStored();
}
//----------------------------------------------------------------------
template <typename T, int Sz>
int P_TQueueSimpleCS<T, Sz>::getFree()
{
  criticalLock CL(CSQ);
  return Queue.getFree();
}
//----------------------------------------------------------------------
template <typename T, int Sz>
int P_TQueueSimpleCS<T, Sz>::copyTo(PVect<T>& set)
{
  criticalLock CL(CSQ);
  return Queue.copyTo(set);

}
//----------------------------------------------------------------------

