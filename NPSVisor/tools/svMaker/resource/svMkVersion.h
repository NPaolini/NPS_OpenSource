#include "svMkVersionM.h"
#include "svMkVersionL.h"

#define svMkFILEVERSION     svVerM1,svVerM2,svVerL1,svVerL2
#define svMkPRODUCTVERSION  svVerM1,svVerM2,0,0

#define STRINGER(a) #a
#define svSTR_Make(a,b,c,d) STRINGER(a) ", " STRINGER(b)  ", " STRINGER(c) ", " STRINGER(d)

#define svMkSTR_FileVersion   svSTR_Make(svVerM1,svVerM2,svVerL1,svVerL2)
#define svMkSTRProductVersion svSTR_Make(svVerM1,svVerM2,0,0)

#ifdef NO_SSE2
  #define svSTRSSE2 "No SSE2 Instructions"
  #define svSTR_DESCR "SvMaker for Windows NoSSE2"
#else
  #define svSTRSSE2 "With SSE2 Instructions"
  #define svSTR_DESCR "SvMaker for Windows"
#endif
