//---------- manageMemBase.cpp -----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include "mainclient.h"

#include "manageMemBase.h"
#include "p_Date.h"
#include "p_Queue.h"
#include "set_cfg.h"
#include "config.h"
#include "newNormal.h"
#include "perif.h"
#include "file_lck.h"
#include "p_file.h"
#include "def_dir.h"
//#include "log_oper.h"
#include "val_ord.h"

#include "imntnce.h"
#include "1.h"
//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include "p_util.h"
#include "p_avl.h"
//---------------------------------------------------------------------
class oldFile : public genericSet
{
  public:
    oldFile(const FILETIME& ft, LPCTSTR path, __int64 size) : Ft(ft), Path(str_newdup(path)), Size(size) { }
    ~oldFile() { delete []Path; }
    int Val;

    bool operator >(const oldFile& other) { return Ft > other.Ft; }

    LPCTSTR getPath() const { return Path; }
    __int64 getDim() const { return Size; }
  private:
    LPCTSTR Path;
    __int64 Size;
    FILETIME Ft;

    NO_COPY_COSTR_OPER(oldFile)
};
//---------------------------------------------------------------------
typedef genericAvl<oldFile> orderedOldFile;
//---------------------------------------------------------------------
static void setBitLastDWORD(ImplJobMemBase* cl, uint offs, bool set)
{
  prfData data;
  data.typeVar = prfData::tBitData;
  data.lAddr = SIZE_OTHER_DATA - 1;
  data.U.dw = MAKELONG(MAKEWORD(1, offs), set);
  cl->set(data);
}
//---------------------------------------------------------------------
static void setBitRun(ImplJobMemBase* cl, bool set)
{
  setBitLastDWORD(cl, 0, set);
}
//---------------------------------------------------------------------
static void setBitOnline(ImplJobMemBase* cl, bool set)
{
  setBitLastDWORD(cl, 1, set);
}
//---------------------------------------------------------------------
static inline void MSG__(PWin* parent, int idMsg, int idTitle)
{
  LPCTSTR msg = getStringOrId(idMsg);
  LPCTSTR title = getStringOrId(idTitle);
  svMessageBox(parent, msg, title, MB_OK);
}
//---------------------------------------------------------------------
#define JOB_FILE_NAME _T("job_base.dat")
//---------------------------------------------------------------------
#define DEF_TIME_TO_SAVE 120000
#define MIN_TIME_TO_SAVE 50
//#define MIN_TIME_TO_SAVE 2000
//---------------------------------------------------------------------
class saveByTime
{
  public:
    saveByTime(bool& needSave, criticalSect& cSect, uint time = DEF_TIME_TO_SAVE);
    ~saveByTime();

    uint setTime(uint time);
    uint getTime() { return Time; }

  private:
    bool& needSave;
    criticalSect& cSect;

    bool Quit;

    HANDLE hEvent;
    HANDLE hThread;

    volatile uint Time;

    void reset();
    void setNeedSave() { criticalLock cl(cSect); needSave = true; }

    friend DWORD WINAPI saveByTimeProc(LPVOID);
};
//---------------------------------------------------------------------
class clearOldFile
{
  public:
    clearOldFile(DWORD megabyte, DWORD minFile) : hEvent(CreateEvent(0, 1, 0, 0)),
        MegaByte(megabyte), onExec(false), minFile(minFile), canCheck(false),
        needShow(true), idThread(0)
    {}
    ~clearOldFile();

    bool init(PWin* parent);
    void check(PWin* parent);
  private:
    HANDLE hEvent;
    DWORD idThread;
    bool onExec;
    DWORD minFile;
    bool canCheck;
    bool needShow;

