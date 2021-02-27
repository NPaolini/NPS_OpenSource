//--------------- P_TQUEUE.HPP ----------------------------------------
//---------------------------------------------------------------------
template <class T, bool isArray>
P_TQueue<T, isArray>::P_TQueue(bool autodelete)
  : AutoDelete(autodelete), First(0), Last(0), FreeList(sizeof(List)) {  }
//----------------------------------------------------------------------
template <class T, bool isArray>
void P_TQueue<T, isArray>::Flush()
{
  while(First) {
    List *tmp = First;
    First = First->Next;
    if(AutoDelete)
      if(isArray)
        delete []tmp->L;
      else
        delete tmp->L;
    FreeList.addToFree(tmp);
    }
  Last = 0;
}
//----------------------------------------------------------------------
//  aggiunge un dato in coda
template <class T, bool isArray>
bool P_TQueue<T, isArray>::PushAlways(T f)
{
  List *tmp = (List*)FreeList.getFree();
  if(!tmp)
    return(false);
  if(!Last) {
    Last = tmp;
    First = Last;
    }
  else {
    Last->Next = tmp;
    Last = tmp;
    }
  tmp->Next = 0;
  tmp->L = f;
  return true;
}
//----------------------------------------------------------------------
//  aggiunge un dato in coda
template <class T, bool isArray>
bool P_TQueue<T, isArray>::Push(T f)
{
  List *tmp = First;
  while(tmp) {
    if(isEqual(f, tmp->L))
      return false;
    tmp = tmp->Next;
    }
  return PushAlways(f);
}
//----------------------------------------------------------------------
 // aggiunge in testa
template <class T, bool isArray>
bool P_TQueue<T, isArray>::atTop(T f)
{
  List *tmp = (List*)FreeList.getFree();
  if(!tmp)
    return false;
  tmp->Next = First;
  First = tmp;
  First->L = f;
  if(!Last)
    Last = First;
  return true;
}
//----------------------------------------------------------------------
template <class T, bool isArray>
bool P_TQueue<T, isArray>::Pop()
{
  if(!First)
    return false;
  List *tmp = First;
  First = First->Next;
  if(!First)
    Last = 0;
  if(AutoDelete)
    freeData(tmp->L);
  FreeList.addToFree(tmp);
  return true;
}
//----------------------------------------------------------------------
template <class T, bool isArray>
T P_TQueue<T, isArray>::getCurr()
{
  if(First)
    return First->L;
  return 0;
}
//----------------------------------------------------------------------
template <class T, bool isArray>
bool P_TQueue<T, isArray>::getFirst(LPVOID& curr, T& value)
{
  if(!First)
    return false;
  value = First->L;
  curr = (LPVOID)(First->Next);
  return true;
}
//----------------------------------------------------------------------
template <class T, bool isArray>
bool P_TQueue<T, isArray>::getNext(LPVOID& curr, T& value)
{
  if(!First || !curr)
    return false;
  List* c = (List*)curr;
  value = c->L;
  curr = (LPVOID)(c->Next);
  return true;
}
//----------------------------------------------------------------------
template <class T, bool isArray>
uint P_TQueue<T, isArray>::getElem() const
{
  uint nElem = 0;
  List *tmp = First;
  while(tmp) {
    ++nElem;
    tmp = tmp->Next;
    }
  return nElem;
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------

