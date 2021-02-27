//-------------------- svmHistory.cpp --------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmHistory.h"
#include "svmObject.h"
//-----------------------------------------------------------
static PRect getObjRect(svmBaseObject* obj)
{
  svmObject* o = dynamic_cast<svmObject*>(obj);
  if(o)
    return o->getRect();
  return PRect();
}
//-----------------------------------------------------------
static int getObjId(svmBaseObject* obj)
{
  svmObject* o = dynamic_cast<svmObject*>(obj);
  if(o)
    return o->getId();
  return 0;
}
//-----------------------------------------------------------
svmObjHistory::svmObjHistory(svmBaseObject* obj, svmObjHistory::typeOfAction action,
        svmBaseObject* prev) :
      Curr(obj), Action(action), Prev(prev), Rect(getObjRect(obj)),
      Id(getObjId(obj)), Next(0), Cloned(0)
{}
//-----------------------------------------------------------
svmObjHistory::~svmObjHistory() { if(Group != Action) delete Cloned; }
//-----------------------------------------------------------

