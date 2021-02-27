//----------- log_ord.cpp --------------------------------------
#include "precHeader.h"

#include "log_ord.h"
#include "val_ord.h"
#include "p_base.h"
#include <stdlib.h>
#include "def_dir.h"
#include "p_Util.h"

#include "1.h"
#include "5500.h"

stat_gen *allocOrder() { return new stat_order; }
//--------------------------------------------------------------
#define Z_TIME(a) (reinterpret_cast<LARGE_INTEGER*>(&a))->QuadPart = 0

val_order::val_order() : status(oUndef)
{
  Z_TIME(startDate);
  Z_TIME(endDate);
  fillStr(cod_client, 0, SIZE_A(cod_client));
  fillStr(name, 0, SIZE_A(name));
}
//--------------------------------------------------------------
UDimF val_order::read(P_File &f)
{
  UDimF len = val_gen::read(f);
  len += f.P_read(name, sizeof(name));
  len += f.P_read(cod_client, sizeof(cod_client));
  len += f.P_read(&startDate, sizeof(startDate));
  len += f.P_read(&endDate, sizeof(endDate));

  return len;
}
//--------------------------------------------------------------
UDimF val_order::write(P_File &f)
{
  UDimF len = val_gen::write(f);
  len += f.P_write(name, sizeof(name));
  len += f.P_write(cod_client, sizeof(cod_client));
  len += f.P_write(&startDate, sizeof(startDate));
  len += f.P_write(&endDate, sizeof(endDate));
  return len;
}
//--------------------------------------------------------------
// rimpiazza il separatore dei decimali di default (.) con quello
// passato [definita in DListExp.cpp]
extern void replaceSep(LPTSTR buff, LPCTSTR sep);

