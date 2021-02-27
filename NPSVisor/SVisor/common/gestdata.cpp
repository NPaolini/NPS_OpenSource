//---------- gestdata.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include "gestData.h"
#include "manage.h"

#include "mainclient.h"
#include "svDialog.h"
#include "manageMemBase.h"
#include "p_Date.h"
#include "set_cfg.h"
#include "config.h"
#include "newNormal.h"
#include "perif.h"
#include "1.h"
#include "p_util.h"
#include "printscreen.h"

#include "currAlrm.h"

#include "printDataPage.h"
//---------------------------------------------------------------------
ImplJobData::ImplJobData(mainClient *parent) : baseClass(parent),
              Stops(0), Opers(0), prfReader(0), onShowError(0)
{
}
//---------------------------------------------------------------------
ImplJobData::~ImplJobData()
{
  delete Opers;
  delete Stops;
}
//---------------------------------------------------------------------
void ImplJobData::init()
{
  baseClass::init();

  prfData data;
  data.lAddr = TEST_BIT_MODAL;
  data.typeVar = prfData::tDWData;
  data.U.dw = 0;
  set(data);

  const cfg &Cfg = config::getAll();

  if(Cfg.useCardOper) {
    LPCTSTR p = getString(ID_PRF_READER);
    if(p) {
      prfReader = _ttoi(p);
      genericPerif* reader = Par->getGenPerif(prfReader);
      if(reader) {
        if(codeOper().getElem()) {
          Stops = new manageStop(Par);
          Opers = new manageOperator(Par);
          }
        else {
          msgBoxByLangGlob(Par, ID_NOT_OPER_DEFINED, ID_NOT_OPER_DEFINED_TITLE, MB_OK | MB_ICONSTOP);
          }
        }
      }
    }
}
//---------------------------------------------------------------------
void ImplJobData::end()
{
  baseClass::end();
  delete Opers;
  Opers = 0;
  delete Stops;
  Stops = 0;
}
//---------------------------------------------------------------------
genericPerif* getAddrCodeOperData(mainClient *parent, uint& addr)
{
  static int prph = 0;
  static uint lAddr = 0;
  if(!prph) {
    LPCTSTR p = getString(ID_ADDR_CODE_OPER);
    if(!p)
      prph = -1;
    else {
      prph = _ttoi(p);
      p = findNextParamTrim(p, 1);
      if(p)
        lAddr = _ttoi(p);
      else
        prph = -1;
      }
    if(!prph)
      prph = -1;
    }
  if(-1 == prph)
    return 0;
  genericPerif* prf = parent->getGenPerif(prph);
  if(!prf) {
    prph = -1;
    return 0;
    }
  addr = lAddr;
  return prf;
}
//---------------------------------------------------------------------
void ImplJobData::checkCauseAlternateInput(gestPerif* plc)
{
  static int addr = 0;
  if(-1 == addr)
    return;

  if(!addr) {
    LPCTSTR p = getString(ID_ADDR_ALTERNATE_INPUT_CAUSE);
    if(!p) {
      addr = -1;
      return;
      }
    addr = _ttoi(p);
    if(!addr) {
      addr = -1;
      return;
      }
    }
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tDWData;
  data.U.dw = 0;
  plc->get(data);
  if(data.U.dw) {
    DWDATA value = data.U.dw;
    data.U.dw = 0;
    gestPerif::howSend old = plc->setAutoShow(gestPerif::NO_CHECK);
    plc->set(data);
    plc->setAutoShow(old);
    Stops->setAlternateChoose(value);
    }
}
//---------------------------------------------------------------------
void ImplJobData::notify()
{
  baseClass::notify();
  // la gestione della finestrina di attesa richiede di rientrare in notify()
  // per cui la parte sotto non può essere inserita in derivedNotify() ma
  // direttamente qui
  gestPerif* plc = Par->getPerif(WM_REAL_PLC);
  if(plc) {
    // da implementare
    updateFromPlc(plc);
    if(Stops && Opers) {
      int result = Stops->notify(plc, Opers->isOnRequest());
      switch(result) {
        case manageStop::rInitWaitForShow:
          SendStop(plc);
          Opers->showRequest();
          break;

        case manageStop::rHideMsg:
          SendStart(plc);
          break;
        case manageStop::rShowMsg:
        case manageStop::rWaitForShow:
          checkCauseAlternateInput(plc);
          break;
        }
      }

    genericPerif* reader = Par->getGenPerif(prfReader);
    if(Opers && reader && reader->isReady()) {
      int result = Opers->notify(plc, reader);
      switch(result) {
        case manageOperator::rChanged:
          do {
            uint addr;
            genericPerif* prph = getAddrCodeOperData(Par, addr);
            bool chN = false;
            if(prph) {
              prfData data;
              data.lAddr = addr;
              data.typeVar = prfData::tDWData;
              prph->get(data);
              DWORD code = data.U.dw;
              if(code) {
                codeOper set;
                LPCTSTR name = set.getName(code);
                if(name) {
                  Par->addInfoTitle(name);
                  chN = true;
                  }
                }
              }
            if(!chN)
              Par->addInfoTitle(_T("???"));
            } while(false);
        // fall throught
        case manageOperator::rHideMsg:
          Stops->enableShow(true);
          onShowError = 0;
          break;
        case manageOperator::rCodeNotValid:
          if(!onShowError) {
            onShowError = (DWORD)-1;
            msgBoxByLangGlob(Par, ID_NOT_OPER_DEFINED, ID_NOT_OPER_DEFINED_TITLE, MB_OK | MB_ICONSTOP);
            onShowError = 50;
            }
          else
            --onShowError;
          break;
        }
      }
    }
}
//---------------------------------------------------------------------
#define DAY_TO_I64 (HOUR_TO_I64 * 24)
//---------------------------------------------------------------------
void ImplJobData::loadDate()
{
  static int addr = 0;
  if(-1 == addr)
    return;

  if(!addr) {
    LPCTSTR p = getString(ID_ADDR_CURRENT_DATETIME);
    if(!p) {
      addr = -1;
      return;
      }
    addr = _ttoi(p);
    if(!addr) {
      addr = -1;
      return;
      }
    }
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tDateHour;
  data.U.ft = getFileTimeCurr();
  set(data);
//  set(data, true);
  MK_I64(data.U.ft) = MK_I64(data.U.ft) % DAY_TO_I64;
  data.lAddr += 2;
  set(data);
//  set(data, true);
}
//---------------------------------------------------------------------
void ImplJobData::derivedNotify()
{
  baseClass::derivedNotify();
  printScreen();
  saveScreen();
  checkFilterAlarm();
  checkPrintDataVar();

//  checkPageAndShutdown();
  loadDate();
  if(started())
    SV_Refresh();
}
//---------------------------------------------------------------------
/*
implementate tramite tasti funzione
#define ID_PAGE_SHUTDOWN_BIT 1234567
//---------------------------------------------------------------------
#define BIT_SHUTDOWN        0
#define BIT_PREV_PAGE       1
#define BIT_SHOW_TREEVIEW   2
#define BIT_PRINT_TREEVIEW  3
//---------------------------------------------------------------------
void ImplJobData::checkPageAndShutdown()
{
  static int addr = 0;
  if(-1 == addr)
    return;

  if(!addr) {
    LPCTSTR p = getString(ID_PAGE_SHUTDOWN_BIT);
    if(!p) {
      addr = -1;
      return;
      }
    addr = _ttoi(p);
    if(!addr) {
      addr = -1;
      return;
      }
    }
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tDWData;
  if(prfData::isChanged == get(data)) {
    DWDATA bits = data.U.dw;
    data.U.dw = 0;
    set(data, false);
    mainClient* mw = getMain(Par);
    if(bits & BIT_SHUTDOWN) {
      PostMessage(*mw, WM_CUSTOM, MAKEWPARAM(MSG_CLOSE_PROG, 0), 0);
      return;
      }
    if(bits & BIT_PREV_PAGE)
      PostMessage(*mw, WM_CUSTOM, MAKEWPARAM(MSG_PREV_PAGE, 0), 0);

    if(bits & BIT_PRINT_TREEVIEW)
      PostMessage(*mw, WM_CUSTOM, MAKEWPARAM(MSG_PRINT_TREEVIEW, 0), 0);
    if(bits & BIT_SHOW_TREEVIEW)
      PostMessage(*mw, WM_CUSTOM, MAKEWPARAM(MSG_SHOW_TREEVIEW, 0), 0);
    }
}
*/
//---------------------------------------------------------------------
void ImplJobData::updateFromPlc(gestPerif* plc)
{

}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static
void SendBitStartStop(gestPerif* plc, bool set)
{
  LPCTSTR pp = getString(ID_BIT_CARD);
  if(!pp)
    return;

  int addr = 0;
  int type = 0;
  int offs = 0;
  _stscanf_s(pp, _T("%d,%d,%d"), &addr, &type, &offs);
  prfData data;
  data.lAddr = addr;
  data.typeVar = type;

  if(set) {
    if(prfData::tBitData == type)
      data.U.dw = MAKELONG(MAKEWORD(1, offs), 1);
    else {
      plc->get(data);
      data.U.dw |= 1 << offs;
      }
    }
  else {
    if(prfData::tBitData == type)
      data.U.dw = MAKELONG(MAKEWORD(1, offs), 0);
    else {
      plc->get(data);
      data.U.dw &= ~(1 << offs);
      }
    }
  plc->set(data);
}
//---------------------------------------------------------------------
void ImplJobData::SendStop(gestPerif* plc)
{
  SendBitStartStop(plc, true);
}
//---------------------------------------------------------------------
void ImplJobData::SendStart(gestPerif* plc)
{
  SendBitStartStop(plc, false);
}
//---------------------------------------------------------------------
void ImplJobData::resetBitAsPing()
{
  static int Prf = 0;
  static int Addr;
  static int Type;
  static int Bit;
  if(!Prf) {
    LPCTSTR p = getString(ID_RESET_BIT_AS_PING);
    if(!p) {
      Prf = -1;
      return;
      }
    _stscanf_s(p, _T("%d,%d,%d,%d"), &Prf, &Addr, &Type, &Bit);
    }
  if(Prf < 0)
    return;

  genericPerif* prf = Par->getGenPerif(Prf);
  if(!prf) {
    Prf = -1;
    return;
    }

  prfData data;
  data.lAddr = Addr;
  data.typeVar = Type;

  prf->get(data);

  static int count;
  if(data.U.dw & (1 << Bit)) {
    if(--count <= 0) {
      count = 5;
      data.U.dw &= ~(1 << Bit);
      prf->set(data, false);
      }
    }
  else
    count = 0;

}
//---------------------------------------------------------------------
void ImplJobData::printScreen()
{
  static int Prf = 0;
  static int Addr;
  static int Bit;
  static int Preview;
  static int Setup;
  static int Type;
  static int fullscreen;
  if(!Prf) {
    LPCTSTR p = getString(ADDR_PRINT_SCREEN);
    if(!p) {
      Prf = -1;
      return;
      }
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"), &Prf, &Addr, &Type, &Bit, &Preview, &Setup, &fullscreen);
    }
  if(Prf < 0)
    return;

  genericPerif* prf = Par->getGenPerif(Prf);
  if(!prf) {
    Prf = -1;
    return;
    }

  prfData data;
  data.lAddr = Addr;
  data.typeVar = Type;

  prf->get(data);

  if(data.U.dw & (1 << Bit)) {
    data.U.dw &= ~(1 << Bit);
    prf->set(data, false);

    PostMessage(*getMain(), WM_CUSTOM, MAKEWPARAM(MSG_PRINT_SCREEN, fullscreen), MAKELPARAM(Preview, Setup));
//    ::printScreen(getCurrBody(), toBool(Preview), toBool(Setup));
    }
}
//---------------------------------------------------------------------
void ImplJobData::saveScreen()
{
  static int Prf = 0;
  static int Addr;
  static int Bit;
  static int Type;
  static int Prf2;
  static int Addr2;
  static int Len;
  static LPCTSTR filename;
  if(!Prf) {
    LPCTSTR p = getString(ADDR_SAVE_SCREEN);
    if(!p) {
      Prf = -1;
      return;
      }
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"), &Prf, &Addr, &Type, &Bit, &Prf2, &Addr2, &Len);
    filename = findNextParamTrim(p, 7);
    }
  if(Prf < 0)
    return;

  genericPerif* prf = Par->getGenPerif(Prf);
  if(!prf) {
    Prf = -1;
    return;
    }

  prfData data;
  data.lAddr = Addr;
  data.typeVar = Type;

  prf->get(data);

  if(data.U.dw & (1 << Bit)) {
    data.U.dw &= ~(1 << Bit);
    prf->set(data, false);
    if(Len) {
      data.lAddr = Addr2;
      data.setStr(Len);
      genericPerif* prf2 = Par->getGenPerif(Prf2);
      if(prf2) {
        prf2->get(data);
        TCHAR path[_MAX_PATH];
        copyStrZ(path, data.U.str.buff, Len);
        ::saveScreen(getCurrBody(), path);
        }
      else
        ::saveScreen(getCurrBody(), filename);
      }
    else
      ::saveScreen(getCurrBody(), filename);
    }
}
//--------------------------------------------------------------------
class manageFilterAlarm
{
  public:
    manageFilterAlarm() : addr(-1), autoShow(false), warningLevel(0), use64Filter(false) {}
    int getAddr()  { init(); return addr; }
    bool getAutoShow()  { init(); return autoShow; }
    int getWarningLevel()  { init(); return warningLevel; }
    bool getUse64Filter()  { init(); return use64Filter; }
  private:
    void init();
    int addr;
    bool autoShow;
    int warningLevel;
    bool use64Filter;
};
//--------------------------------------------------------------------
void manageFilterAlarm::init()
{
  if(!addr)
    return;
  if(addr < 0) {
    addr = 0;
    LPCTSTR p = getString(ID_FILTER_ALARM);
    if(!p)
      return;
    LPCTSTR p2 = findNextParamTrim(p);
    if(!p2)
      return;
    addr = _ttoi(p);
    autoShow = toBool(_ttoi(p2));
    p = findNextParamTrim(p2);
    if(p)
      warningLevel = _ttoi(p);
    p = findNextParamTrim(p);
    if(p)
      use64Filter = toBool(_ttoi(p));
    }
}
//--------------------------------------------------------------------
static manageFilterAlarm ManFilterAlarm;
//--------------------------------------------------------------------
int getAddrFilterAlarm()
{
  return ManFilterAlarm.getAddr();
}
//--------------------------------------------------------------------
bool getAutoShowFilterAlarm()
{
  return ManFilterAlarm.getAutoShow();
}
//--------------------------------------------------------------------
int getWarningLevelFilterAlarm()
{
  return ManFilterAlarm.getWarningLevel();
}
//--------------------------------------------------------------------
bool getUse64Filter()
{
  return ManFilterAlarm.getUse64Filter();
}
//----------------------------------------------------------
static
int getAddrUpdateGroupBits(int offs)
{
  int addr = getAddrFilterAlarm();
  if(!addr)
    return -1;
  addr += offs;
  if(!addr || addr >= SIZE_OTHER_DATA)
    addr = -1;

  return addr;
}
//----------------------------------------------------------
#define BITS_4_DWORD 32
//----------------------------------------------------------
static
void updateGroupBitsGen(int addr, DWORD group, bool set)
{
  if(group >= BITS_4_DWORD) {
    addr += STEP_GRP_ALARM_BITS;
    group -= BITS_4_DWORD;
    }
  group = 1 << group;
  mainClient* mc = getMain();
  ImplJobData* job = mc->getIData();
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tDWData;
  job->get(data);
  if(set)
    data.U.dw |= group;
  else
    data.U.dw &= ~group;
  job->set(data);
}
//----------------------------------------------------------
void updateGroupBitsOffset(int offs, DWORD group, bool set)
{
  int addr = getAddrUpdateGroupBits(offs);
  if(addr < 0)
    return;
  updateGroupBitsGen(addr, group, set);
}
//----------------------------------------------------------
void updateGroupBits(DWORD group, bool set)
{
  updateGroupBitsOffset(1, group, set);
}
//----------------------------------------------------------
void updateGroupBits2(DWORD group, bool set)
{
  updateGroupBitsOffset(2, group, set);
}
//----------------------------------------------------------
void updateGroupBits3(DWORD group, bool set)
{
  updateGroupBitsOffset(3, group, set);
}
//----------------------------------------------------------
void updateGroupBits4(DWORD group, bool set)
{
  updateGroupBitsOffset(4, group, set);
}
//----------------------------------------------------------
void updateGroupBits5(DWORD group, bool set)
{
  updateGroupBitsOffset(5, group, set);
}
//--------------------------------------------------------------------
void ImplJobData::checkFilterAlarm()
{
  int addr = getAddrFilterAlarm();
  if(!addr)
    return;
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tDWData;
  int repeat = 1 + getUse64Filter();
  for(int i = 0; i < repeat; ++i) {
    if(prfData::isChanged == get(data)) {
      if(data.U.dw) {
        FilterAlarm* fa = getFilterAlarm();
        fa->setDWordAt(i, data.U.dw);
        data.U.dw = 0;
        set(data, false);
        if(getAutoShowFilterAlarm())
          PostMessage(*getMain(), WM_CUSTOM, MAKEWPARAM(MSG_SHOW_ALARM, 0), 0);
        }
      }
    data.lAddr += STEP_GRP_ALARM_BITS;
    }
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//#define ID_PRINT_DATA_VAR 300101 -> 600
/* nel codice ID_PRINT_DATA_VAR ci sono i codici da cui prendere le info
   es. ID_PRINT_DATA_VAR,12345,15555

   dal codice (es. 12345) in successione ci sono le info per la variabile da controllare
   per avviare la stampa, seguono:
   nome del file di set da caricare
   nome del file template per il modello di stampa
   codice presenti all'interno del file di set che identificano
      1) periferiche
      2) indirizzi
      3) tipi di dati
      4) numero di decimali o lunghezza per tipo stringa
      5) normalizzatori o id per testo variabile a bit
      6-7)seguono due codici per la scelta del preview e del setup

   es. 12345,prf,addr,offset (si usa un solo bit per attivare la stampa)
       12346,inforicetta.txt
       12347,templatericetta.txt
       12348,2000,2001,2002,2003,2004,1,1

       poi all'interno di inforicetta.txt
          2000,1,1,1,1
          2001,100,101,102,103
          2002,4,4,5,1
          2203,1,0,2,0
          2204,12,0,0,10000

          10000,non attivo,attivo

       il file inforicetta.txt può essere una pagina usata per un body o un semplice
       file creato solo per la stampa.

*/
//-----------------------------------------------------------
void ImplJobData::checkPrintDataVar()
{
  static bool inExec;
  if(inExec)
    return;
  // necessario per periferiche esterne, ci potrebbe volere un po' di tempo
  // prima di avere dati aggiornati e ci potrebbe essere un rientro indesiderato
  inExec = true;
  LPCTSTR p = getString(ID_PRINT_DATA_VAR);
  if(!p)
    // inutile azzerare inExec, anzi, in questo modo si eliminano anche
    // le successive ricerche inutili
    return;

  do {
    int id = _ttoi(p);
    if(id) {
      LPCTSTR p2 = getString(id);
      int idPrf = 0;
      int idAddr = 0;
      int offset = 0;
      _stscanf_s(p2, _T("%d,%d,%d"), &idPrf, &idAddr, &offset);
      prfData data;
      data.lAddr = idAddr;
      data.typeVar = prfData::tBitData;
      data.U.dw = MAKELONG(1, offset);
      genericPerif* prf = Par->getGenPerif(idPrf);
      if(prf) {
        prf->get(data);
        if(data.U.dw) {
          data.U.dw = MAKELONG(MAKEWORD(1, offset), 0);
          prf->set(data);
          performPrintDataVar(id + 1);
          }
        }
      }
    p = findNextParam(p, 1);
    } while(p);
  inExec = false;
}
//-----------------------------------------------------------
extern svPrinter* getPrinter();
//-----------------------------------------------------------
void ImplJobData::performPrintDataVar(int id)
{
  LPCTSTR pageSet = getString(id);
  LPCTSTR Template = getString(id + 1);
  LPCTSTR p = getString(id + 2);
  int idPrf = 0;
  int idAddr = 0;
  int idType = 0;
  int idnDec = 0;
  int idNorm = 0;
  int preview = 1;
  int setup = 0;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"), &idPrf, &idAddr, &idType, &idnDec, &idNorm,
        &preview, &setup);
  infoPrintDataPage ipdp(pageSet, idPrf, idAddr, idType, idnDec, idNorm);
  printDataPage pdp(Par, getPrinter(), Template, ipdp);
  if(preview)
    pdp.Preview(toBool(setup));
  else
    pdp.Print(toBool(setup));
}
//-----------------------------------------------------------
#define NBIT_AL (sizeof(DWDATA) * 8)
//-----------------------------------------------------------
int ImplJobData::getNBitAlarm() const { return NBIT_AL; }
//-----------------------------------------------------------
static int addrAlarm = -1;
//-----------------------------------------------------------
int ImplJobData::getAddrAlarm() const
{
  return addrAlarm;
}
//-----------------------------------------------------------
void ImplJobData::logAlarm(log_status &log, int which, bool force)
{
  if(!addrAlarm)
    return;
  LPCTSTR p = getString(ID_MANAGE_ALARM_PLC + 8);
  if(!p) {
    addrAlarm = 0;
    return;
    }
  static int numWordAlarm = 0;
  static DWORD offsEvent = 0;
  static DWORD numWordEvent = 0;
  if(-1 == addrAlarm) {
    _stscanf_s(p, _T("%d,%d,%d,%d"), &addrAlarm, &numWordAlarm, &offsEvent, &numWordEvent);
    if(numWordAlarm <= 0) {
      addrAlarm = 0;
      return;
      }
    }
  LPDWORD curr = currData + addrAlarm;
  LPDWORD old =  oldData + addrAlarm;

  for(int i = 0; i < numWordAlarm; ++i) {
    DWDATA val = curr[i];
    if(!force)
      // con lo xor si attivano solo i bit diversi
      val ^= old[i];
    if(val) {
      for(int j = 0, k = 1; j < NBIT_AL; ++j, k <<= 1) {
        if(val & k) {
          val_gen v;
          v.id = makeIdAlarmByPrph(NBIT_AL * i + j, 1);
          v.val = which;
          if(curr[i] & k)
            log.setEvent(log_status::onAlarm, &v);
          else if(!force)
            log.setEvent(log_status::endAlarm, &v);
          }
        }
      }
    }
}
