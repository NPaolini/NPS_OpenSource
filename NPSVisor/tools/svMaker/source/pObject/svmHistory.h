//----------------- svmHistory.h ----------------------------
//-----------------------------------------------------------
#ifndef SVMHISTORY_H_
#define SVMHISTORY_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pRect.h"
#include "p_Util.h"
//-----------------------------------------------------------
class svmBaseObject;
//-----------------------------------------------------------
// memorizza i dati essenziali a mantenere uno storico delle operazioni
class svmObjHistory : public genericSet
{
  public:
    enum typeOfAction {
        None,
        Move = 1,
        Size,
        Add,
        Rem,
        Order,
        Change, // dipendente dal tipo di oggetto
        Group,
        Abort,
        };

    svmObjHistory(svmBaseObject* obj, svmObjHistory::typeOfAction action, svmBaseObject* prev = 0);
    virtual ~svmObjHistory();

    svmObjHistory* getNext() const;
    void setNext(svmObjHistory* next);

    int getId() const;
    typeOfAction getAction() const;
    const PRect& getRect() const;
    void setRect(const PRect& rect);

    svmBaseObject* getObj();
    svmBaseObject* getPrev();

    svmBaseObject* getClone() const;
    void setClone(svmBaseObject* clone);
  protected:

    // se l'azione è di cancellazione, Prev è l'oggetto che precedeva
    // quello eliminato
    svmBaseObject* Prev;
    // si mantiene il puntatore, in caso di cancellazione mantiene i dati
    svmBaseObject* Curr;

    // link per oggetti successivi, se l'azione ha coinvolto più oggetti.
    // Viene impostato dal chiamante
    svmObjHistory* Next;

    // rettangolo per le dimensioni
    PRect Rect;

    // mantiene una copia dell'oggetto, in caso di modifiche non semplici,
    // usato quando l'azione è typeOfAction::Change
    // nel caso l'azione fosse 'Group' mantiene il puntatore all'oggetto gruppo
    svmBaseObject* Cloned;

    int Id;

    typeOfAction Action;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline int svmObjHistory::getId() const { return Id; }
//-----------------------------------------------------------
inline const PRect& svmObjHistory::getRect() const { return Rect; }
//-----------------------------------------------------------
inline void svmObjHistory::setRect(const PRect& rect) { Rect = rect; }
//-----------------------------------------------------------
inline svmObjHistory::typeOfAction svmObjHistory::getAction() const { return Action; }
//-----------------------------------------------------------
inline svmObjHistory* svmObjHistory::getNext() const { return Next; }
//-----------------------------------------------------------
inline void svmObjHistory::setNext(svmObjHistory* next) { Next = next; }
//-----------------------------------------------------------
inline svmBaseObject* svmObjHistory::getObj() { return Curr; }
//-----------------------------------------------------------
inline svmBaseObject* svmObjHistory::getPrev() { return Prev; }
//-----------------------------------------------------------
inline svmBaseObject* svmObjHistory::getClone() const { return Cloned; }
//-----------------------------------------------------------
inline void svmObjHistory::setClone(svmBaseObject* clone) { Cloned = clone; }
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
