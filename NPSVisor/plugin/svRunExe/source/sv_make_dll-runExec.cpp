//-------- sv_make_dll.h -------------------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "sv_make_dll.h"
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
//------------------------------------------------------------------
#include "p_vect.h"
//-----------------------------------------------------------
// Verifica stato singolo bit
//-----------------------------------------------------------
static bool isSetBit(uint prf, uint ind, uint offset, bool reset)
{
  SV_prfData data;
  data.typeVar = SV_prfData::tDWData;
  data.lAddr = ind;

  npSV_Get(prf, &data);
  if(data.U.dw & (1 << offset)) {
    if(reset) {
      data.U.dw &= ~(1 << offset);
      npSV_Set(prf, &data);
      }
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
static void run(LPTSTR path, uint delay, PROCESS_INFORMATION& pi)
{
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_FORCEONFEEDBACK;

  if(CreateProcess(0, path, 0, 0, 0, NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi)) {
    WaitForInputIdle(pi.hProcess, 10 * 1000);
    Sleep(delay);
    CloseHandle(pi.hThread);
    pi.hThread = 0;
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
class runExec
{
  public:
    runExec(uint prph, uint addr, uint offs, bool hided, LPCTSTR path) :
      Prph(prph), Addr(addr), Offs(offs), Hided(hided), count(0)
    {
      ZeroMemory(&PI, sizeof(PI));
      _tcscpy_s(Path, path);
    }
    ~runExec();

    void update();

  private:
    bool check_running();
    PROCESS_INFORMATION PI;
    TCHAR Path[_MAX_PATH];
    uint Addr;
    uint Prph;
    uint Offs;
    bool Hided;
    uint count;
};
//-----------------------------------------------------------
runExec::~runExec()
{
  if(PI.hProcess)
    CloseHandle(PI.hProcess);
  if(PI.hThread)
    CloseHandle(PI.hThread);
}
//-----------------------------------------------------------
bool runExec::check_running()
{
  if(PI.hProcess) {
    DWORD result = WaitForSingleObject(PI.hProcess, 0);
    switch(result) {
      // è terminato
      case WAIT_OBJECT_0:
      case WAIT_ABANDONED:
        CloseHandle(PI.hProcess);
        PI.hProcess = 0;
        break;

      // è ancora in esecuzione, nessuna azione
      case WAIT_TIMEOUT:
        return true;
      }
    }
  return false;
}
//-----------------------------------------------------------
#define NEED_CHECK_COUNT 10
void runExec::update()
{
  if(!isSetBit(Prph, Addr, Offs, true)) {
    if(++count >= NEED_CHECK_COUNT) {
      count = 0;
      check_running();
      }
    return;
    }

  if(check_running())
    return;
  run(Path, 0, PI);
}
//-----------------------------------------------------------
static PVect<runExec*> RunExecutable;
//------------------------------------------------------------------
#define ADDR_EXE     11
#define MAX_ADDR_EXE 10
#define MAX_BIT_EXEC (sizeof(DWORD) * 8)
//------------------------------------------------------------------
ExpImp_DLL
void npSV_System(DWORD msg)
{
  if(eEND_DLL == msg) {
    flushPV(RunExecutable);
    return;
    }
  else if(eINIT_DLL == msg) {

    uint nEl = 0;
    for(int i = 0; i < MAX_ADDR_EXE; ++i) {
      uint addr = i + ADDR_EXE;

      // questa parte serve solo per evitare di allocare inutilmente se
      // la riga non esiste
      LPCTSTR p = npSV_GetLocalString(addr, getHinstance());
      if(!p)
        continue;
      uint addr2 = 0;
      uint prph;
      uint hide;

      _stscanf_s(p, _T("%d,%d,%d,%d"), &addr2, &prph, &addr, &hide);
      if(!addr2)
        continue;
      for(int j = 0; j < MAX_BIT_EXEC; ++j) {
        LPCTSTR pexec = npSV_GetLocalString(addr2 + j, getHinstance());
        if(!pexec)
          continue;
        runExec* re = new runExec(prph, addr, j, hide, pexec);
        RunExecutable[nEl++] = re;
        }
      }
    }
}
//-----------------------------------------------------------
ExpImp_DLL
void npSV_Refresh()
{
  uint nElem = RunExecutable.getElem();
  for(uint i = 0; i < nElem; ++i)
    RunExecutable[i]->update();
}
//------------------------------------------------------------------
