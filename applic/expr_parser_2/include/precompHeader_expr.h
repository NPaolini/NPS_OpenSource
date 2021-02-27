//------------ precompHeader_expr.h -------------------------
//-----------------------------------------------------------
#pragma once
#ifndef PRECOMPHEADER_EXPR_H_
#define PRECOMPHEADER_EXPR_H_
//-----------------------------------------------------------
#ifdef MAKE_PARSER_DLL
  #define PARSER_IMPORT_EXPORT __declspec(dllexport)

#elif defined USE_PARSER_DLL
  #define PARSER_IMPORT_EXPORT __declspec(dllimport)

#else
  #define PARSER_IMPORT_EXPORT
#endif
//-----------------------------------------------------------
#include <windows.h>
#include <winnt.h>
#include <tchar.h>
#include <stdint.h>
//------------------------------------------------------------------
#define toBool(a) ((a) ? true : false)
//------------------------------------------------------------------
typedef UINT uint;
typedef const BYTE* LPCBYTE;
//------------------------------------------------------------------
#pragma hdrstop
//-----------------------------------------------------------
#endif
