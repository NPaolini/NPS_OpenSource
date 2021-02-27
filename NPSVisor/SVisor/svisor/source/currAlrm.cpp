//------------ curralrm.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "1.h"
#include "currAlrm.h"
#include "p_avl.h"
#include "p_vect.h"
#include "p_name.h"
#include "p_date.h"
#include "p_bits.h"
#include "p_file.h"
#include "mainClient.h"
//----------------------------------------------------------
//    enum alarmStat { asInactive, asActive, asActiveRecognized };
//----------------------------------------------------------
struct currTimeAlarm : public genericSet
{
  currTimeAlarm() { } // per poter caricare da file
  currTimeAlarm(long ixAlarm) { id.splitId(ixAlarm); }
  currTimeAlarm(long ixAlarm, currAlarm::alarmStat stat) : stat(stat),
      ft(getFileTimeCurr()) { getTextAlarm(ixAlarm, id); }

  fullInfoAlarm id;
  FILETIME ft;
  currAlarm::alarmStat stat;

  bool save(P_File& pf);
  bool load(P_File& pf);
};
//----------------------------------------------------------
bool currTimeAlarm::save(P_File& pf)
{
  if(sizeof(ft) != pf.P_write(&ft, sizeof(ft)))
    return false;
  if(sizeof(stat) != pf.P_write(&stat, sizeof(stat)))
    return false;
  return id.save(pf);
}
//----------------------------------------------------------
bool currTimeAlarm::load(P_File& pf)
{
  if(sizeof(ft) != pf.P_read(&ft, sizeof(ft)))
    return false;
  if(sizeof(stat) != pf.P_read(&stat, sizeof(stat)))
    return false;
  return id.load(pf);
}
//----------------------------------------------------------
class container_id : public P_Avl
{
  public:
    container_id() : P_Avl(true) { }
  protected:
    virtual int Cmp(const TreeData ins, const TreeData nodo) const;
};
//----------------------------------------------------------
int container_id::Cmp(const TreeData ins,const TreeData nodo) const
{
  const currTimeAlarm *pIns = reinterpret_cast<const currTimeAlarm*>(ins);
  const currTimeAlarm *pNodo = reinterpret_cast<const currTimeAlarm*>(nodo);
  int diff = pIns->id.prph - pNodo->id.prph;
  if(diff)
    return diff;
  return pIns->id.alarm - pNodo->id.alarm;
}
//----------------------------------------------------------
//----------------------------------------------------------
// solo uno dei due deve deallocare la memoria per i dati
class container_time : public P_Avl
{
  public:
    container_time() : P_Avl(false) { }
  protected:
    virtual int Cmp(const TreeData ins,const TreeData nodo) const;
};
//----------------------------------------------------------
int container_time::Cmp(const TreeData ins,const TreeData nodo) const
{
  const currTimeAlarm *pIns = reinterpret_cast<const currTimeAlarm*>(ins);
  const currTimeAlarm *pNodo = reinterpret_cast<const currTimeAlarm*>(nodo);
  __int64 vIns = (reinterpret_cast<const LARGE_INTEGER*>(&pIns->ft))->QuadPart;
  __int64 vNodo = (reinterpret_cast<const LARGE_INTEGER*>(&pNodo->ft))->QuadPart;
  if(vIns > vNodo)
    return 1;
  if(vIns < vNodo)
    return -1;
  int diff = pIns->id.prph - pNodo->id.prph;
  if(diff)
    return diff;
  return pIns->id.alarm - pNodo->id.alarm;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
static currAlarm CurrAlarm;
//----------------------------------------------------------
currAlarm *getGestAlarm()
{
  return &CurrAlarm;
}
//----------------------------------------------------------
#define MAX_BIT_FILTER_BY_OBJ 32
//----------------------------------------------------------
#define MAX_BIT_FILTER 64
//----------------------------------------------------------
#define UNACKED _T("Unacked_alarm.dat")
//----------------------------------------------------------
currAlarm::currAlarm() :
  CntTime(new container_time), CntId(new container_id), Dirty(MAX_BIT_FILTER_BY_OBJ),
  alsoNotActive(-1), reverseOrder(true)
{
}
//----------------------------------------------------------
currAlarm::~currAlarm()
{
  delete CntTime;
  delete CntId;
}
//----------------------------------------------------------
void currAlarm::initialize()
{
  if(alsoNotActive < 0) {
    LPCTSTR p = getString(ID_SHOW_INACTIVE_ALARM);
    if(p && _ttoi(p))
      alsoNotActive = 1;
    else
      alsoNotActive = 0;
    }

  if(alsoNotActive) {
    P_File pf(UNACKED, P_READ_ONLY);
    if(pf.P_open()) {
      for(;;) {
        currTimeAlarm* cta = new currTimeAlarm;
        if(cta->load(pf)) {
          CntTime->Add(cta);
          CntId->Add(cta);
          }
        else {
          delete cta;
          break;
          }
        }
      }
    pf.P_close();
    }
  DeleteFile(UNACKED);
}
//----------------------------------------------------------
void currAlarm::resetAll()
{
  if(alsoNotActive && CntTime->setFirst()) {
    P_File pf(UNACKED, P_CREAT);
    if(pf.P_open()) {
      do {
        currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntTime->getCurr());
        // salviamo tutto ...
//        if(asActive == curr->stat)
          if(!curr->save(pf))
            break;
        } while(setNext());
      }
    }
  CntTime->Flush();
  CntId->Flush();
}
//----------------------------------------------------------
#if 1
//----------------------------------------------------------
class FilterAlarmImpl : public FilterAlarm
{
  public:
    FilterAlarmImpl() : Bits(MAX_BIT_FILTER) {}

