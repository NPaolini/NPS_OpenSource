//-------------------- precHeader.h ---------------------------
//-----------------------------------------------------------
#pragma once
#ifndef PRECHEADER_H_
#define PRECHEADER_H_
//-----------------------------------------------------------
#ifndef STRICT
  #define STRICT 1
#endif
#define WIN32_LEAN_AND_MEAN
#define GDIPVER 0x0110
//-----------------------------------------------------------
#include <windows.h>
#include <winnt.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#define toBool(a) ((a) ? true : false)
#include "pDef.h"
#include "pWin.h"
//#include "resource.h"
//-----------------------------------------------------------
#pragma hdrstop
//-----------------------------------------------------------
#endif
