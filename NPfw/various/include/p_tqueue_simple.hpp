//--------------- p_tqueue_simple.hpp ---------------------------------
//---------------------------------------------------------------------
template <typename T, int Sz>
P_TQueueSimple<T, Sz>::P_TQueueSimple() : Head(0), Tail(0) { }
//----------------------------------------------------------------------
template <typename T, int Sz>
bool P_TQueueSimple<T, Sz>::Push(T f)
{
  if(Head == Tail - 1 || Head == Sz - 1 && !Tail)
    return false;
  Set[Head] = f;
  if(++Head >= Sz)
    Head -= Sz;
  return true;
}
//----------------------------------------------------------------------
template <typename T, int Sz>
T P_TQueueSimple<T, Sz>::Pop()
{
  if(Tail == Head)
    return 0;
  T b = Set[Tail];
  if(++Tail >= Sz)
    Tail -= Sz;
  return b;
}
//----------------------------------------------------------------------
template <typename T, int Sz>
T P_TQueueSimple<T, Sz>::getCurr()
{
  if(Tail == Head)
    return 0;
  return Set[Tail];
}
//----------------------------------------------------------------------
template <typename T, int Sz>
int P_TQueueSimple<T, Sz>::getStored()
{
  int t = Head - Tail;
  return t < 0 ? Sz + t : t;
}
//----------------------------------------------------------------------
template <typename T, int Sz>
int P_TQueueSimple<T, Sz>::getFree()
{
  int t = Tail - Head;
  return t <= 0 ? Sz + t - 1 : t - 1;
}
//----------------------------------------------------------------------
template <typename T, int Sz>
int P_TQueueSimple<T, Sz>::copyTo(PVect<T>& set)
{
  int stored = getStored();
  if(!stored)
    return 0;
  int nElem = set.getElem();
  set.setDim(stored + nElem);
  for(int i = 0; i < stored; ++i, ++nElem)
    set[nElem] = Set[(i + Tail) % Sz];
  return stored;
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------

