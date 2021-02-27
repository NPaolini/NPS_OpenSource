//----------- log_oper.cpp --------------------------------------
#include "precHeader.h"

#include "log_oper.h"

#include "p_base.h"
#include "p_date.h"
#include "def_dir.h"

#include "1.h"
#include "5500.h"

#define SZ_OPER (sizeof(data.oper) + sizeof(data.turno) + sizeof(data.mt) + \
                   sizeof(data.kg))

#define SIZE_OPER (SZ_OPER + SZ_BASE)

stat_gen *allocOper() { return new stat_oper; }
stat_gen *allocTurn() { return new stat_turn; }
//--------------------------------------------------------------
#define RD(a) f.P_read(&a, sizeof(a))
UDimF val_oper::read(P_File &f)
{
 // si potrebbe salvare tutta la struttura in una sola volta, ma
 // i dati della classe base verrebbero inseriti in coda
  UDimF len = val_gen::read(f);
  len += RD(oper);
  len += RD(turno);
  len += RD(mt);
  len += RD(kg);
  return len;
}
//--------------------------------------------------------------
#define WRT(a) f.P_write(&a, sizeof(a))
UDimF val_oper::write(P_File &f)
{
  UDimF len = val_gen::write(f);
  len += WRT(oper);
  len += WRT(turno);
  len += WRT(mt);
  len += WRT(kg);
  return len;
}
//--------------------------------------------------------------
bool val_oper::exportData(P_File &f)
{
  TCHAR buff[100];
  LPCTSTR tab = getStringOrId(ID_TAB);
  wsprintf(buff, _T("%d%s%d%s%d%s%d%s"), oper, tab, turno, tab, mt, tab, kg, tab);
  return toBool(f.P_writeString(buff));
}
//--------------------------------------------------------------
bool val_oper::exportHeader(P_File &f)
{
  TCHAR buff[500];
  LPCTSTR tab = getStringOrId(ID_TAB);
  wsprintf(buff, _T("%s%s%s%s%s%s%s%s"), getStringOrId(ID_EXP_OPER), tab,
        getStringOrId(ID_EXP_TURN), tab, getStringOrId(ID_EXP_MT), tab,
        getStringOrId(ID_EXP_KG), tab);
  return toBool(f.P_writeString(buff));
}
//--------------------------------------------------------------
//--------------------------------------------------------------
int stat_oper_turn::save()
{
  if(!makeFileName())
    return 1;
  P_File f(File);
  if(f.P_open()) {
    if(f.get_len() < SIZE_OPER * MAX_DAY) {
      BYTE tmp[SIZE_OPER];
      ZeroMemory(tmp, sizeof(tmp));
      for(int i = 0; i < MAX_DAY; ++i)
        f.P_write(tmp, SIZE_OPER);
      }
    f.P_seek(getDay() * SIZE_OPER);
    return (int)write(f);
    }
  return 0;
}
//--------------------------------------------------------------
int stat_oper_turn::init()
{
  if(!stat_gen::init())
    return 0;
  if(!makeFileName())
    return 1;
  P_File f(File);
  if(f.P_open()) {
    if(f.get_len() < SIZE_OPER * MAX_DAY) {
      BYTE tmp[SIZE_OPER];
      ZeroMemory(tmp, sizeof(tmp));
      for(int i = 0; i < MAX_DAY; ++i)
        f.P_write(tmp, SIZE_OPER);
      }
    f.P_seek(getDay() * SIZE_OPER);
    return (int)read(f);
    }
  return 1;
}
//--------------------------------------------------------------
UDimF stat_oper_turn::write(P_File &f)
{
  UDimF n = stat_gen::write(f);
  n += data.write(f);
  return n;
}
//--------------------------------------------------------------
UDimF stat_oper_turn::read(P_File &f)
{
  UDimF n = stat_gen::read(f);
  n += data.read(f);
  return n;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
bool stat_oper_turn::exportData(P_File &t)
{
  P_File f(File, P_READ_ONLY);
  if(f.P_open()) {
    if(f.get_len() < SIZE_OPER * MAX_DAY)
      return false;
    LPCTSTR p = getStringOrId(ID_DATA_HEADER);
    t.P_writeString(p);
    p = getStringOrId(ID_TAB);
    t.P_writeString(p);
    if(!data.exportHeader(t))
      return false;
    if(!stat_gen::exportHeader(t))
      return false;
    t.P_writeString(_T("\r\n")); // termina la riga
    TCHAR buff[14] = _T("01/01/");
    extrYear(File, buff + 6, SIZE_A(buff) - 6);
    long day = d2l(buff);
    for(int i = 0; i < MAX_DAY; ++i, ++day) {
      read(f);
      exportDay(day, t, whichData());
      if(!data.exportData(t)) // esporta dati
        return false;
      if(!stat_gen::exportData(t))    // esporta statistiche allarmi
        return false;
      t.P_writeString(_T("\r\n")); // termina la riga
      }
    return true;
    }
  return false;
}
//--------------------------------------------------------------
int stat_oper_turn::getCod(LPCTSTR filename)
{
  TCHAR t[3];
  getCod(t, filename);
  return _ttoi(t);
}
//--------------------------------------------------------------
void stat_oper_turn::getCod(LPTSTR buff, LPCTSTR filename)
{
  buff[2] = 0;
  TCHAR name[_MAX_PATH];

  _tsplitpath_s(filename, 0, 0, 0, 0, name, SIZE_A(name), 0, 0);
  buff[0] = name[2];
  buff[1] = name[3];
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void stat_oper_turn::fillTotalTime(FILETIME *ft, int fromDay, int toDay)
{
  for(int i = 0; i < MAX_GRP_ALARM; ++i)
    (reinterpret_cast<LARGE_INTEGER*>(ft + i))->QuadPart = 0;
  P_File f(File, P_READ_ONLY);
  if(f.P_open()) {
    if(f.get_len() < SIZE_OPER * MAX_DAY)
      return;
    f.P_seek(fromDay * SIZE_OPER);
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
bool stat_oper::makeFileName()
{
   // il codice operatore non deve superare 99
  data.oper %= BYTE(100);
  if(!data.oper)  // se non c'è un operatore attivo, nessuna azione
    return false;
  _tcscpy_s(File, SIZE_A(File), _T("opxx1999"));
  File[2] = static_cast<TCHAR>(data.oper / 10 + _T('0'));
  File[3] = static_cast<TCHAR>(data.oper % 10 + _T('0'));
  _tcscat_s(File, SIZE_A(File), getExt(dOper));
  makeYear(File + 4);
  getPathOper(File);
  checkForHistory(File);
  return true;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
bool stat_turn::makeFileName()
{
   // il codice del turno non deve superare 99
  data.turno %= BYTE(100);
  if(!data.turno)  // se non c'è un turno attivo, nessuna azione
    return false;
  _tcscpy_s(File, SIZE_A(File), _T("tnxx1999"));
  File[2] = static_cast<TCHAR>(data.turno / 10 + _T('0'));
  File[3] = static_cast<TCHAR>(data.turno % 10 + _T('0'));
  _tcscat_s(File, SIZE_A(File), getExt(dTurn));
  makeYear(File + 4);
  getPathTurn(File);
  checkForHistory(File);
  return true;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
int stat_oper::load(const val_oper* val)
{
  data.oper = val->oper;
  return init();
}
//--------------------------------------------------------------
//--------------------------------------------------------------
int stat_turn::load(const val_oper* val)
{
  data.turno = val->turno;
  return init();
}
//--------------------------------------------------------------
//--------------------------------------------------------------
log_status::errEvent stat_oper_turn::setEvent(const val_gen* val)
{
  save(); // prima salva i dati attuali (se attivo)
  if(val) {
    if(1 & val->id) {       // primo avvio
   // è necessario ricaricare i dati da file per le statistiche allarmi
      const val_oper *v = reinterpret_cast<const val_oper*>(val);
      load(v);
//      val->id = 0; // il turno conserva qui informazioni sul tipo
      // potrebbe essere cambiato operatore o turno
      data.oper = v->oper;
      data.turno = v->turno;
      }
    else
      set_data(val);
    return save() > 0 ? log_status::EvNoErr : log_status::EvErrOper;
    }
  return log_status::EvNoErr;
}

