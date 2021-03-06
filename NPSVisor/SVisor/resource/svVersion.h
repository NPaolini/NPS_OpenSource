#include "svVersionM.h"
#include "svVersionL.h"

#define svFILEVERSION     svVerM1,svVerM2,svVerL1,svVerL2
#define svPRODUCTVERSION  svVerM1,svVerM2,svVerL1,0

#define STRINGER(a) #a
#define svSTR_Make(a,b,c,d) STRINGER(a) ", " STRINGER(b)  ", " STRINGER(c) ", " STRINGER(d)

#define svSTR_FileVersion   svSTR_Make(svVerM1,svVerM2,svVerL1,svVerL2)
#define svSTRProductVersion svSTR_Make(svVerM1,svVerM2,svVerL1,0)

#ifdef NO_SSE2
  #define svSTRSSE2 "No SSE2 Instructions"
  #define svSTR_DESCR "SuperVisor for Windows NoSSE2"
#else
  #define svSTRSSE2 "With SSE2 Instructions"
  #define svSTR_DESCR "SuperVisor for Windows"
#endif
