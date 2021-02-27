//------------ DSERCHFL.H ----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DSERCHFL_H_
#define DSERCHFL_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#ifndef SVDIALOG_H_
  #include "svdialog.h"
#endif

#ifndef PLISTBOX_H_
  #include "plistbox.h"
#endif

#ifndef P_UTIL_H_
  #include "p_util.h"
#endif
#include "setPack.h"
//-----------------------------------------------------------
struct infoSearchParam
{
  union {
    struct {
      DWORD history : 1;
      DWORD reverse : 1;
      DWORD withDate : 1;
      DWORD orderByDate : 1;
      };
    DWORD v;
    } U;
  infoSearchParam() { U.v = 0; }
  infoSearchParam(bool h, bool r, bool d, bool o) {
    U.v = 0; U.history = h; U.reverse = r; U.withDate = d; U.orderByDate = o; }
  operator DWORD&() { return U.v; }
};
//----------------------------------------------------------------------------
void setKeyInfoSearch(LPCTSTR keyName, DWORD value);
void getKeyInfoSearch(LPCTSTR keyName, DWORD& value);
//----------------------------------------------------------------------------
class infoSearch
{
  public:
    infoSearch() :
      Title(0), Path(0),
      autoDelete(true), history(false), reverse(false), withDate(true),
      orderByDate(true)
 {}
    infoSearch(LPCTSTR title, LPCTSTR path, const infoSearch &info) :
      Title(str_newdup(title)), Path(str_newdup(path)),
      autoDelete(true), history(info.history), reverse(info.reverse), withDate(info.withDate),
      orderByDate(info.orderByDate)
 {}
    infoSearch(LPCTSTR title, LPCTSTR path, const infoSearchParam &info) :
      Title(str_newdup(title)), Path(str_newdup(path)),
      autoDelete(true), history(info.U.history), reverse(info.U.reverse), withDate(info.U.withDate),
      orderByDate(info.U.orderByDate)
 {}
    infoSearch(const infoSearchParam &info) :
      Title(0), Path(0),
      autoDelete(false), history(info.U.history), reverse(info.U.reverse), withDate(info.U.withDate),
      orderByDate(info.U.orderByDate)
 {}
    // copia di stringhe, viene allocata memoria e rilasciata nel distruttore
    infoSearch(LPCTSTR title, LPCTSTR path, bool withDate=true, bool orderByDate=false,
          bool history=false, bool reverse=false) :
      Title(str_newdup(title)), Path(str_newdup(path)),
      autoDelete(true), history(history), reverse(reverse), withDate(withDate),
      orderByDate(orderByDate)
 {}

    // viene passato un oggetto allocato esternamente, non viene allocata memoria
    // e non vengono rilasciate le risorse. L'oggetto che viene passato
    // deve restare attivo per tutta la durata di questa classe
    infoSearch(const infoSearch &info)  { *this = info; autoDelete = false; }

// in pratica, il primo costruttore viene utilizzato dal chiamante,
// il secondo dal chiamato

    ~infoSearch() { if(autoDelete) { delete []Title; delete []Path; } }

    LPCTSTR getTitle()  { return Title; }
    LPCTSTR getPath()   { return Path; }

    void copyData(const infoSearch &info)  {
      history = info.history;
      reverse = info.reverse;
      withDate = info.withDate;
      orderByDate = info.orderByDate;
      }
    void copyDataTo(infoSearchParam &info)  {
      info.U.history = history;
      info.U.reverse = reverse;
      info.U.withDate = withDate;
      info.U.orderByDate = orderByDate;
      }
    bool autoDelete;
    bool history;
    bool reverse;
    bool withDate;
    bool orderByDate;
    bool chooseParam(PWin* parent, bool noHistory);
  private:
    LPTSTR Title;  // titolo della window
    LPTSTR Path;   // percorso di ricerca dei file completo di nome del file o jolly
};
//----------------------------------------------------------------------------
bool gSearchFile(const infoSearch& info, PWin* parent, LPTSTR target);
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

