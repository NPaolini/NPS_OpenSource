//-------------------- P_Tree.cpp -----------------------------------------
//-------------------------------------------------------------------------
#include "precHeader.h"
#include "p_tree.h"
//-------------------------------------------------------------------------
bool P_Tree::setFirst() const
{
  if(!Radix)
    Curr = 0;
  else {
    Curr = Radix;
    set_first(Curr->Sx);
    }
  return Curr != 0;
}
//-------------------------------------------------------------------------
void P_Tree::set_first(pNode nodo) const
{
  if(nodo) {
    while(nodo->Sx)
      nodo = nodo->Sx;
    Curr = nodo;
    }
}
//-------------------------------------------------------------------------
bool P_Tree::setLast() const
{
  if(!Radix)
    Curr = 0;
  else {
    Curr = Radix;
    set_last(Curr->Dx);
    }
  return Curr != 0;
}
//-------------------------------------------------------------------------
void P_Tree::set_last(pNode nodo) const
{
  if(nodo) {
    while(nodo->Dx)
      nodo = nodo->Dx;
   Curr = nodo;
    }
}
//-------------------------------------------------------------------------
P_Tree::pNode P_Tree::getFirst() const
{
  if(!Radix)
    return 0;
  pNode nodo = Radix;
  while(nodo->Sx)
    nodo = nodo->Sx;
  return nodo;
}
//-------------------------------------------------------------------------
P_Tree::pNode P_Tree::getLast() const
{
  if(!Radix)
    return 0;
  pNode nodo = Radix;
  while(nodo->Dx)
    nodo = nodo->Dx;
  return nodo;
}
//-------------------------------------------------------------------------
P_Tree::pNode P_Tree::getNext(pNode curr) const
{
  if(!curr)
    return 0;
  if(curr->Dx) {      // se c'è un nodo di destra, torna il più a sinistra
    curr = curr->Dx;
    while(curr->Sx)
      curr = curr->Sx;
    return curr;
    }
  if(curr->Parent) {  // altrimenti, se c'è il genitore, risale l'albero
    do {
      if(curr->Parent->Sx == curr)  // se proviene da Sx ha trovato il nodo cercato
        return curr->Parent;
      curr = curr->Parent;             // altrimenti continua a salire
      } while(curr->Parent);   // finché non arriva alla radice
    }
  return 0;             // e quindi non ci sono nodi a destra
}
//-------------------------------------------------------------------------
P_Tree::pNode P_Tree::getPrev(pNode curr) const
{
  if(!curr)
    return 0;
  if(curr->Sx) {      // se c'è un nodo di sinistra, torna il più a destra
    curr = curr->Sx;
    while(curr->Dx)
      curr = curr->Dx;
    return curr;
    }
  if(curr->Parent) {  // altrimenti, se c'è il genitore, risale l'albero
    do {
      if(curr->Parent->Dx == curr)  // se proviene da Dx ha trovato il nodo cercato
        return curr->Parent;
      curr = curr->Parent;             // altrimenti continua a salire
      } while(curr->Parent);   // finché non arriva alla radice
    }
  return 0;             // e quindi non ci sono nodi a destra
}
//-------------------------------------------------------------
P_Tree::pNode P_Tree::getCurrN()
{
  return Curr;
}
//-------------------------------------------------------------
void P_Tree::setCurrN(pNode curr)
{
  Curr = curr;
}
//-------------------------------------------------------------------------
//----------- RITORNA IL PRIMO NODO A DESTRA DI [CURR] --------------------
bool P_Tree::setNext() const
{
  if(!Radix) {         // albero vuoto
    Curr = 0;
    return false;
    }
  if(!Curr) {         // probabilmente c'è qualche errore
    setLast();       // setta all'ultimo
    return false;        // e ritorna
    }
  pNode t = Curr;
  if(t->Dx) {      // se c'è un nodo di destra, setta il più a sinistra
    set_first(t->Dx); // di quel nodo
    return true;         // e ritorna successo
    }
  if(t->Parent) {  // altrimenti, se c'è il genitore, risale l'albero
    do {
      if(t->Parent->Sx == t) { // se proviene da Sx
        Curr = t->Parent;        // ha trovato il nodo cercato
        return true;
        }
      t = t->Parent;             // altrimenti continua a salire
      } while(t->Parent);   // finché non arriva alla radice
    }
  return false;             // e quindi non ci sono nodi a destra
}
//-------------------------------------------------------------------------
//----------- RITORNA IL PRIMO NODO A SINISTRA DI [CURR] ------------------
bool P_Tree::setPrev() const
{
  if(!Radix) {         // albero vuoto
    Curr = 0;
    return false;
    }
  if(!Curr) {
    setFirst();
    return false;
    }
  pNode t = Curr;
  if(t->Sx) {      // se c'è un nodo di sinistra, setta il più a destra
    set_last(t->Sx); // di quel nodo
    return true;
    }
  if(t->Parent) {  // altrimenti, se c'è il genitore, risale l'albero
    do {
      if(t->Parent->Dx == t) { // se proviene da Dx
        Curr = t->Parent;      // ha trovato il nodo cercato
        return true;
        }
      t = t->Parent;           // altrimenti continua a salire
      } while(t->Parent);      // finché non arriva alla radice
    }
  return false;                // e quindi non ci sono nodi a sinistra
}
//-------------------------------------------------------------------------
//--------- ROUTINE DI RICERCA DEL NODO [A] -------------------------------
bool P_Tree::search(pNode nodo, TreeData a) const
{
  int cmp = 0;
  while(nodo) {
    Curr = nodo;
    cmp = Cmp(a, nodo->D);
    if(!cmp)  // trovato
      return true;
    if(cmp < 0)         // se il valore é minore
      nodo = nodo->Sx;  // cerca nel nodo Sx
    else
      nodo = nodo->Dx;  // altrimenti cerca nel nodo Dx
    }
  // il nodo corrente deve essere sempre minore (se non trovato)
  if(cmp < 0)
    setPrev();
  return false;
}
//-------------------------------------------------------------------------
void P_Tree::Flush()
{
  freeNode(Radix);
  Radix = 0;
  Curr = 0;
  NumElem = 0;
}
//-------------------------------------------------------------------------
#ifdef CHECK_LEAK
  void remNode(LPVOID t);
#else
#define remNode(t)
#endif
//-------------------------------------------------------------------------
void P_Tree::freeNode(pNode a)
{
  if(!a)               // tappo della ricorsione
    return;
  freeNode(a->Sx);     // libera il ramo sinistro
  freeNode(a->Dx);     // libera il ramo destro
  if(autoDelete)
    freeData(a);       // libera dati
  remNode((LPVOID)a);
  delete a;          // e poi libera se stesso
}
//-------------------------------------------------------------------------
void P_Tree::freeData(Node* a) { delete a->D; }
//-------------------------------------------------------------------------
void P_Tree::exportTree(P_File& file, fzSave save, int level,
        int offset, pNode node)
{
  if(!node)
    return;
  exportTree(file, save, level + 1, offset - 1, node->Sx);
  save(file, node, level, offset);
  exportTree(file, save, level + 1, offset + 1, node->Dx);
}
//-------------------------------------------------------------------------
