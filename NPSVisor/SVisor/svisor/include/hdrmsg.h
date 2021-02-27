//------------ hdrmsg.h ---------------------------------

// header di messaggi di colloquio tra il programma principale
// e i programmi di comunicazione con le periferiche

#ifndef HDRMSG_H_
#define HDRMSG_H_

// codici comuni a tutti i supervisori
extern const UINT WM_MAIN_PRG;
extern const UINT WM_CUSTOM;

// da inserire nel codice specifico del supervisore
// tipi ammessi di periferiche

// macro per la creazione della stringa per RegisterWindowMessage()
#define RegSTR_WM_PRPH(msg, id)  \
  { \
    TCHAR buff[50]; \
    wsprintf(buff, _T("WM_PRPH_%d"), id); \
    msg = RegisterWindowMessage(buff);  \
  }
//-----------------------------------------------------------
#define MAKE_MAPPED_FILENAME(buff, wp, lp) \
    wsprintf(buff, _T("WM_MAPPED_%d_%d"), wp, lp)

// i codici riportati da RegisterWindowMessage() sono nella
// forma -> UINT WM_PRPH,  sono creati in base al
// numero della periferica e si trovano nella classe commIdentity

//-----------------------------------------------------------
// il parametro wParam è formato da LOWORD(codice),
// HIWORD(v) dipendente dal messaggio (usato quando non è sufficiente
// lParam)
//-----------------------------------------------------------
// codici di invio dalle periferiche al programma principale

// msg
//  WM_PRF_1
//  WM_PRF_2
//  WM_PRF_2

// TParam1
#define MSG_HWND                    1
  //TParam2
    // torna l'handle della window di gestione della periferica
//-----------------------------------------------------------
// LOWORD(wParam)
// avvisa il supervisore che si sta chiudendo
#define MSG_CLOSING                 2
  //HIWORD(wParam)
    // errore in inizializzazione porta di comunicazione
    #define MSG_INIT_ERR      1
    // l'applicazione si sta chiudendo non comandata dal programma principale
    #define MSG_I_AM_CLOSING  2
    // l'applicazione si sta chiudendo comandata dal programma principale
    #define MSG_OK_CLOSING    3
    // lParam -> hwnd driver
//-----------------------------------------------------------
// LOWORD(wParam)
// torna l'ok o il fallimento al termine della send o receive
#define MSG_RESPONCE                3
  // HIWORD(wParam)
  // uno dei successivi valori
#if !defined(GESTCOMM_H_)
    enum eResponce { NO, OK_READ, OK_WRITE, FAILED, WAIT_NEXT_BLOCK };
#endif
  //lParam
//-----------------------------------------------------------
// LOWORD(wParam)
// inviati da una periferica che richiede dati al supervisore e che usa
// la gestione ad eventi anziché il polling col timer.
// Per ora l'unica periferica che li usa è [WM_Trm].
// Il supervisore può ignorare i due HAS_DATA e leggere i dati dal file
// dall'interno del timer.
// Deve però provvedere a preparare i dati quando avviene la richiesta
// HIWORD(wParam) contiene informazioni supplementari (sResponce)
#define MSG_INIT_HAS_DATA           4
#define MSG_HAS_DATA                5

#define MSG_REQ_DATA                6
#define MSG_END_REQ_DATA            7
  //lParam
    // in MSG_HAS_DATA e in MSG_END_REQ_DATA torna zero se errore
    // in ricezione o trasmissione. Negli altri due viene passato Responce
//-----------------------------------------------------------
// LOWORD(wParam)
// inviata dagli azionamenti per indicare un problema con un motore.
// HIWORD(wParam) contiene il numero del motore (1 based)
#define MSG_ERROR_AZ                8
  //TParam2
      // Se in lParam il valore è 0, resetta.
      // altrimenti è l'errore
//-----------------------------------------------------------
// LOWORD(wParam)
// inviati da una periferica per indicare che i suoi dati sono cambiati.
// Per evitare al supervisore di caricare ogni volta i dati da file
#define MSG_CHANGED_DATA            9
  //lParam
      // Se lParam (ed eventualmente anche HIWORD(wParam) è sufficiente
      // a contenere il valore non c'è bisogno di usare un file di appoggio
//-----------------------------------------------------------
// LOWORD(wParam)
// inviati dalla periferica per indicare che ha terminato l'inizializzazione
// ed è pronta.
#define MSG_IS_READY               10
  //lParam
    // se diverso da zero indica che si deve forzare un dirty ad ogni
    // MSG_CHANGED_DATA anche se i dati non sono cambiati (viene usato
    // dal lettore di card, per comunicare che è stata inserita la card,
    // anche se la card è la stessa della precedente lettura)
//-----------------------------------------------------------
// LOWORD(wParam)
// inviati dal server delle periferiche ai client per indicare che si sta chiudendo
#define MSG_ENDING_SERVER          11
  //lParam
//-----------------------------------------------------------
// LOWORD(wParam)
// inviati dal client al server per richiedere l'inclusione
// nella lista dei client della periferica
#define MSG_I_AM_HERE              12
  //lParam
    // contiene l'id della periferica
