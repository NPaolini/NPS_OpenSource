//-------- pInfoMoveObj.h ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "pInfoMoveObj.h"
#include "mainclient.h"
#include "newNormal.h"
#include <stdio.h>
//----------------------------------------------------------------------------
bool pInfoMoveObj::init(LPCTSTR p)
{
  if(!p)
    return false;

  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;
  int code[MAX_MOVING_DATA];
  int dir = 0;
  int absSize = 1;
  for(int i = 0; i < MAX_MOVING_DATA; ++i)
    code[i] = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
          &x1, &y1, &x2, &y2,
          &code[0], &code[1], &code[2], &code[3], &code[4], &code[5], &dir, &absSize);

  for(int i = FIRST_NEEDED; i < MAX_MOVING_DATA; ++i) {
    if(!code[i])
      return false;
    }
  movData.sz.cx = R__X(x2 - x1);
  movData.sz.cy = R__Y(y2 - y1);
  // il calcolo del rettangolo reale viene rimandato al primo utilizzo

  movData.dir = (eMoveDir)dir;
  movData.absoluteSize = toBool(absSize);
  for(int i = 0; i < MAX_MOVING_DATA; ++i) {
    if(!code[i])
      continue;
    if(!fillData(movData.Data[i], code[i]))
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
bool pInfoMoveObj::fillData(infoData& Data, int code)
{
  Data.prph = 0;
  manageObjId moi(code);
  uint id = moi.getFirstExtendId();
  LPCTSTR p = Owner->getOwner()->getPageString(id);
  if(!p)
    return false;

  int idprph = 0;
  DWDATA addr = 0;
  uint type = 0;
  DWDATA norm = 0;
  // il valore constante viene sempre salvato come (cast) a dword, type decide se è intero o float
  _stscanf_s(p, _T("%d,%u,%d,%d"), &idprph, &addr, &type, &norm);
  Data.data.typeVar = (prfData::tData)type;

  Data.data.lAddr = addr;
  Data.data.pAddr = 0;
  Data.norm = norm;
  Data.prph = idprph;
  return true;
}
//----------------------------------------------------------------------------
static
double getVal(infoData& Data)
{
  const prfData& data = Data.data;

  if(PRPH_4_CONST == Data.prph) {
    DWDATA t = data.lAddr;
    if(prfData::tDWData == data.typeVar)
      return (long)t;
    return *(float*)&t;
    }

  double norm = getNorm(Data.norm);
  switch(data.typeVar) {
    case prfData::tBData:
      return data.U.b * norm;

    case prfData::tBsData:
      return data.U.sb * norm;

    case prfData::tWData:
      return data.U.w * norm;

    case prfData::tWsData:
      return data.U.sw * norm;

    case prfData::tDWData:
      return data.U.dw * norm;

    case prfData::tDWsData:
      return data.U.sdw * norm;

    case prfData::tFRData:
      return data.U.fw * norm;

    case prfData::ti64Data:
      return (double)data.U.li.QuadPart * norm;

    case prfData::tRData:
      return data.U.rw * norm;

    default:
      return 0;
    }
}
//----------------------------------------------------------------------------
static
void getData(infoData& data)
{
  if(PRPH_4_CONST == data.prph)
    return;

  mainClient* Par = getMain();
  if(!Par)
    return;

  genericPerif* prph = Par->getGenPerif(data.prph);
  if(prph)
    prph->get(data.data);
}
//----------------------------------------------------------------------------
void pInfoMoveObj::updateMoving(bool& needRefresh)
{
  if(!Owner->canMove())
    return;
  if(NOT_VALID_POINT == movData.orig.x) {
    PRect r = Owner->getRectMove();
    if(!r.Width())
      return;
    if(movData.absoluteSize) {
      movData.sz.cx -= r.Width();
      movData.sz.cy -= r.Height();
      }
    movData.orig.x = r.left;
    movData.orig.y = r.top;
    }
  for(int i = 0; i < MAX_MOVING_DATA; ++i)
    getData(movData.Data[i]);

  enum { ID_MIN_X, ID_MIN_Y, ID_MAX_X, ID_MAX_Y, ID_CURR_X, ID_CURR_Y };

  double rangeX = getVal(movData.Data[ID_MAX_X]);
  double minX = getVal(movData.Data[ID_MIN_X]);
  rangeX -= minX;
  if(rangeX <= 0.0)
    rangeX = 1.0;

  double rangeY = getVal(movData.Data[ID_MAX_Y]);
  double minY = getVal(movData.Data[ID_MIN_Y]);
  rangeY -= minY;
  if(rangeY <= 0.0)
    rangeY = 1.0;

  double scaleX = movData.sz.cx / rangeX;
  double scaleY = movData.sz.cy / rangeY;
  POINT pt;
  pt.x = (LONG)((getVal(movData.Data[ID_CURR_X]) - minX) * scaleX);
  pt.y = (LONG)((getVal(movData.Data[ID_CURR_Y]) - minY) * scaleY);

  if(pt.x > movData.sz.cx)
    pt.x = movData.sz.cx;
  else if(pt.x < 0)
    pt.x = 0;

  if(pt.y > movData.sz.cy)
    pt.y = movData.sz.cy;
  else if(pt.y < 0)
    pt.y = 0;

  switch(movData.dir) {
    case emdFromTopLeft:
      break;
    case emdFromBottomRight:
      pt.y = movData.sz.cy - pt.y;
      // fall through
    case emdFromTopRight:
      pt.x = movData.sz.cx - pt.x;
      break;
    case emdFromBottomLeft:
      pt.y = movData.sz.cy - pt.y;
      break;
    }

  if(needRefresh || movData.last.x != pt.x || movData.last.y != pt.y) {
    PRect r = Owner->getRectMove();
    PRect r2(r);
    r.Offset(movData.last.x, movData.last.y);
    movData.last = pt;
    Owner->invalidate(r, true);
    pt.x += movData.orig.x;
    pt.y += movData.orig.y;
    r2.MoveTo(pt.x, pt.y);
    Owner->invalidate(r2, true);
    needRefresh = false;
    }
}
