//------------------ P_HEAP.HPP ------------------------------------------
//------------------------------------------------------------------------
template <class T>
P_Heap<T>::P_Heap(uint n, uint add) : N_Elem(0), Max(n), N_Add(add), Vect(0)
{
  if(!N_Add)
    N_Add = 10;
  allocBuffer(n);
}
//------------------------------------------------------------------------
template <class T>
P_Heap<T>::~P_Heap()
{
  delete []Vect;
}
//------------------------------------------------------------------------
template <class T>
void P_Heap<T>::allocBuffer(uint n)
{
  n /= N_Add;       // verifica se n è un multiplo di N_Add
  n *= N_Add;

  if(n != Max) {   // se non lo è lo aggiusta
    n += N_Add;
    Max = n;
    }

  if(!Max)        // se è stato passato 0, assegna almeno N_Add
    Max = N_Add;

  Vect = new T[Max];
}
//------------------------------------------------------------------------
template <class T>
void P_Heap<T>::Redim()
{
  T *tmp = new T[Max + N_Add];
  for(uint i = 0; i < N_Elem; ++i)
    tmp[i] = Vect[i];
  delete []Vect;
  Vect = tmp;
  Max += N_Add;
}
//------------------------------------------------------------------------
template <class T>
void P_Heap<T>::Make(T *v, uint n)
{
  delete []Vect;
  allocBuffer(n);       // nuova allocazione
  N_Elem = 0;
  for(uint i = 0; i < n; ++i)
    Push(v[i]);
}
//------------------------------------------------------------------------
template <class T>
T P_Heap<T>::Swap(T new_el)
{
 T ret = Vect[0];
 Vect[0] = new_el;
 heapDown();
 return ret;
}
//------------------------------------------------------------------------
template <class T>
T P_Heap<T>::Pop()
{
  T ret;
  if(N_Elem) {
    ret = Vect[0];
    Vect[0] = Vect[N_Elem-1];
    --N_Elem;
    heapDown();
    }
  return ret;
}
//------------------------------------------------------------------------
template <class T>
bool P_Heap<T>::Pop(T& target)
{
  if(N_Elem) {
    target = Vect[0];
    Vect[0] = Vect[N_Elem-1];
    --N_Elem;
    heapDown();
    return true;
    }
  return false;
}
//------------------------------------------------------------------------
template <class T>
bool P_Heap<T>::Push(T v)
{
  if(N_Elem >= Max)
    Redim();
  Vect[N_Elem] = v;
  for(uint i = N_Elem; i; ) {
    uint k = (i - 1) >> 1; // genitore
    if(Great(k, i)) {   // se il genitore è più grande scambia
      T w = Vect[k];
      Vect[k] = Vect[i];
      Vect[i] = w;
      i = k;              // e continua con il genitore che diventa figlio,
      }                 // sale nell'albero
    else    // altrimenti ha trovato il suo posto
      break;
    }
  ++N_Elem;
  return true;
}
//------------------------------------------------------------------------
template <class T>
void P_Heap<T>::heapDown()
{
 // a parte la testa, il resto è collocato esattamente in struttura, quindi
 // spostiamo in basso la testa
  for(uint i = 0; ; ) {
    uint j = (i << 1) + 2;  // figlio di destra
    if(j > N_Elem)
      break;         // se supera la lista termina
    if(j < N_Elem) { // se ci sono due figli il confronto va fatto col minore
      if(Great(j, j - 1))
        --j;
      }
    else
      --j;
    if(Great(i, j)) { // se il genitore è più grande del figlio scambia
      T k = Vect[i];
      Vect[i] = Vect[j];
      Vect[j] = k;
      i = j;  // continua con il figlio, scende nell'albero
      }
    else          // altrimenti è al posto giusto
      break;
    }
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
