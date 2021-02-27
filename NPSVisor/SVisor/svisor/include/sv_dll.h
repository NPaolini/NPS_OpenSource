//-------- sv_dll.h ------------------------------------------------
#ifndef SV_DLL_H_
#define SV_DLL_H_
//------------------------------------------------------------------
//------------------------------------------------------------------
#ifdef USE_SV_DLL
  // le varie define ed include ci sono già
  #define ExpImp_SV  __declspec(dllexport)
  #define ExpImp_DLL __declspec(dllimport)

  #include "setPack.h"
//------------------------------------------------------------------
#elif defined MAKE_SV_DLL

  #define ExpImp_DLL __declspec(dllexport)
  #define ExpImp_SV  __declspec(dllimport)
//------------------------------------------------------------------
// per evitare di duplicare eventuali typedef
// N.B. se si usano funzioni di libreria occorre richiamare l'include di pDef.h
// (o di una che la richiama) prima di questa
  #ifndef PDEF_H_
    #ifndef STRICT
      #define STRICT
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
      #define WIN32_LEAN_AND_MEAN
    #endif
  //------------------------------------------------------------------
    #include <windows.h>
    #include <winnt.h>
    #include <tchar.h>
  //------------------------------------------------------------------
    typedef short int SWORD;
  //------------------------------------------------------------------
    typedef char  BsDATA;
    typedef short WsDATA;
    typedef long  DWsDATA;
  //------------------------------------------------------------------
    typedef unsigned char  BDATA;
    typedef unsigned short WDATA;
    typedef unsigned long  DWDATA;
    typedef float          fREALDATA;
    typedef double         REALDATA;
  //------------------------------------------------------------------
    typedef unsigned long  uint;
  #endif
  #include "setPack.h"
//------------------------------------------------------------------
// copiata da def_dir.h
enum eSvDir {
  dAlarm,
  dRicette,
  dOrdini,
  dOper,
  dTurn,
  dSystem,
  dHistory, // se passata come parametro per ext, non aggiunge estensione
  dBob,
  dRepAlarm,
  dExRep,
  dExport,
  // usata solo come argomento di default per estensione
  dUseExtDefault = 1000
  };
//------------------------------------------------------------------
#else
  #define ExpImp_DLL
  #define ExpImp_SV
  #include "setPack.h"
#endif
//------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------
// strutture per scambio dati
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct tag_SV_prfData
{
  enum tData   {  tNoData,  // nessun tipo definito -  0
                  tBitData, // campo di bit         -  1
                  tBData,   // 8 bit -> 1 byte      -  2
                  tWData,   // 16 bit-> 2 byte      -  3
                  tDWData,  // 32 bit-> 4 byte      -  4
                  tFRData,  // 32 bit float         -  5
                  ti64Data, // 64 bit               -  6
                  tRData,   // 64 bit float         -  7
                  tDateHour,// 64 bit per data + ora-  8
                  tDate,    // 64 bit per data      -  9
                  tHour,     // 64 bit per ora      - 10
                  // valori signed
                  tBsData,   // 8 bit -> 1 byte      -  11
                  tWsData,   // 16 bit-> 2 byte      -  12
                  tDWsData,  // 32 bit-> 4 byte      -  13

                  tStrData,  // struttura formata dalla dimensione del buffer
                             // ed il puntatore (32 + 32 bit)

                  tNegative = 1 << 30,
//                  tArray = 1 << 31,
               };

  enum tResultData   {  invalidType,
                        invalidAddress,
                        invalidPerif,

                        // per verificare velocemente se si è in errore

                        // da failed (compreso) andando indietro ci sono gli errori
                        failed = invalidPerif,

                        // da okData (compreso) in poi sono tutti validi,
                        okData,

                        notModified = okData,
                        isChanged
                     };

      // buffer per appoggio dato da caricare

      // se campo di bit l'indirizzo è comunque relativo
      // all'indirizzamento, il numero di bit e l'offset richiesti vanno
      // inseriti in u.dw come MAKELONG(nBit, offset). L'offset è
      // inteso come numero di bit da shiftare a dx prima di prendere i bit
      // che servono, es. se i bit sono tre e l'offset 2
      // (per 16 bit f edc ba9 876 543 210 -> - -fe dcb a98 765 432 -> xx

      // in caso di invio bit occorre usare MAKELONG(MAKEWORD(nBit, offset), val)
      // se il valore da inviare supera la capacità di 16 bit probabilmente avrebbe
      // più senso usarlo come word a parte
    union uData {
        BDATA b;
        WDATA w;
        DWDATA dw;

        BsDATA sb;
        WsDATA sw;
        DWsDATA sdw;

        fREALDATA fw;
        REALDATA rw;
        LARGE_INTEGER li;
        FILETIME ft; // usato sia per data/ora/data_ora
        struct strData {
          DWDATA len;
#ifdef _UNICODE
          BYTE buff[1024 - sizeof(DWDATA) - sizeof(uint) - sizeof(DWDATA)];
#else
          BYTE buff[1024 - sizeof(DWDATA)];
#endif
          } str;
        } U;

      // indirizzo logico di lettura, nel buffer
    uint lAddr;

      // tipo di dato da leggere
    DWDATA typeVar;

} SV_prfData, *pSV_prfData;
//------------------------------------------------------------------
//------------------------------------------------------------------
DECLARE_HANDLE(HANDLE_BODY);
//------------------------------------------------------------------
struct listBoxFields
{
  // se shows == NULL i campi saranno tutti visibili
  int* tabs;    // punta ad un vettore che indica la dimensione dei campi
  int* shows;   // punta ad un vettore che indica la visibilità dei campi, se il valore è 2 il campo verrà allineato al centro
  uint nField;  // numero di campi di ciascun vettore
  union {
    uint id;      // id della listbox della pagina
    HWND hwndLb;
    };
};
//------------------------------------------------------------------
// usata per i dati necessari al trasferimento blocco
struct blockInfoPrph
{
  DWORD idPrph;
  DWORD addr;
  DWORD numdw;
};
//------------------------------------------------------------------
// usata come sostituta di eSV_GET_FILE per non mostrare la finestra di scelta
struct loadFileInfo
{
  TCHAR title[256];
  // in e out, in ingresso contiene la dir base + caratteri jolly ed ext, in uscita il nome del file, senza path né ext
  TCHAR path[_MAX_PATH];

