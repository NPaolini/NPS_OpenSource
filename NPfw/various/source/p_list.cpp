//----------------- P_LIST.CPP --------------------------------
#include "precHeader.h"
//-------------------------------------------------------------
#include "p_list.h"
//-------------------------------------------------------------
void P_List_Gen::Flush()
{
  while(First) {
    Curr = First;
    First = First->Next;
    if(AutoDelete)
      freeData(Curr->L);
    delete Curr;
    }
  Curr = 0;
}
//-------------------------------------------------------------
//  aggiunge un dato in coda, ritorna 1 ->ok
bool P_List_Gen::atEnd(ListData f)
{
  if(!First) {
    First = new List;
    if(!First)
      return false;
    Curr = First;
    Curr->Next = 0;
    }
  else {
    if(!Curr)
      Curr = First;

    while(Curr->Next)
      Curr = Curr->Next;

    Curr->Next = new List;

    if(Curr->Next != NULL) {
      Curr->Next->Prev = Curr;
      Curr = Curr->Next;
      }
    else
      return false;
    }
  Curr->Next = 0;
  Curr->L = f;
  return true;
}
//-------------------------------------------------------------
bool P_List_Gen::Add(ListData f) // aggiunge sempre in testa
{
  if(!First) {
    First = new List;
    if(!First)
      return false;
    Curr = First;
    Curr->Next = 0;
    }
  else {
    Curr = new List;
    if(Curr) {
      Curr->Next =  First;
      First->Prev = Curr;
      First = Curr;
      }
    else {
      Curr = First;
      return false;
      }
    }
  Curr->Prev = 0;
  Curr->L = f;
  return true;
}
//-------------------------------------------------------------
bool P_List_Gen::insert(ListData f, bool before)
{
  if(!First || (before && Curr == First))
    return Add(f);

  List* tmp = new List;
  if(!tmp)
    return false;
  if(before) {
    Curr->Prev->Next = tmp;
    tmp->Prev = Curr->Prev;
    tmp->Next = Curr;
    Curr->Prev = tmp;
    }
  else {
    if(Curr->Next)
      Curr->Next->Prev = tmp;
    tmp->Next = Curr->Next;
    Curr->Next = tmp;
    tmp->Prev = Curr;
    }
  tmp->L = f;
  return true;
}
//-------------------------------------------------------------
bool P_List_Gen::insert(ListData toAdd, bool before, pList nearTo)
{
  // presuppone che nearTo sia valido, altrimenti da dove viene?
  List* tmp = new List;
  if(!tmp)
    return false;

  if(before) {
    if(nearTo->Prev)
      nearTo->Prev->Next = tmp;
    tmp->Prev = nearTo->Prev;
    tmp->Next = nearTo;
    nearTo->Prev = tmp;
    }
  else {
    if(nearTo->Next)
      nearTo->Next->Prev = tmp;
    tmp->Next = nearTo->Next;
    nearTo->Next = tmp;
    tmp->Prev = nearTo;
    }
  tmp->L = toAdd;
  return true;
}
//-------------------------------------------------------------
ListData P_List_Gen::Rem(void)
{
  if(!Curr)
    return 0;
  List *tmp = Curr;
  ListData f = Curr->L;
  if(Curr->Prev) {
    Curr->Prev->Next = Curr->Next;
    if(Curr->Next)
      Curr->Next->Prev = Curr->Prev;
    Curr = Curr->Prev;
    }
  else {
    if(Curr->Next)
      Curr->Next->Prev = 0;
    Curr = Curr->Next;
    First = Curr;
    }
  delete tmp;
  return f;
}
//-------------------------------------------------------------
bool P_List_Gen::Search(ListData f)
{
  if(!First)
    return false;
  if(!Curr)
    Curr = First;
  List *t = Curr;
  do {
    if(!Cmp(f, t->L))
      break;
    t = t->Next;
    } while(t);
  if(t) {
    Curr = t;
    return true;
    }
  return false;
}
//-------------------------------------------------------------
P_List_Gen::pList P_List_Gen::getCurrL()
{
  return Curr;
}
//-------------------------------------------------------------
void P_List_Gen::setCurrL(pList curr)
{
  Curr = curr;
}
//-------------------------------------------------------------
bool P_List_Gen::setFirst()
{
  Curr = First;
  return First != 0;
}
//-------------------------------------------------------------
bool P_List_Gen::setLast()
{
  if(!Curr)
    Curr = First;
  if(!Curr)
    return false;
  while(Curr->Next)
    Curr = Curr->Next;
  return true;
}
//-------------------------------------------------------------
bool P_List_Gen::setNext()
{
  if(Curr && Curr->Next) {
    Curr = Curr->Next;
    return true;
    }
  return false;
}
//-------------------------------------------------------------
bool P_List_Gen::setPrev()
{
  if(Curr && Curr->Prev) {
    Curr = Curr->Prev;
    return true;
    }
  return false;
}
//-------------------------------------------------------------
P_List_Gen::pList P_List_Gen::getFirstL()
{
  return First;
}
//-------------------------------------------------------------
P_List_Gen::pList P_List_Gen::getLastL()
{
  pList curr = Curr;
  if(!curr)
    curr = First;
  if(!curr)
    return 0;
  while(curr->Next)
    curr = curr->Next;
  return curr;
}
//-------------------------------------------------------------
P_List_Gen::pList P_List_Gen::getNext(pList curr)
{
  if(curr && curr->Next)
    return curr->Next;
  return 0;
}
//-------------------------------------------------------------
P_List_Gen::pList P_List_Gen::getPrev(pList curr)
{
  if(curr && curr->Prev)
    return curr->Prev;
  return 0;
}
//-------------------------------------------------------------
bool P_List_Gen::levelUp()
{
  if(Curr && Curr->Prev) {
    List *t = Curr->Prev;
    if(t->Prev)
      t->Prev->Next = Curr;
    else
      First = Curr;
    Curr->Prev = t->Prev;
    t->Next = Curr->Next;
    if(Curr->Next)
      Curr->Next->Prev = t;
    Curr->Next = t;
    t->Prev = Curr;
    return true;
    }
  return false;
}
//-------------------------------------------------------------
bool P_List_Gen::levelDown()
{
  if(Curr && Curr->Next) {
    List *t=Curr->Next;
    if(t->Next)
      t->Next->Prev = Curr;
    Curr->Next = t->Next;
    t->Prev = Curr->Prev;
    if(Curr->Prev)
      Curr->Prev->Next = t;
    else
      First = t;
    Curr->Prev = t;
    t->Next = Curr;
    return true;
    }
  return false;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
bool P_List_Queue::Add(ListData f) // aggiunge sempre in fondo
{
  if(!First) {
    First = new List;
    if(!First)
      return false;
    Curr = First;
    Curr->Prev = 0;
    }
  else {
    if(!Curr)
      Curr =  First;
    while(Curr->Next)
      Curr = Curr->Next;
    List *t = new List;
    if(t) {
      Curr->Next = t;
      t->Prev = Curr;
      Curr = t;
      }
    else
      return false;
    }
  Curr->Next = 0;
  Curr->L = f;
  return true;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
bool P_List_Ord::Search(ListData f)
{
  if(!First)
    return false;
  if(!Curr)
    Curr = First;
  List *t = Curr;
  int different = Cmp(f, t->L);
  if(different < 0) {
    while(t->Prev && different < 0) {
      t = t->Prev;
      different = Cmp(f, t->L);
      }
    }
  else if(different > 0) {
    while(t->Next && different > 0) {
      t = t->Next;
      different = Cmp(f, t->L);
      }
    }
  if(!different) {
    Curr = t;
    return true;
    }
  return false;
}
//-------------------------------------------------------------
#define BEFORE 0
#define AFTER  1
//-------------------------------------------------------------
void P_List_Ord::search4add(ListData f)
{
  if(!Curr)
    Curr = First;
  Where = BEFORE;
  if(Cmp(f, Curr->L) < 0) {
    do {
      if(Cmp(f, Curr->L) >= 0)
        break;
      Curr = Curr->Prev;
      } while(Curr);
    if(Curr)
      Curr = Curr->Next;
    }
  else {
    do {
      if(Cmp(f, Curr->L) <= 0)
        break;
      Curr = Curr->Next;
      } while(Curr);
    if(!Curr)
      Where = AFTER;
    }
}
//-------------------------------------------------------------
bool P_List_Ord::Add(ListData f)
{
  if(!First) {         // la lista è vuota
    First = new List;
    if(!First)
      return false;
    Curr = First;
    Curr->Next = 0;
    Curr->Prev = 0;
    }
  else {
    List *tmp = Curr;  // memorizza nel caso venga inserito in fondo
          // per evitare di scorrere tutta la lista
    search4add(f);  // nella ricerca il nuovo nodo va inserito prima di Curr
        // se Curr è NULL il nuovo nodo è il primo o l'ultimo e viene
        // stabilito da Where
    List *t = new List;
    if(!t)
      return false;
    if(!Curr) {
      if(Where == BEFORE) {    // prima di First
        Curr =  t;
        Curr->Prev = 0;
        Curr->Next = First;
        First->Prev = Curr;
        First = Curr;
        }
      else {                 // dopo l'ultimo
        if(tmp)
          Curr = tmp;
        else
          Curr = First;
        while(Curr->Next)
          Curr = Curr->Next;
        Curr->Next = t;
        t->Next = 0;
        t->Prev = Curr;
        Curr = t;
        }
      }
    else {
      t->Prev = Curr->Prev;
      if(t->Prev)
        t->Prev->Next = t;
      t->Next = Curr;
      Curr->Prev = t;
      Curr =  t;
      if(!Curr->Prev)
        First = Curr;
      }
    }
  Curr->L = f;
  return true;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
