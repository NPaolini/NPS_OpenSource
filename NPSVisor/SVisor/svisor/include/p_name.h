//-------------- p_name.h -----------------------------------------
//-----------------------------------------------------------------
#ifndef P_NAME_H_
#define P_NAME_H_
//-----------------------------------------------------------------
//-----------------------------------------------------------------
#include "precHeader.h"
#include "p_util.h"
#include "p_txt.h"
#include "smartPS.h"
//-----------------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------------
#ifndef WM_PLC
  #define WM_PLC 2
#endif
#define FIRST_PRPH_ALARM 1
//----------------------------------------------------------------------------
#define PAGE_EXT _T(".npt")
#define FIRST_PAGE _T("main_page") PAGE_EXT
//----------------------------------------------------------------------------
#define STD_MSG_NAME _T("base_data") PAGE_EXT
//-----------------------------------------------------------------
// da richiamare all'inizio per creare l'albero, verrà distrutto
// automaticamente al termine del programma
void load_Txt();
void load_Global_Txt();
void load_Txt(setOfString& set, LPCTSTR name);
// se non trova l'id torna 0
LPCTSTR getString(long id);

LPCTSTR getGlobalPageString(long id);

//sostituisce/aggiunge
void replaceString(long id, LPTSTR newString, bool addIfNotExist = false, bool deleteOld = true);

// se non trova l'id torna, comunque, un messaggio con l'indicazione
// dell'id mancante
LPCTSTR getStringOrId(long id);
// azzera il set di testi
void resetTxt();
//-----------------------------------------------------------------
// come sopra, ma per le stringhe di allarme
void load_Alarm();
LPCTSTR getStringAlarm(long id);
LPCTSTR getStringOrIdAlarm(long id);

// nuova versione per filtro, se file vecchio torna il gruppo
// altrimenti il codice
// torna -1 se l'id non esiste
long getAlarmFilterCode(long id);
void checkIfUseFilterCode();

// nuova gestione per report
bool useAlarmSection();
bool useMultipleAlarmFile();
LPCTSTR getAlarmFullRow(long id);
uint getPrphFromAlarm(uint id, uint& newId);
//-----------------------------------------------------------------
uint makeIdAlarmByPrph(uint id, uint prph);
void splitIdAlarmByPrph(long id, uint& alarm, uint& prph);
void splitIdAlarmByPrph(long id, int& alarm, int& prph);
//-----------------------------------------------------------------
void setAckAlarm(long& id);
bool isAckAlarm(long& id, bool clearBit = true);
void clearUnusedBitAlarm(long& id);
//-----------------------------------------------------------------
void allocInfoAlarm(PVect<setOfBlockString*>& target);
//-----------------------------------------------------------------
// nuova gestione allarmi con valore associato, testo separato e gestione lingua
// non si può più utilizzare la getStringAlarm() per recuperare la descrizione
// dell'allarme. Va usata la getInfoAlarm() per reperire tutte le informazioni.
// Anche le altre routine verranno invalidate se non necessarie
//-----------------------------------------------------------------
/*
  I metodi clone(), copy() e move() hanno i seguenti scopi:
  clone() -> esegue una copia completa dell'oggetto, quindi copia il puntatore
             a sottooggetti allocati se questi non necessitano di essere distrutti
             mentre allocano nuovo spazio se hanno il needDelete attivo.
  copy() ->  usato per oggetti temporanei a sola lettura, non alloca nulla e pone
             il proprio needDelete a false
  move() ->  sposta gli oggetti ponendone a zero il puntatore nell'oggetto
             passato e incaricandosi della loro eventuale distruzione, usato
             per il passaggio tra oggetti
*/
//-----------------------------------------------------------------
struct infoAssocAlarm
{
  enum tType {
    NO_ASSOC,
    tDW,    // valore intero
    tFLT,   // valore float
    tSTR,   // puntatore a stringa
    tDW4IX, // stringa ma non è stata richiesta e viene tornato solo l'id (nel campo dw)
            // viene utilizzato per salvataggio su disco
    };
  union {
    DWORD dw;
    float flt;
    LPCTSTR str;
    };
  uint type;
  uint nDec; // valido se è tipo float
  bool needDelete; // se attivo si tratta di una stringa allocata e viene distrutta
  infoAssocAlarm() : type(NO_ASSOC), dw(0), needDelete(false), nDec(0) {}
  ~infoAssocAlarm() { if(needDelete) delete []str; }

  void clone(const infoAssocAlarm& other);
  void copy(const infoAssocAlarm& other);
  void move(infoAssocAlarm& other);

  bool save(P_File& pf);
  bool load(P_File& pf);
};
//-----------------------------------------------------------------
struct fullInfoAlarm
{
  int alarm;
  int prph; // è zero based, quindi zero corrisponde alla periferica FIRST_PRPH_ALARM
  int idGrp;
  int idFlt;
  smartPointerConstString descr;
  infoAssocAlarm iaa;
  int getTruePrph() const { return prph + FIRST_PRPH_ALARM; }
  fullInfoAlarm(long ix) : alarm(0), prph(0), idGrp(-1), idFlt(-1) { splitId(ix); }
  fullInfoAlarm() : alarm(0), prph(0), idGrp(-1), idFlt(-1) {}

  void clone(const fullInfoAlarm& other);
  void copy(const fullInfoAlarm& other);
  void move(fullInfoAlarm& other);

  long makeFullIdAlarm() const;
  void splitId(long ix);

  bool save(P_File& pf);
  bool load(P_File& pf);

};
//-----------------------------------------------------------------
// se attivo onlyValue non carica il puntatore al testo, ma il valore
//void getAssocValue(long id, infoAssocAlarm& iaa, bool onlyValue);
// torna il testo associato (se esiste, zero altrimenti)
//smartPointerConstString getAssocText(long id, DWORD value);

bool getTextAlarm(long id, fullInfoAlarm& fia, bool onlyValue = false);
//-----------------------------------------------------------------
bool getTextReport(long id, fullInfoAlarm& fia, const infoAssocAlarm& info);
//-----------------------------------------------------------------
void makeAlarmDescr(LPTSTR descr, size_t len, const fullInfoAlarm& fia);
//-----------------------------------------------------------------
// carica i file predefiniti, ma prima li decripta
// tornano il CRC (WORD),
// tornano (DWORD)-2 se il file non esiste
// tornano (DWORD)-1 errore
//DWORD load_CriptedTxt(LPCBYTE key, int dimKey, int step);
//DWORD load_CriptedAlarm(LPCBYTE key, int dimKey, int step);
DWORD load_CriptedTxt(LPCBYTE key);
DWORD load_CriptedTxt(LPCBYTE key, setOfString& set, LPCTSTR name);
DWORD load_CriptedAlarm(LPCBYTE key);
//-----------------------------------------------------------------
#define CRYPT_HEAD "CRYPNPS"
#define DIM_HEAD sizeof(CRYPT_HEAD)
//-----------------------------------------------------------------
#define CRYPT_STEP    16
#define DIM_KEY_CRYPT  8
//-----------------------------------------------------------------
// se riesce a trovare il file nella sottocartella system usa quello,
// altrimenti usa il path del programma
void checkSystemPathTxt(LPTSTR target, LPCTSTR name);
//-----------------------------------------------------------------
#include "restorePack.h"

#endif