 // bit0 -> (1) mostra data-ora dei file
 // bit1 -> (0) ordina per nome - (1) ordina per data
 // bit2 -> (0) ordine normale, (1) ordine rovesciato
 // bit3 -> (1) ricerca anche in history (dove previsto)
  DWORD flag;
};
//------------------------------------------------------------------
// per poter gestire il disegno della listbox occorre caricargli una callback che verrà chiamata
// per i metodi virtuali:
//    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
//    virtual void paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo);
//    virtual void paintCustom(HDC hDC, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
//------------------------------------------------------------------
enum icallbackCode  { icbcNone, icbcWindowProc, icbcSetColor, icbcPaintTxt, icbcPaintCustom, icbPreProcessMsg };
//------------------------------------------------------------------
enum icallbackFlag  { icbfBefore, icbfCanExec, icbfValid, icbfNeedAfter };
//------------------------------------------------------------------
struct infoCallBackListBox
{
  uint code; // codice della funzione
  uint flag; // bit 0 -> beforeExecute, bit1 -> canExecute, bit2 -> validResult
  // beforeExecute -> indica se viene chiamata prima della funzione base
  // canExecute    -> indica (al ritorno) se può eseguire la funzione base
  // validResult   -> result ha un valore valido

  LRESULT result; // impostato dalla funzione che viene eseguita prima
  HWND hwnd;      // per controllo, che ci si riferisca alla listbox giusta
  LPVOID owner;   // punta all'oggeto che installa la callback
  LPVOID var;     // punta ad una struttura che dipende dal codice della funzione
};
//------------------------------------------------------------------
typedef bool (*callBackLB)(infoCallBackListBox* info);
//------------------------------------------------------------------
struct infoCallBackSend
{
  callBackLB fz;
  LPVOID objOwner;
};
//------------------------------------------------------------------
// strutture valide per callback listbox
//------------------------------------------------------------------
// codice icbcWindowProc
struct CallBackListBox_WindowProc
{
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
};
//------------------------------------------------------------------
// codice icbcSetColor
struct CallBackListBox_setColor
{
  HDC hDC;
  DRAWITEMSTRUCT FAR* drawInfo;
  LPCTSTR buff;
};
//------------------------------------------------------------------
// codice icbcPaintTxt
struct CallBackListBox_paintTxt
{
  HDC hDC;
  int x;
  int y;
  LPCTSTR buff;
  int ix;
  DRAWITEMSTRUCT FAR* drawInfo;
};
//------------------------------------------------------------------
// codice icbcPaintCustom
struct CallBackListBox_paintCustom
{
  HDC hDC;
  RECT* r;
  LPCTSTR buff;
  int ix;
  DRAWITEMSTRUCT FAR* drawInfo;
};
//------------------------------------------------------------------
// codice icbPreProcessMsg
struct CallBackListBox_PreProcessMsg
{
  MSG msg;
};
//------------------------------------------------------------------
struct svMakePath // richiama void makePath(LPTSTR file, eSvDir which, bool history, eSvDir ext);
{
  TCHAR file[_MAX_PATH]; // in ingresso contiene il nome del file privo di estensione, in uscita il path completo
  uint which;
  bool history;
  uint ext;
};
//------------------------------------------------------------------
enum SV_E_CODE {
  // per eliminare la compatibilità con le versioni precedenti, invece che da zero parte da 100
    eSV_NOT_CODE = 100,
    eSV_GET_BODY,   // LRESULT = HANDLE_BODY, p1 = 0, p2 = 0
    eSV_OPEN_MODAL, // LRESULT = return success, p1 = filename, p2 = 0

