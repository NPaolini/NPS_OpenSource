#include "svMkVersionM.h"
#include "svMkVersionL.h"

#define svMkFILEVERSION     svVerM1,svVerM2,svVerL1,svVerL2
#define svMkPRODUCTVERSION  svVerM1,svVerM2,0,0

#define STRINGER(a) #a
#define svSTR_Make(a,b,c,d) STRINGER(a) ", " STRINGER(b)  ", " STRINGER(c) ", " STRINGER(d)

#define svMkSTR_FileVersion   svSTR_Make(svVerM1,svVerM2,svVerL1,svVerL2)
#define svMkSTRProductVersion svSTR_Make(svVerM1,svVerM2,0,0)
