//---------------- groupObject.h ----------------------------
//-----------------------------------------------------------
#ifndef groupObject_H_
#define groupObject_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "common.h"
#include "p_txt.h"
#include "svmHistory.h"
//-----------------------------------------------------------
#define ID_FIRST_GROUP 780001
//-----------------------------------------------------------
class groupObject;
class PListHistory;
//-----------------------------------------------------------
class svmBaseObject : public genericSet
{
  private:
    typedef genericSet baseClass;
  public:
    svmBaseObject() : Parent(0), Locked(0) {}
    virtual ~svmBaseObject() {}

    void setGroup(groupObject* par);
    void setGroupSimple(groupObject* par);

    virtual void Select(HDC hdc);
    virtual void Unselect(HDC hdc);
    virtual void setSelectSimple(bool set);
    virtual void standBySel(bool add);

    virtual void setLock(bool set);
    virtual void setVisible(bool set);

    // va richiamata dalla classe finale sostituendo il numero progressivo base con
    // l'id vero salvato su file
    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);

    groupObject* getGroup() { return Parent; }
    groupObject* getGroup() const { return Parent; }
    void setLockGroup() { ++Locked; }
    void releaseLockGroup() { if(Locked) --Locked; }

    bool sameGroup(svmBaseObject* other);
    bool is_Locked() const { return Locked > 0; }

    virtual svmObjHistory* makeHistoryObject(svmObjHistory::typeOfAction, svmBaseObject* prev = 0, bool useOld = false);
    virtual void restoreByHistoryObject(const svmObjHistory& history);

  protected:
    // se fa parte di un gruppo
    groupObject* Parent;
    uint Locked;
    virtual bool useBlock() const;
};
//-----------------------------------------------------------
class groupObject : public svmBaseObject
{
  private:
    typedef svmBaseObject baseClass;
  public:
    groupObject() {}
    virtual ~groupObject() {}

    // obj può essere sia un gruppo che l'oggetto vero
    void addObject(svmBaseObject* obj);
    void remObject(svmBaseObject* obj);

    virtual void Select(HDC hdc, svmBaseObject* sender);
    virtual void Unselect(HDC hdc, svmBaseObject* sender);
    virtual void setSelectSimple(bool set, svmBaseObject* sender);
    virtual void standBySel(bool add, svmBaseObject* sender);
    virtual void setLock(bool set, svmBaseObject* sender);
    virtual void setVisible(bool set, svmBaseObject* sender);

    void setId(uint id) { Id = id; }
    void resolveId();
    void setId(uint id, svmBaseObject* sender);

    virtual bool save(P_File& pf);
    static groupObject* getHighGroup(svmBaseObject* obj);
    static void explodeHigh(svmBaseObject* obj, class PListHistory& history);
    static void explodeAll(svmBaseObject* obj, class PListHistory& history);

//    void explodeHigh();
//    void explodeAll();

    void explodeHigh(PListHistory& history);
    svmObjHistory* explodeAll(PListHistory& history, svmObjHistory* h = 0);

    uint getId() { return Id; }

    uint getElem() const { return Child.getElem(); }
    svmBaseObject* getObj(uint ix)  { if(ix <getElem()) return Child[ix].obj; return 0; }
  protected:
    struct infoObj {
      uint Id;
      svmBaseObject* obj;
      infoObj(svmBaseObject* obj, uint id = 0) : Id(id), obj(obj) {}
      infoObj() : Id(0), obj(0) {}
      };
    PVect<infoObj> Child;
    uint Id;
};
//-----------------------------------------------------------
class manageGroupObject
{
  public:
    manageGroupObject() {}
    virtual ~manageGroupObject() {}

    // crea un gruppo vuoto, lo inserisce nella lista e lo ritorna
    groupObject* createGroup();
    void addObject(groupObject& group, svmBaseObject* obj) { group.addObject(obj); }
    void remObject(groupObject& group, svmBaseObject* obj) { group.remObject(obj); }

    void explodeHigh(svmBaseObject* obj, PListHistory& history) { groupObject::explodeHigh(obj, history); }
    void explodeAll(svmBaseObject* obj, PListHistory& history) { groupObject::explodeAll(obj, history); }

    void explodeHigh(groupObject& group, PListHistory& history) { group.explodeHigh(history); }
    void explodeAll(groupObject& group, PListHistory& history) { group.explodeAll(history); }

    bool save(P_File& pf);

    // da richiamarsi solo prima dell'apertura di una nuova pagina
    void flushAll();

  protected:
    PVect<groupObject*> Groups;
};
//-----------------------------------------------------------
manageGroupObject& getManageGroupObject();
//-----------------------------------------------------------
struct dualGroupObject
{
  uint idObj;
  groupObject* group;
  dualGroupObject() : idObj(0), group(0) {}
  dualGroupObject(groupObject* group, uint idObj) : idObj(idObj), group(group) {}
  dualGroupObject(uint idObj) : idObj(idObj), group(0) {}

  bool operator <(const dualGroupObject& other) const {
    return int(idObj) < int(other.idObj);
    }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class createGroupObject
{
  public:
    createGroupObject(manageGroupObject& mgo, setOfString& set) : manGroupObj(mgo) { make(set); }
    virtual ~createGroupObject() {}

    void setGroup(svmBaseObject* obj, uint id);
  protected:
    PVect<dualGroupObject> objSet;
    PVect<dualGroupObject> groupSet;
    manageGroupObject& manGroupObj;
    void make(setOfString& set);
};
//-----------------------------------------------------------
#define FNZ_IF(fnz, param) \
  if(Parent && useBlock()) \
    Parent->fnz(param, this);
//-----------------------------------------------------------
inline
void svmBaseObject::Select(HDC hdc)
{
  FNZ_IF(Select, hdc);
}
//-----------------------------------------------------------
inline
void svmBaseObject::Unselect(HDC hdc)
{
  FNZ_IF(Unselect, hdc);
}
//-----------------------------------------------------------
inline
void svmBaseObject::setSelectSimple(bool set)
{
  FNZ_IF(setSelectSimple, set);
}
//-----------------------------------------------------------
inline
void svmBaseObject::standBySel(bool add)
{
  FNZ_IF(standBySel, add);
}
//-----------------------------------------------------------
#undef FNZ_IF
//-----------------------------------------------------------
inline
bool svmBaseObject::save(P_File& pfCript, P_File& pfClear, uint order)
{
  if(Parent)
    Parent->setId(order, this);
  return true;
}
//-----------------------------------------------------------
inline
void svmBaseObject::setGroupSimple(groupObject* par)
{
  Parent = par;
}
//-----------------------------------------------------------
inline
void svmBaseObject::setGroup(groupObject* par)
{
  if(!par) {
    if(Parent)
      Parent->remObject(this);
    }
  Parent = par;
  if(Parent)
    Parent->addObject(this);
}
//-----------------------------------------------------------
inline
bool svmBaseObject::sameGroup(svmBaseObject* other)
{
  if(!other || !getGroup() || !other->getGroup())
    return false;
  return groupObject::getHighGroup(this) == groupObject::getHighGroup(other);
}
//-----------------------------------------------------------
#endif
