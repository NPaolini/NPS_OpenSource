//----------- smartPS.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef smartPS_H_
#define smartPS_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
template <typename T, bool isArray=true>
class P_SmartPointer
{
  public:
    P_SmartPointer() : P(0), autoDelete(false) {}
    explicit P_SmartPointer(T p, bool autodelete) : P(p), autoDelete(autodelete) {}
    P_SmartPointer(const P_SmartPointer& other) { P = 0; move(other); }
    const P_SmartPointer& operator=(const P_SmartPointer& other) { move(other); return *this; }
    const P_SmartPointer& operator=(T other) { move(P_SmartPointer(other, false)); return *this; }

    ~P_SmartPointer() { if(autoDelete) { if(isArray) delete []P; else delete P; } }
    T operator&() const { return P; }
    operator T() const { return P; }
    bool operator!() const { return P ? false : true; }
    bool operator()() const { return P ? true : false; }
  private:
    mutable T P;
    mutable bool autoDelete;

    void move(const P_SmartPointer& other);
};
//----------------------------------------------------------------------------
#include "smartPS.hpp"
//----------------------------------------------------------------------------
typedef P_SmartPointer<LPCTSTR> smartPointerConstString;
//----------------------------------------------------------------------------
typedef P_SmartPointer<LPTSTR> smartPointerString;
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
