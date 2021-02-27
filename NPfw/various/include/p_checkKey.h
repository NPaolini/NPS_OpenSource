//----- p_checkKey.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef p_checkKey_H
#define p_checkKey_H
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_util.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
namespace p_checkKey
{
//----------------------------------------------------------------------------
// versione attuale
#define HI_VER 3
#define LO_VER 3
//----------------------------------------------------------------------------
#define MAKE_VER(h, l) MAKELPARAM(l, h)
//----------------------------------------------------------------------------
#define MAX_DIM_KEY 2048
//----------------------------------------------------------------------------
#define MAX_DUMMY_BYTES 64
//----------------------------------------------------------------------------
#define MAKE_DW(s) ((uint(s[3]) << 24) | (uint(s[2]) << 16) | (uint(s[1]) << 8) | uint(s[0]))
//----------------------------------------------------------------------------
#define HEAD_NPS_MIN "NPS\0"
#define HEAD_NPS_MIN_DW MAKE_DW(HEAD_NPS_MIN)
//----------------------------------------------------------------------------
#define INFINITE_DATE I64_TO_FT(-1)
//----------------------------------------------------------------------------
#define DIM_svKEY 8
//----------------------------------------------------------------------------
struct key_dataInfo
{
// dati presenti da questa versione (3.0)
  FILETIME ftCurr;
  FILETIME ftLast;
// dati presenti da questa versione (3.1)
  BYTE svKey[DIM_svKEY * 2];

// dati presenti da questa versione (3.2)
  WORD maxAllowed; // numero max di applicazioni simultanee, zero -> senza limiti
  WORD numCode;    // numero di codici abbinati + data scadenza (strutture infoKeyItemV2)
  DWORD dwReserved; // per allineamento a 8 byte
#if 0
  BYTE reserved[63][8];
#else
// da questa versione (3.3) *non cambia la dimensione
  BYTE customer[8];
  BYTE reserved[62][8];
#endif
  key_dataInfo() : ftCurr(I64_TO_FT(0)), ftLast(I64_TO_FT(0)), maxAllowed(0), numCode(0), dwReserved(0)
  { ZeroMemory(svKey, sizeof(svKey)); memset(customer, ' ', sizeof(customer)); ZeroMemory(reserved, sizeof(reserved)); }
};
//----------------------------------------------------------------------------
struct key_head
{
  char nps[4];
  DWORD ver;
  // offset inizio dati. Le personalizzazioni partono dopo key_head + lendummy (viene sommato
  // a runtime), quindi key_head, poi lendummy, poi key_dataInfo ed infine le chiavi
  DWORD offset;
  key_head() : ver(MAKE_VER(HI_VER, LO_VER)), offset(sizeof(key_dataInfo) + sizeof(key_head))
  {
    *(LPDWORD)nps = HEAD_NPS_MIN_DW;
  }
};
//----------------------------------------------------------------------------
struct infoKeyItem
{
  char pKey[MAX_DIM_KEY];
  int lenKey;
  infoKeyItem() : lenKey(0) { ZeroMemory(pKey, sizeof(pKey)); }
};
//----------------------------------------------------------------------------
struct infoKeyItemV2
{
  BYTE pKey[DIM_svKEY * 2];
  FILETIME ftEnd;
  infoKeyItemV2() : ftEnd(I64_TO_FT(0)) { ZeroMemory(pKey, sizeof(pKey)); }
};
//----------------------------------------------------------------------------
struct infoKeyItemResultV2
{
  infoKeyItemV2 svKey;
  uint maxAllowed;
  infoKeyItemResultV2() : maxAllowed(0) {  }
};
//----------------------------------------------------------------------------
struct infoKeyCheck
{
  key_head head;
  uint lenDummyAdded;
  uint enabled;
  bool reversedWord;
  PWin* owner;
  infoKeyCheck() : lenDummyAdded(0), reversedWord(false), enabled(0), owner(0) {}
};
//----------------------------------------------------------------------------
// non rispecchia la posizione su file, viene usata per leggere/scrivere
struct infoKey
{
  key_head head;
  key_dataInfo kdi;
  uint lenDummyAdded;
  uint enabled;
  bool reversedWord;
  PVect<infoKeyItem> items;

