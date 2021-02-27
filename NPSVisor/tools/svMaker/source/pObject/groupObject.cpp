//---------------- groupObject.cpp --------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "groupObject.h"
#include "svmDataProject.h"
#include "svmBase.h"
//-----------------------------------------------------------
bool svmBaseObject::useBlock() const
{
  return !Locked && !notUseGroup();
}
//-----------------------------------------------------------
void svmBaseObject::restoreByHistoryObject(const svmObjHistory& history)
{
  if(svmObjHistory::Group == history.getAction()) {
    groupObject* grp = (groupObject*)history.getClone();
    setGroup(grp);
    }
}
//-----------------------------------------------------------
svmObjHistory* svmBaseObject::makeHistoryObject(svmObjHistory::typeOfAction act, svmBaseObject* prev, bool useOld)
{
  svmObjHistory* obj = new svmObjHistory(this, act, prev);
  if(!obj)
    return 0;
  if(svmObjHistory::Group == act) {
    svmBaseObject* clone = getGroup();
    obj->setClone(clone);
    }
  return obj;
}
//-----------------------------------------------------------
void svmBaseObject::setLock(bool set)
{
  if(Parent)
    Parent->setLock(set, this);
}
//-----------------------------------------------------------
void svmBaseObject::setVisible(bool set)
{
  if(Parent)
    Parent->setVisible(set, this);
}
//-----------------------------------------------------------
class svmFactoryGroupManager
{
  public:
    svmFactoryGroupManager() {}
    ~svmFactoryGroupManager() {}
    manageGroupObject& getManageGroupObject()
    {
      const dataProject& dp = getDataProject();
      return GroupManager[dp.currPage];
    }

