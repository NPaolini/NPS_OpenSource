//---------- customLoadClass.cpp ---------------------------------------------
//----------------------------------------------------------------------------
// implementazione standard, la classe deve chiamarsi così ed avere il membro statico
class pDll
{
  public:
    ~pDll();

    static prtConnProcDLL ConnProcDLL;
  private:
    // non si deve instanziare
    pDll();
    static pDll dummy;
    static HMODULE hDLL;
    static int errLoadedWSA;
    static bool loadProc();
    static LRESULT ConnProcDummy(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn) { return 0; }
};
//------------------------------------------------------------------
pDll pDll::dummy;
HMODULE pDll::hDLL;
int pDll::errLoadedWSA;
prtConnProcDLL pDll::ConnProcDLL;
//------------------------------------------------------------------
#define NAME_DLL _T("nps_conn")
//------------------------------------------------------------------
pDll::pDll()
{
  if(!loadProc())
    MessageBox(0, _T("Unable to load [") NAME_DLL _T(".dll]"), _T("Error"), MB_OK | MB_ICONSTOP);
  else {
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;

    errLoadedWSA = WSAStartup(wVersionRequested, &wsaData);
    }
}
//------------------------------------------------------------------
pDll::~pDll()
{
  if(hDLL)
    FreeLibrary(hDLL);
  if(!errLoadedWSA)
    WSACleanup();

}
//------------------------------------------------------------------
static LPCTSTR libName = NAME_DLL _T(".dll");
static LPCSTR procName = "_ConnProcDLL";
//------------------------------------------------------------------
bool pDll::loadProc()
{
  hDLL = LoadLibrary(libName);
  if(hDLL) {
    FARPROC f = GetProcAddress(hDLL, procName);
    if(!f)
      f = GetProcAddress(hDLL, procName + 1);
    if(f) {
      ConnProcDLL = (prtConnProcDLL)f;
      return true;
      }
    }
  ConnProcDLL = ConnProcDummy;
  return false;
}
//----------------------------------------------------------------------------
