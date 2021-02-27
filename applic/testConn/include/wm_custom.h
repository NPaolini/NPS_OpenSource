//----------- wm_custom.h ----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef WM_CUSTOM_H_
#define WM_CUSTOM_H_
//----------------------------------------------------------------------------
#define REG_WM(a) const UINT WM_##a = RegisterWindowMessage(_T("WM_") _T(#a))
//----------------------------------------------------------------------------
extern const UINT WM_CUSTOM_MSG_CONNECTION;
//----------------------------------------------------------------------------
// messaggio comune per tutte le connessioni
// HIWORD(WPARAM) == numero di Byte
// LPARAM == idCom che ha inviato il messaggio (* non va usato)
#define CM_MSG_HAS_CHAR          1
//----------------------------------------------------------------------------
// messaggi specifici del tipo di comunicazione
// se HIWORD(WPARAM) == 0 indica l'uscita dal thread di comunicazione
// altrimenti, dove specificato, l'evento occorso

#define CM_BY_SERIAL_DIRECT      2

#define CM_BY_MODEM              3
    #define ID_MODEM_CONNECTED 1
    #define ID_MODEM_ERR_RLSD  3
    #define ID_MODEM_TIMEOUT   8

#define CM_BY_LAN_CLIENT         4

#define CM_BY_LAN_SERVER         5
    // LPARAM = handle client
    #define HIWP_ADD_CLIENT   1
//----------------------------------------------------------------------------
// * è un codice interno e può servire solo se si abbinano più connessioni alla
//   stessa hwnd per distinguere la connessione
//----------------------------------------------------------------------------
#endif
