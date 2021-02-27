//-------------------- pCommonFilter.h ---------------------------
//-----------------------------------------------------------
#ifndef PCOMMONFILTER_H_
#define PCOMMONFILTER_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PEDIT_H_
  #include "pEdit.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
template <typename T>
PFilter* t_allocCopy(const T& source)
{
  T* obj = new T;
  obj->clone(source);
  return obj;
}

//-----------------------------------------------------------
class PNumbFilter : public PFilter
{
  public:
    bool accept(WPARAM& key, HWND ctrl);
    virtual PFilter* allocCopy()  const { return t_allocCopy<PNumbFilter>(*this); }
};
//-----------------------------------------------------------
class PNumbFilterComma : public PNumbFilter
{
  private:
    typedef PNumbFilter baseClass;
  public:
    PNumbFilterComma();
    bool accept(WPARAM& key, HWND ctrl);
    virtual PFilter* allocCopy()  const { return t_allocCopy<PNumbFilterComma>(*this); }
  protected:
    UINT comma;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class PDateTimeFilter : public PFilter
{
  private:
    typedef PFilter baseClass;
  public:
    PDateTimeFilter() : yearLast(true) { }
    bool accept(WPARAM& key, HWND ctrl);
    virtual void initCtrl(HWND ctrl);
    bool isStandard() const { return yearLast; }
    void setStandard(bool set) { yearLast = set; }

    virtual void clone(const PDateTimeFilter& other) { if(this != &other) *this = other; }
    virtual void clone(const PFilter& other);
  protected:
    virtual TCHAR getSep() = 0;
    virtual int getLimitLen() = 0;

    // true indica se la data è nel formato standard xx/xx/aaaa
    // false nel formato inverso aaaa/xx/xx
    // non fa controlli se i dati siano giusti, solo la posizione
    // del separatore
    bool yearLast;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#define MAX_DATE_LEN 10
//-----------------------------------------------------------
class PDateFilter : public PDateTimeFilter
{
  public:
    virtual PFilter* allocCopy()  const { return t_allocCopy<PDateFilter>(*this); }
    virtual void onKillFocus(HWND ctrl);
  protected:
    virtual TCHAR getSep() { return _T('/'); }
    virtual int getLimitLen() { return MAX_DATE_LEN; }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#define MAX_TIME_LEN 8
//-----------------------------------------------------------
class PTimeFilter : public PDateTimeFilter
{
  public:
    virtual PFilter* allocCopy()  const { return t_allocCopy<PTimeFilter>(*this); }
  protected:
    virtual TCHAR getSep() { return _T(':'); }
    virtual int getLimitLen() { return MAX_TIME_LEN; }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class PDateAndTimeFilter : public PFilter
{
  private:
    typedef PFilter baseClass;
  public:
    PDateAndTimeFilter() : yearLast(true) { }
    bool accept(WPARAM& key, HWND ctrl);
    virtual void initCtrl(HWND ctrl);
    bool isStandard() const { return yearLast; }
    void setStandard(bool set) { yearLast = set; }

    virtual void clone(const PDateAndTimeFilter& other) { if(this != &other) *this = other; }
    virtual void clone(const PFilter& other);
    virtual PFilter* allocCopy()  const { return t_allocCopy<PDateAndTimeFilter>(*this); }
  protected:
    virtual int getLimitLen()  { return MAX_DATE_LEN + MAX_TIME_LEN + 3; }

    // true indica se la data è nel formato standard xx/xx/aaaa
    // false nel formato inverso aaaa/xx/xx
    // non fa controlli se i dati siano giusti, solo la posizione
    // del separatore
    bool yearLast;
};
//-----------------------------------------------------------
class PHexFilter : public PFilter
{
  public:
    virtual PFilter* allocCopy()  const { return t_allocCopy<PHexFilter>(*this); }
  public:
    bool accept(WPARAM& key, HWND ctrl);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class PBinaryFilter : public PFilter
{
  public:
    virtual PFilter* allocCopy()  const { return t_allocCopy<PBinaryFilter>(*this); }
  public:
    bool accept(WPARAM& key, HWND ctrl);
};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