    __int64 getAllFile(orderedOldFile& set);
    void clearFile();
    DWORD getNextTime(bool& before);
    __int64 fillSet(orderedOldFile& set, LPTSTR path);
    DWORD MegaByte;
    friend DWORD WINAPI clearOldFileProc(LPVOID);
};
//---------------------------------------------------------------------
static clearOldFile* COF;
//---------------------------------------------------------------------
#define DELAY_TO_CHECK_HISTORY (1000 * 60 * 60 * 12)
//---------------------------------------------------------------------
ImplJobMemBase::ImplJobMemBase(mainClient *parent) :
    Par(parent), numTrend(0), SaveTrend(0), Started(false),
    BitsNoCommit(SIZE_OTHER_DATA), pMF(0), currData(0) //, G_M(allocMaint())
{
  ZeroMemory(&jobFile, sizeof(jobFile));
  currData = jobFile.data;
  lastTick = GetTickCount() - DELAY_TO_CHECK_HISTORY * 2;
}
//---------------------------------------------------------------------
ImplJobMemBase::~ImplJobMemBase()
{
  save();
//  delete G_M;
  delete COF;
  delete pMF;
}
//-----------------------------------------------------------------
void ImplJobMemBase::openMapping(bool useFileMapped)
{
  if(useFileMapped) {
    currData = jobFile.data;
    if(!pMF) {
      TCHAR path[_MAX_PATH] = JOB_FILE_NAME;
      getBasePath(path);
      bool exist = P_File::P_exist(path);
      pMF = new p_MappedFile(path);
      if(!pMF->P_open(sizeof(job_file))) {
        delete pMF;
        pMF = 0;
        return;
        }
      currData = ((LPDWORD)pMF->getAddr());
      if(!exist)
        ZeroMemory(currData, sizeof(job_file));
      }
    else {
      delete pMF;
      pMF = 0;
      openMapping(useFileMapped);
      }
    }
  else { // !useFileMapped
    if(pMF) {
      delete pMF;
      pMF = 0;
      }
    currData = jobFile.data;
    }
}
//---------------------------------------------------------------------
//#define INIT_FIXED_DATA 800
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//#define ID_ADDR_ALWAYS_LOCAL 49
//-----------------------------------------------------------
typedef PVect<int> pVectInt;
//-----------------------------------------------------------
void fillSetWithRange(pVectInt& v, LPCTSTR p)
{
  int i = 0;
  while(p && *p) {
    v[i] = _ttoi(p);
    ++i;
    LPCTSTR p2 = p + 1;
    while(*p2 && _istdigit(*p2))
      ++p2;
    if(_T('-') == *p2) {
      p = p2 + 1;
      int last = _ttoi(p);
      int curr = v[i - 1] + 1;
      while(curr < last)
        v[i++] = curr++;
      }
    else if(*p2)
      p = p2 + 1;
    else
      p = p2;
    while(p && *p && !_istdigit((unsigned)*p))
      ++p;
    }
}
//---------------------------------------------------------------------
extern bool useMappedFilePrph1();
//---------------------------------------------------------------------
void ImplJobMemBase::init()
{
  // se non è in sola lettura, usa il file mappato
  openMapping(!isReadOnly() && useMappedFilePrph1());

  // la prima volta carica tutto da remoto (se remoto)
//  load();

  do {
    LPCTSTR p = getString(ID_ADDR_ALWAYS_LOCAL);
    if(p && -1 == _ttoi(p))
      StatusBits.U.b.readOnlyWrite = 1;
    else
      fillSetWithRange(vNoReload, p);
    } while(false);

  load();

  setBitRun(this, false);

  LPCTSTR p = getString(INIT_FIXED_DATA);
  if(p) {
    int nData = _ttoi(p);
    for(int i = 0; i < nData; ++i) {
      p = getString(INIT_FIXED_DATA + i + 1);
      if(!p)
        break;
      int addr = _ttoi(p);
      p = findNextParam(p, 1);
      if(findNextParam(p, 1, _T('.'))) {
        float val = (float)_tstof(p);
        *(float*)(&currData[addr]) = val;
        }
      else {
        int val = _ttoi(p);
        currData[addr] = val;
        }
      }
    save();
    }

  for(int i = 0; i < SIZE_A(oldData); ++i)
    oldData[i] = currData[i];
//    oldData[i] = ~currData[i];

//  if(isReadOnly())
//    return ;

  do {
    p = getString(INIT_TREND_DATA);
    PVect<uint> trendCode;
    while(p) {
      uint code = _ttol(p);
      if(!code)
        break;
      trendCode[numTrend] = code;
      ++numTrend;
      p = findNextParam(p, 1);
      }
    if(numTrend)
      allocTrend(trendCode);
    } while(false);
  Started = true;
  if(isReadOnly())
    return ;

    // attiva/disattiva il flag per il report di tutti gli allarmi
  do {
    const cfg &Cfg = config::getAll();
    val_gen val;
    val.id = Cfg.logAllAlarm;
    Par->setEvent(log_status::setRepAll, &val);
    } while(false);

    // inizializzazione di default
  Par->setEvent(log_status::onInit);

  Par->logAlarm(rALL, true);
  Par->checkRunOrStop(true, rALL);

  notifySaveLog(true);

#define MIN_TREND_DATA_DIM 50
#define MAX_TREND_DATA_DIM  20000

  if(COF)
    return;
  p = getString(ID_MAX_TREND_DATA_DIM);
  DWORD mb = 0;
  if(numTrend && p) {
    mb = _ttoi(p);
    if(mb < MIN_TREND_DATA_DIM || mb > MAX_TREND_DATA_DIM)
      mb = 0;
    }
  if(mb) {
    COF = new clearOldFile(mb, numTrend * 2);
    if(!COF->init(Par)) {
      delete COF;
      COF = 0;
      }
    }
}
//---------------------------------------------------------------------
void ImplJobMemBase::allocTrend(const PVect<uint>& trendCode)
{
  SaveTrend = new save_trend*[numTrend];
  int j = 0;
  for(uint i = 0; i < numTrend; ++i) {
    SaveTrend[j] = new save_trend(cSect, trendCode[j]);
    if(!SaveTrend[j]->init(this))
      delete SaveTrend[j];
    else
      ++j;
    }
  numTrend = j;
}
//---------------------------------------------------------------------
void ImplJobMemBase::end()
{
  if(!isReadOnly()) {
    notifySaveLog(true);
    // se c'è un ordine attivo lo sospende
    chgOrd(oSuspended);
    }
  for(int i = numTrend - 1; i >= 0; --i)
    delete SaveTrend[i];
  delete []SaveTrend;
  SaveTrend = 0;
  numTrend = 0;
}
//---------------------------------------------------------------------
void ImplJobMemBase::save()
{
  if(pMF)
    return;
  criticalLock cl(cSectSet);
  if(StatusBits.U.b.accessingFile)
    return;
  StatusBits.U.b.accessingFile = true;

  TCHAR path[_MAX_PATH];
  if(isReadOnly()) {
    _tcscpy_s(path, SIZE_A(path), getLocalBaseDir());
    if(*path)
      appendPath(path, JOB_FILE_NAME);
    else
      _tcscpy_s(path, SIZE_A(path), JOB_FILE_NAME);
    }
  else {
    _tcscpy_s(path, SIZE_A(path), JOB_FILE_NAME);
    getBasePath(path);
    }

  #if 1
  P_File_Lock f(path);
  if(f.isLocked())
  #else
  P_File f(path, P_CREAT);
  if(f.P_open())
  #endif
    f.P_write(&jobFile, sizeof(jobFile));
  StatusBits.U.b.accessingFile = false;
}
//---------------------------------------------------------------------
void ImplJobMemBase::load()
{
  if(pMF)
    return;
  criticalLock cl(cSectSet);
  if(StatusBits.U.b.accessingFile)
    return;
  StatusBits.U.b.accessingFile = true;
  job_file jf;
  ZeroMemory(&jf, sizeof(jf));
  bool online = false;
  do {
    TCHAR path[_MAX_PATH];

    if(isReadOnly() && StatusBits.U.b.readOnlyWrite) {
      _tcscpy_s(path, SIZE_A(path), getLocalBaseDir());
      if(*path)
        appendPath(path, JOB_FILE_NAME);
      else
        _tcscpy_s(path, SIZE_A(path), JOB_FILE_NAME);
      }
    else {
      _tcscpy_s(path, SIZE_A(path), JOB_FILE_NAME);
      getBasePath(path);
      }

    P_File f(path, P_READ_ONLY);
    for(uint i = 0; i < 2; ++i) {
      bool success = false;
      if(f.P_open()) {
        lUDimF len = (lUDimF)f.get_len();
        if(len) {
          len = min(len, sizeof(jf));
          success = len == f.P_read(&jf, len);
          }
        else
          success = true;
        f.P_close();
        }
      if(success) {
        online = true;
        break;
        }
      Sleep(100);
      }
    } while(false);
  if(online) {
    if(isReadOnly() && !StatusBits.U.b.readOnlyWrite) {
      int nElem = vNoReload.getElem();
      for(int i = 0; i < nElem; ++i) {
        int ix = vNoReload[i];
        jf.data[ix] = currData[ix];
        }
      setBitOnline(this, online);
      }
    memcpy(&jobFile, &jf, sizeof(jf));
    }
  StatusBits.U.b.accessingFile = false;
}
//---------------------------------------------------------------------
static void moveFileToHistory(LPCTSTR file)
{
  TCHAR tmpfile[_MAX_PATH] = _T("\0");

  makePath(tmpfile, file, dExRep, true, _T(""));

  int len = _tcslen(file);
  // raggiunge l'ultima '\\' (5 == "\\" + ".ext")
  int i = _tcslen(tmpfile) - 1 - len;
  tmpfile[i] = 0;

  CreateDirectory(tmpfile, 0);

  // ripristina
  tmpfile[i] = _T('\\');

  TCHAR pathHist[_MAX_PATH];
  LPTSTR dummy;
  GetFullPathName(tmpfile, SIZE_A(pathHist), pathHist, &dummy);

  makePath(tmpfile, file, dExRep, false, _T(""));

  TCHAR pathCurr[_MAX_PATH];
  GetFullPathName(tmpfile, SIZE_A(pathCurr), pathCurr, &dummy);
#if 1
  DeleteFile(pathHist);
#else
  do {
    WIN32_FIND_DATA fd;
    HANDLE hfff = FindFirstFile(pathHist, &fd);
    if(INVALID_HANDLE_VALUE != hfff) {
      FindClose(hfff);
      if(IDNO == svMessageBox(getCurrBody()->getParent(), getStringOrId(ID_MSG_EXIST_MOVE),
                  getStringOrId(ID_TITLE_EXIST_MOVE),
                  MB_YESNO | MB_ICONSTOP))
        return;
      DeleteFile(pathHist);
      }
    } while(0);
#endif
  MoveFile(pathCurr, pathHist);
}
//---------------------------------------------------------------------
void ImplJobMemBase::notifySaveLog(bool onQuit)
{
  if(isReadOnly())
    return ;
  for(uint i = 0; i < numTrend; ++i) {
    if(onQuit || SaveTrend[i]->canSave()) {
      SaveTrend[i]->resetSave();
      if(!SaveTrend[i]->getEnableSave(Par))
        continue;

      if(!SaveTrend[i]->fillData(Par, onQuit))
        continue;

      TCHAR path[_MAX_PATH];
      bool history;
      if(!SaveTrend[i]->makeName(Par, path, history))
        continue;

      if(history) {
        DWORD diffTick = GetTickCount() - lastTick;
        if(DELAY_TO_CHECK_HISTORY <= diffTick) {
          lastTick = GetTickCount();
          TCHAR tPath[_MAX_PATH];
          SaveTrend[i]->makeName(Par, tPath, history, 1);
          TCHAR tmp[_MAX_PATH];
          makePath(tmp, tPath, dExRep, false, _T(""));
          if(P_File::P_exist(tmp))
            moveFileToHistory(tPath);
          }
        }
      makePath(path, dExRep, false, _T(""));
      P_File pf(path);
      if(pf.P_open()) {
#if 1
        DWORD lenrec = SaveTrend[i]->getLenRec();
        DWORD pos = (DWORD)pf.get_len();
        pos /= lenrec;
        pos *= lenrec;
        pf.P_seek(pos);
#else
        pf.P_seek(0, SEEK_END_);
#endif
        SaveTrend[i]->save(pf);
        }
      }
    }
  if(!onQuit && COF)
    COF->check(Par);
}
//---------------------------------------------------------------------
#define WAIT_RELOAD 4
//---------------------------------------------------------------------
void ImplJobMemBase::notify()
{
  if(StatusBits.U.b.onProcessing)
    return;
  StatusBits.U.b.onProcessing = 1;

  // per modifiche da connessione remota
  static int needReload;
  if(++needReload >= WAIT_RELOAD) {
    // se non c'è stato un set() da qualche pagina
    if(!StatusBits.U.b.needSaveJob) {
      needReload = 0;
      load();
      }
    }

  notifySaveLog();

  gestPerif* prf = Par->getPerif(WM_REAL_PLC);
  if(prf) {
//  if(!isReadOnly() && prf) {
    gestPerif::statMachine running = prf->isRunning();

    if(gestPerif::sOnRun == running) {
//      if(!G_M->isRunning()) {
//        G_M->start();
        setBitRun(this, true);
//        }
      }
    else if(gestPerif::sOnStop == running) {
//      if(G_M->isRunning()) {
//        G_M->stop();
        setBitRun(this, false);
//        }
      }
    }
  StatusBits.U.b.saveLater = 1;
  derivedNotify();
  StatusBits.U.b.saveLater = 0;

  if(StatusBits.U.b.needSaveJob) {
    StatusBits.U.b.needSaveJob = false;
    save();
    }

  perifsSet *pSet = Par->getPerifs();
  if(pSet->isReady() || !pSet->setFirst()) {
    StatusBits.U.b.needRefresh = false;
    }
  StatusBits.U.b.onProcessing = 0;
}
//---------------------------------------------------------------------
void ImplJobMemBase::checkTimerTrend()
{
  for(uint i = 0; i < numTrend; ++i)
    SaveTrend[i]->checkTimerTrend(this);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
#define NBITS_DWORD (8 * sizeof(DWORD))
//-----------------------------------------------------------
void ImplJobMemBase::commit()
{
  criticalLock cl(cSectSet);
  if(StatusBits.U.b.onProcessing)
    return;
  if(StatusBits.U.b.needSaveJob) {
    StatusBits.U.b.needSaveJob = 0;
    save();
    }
  if(BitsNoCommit.hasSet()) {
    static DWORD tmp[sizeof(oldData) / sizeof(DWORD)];
    memcpy(tmp, oldData, sizeof(oldData));
    memcpy(oldData, currData, sizeof(oldData));
    DWORD ndw = BitsNoCommit.getNumDWORD();
    for(uint i = 0; i < ndw; ++i) {
      if(BitsNoCommit.isDWordSet(i)) {
        DWORD ix = i * NBITS_DWORD;
        for(int j = 0; j < NBITS_DWORD; ++j, ++ix) {
          if(BitsNoCommit.isSet(ix))
            oldData[ix] = tmp[ix];
          }
        }
      }
    BitsNoCommit.clearAll();
    }
  else
    memcpy(oldData, currData, sizeof(oldData));
}
//---------------------------------------------------------------------
log_status::errEvent ImplJobMemBase::chgOrd(ordStat cause)
{
  val_order val;

  switch(cause) {
    case oSuspended:  // richiamata dal termine programma o da pulsante
    case oEnded:      // richiamata da pulsante o fine naturale dell'ordine
    case oSave:       // richiamata dal salvataggio automatico
      // riempie la classe con tutti i dati
      do {
        gestPerif *plc = Par->getPerif(WM_REAL_PLC);
        if(!plc)
          return log_status::EvErrOrd;
        // prima legge i dati attuali
        val.status = oActive;
        Par->setEvent(log_status::chgOrder, &val);

        val.endDate = getFileTimeCurr();

        if(oSave != cause) {
//          setDirtyOrder();
          }

        } while(0);
      break;
    default:
      return log_status::EvErrOrd;
    }
  // se l'ordine era di terminazione i dati sono a posto
  // e si possono spostare i file nello storico
  val.status = cause;
  log_status::errEvent ret = Par->setEvent(log_status::chgOrder, &val);

  return ret;
}
//---------------------------------------------------------------------
void ImplJobMemBase::getBlockData(LPDWORD target, uint ndw, uint startAddr)
{
  if(startAddr > SIZE_OTHER_DATA)
    startAddr = SIZE_OTHER_DATA;
  ndw = min(ndw, SIZE_OTHER_DATA - startAddr);

  criticalLock cl(cSectSet);
  memcpy(target, currData + startAddr, ndw * sizeof(DWDATA));
}
//---------------------------------------------------------------------
void ImplJobMemBase::copyBlockData(LPDWORD source, uint ndw, uint startAddr)
{
  if(startAddr > SIZE_OTHER_DATA)
    startAddr = SIZE_OTHER_DATA;
  ndw = min(ndw, SIZE_OTHER_DATA - startAddr);

  criticalLock cl(cSectSet);
  memcpy(currData + startAddr, source, ndw * sizeof(DWDATA));
}
//---------------------------------------------------------------------
void ImplJobMemBase::fillBlockData(DWORD value, uint ndw, uint startAddr)
{
  if(startAddr > SIZE_OTHER_DATA)
    startAddr = SIZE_OTHER_DATA;
  ndw = min(ndw, SIZE_OTHER_DATA - startAddr);

  criticalLock cl(cSectSet);
  LPDWORD target = currData + startAddr;
  for(uint i = 0; i < ndw; ++i)
    target[i] = value;
}
//---------------------------------------------------------------------
prfData::tResultData ImplJobMemBase::get(prfData& target) const
{
  if(target.lAddr >= SIZE_OTHER_DATA)
    return prfData::invalidAddress;
  criticalLock cl(cSectSet);
  prfData::tResultData result = prfData::invalidType;
  prfData::tData type = (prfData::tData)(target.typeVar & ~prfData::tNegative);
  switch(type) {
    case prfData::tBitData:
      do {
        int nBit = LOWORD(target.U.dw);
        int offs = HIWORD(target.U.dw);
        DWDATA mask = 0xffffffff >> (32 - nBit);

        target.U.dw = currData[target.lAddr];
        target.U.dw >>= offs;
        target.U.dw &= mask;

        DWDATA old = oldData[target.lAddr];
        old  >>= offs;
        old &= mask;

        result = old == target.U.dw ? prfData::notModified : prfData::isChanged;
        if(target.typeVar & prfData::tNegative)
          target.U.dw = ~target.U.dw & mask;;
        } while(false);
      break;

    case prfData::tBData:
    case prfData::tBsData:
      target.U.dw = 0;
      target.U.b = (BDATA)currData[target.lAddr];
      result = target.U.b == (BDATA)(oldData[target.lAddr] & 0xff) ?
                prfData::notModified : prfData::isChanged;
      break;
    case prfData::tWData:
    case prfData::tWsData:
      target.U.dw = 0;
      target.U.w = (WDATA)currData[target.lAddr];
      result = target.U.w == (WDATA)(oldData[target.lAddr] & 0xffff) ?
                prfData::notModified : prfData::isChanged;
      break;

    // il float viene memorizzato usando i suoi bit senza conversione
    case prfData::tFRData:
    case prfData::tDWData:
    case prfData::tDWsData:
      target.U.dw = currData[target.lAddr];
      result = target.U.dw == oldData[target.lAddr] ?
                prfData::notModified : prfData::isChanged;
      break;

    // anche i valori a 64 bit vengono memorizzato usando i loro bit senza conversione
    case prfData::tRData:   // 64 bit float
    case prfData::ti64Data: // 64 bit
    case prfData::tDateHour:// 64 bit per data + ora
    case prfData::tDate:    // 64 bit per data
    case prfData::tHour:    // 64 bit per ora
      do {
        const __int64* buff = reinterpret_cast<const __int64*>(currData + target.lAddr);
        target.U.li.QuadPart = *buff;

        buff = reinterpret_cast<const __int64*>(oldData + target.lAddr);
        result = target.U.li.QuadPart == *buff ?
               prfData::notModified : prfData::isChanged;
        } while(false);
      break;

    case prfData::tStrData:
      do {
        int len = target.U.str.len  + target.lAddr * sizeof(currData[0]);
        if(len > SIZE_OTHER_DATA * sizeof(currData[0]))
          break;

        const DWDATA* curr = currData + target.lAddr;
        memcpy(target.U.str.buff, curr, target.U.str.len);
        const DWDATA* old = oldData + target.lAddr;

        int diff = memcmp(curr, old, target.U.str.len);
        result = diff ? prfData::isChanged : prfData::notModified;
        } while(false);
      break;

    default:
      break;
    }
  return StatusBits.U.b.needRefresh ? prfData::isChanged : result;
}
//---------------------------------------------------------------------
prfData::tResultData ImplJobMemBase::getset(prfData& data, bool noCommit)
{
  criticalLock cl(cSectSet);
  prfData::tResultData result = set(data, noCommit);
  get(data);
  return result;
}
//---------------------------------------------------------------------
prfData::tResultData ImplJobMemBase::set(const prfData& data, bool noCommit)
{
//  if(isReadOnly())
//    return prfData::okData;
  criticalLock cl(cSectSet);
  prfData::tResultData result = prfData::invalidType;

  prfData::tData type = (prfData::tData)(data.typeVar & ~prfData::tNegative);

  DWDATA lowTarget = currData[data.lAddr];
  DWDATA highTarget = data.lAddr <  SIZE_OTHER_DATA - 1 ? currData[data.lAddr + 1] : 0;

  switch(type) {
    case prfData::tBitData:
      do {
        WORD offAndBit = LOWORD(data.U.dw);
        DWORD val = HIWORD(data.U.dw);
        BYTE nBit = LOBYTE(offAndBit);
        BYTE offs = HIBYTE(offAndBit);

        DWDATA mask = 0xffffffff >> (32 - nBit);
        mask <<= offs;

        lowTarget = currData[data.lAddr];
        if(data.typeVar & prfData::tNegative)
          val = ~val;
        val <<= offs;
        val &= mask;
        mask = ~mask;
        lowTarget &= mask;
        lowTarget |= val;
        result = prfData::okData;
        } while(false);
      break;

    case prfData::tBData:
    case prfData::tBsData:
      lowTarget &= ~0xff;
      lowTarget |= data.U.b;
      result = prfData::okData;
      break;

    case prfData::tWData:
    case prfData::tWsData:
      lowTarget &= ~0xffff;
      lowTarget |= data.U.w;
      result = prfData::okData;
      break;

    case prfData::tFRData:
    case prfData::tDWData:
    case prfData::tDWsData:
      lowTarget = data.U.dw;
      result = prfData::okData;
      break;

    case prfData::tRData:   // 64 bit float
    case prfData::ti64Data: // 64 bit
    case prfData::tDateHour:// 64 bit per data + ora
    case prfData::tDate:    // 64 bit per data
    case prfData::tHour:    // 64 bit per ora
      lowTarget = data.U.li.u.LowPart;
      highTarget = data.U.li.u.HighPart;
      result = prfData::okData;
      break;

    case prfData::tStrData:
      do {
        uint len = data.U.str.len  + data.lAddr * sizeof(currData[0]);
        if(len > SIZE_OTHER_DATA * sizeof(currData[0]))
          break;

        DWDATA* curr = currData + data.lAddr;
        len = (data.U.str.len / sizeof(DWDATA)) + toBool(data.U.str.len % sizeof(DWDATA));
        for(uint i = 0; i < len; ++i)
          curr[i] = 0;
        memcpy(curr, data.U.str.buff, data.U.str.len);
        if(!StatusBits.U.b.saveLater)
          save();
        else
          StatusBits.U.b.needSaveJob = true;

        if(noCommit) {
          for(uint i = 0; i < len; ++i)
            BitsNoCommit.set(data.lAddr + i);
          }
        return prfData::okData;
        } while(false);
      break;

    default:
      break;
    }
  if(prfData::invalidType != result) {
    currData[data.lAddr] = lowTarget;
    if(data.lAddr < SIZE_OTHER_DATA - 1)
      currData[data.lAddr + 1] = highTarget;
    if(!StatusBits.U.b.saveLater)
      save();
    else
      StatusBits.U.b.needSaveJob = true;

    if(noCommit) {
      BitsNoCommit.set(data.lAddr);
      if(data.lAddr < SIZE_OTHER_DATA - 1) {
        switch(type) {
          case prfData::tRData:   // 64 bit float
          case prfData::ti64Data: // 64 bit
          case prfData::tDateHour:// 64 bit per data + ora
          case prfData::tDate:    // 64 bit per data
          case prfData::tHour:    // 64 bit per ora
            BitsNoCommit.set(data.lAddr + 1);
            break;
          }
        }
      }
    }

  return result;
}
//---------------------------------------------------------------------
void ImplJobMemBase::dontSaveNow()
{
  StatusBits.U.b.saveLater = true;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
struct infoValue
{
  uint prph;
  uint addr;
  uint type;
  uint nbit;
  uint offs;
  int norm;
  infoValue(uint prph=0, uint addr=0, uint type=0, uint nbit=0, uint offs=0, int norm = 0):
    prph(prph), addr(addr), type(type), nbit(nbit), offs(offs), norm(norm)  { if(prfData::tBitData == type) type = prfData::tDWData; }
};
//------------------------------------------------------------------
class retrieveValue
{
  public:
    retrieveValue(const infoValue& iv) : iV(iv) {}
    virtual ~retrieveValue() {}

    virtual fREALDATA getValue(mainClient* Owner);
  protected:
    infoValue iV;
    virtual void getData(prfData& data, mainClient* Owner);
    virtual fREALDATA performGetValue(prfData& data, REALDATA nrm) = 0;
};
//------------------------------------------------------------------
#define DEF_CLASS(CL) \
class get_##CL : public retrieveValue \
{\
  private:\
    typedef retrieveValue baseClass;\
  public:\
    get_##CL(const infoValue& is) : baseClass(is) {}\
  protected: \
    virtual fREALDATA performGetValue(prfData& data, REALDATA nrm);\
};
//------------------------------------------------------------------
class get_Const : public retrieveValue
{
  private:
    typedef retrieveValue baseClass;
  public:
    get_Const(const infoValue& is) : baseClass(is) {}
    virtual fREALDATA getValue(mainClient* Owner);
  protected:
    virtual fREALDATA performGetValue(prfData& data, REALDATA nrm) { return 0; }
};
//------------------------------------------------------------------
class get_Null : public retrieveValue
{
  private:
    typedef retrieveValue baseClass;
  public:
    get_Null(const infoValue& is) : baseClass(is) {}
    virtual fREALDATA getValue(mainClient* Owner) { return 0; }
  protected:
    virtual fREALDATA performGetValue(prfData& data, REALDATA nrm) { return 0; }
};
//------------------------------------------------------------------
DEF_CLASS(S_Byte)
DEF_CLASS(U_Byte)
DEF_CLASS(S_Short)
DEF_CLASS(U_Short)
DEF_CLASS(S_Int)
DEF_CLASS(U_Int)
DEF_CLASS(FR)
DEF_CLASS(R)
DEF_CLASS(I64)
//------------------------------------------------------------------
retrieveValue* makeGetValue(const infoValue& is)
{
  if(PRPH_4_CONST == is.prph)
    return new get_Const(is);

  switch(is.type) {
    case prfData::tBData:
      return new get_U_Byte(is);
    case prfData::tBsData:
      return new get_S_Byte(is);

    case prfData::tWData:
      return new get_U_Short(is);
    case prfData::tWsData:
      return new get_S_Short(is);

    case prfData::tDWData:
      return new get_U_Int(is);
    case prfData::tDWsData:
      return new get_S_Int(is);

    case prfData::tFRData:
      return new get_FR(is);

    case prfData::ti64Data: // 64 bit
      return new get_I64(is);

    case prfData::tRData:   // 64 bit float
      return new get_R(is);

    default:
      return new get_Null(is);
    }
  return 0;
}
//-------------------------------------------
void retrieveValue::getData(prfData& data, mainClient* Owner)
{
  genericPerif* gprph = Owner->getGenPerif(iV.prph);
  if(!gprph)
    return;
  data.lAddr = iV.addr;
  data.typeVar = iV.type;
  data.U.li.QuadPart = 0;
  gprph->get(data);
  if(iV.nbit) {
    data.U.li.QuadPart >>= iV.offs;
    data.U.li.QuadPart &= (1 << iV.nbit) - 1;
    }
}
//------------------------------------------------------------------
fREALDATA retrieveValue::getValue(mainClient* Owner)
{
  prfData data;
  getData(data, Owner);
  REALDATA nrm = getNorm(iV.norm);
  return performGetValue(data, nrm);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
fREALDATA get_Const::getValue(mainClient* Owner)
{
  if(prfData::tFRData == iV.type)
    return *(fREALDATA*)&iV.offs;
  return (fREALDATA)iV.offs;
}
//------------------------------------------------------------------
fREALDATA get_U_Byte::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.b, nrm);
}
//------------------------------------------------------------------
fREALDATA get_S_Byte::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.sb, nrm);
}
//------------------------------------------------------------------
fREALDATA get_U_Short::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.w, nrm);
}
//------------------------------------------------------------------
fREALDATA get_S_Short::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.sw, nrm);
}
//------------------------------------------------------------------
fREALDATA get_U_Int::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.dw, nrm);
}
//------------------------------------------------------------------
fREALDATA get_S_Int::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.sdw, nrm);
}
//------------------------------------------------------------------
fREALDATA get_FR::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.fw, nrm);
}
//------------------------------------------------------------------
fREALDATA get_R::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.rw, nrm);
}
//------------------------------------------------------------------
fREALDATA get_I64::performGetValue(prfData& data, REALDATA nrm)
{
  return (fREALDATA)getNormalized(data.U.li.QuadPart, nrm);
}
//---------------------------------------------------------------------
static
bool getConstValue(LPCTSTR p, DWDATA& value)
{
  bool isReal = false;
  LPCTSTR p2 = p;
  while(*p2 && *p2 != _T(',')) {
    if(_T('.') == *p2) {
      isReal = true;
      break;
      }
    ++p2;
    }
  if(isReal) {
    float v = (float)_tstof(p);
    value = *(LPDWORD)&v;
    }
  else
    value = _ttoi(p);
  return isReal;
}
//---------------------------------------------------------------------
manageValue::manageValue(mainClient* owner, uint mainCode) : Owner(owner)
{
  LPCTSTR p = getString(mainCode);
  p = findNextParamTrim(p);
  if(!p)
    return;

  uint dataCod = _ttoi(p);
  p = getString(dataCod);
  if(!p)
    return;

  uint numOfData = _ttoi(p);
  Set.setDim(numOfData);

  p = findNextParamTrim(p);
  if(!p)
    return;
  uint addrCode = _ttoi(p);

  for(uint i = 0; i < numOfData; ++i) {
    p = getString(addrCode + i);
    infoValue iv;
    if(p && PRPH_4_CONST == _ttoi(p)) {
      p = findNextParamTrim(p, 7);
      if(!p)
        continue;
      DWDATA v;
      bool isReal = getConstValue(p, v);
      iv.offs = v;
      iv.prph = PRPH_4_CONST;
      if(isReal)
        iv.type = prfData::tFRData;
      else
        iv.type = prfData::tDWData;
      }
    else if(findNextParamTrim(p, 7)) {
      int dummy;
      _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d"), &iv.prph, &iv.addr, &iv.type, &iv.norm, &dummy, &dummy, &iv.nbit, &iv.offs);
      }
    else if(p)
      _stscanf_s(p, _T("%d,%d,%d,%d"), &iv.prph, &iv.addr, &iv.type, &iv.norm);
    Set[i] =  makeGetValue(iv);
    }
}
//---------------------------------------------------------------------
manageValue::~manageValue() { flushPV(Set); }
//------------------------------------------------------------------
void manageValue::fillData(fREALDATA* buff, bool onQuit)
{
  uint nElem = Set.getElem();
  if(onQuit)
    for(uint i = 0; i < nElem; ++i)
      buff[i] = 0;
  else
    for(uint i = 0; i < nElem; ++i)
      buff[i] = Set[i]->getValue(Owner);
}
//---------------------------------------------------------------------
const ImplJobMemBase::save_trend* ImplJobMemBase::getSaveTrendByCode(uint code) const
{
  for(uint i = 0; i < numTrend; ++i)
    if(code == SaveTrend[i]->getCode())
      return SaveTrend[i];
  return 0;
}
//---------------------------------------------------------------------
ImplJobMemBase::save_trend::~save_trend()
{
  delete notifyByTime;
  delete []buff;
  delete manValue;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::init(ImplJobMemBase *owner)
{
  LPCTSTR p = Set ? Set->getString(mainCode) : getString(mainCode);
  if(!p) {
    mainCode = 0;
    return false;
    }
  uint addrTimer = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return false;

  uint dataCod = _ttoi(p);

  if(Set)
    p = Set->getString(dataCod);
  else
    p = getString(dataCod);
  if(!p)
    return false;

  uint numOfData = _ttoi(p);

  lenRec = numOfData * sizeof(fREALDATA) + sizeof(FILETIME);
  buff = new BYTE[lenRec];
  ZeroMemory(buff, lenRec);

  if(!Set && owner) {
    prfData data;
    data.lAddr = addrTimer;
    data.typeVar = prfData::tDWData;
    owner->get(data);
    notifyByTime = new saveByTime(needSave, cSect, data.U.dw);
    }
  if(!owner->isReadOnly()) {
    TCHAR path[_MAX_PATH];
    if(makeHaderName(path)) {
#if 1
// su richiesta ... nessun .bak
      P_File pf(path, P_CREAT);
      if(pf.P_open())
        makeHeader(pf);
#else
      do {
        P_File pf(path);
        pf.appendback();
        } while(false);
      P_File pf(path, P_CREAT);
      if(pf.P_open())
        makeHeader(pf);
      else {
        _tcscat_s(path, _T(".bak"));
        P_File pf(path);
        pf.unappendback();
        }
#endif
      }
    }
  return true;
}
//---------------------------------------------------------------------
void ImplJobMemBase::save_trend::checkTimerTrend(ImplJobMemBase *owner)
{
  if(!mainCode ||!owner)
    return;

  LPCTSTR p = getString(mainCode);
  uint addrTimer = _ttoi(p);
  prfData data;
  data.lAddr = addrTimer;
  data.typeVar = prfData::tDWData;
  data.U.dw = 0;
  owner->get(data);

  notifyByTime->setTime(data.U.dw);
}
//---------------------------------------------------------------------
uint ImplJobMemBase::save_trend::setTime(ImplJobMemBase *owner, uint time)
{
  if(!mainCode ||!owner)
    return false;

  LPCTSTR p = getString(mainCode);
  uint addrTimer = _ttoi(p);
  prfData data;
  data.lAddr = addrTimer;
  data.typeVar = prfData::tDWData;
  data.U.dw = time;
  owner->set(data, true);

  return notifyByTime->setTime(time);
}
//---------------------------------------------------------------------
uint ImplJobMemBase::save_trend::getTime() const
{
  return notifyByTime->getTime();
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::fillData(mainClient *parent, bool onQuit)
{
  if(!mainCode)
    return false;
  if(!manValue)
    manValue = new manageValue(parent, mainCode);

  FILETIME ft =  getFileTimeCurr();
  if(!MK_I64(ft))
    return false;
  *(reinterpret_cast<FILETIME*>(buff)) = ft;

  fREALDATA* pData = reinterpret_cast<fREALDATA*>(buff + sizeof(FILETIME));
  manValue->fillData(pData, onQuit);
  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::getEnableSave(mainClient *parent)
{
  if(!mainCode)
    return false;

  LPCTSTR p = getString(mainCode);
  p = findNextParam(p, 1);
  if(!p)
    return false;

  uint dataCod = _ttoi(p);

  p = getString(dataCod);
  if(!p)
    return false;
  p = findNextParam(p, 2);
  if(!p)
    return true;

  uint addrEnable = _ttoi(p);
  p = getString(addrEnable);
  if(!p)
    return true;

  int idPrf = 0;
  int addr = 0;
  int dataType = 0;
  int nBit = 0;
  int offset = 0;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &idPrf, &addr, &dataType, &nBit, &offset);
  genericPerif* gprf = parent->getGenPerif(idPrf);
  if(!gprf)
    return false;
  prfData data;
  data.lAddr = addr;
  data.typeVar = dataType;
  data.U.dw = 0;
  if(prfData::tBitData == dataType)
    data.U.dw = MAKELONG(nBit, offset);
  gprf->get(data);
  if(prfData::tBitData != dataType) {
    if(nBit) {
      DWDATA mask = 0xffffffff >> (32 - nBit);
      mask <<= offset;
      data.U.dw &= mask;
      }
    }
  return toBool(data.U.dw);
}
//---------------------------------------------------------------------
void getPrefixExt(LPTSTR prefix, LPTSTR ext, LPCTSTR source)
{
  int len = _tcslen(source);
  *prefix = 0;
  *ext = 0;
  int i;
  for(i = 0; i < len; ++i)
    if(_T(' ') < source[i])
      break;
  int j = 0;
  for(; i < len; ++i, ++j) {
    if(_T(',') == source[i])
      break;
    prefix[j] = source[i];
    }
  prefix[j] = 0;


  for(++i; i < len; ++i)
    if(_T(' ') < source[i])
      break;

  j = 0;
  if(_T('.') != source[i])
    ext[j++] = _T('.');

  for(; i < len; ++i, ++j) {
    if(_T(',') == source[i])
      break;
    ext[j] = source[i];
    }
  ext[j] = 0;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::makeName(LPTSTR target, const FILETIME& ft) const
{
  *target = 0;
  if(!mainCode)
    return false;

  LPCTSTR p = getString(mainCode);
  p = findNextParam(p, 2);
  if(!p)
    return false;

  uint type = _ttoi(p);
  if(type >= 100)
    type  -= 100;

  p =  findNextParam(p, 1);
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  TCHAR prefix[50];
  TCHAR ext[20];
  getPrefixExt(prefix, ext, p);

  switch(type) {
    case 0:
      wsprintf(target, _T("%s_%04d_%02d_%02d%s"),
              prefix, st.wYear, st.wMonth, st.wDay, ext);
      break;

    case 1:
      wsprintf(target, _T("%s_%04d_%02d%s"),
              prefix, st.wYear, st.wMonth, ext);
      break;

    case 2:
      wsprintf(target, _T("%s_%04d%s"), prefix, st.wYear, ext);
      break;
    }
  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::
makeNameByDate(LPTSTR target, uint type,  LPCTSTR p, int subIfHistory) const
{
  FILETIME ft = getFileTimeCurr();
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  TCHAR prefix[50];
  TCHAR ext[20];
  getPrefixExt(prefix, ext, p);

  switch(type) {
    case 0:
      wsprintf(target, _T("%s_%04d_%02d_%02d%s"),
              prefix, st.wYear, st.wMonth, st.wDay - subIfHistory, ext);
      break;

    case 1:
      wsprintf(target, _T("%s_%04d_%02d%s"),
              prefix, st.wYear, st.wMonth - subIfHistory, ext);
      break;

    case 2:
      wsprintf(target, _T("%s_%04d%s"), prefix, st.wYear - subIfHistory, ext);
      break;
    }
  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::makeNameByVar(mainClient *parent, LPTSTR target, LPCTSTR p) const
{
  TCHAR prefix[50];
  TCHAR ext[20];
  getPrefixExt(prefix, ext, p);
  p =  findNextParam(p, 2);
  _tcscpy_s(target, _MAX_PATH, prefix);
  if(!p) {
    _tcscat_s(target, _MAX_PATH, ext);
    return true;
    }

  uint code = _ttoi(p);
  p = getString(code);
  if(!p)
    return false;

  do {
    int idPrf = 0;
    int addr = 0;
    int dataType = 0;
    int len;
    _stscanf_s(p, _T("%d,%d,%d,%d"), &idPrf, &addr, &dataType, &len);
    genericPerif* gprf = parent->getGenPerif(idPrf);
    if(!gprf)
      return false;
    if(prfData::tStrData == dataType) {
      prfData data;
      data.lAddr = addr;
      data.setStr(len);
      gprf->get(data);
      _tcscat_s(target, _MAX_PATH, _T("_"));
      LPTSTR t = new TCHAR[len + 1];
      copyStr(t, data.U.str.buff, len + 1);
      _tcscat_s(target, _MAX_PATH, t);
      delete []t;
      }
    else {
      prfData data;
      data.lAddr = addr;
      data.typeVar = dataType;
      data.U.dw = 0;
      gprf->get(data);
      TCHAR t[50];
      if(len > 9)
        len = 9;
      _stprintf_s(t, SIZE_A(t), _T("_%0*d"), len, data.U.dw);
      _tcscat_s(target, _MAX_PATH, t);
      }
    p =  findNextParam(p, 4);
    } while(p);

  _tcscat_s(target, _MAX_PATH, ext);
  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::makeCurrentName(mainClient *parent, LPTSTR target) const
{
  bool useHistory;
  ImplJobMemBase::save_trend* pt = (ImplJobMemBase::save_trend*)this;
  return pt->makeName(parent, target, useHistory);
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::makeName(mainClient *parent, LPTSTR target, bool& useHistory, int subIfHistory) const
{
  *target = 0;
  if(!mainCode)
    return false;

  LPCTSTR p = getString(mainCode);
  p = findNextParam(p, 2);
  if(!p)
    return false;

  useHistory = false;
  uint type = _ttoi(p);
  if(type >= 100) {
    useHistory = true;
    type  -= 100;
    }
  p =  findNextParam(p, 1);
  switch(type) {
    case 0:
    case 1:
    case 2:
      return makeNameByDate(target, type, p, subIfHistory);

    case 3:
//      p =  findNextParam(p, 1);
      return makeNameByVar(parent, target, p);

    default:
      return false;
    }
//  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::
makeFindableNameByDate(LPTSTR target, uint type,  LPCTSTR p) const
{
  TCHAR prefix[50];
  TCHAR ext[20];
  getPrefixExt(prefix, ext, p);

  switch(type) {
    case 0:
      wsprintf(target, _T("%s_????_??_??%s"), prefix, ext);
      break;

    case 1:
      wsprintf(target, _T("%s_????_??%s"), prefix, ext);
      break;

    case 2:
      wsprintf(target, _T("%s_????%s"), prefix, ext);
      break;
    }
  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::makeFindableNameByVar(LPTSTR target, LPCTSTR p) const
{
  TCHAR prefix[50];
  TCHAR ext[20];
  getPrefixExt(prefix, ext, p);
  wsprintf(target, _T("%s_*%s"), prefix, ext);
  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::makeFindableName(LPTSTR target, bool& useHistory) const
{
  *target = 0;
  if(!mainCode)
    return false;

  LPCTSTR p = getString(mainCode);
  p = findNextParam(p, 2);
  if(!p)
    return false;

  useHistory = false;
  uint type = _ttoi(p);
  if(type >= 100) {
    useHistory = true;
    type  -= 100;
    }
  p =  findNextParam(p, 1);
  switch(type) {
    case 0:
    case 1:
    case 2:
      return makeFindableNameByDate(target, type, p);

    case 3:
//      p =  findNextParam(p, 1);
      return makeFindableNameByVar(target, p);

    default:
      return false;
    }

}
//---------------------------------------------------------------------
int ImplJobMemBase::save_trend::checkTimeRes(P_File& pfSource, DWORD& resolution)
{
  PVect<BYTE> vBuff;
  vBuff.setDim(lenRec);

  LPBYTE buff = (LPBYTE)&vBuff;
  const int nRead = 10;

  if(lenRec != pfSource.P_read(buff, lenRec))
    return -1;

  __int64 ftInit = *(__int64*)buff;
  __int64 ftLast = ftInit;
  __int64 ftDiff = HOUR_TO_I64;

  for(int i = 0; i < nRead; ++i) {
    if(lenRec != pfSource.P_read(buff, lenRec))
      return -1;
    __int64 t = *(__int64*)buff - ftLast;
    if(t > 0 && t < ftDiff)
      ftDiff = t;
    ftLast = *(__int64*)buff;
    }

  ftLast -= ftInit;
  // ora ftLast contiene la media delle differenze
  ftLast /= nRead;
  __int64 toll = ftDiff / 2;

  if(ftLast > ftDiff + toll || ftLast < ftDiff - toll)
    return 0;
  // se la differenza è dell'ordine dei secondi non c'è bisogno
  // di un campo aggiuntivo
  if(ftDiff >= SECOND_TO_I64) {
    resolution = 0;
    return 1;
    }

  // partiamo con il valore sui millisecondi
  ftDiff /= SECOND_TO_I64 / 1000;
  resolution = 1000;
  while(ftDiff >= 10) {
    resolution /= 10;
    ftDiff /= 10;
    }
  return 1;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::exportHeader(P_File& pf)   const
{
  if(!mainCode)
    return false;

  LPCTSTR p = Set ? Set->getString(mainCode) : getString(mainCode);

  p = findNextParam(p, 1);
  if(!p)
    return false;

  uint dataCod = _ttoi(p);

/*
  timeRes = 0;
  p = findNextParam(p, 4);
  if(p) {
    DWORD v = _ttoi(p);
    if(10 == v || 100 == v || 1000 == v)
      timeRes = v;
    }
*/
  if(Set)
    p = Set->getString(dataCod);
  else
    p = getString(dataCod);
  if(!p)
    return false;

  uint numOfData = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return true;

  uint idInitHeader = _ttoi(p);

  timeRes = 0;
  p = findNextParam(p, 2);
  if(p) {
    DWORD v = _ttoi(p);
    if(10 == v || 100 == v || 1000 == v)
      timeRes = v;
    }

  LPCTSTR tab = getString(ID_TAB);
  if(!tab)
    tab = _T("\t");
  TCHAR buff[500];
  wsprintf(buff, _T("Date%sHH:MM:SS%s"), tab, tab);
  pf.P_writeString(buff);

  if(timeRes) {
//    wsprintf(buff, _T("mSec%s"), tab);
    wsprintf(buff, _T("%d°/Sec%s"), timeRes, tab);
    pf.P_writeString(buff);
    }

  nDecForExport.reset();

  for(uint i = 0; i < numOfData; ++i) {
    if(Set)
      p = Set->getString(idInitHeader + i);
    else
      p = getString(idInitHeader + i);
    nDecForExport[i] = -1;
    if(!p)
      p = _T("???");
    else {
      p = findNextParam(p, 4);
      if(!p)
        p = _T("???");
      else {
        if(_istdigit((unsigned)*p)) {
          nDecForExport[i] = _ttoi(p);
          if(nDecForExport[i] > 5)
            nDecForExport[i] = 5;
          p = findNextParam(p, 1);
          }
        if(!p)
          p = _T("???");
        }
      }
    uint idTxt = _ttoi(p);
    if(idTxt && idTxt >= 800000) {
      LPCTSTR p2 = 0;
      if(Set)
        p2 = Set->getString(idTxt);
      else
        p2 = getString(idTxt);
      if(p2)
        p = p2;
      }
    smartPointerConstString sp = getStringByLangSimple(p);

    wsprintf(buff, _T("%s%s"), tab, &sp);
    pf.P_writeString(buff);
    }
  pf.P_writeString(_T("\r\n"));
  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::makeHaderName(LPTSTR target) const
{
  if(!mainCode)
    return false;

  LPCTSTR p = Set ? Set->getString(mainCode) : getString(mainCode);

  p = findNextParam(p, 3);
  if(!p)
    return false;

  TCHAR prefix[50];
  TCHAR ext[20];
  getPrefixExt(prefix, ext, p);

  wsprintf(target, _T("%s%s.txt"), prefix, ext);
  makePath(target, dExRep, false, _T(""));
  return true;
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::makeHeader(P_File& pf)   const
{
  if(!mainCode)
    return false;

  LPCTSTR p = Set ? Set->getString(mainCode) : getString(mainCode);

  p = findNextParam(p, 1);
  if(!p)
    return false;

  uint dataCod = _ttoi(p);

  if(Set)
    p = Set->getString(dataCod);
  else
    p = getString(dataCod);
  if(!p)
    return false;

  uint numOfData = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return true;

  uint idInitHeader = _ttoi(p);

  for(uint i = 0; i < numOfData; ++i) {
    if(Set)
      p = Set->getString(idInitHeader + i);
    else
      p = getString(idInitHeader + i);
    if(!p)
      continue;
    p = findNextParam(p, 4);
    if(!p)
      continue;
    if(_istdigit((unsigned)*p))
      p = findNextParam(p, 1);
    if(!p)
      continue;

    uint idTxt = _ttoi(p);
    if(idTxt && idTxt >= 800000) {
      LPCTSTR p2 = 0;
      if(Set)
        p2 = Set->getString(idTxt);
      else
        p2 = getString(idTxt);
      if(p2)
        p = p2;
      }
    smartPointerConstString sp = getStringByLangSimple(p);
    TCHAR t[512];
    wsprintf(t, _T("%d,%s\r\n"), i + 2, &sp);
    pf.P_writeString(t);
    }
  return true;
}
//---------------------------------------------------------------------
extern void replaceSep(LPTSTR buff, LPCTSTR sep);
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::exportData(P_File& pf, LPBYTE data) const
{
  if(!mainCode)
    return false;

  LPCTSTR tab = getString(ID_TAB);
  if(!tab)
    tab = _T("\t");
  LPCTSTR sep = getString(ID_SEP_DEC);
  if(!sep)
    sep = _T(",");

  TCHAR buff[100];

  FILETIME* ft = reinterpret_cast<FILETIME*>(data);
  set_format_data(buff, SIZE_A(buff), *ft, whichData(), tab);
  _tcscat_s(buff, SIZE_A(buff), tab);
  pf.P_writeString(buff);

  if(timeRes) {
    __int64 t = MK_I64(*ft);
    t %= SECOND_TO_I64;
    t /= SECOND_TO_I64 / timeRes;
    switch(timeRes) {
      case 10:
        wsprintf(buff, _T("%s%01d"), tab, (int)t);
        break;
      case 100:
        wsprintf(buff, _T("%s%02d"), tab, (int)t);
        break;
      case 1000:
      default:
        wsprintf(buff, _T("%s%03d"), tab, (int)t);
        break;
      }
    pf.P_writeString(buff);
    }

  uint numOfData = (lenRec - sizeof(FILETIME)) / sizeof(fREALDATA);
  fREALDATA* pData = reinterpret_cast<fREALDATA*>(data + sizeof(FILETIME));

  for(uint i = 0; i < numOfData; ++i) {
    fREALDATA t = pData[i];
    if(t < 0)
      t += -0.000005f;
    else
      t += 0.000005f;
    if(nDecForExport[i] >= 0) {
      _stprintf_s(buff, SIZE_A(buff), _T("%s%0.*f"), tab, nDecForExport[i], t);
      Z_END(buff);
      }
    else {
      _stprintf_s(buff, SIZE_A(buff), _T("%s%0.5f"), tab, t);
      Z_END(buff);
      zeroTrim(buff);
      }
    replaceSep(buff, sep);
    pf.P_writeString(buff);
    }
  pf.P_writeString(_T("\r\n"));
  return true;
}
//---------------------------------------------------------------------
int ImplJobMemBase::save_trend::getType() const
{
  if(!mainCode)
    return -1;
  LPCTSTR p = Set ? Set->getString(mainCode) : getString(mainCode);
  p = findNextParam(p, 2);
  if(!p)
    return -1;

  return _ttoi(p);
}
//---------------------------------------------------------------------
bool ImplJobMemBase::save_trend::save(P_File& pf)
{
  return lenRec == pf.P_write(buff, lenRec);
}
//---------------------------------------------------------------------
void ImplJobMemBase::save_trend::resetSave()
{
  criticalLock cl(cSect);
  needSave = false;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
saveByTime::saveByTime(bool& needSave, criticalSect& cSect, uint time) :
    needSave(needSave), cSect(cSect), hEvent(0), hThread(0), Quit(false),
    Time(time  >= MIN_TIME_TO_SAVE ? time : time > 0 ? MIN_TIME_TO_SAVE : DEF_TIME_TO_SAVE)
{
  hEvent = CreateEvent( 0, FALSE, FALSE, 0);
  if(hEvent) {
    DWORD idThread;
    hThread = CreateThread(0, 0, saveByTimeProc, this, 0, &idThread);
    if(!hThread) {
      CloseHandle(hEvent);
      hEvent = 0;
      }
    }
}
//-------------------------------------------------------------------------
saveByTime::~saveByTime()
{
  Quit = true;
  reset();
  if(hThread) {
    WaitForSingleObject(hThread, 30000);
    CloseHandle(hThread);
    }
  if(hEvent)
    CloseHandle(hEvent);
}
//----------------------------------------------------------------------------
void saveByTime::reset()
{
  if(hEvent)
    SetEvent(hEvent);
}
//----------------------------------------------------------------------------
uint saveByTime::setTime(uint time)
{
  if(time != Time) {
    if(!time)
      Time = DEF_TIME_TO_SAVE;
    else if(time < MIN_TIME_TO_SAVE)
      Time = MIN_TIME_TO_SAVE;
    else
      Time = time;
    reset();
    }
  return Time;
}
//----------------------------------------------------------------------------
DWORD WINAPI saveByTimeProc(LPVOID cl)
{
  saveByTime *pGest = reinterpret_cast<saveByTime*>(cl);

  DWORD result = EXIT_SUCCESS;
  while(!pGest->Quit) {
    DWORD result = WaitForSingleObject(pGest->hEvent, pGest->getTime());

    if(WAIT_TIMEOUT == result)
      pGest->setNeedSave();

    // segnalazione di reset, potrebbe essere cambiato il time o fine programma
    else if(WAIT_OBJECT_0 == result)
      continue;

    // uscita anomala
    else {
      result = EXIT_FAILURE;
      break;
      }
    }
  return result;
}
//---------------------------------------------------------------------
clearOldFile::~clearOldFile()
{
  SetEvent(hEvent);
  while(true) {
    if(!idThread)
      break;
    Sleep(100);
    }
  if(hEvent)
    CloseHandle(hEvent);
}
//---------------------------------------------------------------------
bool clearOldFile::init(PWin* parent)
{
  HANDLE hThread = CreateThread(0, 0, clearOldFileProc, this, 0, &idThread);
  if(!hThread)
    return false;
  CloseHandle(hThread);

  SYSTEMTIME st;
  GetLocalTime(&st);
  canCheck = st.wHour >= 12;
  return true;
}
//---------------------------------------------------------------------
#define PERC_THRESHOLD 90
//---------------------------------------------------------------------
void clearOldFile::check(PWin* parent)
{
  if(!canCheck || onExec)
    return;
  onExec = true;
  canCheck = false;
  orderedOldFile set(true);
  __int64 dim = getAllFile(set);

  while(set.setFirst()) {
    uint nFile = set.getNumElem();
    if(nFile <= minFile)
      break;

    dim /= MegaByte;
    dim /= (1024 * 1024) / 100;
    if(dim > PERC_THRESHOLD) {
      if(needShow)
        msgBoxByLangGlob(parent, ID_MSG_FILESIZE_LIMIT, ID_TITLE_FILESIZE_LIMIT, MB_ICONINFORMATION);
      needShow = false;
      }
    else
      needShow = true;
    break;
    }
  onExec = false;
}
//---------------------------------------------------------------------
__int64 clearOldFile::fillSet(orderedOldFile& set, LPTSTR path)
{
  __int64 tot = 0;
  WIN32_FIND_DATA FindFileData;
  HANDLE hf = FindFirstFile(path, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    int len = _tcslen(path);
    LPTSTR p = 0;
    for(int i = len - 1; i > 0; --i) {
      if(_T('\\') == path[i]) {
        path[i + 1] = 0;
        p = path + i + 1;
        break;
        }
      }
    do {
      if(!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)) {
        LARGE_INTEGER li = { FindFileData.nFileSizeLow, FindFileData.nFileSizeHigh };
        // non cancella i file abbastanza piccoli perché potrebbero essere di
        // configurazione
        if(li.QuadPart < (32 * 1024))
          continue;
        *p = 0;
        _tcscpy_s(p, _MAX_PATH - (p - path), FindFileData.cFileName);
        tot += li.QuadPart;
        oldFile* of = new oldFile(FindFileData.ftLastWriteTime, path, li.QuadPart);
        set.Add(of);
        }
      } while(FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }
  return tot;
}
//---------------------------------------------------------------------
__int64 clearOldFile::getAllFile(orderedOldFile& set)
{
  TCHAR path[_MAX_PATH] = _T("\0");
  makePath(path, dExRep, true, _T(""));
  appendPath(path, _T("*.*"));
  __int64 dim = fillSet(set, path);

  path[0] = 0;
  makePath(path, dExRep, false, _T(""));
  appendPath(path, _T("*.*"));
  dim += fillSet(set, path);
  if(!set.setFirst())
    return 0;
  return dim;
}
//---------------------------------------------------------------------
void clearOldFile::clearFile()
{
  orderedOldFile set(true);
  __int64 dim = getAllFile(set);

  if(!set.setFirst())
    return;
  __int64 mDim = MegaByte;
  mDim *= (1024 * 1024) / 2;
  uint nFile = set.getNumElem();
  while(dim >= mDim && nFile > minFile) {
    oldFile* curr = set.getCurrData();
    DeleteFile(curr->getPath());
    dim -= curr->getDim();
    if(!set.setNext())
      break;
    --nFile;
    }
}
//---------------------------------------------------------------------
DWORD clearOldFile::getNextTime(bool& before)
{
  SYSTEMTIME st;
  GetLocalTime(&st);

  before = st.wHour < 12;
  FILETIME ft;
  SystemTimeToFileTime(&st, &ft);
  FILETIME ft2 = ft;
  if(before) {
    st.wHour = 12;
    }
  else {
    MK_I64(ft) += HOUR_TO_I64 * 24;
    FileTimeToSystemTime(&ft, &st);
    st.wHour = 0;
    }
  st.wMinute = 0;
  st.wSecond = 0;
  SystemTimeToFileTime(&st, &ft);
  ft -= ft2;
  DWORD msec = DWORD(MK_I64(ft) / SECOND_TO_I64) * 1000;
  return msec;
}
//---------------------------------------------------------------------
DWORD WINAPI clearOldFileProc(LPVOID cl)
{
  clearOldFile* cof = (clearOldFile*)cl;
//  cof->clearFile();
  bool before = false;
  DWORD timeout = cof->getNextTime(before);
  while(true) {
    DWORD result = WaitForSingleObject(cof->hEvent, timeout);
    if(WAIT_TIMEOUT == result) {
      if(before)
        cof->canCheck = true;
      else
        cof->clearFile();
      }
    else
      break;
    timeout = cof->getNextTime(before);
    }
  cof->idThread = 0;
  return 0;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include "gestData.h"
//---------------------------------------------------------------------
gestJobData::gestJobData(mainClient *parent) :  genericPerif(PRF_MEMORY),
  Data(new ImplJobData(parent))
{
  typeAddress = prfData::tDWData;
  Data->setReadOnly(isReadOnly());
}
//---------------------------------------------------------------------
gestJobData::~gestJobData()
{
  delete Data;
}
//---------------------------------------------------------------------
void gestJobData::notify()
{
  Data->notify();
}
//---------------------------------------------------------------------
void gestJobData::setDirty()
{
  Data->setDirty();
}
//---------------------------------------------------------------------
void gestJobData::notifySaveLog()
{
//  Data->notifySaveLog();
}
//---------------------------------------------------------------------
void gestJobData::save()
{
  Data->save();
}
//---------------------------------------------------------------------
void gestJobData::load()
{
  Data->load();
}
//---------------------------------------------------------------------
void gestJobData::commit()
{
  Data->commit();
}
//---------------------------------------------------------------------
void gestJobData::init()
{
  Data->init();
}
//---------------------------------------------------------------------
void gestJobData::end()
{
  Data->end();
}
//---------------------------------------------------------------------
void gestJobData::checkTimerTrend()
{
  Data->checkTimerTrend();
}
//---------------------------------------------------------------------
prfData::tResultData gestJobData::getset(prfData& data, bool noCommit)
{
  return Data->getset(data, noCommit);
}
//---------------------------------------------------------------------
prfData::tResultData gestJobData::get(prfData& target) const
{
  return Data->get(target);
}
//---------------------------------------------------------------------
prfData::tResultData gestJobData::set(const prfData& data, bool noCommit)
{
  return Data->set(data, noCommit);
}
//---------------------------------------------------------------------
/*
IMaint *gestJobData::getMaint()
{
  return Data->getMaint();
}
*/
//---------------------------------------------------------------------
void gestJobData::dontSaveNow()
{
  Data->dontSaveNow();

}
//---------------------------------------------------------------------
void gestJobData::saveNow()
{
  Data->save();
}
//---------------------------------------------------------------------
void gestJobData::getBlockData(LPDWORD target, uint ndw, uint startAddr)
{
  Data->getBlockData(target, ndw, startAddr);
}
//---------------------------------------------------------------------
void gestJobData::copyBlockData(LPDWORD source, uint ndw, uint startAddr)
{
  Data->copyBlockData(source, ndw, startAddr);
}
//---------------------------------------------------------------------
void gestJobData::fillBlockData(DWORD value, uint ndw, uint startAddr)
{
  Data->fillBlockData(value, ndw, startAddr);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void gestJobData::logAlarm(class log_status &log, int which, bool force)
{
  Data->logAlarm(log, which, force);
}
//---------------------------------------------------------------------
int gestJobData::getNBitAlarm() const
{
  return Data->getNBitAlarm();
}
//---------------------------------------------------------------------
int gestJobData::getAddrAlarm() const
{
  return Data->getAddrAlarm();
}
//---------------------------------------------------------------------
bool gestJobData::isReady() const
{
  return Data->started();
}
//---------------------------------------------------------------------
bool okOnEmerg(mainClient *par)
{
  perifPLC *plc = dynamic_cast<perifPLC*>(par->getPerif(WM_REAL_PLC));
  if(plc) {
    if(!plc->isOnEmerg()) {
      HWND old = ::GetFocus();
      svMessageBox(par, getStringOrId(ID_MSG_ON_EMERG), getStringOrId(ID_TITLE_ON_EMERG),
        MB_ICONINFORMATION);
      ::SetFocus(old);
      return false;
      }
    }
  return true;
}
//---------------------------------------------------------------------
