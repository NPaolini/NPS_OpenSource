//--------------- P_QUEUE.CPP -----------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "p_queue.h"
//---------------------------------------------------------------------
P_Queue::P_Queue(bool autodelete) : AutoDelete(autodelete), First(0) {  }
//---------------------------------------------------------------------
void P_Queue::Flush()
{
  while(First) {
    List *tmp = First;
    First = First->Next;
    if(AutoDelete)
      delete tmp->L;
    delete tmp;
    }
}
//---------------------------------------------------------------------
//  aggiunge un dato in coda, ritorna 1 ->ok
//  verifica l'esistenza del dato
bool P_Queue::Push(CntData f)
{
  List *tmp;
  if(!First) {
    First = new List;
    if(!First)
      return(false);
    tmp = First;
    }
  else {
    tmp = First;
    List *t = tmp;
    do {
      if(isEqual(f, t->L))
        return false;
      tmp = t;
      t = t->Next;
      } while(t);
    tmp->Next = new List;
    tmp = tmp->Next;
    }
  if(tmp) {
    tmp->Next = 0;
    tmp->L = f;
    }
  return(tmp != 0);
}
//---------------------------------------------------------------------
 // aggiunge sempre in testa senza nessun controllo di esistenza
bool P_Queue::atTop(CntData f)
{
  List *tmp = new List;
  if(!tmp)
    return false;
  tmp->Next = First;
  First = tmp;
  First->L = f;
  return true;
}
//---------------------------------------------------------------------
bool P_Queue::Pop()
{
  if(!First)
    return false;
  List *tmp = First;
  First = First->Next;
  if(AutoDelete)
    freeData(tmp->L);
  delete tmp;
  return true;
}
//---------------------------------------------------------------------
CntData P_Queue::getCurr()
{
  if(First)
    return First->L;
  return 0;
}
//---------------------------------------------------------------------
void P_Queue::freeData(CntData d) { delete d; }
//---------------------------------------------------------------------