    virtual bool isEmpty();
    virtual bool isSetBit(int offset);
    virtual void setBit(int offset, bool set);
    virtual FilterAlarm* makeclone();
    virtual void copyAll(FilterAlarm* other, bool remove);
    virtual DWORD getDWordAt(uint offset) const;
    virtual void setDWordAt(uint offset, DWORD val);
    virtual void clearAll() { Bits.clearAll(); }
  private:
    P_Bits Bits;
};
//----------------------------------------------------------
bool FilterAlarmImpl::isEmpty()
{
  return !Bits.hasSet();
}
//----------------------------------------------------------
bool FilterAlarmImpl::isSetBit(int offset)
{
  --offset;
  if(offset < 0)
    return false;
  return Bits.isSet(offset);
}
//----------------------------------------------------------
void FilterAlarmImpl::setBit(int offset, bool set)
{
  --offset;
  if(offset < 0)
    return;
  if(set)
    Bits.set(offset);
  else
    Bits.clear(offset);
}
//----------------------------------------------------------
FilterAlarm* FilterAlarmImpl::makeclone()
{
  return new FilterAlarmImpl(*this);
}
//----------------------------------------------------------
void FilterAlarmImpl::copyAll(FilterAlarm* other, bool remove)
{
  Bits = ((FilterAlarmImpl*)other)->Bits;
  if(remove)
    delete other;
}
//----------------------------------------------------------
void FilterAlarmImpl::setDWordAt(uint offset, DWORD val)
{
  Bits.setDWordAt(offset, val);
}
//----------------------------------------------------------
DWORD FilterAlarmImpl::getDWordAt(uint offset) const
{
  return Bits.getDWordAt(offset);
}
//----------------------------------------------------------
static FilterAlarmImpl FilterAlarmState;
//----------------------------------------------------------------------------
void setFilterAlarm(FilterAlarm* other, bool remove)
{
  FilterAlarmState.copyAll(other, remove);
}
//----------------------------------------------------------------------------
FilterAlarm* getFilterAlarm()
{
  return &FilterAlarmState;
}
//----------------------------------------------------------
void currAlarm::setDirty() { Dirty.setAll(); }
//----------------------------------------------------------
#else
//----------------------------------------------------------
static DWORD FilterAlarmState;
//----------------------------------------------------------
DWORD getFilterAlarm()
{
  return FilterAlarmState;
}
//----------------------------------------------------------
void setFilterAlarm(DWORD state)
{
  FilterAlarmState = state;
}
//----------------------------------------------------------
void currAlarm::setDirty() { Dirty = 0xffffffff; }
//----------------------------------------------------------
#endif
//----------------------------------------------------------
void currAlarm::reloadDescr()
{
  if(!setFirst())
    return;
  do {
    currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntTime->getCurr());
    long fullIx = curr->id.makeFullIdAlarm();
    getTextAlarm(fullIx, curr->id);
    } while(setNext());
}
//----------------------------------------------------------
int currAlarm::matchCurrFilter()
{
  FilterAlarm* filter = getFilterAlarm();
  if(filter->isEmpty())
    return 1;
  currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntTime->getCurr());
  int group = curr->id.idFlt;
  if(-1 == group)
    return 1;