    // LRESULT = return success, p1 = return status (0(none), 1(event), 2(alarm), p2 = 0
    eSV_GET_ALARM_STATUS,

    // LRESULT = return success, p1 = (in) percorso con caratteri jolly,
    //                                (out) il file senza path né estensione
    // p2 = titolo finestrina
    eSV_GET_FILE,

    // se p1 è diverso da zero deve essere ritornato da eSV_GET_BODY o da eSV_GET_CURR_MODAL
    eSV_GET_CURR_MODAL,   // LRESULT = HANDLE_BODY, p1 = HANDLE_BODY = 0, p2 = 0
    // seguiranno altri codici secondo le necessità

    // viene tornata l'HWND del driver di periferica (se è presente e si tratta di un programma
    // esterno) per permettere di inviare messaggi direttamente.
    // Da usare così:
    // HWND hwnd;
    // if(SV_GetBodyRoutine(eSV_GET_HANDLE_PRPH, (LPDWORD)&hwnd, (LPDWORD)p2)) { usa hwnd }
    eSV_GET_HANDLE_PRPH,  // LRESULT = return success, p1 = return HWND driver, p2 = idPrph

    // carica in memoria la ricetta passata usando la pagina per i riferimenti ai dati
    eSV_LOAD_RECIPE, // LRESULT = return success, p1 = pageName, p2 = recipeName

    // spedisce la ricetta correntemente in memoria usando la pagina per i riferimenti ai dati
    // se p2 != NULL prima la carica in memoria
    eSV_SEND_RECIPE, // LRESULT = return success, p1 = pageName, p2 = [0 | recipeName]

    // la listbox non ha codice associato che gli dica come deve essere riempita, nè quanti
    // campi deve contenere. Mentre il riempimento non crea problemi dalla dll, l'impostazione
    // del numero di campi comporta la deallocazione e allocazione di memoria che non si può
    // fare in due moduli diversi. Usando questo codice è possibile istruire la listbox sui
    // campi da usare. Vedere la struct corrispondente
    eSV_SET_TAB_LISTBOX, // LRESULT = return success, p1 = struct, p2 = 0

    eSV_GET_DEF_TYPE_PRPH, // LRESULT = return type, p1 = idPrph, p2 = 0

    eSV_GET_BLOCK_DATA_PRPH, // LRESULT = return success, p1 = &blockInfoPrph, p2 = buffer

    eSV_RELOAD_TEXT_SET, // LRESULT = return success, p1 = HINSTANCE hdll, p2 = 0

    eSV_GET_FILE2, // LRESULT = return success, p1 = loadFileInfo, p2 = 0

    eSV_GET_PASSWORD, // LRESULT = return password, p1 = password requested, p2 = 0

    // se serve un controllo più completo, aggiungiamo anche una callback per la windowprocedure ed una per la preprocess
    // forWinProc vale uno per la windowprocedure e vale due per la preprocess
    eSV_SET_CALL_BACK_LBOX, // LRESULT = HWND della listbox o zero se fallisce, p1 = (LPDWORD)MAKEWPARAM(indice listbox, forWinProc), p2 = infoCallBackSend

    eSV_MAKE_PATH, // LRESULT = success, p1 = svMakePath*, p2 = 0. N.B. non controlla che in p1 ci sia effettivamente la giusta struttura

    eSV_SET_TAB_LISTBOX_BY_HWND, // LRESULT = return success, p1 = struct, p2 = 0

    eSV_MANAGE_DIALOG_FUNCT_KEY, // LRESULT = return success, p1 = add/rem, p2 = 0

