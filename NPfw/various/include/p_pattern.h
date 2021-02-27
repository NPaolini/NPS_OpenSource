//------------------- p_pattern.h --------------------------------
#ifndef p_pattern_H_
#define p_pattern_H_
//----------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------
template <typename T, T J_FULL = (T)'*', T J_1 = (T)'?'>
class p_pattern
{
  public:
    p_pattern(const T* toCheck, const T* pattern) :
        toCheck(toCheck), pattern(pattern), posC(0), posP(0), found(false) {}
    bool match();
  private:
    const T* toCheck;
    const T* pattern;
    unsigned int posC;
    unsigned int posP;
    bool found;
    void state_0();
    void state_1();
    void state_2();
    void state_3();
    void checkNext();
};
//--------------------------------
template <typename T, T J_FULL, T J_1>
bool p_pattern<T, J_FULL, J_1>::match()
{
  state_0();
  return found;
}
//--------------------------------
template <typename T, T J_FULL, T J_1>
void p_pattern<T, J_FULL, J_1>::state_0()
{
  T c = pattern[posP];
  switch(c) {
    case 0:
      if(!toCheck[posC])
        found = true;
      break;
    case J_FULL:
      state_1();
      break;
    case J_1:
      state_2();
      break;
    default:
      state_3();
      break;
    }
}
//--------------------------------
template <typename T, T J_FULL, T J_1>
void p_pattern<T, J_FULL, J_1>::state_1()
{
  while(true) {
    T c = pattern[++posP];
    switch(c) {
      case 0: // terminato il pattern col jolly, si accetta tutto quel che segue
        found = true;
        return;
      case J_FULL:
      case J_1:
        // c'è un altro jolly, lo ignoriamo e proseguiamo
        break;
      default:
        checkNext();
        return;
      }
    }
}
//--------------------------------
template <typename T, T J_FULL, T J_1>
void p_pattern<T, J_FULL, J_1>::state_2()
{
  ++posC;
  ++posP;
  state_0();
}
//--------------------------------
template <typename T, T J_FULL, T J_1>
void p_pattern<T, J_FULL, J_1>::state_3()
{
  T p = pattern[posP++];
  T c = toCheck[posC++];
  if(p == c)
    state_0();
}
//--------------------------------
template <typename T, T J_FULL, T J_1>
void p_pattern<T, J_FULL, J_1>::checkNext()
{
  T p = pattern[posP];
  T c;
  do {
    c = toCheck[posC++];
    if(p == c) {
      int oldP = posP++;
      int oldC = posC;
      state_0();
      if(found)
        break;
      posC = oldC;
      posP = oldP;
      }
    } while(c);
}
//----------------------------------------------------------------
#include "restorePack.h"

#endif
