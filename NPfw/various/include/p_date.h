//------------- p_date.h ---------------------------------------
//--------------------------------------------------------------
#ifndef P_DATE_H_
#define P_DATE_H_
//--------------------------------------------------------------
#include "precHeader.h"

#ifndef PDEF_H_
  #include "pDef.h"
#endif
#include "setPack.h"
//--------------------------------------------------------------
#define MAX_DAY 366
//--------------------------------------------------------------
// la data può avere un qualsiasi separatore non numerico,
// deve essere una stringa ASCIIZ
//--------------------------------------------------------------
enum eDataFormat { European = 1, American, Other };
// European -> dd/mm/yyyy
// American -> mm/dd/yyyy
// Other    -> yyyy/mm/dd
eDataFormat whichData();

// trasforma [day] in data secondo il formato passato, ci attacca
// il tab e lo scrive nel file
void exportDay(long day, class P_File &t, eDataFormat format = European, LPCTSTR tab = _T("\t"));
//--------------------------------------------------------------
// estrae l'anno dal nome del file (gli ultimi quattro caratteri, esclusa l'estensione)
// e li scrive in buff
void extrYear(LPCTSTR File, LPTSTR buff, size_t lenBuff);
//--------------------------------------------------------------
// torna 1 se l'anno è bisestile
int isBisestile(int anno);
//--------------------------------------------------------------
// torna il numero del giorno dall'inizio dell'anno (anno corrente se tm == 0)
long getDay(SYSTEMTIME* tm = 0);

inline long getDay(FILETIME* ftm)
{
  SYSTEMTIME stm;
  FileTimeToSystemTime(ftm, &stm);
  return getDay(&stm);
}
//--------------------------------------------------------------
// torna il numero dell'ultimo giorno del mese (data corrente se tm == 0)
int getLastDay(SYSTEMTIME* tm = 0);

inline int getLastDay(FILETIME* ftm)
{
  SYSTEMTIME stm;
  FileTimeToSystemTime(ftm, &stm);
  return getLastDay(&stm);
}
//--------------------------------------------------------------
// conversione da data in formato testo a numero long e viceversa
long d2l(LPCTSTR d, eDataFormat format = European);
LPTSTR l2d(long d, LPTSTR s, size_t lenBuff, eDataFormat format = European);
//----------------------------------------------------------------
// scrive in buff il formato dd/mm/yyyy (mm/dd/yyyy se !european),
// inserisce il tab e continua con hh:mm:ss
void set_format_data(LPTSTR buff, size_t lenBuff, const SYSTEMTIME& stm, eDataFormat format, LPCTSTR tab);
//----------------------------------------------------------------
inline void set_format_data(LPTSTR buff, size_t lenBuff, const FILETIME& ftm, eDataFormat format, LPCTSTR tab)
{
  SYSTEMTIME stm;
  FileTimeToSystemTime(&ftm, &stm);
  set_format_data(buff, lenBuff, stm, format, tab);
}
//----------------------------------------------------------------
// come quella sopra, ma scrive anche i msec, hh:mm:ss,xxx
void set_format_data_full(LPTSTR buff, size_t lenBuff, const SYSTEMTIME& stm, eDataFormat format, LPCTSTR tab);
//----------------------------------------------------------------
inline void set_format_data_full(LPTSTR buff, size_t lenBuff, const FILETIME& ftm, eDataFormat format, LPCTSTR tab)
{
  SYSTEMTIME stm;
  FileTimeToSystemTime(&ftm, &stm);
  set_format_data_full(buff, lenBuff, stm, format, tab);
}
//--------------------------------------------------------------
void unformat_data(LPCTSTR buff, SYSTEMTIME& stm, eDataFormat format);
//--------------------------------------------------------------
inline void unformat_data(LPCTSTR buff, FILETIME& ftm, eDataFormat format)
{
  SYSTEMTIME stm;
  unformat_data(buff, stm, format);
  SystemTimeToFileTime(&stm, &ftm);
}
//--------------------------------------------------------------
// scrive in buff il formato h:mm:ss
void set_format_time(LPTSTR buff, size_t lenBuff, const FILETIME& ftm);
//--------------------------------------------------------------
inline void set_format_time(LPTSTR buff, size_t lenBuff, const SYSTEMTIME& stm)
{
  FILETIME ftm;
  SystemTimeToFileTime(&stm, &ftm);
  set_format_time(buff, lenBuff, ftm);
}
//--------------------------------------------------------------
// come quella sopra ma aggiunge i msec
void set_format_time_full(LPTSTR buff, size_t lenBuff, const FILETIME& ftm);
//--------------------------------------------------------------
inline void set_format_time_full(LPTSTR buff, size_t lenBuff, const SYSTEMTIME& stm)
{
  FILETIME ftm;
  SystemTimeToFileTime(&stm, &ftm);
  set_format_time_full(buff, lenBuff, ftm);
}
//--------------------------------------------------------------
void unformat_time(LPCTSTR buff, SYSTEMTIME& stm);
//--------------------------------------------------------------
//--------------------------------------------------------------
inline void unformat_time(LPCTSTR buff, FILETIME& ftm)
{
  SYSTEMTIME stm;
  unformat_time(buff, stm);
  SystemTimeToFileTime(&stm, &ftm);
}
//--------------------------------------------------------------
// scrive in name, nei primi quattro caratteri, l'anno corrente.
// non termina la stringa con lo zero
void makeYear(LPTSTR name, const SYSTEMTIME& stm);
// subYear indica gli anni da togliere a quello corrente
void makeYear(LPTSTR name, int subYear = 0);

// come in makeYear(), poi carica nei successivi due caratteri il mese corrente
void makeYearAndMonth(LPTSTR name, const SYSTEMTIME& stm);
// subMonth indica i mesi da togliere a quello corrente
void makeYearAndMonth(LPTSTR name, int subMonth = 0);

// come sopra, ma usa il time passato, poi lo modifica  e ritorna il nuovo valore
SYSTEMTIME makeYearAndMonth(LPTSTR name, const SYSTEMTIME& st, int subMonth);

// scrive in name aaaammdd
void makeYearMonthAndDay(LPTSTR name, const SYSTEMTIME& stm);
void makeYearMonthAndDay(LPTSTR name);
//--------------------------------------------------------------
// torna la data/ora di sistema nel formato FILETIME
FILETIME getFileTimeCurr();
FILETIME getFileTimeUTC();
//--------------------------------------------------------------
#include "restorePack.h"

#endif
