//------- infoTree.h ---------------------------------------
//----------------------------------------------------------
#ifndef INFOTREE_H_
#define INFOTREE_H_
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#ifndef MANAGE_H_
#include "manage.h"
#endif
#include "p_avl.h"
//----------------------------------------------------------
template <class T, class F>
class infoFilter
{
  public:
    infoFilter() {}
    infoFilter(const T& filter);
    infoFilter(const infoFilter& filter);

    const infoFilter& operator=(const infoFilter& other);

    virtual int validate(const F& check) const = 0;

    const T& getData() const { return Filter; }

  protected:
  private:
    T Filter;
};
//----------------------------------------------------------
static inline FILETIME zeroTime()
{
  FILETIME ft;
  MK_I64(ft) = 0;
  return ft;
}
//----------------------------------------------------------
class rangeTime
{
  public:
    FILETIME from;
    FILETIME to;

    rangeTime(FILETIME from, FILETIME to) :
      from(from), to(to) {}
    rangeTime() : from(zeroTime()), to(zeroTime()) {}
};
//----------------------------------------------------------
//----------------------------------------------------------
typedef PVect<codeType> vCodeType;
typedef PVect<rangeTime> vRangeTime;
//----------------------------------------------------------
typedef infoFilter<codeType, fileCheck> tFilterCode;
//----------------------------------------------------------
class infoFilterCode : public tFilterCode
{
  public:
    infoFilterCode() : tFilterCode() {  }
    infoFilterCode(const codeType& filter) :
      tFilterCode(filter) {}

    int validate(const fileCheck& check) const;
};
//----------------------------------------------------------
typedef infoFilter<rangeTime, fileCheck> tFilterDate;
//----------------------------------------------------------
class infoFilterDate : public tFilterDate
{
  public:
    infoFilterDate() : tFilterDate() {  }

    infoFilterDate(const rangeTime& filter) :
      tFilterDate(filter) {}

    // torna -1 se check.init è più recente di Filter.end
    int validate(const fileCheck& check) const;
};
//----------------------------------------------------------
typedef PVect<infoFilterDate> vInfoFilterDate;
typedef PVect<infoFilterCode> vInfoFilterCode;
//----------------------------------------------------------
//----------------------------------------------------------
class dataMainList : public genericSet
{
  public:
    dataMainList(codeType code) : code(code), range(20) {}
    dataMainList() : code(0), range(20) {}
    codeType code;
    vRangeTime range;
};
//----------------------------------------------------------
//----------------------------------------------------------
// Si crea una lista(avl) con, in ogni nodo, il codice (causa o operatore)
// ed una lista(vettore) di time di inizio e fine.
// La lista primaria è mantenuta ordinata per una ricerca più veloce,
// la lista secondaria può essere implementata con l'inserimento sempre
// in coda, visto che, nel file da cui legge, i dati sono già in ordine
// cronologico.

// Nella lista secondaria può essere implementata una ricerca binaria
// per un più efficiente trasferimento nel TreeView.

// Nella lettura del file si sottopone ogni record ai vari filtri.
// I filtri sono in OR.
// Prima di iniziare a leggere dal file si leggono i filtri per stabilire
// il range massimo. Se nessun filtro è usato, i dati si riferiscono
// all'anno corrente.
//----------------------------------------------------------
class infoList : public P_Avl
{
  public:
    infoList() : P_Avl(true) {}
    ~infoList() {  Flush(); }

    bool addData(const fileCheck& check);

    bool find(const codeType& code) const;
    const codeType& getCurrCode() const;
    const vRangeTime& getCurrRange() const;

    // riempie code con tutti i codici che rientrano nel range (dispendiosa!)
//    bool getCode(vCodeType& code, const rangeTime& range) const;

