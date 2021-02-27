//--------- lnk_body.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "lnk_body.h"
#include "p_util.h"
#include "1.h"
//----------------------------------------------------------------------------
extern P_Body *allocMainMenu(PWin *parent);
extern bool isMainPage(LPCTSTR page);
//----------------------------------------------------------------------------
struct infoBody
{
  LPCTSTR oldPage;
  int id;
  bool readOnly;
  infoBody() : oldPage(0), id(0), readOnly(false) {}
};
//----------------------------------------------------------------------------
#define MAX_ELEM 100
//----------------------------------------------------------------------------
class pStack
{
  public:
    pStack();
    ~pStack();

    bool pop(infoBody& target);
    bool push(const infoBody& source);
    bool existPageName(LPCTSTR newPageDescr);
  private:

    int curr;
    infoBody iB[MAX_ELEM];
};
//----------------------------------------------------------------------------
static pStack bodyStack;
//----------------------------------------------------------------------------
static bool isSamePage( LPCTSTR p1, LPCTSTR p2)
{
  if(!p1 && !p2)
    return true;
  if(!p1 || !p2)
    return false;
  return !_tcsicmp(p1, p2);
}
//----------------------------------------------------------------------------
P_Body *getBody(PWin *parent, LPCTSTR newPageDescr, LPCTSTR currPageDescr, int newId, int currId)
{
  P_Body* currBody = getCurrBody();

  LPCTSTR p = getString(ID_NO_MEM_PATH_PAGE);
  bool useStack = !p || !_ttoi(p);
// IMPORTANTE. Non invertire l'ordine, prima viene verificato se ha un newId e poi
//             se ha una pagina generica.
  // si richiede una pagina specifica
  if(newId) {
    if(newId == currId && isSamePage(newPageDescr, currPageDescr))
      return 0;
    if(useStack) {
      if(newPageDescr && bodyStack.existPageName(newPageDescr))
        return 0;

      infoBody ib;
      // necessita della pagina cui ritornare e/o del vecchio id se
      // la richiesta non arriva da una pagina generica
      if(currPageDescr)
        ib.oldPage = str_newdup(currPageDescr);

      if(IDD_STANDARD != currId)
        ib.id = currId;

      if(currBody)
        ib.readOnly = currBody->isReadOnly();
      if(!bodyStack.push(ib)) {
        delete []ib.oldPage;
        return 0;
        }
      }
    // esegue vecchia routine senza inviare id di ritorno
    P_Body *bd = getBody(newId, parent, 0);
    if(newPageDescr) {
      P_DefBody* pdb = dynamic_cast<P_DefBody*>(bd);
      if(pdb)
        pdb->setPageName(newPageDescr);
      }
    return bd;
    }

  // si richiede una nuova pagina generica
  if(newPageDescr) {
    // evita di richiamare se stessa
    if(currPageDescr && !_tcsicmp(newPageDescr, currPageDescr))
      return 0;
    if(useStack) {
      if(bodyStack.existPageName(newPageDescr))
        return 0;
      if(!isMainPage(newPageDescr)) {
        infoBody ib;
        ib.id = currId;
        if(currBody)
          ib.readOnly = currBody->isReadOnly();

        // se la richiesta avviene da una pagina generica annulla l'id
        if(currPageDescr) {
          ib.oldPage = str_newdup(currPageDescr);
          if(IDD_STANDARD == currId)
            ib.id = 0;
          }
        if(!bodyStack.push(ib)) {
          delete []ib.oldPage;
          return 0;
          }
        }
      }
    return new P_DefBody(parent, newPageDescr);
    }

  if(!useStack) {
    if(isMainPage(currPageDescr))
      return 0;
    return allocMainMenu(parent);
    }
  // è un ritorno alla pagina precedente
  infoBody ib;
  if(!bodyStack.pop(ib))
    return 0;

  P_Body *bd;
  // se c'è l'id la pagina attuale è stata chiamata da una pagina
  // non generica
  if(ib.id) {
    bd = getBody(ib.id, parent, 0);
    if(ib.oldPage) {
      P_DefBody* pdb = dynamic_cast<P_DefBody*>(bd);
      if(pdb)
        pdb->setPageName(ib.oldPage);
      }
    }
  else
    bd = new P_DefBody(parent, ib.oldPage);
  if(bd)
    bd->setReadOnly(ib.readOnly);

  delete []ib.oldPage;
  return bd;
}
//----------------------------------------------------------------------------
pStack::pStack() : curr(0) {}
//----------------------------------------------------------------------------
pStack::~pStack()
{
  // non dovrebbe mai verificarsi
  for(int i = curr - 1; i >= 0; --i)
    delete []iB[i].oldPage;
}
//----------------------------------------------------------------------------
bool pStack::pop(infoBody& target)
{
  if(!curr)
    return false;
  target = iB[--curr];
  return true;
}
//----------------------------------------------------------------------------
bool pStack::push(const infoBody& source)
{
  if(curr >= SIZE_A(iB))
    return false;
  iB[curr++] = source;
  return true;
}
//----------------------------------------------------------------------------
bool pStack::existPageName(LPCTSTR newPageDescr)
{
#if 1
// 31-08-2009 anziché verificare se esiste e non permettere di andarci
// la toglie dalla pila
  // si richiede la prima pagina, si svuota lo stack
  if(isMainPage(newPageDescr)) {
    for(int i = curr - 1; i >= 0; --i) {
      delete []iB[i].oldPage;
      }
    curr = 0;
    return false;
    }
  for(int i = 0; i < curr; ++i) {
    if(iB[i].oldPage) {
      // pagina già visitata, si elimina dalla pila
      if(!_tcsicmp(newPageDescr, iB[i].oldPage)) {
        delete []iB[i].oldPage;
        for(++i; i < curr; ++i)
          iB[i - 1] = iB[i];
        --curr;
        }
      }
    }
  return false;
#else
  for(int i = 0; i < curr; ++i)
    if(iB[i].oldPage)
      if(!_tcsicmp(newPageDescr, iB[i].oldPage))
        return true;
  return false;
#endif
}