    eSV_GET_HWND_MAIN, // LRESULT = return hwnd mainClient, p1 = 0, p2 = 0

    // usata per visualizzare il risultato di una operazione di salvataggio su file, oppure
    // per richiedere conferma alla sovrascrittura di un file
    // p1 -> (DWORD)op -> 0 = file_result, p2 = toBool(success)
    // p1 -> 1 -> richiesta, torna 1 -> ok
    eSV_SHOW_OP_FILE_RESULT, // LRESULT = null, p1 = opcode, p2 = bool success

    eSV_SET_BLOCK_DATA_PRPH, // LRESULT = return success, p1 = &blockInfoPrph, p2 = buffer
    eSV_FILL_BLOCK_DATA_PRPH, // LRESULT = return success, p1 = &blockInfoPrph, p2 = value
    };
//------------------------------------------------------------------
// routine presenti nel motore e richiamabili dalla dll
//------------------------------------------------------------------
#ifdef USE_SV_DLL
// nuova modifica, il richiamo alle routine non è più linkato tramite lib
// ma devono essere caricate con la GetProcAddress() nella dll
//------------------------------------------------------------------
// legge una stringa globale, torna 0 se non esiste
ExpImp_SV
LPCTSTR
npSV_GetGlobalString(uint id);
//------------------------------------------------------------------
    // legge una stringa specifica della pagina corrente, torna 0 se non esiste
ExpImp_SV
LPCTSTR
npSV_GetPageString(uint id);
//------------------------------------------------------------------
    // legge una stringa specifica della dll, torna 0 se non esiste
ExpImp_SV
LPCTSTR
npSV_GetLocalString(uint id, HINSTANCE hdll);
//------------------------------------------------------------------
// quando si deve leggere una variabile e subito impostarla in una operazione atomica
ExpImp_SV
SV_prfData::tResultData
npSV_GetSet(uint idPrf, pSV_prfData data);
//------------------------------------------------------------------
ExpImp_SV
SV_prfData::tResultData
npSV_Set(uint idPrf, const pSV_prfData data);
//------------------------------------------------------------------
ExpImp_SV
SV_prfData::tResultData
npSV_Get(uint idPrf, pSV_prfData data);
//------------------------------------------------------------------
//------------------------------------------------------------------
ExpImp_SV
SV_prfData::tResultData
npSV_MultiSet(uint idPrf, const pSV_prfData data, uint nData);
//------------------------------------------------------------------
ExpImp_SV
fREALDATA
npSV_GetNormaliz(uint id);
//------------------------------------------------------------------
ExpImp_SV
LRESULT
npSV_GetBodyRoutine(uint codeMsg, LPDWORD p1, LPDWORD p2);
//------------------------------------------------------------------
#endif
//------------------------------------------------------------------
// routine presenti nella dll richiamata dal motore
//------------------------------------------------------------------
// Poiché viene caricata a runtime (la dll può essere omessa) le routine
// devono avere questo nome preciso.

//enum msg4Dll { eINIT_DLL = 1, eEND_DLL };
// richiamata all'inizio e al termine con i codici eINIT_DLL e eEND_DLL
// serve per eventuali inizializzazioni-terminazioni.

//ExpImp_DLL
//void npSV_System(DWORD msg);
//------------------------------------------------------------------
//------------------------------------------------------------------
// richiamata all'interno del refresh generale (non permette il
// rientro durante l'elaborazione, viene settato un flag di
// elaborazione in corso)

//ExpImp_DLL
//void npSV_Refresh();
//------------------------------------------------------------------
//------------------------------------------------------------------
// la routine disponibile per le pagine contiene l'id della pagina ed il
// nome del file di testo della pagina.
// nella dll si esegue una verifica dell'id, se è un id generico allora
// l'identificazione della pagina viene presa dal nome, altrimenti è
// l'id stesso che identifica la pagina.
//------------------------------------------------------------------
// Il prototipo per la funzione è:

// se torna false non viene eseguita la routine base.
// force indica se è necessario un refresh forzato

// visto che si usa sempre il pageName e l'idBody non è stato mai utilizzato lo utilizziamo come
// avviso che la pagina sta per chiudersi, per eventuali azioni di salvataggio e/o pulizia in cui
// si richiede una conferma e quindi il body deve essere ancora visibile
#define ID_BODY_FOR_EXIT 10000000
// bool npSV_RefreshBody(uint idBody, LPCTSTR pageName, bool force);

//------------------------------------------------------------------
#ifdef __cplusplus
 }
#endif
//------------------------------------------------------------------
#include "restorePack.h"
#endif