//#define ID_MONTH 100
bool val_order::exportData(P_File &f)
{
  TCHAR buff[1000];
  SYSTEMTIME t_ini;
  SYSTEMTIME t_end;
  FileTimeToSystemTime(&startDate, &t_ini);
  FileTimeToSystemTime(&endDate, &t_end);
  LPCTSTR tab = getStringOrId(ID_TAB);
  wsprintf(buff,
          _T("%s%s")
          _T("%d %s %d%s")
          _T("%02d:%02d:%02d%s")
          _T("%d %s %d%s")
          _T("%02d:%02d:%02d%s"),
          cod_client, tab,
          t_ini.wDay, getStringOrId(ID_FIRST_MONTH + t_ini.wMonth - 1), t_ini.wYear,tab,
          t_ini.wHour, t_ini.wMinute, t_ini.wSecond,tab,
          t_end.wDay, getStringOrId(ID_FIRST_MONTH + t_end.wMonth - 1), t_end.wYear,tab,
          t_end.wHour, t_end.wMinute, t_end.wSecond,tab);
  LPCTSTR sep = getString(ID_SEP_DEC);
  replaceSep(buff, sep);

  if(!f.P_writeString(buff))
    return false;

  return true;
/*
  char buff[1000];
  SYSTEMTIME t_ini;
  SYSTEMTIME t_end;
  FileTimeToSystemTime(&startDate, &t_ini);
  FileTimeToSystemTime(&endDate, &t_end);
  LPCTSTR tab = getStringOrId(ID_TAB);
  wsprintfA(buff,
          "%s%s"
          "%d %s %d%s"
          "%02d:%02d:%02d%s"
          "%d %s %d%s"
          "%02d:%02d:%02d%s",
          cod_client, tab,
          t_ini.wDay, getStringOrId(ID_FIRST_MONTH + t_ini.wMonth - 1), t_ini.wYear,tab,
          t_ini.wHour, t_ini.wMinute, t_ini.wSecond,tab,
          t_end.wDay, getStringOrId(ID_FIRST_MONTH + t_end.wMonth - 1), t_end.wYear,tab,
          t_end.wHour, t_end.wMinute, t_end.wSecond,tab);
  LPCTSTR sep = getString(ID_SEP_DEC);
  replaceSep(buff, (const char*)sep);

  if(!f.P_writeString(buff))
    return false;

  return true;
*/
}
//--------------------------------------------------------------
bool val_order::exportHeader(P_File &f)
{
  LPCTSTR tab = getStringOrId(ID_TAB);
  for(int i = ID_EXP_ORDER_INIT; i <= ID_EXP_ORDER_END; ++i) {
    LPCTSTR p = getStringOrId(i);
    if(!f.P_writeString(p))
      return false;
    f.P_writeString(tab);
    }
  return true;
}
//--------------------------------------------------------------
typedef LPCTSTR (*pFzExt)(eDir);
typedef void (*pFzPath)(LPTSTR);
//--------------------------------------------------------------
static void moveFileToHistory(LPCTSTR file, eDir dir)
{
  TCHAR tmpfile[_MAX_PATH] = _T("\0");

  makePath(tmpfile, file, dir, true);

  int len = _tcslen(file);
  // raggiunge l'ultima '\\' (5 == "\\" + ".ext")
  int i = _tcslen(tmpfile) - 5 - len;
  tmpfile[i] = 0;

  CreateDirectory(tmpfile, 0);

  // ripristina
  tmpfile[i] = _T('\\');

  TCHAR pathHist[_MAX_PATH];
  LPTSTR dummy;
  GetFullPathName(tmpfile, SIZE_A(pathHist), pathHist, &dummy);

  makePath(tmpfile, file, dir, false);

  TCHAR pathJob[_MAX_PATH];
  GetFullPathName(tmpfile, SIZE_A(pathJob), pathJob, &dummy);
  do {
    WIN32_FIND_DATA fd;
    HANDLE hfff = FindFirstFile(pathHist, &fd);
    if(INVALID_HANDLE_VALUE != hfff) {
      FindClose(hfff);
      if(IDNO == ::MessageBox(0, getStringOrId(ID_MSG_EXIST_MOVE),
                  getStringOrId(ID_TITLE_EXIST_MOVE),
                  MB_YESNO | MB_ICONSTOP))
        return;
      DeleteFile(pathHist);
      }
    } while(0);
  MoveFile(pathJob, pathHist);
}
//--------------------------------------------------------------
stat_order::stat_order() : Which(eCurrent), data(new val_order) { }
//--------------------------------------------------------------
stat_order::~stat_order()
{
  delete data;
}
//--------------------------------------------------------------
void stat_order::set_data(const val_gen *val)
{
  *data = *val;
}
//--------------------------------------------------------------
const val_order& stat_order::get_data()
{
  return *data;
}
//--------------------------------------------------------------
void stat_order::get_data(val_gen& buff)
{
  buff = *data;
}
//--------------------------------------------------------------
#define dSetOfJob dBob
int stat_order::action(bool load)
{
  if(load && !stat_gen::init())
    return 0;
  if(!*data->name) // non c'è il nome, ordine non attivo
    return 0;
  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, SIZE_A(file), data->name);
  getPathOrd(file);
  if(eHistory == Which)
    getPath(file, dHistory);
  _tcscat_s(file, SIZE_A(file), getExt(dOrdini));
  int ret = 0;
  do {
    P_File f(file);
    if(f.P_open()) {
      if(load)
        ret = (int)read(f);
      else
        ret = (int)write(f);
      }
    } while(0);
      // se è in scrittura ed è terminato l'ordine sposta il file nello storico
  if(!load && eCurrent == Which) {
    if(oEnded == data->status) {
      moveFileToHistory(data->name, dOrdini);
      moveFileToHistory(data->name, dSetOfJob);
      }
    // se lo stato indica una disattivazione viene cancellato il nome
    if(oEnded == data->status || oSuspended == data->status || oSaveNoActive == data->status)
      data->name[0] = 0;
    }
  if(load) {
    // carica la data corrente e la assegna alla data finale
    SYSTEMTIME st;
    GetLocalTime(&st);
    SystemTimeToFileTime(&st, &data->endDate);

    // se non è stato riaperto un ordine sospeso
    // la data di inizia è quella appena caricata e resetta
    // tutti gli allarmi
    if( oSuspended != data->status) {
      data->startDate = data->endDate;
      for(int i = 0; i < MAX_GRP_ALARM; ++i)
        reset_alarm(i);
      }
    data->status = oLoad;
    }
  return ret;
}
//--------------------------------------------------------------
UDimF stat_order::write(P_File &f)
{
  UDimF n = stat_gen::write(f);
  n += data->write(f);
  return n;
}
//--------------------------------------------------------------
UDimF stat_order::read(P_File &f)
{
  UDimF n = stat_gen::read(f);
  n += data->read(f);
  return n;
}
//--------------------------------------------------------------
bool stat_order::exportData(P_File &t)
{
  if(!*data->name) // non c'è il nome, ordine non caricato
    return false;
  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, SIZE_A(file), data->name);
  getPathOrd(file);
  if(eHistory == Which)
    getPath(file, dHistory); // solo gli ordini chiusi e quindi nello storico
  _tcscat_s(file, SIZE_A(file), getExt(dOrdini));
  P_File f(file, P_READ_ONLY);
  if(f.P_open()) {
    if(read(f)) {
      if(!data->exportHeader(t))
        return false;
      if(!stat_gen::exportHeader(t))
        return false;
      t.P_writeString(_T("\r\n")); // termina la riga
      if(!data->exportData(t))
        return false;
      if(!stat_gen::exportData(t))
        return false;
      t.P_writeString(_T("\r\n")); // termina la riga
      return true;
      }
    }
  return false;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
log_status::errEvent stat_order::setEvent(const val_gen* val)
{
  if(val) {
    const val_order *ord = static_cast<const val_order*>(val);
    switch(ord->status) {
/*
      case oInit:    // nuovo ordine o annulla dati
        set_data(ord);
        save();
        // fall through
*/
      case oLoad:     // carica ordine (sospeso) da disco
        set_data(ord);
        return init() > 0 ? log_status::EvNoErr : log_status::EvErrOrd;

      case oSaveNoActive:
      case oSave:
      case oSuspended:  // terminato o sospeso, salva su disco
      case oEnded:
        set_data(ord);
        return save() > 0 ? log_status::EvNoErr : log_status::EvErrOrd;
      case oActive: // richiede i dati attuali dell'ordine
        CopyMemory(const_cast<val_order*>(ord), &get_data(), sizeof(*ord));
        return log_status::EvNoErr;
      default:    // caso non previsto
        return log_status::EvErrOrd;
      }
    }
  return log_status::EvNoErr; // non sono stati passati dati, ignoriamo
}

