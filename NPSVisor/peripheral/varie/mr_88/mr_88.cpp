//--------------- mr_88.cpp ------------------------------------------
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#include "mr_88.h"
//------------------------------------------------------------------------
gestComm *allocGestComm(void* par, uint idPrph)
{
  PWin * w = reinterpret_cast<PWin*>(par);
  if(w) {
    TCHAR buff[100];
    wsprintf(buff, _T("Mixer MR 88 Ver.7 [%d]"), idPrph);
    w->setCaption(buff);
    }
  TCHAR name[_MAX_PATH];
  MK_NAME_FILE_DATA(name, idPrph);
  return new mr_88(name, FLAG_PERIF(idPrph));
}
//------------------------------------------------------------------------
#define SIZE_DATA_DEF 4
#define MUL_ADDR(a) ((a) * SIZE_DATA_DEF)
#define DIV_ADDR(a) ((a) / SIZE_DATA_DEF)

#define GET_OUTPUT_CHANNEL 0x00

#define GET_INPUT_CHANNEL1 0x01
#define GET_INPUT_CHANNEL2 0x02
#define GET_INPUT_CHANNEL3 0x03
#define GET_INPUT_CHANNEL4 0x04
#define GET_INPUT_CHANNEL5 0x05
#define GET_INPUT_CHANNEL6 0x06
#define GET_INPUT_CHANNEL7 0x07
#define GET_INPUT_CHANNEL8 0x08
#define GET_SYSTEM_SETTING 0x09
#define GET_MONITOR_SETTING 0x0A
#define GET_STATUS_METERS  0x0B
#define GET_VERSION        0x7F
//------------------------------------------------------------------------
#if true
#define DIM_GET_OUTPUT_CHANNEL 5
#define DIM_GET_INPUT_CHANNEL1 14
#define DIM_GET_SYSTEM_SETTING 15
#define DIM_GET_MONITOR_SETTING 4
#define DIM_GET_STATUS_METERS  17
#define DIM_GET_VERSION        7
#else
#define DIM_GET_OUTPUT_CHANNEL 4
#define DIM_GET_INPUT_CHANNEL1 13
#define DIM_GET_SYSTEM_SETTING 13
#define DIM_GET_MONITOR_SETTING 3
#define DIM_GET_STATUS_METERS  13
#define DIM_GET_VERSION        7
#endif

#define STEP_INPUT_CHANNEL 20
//------------------------------------------------------------------------
#define ADDR_GET_OUTPUT_CHANNEL 5
#define ADDR_GET_INPUT_CHANNEL1 11

#define GET_INP_CH_PARAM(a) (a), ADDR_GET_INPUT_CHANNEL1 + STEP_INPUT_CHANNEL * ((a) - 1), DIM_GET_INPUT_CHANNEL1

#define ADDR_GET_SYSTEM_SETTING   171
#define ADDR_GET_MONITOR_SETTING  190
#define ADDR_GET_STATUS_METERS    200
#define ADDR_GET_VERSION          220
//------------------------------------------------------------------------
#define OFFSET_SET 1000
#define OFFSET_SET_BITS 2000
//------------------------------------------------------------------------
#define SET_OUTPUT_CHANNEL 0x0C

#define SET_INPUT_CHANNEL1 0x0D
#define SET_INPUT_CHANNEL2 0x0E
#define SET_INPUT_CHANNEL3 0x0F
#define SET_INPUT_CHANNEL4 0x10
#define SET_INPUT_CHANNEL5 0x11
#define SET_INPUT_CHANNEL6 0x12
#define SET_INPUT_CHANNEL7 0x13
#define SET_INPUT_CHANNEL8 0x14

#define SET_SYSTEM_SETTING 0x15
#define SET_MONITOR_SETTING 0x16
#define SET_RESET_FACTORY  0x17
//------------------------------------------------------------------------
#define ADDR_SET_OUTPUT_CHANNEL (ADDR_GET_OUTPUT_CHANNEL + OFFSET_SET)
#define ADDR_SET_INPUT_CHANNEL1 (ADDR_GET_INPUT_CHANNEL1 + OFFSET_SET)

