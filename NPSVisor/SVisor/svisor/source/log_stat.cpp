//----------- log_stat.cpp -------------------------------
#include "precHeader.h"
#include <stdlib.h>

#include "p_base.h"

#include "log_stat.h"

#include "log_alrm.h"
#include "p_date.h"

#include "currAlrm.h"

// gestione directory di default
#include "def_dir.h"

#include "1.h"
#include "p_name.h"
//--------------------------------------------------------------
//--------------------------------------------------------------
// cast generico per salvataggio oggetto (non puntatore!!)
#define WRT(dat) f.P_write(&dat, sizeof(dat))
// cast generico per lettura oggetto (non puntatore!!)
#define RD(dat) f.P_read(&dat, sizeof(dat))
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
UDimF val_gen::write(P_File &f)
{
  UDimF len = WRT(id);
  if(!len)
    return 0;
  return len + WRT(val);
}
//--------------------------------------------------------------
UDimF val_gen::read(P_File &f)
{
  UDimF len = RD(id);
  if(!len)
    return 0;
  return len + RD(val);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
stat_gen::stat_gen()
{
  alarm[0] = new status_time;
  for(int i = 1; i < MAX_GRP_ALARM; ++i)
    alarm[i] = new status_alarm;
}
//--------------------------------------------------------------
stat_gen::~stat_gen()
{
  for(int i = 0; i < MAX_GRP_ALARM; ++i)
    delete alarm[i];
}
//--------------------------------------------------------------
UDimF stat_gen::write(P_File &f)
{
    FILETIME ftm = getFileTimeCurr();
    UDimF nByte = WRT(ftm);
    for(int i = 0; i < MAX_GRP_ALARM; ++i) {
      int n = 0;
      while(alarm[i]->stopTime())
        ++n;
      alarm[i]->getTotal(&ftm);
      nByte += WRT(ftm);
      while(n--)
        alarm[i]->startTime();
    }
    return nByte;
}
//--------------------------------------------------------------
UDimF stat_gen::read(P_File &f)
{
    FILETIME ftm;
    UDimF n = RD(ftm); // scarica time
    for(int i = 0; i < MAX_GRP_ALARM; ++i) {
      FILETIME ftm;
      (reinterpret_cast<LARGE_INTEGER*>(&ftm))->QuadPart = 0;
      n += RD(ftm);
      alarm[i]->init_alarm(ftm);
    }
    return n;
}
//--------------------------------------------------------------
bool stat_gen::getTime(FILETIME *ft, uint id) const
{
  if(id < MAX_GRP_ALARM) {
    alarm[id]->getTotal(ft);
    return true;
  }
  return false;
}
//--------------------------------------------------------------
bool stat_gen::getTime(SYSTEMTIME *st, uint id) const
{
  if(id < MAX_GRP_ALARM) {
    alarm[id]->getTotal(st);
    return true;
  }
  return false;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
bool stat_gen::exportData(P_File &f)
{
    FILETIME ftm;
    TCHAR buff[100];
    LPCTSTR tab = getStringOrId(ID_TAB);
    for(int i = 0; i < MAX_GRP_ALARM; ++i) {
      alarm[i]->getTotal(&ftm);
      set_format_time(buff, SIZE_A(buff), ftm);
      if(!f.P_writeString(buff))
        return false;
      if(!f.P_writeString(tab))
        return false;
    }
    return true;
}
//--------------------------------------------------------------
bool stat_gen::exportHeader(P_File &f)
{
    TCHAR buff[500] = _T("\0");
    LPCTSTR tab = getStringOrId(ID_TAB);
    for(int i = 0; i < MAX_GRP_ALARM; ++i) {
      smartPointerConstString p = getStringOrIdByLangGlob(ID_EXP_ALARM1 + i);
      _tcscat_s(buff, SIZE_A(buff), p);
      _tcscat_s(buff, SIZE_A(buff), tab);
    }
    return toBool(f.P_writeString(buff));
}
//--------------------------------------------------------------
static void writeCol(P_File &t, LPCTSTR tab, uint id)
{
  smartPointerConstString p = getStringOrIdByLangGlob(id);

  t.P_writeString(p);
  t.P_writeString(tab);
}
//--------------------------------------------------------------
#ifndef D_DESCR_ALARM
  #define D_DESCR_ALARM 200
#endif
//--------------------------------------------------------------
bool rep_alarm::exportData(LPCTSTR file, P_File &t)
{
  P_File f(file, P_READ_ONLY);
  if(!f.P_open())
    return false;
  if(!f.get_len())
    return false;
  LPCTSTR tab = getStringOrId(ID_TAB);

  writeCol(t, tab, ID_REP_CAUSA);
  writeCol(t, tab, ID_COD_REP_ALARM);
  writeCol(t, tab, ID_REP_ALARM);
  writeCol(t, tab, ID_REP_DATA);
  writeCol(t, tab, ID_REP_ORA);

  t.P_writeString(_T("\r\n")); // termina la riga
  FILETIME ft;
  long al;
  TCHAR buff[200];
  smartPointerConstString init = getStringOrIdByLangGlob(ID_DATA_TYPE_INIT);
  smartPointerConstString end = getStringOrIdByLangGlob(ID_DATA_TYPE_END);
  smartPointerConstString ack = getStringOrIdByLangGlob(ID_DATA_TYPE_ACK);

  for(;;) {
    if(!f.P_read(&al, sizeof(al)))
      break;
    if(!f.P_read(&ft, sizeof(ft)))
      break;
    if(al > 0) {
      if(isAckAlarm(al, true))
        t.P_writeString(ack);
      else
        t.P_writeString(init);
      }
    else {
      t.P_writeString(end);
      al = -al;
      }
    t.P_writeString(tab);

    clearUnusedBitAlarm(al);

    infoAssocAlarm info;
    if(!f.P_read(&info.dw, sizeof(info.dw)))
      break;
    if(!f.P_read(&info.type, sizeof(info.type)))
      break;
    fullInfoAlarm fia;
    if(getTextReport(al, fia, info)) {
      _itot_s(fia.alarm, buff, SIZE_A(buff), 10);
      t.P_writeString(buff);
      t.P_writeString(tab);

      TCHAR descr[D_DESCR_ALARM + 2];
      makeAlarmDescr(descr, D_DESCR_ALARM, fia);
      t.P_writeString(descr);
      }
    t.P_writeString(tab);

    set_format_data(buff, SIZE_A(buff), ft, whichData(), tab);
    t.P_writeString(buff);

    if(!t.P_writeString(_T("\r\n"))) // termina la riga
      return false;
    }
  return true;
}
//--------------------------------------------------------------
// report dei singoli allarmi (attivazione/disattivazione/acknoledge)
void rep_alarm::setTime(int n, bool start, const infoAssocAlarm& iaa)
{
  LPCTSTR p = getString(ID_READ_ONLY_PROGRAM);
  bool readOnly = toBool(p && _ttoi(p));
  if(readOnly)
    return;
  TCHAR file[_MAX_PATH] = NAME_FILE_REP_ALARM2;
  makeYearAndMonth(file + SIZE_A(NAME_FILE_REP_ALARM2) - 1);
  file[MAXLEN_NAME_FILE_DAT] = 0;
  _tcscat_s(file, SIZE_A(file), getExt(dRepAlarm));

  getPathRepAlarm(file);

  checkForHistoryMonth(file);

  P_File f(file, P_RW);
  FILETIME ftm = getFileTimeCurr();
  if(!MK_I64(ftm))
    return;
  if(f.P_open()) {
    f.P_seek(0L, SEEK_END_);
    int v = start ? n : -n;
    WRT(v);
    WRT(ftm);
    WRT(iaa.dw);
    WRT(iaa.type);
  }
}
//-------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <pshpack1.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(push, 1)

#else
  #error Occorre definire l'allineamento a byte per le strutture seguenti
#endif
//-------------------------------------------------------------
#define OLD_MUL_STEP_PRF_ALARM  4000
#define MAX_PERIF_ALARM 8
//-------------------------------------------------------------
struct dataOld
{
  short id;
  FILETIME ft;
};
//-------------------------------------------------------------
struct dataNew
{
  int id;
  FILETIME ft;
  DWORD dw;
  uint type;
  dataNew(const dataOld& old) : id(old.id), ft(old.ft), dw(0), type(0)
  {
    --id;
    uint prph = id / OLD_MUL_STEP_PRF_ALARM;
    if(prph >= MAX_PERIF_ALARM)
      prph = 0;
    uint alarm = id % OLD_MUL_STEP_PRF_ALARM + 1;
    id = makeIdAlarmByPrph(alarm, prph + WM_PLC);
  }
};
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <poppack.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(pop)

#endif
//---------------------------------------------------------------------
//--------------------------------------------------------------
static
void performConvert(LPCTSTR target, LPCTSTR source)
{
  do {
    P_File pT(target, P_CREAT);
    if(!pT.P_open())
      return;
    P_File pS(source, P_READ_ONLY);
    if(!pS.P_open())
      return;
    dataOld old;
    for(;;) {
      if(!pS.P_read(&old, sizeof(old)))
        break;
      dataNew n(old);
      if(!pT.P_write(&n, sizeof(n)))
        break;
      }
    FILETIME ft;
    if(pS.getData(ft))
      pT.setData(ft);
    } while(false);
  TCHAR path[_MAX_PATH] = _T("oldRep");
  CreateDirectory(path, 0);
  appendPath(path, source);
  MoveFile(source, path);
}
//--------------------------------------------------------------
static
void perform_check_old_rep(LPTSTR path)
{
  TCHAR oldPath[_MAX_PATH];
  GetCurrentDirectory(SIZE_A(oldPath), oldPath);
  if(!SetCurrentDirectory(path))
    return;
  TCHAR file[_MAX_PATH] = NAME_FILE_REP_ALARM;
  _tcscat_s(file, _T("*"));
  _tcscat_s(file, getExt(dRepAlarm));
  WIN32_FIND_DATA fff;
  HANDLE hf = FindFirstFile(file, &fff);
  if(INVALID_HANDLE_VALUE != hf) {
    uint lenName = _tcslen(NAME_FILE_REP_ALARM);
    do {
      if(FILE_ATTRIBUTE_DIRECTORY != fff.dwFileAttributes) {
        TCHAR file2[_MAX_PATH] = NAME_FILE_REP_ALARM2;
        _tcscat_s(file2, fff.cFileName + lenName);
        if(!P_File::P_exist(file2))
          performConvert(file2, fff.cFileName);
        }
      } while(FindNextFile(hf, &fff));
    FindClose(hf);
    }
  SetCurrentDirectory(oldPath);
}
//--------------------------------------------------------------
void check_old_rep()
{
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, getDir(dRepAlarm));
  perform_check_old_rep(path);
  _tcscpy_s(path, getDir(dHistory));
  appendPath(path, getDir(dRepAlarm));
  perform_check_old_rep(path);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
stat_alarm::stat_alarm()  { }
//--------------------------------------------------------------
int stat_alarm::action(bool load)
{
  LPCTSTR p = getString(ID_READ_ONLY_PROGRAM);
  bool readOnly = toBool(p && _ttoi(p));
  TCHAR file[_MAX_PATH] = NAME_FILE_GLOB_ALARM;
  makeYear(file + 4);
  file[MAXLEN_NAME_FILE_DAT] = 0;
  _tcscat_s(file, SIZE_A(file), getExt(dAlarm));

  getPathAlarm(file);

  P_File f(file, P_RW);
  if(f.P_open()) {
    if(!readOnly && f.get_len() < SZ_BASE_STAT * MAX_DAY) {
      BYTE tmp[SZ_BASE_STAT];
      ZeroMemory(tmp, sizeof(tmp));
      for(int i = 0; i < MAX_DAY; ++i)
        f.P_write(tmp, SZ_BASE_STAT);
    }
    f.P_seek(getDay() * SZ_BASE_STAT);
    if(load) {
      if(!readOnly)
        checkForHistory(file);
      read(f);
    }
    else if(!readOnly)
      write(f);
    return 1;
  }
  return 0;
}
//--------------------------------------------------------------
bool stat_alarm::exportData(LPCTSTR File, P_File &t)
{
  P_File f(File, P_READ_ONLY);
  if(!f.P_open())
    return false;
  if(f.get_len() < SZ_BASE_STAT * MAX_DAY)
    return false;
#if 1
  bool needDelete;
  LPCTSTR p = getStringOrIdByLangGlob(ID_DATA_HEADER, needDelete);
  smartPointerConstString s1(p, needDelete);
#else
  LPCTSTR p = getStringOrId(ID_DATA_HEADER);
#endif
  t.P_writeString(p);
  p = getStringOrId(ID_TAB);
  t.P_writeString(p);
  stat_gen::exportHeader(t);
  t.P_writeString(_T("\r\n")); // termina la riga
  TCHAR buff[14] = _T("01/01/");
  extrYear(File, buff + 6, SIZE_A(buff) - 6);
  long day = d2l(buff);
  for(int i = 0; i < MAX_DAY; ++i, ++day) {
    read(f);
    exportDay(day, t, whichData());
    if(!exportData(t))      // esporta statistiche allarmi
      return false;
    t.P_writeString(_T("\r\n")); // termina la riga
  }
  return true;
}
//--------------------------------------------------------------
void stat_alarm::fillTotalTime(LPCTSTR File, FILETIME *ft, int fromDay, int toDay)
{
  for(int i = 0; i < MAX_GRP_ALARM; ++i)
    (reinterpret_cast<LARGE_INTEGER*>(ft + i))->QuadPart = 0;
  P_File f(File, P_READ_ONLY);
  if(f.P_open()) {
    if(f.get_len() < SZ_BASE_STAT * MAX_DAY)
      return;
    f.P_seek(fromDay * SZ_BASE_STAT);
    if(toDay >= MAX_DAY)
      toDay = MAX_DAY - 1;
    for(int i = fromDay; i <= toDay; ++i) {
      read(f);
      for(int j = 0; j < MAX_GRP_ALARM; ++j) {
        FILETIME curr;
        if(getTime(&curr, j))
          (reinterpret_cast<LARGE_INTEGER*>(ft + j))->QuadPart +=
            (reinterpret_cast<LARGE_INTEGER*>(&curr))->QuadPart;
      }
    }
  }
}
//--------------------------------------------------------------
//--------------------------------------------------------------
extern stat_gen *allocOper();
extern stat_gen *allocTurn();
extern stat_gen *allocOrder();
//     enum eRepType { NO_REP, ONLY_ALARMS, ALSO_EVENTS };

log_status::log_status(eRepType rep_all) : repAll(rep_all), readOnly(false)
{
  LPCTSTR p = getString(ID_READ_ONLY_PROGRAM);
  readOnly = toBool(p && _ttoi(p));
  oper = allocOper();
  turno = allocTurn();
  ordine = allocOrder();

  global = new stat_alarm;
  allAlarm = new rep_alarm;
}
//--------------------------------------------------------------
log_status::~log_status()
{
  if(!readOnly)
    global->save();

  // vengono forzati da programma
//  ordine->save();
//  oper->save();
//  turno->save();

  delete oper;
  delete turno;
  delete ordine;
  delete global;
  delete allAlarm;
}
//--------------------------------------------------------------
const stat_gen* log_status::getObj(eType obj) const
{
  stat_gen *stat[] = { oper, turno, ordine, global };
  return stat[obj];
}
//--------------------------------------------------------------
stat_gen* log_status::getObj(eType obj)
{
  stat_gen *stat[] = { oper, turno, ordine, global };
  return stat[obj];
}
//--------------------------------------------------------------
#define N_OF_STAT 4
log_status::errEvent log_status::setEvent(event type, const val_gen* val)
{
// deve rispettare l'ordine per attivazione allarmi (rOPER, rTURN, ...)
  stat_gen *stat[N_OF_STAT] = { oper, turno, ordine, global };
  switch(type) {
    case onInit:
//      makeAllDir();
      global->init();
      //
      break;
    case chgOper: // cambio operatore
      return oper->setEvent(val);
    case chgTurno: // cambio turno
      return turno->setEvent(val);
    case chgOrder:
      return ordine->setEvent(val);
    case onPause: // la macchina è in stato di pausa
      if(val) {
        for(int i = 0; i < N_OF_STAT; ++i)
          if(val->id & (1 << i))
            stat[i]->stop_alarm(ID_TIME_RUN);
      }
      break;
    case onRun: // terminato lo stato di pausa, torna o inizia in run
      if(val) {
        for(int i = 0; i < N_OF_STAT; ++i)
          if(val->id & (1 << i))
            stat[i]->start_alarm(ID_TIME_RUN);
      }
      break;
    case onAlarm:  // nuovo allarme
    case endAlarm: // eliminato allarme
    case onAckAlarm:
      return chgAlarm(val, stat, type);
    case refreshStat:
      if(!readOnly)
        global->save();
#if 0
// è necessario salvare anche i dati di produzione correnti,
// quindi vanno spostate nella notifica di salvataggio della gestione dati
      turno->save();
      oper->save();
#endif
      break;
    case setRepAll:
      if(val)
        repAll = static_cast<eRepType>(val->id);
      break;
    default:
      return EvErrUnknow;
  }
  return EvNoErr;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
log_status::errEvent log_status::chgAlarm(const val_gen* val, stat_gen *stat[], log_status::event type)
{
  errEvent result = EvNoErr;
  if(val) {
    int which = val->val;
    long id = val->id + 1;

    if(log_status::onAckAlarm != type)
      getGestAlarm()->setTime(id, log_status::onAlarm == type);
    uint grp = 0;
    fullInfoAlarm fia;
    if(getTextAlarm(id, fia, true)) {
      if(log_status::onAckAlarm != type) {
        grp = fia.idGrp;
  //      grp = fia.idGrp - 1; // uno -> zero based ???
        if(grp < MAX_GRP_ALARM - ID_INIT_CATG) {
          grp += ID_INIT_CATG;
          for(int i = 0; i < N_OF_STAT; ++i) {
            if(which & (1 << i)) {
              if(log_status::onAlarm == type)
                stat[i]->start_alarm(grp);
              else if(log_status::endAlarm == type)
                stat[i]->stop_alarm(grp);
              }
            }
          }
        else
          result = EvErrAlarm1;
        }
      }
    else
      result = EvErrAlarm2;

    if(which & rALL_ALARM) {
      switch(repAll) {
        case ONLY_ALARMS:
          if(!grp)
            break;
        case ALL_EVENTS:
          switch(type) {
            case log_status::onAckAlarm:
              setAckAlarm(id);
              // fall through
            case log_status::onAlarm:
              allAlarm->startTime(id, fia.iaa);  // zero -> uno based
              break;
            case log_status::endAlarm:
              allAlarm->stopTime(id, fia.iaa);  // zero -> uno based
              break;
            }
          break;
        }
      }
    }
  else
    result = EvErrAlarm3;
  return result;
}
//--------------------------------------------------------------
void stat_gen::start_alarm(int al)
{
  alarm[al]->startTime();
  if(al > ID_TIME_STOP) // se non è il time of run or stop
    alarm[ID_TIME_STOP]->startTime();
}
//--------------------------------------------------------------
void stat_gen::stop_alarm(int al)
{
  alarm[al]->stopTime();
  if(al > ID_TIME_STOP) // se non è il time of run or stop
    alarm[ID_TIME_STOP]->stopTime();
}
//--------------------------------------------------------------
void stat_gen::reset_alarm(int al)
{
  alarm[al]->reset_alarm();
}
//--------------------------------------------------------------
int stat_gen::init()
{
  for(int i = 0; i < MAX_GRP_ALARM; ++i)
    reset_alarm(i);
  return 1;
}
//---------------------------------------------------------------