  protected:
    virtual int Cmp(const TreeData ins, const TreeData nodo) const;
    vRangeTime& getRange();
//    bool addCodeIf(vCodeType& code, const rangeTime& range, const vRangeTime& v) const;
  private:
    typedef P_Avl baseClass;
};
//----------------------------------------------------------
//----------------------------------------------------------
// Si instanzia l'oggetto, si aggiungono i filtri necessari,
// si richiama perform() per creare l'albero con tutti i dati
// infine si richiede il reference costante alla infoList.
class manageInfo
{
  public:
    manageInfo(LPCTSTR prefixName);
    ~manageInfo();

    bool addFilterDate(const infoFilterDate& filter);
    bool addFilterCode(const infoFilterCode& filter);

    bool perform();

    const infoList& getInfo() const;

  protected:
  private:
    LPCTSTR prefixName;
    infoList Info;
    vInfoFilterDate FilterDate;
    vInfoFilterCode FilterCode;

    rangeTime initRangeTime();
    int makeFileList(PVect<LPTSTR>& fileList, const rangeTime& rT);
    int accept(const fileCheck& fCk);
};
//----------------------------------------------------------
//----------------------------------------------------------
//  inline
//----------------------------------------------------------
//----------------------------------------------------------
inline
manageInfo::manageInfo(LPCTSTR prefixName) :
    prefixName(str_newdup(prefixName))  { }
//----------------------------------------------------------
inline
manageInfo::~manageInfo()
{
  delete []prefixName;
}
//----------------------------------------------------------
inline
bool manageInfo::addFilterDate(const infoFilterDate& filter)
{
  FilterDate[FilterDate.getElem()] = filter;
  return true;
}
//----------------------------------------------------------
inline
bool manageInfo::addFilterCode(const infoFilterCode& filter)
{
  FilterCode[FilterCode.getElem()] = filter;
  return true;
}
//----------------------------------------------------------
inline
const infoList& manageInfo::getInfo() const { return Info; }
//----------------------------------------------------------
//----------------------------------------------------------
template <class T, class F>
infoFilter<T, F>::infoFilter(const T& filter) : Filter(filter) {}
//----------------------------------------------------------
template <class T, class F>
infoFilter<T, F>::infoFilter(const infoFilter& filter) :  Filter(filter.Filter) {}
//----------------------------------------------------------
template <class T, class F>
const infoFilter<T, F>& infoFilter<T, F>::operator=(const infoFilter& other)
{
  Filter = other.Filter;
  return *this;
}
//----------------------------------------------------------
//----------------------------------------------------------
// torna true se i due range si intersecano
inline
bool operator &(const rangeTime& first, const rangeTime& other)
{
  if(cMK_I64(first.from) > cMK_I64(other.to))
    return false;

  if(cMK_I64(first.to) < cMK_I64(other.from))
    return false;

  return true;
}
//----------------------------------------------------------
//----------------------------------------------------------
inline
bool infoList::find(const codeType& code) const
{
  dataMainList data(code);
  return baseClass::Search(reinterpret_cast<TreeData>(&data));
}
//----------------------------------------------------------
inline
const codeType& infoList::getCurrCode() const
{
  const dataMainList* data = reinterpret_cast<dataMainList*>(baseClass::getCurr());
  return data->code;
}
//----------------------------------------------------------
inline
const vRangeTime& infoList::getCurrRange() const
{
  const dataMainList* data = reinterpret_cast<dataMainList*>(baseClass::getCurr());
  return data->range;
}
//----------------------------------------------------------
inline
vRangeTime& infoList::getRange()
{
  dataMainList* data = reinterpret_cast<dataMainList*>(baseClass::getCurr());
  return data->range;
}
//----------------------------------------------------------
inline
int infoList::Cmp(const TreeData ins,const TreeData node) const
{
  dataMainList* dataIns = reinterpret_cast<dataMainList*>(ins);
  dataMainList* dataNode = reinterpret_cast<dataMainList*>(node);
  return dataIns->code > dataNode->code ? 1 :
          dataIns->code < dataNode->code ? -1 : 0;

}
//----------------------------------------------------------
//----------------------------------------------------------
#endif