  private:
    manageGroupObject GroupManager[MAX_PAGES];
};
//-----------------------------------------------------------
static svmFactoryGroupManager staticFactoryGroupManager;
//-----------------------------------------------------------
manageGroupObject& getManageGroupObject()
{
  return staticFactoryGroupManager.getManageGroupObject();
}
//-----------------------------------------------------------
//-----------------------------------------------------------
groupObject* groupObject::getHighGroup(svmBaseObject* obj)
{
  groupObject* group = obj->getGroup();
  if(!group)
    return 0;
  while(group->getGroup())
    group = group->getGroup();
  return group;
}
//-----------------------------------------------------------
void groupObject::explodeHigh(svmBaseObject* obj, PListHistory& history)
{
  groupObject* group = getHighGroup(obj);
  group->explodeHigh(history);
}
//-----------------------------------------------------------
void groupObject::explodeAll(svmBaseObject* obj, PListHistory& history)
{
  groupObject* group = getHighGroup(obj);
  group->explodeAll(history);
}
//-----------------------------------------------------------
void groupObject::explodeHigh(PListHistory& history)
{
  int nElem = Child.getElem();
  svmObjHistory* h = 0;
  for(int i = nElem - 1; i >= 0; --i) {
    svmBaseObject* t = Child[i].obj;
    svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Group);
    if(!h)
      history.Push(th);
    else
      h->setNext(th);
    h = th;
    t->setGroupSimple(0);
    Child.remove(i);
    }
}
//-----------------------------------------------------------
svmObjHistory* groupObject::explodeAll(PListHistory& history, svmObjHistory* h)
{
  int nElem = Child.getElem();
  for(int i = nElem - 1; i >= 0; --i) {
    svmBaseObject* t = Child[i].obj;
    svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Group);
    if(!h)
      history.Push(th);
    else
      h->setNext(th);
    h = th;
    groupObject* group = dynamic_cast<groupObject*>(t);
    if(group)
      h = group->explodeAll(history, h);
    else {
      Child[i].obj->setGroupSimple(0);
      }
    Child.remove(i);
    }
  return h;
}
//-----------------------------------------------------------
void groupObject::setId(uint id, svmBaseObject* sender)
{
  uint nElem = Child.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(Child[i].obj == sender) {
      Child[i].Id = id;
      break;
      }
    }
}
//-----------------------------------------------------------
void groupObject::addObject(svmBaseObject* obj)
{
  uint nElem = Child.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(Child[i].obj == obj)
      return;
    }
  obj->setGroupSimple(this);
  Child[nElem] = infoObj(obj);
}
//-----------------------------------------------------------
void groupObject::remObject(svmBaseObject* obj)
{
  int nElem = Child.getElem();
  for(int i = nElem - 1; i >= 0; --i) {
    if(Child[i].obj == obj) {
      obj->setGroupSimple(0);
      Child.remove(i);
      break;
      }
    }
}
//-----------------------------------------------------------
void groupObject::resolveId()
{
  uint nElem = Child.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!Child[i].Id) {
      groupObject* group = dynamic_cast<groupObject*>(Child[i].obj);
      if(group)
        Child[i].Id = group->getId();
      }
    }
}
//-----------------------------------------------------------
bool groupObject::save(P_File& pf)
{
  uint nElem = Child.getElem();
  if(!nElem)
    return true;
  TCHAR buff[256];
  wsprintf(buff, _T("%d"), Id);
  if(!writeStringChkUnicode(pf, buff))
    return false;
  for(uint i = 0; i < nElem; ++i) {
    if(Child[i].Id) {
      wsprintf(buff, _T(",%d"), Child[i].Id);
      if(!writeStringChkUnicode(pf, buff))
        return false;
      }
    }
  if(!writeStringChkUnicode(pf, _T("\r\n")))
    return false;
  return true;
}
//-----------------------------------------------------------
#define tFNZ(fnz, param) \
{ \
  setLockGroup(); \
  if(Parent) \
    Parent->fnz(param, this); \
  uint nElem = Child.getElem(); \
  for(uint i = 0; i < nElem; ++i) { \
    if(Child[i].obj != sender) { \
      bool locked = Child[i].obj->is_Locked(); \
      Child[i].obj->setLockGroup(); \
      groupObject* go = dynamic_cast<groupObject*>(Child[i].obj); \
      if(go) { \
        if(!locked) \
          go->fnz(param, this); \
        } \
      else\
        Child[i].obj->fnz(param); \
      Child[i].obj->releaseLockGroup(); \
      } \
    } \
  releaseLockGroup(); \
}
//-----------------------------------------------------------
void groupObject::Select(HDC hdc, svmBaseObject* sender)
{
//  tFNZ(Select, hdc);
  setLockGroup();
  if(Parent)
    Parent->Select(hdc, this);

  uint nElem = Child.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(Child[i].obj != sender) {
      bool locked = Child[i].obj->is_Locked();
      Child[i].obj->setLockGroup();
      groupObject* go = dynamic_cast<groupObject*>(Child[i].obj);
      if(go) {
        if(!locked)
          go->Select(hdc, this);
        }
      else
        Child[i].obj->Select(hdc);
      Child[i].obj->releaseLockGroup();
      }
    }
  releaseLockGroup();
}
//-----------------------------------------------------------
void groupObject::Unselect(HDC hdc, svmBaseObject* sender)
{
  tFNZ(Unselect, hdc);
}
//-----------------------------------------------------------
void groupObject::setSelectSimple(bool set, svmBaseObject* sender)
{
  tFNZ(setSelectSimple, set);
}
//-----------------------------------------------------------
void groupObject::standBySel(bool add, svmBaseObject* sender)
{
  tFNZ(standBySel, add);
}
//-----------------------------------------------------------
void groupObject::setLock(bool set, svmBaseObject* sender)
{
  tFNZ(setLock, set);
}
//-----------------------------------------------------------
void groupObject::setVisible(bool set, svmBaseObject* sender)
{
  tFNZ(setVisible, set);
}
//-----------------------------------------------------------
#undef tFNZ
//-----------------------------------------------------------
//-----------------------------------------------------------
groupObject* manageGroupObject::createGroup()
{
  groupObject* group = new groupObject;
  Groups[Groups.getElem()] = group;
  return group;
}
//-----------------------------------------------------------
bool manageGroupObject::save(P_File& pf)
{
  uint nElem = Groups.getElem();
  for(uint i = 0, j = ID_FIRST_GROUP; i < nElem; ++i) {
    if(Groups[i]->getElem())
      Groups[i]->setId(j++);
    }
  for(uint i = 0; i < nElem; ++i)
    Groups[i]->resolveId();

  for(uint i = 0; i < nElem; ++i) {
    if(!Groups[i]->save(pf))
      return false;
    }
  return true;
}
//-----------------------------------------------------------
void manageGroupObject::flushAll()
{
  flushPV(Groups);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void createGroupObject::make(setOfString& set)
{
  for(uint i = 0; i < 2000; ++i) {
    LPCTSTR p = set.getString(ID_FIRST_GROUP + i);
    if(!p)
      break;

    groupObject* go = 0;
    dualGroupObject dgo(ID_FIRST_GROUP + i);
    uint pos = 0;
    if(groupSet.find(dgo, pos))
      go = groupSet[pos].group;
    else {
      go = manGroupObj.createGroup();
      dgo.group = go;
      groupSet.insert(dgo);
      }
    while(p) {
      uint id = _ttoi(p);
      if(id >= ID_FIRST_GROUP && id < ID_FIRST_GROUP + 2000) {
        groupObject* sub = 0;
        dualGroupObject dgo(id);
        if(groupSet.find(dgo, pos))
          sub = groupSet[pos].group;
        else {
          sub = manGroupObj.createGroup();
          dgo.group = sub;
          groupSet.insert(dgo);
          }
        go->addObject(sub);
        sub->setGroupSimple(go);
        }
      else {
        dualGroupObject dgo(go, id);
        objSet.insert(dgo);
        }
      p = findNextParamTrim(p);
      }
    }
}
//-----------------------------------------------------------
void createGroupObject::setGroup(svmBaseObject* obj, uint id)
{
  dualGroupObject dgo(id);
  uint pos;
  if(objSet.find(dgo, pos))
    obj->setGroup(objSet[pos].group);
}
