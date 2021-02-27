//----------- def_dir.h ------------------------------------------------
//----------------------------------------------------------------------
#ifndef DEF_DIR_H_
#define DEF_DIR_H_
//----------------------------------------------------------------------
#include "setPack.h"

// enumeratore per directory ed estensioni di default
enum eDir {
  dAlarm,
  dRicette,
  dOrdini,
  dOper,
  dTurn,
  dSystem,
  dHistory,
  dBob,
  dRepAlarm,
  dExRep,
  dExport,
  // usata solo come argomento di default per estensione
  dUseExtDefault = 1000
  };

LPCTSTR getExt(eDir path);

LPCTSTR getExtExport();

// tutti i buffer passati devono essere almeno _MAX_PATH

// costruisce il path formato dalla dir prescelta e dal file passato,
// il nuovo path si ritrova in path
void getPath(LPTSTR path, eDir which);

// come sopra, ma torna il percorso relativo
void getRelPath(LPTSTR path, eDir which);

// usano il percorso assoluto, da non usare insieme
// alla getPathHistory

// se occorre un percorso aggiuntivo per history usare la makePath()
//inline void getPathHistory(LPTSTR  path){ getPath(path, dHistory); }

inline void getPathAlarm(LPTSTR path)  { getPath(path, dAlarm); }
inline void getPathRepAlarm(LPTSTR path)   { getPath(path, dRepAlarm); }
inline void getPathRic(LPTSTR path)    { getPath(path, dRicette); }
inline void getPathOrd(LPTSTR path)    { getPath(path, dOrdini); }
inline void getPathOper(LPTSTR path)   { getPath(path, dOper); }
inline void getPathTurn(LPTSTR path)   { getPath(path, dTurn); }
inline void getPathSystem(LPTSTR path) { getPath(path, dSystem); }
inline void getPathBob(LPTSTR path)    { getPath(path, dBob); }
inline void getPathExt(LPTSTR path)    { getPath(path, dExRep); }

inline LPCTSTR getExtAlarm()  { return getExt(dAlarm); }
inline LPCTSTR getExtRepAlarm()   { return getExt(dRepAlarm); }
inline LPCTSTR getExtRic()    { return getExt(dRicette); }
inline LPCTSTR getExtOrd()    { return getExt(dOrdini); }
inline LPCTSTR getExtOper()   { return getExt(dOper); }
inline LPCTSTR getExtTurn()   { return getExt(dTurn); }
inline LPCTSTR getExtSystem(eDir dir) { return getExt(dSystem); }
inline LPCTSTR getExtBob()    { return getExt(dBob); }
inline LPCTSTR getExtRepEx()    { return getExt(dExRep); }

// il nome deve essere privo di estensione, verrà aggiunta dalla funzione
void makePath(LPTSTR result, LPCTSTR name, eDir which, bool history, eDir ext = dUseExtDefault);
// come sopra, ma usa file sia in input che in output
void makePath(LPTSTR file, eDir which, bool history, eDir ext = dUseExtDefault);

// se occorre un'estensione diversa
void makePath(LPTSTR result, LPCTSTR name, eDir which, bool history, LPCTSTR ext);
// come sopra, ma usa file sia in input che in output
void makePath(LPTSTR file, eDir which, bool history, LPCTSTR ext);

// se il percorso di esport contiene un '\\' viene preso così com'è, senza
// appendere history, altrimenti richiama la makePath()
void makeExportPath(LPTSTR result, LPCTSTR name, bool history, eDir ext);

// crea il percorso locale relativo se il path è di rete togliendo il doppio
// backslash iniziale, torna true se il percorso è di rete
bool makeLocalDir(LPCTSTR  path);

void checkForHistory(LPCTSTR  path);
void checkForHistoryMonth(LPCTSTR  path);

// da richiamare all'avvio per creare tutte le directory di cui si ha bisogno.
// Prima di creare le directory richiama setBasePath() (per compatibilità
// con le versioni precedenti)
void makeAllDir();

// torna il path
extern LPCTSTR  getDir(eDir which);

// torna il percorso base (anche rete), necessario per file senza percorso specifico
extern LPCTSTR  getBaseDir();

extern void getBasePath(LPTSTR  file);

// torna il percorso base locale
extern LPCTSTR  getLocalBaseDir();

// da richiamare all'avvio del programma (occorre inizializzare i percorsi di lavoro)
// e comunque ogni volta che si cambia il percorso base. Se si richiama la
// routine makeAllDir() e non si cambia il percorso base non è necessario chiamarla
extern void setBasePath(LPCTSTR  base = 0);

#include "restorePack.h"
#endif
