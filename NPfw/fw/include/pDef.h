//------------------ pDef.h ----------------------------------------
//------------------------------------------------------------------
#ifndef PDEF_H_
#define PDEF_H_
//------------------------------------------------------------------
#ifndef STRICT
  #define STRICT
#endif
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
//------------------------------------------------------------------
#include "precHeader.h"
#include <tchar.h>
//------------------------------------------------------------------
// per libreria framework
#ifdef MAKE_DLL
  #define IMPORT_EXPORT __declspec(dllexport)

#elif defined USE_DLL
  #define IMPORT_EXPORT __declspec(dllimport)

#else
  #define IMPORT_EXPORT

#endif
//------------------------------------------------------------------
#ifdef __cplusplus
#define SIZE_A(arr) ( \
   0 * sizeof(reinterpret_cast<const ::Bad_arg_to_SIZE_A*>(arr)) + \
   0 * sizeof(::Bad_arg_to_SIZE_A::check_type((arr), &(arr))) + \
   sizeof(arr) / sizeof((arr)[0]) )

struct Bad_arg_to_SIZE_A {
   class Is_pointer; // incomplete
   class Is_array {};
   template <typename T>
   static Is_pointer check_type(const T*, const T* const*);
   static Is_array check_type(const void*, const void*);
};
#else
#define SIZE_A(a) (sizeof(a) / sizeof(*a))
#endif
#define SIZE_A_c(a) SIZE_A(a)
//------------------------------------------------------------------
#define NO_COPY_COSTR_OPER(a) \
  private:\
    a(const a&);\
    const a& operator=(const a&);
//------------------------------------------------------------------
#define CLONE(a) a = other.a
//------------------------------------------------------------------
typedef UINT uint;
typedef const BYTE* LPCBYTE;
//------------------------------------------------------------------
#endif
