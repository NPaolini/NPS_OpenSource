//-------------------- precompHeader.h -------------------------
//-----------------------------------------------------------
#pragma once
#ifndef PRECOMPHEADER_H_
#define PRECOMPHEADER_H_
//-----------------------------------------------------------
#ifdef MAKE_CONN_DLL
  #define CONN_IMPORT_EXPORT __declspec(dllexport)

#elif defined USE_CONN_DLL
  #define CONN_IMPORT_EXPORT __declspec(dllimport)

#else
  #define CONN_IMPORT_EXPORT
#endif
//-----------------------------------------------------------
#include <Winsock2.h>
#include <windows.h>
#include <winnt.h>
#include <tchar.h>
//------------------------------------------------------------------
/*
// da decommentare per uso fuori dalla mia libreria
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
*/
#define toBool(a) ((a) ? true : false)
//------------------------------------------------------------------
typedef UINT uint;
typedef const BYTE* LPCBYTE;
//------------------------------------------------------------------
#pragma hdrstop
//-----------------------------------------------------------
#endif
