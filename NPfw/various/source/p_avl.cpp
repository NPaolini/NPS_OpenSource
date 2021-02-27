//-------------------- P_AVL.CPP ----------------------------------------
#include "precHeader.h"
#include "p_avl.h"
//-----------------------------------------------------------------------
enum { SX, DX, XX };
//-----------------------------------------------------------------------
#ifdef CHECK_LEAK
static PVect<LPVOID> _trace_node;
#include "p_file.h"
//-----------------------------------------------------------------------
void saveResult(P_File& pf, LPVOID t = 0)
{
  TCHAR buff[256];
  if(t) {
    wsprintf(buff, _T("Elem?? = %p\r\n"), t);
    pf.P_writeString(buff);
    return;
    }
  uint nElem = _trace_node.getElem();
  wsprintf(buff, _T("Max Elem. = %d\r\n"), nElem);
  pf.P_writeString(buff);
  for(uint i = 0; i < nElem; ++i) {
    if(_trace_node[i]) {
      wsprintf(buff, _T("Elem[%d] = %p\r\n"), i, _trace_node[i]);
      pf.P_writeString(buff);
      }
    }
  _trace_node.reset();
}
//-----------------------------------------------------------------------
static void addNode(LPVOID t)
{
  uint nElem = _trace_node.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!_trace_node[i]) {
      nElem = i;
      break;
      }
    }
  _trace_node[nElem] = t;
}
//-----------------------------------------------------------------------
void remNode(LPVOID t)
{
  uint nElem = _trace_node.getElem();
  if(nElem) {
    for(uint i = 0; i < nElem; ++i) {
      if(t == _trace_node[i]) {
        _trace_node[i] = 0;
        return;
        }
      }
    }
//  else {
    P_File pf(_T("test_leak.txt"));
    if(pf.P_open()) {
      pf.P_seek(0, SEEK_END_);
      saveResult(pf, t);
      }
//    }
}
//void saveResult(P_File& pf) {}
//-----------------------------------------------------------------------
#else
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
#define addNode(t)
//-----------------------------------------------------------------------
#define remNode(t)
//-----------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------
bool P_Avl::Add(TreeData a)
{
  pNode t = new Node;  // alloca nuovo nodo
  if(!t)
    return false;
  t->D = a;                          // assegna il dato
  pNode ret = ins_0(t);     // ritorna la radice o NULL se errore
  if(ret) {
    ++NumElem;        // incrementa numero di elementi presenti
    Radix = ret;         // assegna la radice
    if(!Curr)          // se Curr non è assegnato lo assegna alla radice
      Curr = ret;
    addNode((LPVOID)t);
    return true;
    }
  delete t;           // dealloca il nodo se errore
  return false;
}
//-----------------------------------------------------------------------
TreeData P_Avl::Rem()
{
  pNode t = Curr;
  if(!setNext())     // se non c'è un successivo
    setPrev();       // sposta Curr sul precedente
  Incr = 0;
  t = extract(&(Radix), t); // ed elimina il Curr
  TreeData ret;
  if(t) {                   // se successo
    --NumElem;             // decrementa numero di elementi
    if(Radix)               // se non era l'unico nodo
      while(Radix->Parent)  // potrebbero esserci state delle rotazioni
        Radix = Radix->Parent; // allora risale l'albero
    else
      Curr = 0;
    ret = t->D;      // prende i dati
    remNode((LPVOID)t);
    delete t;      // dealloca il nodo
    }
  else
    ret = 0;
  return ret;
}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
P_Tree::pNode P_Avl::ins_0(pNode a)
{ // poiché la ricorsione si ferma con un nodo valido il test
  // di non validità del nodo è stato separato dalla ricorsione
  if(!Radix)
    return a;
  Incr = 0; // variabile che controlla l'aggiunta di livello
  return ins_1(a, Radix);
}
//-----------------------------------------------------------------------
P_Tree::pNode P_Avl::ins_1(pNode a, pNode node)
{
  int where = XX;  // nel caso di chiave duplicata sceglie il ramo migliore
  int cmp = Cmp(a->D, node->D);
  if(!cmp) { // chiave duplicata
    if(node->Flag <= 0)
      where = DX;
    else
      where = SX;
    }
  if(cmp < 0 || where == SX) {
    if(!node->Sx) {      // ha trovato la posizione
      node->Sx = a;      // assegna e aggiusta i puntatori
      a->Parent = node;
      --node->Flag;      // decrementa il Flag di bilanciamento
      if(node->Flag)  // se si è aggiunto un livello
        Incr = 1;
      else
        Incr = 0;   // azzera Flag di livello
      }
    else {    // inserisce sul ramo sinistro
      node->Sx = ins_1(a, node->Sx);
      // se è stato aggiunto un livello ed il bilanciamento è -1
      if(Incr) {
        if(0 > node->Flag) {
//        if(-1 == node->Flag) {
          Incr = 0;// riazzera il Flag di livello
          if(node->Sx->Flag < 0)
            return rotate_SS(node);  // a seconda del Flag del figlio di
          else        // sinistra decide il tipo di rotazione
            return rotate_SD(node);
          }
        else    // se è stato aggiunto un livello e non è stata effettuata
          Incr = --node->Flag;// rotazione decrementa il Flag di bilanciamento
        }
      }
    }
  // effettua le operazioni di sopra ma in modo simmetrico
  else {
    if(!node->Dx) {
      node->Dx = a;
      a->Parent = node;
      ++node->Flag;
      if(node->Flag)
        Incr = 1;
      else
        Incr = 0;
      }
    else {
      node->Dx = ins_1(a, node->Dx);
      if(Incr) {
        if(0 < node->Flag) {
//        if(1 == node->Flag) {
          Incr = 0;
          if(node->Dx->Flag > 0)
            return rotate_DD(node);
          else
            return rotate_DS(node);
          }
        else
          Incr = ++node->Flag;
        }
      }
    }
  return node;
}
//-----------------------------------------------------------------------
P_Tree::pNode P_Avl::rotate_SS(pNode node)
/*
   //       a == nodo
   //      /                b == t
   //     b      ->        / \
   //    /                c   a
   //   c
*/
{
  pNode t = node->Sx;     // nodo che deve salire

  pNode t1 = t->Dx;       // nodo che verrà assegnato alla Sx di nodo
  node->Sx = t1;    // aggiusta i puntatori tra t1 e nodo
  if(t1)
    t1->Parent = node;

  t->Parent = node->Parent; // aggiusta i puntatori tra t e nodo
  t->Dx = node;

  adjust_parent(t, node);
  node->Parent = t;
  if(t->Flag < 0) {
    t->Flag = 0;
    node->Flag = 0;
    }
  else {
    node->Flag = -1;
    t->Flag = 1;
    }
  return t;
}
//-----------------------------------------------------------------------
P_Tree::pNode P_Avl::rotate_SD(pNode node)
/*
   //         a == node
   //      /               c  == t
   //    b      ->        / \
   //     \              b   a
   //      c
*/
{
  if(/*!node->Dx ||*/ node->Sx->Dx) {
//  if(!node->Dx || node->Dx->Sx) {
    pNode t = node->Sx->Dx;     // node che deve salire

    pNode t1 = node->Sx;      // intermedio tra node e t
    t1->Parent = t;
    t1->Dx = t->Sx;
    t->Sx = t1;
    if(t1->Dx)
      t1->Dx->Parent = t1;

    node->Sx = t->Dx;
    if(node->Sx)
      node->Sx->Parent = node;
    t->Parent = node->Parent;
    t->Dx = node;

    adjust_parent(t, node);
    node->Parent = t;
    set_flag_SD_DS(t, node, t1);
    return t;
    }
  else {

  // nel caso non esista il nodo Sx del nodo Dx di a, scambia
  // solo i due nodi a e b
/*
  //         a == node    b  == t
  //      /                 \
  //    b      ->             a
  //     \                   /
  //      c                 c
*/
    pNode t = node->Sx;     // node che deve salire

    node->Sx = t->Dx;
    if(t->Dx)
      t->Dx->Parent = node;

    t->Parent = node->Parent;
    t->Dx = node;

    adjust_parent(t, node);
    node->Parent = t;

    ++node->Flag;

    return t;
    }

}
//-----------------------------------------------------------------------
P_Tree::pNode P_Avl::rotate_DD(pNode node)  // simmetrica a SS
{
  pNode t = node->Dx;     // node che deve salire

  pNode t1 = t->Sx;       // node che verrà assegnato alla Sx di node
  node->Dx = t1;
  if(t1)
    t1->Parent = node;

  t->Parent = node->Parent;
  t->Sx = node;

  adjust_parent(t, node);
  node->Parent = t;

  if(t->Flag > 0) {
    t->Flag = 0;
    node->Flag = 0;
    }
  else {
    node->Flag = 1;
    t->Flag = -1;
    }
  return t;
}
//-----------------------------------------------------------------------
P_Tree::pNode P_Avl::rotate_DS(pNode node)  // simmetrica a SD
{
  if(/*!node->Sx ||*/ node->Dx->Sx) {
//  if(!node->Sx || node->Sx->Dx) {
    pNode t = node->Dx->Sx;     // node che deve salire

    pNode t1 = node->Dx;      // node che punta a t che punterà a node
    t1->Parent = t;
    t1->Sx = t->Dx;
    t->Dx = t1;
    if(t1->Sx)
      t1->Sx->Parent = t1;

    node->Dx = t->Sx;
    if(node->Dx)
      node->Dx->Parent = node;
    t->Parent = node->Parent;
    t->Sx = node;

    adjust_parent(t, node);
    node->Parent = t;
    set_flag_SD_DS(t, t1, node);
    return t;
    }

  else {
    pNode t = node->Dx;     // node che deve salire

    node->Dx = t->Sx;
    if(t->Sx)
      t->Sx->Parent = node;

    t->Parent = node->Parent;
    t->Sx = node;

    adjust_parent(t, node);
    node->Parent = t;

    node->Flag = 0;
    t->Flag = 0;

    return t;
    }
/**/
}
//-----------------------------------------------------------------------
void P_Avl::set_flag_SD_DS(pNode a, pNode b, pNode c) // t,[node,t1][t1,node]
{
  if(0 == a->Flag) {
    c->Flag = 0;
    b->Flag = 0;
    }
  else if(a->Flag < 0) {
    c->Flag = 0;
    if(b->Dx)
      b->Flag = 1;
    else
      b->Flag = 0;
    }
  else {
    if(c->Sx)
      c->Flag = -1;
    else
      c->Flag = 0;
    b->Flag = 0;
    }
  a->Flag = 0;
}
//-----------------------------------------------------------------------
bool P_Avl::adjust_parent(pNode a, pNode b)
{
  if(b->Parent) {
    if(b->Parent->Sx == b)
      b->Parent->Sx = a;
    else
      b->Parent->Dx = a;
    return true;
    }
  return false;
}
//-----------------------------------------------------------------------
/*
  percorre l'albero a trovare il node da estrarre; se trovato, secondo
  lo sbilanciamento, estrae da SX o da DX il node corrispondente al
  valore appena inferiore o superiore al node da estrarre e lo sostituisce;
  tornando indietro aggiusta i Flag di bilanciamento e, se necessario,
  ribilancia tramite rotazioni; il node ritornato dall'estrai_XX contiene
  nel Flag se aggiustare il bilanciamento.
  Rad è dichiarato come puntatore a puntatore ad Albero poiché, nel caso
  il valore da estrarre sia proprio la radice, deve poter modificare
  l'indirizzo della radice dell'albero
*/
//-----------------------------------------------------------------------
#define Decr Incr
//-----------------------------------------------------------------------
P_Tree::pNode P_Avl::extract(pNode *rad, pNode e)
{
  if(!*rad) {   // il node non esiste
   Decr = 0;
   return 0;
   }
  pNode node = *rad;
  int cmp = Cmp(e->D, node->D);
  if(!cmp) { // trovato il valore da estrarre
    // sembra ci sia un bug nell'assegnazione del flag
    // per cui per il momento, per evitare il crash, assegno il flag se non
    // sono presenti entrambi i nodi
#if 1
    if(toBool(node->Sx) ^ toBool(node->Dx)) {
      if(!node->Sx && node->Flag <= 0)
        node->Flag = 1;
      else if(!node->Dx && node->Flag >= 0)
        node->Flag = -1;
      }
#endif
    if(node->Sx || node->Dx) {  // se ha almeno un ramo
      pNode t;
      if(node->Flag < 0) {
//        if(!node->Sx)
//          return 0;
      // estrae, partendo da SX, il node più a DX
        t = extract_DX(node->Sx);  // ritorna t da sostituire a node
        Decr = t->Flag;  // l'eventuale necessità di bilanciamento ritorna in t
        t->Flag = node->Flag + (Decr != 0); // aggiorna il bilanciamento
        }
      else if(node->Flag > 0) {
//        if(!node->Dx)
//          return 0;
        t = extract_SX(node->Dx);
        Decr = t->Flag;
        t->Flag = node->Flag - (Decr != 0);
        }
      else {      // se è bilanciato va a SX ??? bug?
        if(node->Sx)
          t = extract_DX(node->Sx); // t mantiene il Flag
        else
          t = extract_SX(node->Dx);
        Decr = 0;
        }
      t->Parent = node->Parent; // sostituisce node con t
      if((t->Sx = node->Sx) != 0)
        t->Sx->Parent = t;
      if((t->Dx = node->Dx) != 0)
        t->Dx->Parent = t;
      if(!adjust_parent(t, node)) // se node era la radice principale
        *rad = t;                 // la radice ora è t
      }
    else {                         // non ha rami
      Decr = 1;
      if(!adjust_parent(0, node))
        *rad = 0;                 // ed era l'unico node esistente
      }
    }
  else if(cmp < 0) {               // altrimenti scende nel ramo SX
    pNode t = extract(&node->Sx, e);
    if(t && Decr)
      Decr = (node->Flag > 0) || !(++(node->Flag));   // ribilancia il Flag
    if(t && Decr && 0 < node->Flag) {    // è necessaria una rotazione ?
//    if(t && Decr && 1 == node->Flag) {    // è necessaria una rotazione ?
      if(node->Dx) {
        if(node->Dx->Flag > 0)
          rotate_DD(node);
        else if(node->Dx->Flag < 0)
          rotate_DS(node);
        else {
          rotate_DD(node);
          Decr = 0;
          }
        }
      }
    node = t;
    }
  else {                            // altrimenti scende nel ramo DX
    pNode t = extract(&node->Dx, e);
    if(t && Decr)
      Decr = (node->Flag < 0) || !(--(node->Flag));
    if(t && Decr && 0 > node->Flag) {
//    if(t && Decr && -1 == node->Flag) {
      if(node->Sx) {
        if(node->Sx->Flag < 0)
          rotate_SS(node);
        else if(node->Sx->Flag > 0)
          rotate_SD(node);
        else {
          rotate_SS(node);
          Decr = 0;
          }
        }
      }
    node = t;
    }
  return node;
}
//-----------------------------------------------------------------------
/*
  arriva in fondo all'albero seguendo il ramo SX, toglie la foglia
  terminale aggiustando i puntatori, nel ritorno aggiusta i Flag dei nodi
  ed effettua, se necessario delle rotazioni di ribilanciamento
*/
P_Tree::pNode P_Avl::extract_SX(pNode node)
{
  if(!node->Sx) {
    if(node->Dx) {
      if(node->Parent->Dx != node) {
        node->Parent->Sx = node->Dx;
        node->Dx->Parent = node->Parent;
        }
      else
        node->Parent->Dx = node->Dx;
      }
    else if(node->Parent->Dx == node)
      node->Parent->Dx = 0;
    else
      node->Parent->Sx = 0;
    node->Flag = 1;
    return node;
    }
  pNode t = extract_SX(node->Sx);
  if(t->Flag && node->Flag > 0) {
//  if(t->Flag && node->Flag == 1) {
    if(node->Dx) {
      if(node->Dx->Flag > 0)
        rotate_DD(node);
      else if(node->Dx->Flag < 0)
        rotate_DS(node);
      else {
        rotate_DD(node);
        t->Flag = 0;
        }
      }
    }
  else if(t->Flag)
    t->Flag = !(++(node->Flag));
  return t;
}
//-----------------------------------------------------------------------
P_Tree::pNode P_Avl::extract_DX(pNode node)
{
  if(!node->Dx) {
    if(node->Sx) {
      if(node->Parent->Sx != node) {
        node->Parent->Dx = node->Sx;
        node->Sx->Parent = node->Parent;
        }
      else
        node->Parent->Sx = node->Sx;
      }
    else if(node->Parent->Sx == node)
      node->Parent->Sx = 0;
    else
      node->Parent->Dx = 0;
    node->Flag = 1;
    return node;
    }
  pNode t = extract_DX(node->Dx);
  if(t->Flag && node->Flag < 0) {
//  if(t->Flag && node->Flag == -1) {
    if(node->Sx) {
      if(node->Sx->Flag < 0)
        rotate_SS(node);
      else if(node->Sx->Flag > 0)
        rotate_SD(node);
      else {
        rotate_SS(node);
        t->Flag = 0;
        }
      }
    }
  else if(t->Flag)
    t->Flag = -(!(--(node->Flag)));
//    t->Flag = !(--(node->Flag));

  return t;
}
//-----------------------------------------------------------------------

