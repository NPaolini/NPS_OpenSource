//-------- conn_dll.h ----------------------------------------------
#ifndef CONN_DLL_H_
#define CONN_DLL_H_
//------------------------------------------------------------------
#include "precompHeader.h"
//------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
//------------------------------------------------------------------
//------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <pshpack4.h>

#elif defined(_MSC_VER)
  #pragma pack(push, 4)

#else
  #error Occorre definire l'allineamento a 4 byte per le strutture seguenti
#endif
//------------------------------------------------------------------
DECLARE_HANDLE(HANDLE_CONN);
//------------------------------------------------------------------
typedef struct tag_ConnInfoA
{
  // tipo di connessione richiesta
  int connType;

  // vale paramConn per la Seriale, la porta per la Lan
  DWORD paramPort;

  // t1 è il valore del time (in ms) di attesa tra i comandi di inizializzazione per il modem
  // se zero viene preso il valore di default (200 ms).

  // Per la lan server contiene l'handle della window client, può essere zero e impostato
  // dopo la connessione
  UINT_PTR t1;

  // t2 è il numero di ripetizioni per un timeout interno (il time è fisso):
  // time_out(DWORD _ms = 20, DWORD rep = 100)
  // se zero viene usato il valore di default (100) (2 sec di timeout)
  DWORD t2;

  // per il modem contiene il numero di telefono, poi, separato da un
  // carattere '#' la stringa di inizializzazione. E' possibile far iniziare
  // la stringa con il carattere 'P' o 'T' per scegliere il tipo di modulazione
  // Pulse o Tone. Se non viene immesso nulla il default è Tone.
  // Per il client lan contiene l'ip nella forma standard xxx.xxx.xxx.xxx
  // oppure il nome da risolvere (es. www.google.it)
  // Per l'attesa modem basta non inserire alcun numero e far iniziare
  // la stringa direttamente con l'inizializzazione
  char data[_MAX_PATH];

  // window che riceverà i messaggi. E' possibile non utilizzarla (es. in consolle)
  // occorrerà poi eseguire un polling per verificare la presenza di dati.
  // In caso di lan server è obbligatoria poiché l'handle della connessione in
  // ingresso viene recapitata tramite messaggio
  HWND Owner;
} ConnInfoA, *pConnInfoA;
//------------------------------------------------------------------
// struttura equivalente per l'unicode
typedef struct tag_ConnInfoW
{
  int connType;
  DWORD paramPort;
  UINT_PTR t1;
  DWORD t2;
  wchar_t data[_MAX_PATH];
  HWND Owner;
} ConnInfoW, *pConnInfoW;
//------------------------------------------------------------------
typedef struct tag_BuffConn
{
  LPBYTE buff;

  // [in/out] in ingresso il numero di byte da leggere/scrivere
  // in uscita quelli effettivamente letti/scritti
  DWORD len;
  // N.B. il buffer deve essere allocato e len non deve superarne la
  // dimensione
} BuffConn, *pBuffConn;
//------------------------------------------------------------------
// finora l'unico messaggio attivo è codeMsg = HIWP_ADD_CLIENT
// msg -> HANDLE_CONN conn = (HANDLE_CONN)msg
// custom -> InfoCallBack.customData
typedef LRESULT (*prtCallBackConn)(uint codeMsg, LPVOID msg, LPVOID custom);
//------------------------------------------------------------------
typedef struct tag_infoCallBack
{
  prtCallBackConn fz_callBack;
  LPVOID customData;
}  InfoCallBack, *pInfoCallBack;
//------------------------------------------------------------------
enum ConnCode {
  ccNotCode,
  cMakeConnA,      // LRESULT = handle conn, p1 = 0, p2 = ConnInfo
  cOpenConn,      // LRESULT = operation result, p1 = 0, p2 = handle

  // chiude la connessione e distrugge l'handle che non è più valido
  cDestroyConn,   // LRESULT = operation result, p1 = 0, p2 = handle

  cGetAvail,      // LRESULT = operation result, p1 = 0, p2 = handle
  cReceiveBytes,  // LRESULT = operation result, p1 = pBuffConn, p2 = handle
  cSendBytes,     // LRESULT = operation result, p1 = pBuffConn, p2 = handle
  // per evitare di ricevere troppe notifiche di byte ricevuti, nella routine di
  // acquisizione si può disabilitare l'invio del messaggio per poi ripristinarlo
  // all'uscita
  cReqNotify,     // LRESULT = previous value, p1 = newReq(true/false), p2 = handle
  // azzera il buffer interno di ricezione
  cReset,         // LRESULT = 0, p1 = 0, p2 = handle

  // verifica se l'idCode presente in LPARAM alla ricezione di dati appartenga
  // all'handle
  cIsThis,      // LRESULT = operation result, p1 = idCode, p2 = handle
  // specifici per il modem
  // nel caso di attesa indica quando si è connessi, anche se si viene comunque
  // avvisati tramite messaggio. Torna -1 se non si è connessi tramite modem
  cIsOnLine,
   // legge dal buffer interno senza scaricarlo.
  cReceiveNoRemove,

