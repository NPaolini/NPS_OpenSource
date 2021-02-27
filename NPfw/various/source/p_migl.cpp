//--------------- P_MIGL.CPP ------------------------------------------
#include "precHeader.h"
#include "p_migl.h"
//---------------------------------------------------------------------
void migliaia(LPCTSTR s, LPTSTR d, unsigned int c)
{
  if(*s == _T('-')) {
    s++;
    *d++ = _T('-');
    }
  size_t lenS = _tcslen(s);
  size_t k = lenS - 1;
  size_t lenD =  k + k / 3;
  d[lenD + 1] = 0;
  for(size_t i = 0; i < lenS; ++i, --lenD, --k) {
    if(i && !(i%3))
      d[lenD--] = TCHAR(c);
    d[lenD] = s[k];
    }
}
//---------------------------------------------------------------------