#define SET_INP_CH_PARAM(a) (a) + SET_INPUT_CHANNEL1 - 1, ADDR_SET_INPUT_CHANNEL1 + STEP_INPUT_CHANNEL * ((a) - 1), DIM_GET_INPUT_CHANNEL1

#define ADDR_SET_SYSTEM_SETTING (ADDR_GET_SYSTEM_SETTING + OFFSET_SET)
#define ADDR_SET_MONITOR_SETTING (ADDR_GET_MONITOR_SETTING + OFFSET_SET)

#define ADDR_SET_RESET_FACTORY    1200
#define DIM_SET_RESET_FACTORY     0
//------------------------------------------------------------------------
#define MAX_BIT_RECEIVE 13
#define ALL_BIT_RECEIVE ((1 << MAX_BIT_RECEIVE) - 1)

#define MAX_BIT_SEND    12
#define ALL_BIT_SEND ((1 << MAX_BIT_SEND) - 1)

#define ADDR_BITS_F_RECEIVE    2000
#define ADDR_BITS_A_RECEIVE    2001
#define ADDR_BITS_SEND         2002
#define ADDR_BITS_SEND_ALWAYS  2003
#define ADDR_BITS_A_RECEIVE_RESET    1
#define ADDR_BITS_SEND_ALWAYS_RESET  3

#define ADDR_DISABLE_OUTPUT 1198