  // dalla V 3.2
  uint maxAllowed;
  uint numCode;
  PVect<infoKeyItemV2> items2;

  PWin* owner;
  infoKey() : lenDummyAdded(0), reversedWord(false), owner(0), enabled(0), maxAllowed(0), numCode(0) {}
  infoKey(const infoKeyCheck& ikc) :
      lenDummyAdded(ikc.lenDummyAdded), reversedWord(ikc.reversedWord), head(ikc.head),
      owner(ikc.owner), enabled(ikc.enabled), maxAllowed(0), numCode(0) { }
  infoKey(const infoKey& ik) :
      lenDummyAdded(ik.lenDummyAdded), reversedWord(ik.reversedWord), head(ik.head),
      items(ik.items), kdi(ik.kdi), owner(ik.owner), enabled(ik.enabled), maxAllowed(ik.maxAllowed),
      numCode(ik.numCode), items2(ik.items2) { }
};
//----------------------------------------------------------------------------
enum errCheckKey {
  eckNo_Err, eckNoFile, eckNoWriteFile,
  eckNoReadFile, eckErrCRC, eckNoKey,
  eckWrongFile, eckWrongDate, eckOutOfDate, eckInfiniteDate, eckWrongVersion, eckUnknowErr
  };
//----------------------------------------------------------------------------
// usate da programmi di gestione key
errCheckKey saveKey(LPCTSTR filename, const infoKey& ik, bool prompt);
errCheckKey loadKey(LPCTSTR filename, infoKey& ik, bool prompt);
//----------------------------------------------------------------------------
// carica in svKeyTarget la nuova chiave di abbinamento tra due programmi (sVisor e svMaker)
// se trueVal il valore tornato è in chiaro e a 64 bit, altrimenti è a 128 bit e occorre usare la funzione
// sotto per recuperare il valore vero
// Ver 3.2, svKeyTarget deve contenere una chiave da verificare, visto che posso esserci più codice
// di abbinamento
errCheckKey check_svKey(LPCTSTR filename, const infoKeyCheck& ikc, LPBYTE svKeyTarget, bool trueVal);
//----------------------------------------------------------------------------
// nuove funzioni per versione >= 3.2, si passa la struttura completa per chiarezza, ma la data non serve
// al suo posto viene usato un flag per indicare se la chiave è in chiaro (zero) o codificata (!= zero)
// semplice verifica, non è usata nemmeno quella base, inutile implementarla
// errCheckKey checkKey(LPCTSTR filename, const infoKeyCheck& ikc, const infoKeyItemV2* iki/* = 0*/);

// verifica e aggiorna data, in iki->maxAllowed viene tornato il maxAlloved della chiave
// se iki == null viene usata la vecchia funzione
errCheckKey checkAndSaveKey(LPCTSTR filename, const infoKeyCheck& ikc, infoKeyItemResultV2* iki = 0);
//----------------------------------------------------------------------------
// in chiaro è a 64bit, codificata a 128
void make_svKey(LPBYTE target, LPCBYTE source); // crea la chiave 64 -> 128
void get_svKey(LPBYTE target, LPCBYTE source);  // ritorna la chiave 128 -> 64

// fullVal indica se è a 128 bit(codif.) o a 64 bit(in chiaro)
void make_svEmptyKey(LPBYTE target, bool fullVal); // crea la chiave vuota
bool is_svEmptyKey(LPCBYTE source, bool fullVal); // verifica se sia la chiave vuota

// verifica l'eguaglianza di due chiavi codificate a 128 bit
bool verify_svKey(LPCBYTE k1, LPCBYTE k2);
//----------------------------------------------------------------------------
}; // end namespace
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
