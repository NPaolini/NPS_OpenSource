//------------------ svmBase.cpp ----------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//-----------------------------------------------------------
#include "svmBase.h"
#include "svmMainWorkArea.h"
#include "p_file.h"
#include "svmHistory.h"
#include "common.h"
//#include "PDiagOffset.h"
//-----------------------------------------------------------
#define RCAST_OBJ(a) (reinterpret_cast<svmObject*>(a))
#define DCAST_OBJ(a) (dynamic_cast<svmObject*>(a))
//-----------------------------------------------------------
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
  if(baseClass::insert(d, before)) {
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
//-----------------------------------------------------------
svmBase::svmBase(PWin* owner) : Owner(owner), LinkedStat(0), oldBtnStat(0), hBmpWork(0), mdcWork(0)
{
}
//-----------------------------------------------------------
svmBase::~svmBase()
{
  flushSelected4Paste();
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
void svmBase::flushSelected4Paste()
{
  flushPV(selected4Paste);
}
//-----------------------------------------------------------
bool svmBase::canPaste()
{
  dataProject& dp = getDataProject();
  return selected4Paste.getElem() &&
      *dp.getcurrPath() && *dp.gettemplateName() && *dp.getvarFileName();
}
//-----------------------------------------------------------
void svmBase::pasteCopied(HDC hdc)
{
  if(!canPaste())
    return;
  int nElem = selected4Paste.getElem();
  if(!nElem)
    return;

  if(List.setFirst()) {
    do {
      svmObject *t = RCAST_OBJ(List.getCurr());
      t->Unselect(hdc);
      } while(List.setNext());
    }

  svmObjHistory* h = 0;
  for(int i = 0; i < nElem; ++i) {
    svmObject* t = selected4Paste[i]->makeClone();
    t->resolveFont(selected4Paste[i]->getSaved());
    if(List.Push(t)) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Add, 0);
      if(!h)
        History.Push(th);
      else
        h->setNext(th);
      h = th;
      t->standBySel(true);
      childScrollWorkArea* mc = getParentWin<childScrollWorkArea>(Owner);
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
        selected4Paste[c] = p;
        ++c;
        }
      } while(List.setPrev());
    }
}
//-----------------------------------------------------------
void svmBase::deleteSelected()
{
  if(List.setFirst()) {
    bool isFirst;
    svmObjHistory* h = 0;
    do {
      svmObject* t = RCAST_OBJ(List.getCurr());
      if(!t)
        break;
      isFirst = false;
      if(t->isSelected()) {
        svmObject * prev = 0;
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
    svmObjHistory* h = obj->makeHistoryObject(svmObjHistory::Add, 0);
    History.Push(h);
    if(hdc) {
      List.setFirst();
      do {
        svmObject *t = RCAST_OBJ(List.getCurr());
        t->Unselect(hdc);
        } while(List.setNext());
      obj->standBySel(true);
      }
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

  if(!List.setFirst())
    return select;

  int selected = 0;

  // preverifica:
  //  1) se il click avviene all'interno dell'oggetto
  //  2) punto sensibile
  //  3) quanti sono attualmente selezionati
  //  4) il primo oggetto selezionato su cui avviene il click

  svmObject* firstSelected = 0;
//  bool clickedSelected = false;
  svmObject::typeOfAnchor anchor = svmObject::Extern;
  // esegue prima un controllo per verificare quanti sono
  // selezionati e se il punto scelto cade in un punto sensibile
  do {
    svmObject *t = DCAST_OBJ(List.getCurr());
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
//      if(!clickedSelected && svmObject::Extern != anchor)
//        if(svmObject::yesSel == t->isSelected())
//          clickedSelected = true;
      }
    } while(List.setNext());

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

  List.setFirst();
  int result = select;
  bool deselectOther = false;
  bool activeInside = false;
  do {
    svmObject *t = DCAST_OBJ(List.getCurr());
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
    } while(List.setNext());
  return result;

}
//--------------------------------------------------------------
int svmBase::findPointAdd(HDC hdc, const POINT&pt)
{
  if(!List.setFirst())
    return select;

  int result = select;
  do {
    svmObject *t = DCAST_OBJ(List.getCurr());
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
    } while(List.setNext());
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
  bool add = needAddingObject(keyFlag);
  do {
    svmObject *t = DCAST_OBJ(List.getCurr());
    if(!t->isVisible() || t->isLocked())
        continue;
    if(t->intersec(frame))
      t->standBySel(true);
    else if(!add && svmObject::yesSel == t->isSelected())
      t->standBySel(false);
    } while(List.setNext());
}
//-----------------------------------------------------------
void svmBase::showSelected(HDC hdc)
{
  if(!List.setFirst())
    return;
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
    if(svmObject::signalAdd == t->isSelected())
      t->Select(hdc);
    else if(svmObject::signalRemove == t->isSelected())
      t->Unselect(hdc);
    } while(List.setNext());
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
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected())
      t->Drag(hdc, pt);
    } while(List.setNext());
}
//-----------------------------------------------------------
void svmBase::drawXorBlockFinal(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  svmObjHistory* h = 0;
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
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
    } while(List.setNext());

  invalidate();
}
//-----------------------------------------------------------
void svmBase::drawXorObject(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      t->Drag(hdc, pt);
      break;
      }
    } while(List.setNext());

}
//-----------------------------------------------------------
void svmBase::drawXorObjectFinal(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Size);
      if(t->endDrag(hdc, pt))
        History.Push(th);
      else
        delete th;
      break;
      }
    } while(List.setNext());

  invalidate();
}
//-----------------------------------------------------------
void svmBase::drawResizeAllObject(HDC hdc, const POINT& pt)
{
  if(!List.setFirst())
    return;
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      t->DragOnResizeByKey(hdc, pt);
      }
    } while(List.setNext());

}
//-----------------------------------------------------------
void svmBase::drawResizeAllObjectFinal(HDC hdc)
{
  if(!List.setFirst())
    return;

  svmObjHistory* h = 0;
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
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
    } while(List.setNext());


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
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
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
    } while(List.setNext());
  if(svmObject::Extern == anchor) {
    List.setFirst();
    do  {
      svmObject* t = DCAST_OBJ(List.getCurr());
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
      } while(List.setNext());
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
  svmObject* t = 0;
  do  {
    t = DCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      anchor = t->beginDrag(pt);
      break;
      }
    } while(List.setNext());
  while(List.setNext()) {
    t = DCAST_OBJ(List.getCurr());
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
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected())
      t->beginDragSizeByKey();
    } while(List.setNext());
}
//-----------------------------------------------------------
void svmBase::Undo()
{
  svmObjHistory* th = RCAST_HISTORY(History.Rem());
  if(!th)
    return;
  switch(th->getAction()) {
    case svmObjHistory::Change:
    case svmObjHistory::Move:
    case svmObjHistory::Size:
      if(!List.setFirst())   // non può verificarsi
        break;
      while(th) {
        svmObject *t = DCAST_OBJ(th->getObj());
        t->restoreByHistoryObject(*th);
        svmObjHistory* hl = th;
        th = th->getNext();
        delete hl;
        }
      invalidate();
      break;

    case svmObjHistory::Order:
      while(th) {
        if(!List.setFirst())
          break;
        svmObject *old = th->getObj();
        do {
          svmObject *t = RCAST_OBJ(List.getCurr());
          if(old == t) {
            List.Rem();
            List.setFirst();
            svmObject *tPrev = th->getPrev();
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
        th = th->getNext();
        }
      invalidate();
      break;

    case svmObjHistory::Add:
      if(!List.setFirst())
        break;
      while(th) {
        svmObject *to = th->getObj();
        List.setFirst();
        do {
          svmObject *t = RCAST_OBJ(List.getCurr());
          if(th->getObj() == t) {
            delete RCAST_OBJ(List.Rem());
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
        svmObject *to = th->getObj();
        svmObject *tPrev = th->getPrev();

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
//    case svmObjHistory::Change: // da definire
//      break;
    }
}
//-----------------------------------------------------------
#define DEF_COLOR_BKG GetSysColor(COLOR_BTNFACE)
//-----------------------------------------------------------
void svmBase::paint(HDC hdc, const PRect& rect, bool all)
{
#if 1
  if(all) {
    if(ListLinked.setLast() && LinkedStat > 0) {
      do  {
        svmObject* t = RCAST_OBJ(ListLinked.getCurr());
        t->DrawObjectLinked(hdc, rect);
        } while(ListLinked.setPrev());
      }
    if(List.setLast()) {
      do  {
        svmObject* t = DCAST_OBJ(List.getCurr());
        t->DrawOnlyObject(hdc, rect);
        } while(List.setPrev());
      }
    }
  else {
    if(List.setLast()) {
      do  {
        svmObject* t = DCAST_OBJ(List.getCurr());
        t->DrawOnlySelect(hdc, rect);
        } while(List.setPrev());
      }
    }
#else
  if(!hBmpWork) {
    makeBkg();

    HBRUSH br = CreateSolidBrush(DEF_COLOR_BKG);
    PRect r;
    GetClientRect(*Owner, r);
    getDataProject().calcFromScreen(r);
    FillRect(mdcWork, r, br);
    DeleteObject(HGDIOBJ(br));

    if(List.setLast()) {
      do  {
        svmObject* t = DCAST_OBJ(List.getCurr());
        t->DrawOnlyObject(mdcWork, rect);
        } while(List.setPrev());
      }
    }
  if(all)
    BitBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), mdcWork, rect.left, rect.top, SRCCOPY);
  else {
    if(List.setLast()) {
      do  {
        svmObject* t = DCAST_OBJ(List.getCurr());
        t->DrawOnlySelect(hdc, rect);
        } while(List.setPrev());
      }
    }
#endif
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
void svmBase::ReInside()
{
  if(!List.setFirst())
    return;
  svmObjHistory* h = 0;
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

      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Order, prev);
      if(!h)
        History.Push(th);
      else
        h->setNext(th);
      h = th;
      if(oTop == how) {
        List.Rem();
        List.setFirst();
        List.insert(t, true);
        do {
          t = RCAST_OBJ(List.getCurr());
          if(t == prev)
            break;
          } while(List.setNext());
        }
      else {
        List.levelUp();
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
  do {
    svmObject *t = RCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      svmObject * prev = 0;
      if(List.setPrev()) {
        prev = RCAST_OBJ(List.getCurr());
        List.setNext();
        }

      svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Order, prev);
      if(!h)
        firstH = th;
      else
        h->setNext(th);
      h = th;
      if(oBottom == how) {
        List.Rem();
        List.setLast();
        List.insert(t, false);
        do {
          t = RCAST_OBJ(List.getCurr());
          if(t == prev)
            break;
          } while(List.setPrev());
        }
      else {
        List.levelDown();
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
  if(!List.setLast())
    return;

  int selected = 0;
  unionProperty uProp;
  do  {
    svmObject* t = DCAST_OBJ(List.getCurr());
    if(svmObject::yesSel == t->isSelected()) {
      t->initUnion(uProp);
      t->unionBits(uProp.flagBits);
      ++selected;
      }
    } while(List.setPrev());

  svmObjHistory* h = 0;
  svmObjHistory::typeOfAction result = svmObjHistory::None;
  if(selected > 1 && !editSingle) {
    uProp.totObject = selected;
    result = commonProperty(uProp);
    if(uProp.flagBits && svmObjHistory::None != result) {
      List.setLast();
      do  {
        svmObject* t = RCAST_OBJ(List.getCurr());
        if(svmObject::yesSel == t->isSelected()) {
          DWORD capable = 0;
          t->unionBits(capable);
          if(capable & uProp.flagBits) {
            svmObjHistory* th = t->makeHistoryObject(result);
            if(!h)
              History.Push(th);
            else
              h->setNext(th);
            h = th;
            t->setCommonProperty(uProp);
            }
          }
        } while(List.setPrev());
      }
    }
  else {
  // se si parte dal primo della lista è l'ultimo in visualizzazione
    List.setLast();
    do  {
      svmObject* t = DCAST_OBJ(List.getCurr());
      if(svmObject::yesSel == t->isSelected()) {
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
  invalidate();
}
//-----------------------------------------------------------
#define BITS_RECT ((1 << CHANGED_X) | (1 << CHANGED_Y) | \
                   (1 << CHANGED_W) | (1 << CHANGED_H) | \
                   (1 << CHANGED_DX) | (1 << CHANGED_DY))
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmBase::commonProperty(unionProperty& prop)
{
  if(IDOK == svmCommonDialog(prop, Owner).modal()) {
    return svmObjHistory::Change;
    }
  return svmObjHistory::None;
}
//-----------------------------------------------------------
void svmBase::setOffset(const POINT& pt)
{
  /*
  if(List.setFirst()) {
    Property prop;
    DWORD bits = 0;
    if(pt.x)
      bits |= 1 << CHANGED_DX;
    if(pt.y)
      bits |= 1 << CHANGED_DY;
    prop.type1 = pt.x;
    prop.type2 = pt.y;
    do  {
      svmObject* t = RCAST_OBJ(List.getCurr());
      t->setCommonProperty(&prop, bits);
      } while(List.setNext());
    }
    */
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
bool svmBase::save(P_File& pf, LPCTSTR fileVars, uint* ids)
{
  if(List.setLast()) {
    do {
      svmObject* t = DCAST_OBJ(List.getCurr());
      if(!t->save(pf))
        return false;
      } while(List.setPrev());
    }
  TCHAR t[64];
  wsprintf(t, _T("%d,"), ID_FILE_DATA);
  if(!writeStringChkUnicode(pf, t))
    return false;
  do {
    TCHAR path[_MAX_PATH];
    translateFromCRNL(path, fileVars);
    if(!writeStringChkUnicode(pf, path))
      return false;
    } while(false);
  if(!writeStringChkUnicode(pf, _T("\r\n")))
    return false;

  wsprintf(t, _T("%d"), ID_FILE_DATA_VARS);
  if(!writeStringChkUnicode(pf, t))
    return false;
  for(uint i = 0; i < MAX_ID_DATA_VARS; ++i) {
    wsprintf(t, _T(",%d"), ids[i]);
    if(!writeStringChkUnicode(pf, t))
      return false;
    }
  if(!writeStringChkUnicode(pf, _T("\r\n")))
    return false;

  dataProject& dp = getDataProject();
  wsprintf(t, _T("%d,%d,%d\r\n"), ID_FILE_PAGE_DIM, dp.getPageWidth(), -dp.getPageHeight());
  if(!writeStringChkUnicode(pf, t))
    return false;
  if(*dp.getlinkedFileName()) {
    wsprintf(t, _T("%d,"), ID_FILE_LINKED);
    if(!writeStringChkUnicode(pf, t))
      return false;
    if(!writeStringChkUnicode(pf, dp.getlinkedFileName()))
      return false;
    if(!writeStringChkUnicode(pf, _T("\r\n")))
      return false;
    }
  return true;
}
//-----------------------------------------------------------
bool svmBase::setActionLock(bool set)
{
  bool found = false;
  if(List.setFirst()) {
    if(set) {
      do {
        svmObject* t = DCAST_OBJ(List.getCurr());
        if(svmObject::yesSel == t->isSelected()) {
          t->setLock(true);
          t->setSelectSimple(false);
          found = true;
          }
        } while(List.setNext());
      }
    else {
      do {
        svmObject* t = DCAST_OBJ(List.getCurr());
        if(t->isLocked() && t->isVisible()) {
          t->setLock(false);
          t->setSelectSimple(true);
          found = true;
          }
        } while(List.setNext());
      }
    if(found)
      invalidate();
    }
  return found;
}
//-----------------------------------------------------------
bool svmBase::setActionHide(bool set)
{
  bool found = false;
  if(List.setFirst()) {
    if(set) {
      do {
        svmObject* t = DCAST_OBJ(List.getCurr());
        if(svmObject::yesSel == t->isSelected()) {
          t->setVisible(false);
          t->setSelectSimple(false);
          found = true;
          }
        } while(List.setNext());
      }
    else {
      do {
        svmObject* t = DCAST_OBJ(List.getCurr());
        if(!t->isVisible()) {
          t->setVisible(true);
          if(!t->isLocked())
            t->setSelectSimple(true);
          found = true;
          }
        } while(List.setNext());
      }
    if(found)
      invalidate();
    }
  return found;
}
//-----------------------------------------------------------
void svmBase::findBtnStat()
{
  if(List.setFirst()) {
    do {
      svmObject* t = DCAST_OBJ(List.getCurr());
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
      } while(List.setNext());
    }
}
//-----------------------------------------------------------
extern bool getOffsetVar(PWin*par, int& v);
//-----------------------------------------------------------
bool svmBase::setActionOffset()
{
  static int Offset;
  if(!canProperty())
    return false;
  if(!getOffsetVar(Owner, Offset))
    return false;
  bool changed = false;
  if(List.setFirst()) {
    svmObjHistory* h = 0;
    do {
      svmObject* t = DCAST_OBJ(List.getCurr());
      if(svmObject::yesSel == t->isSelected()) {
        if(t->setOffsetVar(Offset)) {
          svmObjHistory* th = t->makeHistoryObject(svmObjHistory::Change);
          if(!h)
            History.Push(th);
          else
            h->setNext(th);
          changed = true;
          }
        }
      } while(List.setNext());
    }
  if(changed)
    invalidate();
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
