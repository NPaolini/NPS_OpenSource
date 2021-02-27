//------------------ svmBase.cpp ----------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//-----------------------------------------------------------
#include "svmBase.h"
#include "p_file.h"
#include "svmHistory.h"
#include "common.h"
#include "PDiagAd4.h"
#include "PDiagOffset.h"
#include "PDiagCheckaddr.h"
//-----------------------------------------------------------
#define RCAST_OBJ(a) (reinterpret_cast<svmObject*>(a))
#define DCAST_OBJ(a) (dynamic_cast<svmObject*>(a))
//-----------------------------------------------------------
static int priority[] = {
          oSPIN,
          oEDIT,
          oBUTTON,
          oSLIDER,
          oLISTBOX,
          oCHOOSE,
          oCAM,
          oCURVE,
          oLB_ALARM,
          oTREND,

          oLED,
          oBARGRAPH,
          oTEXT,
          oXSCOPE,
          oPLOT_XY,
          oXMETER,
          oDIAM,
          oTBL_INFO,
          oBITMAP,

          oSIMPLE_TEXT,
          oPANEL,
          oBKG_BITMAP,
          };
//-----------------------------------------------------------
#define NUM_PRIORITY SIZE_A_c(priority)
//-----------------------------------------------------------
static allowSelect AllowSelected;
//-----------------------------------------------------------
allowSelect& getAllow()
{
  return AllowSelected;
}
//-----------------------------------------------------------
class splitByPriority
{
  public:
    splitByPriority(svmListObj& list, allowSelect::allow flag = allowSelect::asOtherThenBkg);

    svmObject* getCurr();  // ritorna il dato corrente

    bool setFirst();   // setta il corrente al primo
    bool setLast();    // setta il corrente all'ultimo
    bool setNext();    // al seguente
    bool setPrev();    // al precedente
    int getCurrNElem() const;
  private:
    PVect<svmObject*> Set[NUM_PRIORITY];
    int currSet;
    int currObject;

