//-------- pInfoMoveObj.h ----------------------------------------------------
#ifndef pInfoMoveObj_H_
#define pInfoMoveObj_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_baseObj.h"
struct infoData
{
  prfData data;
  uint prph;
  uint norm;
  infoData() : prph(0), norm(0) {}
};
//----------------------------------------------------------------------------
enum eMoveDir { emdFromTopLeft, emdFromTopRight, emdFromBottomLeft, emdFromBottomRight };
//----------------------------------------------------------------------------
#define NOT_VALID_POINT -123456789
//----------------------------------------------------------------------------
class baseObj;
//----------------------------------------------------------------------------
class pInfoMoveObj
{
  public:
    pInfoMoveObj(baseObj* owner) : Owner(owner) { }
    bool init(LPCTSTR p);
    void updateMoving(bool& needRefresh);
    enum var { imo_minX, imo_minY, imo_maxX, imo_maxY, imo_currX, FIRST_NEEDED = imo_currX, imo_currY, MAX_MOVING_DATA };

    const infoData* getInfo() { return movData.Data; }
    const POINT& getLast() { return movData.last; }
    const POINT& getOrig() { return movData.orig; }
    void setOrig(const POINT& pt) { movData.orig = pt; }
  protected:
    struct moving_data
    {
      infoData Data[MAX_MOVING_DATA];
      SIZE sz;
      POINT orig;
      POINT last;
      eMoveDir dir;
      bool absoluteSize;
      moving_data() : dir(emdFromTopLeft), absoluteSize(true) {
        orig.x = NOT_VALID_POINT; orig.y = NOT_VALID_POINT;
        last.x = NOT_VALID_POINT; last.y = NOT_VALID_POINT;
        sz.cx = 0; sz.cy = 0;
        }
    };
    moving_data movData;
    baseObj* Owner;
    bool fillData(infoData& Data, int code);
};
//----------------------------------------------------------------------------
#endif