  // cambia window che riceverà i messaggi dalla connessione, va usata quando da una
  // connessione lan server viene creata una connessione lan client e si vuole che sia
  // un'altra window a gestire la nuova connessione
  cChangeHwndOwner, // LRESULT = old HWND, p1 = new HWND, p2 = handle

  // verifica lo stato della connessione
  cIsConnected, // LRESULT = operation result, p1 = 0, p2 = handle

  // specifico per la lan
  // il buffer deve contenere due dword, la prima contiene l'ip, la seconda la porta
  cGetLocalAddrAndPort, // LRESULT = operation result, [out]p1 = buffer, p2 = handle
  // il buffer deve essere grande a sufficienza per contenere la stringa nella
  // forma: xxx.xxx.xxx.xxx:ppppp
  // torna -1 se non è una connessione lan
  cGetLocalAddrAndPortStringA, // LRESULT = operation result, [out]p1 = buffer, p2 = handle

  // validi solo per lan client
  cGetRemoteAddrAndPort, // LRESULT = operation result, [out]p1 = buffer, p2 = handle
  cGetRemoteAddrAndPortStringA, // LRESULT = operation result, [out]p1 = buffer, p2 = handle

  // specifico per la lan, p1 deve essere almeno 256 TCHAR
  cGetLocalNameA, // LRESULT = operation result, [out]p1 = buffer, p2 = handle
  cGetRemoteNameA, // LRESULT = operation result, [out]p1 = buffer, p2 = handle

  // valido solo per lan server TCP/UDP (nel caso non si usi una window per ricevere il socket client)
  // se sono presenti entrambi (callback e hwnd) viene chiamata la callback
  // N.B. la callback viene chiamata nel thread del server
  cSetCallBack, // LRESULT = operation result (1, -1), p1 = pInfoCallBack, p2 = handle

  // per quando è necessaria qualche operazione non prevista (da usare con cautela)
  cGetHandle,  // LRESULT = handle alla connessione (socket x lan, handle per seriale)

// parti differenziate per unicode
  cMakeConnW,
  cGetLocalAddrAndPortStringW,
  cGetRemoteAddrAndPortStringW,
  cGetLocalNameW,
  cGetRemoteNameW,

  // chiude la connessione senza distruggere l'handle che è ancora valido
  cCloseConn,   // LRESULT = operation result, p1 = 0, p2 = handle

   // accetta una array di valori in p1 corrispondenti al costruttore della paramConn e ne ritorna il valore come DWORD
   // i valori devono esseri gli indici dei rispettivi enum
  cFromExplicitToDWord,

  };
// in scrittura e lettura LRESULT è un booleano che indica il successo
// dell'operazione. Soprattutto in caso di lettura può tornare false se la
// richiesta supera i byte disponibili, il valore tornato in len
// è comunque sempre valido e corrisponde ai byte effettivamente letti

// N.B. In caso di connessione modem, al termine dell'apertura iniziale il buffer
//      interno viene resettato, poi tutto quel che viene ricevuto dopo va scaricato
//      a mano, soprattutto nel caso di attesa modem in cui finché non si è online
//      vengono ricevuti i vari codici dal modem (ring, ecc.)
//------------------------------------------------------------------
#ifndef UNICODE
  #define ConnInfo ConnInfoA
  #define pConnInfo pConnInfoA
  #define cMakeConn cMakeConnA
  #define cGetLocalAddrAndPortString cGetLocalAddrAndPortStringA
  #define cGetRemoteAddrAndPortString cGetRemoteAddrAndPortStringA
  #define cGetLocalName cGetLocalNameA
  #define cGetRemoteName cGetRemoteNameA
#else
  #define ConnInfo ConnInfoW
  #define pConnInfo pConnInfoW
  #define cMakeConn cMakeConnW
  #define cGetLocalAddrAndPortString cGetLocalAddrAndPortStringW
  #define cGetRemoteAddrAndPortString cGetRemoteAddrAndPortStringW
  #define cGetLocalName cGetLocalNameW
  #define cGetRemoteName cGetRemoteNameW
#endif
//------------------------------------------------------------------
enum ConnType {
  ctNotCode,
  cDirectSerial,
  cModemCall,
  cModemWait,
  cLanServer,
  cLanClient,
  cLanServerUDP,
  cLanClientUDP,
  };
//------------------------------------------------------------------
#ifdef MAKE_CONN_DLL
//------------------------------------------------------------------
CONN_IMPORT_EXPORT
LRESULT ConnProcDLL(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn);
CONN_IMPORT_EXPORT
LRESULT __stdcall ConnProcDLLstd(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn);
//------------------------------------------------------------------
#else
//------------------------------------------------------------------
typedef LRESULT  (*prtConnProcDLL)(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn);
typedef LRESULT  (__stdcall *prtConnProcDLLstd)(uint codeMsg, LPDWORD p1, HANDLE_CONN pConn);
//------------------------------------------------------------------
#endif
//------------------------------------------------------------------
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <poppack.h>

#elif defined(_MSC_VER)
  #pragma pack(pop)

#endif
//------------------------------------------------------------------
#ifdef __cplusplus
 }
#endif
//------------------------------------------------------------------
#endif