//-----------------------------------------------------------
// LOWORD(wParam)
// inviato dall'esterno per forzare un save del jobFile
// message deve essere WM_PRF_1
#define MSG_SAVE_JOB               13
  //lParam
    // contiene l'id della periferica
//-----------------------------------------------------------
//-----------------------------------------------------------
// codici di invio dal programma principale alle periferiche

// msg
// WM_MAIN_PRG

//-----------------------------------------------------------
// LOWORD(wParam)
#define MSG_READ_CUSTOM             3
  // HIWORD(wParam) contiene la risposta desiderata
  //lParam
    // addr_init = LOWORD(lParam), addr_end = HIWORD(lParam)

#define MSG_WRITE_CUSTOM            4

#define MSG_WRITE_BITS              5
  // LOBYTE(HIWORD(wParam)) contiene la risposta desiderata
  // HIBYTE(HIWORD(wParam)) contiene l'offset
  //lParam
    // LOWORD(lParam) = addr,
    // LOBYTE(HIWORD(lParam)) = valori
    // HIBYTE(HIWORD(lParam)) = mask
    // è possibile usare al max un pacchetto di 8 bit

#define MSG_WRITE_SINGLE_VAR        6
  // HIWORD(wParam) contiene l'indirizzo
  // dato = (DWORD)lParam

#define MSG_WRITE_BITS_16           7
  // la risposta desiderata è stata impostata fissa ad OK_WRITE
  // LOBYTE(HIWORD(wParam)) contiene l'offset
  // HIBYTE(HIWORD(wParam)) contiene il numero di bit
  //lParam
    // LOWORD(lParam) = addr,
    // HIWORD(lParam) = valori
    // è possibile usare al max un pacchetto di 16 bit
//-----------------------------------------------------------
// LOWORD(wParam)
// per tutte le applicazioni child
// il programma principale si sta chiudendo e invia il messaggio
// alle applicazioni di comunicazione
#define MSG_MAIN_IS_CLOSING         8

  //lParam
//------------------------------------------------------------------
// LOWORD(wParam)
// inviato dal server alla periferica per evitare di inviare messaggi ad un client
// rimosso
#define MSG_REMOVE_ID               9

  //lParam
//------------------------------------------------------------------
// LOWORD(wParam)
// inviato dal svisor alla periferica per includere un nuovo client
#define MSG_ADD_ID                 10

  //lParam
//------------------------------------------------------------------
// LOWORD(wParam)
// inviato dal svisor alla periferica per abilitare/disabilitare un determinato indirizzo
#define MSG_ENABLE_DISABLE         11
  // HIWORD(wParam) -> flag di abilitazione(1) disabilitazione(0)
  // lParam -> addr da abilitare/disabilitare
//------------------------------------------------------------------
// LOWORD(wParam)
// inviato dal svisor alla periferica per abilitare la lettura di variabili che nel
// file adr hanno codice azione = 4 (la lettura dipende dalla pagina attiva)
#define MSG_ENABLE_READ            12
  // HIWORD(wParam) -> identificativo del file mappato in memoria da aprire
  // lParam -> aggiunta casuale per unicità del nome del file (è solo in memoria)

// viene anche tornato con gli stessi parametri per indicare che si è terminata
// la lettura dal file.

/*
  Specifiche di comunicazione:
    Ad ogni creazione di pagina viene inviato un messaggio con entrambi i parametri a zero
    ad indicare la pulizia ed il reset dei dati.
    Ogni successivo invio implica una aggiunta ai dati già in lettura.
    Se la periferica, tramite lettura del file .adx, non usa affatto la lettura a richiesta
    non risponde al messaggio informando così il SVisor di non volere altri messaggi.

    Quindi la periferica deve mantenere in memoria la lista delle variabili correntemente in lettura.
    Il SVisor mantiene la lista delle periferiche che non vogliono l'invio della richiesta.

*///------------------------------------------------------------------
// identico a quello sopra, ma per una singola lettura
#define MSG_ENABLE_ONE_READ        13
//------------------------------------------------------------------
// LOWORD(wParam)
// inviato dal sVisor alla periferica per visualizzare/nascondere la finestrella del driver
// in HIWORD(wParam) c'è il parametro per hide (1) o show(0)
// è possibile far avviare il driver già nascosto usando il parametro -n
#define MSG_HIDE_SHOW_DRIVER       14
//------------------------------------------------------------------
// messaggi interni nell'applicazione principale
// msg

//  WM_CUSTOM

// LOWORD(wParam)
// messaggio di tick timer inviato dalla finestra principale alle child
#define MSG_MY_TIMER    1
// messaggio per caricare font personalizzato
#define MSG_MY_SETFONT  2

// Nuove specifiche, sono stati rimossi i pulsanti di default F1 e F12

// va alla pagina precedente
#define MSG_PREV_PAGE       3
// termina il programma
#define MSG_CLOSE_PROG      4

// apre finestra treeview visualizzazione operatori/cause stop
#define MSG_SHOW_TREEVIEW   5
// apre finestra treeview stampa operatori/cause stop
#define MSG_PRINT_TREEVIEW  6

#define MSG_PRINT_SCREEN    7
#define MSG_SHOW_ALARM      8

// HIWORD(wParam) -> idEdit, lParam -> svEdit*
#define MSG_SHOW_KEYB       9


#endif