#define TO_BITS(a) (1 << (a))
#define BITS_ALWAYS_RECEIVE    TO_BITS(11)
//#define BITS_ALWAYS_RECEIVE    (TO_BITS(0) | TO_BITS(10) | TO_BITS(11))
#define BITS_ALWAYS_DEFAULT_SEND (TO_BITS(0) | TO_BITS(1) | TO_BITS(2) | TO_BITS(3) | TO_BITS(4) | TO_BITS(5) | TO_BITS(6) | TO_BITS(7) | TO_BITS(8) | TO_BITS(10))
//#define BITS_ALWAYS_DEFAULT_SEND ((TO_BITS(MAX_BIT_SEND - 3) - 1) | TO_BITS(10))
//------------------------------------------------------------------------
static command* cData_receive[MAX_BIT_RECEIVE] = { 0 };
static command* cData_send[MAX_BIT_SEND] = { 0 };
//------------------------------------------------------------------------
static void make_cData_receive()
{
  cData_receive[0] = new command_receive(GET_OUTPUT_CHANNEL, ADDR_GET_OUTPUT_CHANNEL, DIM_GET_OUTPUT_CHANNEL);
  cData_receive[1] = new command_receive_input_channel(GET_INP_CH_PARAM(1));
  cData_receive[2] = new command_receive_input_channel(GET_INP_CH_PARAM(2));
  cData_receive[3] = new command_receive_input_channel(GET_INP_CH_PARAM(3));
  cData_receive[4] = new command_receive_input_channel(GET_INP_CH_PARAM(4));
  cData_receive[5] = new command_receive_input_channel(GET_INP_CH_PARAM(5));
  cData_receive[6] = new command_receive_input_channel(GET_INP_CH_PARAM(6));
  cData_receive[7] = new command_receive_input_channel(GET_INP_CH_PARAM(7));
  cData_receive[8] = new command_receive_input_channel(GET_INP_CH_PARAM(8));
  cData_receive[9] = new command_receive(GET_SYSTEM_SETTING, ADDR_GET_SYSTEM_SETTING, DIM_GET_SYSTEM_SETTING);
  cData_receive[10] = new command_receive(GET_MONITOR_SETTING, ADDR_GET_MONITOR_SETTING, DIM_GET_MONITOR_SETTING);
  cData_receive[11] = new command_receive(GET_STATUS_METERS, ADDR_GET_STATUS_METERS, DIM_GET_STATUS_METERS);
  cData_receive[12] = new command_receive(GET_VERSION, ADDR_GET_VERSION, DIM_GET_VERSION);
};
static void make_cData_send()
{
  cData_send[0] = new command_send(SET_OUTPUT_CHANNEL, ADDR_SET_OUTPUT_CHANNEL, DIM_GET_OUTPUT_CHANNEL);
  cData_send[1] = new command_send_input_channel(SET_INP_CH_PARAM(1));
  cData_send[2] = new command_send_input_channel(SET_INP_CH_PARAM(2));
  cData_send[3] = new command_send_input_channel(SET_INP_CH_PARAM(3));
  cData_send[4] = new command_send_input_channel(SET_INP_CH_PARAM(4));
  cData_send[5] = new command_send_input_channel(SET_INP_CH_PARAM(5));
  cData_send[6] = new command_send_input_channel(SET_INP_CH_PARAM(6));
  cData_send[7] = new command_send_input_channel(SET_INP_CH_PARAM(7));
  cData_send[8] = new command_send_input_channel(SET_INP_CH_PARAM(8));
  cData_send[9] = new command_send(SET_SYSTEM_SETTING, ADDR_SET_SYSTEM_SETTING, DIM_GET_SYSTEM_SETTING);
  cData_send[10] = new command_send(SET_MONITOR_SETTING, ADDR_SET_MONITOR_SETTING, DIM_GET_MONITOR_SETTING);
  cData_send[11] = new command_send(SET_RESET_FACTORY, ADDR_SET_RESET_FACTORY, DIM_SET_RESET_FACTORY);
};
//------------------------------------------------------------------------
/*
    nel ciclo di lettura si verifica se il blocco ha il bit corrispondente attivo (prima quello di forzatura e poi quello automatico)
    se il bit è attivo si esegue la lettura da periferica e, se forzato, si resetta il bit
    se il bit non è attivo si ignora la richiesta e si torna noAction

    se si attiva un bit di scrittura si esegue direttamente la scrittura del pacchetto e si attiva il bit della lettura forzata

    la lettura/scrittura dei bit avviene rispettivamente nei buffer Buff_Perif/Buff_Lav
    per la lettura e l'invio dei dati si usa Buff_Perif, per la ricezione si può usare il buffer passato
*/
//------------------------------------------------------------------------
mr_88::mr_88(LPCTSTR file, WORD flagCommand) : baseClass(file, flagCommand), Conn(0), disableOutput(false)
{
//  MessageBox(0, _T("wait"), _T("..."), MB_OK); // per poter connettere in debug all'avvio
}
//------------------------------------------------------------------------
mr_88::~mr_88()
{
  if(disableOutput)
    setOutput(0, 0);
  delete Conn;
  for(uint i = 0; i < SIZE_A(cData_receive); ++i)
    safeDeleteP(cData_receive[i]);
  for(uint i = 0; i < SIZE_A(cData_send); ++i)
    safeDeleteP(cData_send[i]);
  saveData();
}
//-------------------------------------------------------
void mr_88::setOutput(int a, int b)
{
  DWORD tmp[MAX_DATA_REC] = {0};
  int addr = getLogicalAddr(ADDR_SET_OUTPUT_CHANNEL);
  LPDWORD pdwL = (LPDWORD)Buff_Lav;
  tmp[0] = pdwL[addr];
  tmp[1] = pdwL[addr + 1];
  tmp[2] = a;
  tmp[3] = b;
  cData_send[0]->run(Conn, tmp);
}
//-------------------------------------------------------
int dualCmp(const dual& d1, const dual& d2)
{
  return (int)d1.addr - (int)d2.addr;
}
//-------------------------------------------------------
void mr_88::makeDual(setOfString& set)
{
  const uint nElem = MAX_DIM_PERIF / DEF_TYPE_SIZE;
  set.setFirst();
  uint nDual = 0;
  alternAddress tmpAddr;
  do {
    if(set.getCurrId() >= nElem)
      break;
    LPCTSTR p = set.getCurrString();
    stringToAddr(1, 1, p, &tmpAddr);
    dual d;
    d.logicAddr = set.getCurrId() - 1;
    d.addr = DIV_ADDR(tmpAddr.addr);
    dualSet.insertEx(d, dualCmp);
    } while(set.setNext());
  set.setFirst();
}
//-------------------------------------------------------
void mr_88::verifySet(setOfString& set)
{
  baseClass::verifySet(set);
  if(set.setFirst()) {
    makeDual(set);
    return;
    }
}
//------------------------------------------------------------------------
gestComm::ComResult mr_88::Init()
{
  make_cData_receive();
  make_cData_send();
  gestComm::ComResult result = baseClass::Init();
#if false
  LPDWORD p = (LPDWORD)Buff_Lav;
  *p = ALL_BIT_RECEIVE;
#endif
  commIdentity *cI = getIdentity();
  uint useLanTest_port = cI->getData();
  if(useLanTest_port)
    Conn = new PConnBase(_T("127.0.0.1"), useLanTest_port, 0, false);
  else {
    DWORD ipc = cI->getPCom();
    Conn = new PConnBase(paramConn(ipc), 0);
    }
  if(!Conn->open())
    result = gestComm::OpenComErr;
  loadData();
  return result;
}
//------------------------------------------------------------------------
#define FILENAME_DATA _T("save_mr88.dat")
//------------------------------------------------------------------------
void mr_88::loadData()
{
  P_File pf(FILENAME_DATA, P_READ_ONLY);
  if(pf.P_open())
    pf.P_read(Buff_Lav, MAX_DIM_PERIF);

  LPDWORD pdwP = (LPDWORD)Buff_Perif;
  LPDWORD pdwL = (LPDWORD)Buff_Lav;
  // forza l'invio automatico del blocco al cambiamento di un valore
  uint addr = getLogicalAddr(ADDR_BITS_SEND_ALWAYS);
  pdwP[addr] |= BITS_ALWAYS_DEFAULT_SEND;
  pdwL[addr] |= BITS_ALWAYS_DEFAULT_SEND;
  // forza la lettura automatica dei meter
  addr = getLogicalAddr(ADDR_BITS_A_RECEIVE);
  pdwP[addr] |= BITS_ALWAYS_RECEIVE;
  pdwL[addr] |= BITS_ALWAYS_RECEIVE;

  // forza la prima lettura
  addr = getLogicalAddr(ADDR_BITS_F_RECEIVE);
  pdwP[addr] |= ALL_BIT_RECEIVE;
  pdwL[addr] |= ALL_BIT_RECEIVE;

  addr = getLogicalAddr(ADDR_DISABLE_OUTPUT);
  disableOutput = toBool(pdwL[addr]);
  if(disableOutput)
    setOutput(1, 2);
}
//------------------------------------------------------------------------
void mr_88::saveData()
{
  P_File pf(FILENAME_DATA, P_CREAT);
  if(pf.P_open())
    pf.P_write(Buff_Perif, MAX_DIM_PERIF);
}
//------------------------------------------------------------------------
gestComm::ComResult mr_88::Send(const addrToComm*, Perif_TypeCommand)
{
  return gestComm::noAction;
}
//------------------------------------------------------------------------
uint mr_88::getLogicalAddr(uint addr)
{
  dual d;
  d.addr = addr;
  uint pos;
  if(dualSet.find(d, pos, dualCmp))
    return dualSet[pos].logicAddr;
  return addr;
}
//-----------------------------------------------------
void mr_88::copyBuffData(LPDWORD buff, uint num_data, uint addr)
{
  LPDWORD pdw = (LPDWORD)Buff_Perif;
  dual d;
  d.addr = addr;
  uint pos;
  for(uint i = 0; i < num_data; ++i, ++d.addr) {
    if(dualSet.find(d, pos, dualCmp))
      buff[i] = pdw[dualSet[pos].logicAddr];
    }
}
//-----------------------------------------------------
static bool is_inside_block(command* cmd, uint addr)
{
  uint a = cmd->get_init_addr();
  return addr >= a && addr < a + cmd->get_len_data();
}
//-----------------------------------------------------
// usato per forzare il commit quando si copiano i dati sia in Buff_Lav che in Buff_Perif
// serve solo se non si usa il memorymapped
#define FORCE_DIFF_ADDR 0
//-----------------------------------------------------
gestComm::ComResult mr_88::saveBuffData(const addrToComm* pAddr)
{
  LPDWORD pdwT = (LPDWORD)Buff_Lav;
  LPDWORD pdwT2 = (LPDWORD)Buff_Perif;
  LPDWORD pdwS = (LPDWORD)pAddr->buff;
  dual d;
  d.addr = DIV_ADDR(pAddr->addr);
  uint pos;
  for(uint i = 0; i < pAddr->len; ++i, ++d.addr) {
    if(dualSet.find(d, pos, dualCmp)) {
      pos = dualSet[pos].logicAddr;
      pdwT[pos] = pdwT2[pos] = pdwS[i];
      }
    }
  pdwT[FORCE_DIFF_ADDR] = 1;
  pdwT2[FORCE_DIFF_ADDR] = 0;

  LPDWORD p = (LPDWORD)Buff_Perif;
  DWORD bits = p[getLogicalAddr(ADDR_BITS_SEND_ALWAYS)];
  uint nblk = SIZE_A(cData_send) - 1; // per il reset non serve ...
  gestComm::ComResult res = gestComm::noActionButSave;
  uint addr = DIV_ADDR(pAddr->addr);
  for(uint i = 0; i < nblk; ++i) {
    if(bits & (1 << i)) {
      if(is_inside_block(cData_send[i], addr)) {
        addrToComm atc = *pAddr;
        atc.addr = MUL_ADDR(ADDR_BITS_SEND);
        *(LPDWORD)atc.buff = 1 << i;
        gestComm::ComResult r = SendData(&atc);
        if(r != gestComm::noActionButSave)
          res = r;
//        break;
        }
      }
    }
  return res;
}
//------------------------------------------------------------------------
gestComm::ComResult mr_88::SaveBuff(addrToComm* pAddr)
{
  LPDWORD pdwT = (LPDWORD)Buff_Lav;
  LPDWORD pdwT2 = (LPDWORD)Buff_Perif;
  uint addr = getLogicalAddr(DIV_ADDR(pAddr->addr));
  pdwT[addr] = *(LPDWORD)pAddr->buff;
  pdwT2[addr] = *(LPDWORD)pAddr->buff;
  return gestComm::noActionButSave;
}
//------------------------------------------------------------------------
gestComm::ComResult mr_88::SendData(addrToComm* pAddr)
{
  if(Logoff)
    return gestComm::NoErr;
  LPDWORD p = (LPDWORD)Buff_Lav;
  uint dwAddr = DIV_ADDR(pAddr->addr);
  switch(dwAddr) {
    case ADDR_BITS_SEND:
      break;
    case ADDR_BITS_A_RECEIVE_RESET:
    case ADDR_BITS_SEND_ALWAYS_RESET:
      do {
        uint addr = getLogicalAddr(dwAddr + OFFSET_SET_BITS);
        p[addr] &= ~*(LPDWORD)pAddr->buff;
        addr = getLogicalAddr(dwAddr);
        p[addr] = 0;
        *(LPDWORD)pAddr->buff = 0;
        } while(false);
      return gestComm::noActionButSave;

    case ADDR_DISABLE_OUTPUT:
      disableOutput = *(LPDWORD)pAddr->buff;
      return SaveBuff(pAddr);

    case ADDR_BITS_F_RECEIVE:
    case ADDR_BITS_A_RECEIVE:
    case ADDR_BITS_SEND_ALWAYS:
      do {
        uint addr = getLogicalAddr(dwAddr);
#if 0
#error provare ad usare direttamente tutto il contenuto (non c'è bisogno del reset) -> *(LPDWORD)pAddr->buff = p[addr];
#endif
         *(LPDWORD)pAddr->buff |= p[addr];
      } while(false);
      return SaveBuff(pAddr);
    default:
      return saveBuffData(pAddr);
    }
  gestComm::ComResult res = gestComm::noActionButSave;
  DWORD bits = *(LPDWORD)pAddr->buff;

  DWORD tmp[MAX_DATA_REC];
  for(uint i = 0; i < SIZE_A(cData_send); ++i) {
    if(bits & (1 << i)) {
      copyBuffData(tmp, cData_send[i]->get_len_data(), cData_send[i]->get_init_addr());
      if(!cData_send[i]->run(Conn, tmp))
        res = gestComm::SendErr;
      if(i < SIZE_A(cData_send) - 1) { // esclude il bit di reset
        // imposta il bit di lettura forzata
        uint addr = getLogicalAddr(ADDR_BITS_F_RECEIVE);
        p[addr] |= 1 << i;
        }
      *(LPDWORD)pAddr->buff &= ~(1 << i);
//      break;
      }
    }
  return res;
}
//------------------------------------------------------------------------
#ifdef TEST_RECEIVE
//------------------------------------------------------------------------
void  mr_88::updateDataSend(uint ix, LPDWORD buff)
{
  int init_source = cData_send[ix]->get_init_addr();
  int len_data = cData_receive[ix]->get_len_data();
  LPDWORD p = (LPDWORD)Buff_Lav;
  for(int i = 0; i < len_data; ++i) {
    int s = getLogicalAddr(init_source + i);
    buff[i] = p[s];
    }
}
//------------------------------------------------------------------------
gestComm::ComResult mr_88::Receive(const addrToComm* pAddr, BDATA* readBuff)
{
  if(Logoff)
    return gestComm::NoErr;
  LPDWORD p = (LPDWORD)Buff_Perif;

  uint addr = getLogicalAddr(ADDR_BITS_F_RECEIVE);
  DWORD f_bits = p[addr];
  DWORD all_bits = f_bits |  p[getLogicalAddr(ADDR_BITS_A_RECEIVE)];
  bool found = false;
  gestComm::ComResult res = gestComm::noAction;
  uint dwAddr = DIV_ADDR(pAddr->addr);

  for(uint i = 0; i < SIZE_A(cData_receive); ++i) {
    if((all_bits & (1 << i)) && cData_receive[i]->get_init_addr() == dwAddr) {
      res = gestComm::noAction;
      if(i != 0 && i != 10 && i != 11 && i < MAX_BIT_SEND - 1 && gestComm::NoErr == res) {
        updateDataSend(i, (LPDWORD)readBuff);
        res = gestComm::NoErr;
        }
      f_bits &= ~(1 << i);
      found = true;
      break;
      }
    }
  if(found) {
    if(f_bits != p[addr]) {
      p = (LPDWORD)Buff_Lav;
      p[addr] = f_bits;
      }
    }
  return res;
}
//------------------------------------------------------------------------
#else
//------------------------------------------------------------------------
void  mr_88::updateDataSend(uint ix)
{
  if(cData_send[ix]->isUpdated())
    return;
  int init_source = cData_receive[ix]->get_init_addr();
  int init_target = cData_send[ix]->get_init_addr();
  int len_data = cData_send[ix]->get_len_data();
  LPDWORD p = (LPDWORD)Buff_Lav;
  for (int i = 0; i < len_data; ++i) {
    int t = getLogicalAddr(init_target + i);
    int s = getLogicalAddr(init_source + i);
    p[t] = p[s];
  }
  cData_send[ix]->setUpdated();
}
//------------------------------------------------------------------------
#define REPEAT_LOAD 0xff
//------------------------------------------------------------------------
gestComm::ComResult mr_88::Receive(const addrToComm* pAddr, BDATA* readBuff)
{
  if(Logoff)
    return gestComm::NoErr;
  uint dwAddr = DIV_ADDR(pAddr->addr);
  if(ADDR_DISABLE_OUTPUT == dwAddr) {
    *(LPDWORD)readBuff = disableOutput;
    return gestComm::NoErr;
    }

  LPDWORD p = (LPDWORD)Buff_Perif;
  uint addr = getLogicalAddr(ADDR_BITS_F_RECEIVE);

  DWORD f_bits = p[addr];
  DWORD all_bits = f_bits |  p[getLogicalAddr(ADDR_BITS_A_RECEIVE)];

  static uint forceReload;
  if(!(forceReload % REPEAT_LOAD))
    all_bits |= (TO_BITS(9) | TO_BITS(11));
  ++forceReload;

  bool found = false;
  gestComm::ComResult res = gestComm::noAction;

  for(uint i = 0; i < SIZE_A(cData_receive); ++i) {
    if((all_bits & (1 << i)) && is_inside_block(cData_receive[i], dwAddr)) {
      res = cData_receive[i]->run(Conn, (LPDWORD)readBuff) ? gestComm::NoErr : gestComm::RecErr;
      if(i < MAX_BIT_SEND - 1 && gestComm::NoErr == res)
        updateDataSend(i);
      f_bits &= ~(1 << i);
      found = true;
      break;
      }
    }
  if(found) {
    if(f_bits != p[addr]) {
      p = (LPDWORD)Buff_Lav;
      p[addr] = f_bits;
      }
    }
  return res;
}
//------------------------------------------------------------------------
#endif
