//---------- pdllComm.h ---------------------------------------------
#ifndef pdllComm_H_
#define pdllComm_H_
#include "precHeader.h"
//-------------------------------------------------------------------
// implementazione standard, la classe deve chiamarsi così ed avere il membro statico
class pDll
{
  public:
    pDll() {}
    ~pDll() {}

    static bool init();
    static void end();
    static prtConnProcDLL ConnProcDLL;
  private:
    static HMODULE hDLL;
    static int loadedWSA;
    static LRESULT ConnProcDummy(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn) { return 0; }
    static bool loadProc();
    static int User;
};
//------------------------------------------------------------------
#endif
