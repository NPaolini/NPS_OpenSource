//------------------- P_EXIT.H ------------------------------------------------
#ifndef P_EXIT__
#define P_EXIT__
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------------------------
void my_exit(LPCTSTR msg, LPCTSTR name, int line);
#define My_Exit(msg) my_exit(msg, _T(__FILE__), __LINE__)
//-----------------------------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------------------------
#endif