    void allowAll(svmListObj& list);
    void allowBkg(svmListObj& list);
    void allowOtherThenBkg(svmListObj& list);
    void allowNoBkgAndLocked(svmListObj& list);
};
//-----------------------------------------------------------
splitByPriority::splitByPriority(svmListObj& list, allowSelect::allow flag) :
        currSet(-1), currObject(-1)
{
  if(list.setFirst()) {
    switch(flag) {
      case allowSelect::asOnlyBkg:
        allowBkg(list);
        break;

      case allowSelect::asOtherThenBkg:
        allowOtherThenBkg(list);
        break;
      case allowSelect::asNoBkgAndLocked:
        allowNoBkgAndLocked(list);
        break;

      case allowSelect::asAllObject:
        allowAll(list);
        break;
      }
    }
}
//-----------------------------------------------------------
void  splitByPriority::allowOtherThenBkg(svmListObj& list)
{
  do {
    svmObject* t = RCAST_OBJ(list.getCurr());
    if(!t)
      break;
    int pos = t->getIdType();
    if(oBKG_BITMAP != pos) {
      if((uint)pos < NUM_PRIORITY) {
        int nElem = Set[pos].getElem();
        Set[pos][nElem] = t;
        }
      }
    } while(list.setNext());
}
//-----------------------------------------------------------
void  splitByPriority::allowNoBkgAndLocked(svmListObj& list)
{
  do {
    svmObject* t = RCAST_OBJ(list.getCurr());
    if(!t)
      break;
    int pos = t->getIdType();
    if(!t->isLocked() && t->isVisible() && oBKG_BITMAP != pos) {
      int nElem = Set[pos].getElem();
      Set[pos][nElem] = t;
      }
    } while(list.setNext());
}
//-----------------------------------------------------------
void  splitByPriority::allowBkg(svmListObj& list)
{
  do {
    svmObject* t = RCAST_OBJ(list.getCurr());
    if(!t)
      break;
    int pos = t->getIdType();
    if(oBKG_BITMAP == pos) {
      int nElem = Set[pos].getElem();
      Set[pos][nElem] = t;
      }
    } while(list.setNext());
}
//-----------------------------------------------------------
void  splitByPriority::allowAll(svmListObj& list)
{
  do {
    svmObject* t = RCAST_OBJ(list.getCurr());
    if(!t)
      break;
    int pos = t->getIdType();
    if((uint)pos < NUM_PRIORITY) {
      int nElem = Set[pos].getElem();
      Set[pos][nElem] = t;
      }
    } while(list.setNext());
}
//-----------------------------------------------------------
int splitByPriority::getCurrNElem() const
{
  if((uint)currSet >= NUM_PRIORITY || currObject < 0)
    return 0;
  int currPriority = priority[currSet];
  return Set[currPriority].getElem();
}
//-----------------------------------------------------------
svmObject* splitByPriority::getCurr()
{
  if((uint)currSet >= NUM_PRIORITY || currObject < 0)
    return 0;
  int currPriority = priority[currSet];
  int nElem = Set[currPriority].getElem();
  if(currObject >= nElem)
    return 0;
  return Set[currPriority][currObject];
}
//-----------------------------------------------------------
bool splitByPriority::setFirst()
{
  currSet = 0;
  currObject = 0;
  if(!toBool(getCurr()))
    return setNext();

  return true;
}
//-----------------------------------------------------------
bool splitByPriority::setLast()
{
  currSet = NUM_PRIORITY - 1;
  int currPriority = priority[currSet];
  int nElem = Set[currPriority].getElem();
  currObject = nElem - 1;
  if(!toBool(getCurr()))
    return setPrev();

  return true;
}
//-----------------------------------------------------------
bool splitByPriority::setNext()
{
  if((uint)currSet >= NUM_PRIORITY || currObject < 0)
    return false;
  while(true) {
    ++currObject;
    int currPriority = priority[currSet];
    int nElem = Set[currPriority].getElem();
    if(currObject < nElem)
      return true;
    ++currSet;
    currObject = -1;
    if((uint)currSet >= NUM_PRIORITY) {
      currSet = -1;
      return false;
      }
    }
}
//-----------------------------------------------------------
bool splitByPriority::setPrev()
{
  while(true) {
    --currObject;
    if(currObject >= 0)
      return true;
    --currSet;
    if(currSet < 0)
      return false;

    int currPriority = priority[currSet];
    currObject = Set[currPriority].getElem();
    }
}
//-----------------------------------------------------------
svmListObj::svmListObj() : P_List_Gen(true), nElem(0) {}
//-----------------------------------------------------------
bool svmListObj::Add(ListData d)
{
  if(baseClass::Add(d)) {
    ++nElem;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
bool svmListObj::atEnd(ListData d)
{
  if(baseClass::atEnd(d)) {
    ++nElem;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
bool svmListObj::insert(ListData d, bool before)
{
  int old = nElem;
  if(baseClass::insert(d, before)) {
    if(old == nElem)
      ++nElem;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
bool svmListObj::insert(ListData d, bool before, pList nearTo)
{
  if(baseClass::insert(d, before, nearTo)) {
    // non serve il controllo, serviva per la Add(), ma non viene richiamata da questa insert
    ++nElem;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
ListData svmListObj::Rem()
{
  ListData t = baseClass::Rem();
  if(t)
    --nElem;
  return t;
}
//-----------------------------------------------------------
void svmListObj::freeData(ListData d)
{
  delete RCAST_OBJ(d);
}
//-----------------------------------------------------------
int svmListObj::cmp(ListData a, ListData b)
{
  return RCAST_OBJ(a)->getId() - RCAST_OBJ(b)->getId();
}
//-----------------------------------------------------------
PListHistory::PListHistory() : P_List_Gen(true) {}
//-----------------------------------------------------------
#define RCAST_HISTORY(a) (reinterpret_cast<svmObjHistory*>(a))
#define DCAST_HISTORY(a) (dynamic_cast<svmObjHistory*>(a))
//-----------------------------------------------------------
void PListHistory::freeData(ListData d)
{
  svmObjHistory* oh = RCAST_HISTORY(d);
  // se nello storico è rimasta una lista di oggetti deleted
  if(svmObjHistory::Rem == oh->getAction()) {
    do {
      delete oh->getObj();
      svmObjHistory* th = oh->getNext();
      delete oh;
      oh = th;
      } while(oh);
    }
  else
    delete oh;
}
//-----------------------------------------------------------
int PListHistory::cmp(ListData a, ListData b)
{
  return RCAST_HISTORY(a)->getId() - RCAST_HISTORY(b)->getId();
}
//-----------------------------------------------------------
PVect<svmObject*> svmBase::selected4Paste;
bool svmBase::selectedFromBkg;
//-----------------------------------------------------------
svmBase::svmBase(PWin* owner) : Owner(owner), LinkedStat(0), oldBtnStat(0), BmpBkg(0),
    hBmpWork(0), mdcWork(0)
{
}
//-----------------------------------------------------------
svmBase::~svmBase()
{
  flushSelected4Paste();
  delete BmpBkg;
  freeBkg();
}
//-----------------------------------------------------------
void svmBase::freeBkg()
{
  if(hBmpWork) {
    SelectObject(mdcWork, oldObj);
    DeleteDC(mdcWork);
    DeleteObject(hBmpWork);
    hBmpWork = 0;
    }
}
//-----------------------------------------------------------
void svmBase::makeBkg()
{
  freeBkg();
  HDC hdc = GetDC(*Owner);
  PRect r;
  GetClientRect(*Owner, r);
  getDataProject().calcFromScreen(r);

  hBmpWork = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
  if(hBmpWork) {
    mdcWork = CreateCompatibleDC(hdc);
    oldObj = SelectObject(mdcWork, hBmpWork);
    }
  ReleaseDC(*Owner, hdc);
}
//-----------------------------------------------------------
void svmBase::setBkg(PBitmap* newBkg)
{
  delete BmpBkg;
  BmpBkg = newBkg;
  freeBkg();
}
//-----------------------------------------------------------
void svmBase::flushSelected4Paste()
{
  flushPV(selected4Paste);
}
//-----------------------------------------------------------
bool svmBase::canPaste()
{
  return toBool(selected4Paste.getElem());
}
//-----------------------------------------------------------
struct dualObjGrp
{
  groupObject* from;
  groupObject* to;
  dualObjGrp(groupObject* from = 0, groupObject* to = 0) : from(from), to(to) {}
};
//-----------------------------------------------------------
static void createNewGroup(PVect<dualObjGrp>& set, svmBaseObject* obj)
{
  if(!obj->getGroup())
    return;
  uint nElem = set.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(obj->getGroup() == set[i].from)
      return;

  dualObjGrp dog(obj->getGroup());
  set[nElem] = dog;
}
//-----------------------------------------------------------
static void resolveNewGroup(PVect<dualObjGrp>& set)
{
  PVect<dualObjGrp> setUp;
  uint nElem = set.getElem();
  manageGroupObject& mgo = getManageGroupObject();
  for(uint i = 0; i < nElem; ++i)
    set[i].to = mgo.createGroup();

  for(uint i = 0; i < nElem; ++i) {
    groupObject* grpFrom = set[i].from;
    groupObject* grpTo = set[i].to;
    while(grpFrom->getGroup()) {
      groupObject* grp = grpFrom->getGroup();
      bool done = false;
      uint nEl = setUp.getElem();
      for(uint j = 0; j < nEl; ++j) {
        if(grp == setUp[j].from) {
          setUp[j].to->addObject(grpTo);
          grpTo = setUp[j].to;
          done = true;
          break;
          }
        }
      if(!done) {
        for(uint j = 0; j < nElem; ++j) {
          if(grp == set[j].from) {
            set[j].to->addObject(grpTo);
            grpTo = set[j].to;
            done = true;
            break;
            }
          }
        if(!done) {
          dualObjGrp dog(grp, mgo.createGroup());
          setUp[nEl] = dog;
          dog.to->addObject(grpTo);
          grpTo = dog.to;
          }
        }
      grpFrom = grp;
      }
    }
  uint nEl = setUp.getElem();
  for(uint i = nElem, j = 0; j < nEl; ++j, ++i)
    set[i] = setUp[j];
}
//-----------------------------------------------------------
static void setNewGroup(const PVect<dualObjGrp>& set, svmBaseObject* obj)
{
  if(!obj->getGroup())
    return;
  uint nElem = set.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(obj->getGroup() == set[i].from) {
      set[i].to->addObject(obj);
      break;
      }
}
//-----------------------------------------------------------
void svmBase::pasteCopied(HDC hdc)
{
  int nElem = selected4Paste.getElem();
  if(!nElem)
    return;

  if(List.setFirst()) {
    do {
      svmObject *t = RCAST_OBJ(List.getCurr());
      t->Unselect(hdc);
      } while(List.setNext());
    }

  PVect<dualObjGrp> grpSet;
  for(int i = 0; i < nElem; ++i)
    createNewGroup(grpSet, selected4Paste[i]);

  resolveNewGroup(grpSet);

  svmObjHistory* h = 0;
  RedoHistory.Flush();
  for(int i = 0; i < nElem; ++i) {
    svmObject* t = selected4Paste[i]->makeClone();
    t->resolveFont(selected4Paste[i]->getSaved());
    t->setGroupSimple(selected4Paste[i]->getGroup());
    setNewGroup(grpSet, t);

    if(List.Push(t)) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Add, 0);
      if(!h)
        History.Push(th);
      else
        h->setNext(th);
      h = th;
      t->standBySel(true);
      svmMainClient* mc = getMainClient(Owner);
      t->chgId(mc->getIncrementGlobalId());
      }
    }
  invalidate();
}
//-----------------------------------------------------------
void svmBase::copySelected()
{
  flushSelected4Paste();
  int c = 0;
  if(List.setLast()) {
    do {
      svmObject* t = RCAST_OBJ(List.getCurr());
      if(!t)
        break;
      if(t->isSelected()) {
        svmObject* p = t->makeClone();
        p->saveInfoFont();
        p->setGroupSimple(t->getGroup());
        selected4Paste[c] = p;
        ++c;
        }
      } while(List.setPrev());
    }
  selectedFromBkg = toBool(getBtnStat() & 4);
}
//-----------------------------------------------------------
void svmBase::deleteSelected()
{
  if(List.setFirst()) {
    bool isFirst;
    svmObjHistory* h = 0;
    RedoHistory.Flush();
    do {
      svmObject* t = RCAST_OBJ(List.getCurr());
      if(!t)
        break;
      isFirst = false;
      if(t->isSelected()) {
        svmObject* prev = 0;
        if(List.setPrev()) {
          prev = RCAST_OBJ(List.getCurr());
          List.setNext();
          }
        svmObject *obj = RCAST_OBJ(List.Rem());
        svmObjHistory* th = obj->makeHistoryObject(svmObjHistory::Rem, prev);
        if(!h)
          History.Push(th);
        else
          h->setNext(th);
        h = th;

        if(!List.setPrev())
          isFirst = true;
        }
      } while(isFirst || List.setNext());
    if(h)
      invalidate();
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmBase::addObjectLinked(svmObject* obj)
{
  ListLinked.Push(obj);
  if(!LinkedStat)
    LinkedStat = 1;
}
//-----------------------------------------------------------
void svmBase::addObject(svmObject* obj, HDC hdc)
{
  if(List.Push(obj)) {
    RedoHistory.Flush();
    svmObjHistory* h = obj->makeHistoryObject(svmObjHistory::Add, 0);
    History.Push(h);
    List.setFirst();
    do {
      svmObject *t = RCAST_OBJ(List.getCurr());
      t->Unselect(hdc);
      } while(List.setNext());
    obj->standBySel(true);
    invalidate();
    }
}
//-----------------------------------------------------------
//    enum action { none, select, moving, sizing };

int svmBase::findPoint(HDC hdc, const POINT&pt, int keyFlag)
{
  if(!List.setFirst())
    return select;
  bool add = needAddingObject(keyFlag);
  if(add)
    return findPointAdd(hdc, pt);
  if(notUseGroup())
    return oldFindPoint(hdc, pt);

  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return select;

  int selected = 0;

  // preverifica:
  //  1) se il click avviene all'interno dell'oggetto
  //  2) punto sensibile
  //  3) quanti sono attualmente selezionati
  //  4) il primo oggetto selezionato su cui avviene il click

  svmObject* firstSelected = 0;
  svmObject::typeOfAnchor anchor = svmObject::Extern;
  // esegue prima un controllo per verificare quanti sono
  // selezionati e se il punto scelto cade in un punto sensibile
  do {
    svmObject *t = sbp.getCurr();
    if(!t->isVisible() || t->isLocked())
      continue;

    // se l'oggetto corrente è selezionato, incrementa il contatore
    // e verifica se è il primo ad essere puntato
    if(svmObject::yesSel == t->isSelected()) {
      ++selected;

      if(!firstSelected) {
        // se non ancora trovato, verifica se il click è interno
        svmObject::typeOfAnchor anchor2 = t->pointIn(pt);
        if(svmObject::Extern != anchor2) {
          firstSelected = t;
          if(firstSelected->getGroup()) {
            anchor = svmObject::Inside;
            t->setAnchor(anchor, pt);
            }
          else if(svmObject::Extern == anchor)
            anchor = anchor2;
          }
        }
      }
    // se non ancora trovato un click interno
    if(svmObject::Extern == anchor) {
      anchor = t->pointIn(pt);
      }
    } while(sbp.setNext());

  // se non tocca nessun punto, si deselezionano quelli attivi
  // si può usare la lista piena anziché la derivata
  if(svmObject::Extern == anchor) {
    if(selected) {
      List.setFirst();
      do {
        svmObject *t = RCAST_OBJ(List.getCurr());
        t->Unselect(hdc);
        --selected;
        } while(selected > 0 && List.setNext());
      }
    return select;
    }

  sbp.setFirst();
  int result = select;
  bool deselectOther = false;
  bool activeInside = false;
  bool forceMove = firstSelected && firstSelected->getGroup();
  do {
    svmObject *t = sbp.getCurr();
    if(!t->isVisible() || t->isLocked())
      continue;

    if(deselectOther) {
      if(!t->sameGroup(firstSelected))
        t->Unselect(hdc);
      }
    else if(activeInside) {
      if(svmObject::yesSel == t->isSelected())
        t->setAnchor(svmObject::Inside, pt);
      }

    else {
      svmObject::typeOfAnchor anchor2 = t->pointIn(pt);
      if(svmObject::Extern == anchor2) {
        if(svmObject::yesSel == t->isSelected()) {
          if(!firstSelected || svmObject::Inside != anchor && !t->sameGroup(firstSelected)) {
              t->Unselect(hdc);
            }
          else
            t->setAnchor(svmObject::Inside, pt);
          }
        }
      // il click è interno, ora occorre verificare:
      // se era selezionato si setta il codice di ritorno in base al punto
      // scelto e si attiva l'azione di modificare l'attributo di
      // inside a tutti gli altri selezionati o di deselezione,
      // se non era selezionato lo si seleziona e si deselezionano gli altri
      else {
        if(svmObject::yesSel == t->isSelected()) {
         if(forceMove || svmObject::Inside == anchor) {
            result = moving;
            t->setAnchor(anchor, pt);
            activeInside = true;
            }
          else {
            result = sizing;
            deselectOther = true;
            }
          }
        else {
          if(firstSelected)
            continue;
          else {
            t->Select(hdc);
            firstSelected = t;
            forceMove = toBool(firstSelected->getGroup());
            if(forceMove) {
              t->setAnchor(svmObject::Inside, pt);
              result = moving;
              }
            else {
              switch(anchor) {
                case svmObject::Inside:
                  result = moving;
                  break;
                default:
                  result = select;
                  break;
                }
              }
            deselectOther = true;
            }
          }
        }
      }
    } while(sbp.setNext());
  return result;
}
//--------------------------------------------------------------
int svmBase::oldFindPoint(HDC hdc, const POINT& pt)
{
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return select;

  int selected = 0;

  // preverifica:
  //  1) se il click avviene all'interno dell'oggetto
  //  2) punto sensibile
  //  3) quanti sono attualmente selezionati
  //  4) il primo oggetto selezionato su cui avviene il click

  svmObject* firstSelected = 0;
  svmObject::typeOfAnchor anchor = svmObject::Extern;
  // esegue prima un controllo per verificare quanti sono
  // selezionati e se il punto scelto cade in un punto sensibile
  do {
    svmObject *t = sbp.getCurr();
    if(!t->isVisible() || t->isLocked())
      continue;

    // se l'oggetto corrente è selezionato, incrementa il contatore
    // e verifica se è il primo ad essere puntato
    if(svmObject::yesSel == t->isSelected()) {
      ++selected;

      if(!firstSelected) {
        // se non ancora trovato, verifica se il click è interno
        svmObject::typeOfAnchor anchor2 = t->pointIn(pt);
        if(svmObject::Extern != anchor2) {
          firstSelected = t;
          if(svmObject::Extern == anchor)
            anchor = anchor2;
          }
        }
      }
    // se non ancora trovato un click interno
    if(svmObject::Extern == anchor) {
      anchor = t->pointIn(pt);
      }
    } while(sbp.setNext());

  // se non tocca nessun punto, si deselezionano quelli attivi
  // si può usare la lista piena anziché la derivata
  if(svmObject::Extern == anchor) {
    if(selected) {
      List.setFirst();
      do {
        svmObject *t = RCAST_OBJ(List.getCurr());
        t->Unselect(hdc);
        --selected;
        } while(selected > 0 && List.setNext());
      }
    return select;
    }

  sbp.setFirst();
  int result = select;
  bool deselectOther = false;
  bool activeInside = false;
  do {
    svmObject *t = sbp.getCurr();
    if(!t->isVisible() || t->isLocked())
      continue;

    if(deselectOther)
      t->Unselect(hdc);

    else if(activeInside) {
      if(svmObject::yesSel == t->isSelected())
        t->setAnchor(svmObject::Inside, pt);
      }

    else {
      svmObject::typeOfAnchor anchor2 = t->pointIn(pt);
      if(svmObject::Extern == anchor2) {
        if(svmObject::yesSel == t->isSelected()) {
          if(!firstSelected || svmObject::Inside != anchor)
            t->Unselect(hdc);
          else
            t->setAnchor(svmObject::Inside, pt);
          }
        }
      // il click è interno, ora occorre verificare:
      // se era selezionato si setta il codice di ritorno in base al punto
      // scelto e si attiva l'azione di modificare l'attributo di
      // inside a tutti gli altri selezionati o di deselezione,
      // se non era selezionato lo si seleziona e si deselezionano gli altri
      else {
        if(svmObject::yesSel == t->isSelected()) {
          if(svmObject::Inside == anchor) {
            result = moving;
            t->setAnchor(svmObject::Inside, pt);
            activeInside = true;
            }
          else {
            result = sizing;
            deselectOther = true;
            }
          }
        else {
          if(firstSelected)
            continue;
          else {
            t->Select(hdc);
            switch(anchor) {
              case svmObject::Inside:
                result = moving;
                break;
              default:
                result = select;
                break;
              }
            deselectOther = true;
            }
          }
        }
      }
    } while(sbp.setNext());
  return result;
}
//--------------------------------------------------------------
int svmBase::findPointAdd(HDC hdc, const POINT&pt)
{
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return select;

  int result = select;
  do {
    svmObject *t = sbp.getCurr();
    if(!t->isVisible() || t->isLocked())
      continue;
    svmObject::typeOfAnchor anchor = t->pointIn(pt);
    if(svmObject::Extern != anchor) {
        // se era selezionato
      if(svmObject::yesSel == t->isSelected())
        t->Unselect(hdc);
      else
        t->Select(hdc);
      result = none;
      break;
      }
    } while(sbp.setNext());
  return result;
}
//-----------------------------------------------------------
// se è premuto lo shift seleziona tutto ciò che interseca il rettangolo
// indipendentemente se era già selezionato o no
// se non è premuto lo shift non dovrebbe esserci niente già selezionato

void svmBase::findObjects(const PRect& frame, int keyFlag)
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;

  bool add = needAddingObject(keyFlag);
  do {
    svmObject *t = sbp.getCurr();
    if(!t->isVisible() || t->isLocked())
        continue;
    if(frame.Intersec(t->getRect()))
      t->standBySel(true);
    else if(!add && svmObject::yesSel == t->isSelected())
      t->standBySel(false);
    } while(sbp.setNext());
}
//-----------------------------------------------------------
void svmBase::showSelected(HDC hdc)
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::signalAdd == t->isSelected())
      t->Select(hdc);
    else if(svmObject::signalRemove == t->isSelected())
      t->Unselect(hdc);
    } while(sbp.setNext());
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void svmBase::unselectAll(HDC hdc)
{
  if(!List.setFirst())
    return;
  do  {
    svmObject* t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected())
      t->Unselect(hdc);
    } while(List.setNext());
  invalidate();
}
//-----------------------------------------------------------
void svmBase::drawXorBlock(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected())
      t->Drag(hdc, pt);
    } while(sbp.setNext());
}
//-----------------------------------------------------------
void svmBase::drawXorBlockFinal(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  RedoHistory.Flush();
  svmObjHistory* h = 0;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Move);
      if(t->endDrag(hdc, pt)) {
        if(!h)
          History.Push(th);
        else
          h->setNext(th);
        h = th;
        }
      else
        delete th;
      }
    } while(sbp.setNext());

  invalidate();
}
//-----------------------------------------------------------
void svmBase::drawXorObject(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      t->Drag(hdc, pt);
      break;
      }
    } while(sbp.setNext());

}
//-----------------------------------------------------------
void svmBase::drawXorObjectFinal(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  RedoHistory.Flush();
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Size);
      if(t->endDrag(hdc, pt))
        History.Push(th);
      else
        delete th;
      break;
      }
    } while(sbp.setNext());

  invalidate();
}
//-----------------------------------------------------------
void svmBase::drawResizeAllObject(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      t->DragOnResizeByKey(hdc, pt);
      }
    } while(sbp.setNext());

}
//-----------------------------------------------------------
void svmBase::drawResizeAllObjectFinal(HDC hdc)
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;

  RedoHistory.Flush();
  svmObjHistory* h = 0;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Size);
      if(t->endDragOnResizeByKey(hdc)) {
        if(!h)
          History.Push(th);
        else
          h->setNext(th);
        h = th;
        }
      else
        delete th;
      }
    } while(sbp.setNext());


  invalidate();
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmBase::getAnchor(const POINT& pt, LPTSTR tips, int size, bool& changed, bool& onSelected)
{
  static svmObject* old = 0;
  changed = false;
  onSelected = false;
  svmObject::typeOfAnchor anchor = svmObject::Extern;
  if(!List.setFirst())
    return anchor;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return anchor;
  do  {
    svmObject* t = sbp.getCurr();
    if(!t)
      break;
    if(t->isSelected() && t->isVisible() && !t->isLocked()) {
      svmObject::typeOfAnchor whichPoint = t->pointIn(pt);
      if(svmObject::Extern != whichPoint) {
        // inserire tips
        t->fillTips(tips, size);
        if(old != t) {
          old = t;
          changed = true;
          }
        onSelected = true;
        anchor = whichPoint;
        break;
        }
      }
    } while(sbp.setNext());
  if(svmObject::Extern == anchor) {
    sbp.setFirst();
    do  {
      svmObject* t = sbp.getCurr();
      if(!t->isVisible() || t->isLocked())
        continue;
      svmObject::typeOfAnchor whichPoint = t->pointIn(pt);
      if(svmObject::Extern != whichPoint) {
        // inserire tips
        t->fillTips(tips, size);
        if(old != t) {
          old = t;
          changed = true;
          }
        anchor = whichPoint;
        break;
        }
      } while(sbp.setNext());
    }
  return anchor;
}
//-----------------------------------------------------------
static bool isSizing(svmObject::typeOfAnchor anchor)
{
  switch(anchor) {
    case svmObject::Extern:
    case svmObject::Inside:
      return false;
    }
  return true;
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmBase::beginXor(const POINT& pt)
{
  svmObject::typeOfAnchor anchor = svmObject::Extern;
  if(!List.setFirst())
    return anchor;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return anchor;
  svmObject* t = 0;
  do  {
    t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      anchor = t->beginDrag(pt);
      break;
      }
    } while(sbp.setNext());
  while(sbp.setNext()) {
    t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected())
      t->beginDrag(pt);
    }
  return anchor;
}
//-----------------------------------------------------------
void svmBase::beginXorSizeByKey()
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected())
      t->beginDragSizeByKey();
    } while(sbp.setNext());
}
//-----------------------------------------------------------
void svmBase::Redo()
{
  svmObjHistory* th = RCAST_HISTORY(RedoHistory.Rem());
  if(!th)
    return;
  svmObjHistory* h = 0;
  switch(th->getAction()) {
    case svmObjHistory::Change:
    case svmObjHistory::Move:
    case svmObjHistory::Size:
    case svmObjHistory::Group:
      if(!List.setFirst())   // non può verificarsi
        break;
      while(th) {
        svmBaseObject *t = th->getObj();

        svmObjHistory* tredo = t->makeHistoryObject(th->getAction());
        svmObject* obj = DCAST_OBJ(t);
        if(obj) {
          if(svmObjHistory::Group != th->getAction()) {
            delete tredo->getClone();
            tredo->setClone(obj->makeClone());
            }
          }
        if(!h)
          History.Push(tredo);
        else
          h->setNext(tredo);
        h = tredo;

        t->restoreByHistoryObject(*th);
        svmObjHistory* hl = th;
        th = th->getNext();
        if(svmObjHistory::Group == hl->getAction())
          hl->setClone(0);
        delete hl;
        }
      invalidate();
      break;

    case svmObjHistory::Order:
      while(th) {
        if(!List.setFirst())
          break;
        svmBaseObject *old = th->getObj();
        do {
          svmObject *t = RCAST_OBJ(List.getCurr());
          if(old == t) {
            svmObject *prev = 0;
            if(List.setPrev()) {
              prev = RCAST_OBJ(List.getCurr());
              List.setNext();
              }
            svmObjHistory* tredo = t->makeHistoryObject(svmObjHistory::Order, prev);
            if(!h)
              History.Push(tredo);
            else
              h->setNext(tredo);
            h = tredo;

            List.Rem();
            List.setFirst();
            svmBaseObject *tPrev = th->getPrev();
            if(!tPrev)
              List.insert(t, true);
            else {
              do {
                svmObject *t2 = RCAST_OBJ(List.getCurr());
                if(tPrev == t2) {
                  List.insert(t, false);
                  break;
                  }
                } while(List.setNext());
              }
            break;
            }
          } while(List.setNext());
        svmObjHistory* tth = th;
        th = th->getNext();
        delete tth;
        }
      invalidate();
      break;

    case svmObjHistory::Add:
      if(!List.setFirst())
        break;
      while(th) {
        svmBaseObject *to = th->getObj();
        svmBaseObject *tPrev = th->getPrev();
        List.setFirst();
        do {
          svmObject *t = RCAST_OBJ(List.getCurr());
          if(th->getObj() == t) {

            svmObjHistory* tredo = t->makeHistoryObject(svmObjHistory::Rem, RCAST_OBJ(tPrev));
            if(!h)
              History.Push(tredo);
            else
              h->setNext(tredo);
            h = tredo;

            List.Rem();
            break;
            }
          } while(List.setNext());
        svmObjHistory* hl = th;
        th = th->getNext();
        delete hl;
        }
      invalidate();
      break;

    case svmObjHistory::Rem:
      while(th) {
        svmBaseObject *to = th->getObj();
        svmBaseObject *tPrev = th->getPrev();

        svmObjHistory* tredo = to->makeHistoryObject(svmObjHistory::Add, RCAST_OBJ(tPrev));
        if(!h)
          History.Push(tredo);
        else
          h->setNext(tredo);
        h = tredo;
        // se la lista è vuota
        if(!List.setFirst())
          List.Push(to);

        // se era il primo della lista
        else if(!tPrev)
          List.insert(to, true);

        else {
          do {
            svmObject *t = RCAST_OBJ(List.getCurr());
            if(tPrev == t) {
              List.insert(to, false);
              break;
              }
            } while(List.setNext());
          }
        svmObjHistory* hl = th;
        th = th->getNext();
        delete hl;
        }
      invalidate();
      break;
    }
}
//-----------------------------------------------------------
void svmBase::Undo()
{
  svmObjHistory* th = RCAST_HISTORY(History.Rem());
  if(!th)
    return;
  svmObjHistory* h = 0;
  switch(th->getAction()) {
    case svmObjHistory::Change:
    case svmObjHistory::Move:
    case svmObjHistory::Size:
    case svmObjHistory::Group:
      if(!List.setFirst())   // non può verificarsi
        break;
      while(th) {
        svmBaseObject *t = th->getObj();

        svmObjHistory* tredo = t->makeHistoryObject(th->getAction());
        svmObject* obj = DCAST_OBJ(t);
        if(obj) {
          if(svmObjHistory::Group != th->getAction()) {
            delete tredo->getClone();
            tredo->setClone(obj->makeClone());
            }
          }
        if(!h)
          RedoHistory.Push(tredo);
        else
          h->setNext(tredo);
        h = tredo;

        t->restoreByHistoryObject(*th);
        svmObjHistory* hl = th;
        th = th->getNext();
        if(svmObjHistory::Group == hl->getAction())
          hl->setClone(0);
        delete hl;
        }
      invalidate();
      break;

    case svmObjHistory::Order:
      while(th) {
        if(!List.setFirst())
          break;
        svmBaseObject *old = th->getObj();
        do {
          svmObject *t = RCAST_OBJ(List.getCurr());
          if(old == t) {
            svmObject *prev = 0;
            if(List.setPrev()) {
              prev = RCAST_OBJ(List.getCurr());
              List.setNext();
              }
            svmObjHistory* tredo = t->makeHistoryObject(svmObjHistory::Order, prev);
            if(!h)
              RedoHistory.Push(tredo);
            else
              h->setNext(tredo);
            h = tredo;

            List.Rem();
            List.setFirst();
            svmBaseObject *tPrev = th->getPrev();
            if(!tPrev)
              List.insert(t, true);
            else {
              do {
                svmObject *t2 = RCAST_OBJ(List.getCurr());
                if(tPrev == t2) {
                  List.insert(t, false);
                  break;
                  }
                } while(List.setNext());
              }
            break;
            }
          } while(List.setNext());
        svmObjHistory* tth = th;
        th = th->getNext();
        delete tth;
        }
      invalidate();
      break;

    case svmObjHistory::Add:
      if(!List.setFirst())
        break;
      while(th) {
        svmBaseObject *to = th->getObj();
        svmBaseObject *tPrev = th->getPrev();
        List.setFirst();
        do {
          svmObject *t = RCAST_OBJ(List.getCurr());
          if(th->getObj() == t) {
            svmObjHistory* tredo = t->makeHistoryObject(svmObjHistory::Rem, RCAST_OBJ(tPrev));
            if(!h)
              RedoHistory.Push(tredo);
            else
              h->setNext(tredo);
            h = tredo;
            List.Rem();
            break;
            }
          } while(List.setNext());
        svmObjHistory* hl = th;
        th = th->getNext();
        delete hl;
        }
      invalidate();
      break;

    case svmObjHistory::Rem:
      while(th) {
        svmBaseObject *to = th->getObj();
        svmBaseObject *tPrev = th->getPrev();

        svmObjHistory* tredo = to->makeHistoryObject(svmObjHistory::Add, RCAST_OBJ(tPrev));
        if(!h)
          RedoHistory.Push(tredo);
        else
          h->setNext(tredo);
        h = tredo;

        // se la lista è vuota
        if(!List.setFirst())
          List.Push(to);

        // se era il primo della lista
        else if(!tPrev)
          List.insert(to, true);

        else {
          do {
            svmObject *t = RCAST_OBJ(List.getCurr());
            if(tPrev == t) {
              List.insert(to, false);
              break;
              }
            } while(List.setNext());
          }
        svmObjHistory* hl = th;
        th = th->getNext();
        delete hl;
        }
      invalidate();
      break;
    }
}
//----------------------------------------------------------------------------
#define SND_MSG_REMOTE(wParam, lParam) SendMessageTimeout(hwnd, registeredMsg, (wParam), (LPARAM)lParam, SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &msgResult)
//----------------------------------------------------------------------------
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  DWORD_PTR msgResult;
  DWORD_PTR id = rand();
  if(SND_MSG_REMOTE(MAKEWPARAM(CM_INIT_COMM, 0), id)) {
    if(id == msgResult) {
      HWND* phwnd = reinterpret_cast<HWND*>(lParam);
      *phwnd = hwnd;
      return false;
      }
    }
  return TRUE;
}
//------------------------------------------------------------------------------
bool getListHwnd(HWND* phwnd)
{
  return toBool(EnumWindows(EnumWindowsProc, (LPARAM)phwnd));
}
//-----------------------------------------------------------
void svmBase::manageCheckAddr(bool refresh)
{
  HWND find = 0;
  getListHwnd(&find);
  if(!find)
    return;
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;

  manageInfoAd4 mset;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      t->addInfoAd4(mset);
      }
    } while(sbp.setNext());
  const PVect<infoAd4*>& set = mset.getSet();
  uint nElem = set.getElem();
  if(!nElem)
    return;
  PVect<varsFilter> flt;
  flt.setDim(nElem);
  for(uint i = 0; i < nElem; ++i) {
    flt[i].prph = set[i]->prph;
    flt[i].addr = set[i]->addr;
    flt[i].type = set[i]->type;
    flt[i].nbit = set[i]->nBit;
    flt[i].offs = set[i]->offs;
    }
  PDiagCheckaddr(Owner, find, flt, refresh).modal();
}
//-----------------------------------------------------------
void svmBase::manageAd4()
{
  if(!List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  manageInfoAd4 mset;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      t->addInfoAd4(mset);
      }
    } while(sbp.setNext());
  const PVect<infoAd4*>& set = mset.getSet();
  if(set.getElem())
    PDiagAd4(Owner, set).modal();
}
//-----------------------------------------------------------
void svmBase::hideShowLinked(bool show)
{
  if(!LinkedStat)
    return;
  LinkedStat = show ? 1 : -1;
  freeBkg();
}
//-----------------------------------------------------------
#define DEF_COLOR_BKG GetSysColor(COLOR_BTNFACE)
//-----------------------------------------------------------
void svmBase::paint(HDC hdc, const PRect& rect, bool all)
{
  if(!hBmpWork) {
    makeBkg();

    if(BmpBkg) {
      POINT pt = { 0, 0 };
      BmpBkg->draw(mdcWork, pt);
      }
    else {
      HBRUSH br = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
      PRect r;
      GetClientRect(*Owner, r);
      getDataProject().calcFromScreen(r);
      FillRect(mdcWork, r, br);
      }
    if(ListLinked.setFirst() && LinkedStat > 0) {
      do {
        splitByPriority sbp(ListLinked, allowSelect::asAllObject);
        sbp.setFirst();
        int nElem = sbp.getCurrNElem();
        int curr = 1;
        do  {
          if(curr > nElem) {
            nElem = sbp.getCurrNElem();
            curr = 1;
            }
          svmObject* t = sbp.getCurr();
          t->set_zOrder(curr++);
          } while(sbp.setNext());
        } while(false);
      splitByPriority sbp(ListLinked, allowSelect::asAllObject);
      sbp.setLast();
      do  {
        svmObject* t = sbp.getCurr();
        t->DrawObjectLinked(mdcWork, rect);
        } while(sbp.setPrev());
      }

    if(List.setFirst()) {
      do {
        splitByPriority sbp(List, allowSelect::asAllObject);
        sbp.setFirst();
        int nElem = sbp.getCurrNElem();
        int curr = 1;
        do  {
          if(curr > nElem) {
            nElem = sbp.getCurrNElem();
            curr = 1;
            }
          svmObject* t = sbp.getCurr();
          t->set_zOrder(curr++);
          } while(sbp.setNext());
        } while(false);
      splitByPriority sbp(List, allowSelect::asAllObject);
      sbp.setLast();
      do  {
        svmObject* t = sbp.getCurr();
        t->DrawOnlyObject(mdcWork, rect);
        } while(sbp.setPrev());
      }
    }
  if(all)
    BitBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), mdcWork, rect.left, rect.top, SRCCOPY);
  else {
    allowSelect::allow allowedObj = getAllow().get();
    splitByPriority sbp(List, allowedObj);
    if(sbp.setLast()) {
      do  {
        svmObject* t = sbp.getCurr();
        t->DrawOnlySelect(hdc, rect);
        } while(sbp.setPrev());
      }
    }
}
//-----------------------------------------------------------
bool svmBase::canAlign()
{
  if(!List.setFirst())
    return false;
  bool oneSelected = false;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      if(oneSelected)
        return true;
      oneSelected = true;
      }
    } while(List.setNext());
  return false;
}
//-----------------------------------------------------------
bool svmBase::canCenter()
{
  return canAlign();
}
//-----------------------------------------------------------
void svmBase::centerVert()
{
  if(!List.setFirst())
    return;
  int pt = -1;

  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      PRect r = t->getRect();
      pt = (r.top + r.bottom) / 2;
      break;
      }
    } while(List.setNext());

  svmObjHistory* h = 0;
  RedoHistory.Flush();
  while(List.setNext()) {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      PRect r = t->getRect();
      int pt2 = (r.top + r.bottom) / 2;
      if(pt2 != pt) {
        svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Move);
        if(!h)
          History.Push(th);
        else
          h->setNext(th);
        h = th;
        r.Offset(0, pt - pt2);
        t->setRect(r);
        }
      }
    }
  invalidate();
}
//-----------------------------------------------------------
void svmBase::centerHorz()
{
  if(!List.setFirst())
    return;
  int pt = -1;

  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      PRect r = t->getRect();
      pt = (r.left + r.right) / 2;
      break;
      }
    } while(List.setNext());

  RedoHistory.Flush();
  svmObjHistory* h = 0;
  while(List.setNext()) {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      PRect r = t->getRect();
      int pt2 = (r.left + r.right) / 2;
      if(pt2 != pt) {
        svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Move);
        if(!h)
          History.Push(th);
        else
          h->setNext(th);
        h = th;
        r.Offset(pt - pt2, 0);
        t->setRect(r);
        }
      }
    }
  invalidate();
}
//-----------------------------------------------------------
int svmBase::getAlignVal(align how, svmObject* obj)
{
  PRect rect = obj->getRect();
  int pt;
  switch(how) {
    case aLeft:
      pt = rect.left;
      break;
    case aTop:
      pt = rect.top;
      break;
    case aRight:
      pt = rect.right;
      break;
    case aBottom:
      pt = rect.bottom;
      break;
    }
  return pt;
}
//-----------------------------------------------------------
void svmBase::unionAlign(align how, svmObject* obj, int& pt)
{
  PRect rect = obj->getRect();
  switch(how) {
    case aLeft:
      if(pt > rect.left)
        pt = rect.left;
      break;
    case aTop:
      if(pt > rect.top)
        pt = rect.top;
      break;
    case aRight:
      if(pt < rect.right)
        pt = rect.right;
      break;
    case aBottom:
      if(pt < rect.bottom)
        pt = rect.bottom;
      break;
    }
}
//-----------------------------------------------------------
void svmBase::Align(align how)
{
  if(!List.setFirst())
    return;
  int pt = -1;

  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      if(pt == -1)
        pt = getAlignVal(how, t);
      else
        unionAlign(how, t, pt);
      }
    } while(List.setNext());

  RedoHistory.Flush();
  svmObjHistory* h = 0;
  List.setFirst();
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Move);
      if(!h)
        History.Push(th);
      else
        h->setNext(th);
      h = th;
      setAlign(how, t, pt);
      }
    } while(List.setNext());
  invalidate();

}
//-----------------------------------------------------------
void svmBase::setAlign(align how, svmObject* obj, int pt)
{
  PRect rect = obj->getRect();
  switch(how) {
    case aLeft:
      rect.MoveTo(pt, rect.top);
      break;
    case aTop:
      rect.MoveTo(rect.left, pt);
      break;
    case aRight:
      rect.MoveTo(pt - rect.Width(), rect.top);
      break;
    case aBottom:
      rect.MoveTo(rect.left, pt - rect.Height());
      break;
    }
  obj->setRect(rect);
}
//-----------------------------------------------------------
void addOrdered(PVect<svmObject*>& set, svmObject *obj, svmBase::align how)
{
  int ix = -1;
  int nElem = set.getElem();
  PRect rect = obj->getRect();
  if(svmBase::aLeft == how) {
    for(int i = 0; i < nElem; ++i) {
      PRect r = set[i]->getRect();
      if(rect.left < r.left) {
        ix = i;
        break;
        }
      }
    }
  else {
    for(int i = 0; i < nElem; ++i) {
      PRect r = set[i]->getRect();
      if(rect.top < r.top) {
        ix = i;
        break;
        }
      }
    }
  if(ix < 0)
    ix = nElem;
  set.insert(obj, ix);
}
//-----------------------------------------------------------
double calcDist(PVect<svmObject*>& set, svmBase::align how)
{
  PRect r1 = set[0]->getRect();
  int nElem = set.getElem();
  PRect r2 = set[nElem - 1]->getRect();
  int dist;
  if(svmBase::aLeft == how) {
    dist = r2.left - r1.right;
    for(int i = 1; i < nElem - 1; ++i) {
      PRect r = set[i]->getRect();
      dist -= r.Width();
      }
    }
  else {
    dist = r2.top - r1.bottom;
    for(int i = 1; i < nElem - 1; ++i) {
      PRect r = set[i]->getRect();
      dist -= r.Height();
      }
    }
  return (double)dist / (double)(nElem - 1);
}
//-----------------------------------------------------------
void setAlignSpace(svmBase::align how, svmObject* obj, double dist, int pos, PRect& r)
{
  PRect rect = obj->getRect();
  dist *= pos;
  switch(how) {
    case svmBase::aLeft:
      pos = r.right;
      r.right += rect.Width();
      rect.MoveTo(pos + (int)dist, rect.top);
      break;
    case svmBase::aTop:
      pos = r.bottom;
      r.bottom += rect.Height();
      rect.MoveTo(rect.left, pos + (int)dist);
      break;
    }
  obj->setRect(rect);
}
//-----------------------------------------------------------
void svmBase::AlignSpace(align how)
{
  if(!List.setFirst())
    return;

  PVect<svmObject*> set;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected())
      addOrdered(set, t, how);
    } while(List.setNext());
  double dist = calcDist(set, how);

  int nElem = set.getElem() - 1;
  PRect first = set[0]->getRect();

  RedoHistory.Flush();
  svmObjHistory* h = 0;
  for(int i = 1; i < nElem; ++i) {
    svmObject *t = set[i];
    svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Move);
    if(!h)
      History.Push(th);
    else
      h->setNext(th);
    h = th;
    setAlignSpace(how, t, dist, i, first);
    }
  invalidate();
}
//-----------------------------------------------------------
bool svmBase::canAlignSpace()
{
  if(!List.setFirst())
    return false;
  int selected = 0;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      if(++selected >= 3)
        return true;
      }
    } while(List.setNext());
  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool svmBase::canProperty()
{
  if(!List.getElem())
    return false;
  List.setFirst();
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected())
      return true;
    } while(List.setNext());
  return false;
}
//-----------------------------------------------------------
bool svmBase::setLockObj(bool hide)
{
  if(!List.getElem())
    return false;
  List.setFirst();
  bool canLock = false;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      canLock = true;
      if(hide)
        t->setVisible(false);
      else
        t->setLock(true);
      }
    else {
      if(hide)
        t->setVisible(true);
      else
        t->setLock(false);
      }
    } while(List.setNext());
  return canLock;
}
//-----------------------------------------------------------
bool svmBase::canOrder()
{
  if(List.getElem() < 2)
    return false;
  List.setFirst();
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected())
      return true;
    } while(List.setNext());
  return false;
}
//-----------------------------------------------------------
bool svmBase::canReInside()
{
  if(!List.setFirst())
    return false;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(t->isOutScreen())
      return true;
    } while(List.setNext());
  return false;
}
//-----------------------------------------------------------
bool svmBase::canGroup()
{
  if(!List.setFirst())
    return false;
  groupObject* grp = 0;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      grp = groupObject::getHighGroup(t);
      break;
      }
    }  while(List.setNext());

  while(List.setNext()) {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      groupObject* grp2 = groupObject::getHighGroup(t);
      if(grp != grp2)
        return true;
      if(!grp && !grp2)
        return true;
      }
    };
  return false;
}
//-----------------------------------------------------------
bool svmBase::canUngroup()
{
  if(!List.setFirst())
    return false;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      if(t->getGroup())
        return true;
      }
    } while(List.setNext());
  return false;
}
//-----------------------------------------------------------
extern bool isObjText4Choose(svmObject* t);
extern bool isObjText4Edit(svmObject* t);

