//--------------- P_Tree.h -------------------------------------
#ifndef P_TREE_H_
#define P_TREE_H_
//--------------------------------------------------------------
#include "precHeader.h"
#ifndef PDEF_H_
  #include "pDef.h"
#endif
#ifndef P_UTIL_H_
  #include "p_util.h"
#endif
//--------------------------------------------------------------
#include "setPack.h"
//--------------------------------------------------------------
typedef genericSet* TreeData;
//--------------------------------------------------------------
class P_File;
//--------------------------------------------------------------
// classe di interfaccia per albero binario
class P_Tree
{
  public:

    // autodelete determina se alla distruzione dell'albero vanno
    // distrutti anche i dati associati ai nodi
    P_Tree(bool autodelete = false);
    virtual ~P_Tree() { Flush(); }

    // pura, aggiunge un dato, ritorna true -> ok
    virtual bool Add(TreeData) = 0;

     // rimuove il nodo corrente, ritorna il dato
    virtual TreeData Rem() = 0;

    TreeData getCurr() const;

    bool setFirst() const;
    bool setLast()  const;
    bool setNext()  const;
    bool setPrev()  const;

    bool Search(TreeData d)  const;

    void Flush();

    struct Node {
      Node* Parent;     // genitore
      Node* Sx;         // ramo sx
      Node* Dx;         // ramo dx

      TreeData D;       // buffer dati
      int Flag;         // servirà per bilanciamento in AVL o altri usi

      Node() : D(0), Parent(0), Sx(0), Dx(0), Flag(0) { }
      private:
        Node(const Node& other);
        Node& operator=(const Node& other);
      };
    typedef P_Tree::Node* pNode;

    DWORD getNumElem() const { return NumElem; }

    // per un eventuale debug. Viene salvato tutto l'albero, level rappresenta
    // la profondità, mentre offset la distanza in orizzontale dalla radice,
    // con valori negativi nei rami di sx e positivi in quelli di dx
    typedef void (*fzSave)(P_File&, pNode, int level, int offset);
    void exportTree(P_File& file, fzSave save)  { exportTree(file, save, 0, 0, Radix); }

    // per non usare lo stato interno e quindi poter essere usata in più punti contemporaneamente
    pNode getFirst() const;
    pNode getLast() const;
    pNode getNext(pNode curr) const;
    pNode getPrev(pNode curr) const;

    // da usare con cautela, specialmente la seconda
    pNode getCurrN();
    void setCurrN(pNode curr);

  protected:

    pNode Radix;
    mutable pNode Curr;

    bool autoDelete;
    DWORD NumElem;

    // funzione di comparazione da ridefinire per esigenze diverse
    virtual int Cmp(const TreeData toCheck, const TreeData current) const = 0;

    void set_first(Node* node) const;
    void set_last(Node* node) const;
    bool search(Node* node, TreeData data) const; // ricerca il dato

    void freeNode(Node* a);

    virtual void freeData(Node* a);

    void exportTree(P_File& file, fzSave save, int level, int offset, pNode node);
  private:

    NO_COPY_COSTR_OPER(P_Tree)
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
inline P_Tree::P_Tree(bool autodelete) :
      autoDelete(autodelete), Radix(0), Curr(0), NumElem(0)  { }
//------------------------------------------------------------------------
inline TreeData P_Tree::getCurr() const
{
  return Curr ? Curr->D : 0;
}
//------------------------------------------------------------------------
inline bool P_Tree::Search(TreeData d) const
{
  return search(Radix, d);
}
//------------------------------------------------------------------------
#include "restorePack.h"
//------------------------------------------------------------------------
#endif
