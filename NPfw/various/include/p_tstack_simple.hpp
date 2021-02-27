//--------------- p_tstack_simple.hpp ---------------------------------
//---------------------------------------------------------------------
template <typename T, int Sz>
P_TStackSimple<T, Sz>::P_TStackSimple() : Added(0) { }
//----------------------------------------------------------------------
template <typename T, int Sz>
bool P_TStackSimple<T, Sz>::Push(T f)
{
  if(Added == Sz)
    return false;
  Set[Added] = f;
  ++Added;
  return true;
}
//----------------------------------------------------------------------
template <typename T, int Sz>
T P_TStackSimple<T, Sz>::Pop()
{
  if(!Added)
    return 0;
  --Added;
  return Set[Added];
}
//----------------------------------------------------------------------
template <typename T, int Sz>
T P_TStackSimple<T, Sz>::getCurr()
{
  if(!Added)
    return 0;
  return Set[Added - 1];
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------