extern bool isObjEdit(svmObject* t);
extern bool isObjChoose(svmObject* t);

extern svmObject* allocObjTextByEdit(svmObject* ed, int id);
extern svmObject* allocObjEdByText(svmObject* text, int id);
extern svmObject* allocObjTextByChoose(svmObject* choose, int id);
extern svmObject* allocObjChooseByText(svmObject* choose, int id);
//-----------------------------------------------------------
#define DYNA_TOGGLE_BY_TXT_ED(o) isObjText4Edit(o)
#define DYNA_TOGGLE_BY_TXT_CHOOSE(o) isObjText4Choose(o)
#define DYNA_TOGGLE_BY_ED(o) isObjEdit(o)
#define DYNA_TOGGLE_BY_CHOOSE(o) isObjChoose(o)
//-----------------------------------------------------------
bool svmBase::canToggleEdTxt()
{
  if(!List.setFirst())
    return false;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      if(DYNA_TOGGLE_BY_TXT_ED(t) || DYNA_TOGGLE_BY_ED(t))
        return true;
      }
    } while(List.setNext());
  return false;
}
//-----------------------------------------------------------
bool svmBase::canToggleChooseTxt()
{
  if(!List.setFirst())
    return false;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      if(DYNA_TOGGLE_BY_TXT_CHOOSE(t) || DYNA_TOGGLE_BY_CHOOSE(t))
        return true;
      }
    } while(List.setNext());
  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