//    return -1;
  if(filter->isSetBit(group))
    return 1;
  return 0;
}
//----------------------------------------------------------
bool currAlarm::matchFilter()
{
  do {
    int match = matchCurrFilter();
    if(match)
      return match > 0;
    }while(isReverseOrder() ? CntTime->setPrev() : CntTime->setNext());
  return false;
}
//----------------------------------------------------------
bool currAlarm::setFirst()
{
  if(reverseOrder) {
    if(!CntTime->setLast())
      return false;
    }
  else {
    if(!CntTime->setFirst())
      return false;
    }
  return matchFilter();
}
//----------------------------------------------------------
bool currAlarm::setNext()
{
  if(reverseOrder) {
    if(!CntTime->setPrev())
      return false;
    }
  else {
    if(!CntTime->setNext())
      return false;
    }
  return matchFilter();
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
void currAlarm::commit()
{
  bool set = Dirty.isSet(DIRTY_BY_ALARM);
  Dirty.clearAll();
  if(set)
    Dirty.set(DIRTY_BY_ALARM);
}
//----------------------------------------------------------
bool currAlarm::find(long id, fullInfoAlarm* idAlarm, FILETIME *ft, alarmStat& stat)
{
  currTimeAlarm t(id);
  bool found = CntId->Search(reinterpret_cast<TreeData>(&t));
  if(found) {
    currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntId->getCurr());
    idAlarm->copy(curr->id);
    *ft = curr->ft;
    stat = curr->stat;
    return true;
    }
  return false;
}
//----------------------------------------------------------
void currAlarm::get(fullInfoAlarm* idAlarm, FILETIME *ft, alarmStat& stat)
{
  currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntTime->getCurr());
  idAlarm->copy(curr->id);
  *ft = curr->ft;
  stat = curr->stat;
}
//----------------------------------------------------------
void currAlarm::get(fullInfoAlarm* idAlarm, SYSTEMTIME *st, currAlarm::alarmStat& stat)
{
  currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntTime->getCurr());
  idAlarm->copy(curr->id);
  FileTimeToSystemTime(&curr->ft, st);
  stat = curr->stat;
}
//----------------------------------------------------------
void currAlarm::setTimeWithoutActive(int id, bool start)
{
  currTimeAlarm t(id);
  bool found = CntId->Search(reinterpret_cast<TreeData>(&t));
  if(found) { // se già esiste
    if(start) // non occorre aggiungerlo
      return;
    TreeData rem = CntId->Rem(); // lo toglie dall'albero
      // deve toglierlo anche da quello ordinato per time
    if(!CntTime->Search(rem)) {
      // errore
      }
    else
      CntTime->Rem();
      // dealloca
    delete rem;
    }
  else { // non esisteva
    if(!start) // ??? non si può togliere quello che non c'è
      return;
    currTimeAlarm *add = new currTimeAlarm(id, asActive);
    if(!MK_I64(add->ft))
      delete add;
    else {
      CntId->Add(add);
      CntTime->Add(add);
      }
    }
  setDirty();
}
//----------------------------------------------------------
void currAlarm::setTimeWithActive(int id, bool start)
{
  currTimeAlarm t(id);
  bool found = CntId->Search(reinterpret_cast<TreeData>(&t));
  if(found) { // se già esiste
    currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntId->getCurr());
    if(start) {
      if(asActive == curr->stat)
        return;
#if 1 // per potergli far ricaricare il valore (se associato)
      setTimeWithoutActive(id, false);
      found = false;
#else
      curr->stat = asActive;
#endif
      }
    else {
      if(asInactive == curr->stat)
        return;
      if(asActiveRecognized == curr->stat)
        setTimeWithoutActive(id, start);
      else
        curr->stat = asInactive;
      }
    }
  if(!found) { // non esisteva
    if(!start) // ??? non si può togliere quello che non c'è
      return;
    currTimeAlarm *add = new currTimeAlarm(id, asActive);
    if(!MK_I64(add->ft))
      delete add;
    else {
      CntId->Add(add);
      CntTime->Add(add);
      }
    }
  setDirty();
}
//----------------------------------------------------------
extern void updateGroupBits(DWORD group, bool set); // presenza nel set
extern void updateGroupBits2(DWORD group, bool set);// attivo all'arrivo dell'allarme, reset manuale
extern void updateGroupBits3(DWORD group, bool set);// solo allarmi attivi
extern void updateGroupBits4(DWORD group, bool set);// solo allarmi attivi ma non riconosciuti
extern void updateGroupBits5(DWORD group, bool set);// solo allarmi non attivi ma non riconosciuti
//----------------------------------------------------------
void currAlarm::findActiveGroup(int idAlarm, infoActiveBitsAlarm& result)
{
  result.group = getAlarmFilterCode(idAlarm);
  if(result.group > 0) {
    if(CntId->setFirst()) {
      do {
        currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntId->getCurr());
        if(result.group == curr->id.idFlt) {
          result.foundExist = true;
          if(asInactive == curr->stat)
            result.foundInactive = true;
          else if(asActiveRecognized == curr->stat)
            result.foundActive = true;
          else if(asActive == curr->stat) {
            result.foundActiveNotAck = true;
            result.foundActive = true;
            }
          if(result.foundInactive && result.foundActive && result.foundActiveNotAck)
            break;
          }
        } while(CntId->setNext());
      }
    }
  --result.group;
 }
