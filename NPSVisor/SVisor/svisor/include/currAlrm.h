//----------- curralrm.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef CURRALRM_H_
#define CURRALRM_H_
//----------------------------------------------------------------------------
#include "P_bits.h"
#include "setPack.h"
//----------------------------------------------------------------------------
struct fullInfoAlarm;
//----------------------------------------------------------------------------
#define DIRTY_BY_ALARM 0 // 1
//----------------------------------------------------------
struct infoActiveBitsAlarm
{
  int group;
  bool foundExist;
  bool foundActive;
  bool foundInactive;
  bool foundActiveNotAck;
  infoActiveBitsAlarm() : group(0), foundExist(false), foundActive(false), foundInactive(false), foundActiveNotAck(false)
  {}
};
//----------------------------------------------------------------------------
class currAlarm
{
  public:
    currAlarm();
    ~currAlarm();

    void initialize();

    enum alarmStat { asInactive, asActive, asActiveRecognized };
    void setTime(int id, bool start = true);
    bool setFirst();
    bool setNext();

    void get(fullInfoAlarm* idAlarm, SYSTEMTIME *st, alarmStat& stat);
    void get(fullInfoAlarm* idAlarm, FILETIME *ft, alarmStat& stat);

    bool find(long id, fullInfoAlarm* idAlarm, FILETIME *ft, alarmStat& stat);

    bool isDirty() { return Dirty.isSet(DIRTY_BY_ALARM); }
    void commit();

    bool isDirtyByObj(uint bitId) { return Dirty.isSet(bitId); }
    void commitByObj(uint bitId) { Dirty.clear(bitId); }

    bool isReverseOrder() const { return reverseOrder; }
    bool setOrder(bool setReverse) { bool old = reverseOrder; reverseOrder = setReverse; return old; }

    // se l'id è negativo rimuove tutti quelli inattivi
    void remove(int idAlarm);

    void reloadDescr();
    void resetAll();
  private:
    class container_id *CntId;
    class container_time *CntTime;
    P_Bits Dirty;
    int alsoNotActive;
    void setTimeWithoutActive(int id, bool start);
    void setTimeWithActive(int id, bool start);

    int matchCurrFilter();
    bool matchFilter();
    bool reverseOrder;
    void removeFromBits(int idAlarm);
    void setDirty();
    void findActiveGroup(int idAlarm, infoActiveBitsAlarm& result);
};
//----------------------------------------------------------------------------
currAlarm *getGestAlarm();
void reloadTextAlarm();
//----------------------------------------------------------------------------
class FilterAlarm
{
  public:
    virtual bool isSetBit(int offset) = 0;
    virtual bool isEmpty() = 0;
    virtual void setBit(int offset, bool set) = 0;
    virtual FilterAlarm* makeclone() = 0;
    virtual void copyAll(FilterAlarm* other, bool remove = true) = 0;

    virtual DWORD getDWordAt(uint offset) const = 0;
    virtual void setDWordAt(uint offset, DWORD val) = 0;

    virtual void clearAll() = 0;
};
//----------------------------------------------------------------------------
void setFilterAlarm(FilterAlarm* other, bool remove = true);
//----------------------------------------------------------------------------
FilterAlarm* getFilterAlarm();
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
