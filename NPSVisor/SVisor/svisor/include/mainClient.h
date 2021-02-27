//----------- mainClient.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MAINCLIENT_H_
#define MAINCLIENT_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "headerMsg.h"

#ifndef P_BASE_H_
#include "p_base.h"
#endif

#ifndef P_BODY_H_
#include "p_body.h"
#endif

#ifndef GESTPRF_H_
#include "gestprf.h"
#endif

#ifndef LOG_STAT_H_
#include "log_stat.h"
#endif

#ifndef PRECT_H_
#include "prect.h"
#endif
#include "language.h"
#include "sv_dll.h"
#include "defgPerif.h"
#include "setPack.h"
//----------------------------------------------------------------------------
// messaggi riservati al supervisore
#define WM_FIRST_SVISOR     WM_FW_FIRST_FREE
#define WM_LAST_SVISOR (WM_FIRST_SVISOR + 50)
//----------------------------------------------------------------------------
//--------- MESSAGGI CORRENTEMENTE USATI -------------------------------------
//----------------------------------------------------------------------------
#define WM_MYHOTKEY (WM_FIRST_SVISOR + 1)

#define WM_MY_HOTKEY WM_MYHOTKEY
//----------------------------------------------------------------------------
// non è possibile settare il flag di dirty all'interno del ciclo
// perché al termine viene eseguita la commit() che lo azzera. Occorre
// richiamare la routine apposita postDirty() dichiarata in P_Base.
// wParam indica chi deve essere posto in Dirty:
// 0 -> body
// 1 -> gestData
#define WM_SET_DIRTY_BODY (WM_FIRST_SVISOR + 2)
//----------------------------------------------------------------------------
// usato per inviare messaggi dai pulsanti al modalBody
#define WM_POST_BTN_MODAL_BODY (WM_FIRST_SVISOR + 3)
//----------------------------------------------------------------------------
// per non far attivare la main o una modalBody owner da una modalBody child
#define WM_POST_ACTIVATE (WM_FIRST_SVISOR + 4)
//----------------------------------------------------------------------------
// per far aprire una modalBody specificando il nome (lParam punta al nome)
#define WM_POST_NAME_MODAL_BODY (WM_FIRST_SVISOR + 5)
//----------------------------------------------------------------------------
// per far aprire/chiudere una modelessBody specificando il nome
// (lParam punta al nome, wParam vale zero se deve chiudere, altro valore per open)
#define WM_POST_NAME_MODELESS_BODY (WM_FIRST_SVISOR + 6)
//----------------------------------------------------------------------------
// per aprire una nuova pagina non da tasti funzione, ma da qualsiasi pulsante
// e in qualsiasi numero, wParam punta all'id che deve essere >
#define WM_POST_OPEN_NEW_PAGE_BODY (WM_FIRST_SVISOR + 7)
//----------------------------------------------------------------------------
// in presenza di touch screen apre la finestra per l'immissione dei caratteri
// HIWORD(wParam) -> id, lParam -> svEdit*
#define WM_POST_OPEN_MODAL_KEYB (WM_FIRST_SVISOR + 8)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define SWM_TRAYMSG   (WM_FIRST_SVISOR + 9)
#define WM_GO_TO_TRAY (WM_FIRST_SVISOR + 10)
#define WM_REMOVE_FROM_TRAY (WM_FIRST_SVISOR + 11)
#define WM_SHOW_HIDE_PRPH (WM_FIRST_SVISOR + 12) // wParam -> 0 == hide, !0 == show with level
//----------------------------------------------------------------------------
#define WM_MY_VAR_LVIEW_ALARM (WM_FIRST_SVISOR + 13)
//----------------------------------------------------------------------------
// avvia un timer per il reset della password
#define WM_POST_RESET_PASSWORD (WM_FIRST_SVISOR + 14)
//----------------------------------------------------------------------------
// messaggio per avvio funzione ricetta da bit
#define WM_POST_RECIPE_BIT_COMMAND (WM_FIRST_SVISOR + 15)
//----------------------------------------------------------------------------
#define TRAYICONID  1     //  ID number for the Notify Icon
//--------- FINE MESSAGGI CORRENTEMENTE USATI --------------------------------
//----------------------------------------------------------------------------
typedef UINT uint;
//----------------------------------------------------------------------------
#define MAX_LEN_KEY_CRYPT 8
// torna true se i file di testo per le pagine sono cryptati
bool isCrypted();
// torna il codice di decriptaggio per le pagine di testo
LPCBYTE getDecryptCode();
//----------------------------------------------------------------------------
// carica in sos le stringhe, verifica se il file è criptato
void pageStrToSet(setOfString& sos, LPCTSTR filename);
//----------------------------------------------------------------------------
// aggiunge in testa al nome del file il simbolo '#' per i file che possono
// essere criptati
void addSymbFile(LPTSTR filename);
//----------------------------------------------------------------------------
// numero di dword per periferica memoria
//#define SIZE_OTHER_DATA ((4096 * 8 - sizeof(DWDATA)) / sizeof(DWDATA))
// definita in defgPerif
//----------------------------------------------------------------------------
// per modali richiamate dal gestData
#define TEST_BIT_MODAL (SIZE_OTHER_DATA - 1)
#define BIT_NEED_EXEC 0
#define BIT_ON_EXEC   1
#define BIT_ON_KEYB   2
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PVarGlobalCam
{
  public:
    PVarGlobalCam(PVarCam* cam, LPCTSTR name);
    ~PVarGlobalCam();

    LPCTSTR getCamName() { return Name; }
    PVarCam* getCam() { return Cam; }

  private:
    PVarCam* Cam;
    LPCTSTR Name;
};
//----------------------------------------------------------------------------
class infoReqRead;
//----------------------------------------------------------------------------
class mainClient : public PWin
//class mainClient : public PDialog
{
  public:
    mainClient(PWin* parent, uint resId = IDD_CLIENT, HINSTANCE hInstance = 0);
    virtual ~mainClient();

    void pushBtn(int btn);

    void addInfoTitle(LPCTSTR name);
    // dovrebbero essere nella zona private, vengono richiamate da un messaggio
    void gotoPreviousPage();
    void shutDown(bool passedPsw = false);

    // modifica il timer per il polling
    void setTimeForTimer(uint msec);
    uint getTimeForTimer() const;

   void addPerif(gestPerif *perif, bool autodelete = true);

    // la periferica può essere sia in memoria che reale
    const genericPerif *getGenPerif(uint id) const;
    genericPerif *getGenPerif(uint id);

      // usata dai vari body per la comunicazione diretta
    const perifsSet *getPerifs() const;
    const gestPerif *getPerif(uint id) const;
    perifsSet *getPerifs();
    gestPerif *getPerif(uint id);

      // per ricavare e/o modificare i dati
    const gestJobData *getData()  const;
    const ImplJobData *getIData() const;
    gestJobData *getData();
    ImplJobData *getIData();

      // per segnalare eventi dai body
//    log_status &getLog()  { return logStat; }
    log_status::errEvent setEvent(log_status::event type, const val_gen* v = 0);

    const stat_gen* getObj(log_status::eType obj) const;


    // verifica lo stato della macchina, which individua chi va aggiornato
    void checkRunOrStop(bool force = false, int which = rALL);

    // attiva/disattiva il log degli allarmi particolareggiato
    void RepAllAlarm(bool log);

    void logAlarm(int which, bool force = false);


    void setHotKey();
    void unsetHotKey();

    // forward al Footer
    void enableBtn(int idBtn, bool enable = true);
    bool isEnabledBtn(int idBtn);
    void setStatus(LPCTSTR msgStatus);
    void setNewText(int idBtn, LPCTSTR newText);

    // abilita l'elaborazione dei tasti funzione da dialoghi popup
    // richiamati dai vari body (es. finestra allarmi)
    // il body non riceve più il messaggio che viene invece inviato al dialogo,
    // ma è comunque disattivato da questo
    void enableDialogFunctionKey();
    void disableDialogFunctionKey();

    // abilita/disabilita l'uso dei tasti funzione tramite hook
    void enableFunctionKey();
    void disableFunctionKey();

    bool hasPopup();

    gestPerif::statMachine isRunning();

    void saveJob();
    void loadJob();


    virtual bool create();
    virtual bool preProcessMsg(MSG& msg);
    void CloseApp();

    void reloadText();

    PVarCam* getCam(LPCTSTR name);
    void addCam(PVarCam* cam, LPCTSTR name);
    bool isGlobalCam(LPCTSTR name);
    void hideCam();
    void showCam(PVarCam* cam);

    void showPart();
    void reloadBody();

    void addToMappedFile(uint idPrph, const PVect<DWORD>& data, bool oneReq = false);
    bool getListPrph(PVect<bool>& set);

    class PTextAutoPanel* getTimeBox() { return timeBox; }

    void show_hide_periph(bool show);

    HWND setListBoxCallBack(infoCallBackSend* fz, uint ixLB);
  protected:
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);

  private:

    class PTextAutoPanel* timeBox;
    PVect<bool> useReqPrph;
    PVect<infoReqRead*> setMappedReq;

    void sendClearListReq(uint idPrph);
    void removeMappedFile(WORD wP, DWORD lP);
    void checkMappedFile();

      // intestazione
    class Header *Hd;

      // corpo (varia a seconda del menù)
    class P_Body *Bd;
    PRect RBody; // spazio per il body

      // fondo pagina con i pulsanti
    class Footer *Ft;

      // set di periferiche
    perifsSet Perif;

      // dati vari
    gestJobData* JobData;

      // gestione log allarmi e eventi in generale
    log_status logStat;

    uint idTimer;  // polling per refresh periferiche
    uint mSecTimer; // valore attuale del timer

    int enableForwardFunctionKey;

    int enableHookFunctionKey;

