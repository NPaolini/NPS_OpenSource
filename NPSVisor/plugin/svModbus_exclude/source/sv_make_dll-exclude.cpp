//-------- sv_make_dll-exclude.h -----------------------------------
//------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "p_util.h"
#include "sv_make_dll.h"
#include "hdrmsg.h"
#include <stdio.h>
//------------------------------------------------------------------
BOOL initProcess(HINSTANCE hI)
{
  return TRUE;
}
//------------------------------------------------------------------
void endProcess()
{
}
//------------------------------------------------------------------
#define _REG(a) const UINT WM_##a = RegisterWindowMessage(_T("WM_") _T(#a))
//------------------------------------------------------------------
_REG(MAIN_PRG);
//------------------------------------------------------------------
/*
  questa DLL necessita del sVisor Versione 4.4.0.9 o successive

  nel file di testo allegato ci devono essere le informazioni nella forma:

  id,idDriver,idPrph,addr,numdw
  es.
  10,2,1,12,1
  11,3,1,13,3
  ecc. fino a max 17

  il tipo di dato è sempre DWORD

  poi, per renderle visibili al programma di checkAddr
  2000000,1,12,4
  2000001,1,13,4-7
*/
//------------------------------------------------------------------
#define ID_PRPH_EXCLUDE       10
#define MAX_ID_PRPH_EXCLUDE   8
//------------------------------------------------------------------
#define MAX_DW   8
//------------------------------------------------------------------
class checkExclude
{
  public:
    checkExclude(LPCTSTR p = 0);
//    ~checkExclude();

    void check();
  private:
    HWND hwnd;
    uint idDriver;

    uint idPrph;
    uint addr;
    uint numDW;

    int count;

    DWDATA oldData[MAX_DW];

};
//------------------------------------------------------------------
checkExclude::checkExclude(LPCTSTR p) :
    hwnd(0), idDriver(0), idPrph(0), addr(0), count(0), numDW(0)
{
  ZeroMemory(oldData, sizeof(oldData));
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d"), &idDriver, &idPrph, &addr, &numDW);
  if(!numDW || numDW > MAX_DW)
    numDW = 1;
}
//------------------------------------------------------------------
void checkExclude::check()
{
  if(!idDriver || !idPrph)
    return;

  if(!hwnd) {
    // siamo in attesa di un nuovo tentativo
    if(--count > 0)
      return;

    if(!npSV_GetBodyRoutine(eSV_GET_HANDLE_PRPH, (LPDWORD)&hwnd, (LPDWORD)idDriver)) {
      // se count è minore di zero è il primo tentativo, allora inizializza il contatore
      if(count < 0)
        count = 40;
      // è il secondo tentativo fallito, azzera il driver così non lo cerca più
      else
        idDriver = 0;
      return;
      }
    }

  // se per qualche motivo l'handle non fosse più valido prova a ricaricarlo
  if(!IsWindow(hwnd)) {
    hwnd = 0;
    count = 0;
    return;
    }

    // controlla quali bit siano cambiati ed invia solo quelli (uno alla volta).
    // nel file adr del modbus-exclude la posizione del bit corrisponde al db.
    // es. bit 0 (primo bit) -> db 1, bit 1 -> db2, ecc.

  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = SV_prfData::tDWData;
  for(uint j = 0, k = 0; j < numDW; ++j, ++data.lAddr, k += 32) {
    if(SV_prfData::okData <= npSV_Get(idPrph, &data)) {
      for(uint i = 0; i < 32; ++i) {
        DWORD mask = 1 << i;
        if((oldData[j] & mask) ^ (data.U.dw & mask))
#ifdef ENABLE_ON_BIT_UP
          // se l'abilitazione si ha col bit alto
          PostMessage(hwnd, WM_MAIN_PRG, MAKEWPARAM(MSG_ENABLE_DISABLE, toBool(data.U.dw & mask)), (LPARAM)(k + i + 1));
#else
          // se la disabilitazione si ha col bit alto
          PostMessage(hwnd, WM_MAIN_PRG, MAKEWPARAM(MSG_ENABLE_DISABLE, toBool(oldData[j] & mask)), (LPARAM)(k + i + 1));
#endif
        }
      oldData[j] = data.U.dw;
      }
    }
}
//------------------------------------------------------------------
static PVect<checkExclude> Check;
//------------------------------------------------------------------
ExpImp_DLL
void npSV_System(DWORD msg)
{
  if(eINIT_DLL == msg) {
    for(int i = 0; i < MAX_ID_PRPH_EXCLUDE; ++i) {
      LPCTSTR p = npSV_GetLocalString(ID_PRPH_EXCLUDE + i, getHinstance());
      if(!p)
        break;
      Check[i] = checkExclude(p);
      }
    }
}
//------------------------------------------------------------------
ExpImp_DLL
void npSV_Refresh()
{
  uint nElem = Check.getElem();
  for(uint i = 0; i < nElem; ++i)
    Check[i].check();
}
//------------------------------------------------------------------
