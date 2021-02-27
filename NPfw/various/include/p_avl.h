//--------------- P_Avl.h -------------------------------------
#ifndef P_AVL_H_
#define P_AVL_H_
//-----------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------
#ifndef P_TREE_H_
  #include "p_tree.h"
#endif
//-----------------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------------
class P_Avl : public P_Tree
{
  public:
    P_Avl(bool autodelete);
    bool Add(TreeData f);
    TreeData Rem();

  private:

    pNode ins_0(pNode a);
    pNode ins_1(pNode a, pNode node);

    pNode rotate_SS(pNode node);
    pNode rotate_SD(pNode node);
    pNode rotate_DD(pNode node);
    pNode rotate_DS(pNode node);

    void set_flag_SD_DS(pNode a, pNode b, pNode c);

    // torna false se non ha parent
    bool adjust_parent(pNode a, pNode b);

    pNode extract(pNode *rad, pNode e);
    pNode extract_SX(pNode node);
    pNode extract_DX(pNode node);

    int Incr;

    NO_COPY_COSTR_OPER(P_Avl)
};
//-----------------------------------------------------------------
inline P_Avl::P_Avl(bool autodelete) : P_Tree(autodelete) { }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*
  per utilizzarla occorre che la classe che contiene il dato erediti da 'genericSet'
  e che definisca l'operatore >
  es.
  //-----------------------------------------------------------------------------
  class sample : public genericSet
  {
    public:
      sample(int val = 0) : Val(val) {}

      int Val;

      bool operator >(const sample& other) { return Val > other.Val; }
  };
  //-----------------------------------------------------------------------------
  typedef genericAvl<sample> orderedSampleValue;

  orderedSampleValue set(true);

*/
//-----------------------------------------------------------------------------
template <class T>
class genericAvl : public P_Avl
{
  public:
    genericAvl(bool autodelete) : P_Avl(autodelete) { }
    T* getCurrData() const { return reinterpret_cast<T*>(getCurr()); }

  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
};
//-----------------------------------------------------------------
template <class T>
int genericAvl<T>::Cmp(const TreeData toCheck, const TreeData current) const
{
  T* check = reinterpret_cast<T*>(toCheck);
  T* curr = reinterpret_cast<T*>(current);
  if(*curr > *check)
    return -1;
  if(*check > *curr)
    return 1;
  return 0;
}
//-----------------------------------------------------------------
#include "restorePack.h"

#endif