// non usa la lettura del time corrente, ma un contatore basato sul tick timer
//    __int64 prevTime;

//    int dCount;

    void CloseApp(HWND hwnd);
    bool EvTimer(uint timerId);
    void EvSize(DWORD typeShow, bool force);
//    void EvSize(uint sizeType, SIZE& size, bool recalc);

    PVect<PVarGlobalCam*> GlobalCam;

    void initializeNewBody(P_Body *newBody);
    void calcCurrRes();
    typedef PWin baseClass;
//    typedef PDialog baseClass;
    DWORD oldShow;

    uint idTimerResetPsw;
};
//----------------------------------------------------------------------------
#if 1
mainClient* getMain();
#else
mainClient* getMain(PWin* win);
#endif
//----------------------------------------------------------------------------
inline uint mainClient::getTimeForTimer() const
{
  return mSecTimer;
}
//----------------------------------------------------------------------------
inline void mainClient::addPerif(gestPerif *perif, bool autodelete)
{
  Perif.add(perif, autodelete);
}
//----------------------------------------------------------------------------
// usata dai vari body per la comunicazione diretta
inline const perifsSet *mainClient::getPerifs() const
{
  return &Perif;
}
//----------------------------------------------------------------------------
inline const genericPerif* mainClient::getGenPerif(uint id) const
{
  if(PRF_MEMORY == id)
    return getData();
  return Perif.get(id);
}
//----------------------------------------------------------------------------
inline genericPerif* mainClient::getGenPerif(uint id)
{
  if(PRF_MEMORY == id)
    return getData();
  return Perif.get(id);
}
//----------------------------------------------------------------------------
inline const gestPerif *mainClient::getPerif(uint id) const
{
  return Perif.get(id);
}
//----------------------------------------------------------------------------
inline perifsSet *mainClient::getPerifs()
{
  return &Perif;
}
//----------------------------------------------------------------------------
inline gestPerif *mainClient::getPerif(uint id)
{
  return Perif.get(id);
}
//----------------------------------------------------------------------------
// per ricavare e/o modificare i dati
inline const gestJobData *mainClient::getData() const
{
  return JobData;
}
//----------------------------------------------------------------------------
// per ricavare e/o modificare i dati
inline const ImplJobData *mainClient::getIData() const
{
  return JobData->get();
}
//----------------------------------------------------------------------------
// per ricavare e/o modificare i dati
inline gestJobData *mainClient::getData()
{
  return JobData;
}
//----------------------------------------------------------------------------
// per ricavare e/o modificare i dati
inline ImplJobData *mainClient::getIData()
{
  return JobData->get();
}
//----------------------------------------------------------------------------
inline log_status::errEvent mainClient::setEvent(log_status::event type, const val_gen* v)
{
  return logStat.setEvent(type, v);
}
//----------------------------------------------------------------------------
inline const stat_gen* mainClient::getObj(log_status::eType obj) const
{
  return logStat.getObj(obj);
}
//----------------------------------------------------------------------------
inline void mainClient::logAlarm(int which, bool force)
{
  Perif.logAlarm(logStat, which, force);
}
//----------------------------------------------------------------------------
inline void mainClient::enableDialogFunctionKey()
{
  ++enableForwardFunctionKey;
}
//----------------------------------------------------------------------------
inline void mainClient::disableDialogFunctionKey()
{
  --enableForwardFunctionKey;
}
//----------------------------------------------------------------------------
inline void mainClient::enableFunctionKey()
{
  ++enableHookFunctionKey;
}
//----------------------------------------------------------------------------
inline void mainClient::disableFunctionKey()
{
  --enableHookFunctionKey;
}
//----------------------------------------------------------------------------
inline bool mainClient::hasPopup()
{
  return toBool(enableForwardFunctionKey);
}
//----------------------------------------------------------------------------
inline void mainClient::saveJob()
{
  JobData->save();
  JobData->setDirty();
}
//----------------------------------------------------------------------------
inline void mainClient::loadJob()
{
  JobData->load();
}
//----------------------------------------------------------------------------
inline void mainClient::CloseApp()
{
  if(getHandle())
    CloseApp(getHandle());
}
//----------------------------------------------------------------------------
extern "C"
{
  typedef void (*SV_SYSTEM)(DWORD msg);
  typedef void (*SV_REFRESH)();
  typedef bool (*SV_REFRESHBODY)(uint idBody, LPCTSTR pageName, bool force);
}
extern void SV_System(DWORD msg);
extern void SV_Refresh();
extern bool SV_RefreshBody(uint idBody, LPCTSTR pageName, bool force);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
extern LPCTSTR getLocalString(uint id, HINSTANCE hdll);
//----------------------------------------------------------------------------
extern WORD getVersionApp();
//----------------------------------------------------------------------------
// se occorresse derivare dalla finestra di dialogo principale
// alloca la window client principale
mainClient* allocMainClient(PWin* parent, HINSTANCE hInstance);
//----------------------------------------------------------------------------
// per modificare lo stile di default o altro.
// Viene richiamata dopo aver settato lo stile di default
void customizeMain(PWin *);
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
