//-------------------- gestComm.h ---------------------------------------------
#ifndef GESTCOMM_H_
#define GESTCOMM_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include "pcom_bs.h"
#ifndef PARAMCOM_H_
  #include "paramcom.h"
#endif

#define PRPH_VER 7
//-----------------------------------------------------------------------------
// messaggio inviato dalla classe alla finestra per indicare
// che ci sono dati nel buffer
#define ID_MSG_HAS_CHAR (WM_USER + 200)
// WPARAM == numero di Byte
// LPARAM == 0
//-----------------------------------------------------------------------------
#ifndef HDRMSG_H_
  enum eResponce { NO, OK_READ, OK_WRITE, FAILED, WAIT_NEXT_BLOCK };
#endif
//-----------------------------------------------------------------------------
#define MAKE_SRESP(a, b)  (static_cast<WORD>(((a) & 0xff) | ((b) << 8)))
#define HIGH_SRESP(a)     (static_cast<WORD>((a) >> 8))
#define LOW_SRESP(a)      (static_cast<WORD>((a) & 0xff))
//-----------------------------------------------------------------------------
#define MAKE_FAILED_REC   MAKE_SRESP(FAILED, gestComm::RecErr)
#define MAKE_FAILED_SEND  MAKE_SRESP(FAILED, gestComm::SendErr)
//-----------------------------------------------------------------------------
#define MAKE_OK_REC       MAKE_SRESP(OK_READ, gestComm::NoErr)
#define MAKE_OK_SEND      MAKE_SRESP(OK_WRITE, gestComm::NoErr)
//-----------------------------------------------------------------------------
#define MAKE_NO_RESP      MAKE_SRESP(NO, gestComm::NoErr)
//-----------------------------------------------------------------------------
class gestComm
{
  public:
    gestComm(LPCTSTR file);
    virtual ~gestComm();
    enum ComResult {
        // codici di errore di comunicazione
        TimeOut = 0, OpenComErr, SendErr, RecErr,
        // codici di errore di gestione file
        OpenFileErr,  ReadFileErr, WriteFileErr,

        // nessun errore
        NoErr,

        // codici di notifica eventi
        doResponce, // per notificare al supervisore l'ok dell'operazione
        StatusErr, ResetErr, // per la notifica di set/reset errori (non di comunicazione)

        // Nella ReqData viene inviato un messaggio al supervisore, tra i
        // possibili valori di ritorno non deve essere presente lo zero
        // poiché viene considerato errore. Il valore di ritorno, quando riesce
        // a contenere il dato, può essere usato per inviarlo alla periferica
        HasData, ReqData,  // per notificare al supervisore di avere/volere dati

        needMoreCycle, // notifica al gestore del ciclo di avere altre richieste da processare
        noAction, // non ha eseguito alcuna azione

        // non ha eseguito alcuna lettura dal device, ma da dati interni. Non deve gestire errori ma
        // deve comunque salvare i dati. Va alternata a noErr (quando invece si legge dal device)
        noActionButSave,

        // se è una richiesta di lettura non a ciclo continuo può succedere che la variabile non sia pronta,
        // con questo codice si dice di non cancellare la richiesta, ma di rimetterla in coda

        noActionButReload,

        // c'è stato un errore di lettura ma deve comunque salvare i dati perché il pacchetto è composto sia da dati interni che dal device.
        // è l'alternativa a noActionButSave
        onErrButSave,


        };

    virtual ComResult Init() { return NoErr; }

    virtual DWORD getLastData() { return RespData; }
    virtual WORD getLastsResponce() { return sResponce; }
    virtual DWORD getLastIdReq() { return idReq; }

    void MustClose() { Logoff = true; }
    virtual void reset() { if(Com) Com->reset(); }
    bool reqNotify(bool set) { if(Com) return Com->reqNotify(set); return false; }
    bool isDirty() { return Dirty; }

    void resetDirty() { Dirty = false; }

     // funzioni segnaposto per implementare un ciclo
     // almeno una virtuale pura per rendere la classe astratta
    virtual ComResult SendCommand() = 0;
    virtual ComResult ConfirmCommand(ComResult last) { return last; }
    virtual ComResult DoData(ComResult last)  { return last; }
    virtual ComResult write(ComResult last)   { return last; }

     // funzioni segnaposto per una gestione con colloquio col supervisore
     // per il parametro nella send(), vedere sopra in ReqData
    virtual ComResult send(DWORD)     { return gestComm::NoErr; }
    virtual ComResult receive() { return gestComm::NoErr; }

    // torna true se gestisce gli errori consecutivi
    virtual bool manageCounter(int& countSend, int& countRec, int& countTot, int& consErr) { return false; }

    virtual uint getTimerTick() const  { return 200; }
    virtual uint getMoreCycleCount() const { return 5; }

    bool needSendResponde() const { return needResponce; }

    virtual bool hasRequestVar() = 0;
  protected:
    ComResult LastErr;

    // se si chiude l'applicazione deve far si che torni successo
    // dalla Comm (ma non scriva i dati con la write())
    bool Logoff;

    // indica se i dati sono cambiati
    bool Dirty;

    // se occorre tornare un valore al chiamante
    DWORD RespData;
    // in caso di richiesta 'doResponce' il byte alto contiene
    // il 'ComResult' dell'eventuale errore, quindi il valore è
    // MAKE_SRESP(eResponce, ComResult)
    WORD sResponce;

    // codice del richiedente la comunicazione
    DWORD idReq;

    // nome del file su cui verranno scritti i dati letti dalla porta
    LPTSTR FileName;

    // comunicazione
    class PComBase *Com;

    virtual bool mustTerminate(ComResult /*last*/) { return true; }

    // aggiunta per evitare l'uscita dal ciclo in caso non si debba notificare
    // l'errore in ricezione/scrittura
    bool needResponce;
};
//------------------------------------------------------
extern gestComm *allocGestComm(void* par, uint idPrf);
//------------------------------------------------------


#endif
