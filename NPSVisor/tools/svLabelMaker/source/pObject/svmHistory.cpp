//-------------------- svmHistory.cpp --------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmHistory.h"
#include "svmObject.h"
//-----------------------------------------------------------
svmObjHistory::svmObjHistory(svmObject* obj, svmObjHistory::typeOfAction action,
        svmObject* prev) :
      Curr(obj), Action(action), Prev(prev), Rect(obj->getRect()),
      Id(obj->getId()), Next(0), Cloned(0)
{}
//-----------------------------------------------------------
svmObjHistory::~svmObjHistory() { delete Cloned; }
//-----------------------------------------------------------

