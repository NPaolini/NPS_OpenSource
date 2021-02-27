//----------- P_EXIT.CPP --------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pDef.h"
#include "p_exit.h"
//-------------------------------------------------------------------
void  my_exit(LPCTSTR msg, LPCTSTR file, int line)
{
  static bool ending = false;
  if(ending)
    return;
  ending = true;
  TCHAR buff[500];
  wsprintf(buff, _T("Errore nel file %s,\nlinea n. %d\n[%s]"), file, line, msg);
//#define FATAL_ERROR
#ifdef FATAL_ERROR
  ::FatalAppExit(0, buff);
#else
  ::MessageBox(0, buff, _T("End Application"), MB_OK | MB_ICONSTOP);
  ::PostQuitMessage(0);
#endif
}