typedef bool (*fz_isObjGen)(svmObject* t);
typedef svmObject* (*fz_allocObjGen)(svmObject* ed, int id);
//-----------------------------------------------------------
void svmBase::toggleObjGen(fz_isObjGen fz_one, fz_allocObjGen fz_allocOne, fz_isObjGen fz_two, fz_allocObjGen fz_allocTwo)
{
  if(!List.setFirst())
    return;
/* Non è semplice gestire i gruppi, allora si eliminano completamente
   quelli presenti negli oggetti selezionati.
   Una soluzione potrebbe essere quella di scambiare i nuovi oggetti con i vecchi
   all'interno dei gruppi, poi però va gestita anche la history.
   Ma forse per l'uso del toggle non è necessario e non ne vale la pena ...
*/
  actionUngroup(true);
  svmObjHistory* h = 0;
  svmMainClient* mc = getMainClient(Owner);
  PVect<svmObject*> toAdd;
  List.setFirst();
  svmObject* prev = 0;
  svmObject* tNew = 0;
  RedoHistory.Flush();
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    tNew = 0;
    prev = 0;
    if(svmObject::yesSel == t->isSelected()) {
      if(fz_one(t))
        tNew = fz_allocOne(t, mc->getIncrementGlobalId());
      else if(fz_two(t))
        tNew = fz_allocTwo(t, mc->getIncrementGlobalId());
      if(tNew) {
        toAdd[toAdd.getElem()] = tNew;

        if(List.setPrev()) {
          prev = RCAST_OBJ(List.getCurr());
          List.setNext();
          }
        svmObject *obj = RCAST_OBJ(List.Rem());
        svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Rem, prev);
        if(!h)
          History.Push(th);
        else
          h->setNext(th);
        h = th;
        }
      }
  } while(prev || !tNew ? List.setNext() : List.setFirst());
  h = 0;
  uint nElem = toAdd.getElem();
  for(uint i = 0; i < nElem; ++i) {
    svmObject *t = toAdd[i];
    if(List.Push(t)) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Add, 0);
      if(!h)
        History.Push(th);
      else
        h->setNext(th);
      h = th;
      t->standBySel(true);
      }
    else
      delete t;
    }
  invalidate();
}
//-----------------------------------------------------------
void svmBase::toggleEdTxt()
{
  toggleObjGen(isObjText4Edit, allocObjEdByText, isObjEdit, allocObjTextByEdit);
}
//-----------------------------------------------------------
void svmBase::toggleChooseTxt()
{
  toggleObjGen(isObjText4Choose, allocObjChooseByText, isObjChoose, allocObjTextByChoose);
}
//-----------------------------------------------------------
void svmBase::actionGroup()
{
  if(!List.setFirst())
    return;

  PVect<svmObject*> set;
  PVect<groupObject*> group;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      groupObject* grp = groupObject::getHighGroup(t);
      if(!grp)
        set[set.getElem()] = t;
      else {
        uint nElem = group.getElem();
        uint i = 0;
        for(; i < nElem; ++i) {
          if(grp == group[i])
            break;
          }
        if(i >= nElem)
          group[nElem] = grp;
        }
      }
    } while(List.setNext());
  uint nElemGrp = group.getElem();
  uint nElemObj = set.getElem();
  if(!(nElemGrp + nElemObj))
    return;
  RedoHistory.Flush();
  svmObjHistory* h = 0;
  for(uint i = 0; i < nElemObj; ++i) {
    svmObject *t = set[i];
    svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Group);
    if(!h)
      History.Push(th);
    else
      h->setNext(th);
    h = th;
    }
  for(uint i = 0; i < nElemGrp; ++i) {
    groupObject *t = group[i];
    svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Group);
    if(!h)
      History.Push(th);
    else
      h->setNext(th);
    h = th;
    }
  manageGroupObject& mgo = getManageGroupObject();
  groupObject* hG = mgo.createGroup();
  for(uint i = 0; i < nElemObj; ++i)
    mgo.addObject(*hG, set[i]);
  for(uint i = 0; i < nElemGrp; ++i)
    mgo.addObject(*hG, group[i]);

}
//-----------------------------------------------------------
void svmBase::actionUngroup(bool all)
{
  if(!List.setFirst())
    return;

  PVect<groupObject*> group;
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      groupObject* grp = groupObject::getHighGroup(t);
      if(grp) {
        uint nElem = group.getElem();
        uint i = 0;
        for(; i < nElem; ++i) {
          if(grp == group[i])
            break;
          }
        if(i >= nElem)
          group[nElem] = grp;
        }
      }
    } while(List.setNext());

  uint nElemGrp = group.getElem();
  for(uint i = 0; i < nElemGrp; ++i) {
    if(all)
      group[i]->explodeAll(History);
    else
      group[i]->explodeHigh(History);
    }
}
//-----------------------------------------------------------
static void getMaxRect(PRect& r, svmBaseObject* obj)
{
  svmObject* p = dynamic_cast<svmObject*>(obj);
  if(p) {
    if(!r.Width() && !r.Height())
      r = p->getRect();
    else
      r |= p->getRect();
    }
  else {
    groupObject* grp = dynamic_cast<groupObject*>(obj);
    if(grp) {
      uint nElem = grp->getElem();
      for(uint i = 0; i < nElem; ++i)
        getMaxRect(r, grp->getObj(i));
      }
    }
}
//-----------------------------------------------------------
static void setGroupMirror(const PRect& r, svmBaseObject* obj, SIZE sz, uint choose, svmObject* t)
{
  svmObject* p = dynamic_cast<svmObject*>(obj);
  if(p) {
    if(p != t) {
      PRect r2 = t->getRect();
      int offsx = emtHorz & choose ? r2.left - r.left : 0;
      int offsy = emtVert & choose ? r2.top - r.top : 0;
      r2 = p->getRect();
      r2.Offset(offsx, offsy);
      p->setRect(r2);
      }
    }
  else {
    groupObject* grp = dynamic_cast<groupObject*>(obj);
    if(grp) {
      uint nElem = grp->getElem();
      for(uint i = 0; i < nElem; ++i)
        setGroupMirror(r, grp->getObj(i), sz, choose, t);
      }
    }
}
//-----------------------------------------------------------
#define EQU_BIT(flag, test) ((flag) == (test))
//-----------------------------------------------------------
static svmObject* getLeftTopObj(const PRect& r, svmBaseObject* obj, SIZE sz, uint choose)
{
  svmObject* p = dynamic_cast<svmObject*>(obj);
  if(p) {
    PRect r2 = p->getRect();
    if(r2.left == r.left && r2.top == r.top) {
      sz.cx -= r.Width() - r2.Width();
      sz.cy -= r.Height() - r2.Height();
      p->setMirror(choose, sz);
      return p;
      }
    else if(EQU_BIT(emtHorz, choose) && r2.left == r.left) {
      sz.cx -= r.Width() - r2.Width();
      p->setMirror(choose, sz);
      return p;
      }
    else if(EQU_BIT(emtVert, choose) && r2.top == r.top) {
      sz.cy -= r.Height() - r2.Height();
      p->setMirror(choose, sz);
      return p;
      }
    }
  else {
    groupObject* grp = dynamic_cast<groupObject*>(obj);
    if(grp) {
      uint nElem = grp->getElem();
      for(uint i = 0; i < nElem; ++i) {
        p = getLeftTopObj(r, grp->getObj(i), sz, choose);
        if(p)
          return p;
        }
      }
    }
  return 0;
}
//-----------------------------------------------------------
static bool alreadyMirror(svmObject* t, PVect<groupObject*>& groups, SIZE sz, uint choose)
{
  groupObject* grp = groupObject::getHighGroup(t);
  if(!grp)
    return false;
  uint nElem = groups.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(grp == groups[i])
      return true;
  PRect r;
  getMaxRect(r, grp);
  t = getLeftTopObj(r, grp, sz, choose);
  if(t)
    setGroupMirror(r, grp, sz, choose, t);
  groups[nElem] = grp;
  return true;
}
//-----------------------------------------------------------
void svmBase::setActionMirror(bool all, uint choose)
{
  if(!all && !List.setFirst())
    return;
  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;
  PRect r;
  GetClientRect(*Owner, r);
  getDataProject().calcFromScreen(r);
  SIZE sz = { r.Width(), r.Height() };

  svmObjHistory* h = 0;
  svmObjHistory::typeOfAction result = svmObjHistory::Move;
  RedoHistory.Flush();
  PVect<groupObject*> groups;
  // occorre un doppio giro per gestire l'history prima che vengano spostati gli oggetti
  do  {
    svmObject* t = sbp.getCurr();
    if(all || svmObject::yesSel == t->isSelected()) {
      svmObjHistory* th = t->makeHistoryObject(result, 0, true);
      if(!h)
        History.Push(th);
      else
        h->setNext(th);
      h = th;
      }
    } while(sbp.setNext());
  sbp.setFirst();
  do  {
    svmObject* t = sbp.getCurr();
    if(all || svmObject::yesSel == t->isSelected()) {
      if(!alreadyMirror(t, groups, sz, choose))
        t->setMirror(choose, sz);
      }
    } while(sbp.setNext());
  invalidate();
}
//-----------------------------------------------------------
void svmBase::ReInside()
{
  if(!List.setFirst())
    return;
  svmObjHistory* h = 0;
  RedoHistory.Flush();
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(t->isOutScreen()) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Move);
      if(!h)
        History.Push(th);
      else
        h->setNext(th);
      h = th;
      t->re_InsideRect();
      }
    } while(List.setNext());
  invalidate();
}
//-----------------------------------------------------------
void svmBase::enableWichOrder(bool buff[])
{
  for(int i = 0; i < oMax; ++i)
    buff[i] = false;

  if(!List.setFirst())
    return;
  bool onlyFirst = false;
  svmObject *t = RCAST_OBJ(List.getCurr());
  if(svmObject::yesSel == t->isSelected())
    onlyFirst = true;
  bool onlyLast = false;
  int nElem = 1;
  int selected = onlyFirst;
  while(List.setNext()) {
    t = RCAST_OBJ(List.getCurr());
    ++nElem;
    if(svmObject::yesSel == t->isSelected()) {
      onlyFirst = false;
      ++selected;
      }
    }
  // se l'ultimo era selezionato
  if(svmObject::yesSel == t->isSelected()) {
    if(nElem > 1 && 1 == selected)
      onlyLast = true;
    }
  if(!onlyFirst) {
    buff[oTop] = true;
    buff[oPrevious] = true;
    }
  if(!onlyLast) {
    buff[oBottom] = true;
    buff[oNext] = true;
    }
}
//-----------------------------------------------------------
void svmBase::orderByFirst(order how)
{
  if(!List.setFirst())
    return;
  RedoHistory.Flush();
  svmObjHistory* h = 0;
  do {
    svmObject* t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      svmObject* prev = 0;
      if(List.setPrev()) {
        prev = RCAST_OBJ(List.getCurr());
        List.setNext();
        }
      if(!prev)
        continue;

      P_List_Gen::pList p = List.getCurrL();
      p = List.getPrev(p);
      while(p) {
        svmObject* tPrev = RCAST_OBJ(p->L);
        if(t->getIdType() == tPrev->getIdType())
          break;
        p = List.getPrev(p);
        }
      // è il primo del suo tipo
      if(!p)
        continue;

      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Order, prev);
      if(!h)
        History.Push(th);
      else
        h->setNext(th);
      h = th;
      List.Rem();
      if(oTop == how) {
        List.setFirst();
        List.insert(t, true);
        do {
          t = RCAST_OBJ(List.getCurr());
          if(t == prev)
            break;
          } while(List.setNext());
        }
      else {
        List.insert(t, true, p);
        List.setNext();
        }
      }
    } while(List.setNext());
}
//-----------------------------------------------------------
static void pushHReverse(svmObjHistory* th, PListHistory& History)
{
  svmObjHistory* next = th->getNext();
  if(next)
    pushHReverse(next, History);
  else
    History.Push(th);
  if(next)
    next->setNext(th);
}
//-----------------------------------------------------------
void svmBase::orderByLast(order how)
{
  if(!List.setLast())
    return;
  svmObjHistory* h = 0;
  svmObjHistory* firstH = 0;
  RedoHistory.Flush();
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      svmObject * prev = 0;
      if(List.setPrev()) {
        prev = RCAST_OBJ(List.getCurr());
        List.setNext();
        }

      P_List_Gen::pList p = List.getCurrL();
      p = List.getNext(p);
      while(p) {
        svmObject* tNext = RCAST_OBJ(p->L);
        if(t->getIdType() == tNext->getIdType())
          break;
        p = List.getNext(p);
        }
      // è l'ultimo del suo tipo
      if(!p)
        continue;

      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Order, prev);
      if(!h)
        firstH = th;
      else
        h->setNext(th);
      h = th;
      List.Rem();
      if(oBottom == how) {
        List.setLast();
        List.insert(t, false);
        do {
          t = RCAST_OBJ(List.getCurr());
          if(t == prev)
            break;
          } while(List.setPrev());
        }
      else {
        List.insert(t, false, p);
        List.setPrev();
        }
      }
    } while(List.setPrev());
  pushHReverse(firstH, History);
  firstH->setNext(0);
}
//-----------------------------------------------------------
void svmBase::setProperty(bool editSingle)
{
  if(!List.setFirst())
    return;

  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);
  if(!sbp.setFirst())
    return;

  int selected = 0;
  do  {
    svmObject* t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected())
      ++selected;
    } while(sbp.setNext());

  DWORD bits = 0;
  Property* prop = 0;
  LPTSTR txt = 0;
  svmObjHistory* h = 0;
  svmObjHistory::typeOfAction result = svmObjHistory::None;

  svmObject* t;
  RedoHistory.Flush();
  // se si parte dal primo della lista è l'ultimo in visualizzazione
  sbp.setLast();
  do  {
    t = sbp.getCurr();
    if(svmObject::yesSel == t->isSelected()) {
      if(selected > 1 && !editSingle) {
        prop = new Property;
        *prop = *t->getProp();
        prop->type1 = selected;
        result = commonProperty(prop, bits, txt);
        break;
        }
      else {
        svmObjHistory::typeOfAction result = t->dialogProperty(false);
        if(svmObjHistory::None != result) {
          svmObjHistory* th = t->makeHistoryObject(result, 0, true);
          if(!h)
            History.Push(th);
          else
            h->setNext(th);
          h = th;
          }
        }
      }
    } while(sbp.setPrev());

  if(prop) {
    if(bits) {
      List.setLast();
      do  {
        svmObject* t = RCAST_OBJ(List.getCurr());
        if(svmObject::yesSel == t->isSelected()) {
          svmObjHistory* th = t->makeHistoryObject(result);
          if(!h)
            History.Push(th);
          else
            h->setNext(th);
          h = th;
          t->setCommonProperty(prop, bits, txt);
          }
        // a causa del controllo se un font è usato o meno nel dialogo della scelta font
        // la posizione corrente della lista viene modificata, allora occorre verificare
        // qui se la posizione corrente corrisponde al puntatore ricavato precedentemente
        // e ripristinare la situazione in caso contrario
        if(t != DCAST_OBJ(List.getCurr())) {
          List.setLast();
          while(t != DCAST_OBJ(List.getCurr()))
            List.setPrev();
          }
        } while(List.setPrev());
      }
    delete prop;
    }
  delete []txt;
  invalidate();
}
//-----------------------------------------------------------
#define BITS_RECT ((1 << CHANGED_X) | (1 << CHANGED_Y) | \
                   (1 << CHANGED_W) | (1 << CHANGED_H) | \
                   (1 << CHANGED_DX) | (1 << CHANGED_DY))
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmBase::commonProperty(Property* prop, DWORD& bits, LPTSTR& simpleText)
{
  if(IDOK == svmCommonDialog(bits, prop, simpleText, Owner).modal()) {
    if(bits & ~BITS_RECT)
      return svmObjHistory::Change;
    return svmObjHistory::Size;
    }
  return svmObjHistory::None;
}
//-----------------------------------------------------------
void svmBase::setOffsetLinked(const POINT& pt)
{
  if(ListLinked.setFirst()) {
    Property prop;
    DWORD bits = 0;
    if(pt.x)
      bits |= 1 << CHANGED_DX;
    if(pt.y)
      bits |= 1 << CHANGED_DY;
    prop.type1 = pt.x;
    prop.type2 = pt.y;
    LPTSTR dummy = 0;
    do  {
      svmObject* t = RCAST_OBJ(ListLinked.getCurr());
      t->setCommonProperty(&prop, bits, dummy);
      } while(ListLinked.setNext());
    }
}
//-----------------------------------------------------------
void svmBase::setOffset(const POINT& pt)
{
  if(List.setFirst()) {
    Property prop;
    DWORD bits = 0;
    if(pt.x)
      bits |= 1 << CHANGED_DX;
    if(pt.y)
      bits |= 1 << CHANGED_DY;
    prop.type1 = pt.x;
    prop.type2 = pt.y;
    LPTSTR dummy = 0;
    do  {
      svmObject* t = RCAST_OBJ(List.getCurr());
      t->setCommonProperty(&prop, bits, dummy);
      } while(List.setNext());
    }
}
//-----------------------------------------------------------
void svmBase::Order(order how)
{
  if(oTop == how || oPrevious == how)
  orderByFirst(how);
  else
    orderByLast(how);
  invalidate();
}
//-----------------------------------------------------------
bool svmBase::saveNumObjects(P_File& pf, int type, int num)
{
  switch(type) {
    case oBUTTON:
      type = ID_VAR_BTN;
      break;
    case oEDIT:
      type = ID_VAR_EDI;
      break;
    case oPANEL:
      type = ID_SIMPLE_PANEL;
      break;
    case oTEXT:
      type = ID_VAR_TXT;
      break;
    case oBARGRAPH:
      type = ID_BARGRAPH;
      break;
    case oLED:
      type = ID_VAR_LED;
      break;
    case oLISTBOX:
      type = ID_VAR_LBOX;
      break;
    case oDIAM:
      type = ID_VAR_DIAM;
      break;
    case oCHOOSE:
      type = ID_VAR_CHOOSE;
      break;
    case oCURVE:
      type = ID_VAR_CURVE;
      break;
    case oSIMPLE_TEXT:
      type = ID_SIMPLE_TXT;
      break;
    case oBITMAP:
      type = ID_VAR_BMP;
      break;
    case oSLIDER:
      type = ID_VAR_SLIDER;
      break;

    case oCAM:
      type = ID_VAR_CAM;
      break;
    case oXMETER:
      type = ID_VAR_XMETER;
      break;
    case oXSCOPE:
      type = ID_VAR_SCOPE;
      break;
    case oLB_ALARM:
      type = ID_VAR_ALARM;
      break;
    case oTREND:
      type = ID_VAR_TREND;
      break;
    case oSPIN:
      type = ID_VAR_SPIN;
      break;
    case oTBL_INFO:
      type = ID_VAR_TABLE_INFO;
      break;
    case oPLOT_XY:
      type = ID_VAR_PLOT_XY;
      break;

    case oBKG_BITMAP:
      type = ID_BMP;
      break;
    default:
      return false;
    }
  TCHAR buff[100];
  wsprintf(buff, _T("%d,%d\r\n"), type, num);
  writeStringChkUnicode(pf, buff);
  return true;
}
//-----------------------------------------------------------
bool svmBase::savePage(P_File& pfCript, P_File& pfClear, int nBtn)
{
  splitByPriority sbp(List, allowSelect::asAllObject);
  if(!sbp.setLast()) {
    saveNumObjects(pfCript, oBKG_BITMAP, 1);
    return false;
    }
  getFontObj().saveFont(pfCript);

  svmObjCount& objCount = getObjCount();
  objCount.reset();
  // il primo è riservato per quello (probabilmente) inserito nella
  // proprietà di pagina
  objCount.getIdCount(oBKG_BITMAP);
  if(nBtn > 0)
    objCount.getIdCount(oBUTTON, nBtn);

  svmObject* t = sbp.getCurr();
  int type = t->getIdType();
  int num;
  bool hasEdit = oEDIT == (objType)type;
  do  {
    t = sbp.getCurr();
    num = objCount.getIdCount((objType)type);
    if(t->getIdType() != type) {
      saveNumObjects(pfCript, type, num);
      type = t->getIdType();
      num = objCount.getIdCount((objType)type);
      hasEdit |= oEDIT == (objType)type;
      }
    t->save(pfCript, pfClear, num);
    } while(sbp.setPrev());

  num = objCount.getIdCount((objType)type);
  saveNumObjects(pfCript, type, num);

  num = objCount.getIdCount(oBKG_BITMAP, 0);
  // non ci sono stati altri bmp di sfondo
  if(1 == num)
    saveNumObjects(pfCript, oBKG_BITMAP, num);

  if(nBtn > 0 && nBtn == objCount.getIdCount(oBUTTON, 0))
    saveNumObjects(pfCript, oBUTTON, nBtn);

  if(!hasEdit) {
    num = objCount.getIdCount(oEDIT, 0);
    if(num)
      saveNumObjects(pfCript, oEDIT, num);
    }
  manageGroupObject& groupMan = getManageGroupObject();
  groupMan.save(pfCript);
  return true;
}
//-----------------------------------------------------------
void svmBase::setActionToBkg(bool set)
{
  if(List.setFirst()) {
    bool found = false;
    do {
      svmObject* t = (svmObject*)List.getCurr();
      if(svmObject::yesSel == t->isSelected()) {
        t->setSelectSimple(false);
        found = true;
        }
      } while(List.setNext());

    if(found)
      invalidate();
    }
}
//-----------------------------------------------------------
bool svmBase::setActionLock(bool set, bool toBkg)
{
  bool found = false;
  splitByPriority sbp(List, toBkg ? allowSelect::asOnlyBkg : allowSelect::asOtherThenBkg);

  if(sbp.setFirst()) {
    if(set) {
      do {
        svmObject* t = sbp.getCurr();
        if(svmObject::yesSel == t->isSelected()) {
          t->setLock(true);
          t->setSelectSimple(false);
          found = true;
          }
        } while(sbp.setNext());
      }
    else {
      do {
        svmObject* t = sbp.getCurr();
        if(t->isLocked() && t->isVisible()) {
          t->setLock(false);
          t->setSelectSimple(true);
          found = true;
          }
        } while(sbp.setNext());
      }
    if(found)
      invalidate();
    }
  return found;
}
//-----------------------------------------------------------
bool svmBase::setActionHide(bool set, bool toBkg)
{
  bool found = false;
  splitByPriority sbp(List, toBkg ? allowSelect::asOnlyBkg : allowSelect::asOtherThenBkg);

  if(sbp.setFirst()) {
    if(set) {
      do {
        svmObject* t = sbp.getCurr();
        if(svmObject::yesSel == t->isSelected()) {
          t->setVisible(false);
          t->setSelectSimple(false);
          found = true;
          }
        } while(sbp.setNext());
      }
    else {
      do {
        svmObject* t = sbp.getCurr();
        if(!t->isVisible()) {
          t->setVisible(true);
          if(!t->isLocked())
            t->setSelectSimple(true);
          found = true;
          }
        } while(sbp.setNext());
      }
    if(found)
      invalidate();
    }
  return found;
}
//-----------------------------------------------------------
void svmBase::findBtnStat(bool onBkg)
{
  splitByPriority sbp(List, onBkg ? allowSelect::asOnlyBkg : allowSelect::asOtherThenBkg);

  if(sbp.setFirst()) {
    do {
      svmObject* t = sbp.getCurr();
      if(!t->isVisible()) {
        oldBtnStat |= eBS_Hide;
        if(eBS_LockAndHide == (oldBtnStat & eBS_LockAndHide))
          break;
        }
      if(t->isLocked()) {
        oldBtnStat |= eBS_Lock;
        if(eBS_LockAndHide == (oldBtnStat & eBS_LockAndHide))
          break;
        }
      } while(sbp.setNext());
    }
}
//-----------------------------------------------------------
bool svmBase::setActionOffset()
{
  static infoOffsetReplace ior;
  if(IDOK != PDiagOffset(Owner, ior).modal())
    return false;

  allowSelect::allow allowedObj = getAllow().get();
  splitByPriority sbp(List, allowedObj);

  bool changed = false;
  infoOffsetReplace ior_t = ior;

  RedoHistory.Flush();
  if(sbp.setLast()) {
    svmObjHistory* h = 0;
    do {
      svmObject* t = sbp.getCurr();
      if(svmObject::yesSel == t->isSelected()) {
        svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Change);
        if(!h)
          History.Push(th);
        else
          h->setNext(th);
        h = th;
        t->setOffsetAddr(ior_t);
        changed = true;
        if(isSetBitf(svmObject::eoProgrBitOffs, ior.flag))
          ++ior_t.bitOffs;
        if(isSetBitf(svmObject::eoProgrAddr, ior.flag))
          ++ior_t.offs;
        }
      } while(sbp.setPrev());
    }
  return changed;
}
//-----------------------------------------------------------
bool svmBase::isUsedFontId(uint idfont)
{
  if(List.setFirst()) {
    do {
      svmObject* t = (svmObject*)List.getCurr();
      if(idfont == t->getFontId())
        return true;
      } while(List.setNext());
    }
  return false;
}
//-----------------------------------------------------------
void svmBase::getUsedFontId(PVect<uint>& set)
{
  if(List.setFirst()) {
    uint nElem = set.getElem();
    do {
      svmObject* t = (svmObject*)List.getCurr();
      bool found = false;
      for(uint i = 0; i < nElem; ++i) {
        if(set[i] == t->getFontId()) {
          found = true;
          break;
          }
        }
      if(!found)
        set[nElem++] = t->getFontId();
      } while(List.setNext());
    }
}
//-----------------------------------------------------------
bool svmBase::decreaseFontIfAbove(uint id_font)
{
  bool changed = false;
  if(List.setFirst()) {
    do {
      svmObject* t = (svmObject*)List.getCurr();
      changed |= t->decreaseFontIfAbove(id_font);
      } while(List.setNext());
    }
  return changed;
}
//-----------------------------------------------------------
bool svmBase::resetFontIfAbove(uint id_font)
{
  bool changed = false;
  if(List.setFirst()) {
    do {
      svmObject* t = (svmObject*)List.getCurr();
      changed |= t->resetFontIfAbove(id_font);
      } while(List.setNext());
    }
  return changed;
}
//-----------------------------------------------------------
bool svmBase::isFontAbove(uint id_font)
{
  if(List.setFirst()) {
    do {
      svmObject* t = (svmObject*)List.getCurr();
      if(t->isFontAbove(id_font))
        return true;
      } while(List.setNext());
    }
  return false;
}
//-----------------------------------------------------------