//----------------------------------------------------------
void currAlarm::setTime(int id, bool start)
{
/* spostato nella initialize()
  if(alsoNotActive < 0) {
    LPCTSTR p = getString(ID_SHOW_INACTIVE_ALARM);
    if(p && _ttoi(p))
      alsoNotActive = 1;
    else
      alsoNotActive = 0;
    }
*/
  if(alsoNotActive)
    setTimeWithActive(id, start);
  else
    setTimeWithoutActive(id, start);

  int group = getAlarmFilterCode(id) - 1;
  if(group >= 0 ) {
    updateGroupBits2(group, start);
    if(start) {
      updateGroupBits(group, start);
      updateGroupBits3(group, start);
      updateGroupBits4(group, start);
      }
    else
      removeFromBits(id);
    }
}
//----------------------------------------------------------
void currAlarm::removeFromBits(int idAlarm)
{
  infoActiveBitsAlarm iaba;
  findActiveGroup(idAlarm, iaba);
  if(!iaba.foundExist) {
    updateGroupBits(iaba.group, false);
    updateGroupBits3(iaba.group, false);
    updateGroupBits4(iaba.group, false);
    updateGroupBits5(iaba.group, false);
    }
  else {
    updateGroupBits5(iaba.group, iaba.foundInactive);
    if(!iaba.foundActive)
      updateGroupBits3(iaba.group, false);
    if(!iaba.foundActiveNotAck)
      updateGroupBits4(iaba.group, false);
    }
}
//----------------------------------------------------------
void currAlarm::remove(int idAlarm)
{
  if(idAlarm < 0) {
    if(setFirst()) {
      PVect<currTimeAlarm*> set;
      int count = 0;
      do {
        currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntTime->getCurr());
        if(asInactive == curr->stat)
          set[count++] = curr;
        } while(setNext());
      if(count) {
        for(int i = 0; i < count; ++i) {
          if(!CntId->Search(set[i])) {
            // errore
            }
          else {
            TreeData rem = CntId->Rem(); // lo toglie dagli alberi
            if(!CntTime->Search(set[i])) {
              // errore
              }
            else
              CntTime->Rem();
            removeFromBits(set[i]->id.makeFullIdAlarm());
            delete rem;
            }
          }
        setDirty();
        }
      }
    }
  else {
    currTimeAlarm t(idAlarm);
    bool found = CntId->Search(reinterpret_cast<TreeData>(&t));
    if(found) {
      currTimeAlarm* curr = reinterpret_cast<currTimeAlarm*>(CntId->getCurr());
      if(asActive == curr->stat) {
        curr->stat = asActiveRecognized;
        setDirty();

        infoActiveBitsAlarm iaba;
        findActiveGroup(idAlarm, iaba);
        if(!iaba.foundActiveNotAck)
          updateGroupBits4(iaba.group, false);
        mainClient* mc = getMain();
        if(mc) {
          val_gen v;
          v.id = idAlarm - 1;
          v.val = rALL;
          mc->setEvent(log_status::onAckAlarm, &v);
          }
        }
      else if(asInactive == curr->stat) {
        TreeData rem = CntId->Rem(); // lo toglie dall'albero
        // deve toglierlo anche da quello ordinato per time
        if(!CntTime->Search(rem)) {
          // errore
          }
        else
          CntTime->Rem();
          // dealloca
        delete rem;
        setDirty();
        removeFromBits(idAlarm);
        mainClient* mc = getMain();
        if(mc) {
          val_gen v;
          v.id = idAlarm - 1;
          v.val = rALL;
          mc->setEvent(log_status::onAckAlarm, &v);
          }
        }
      }
    }
}
//----------------------------------------------------------------------------
void reloadTextAlarm()
{
  currAlarm *Al = getGestAlarm();
  Al->reloadDescr();
}
//----------------------------------------------------------

