//--------P_Body.h -----------------------------------------------------------
#ifndef P_BODY_H_
#define P_BODY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEBODY_H_
  #include "p_baseBody.h"
#endif
#ifndef MAINCLIENT_H_
  #include "mainClient.h"
#endif
#include "setPack.h"
//----------------------------------------------------------------------------
#define MAX_TIMEOUT_EDIT 50

// La finestra principale mantiene il puntatore alla finestra attiva.
// All'attivazione di un'altra finestra, tornata dal metodo pushedBtn(),
// la finestra principale provvede a chiudere e deallocare le risorse per
// la finestra precedentemente attiva.

class P_Body : public P_BaseBody
{
  private:
    typedef P_BaseBody baseClass;
  public:
    P_Body(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);
    virtual ~P_Body();

    // richiamata dalla refresh(), diminuisce il conteggio per timeoutEdit
    // se viene ridefinita e si utilizza il controllo per il timeout
    // deve essere richiamata nella classe erede
    virtual void refreshBody();

    virtual void setReady(bool first);

    // richiamata ad ogni tick del timer, richiama refreshBody() e
    // disattiva il needRefresh
    // !!!! non usare, nelle classi derivate va ridefinita la refreshBody()
    virtual void refresh();

    virtual LPCTSTR getTitle(bool& needDelete);
    virtual smartPointerConstString getTitle();
    // non usa più un set globale di nomi, ma quello privato
    // torna il puntatore al proprio set e carica l'id di inizio per il menù
    setOfString& getInfo(long &id);

      // torna la finestra aperta corrispondente al pulsante premuto
      // deve tornare zero se il pulsante non è attivo o non deve aprire una window
    virtual P_Body* pushedBtn(int idBtn) = 0;

    virtual P_Body* getPreviousPage() = 0;

      // abilita o disabilita il pulsante corrispondente
    void enableBtn(int idBtn, bool enable = true);

      // spedisce un messaggio di stato nella sezione footer
    void setStatus(LPCTSTR msgStatus);

    // viene chiamata dalla client principale al termine della
    // creazione e successiva inizializzazione titolo e info pulsanti
    // può servire per disabilitare temporaneamente alcuni pulsanti
    // o altro.
    // Per default crea testi ed edit, se si ridefinisce occorre richiamarla
//    virtual void setReady();

    bool isReadOnly() { return readOnly; }
    void setReadOnly(bool set) { readOnly = set; }

//    virtual void postOpenModal(LPCTSTR name) { PostMessage(*this, WM_POST_NAME_MODAL_BODY, 0, (LPARAM) name); }
    virtual void postOpenModal(LPCTSTR name);
    virtual void postOpenModeless(LPCTSTR name, bool open);

    PVect<P_Body*> getSetModeless() { return modeless; }

    // verifica l'uso predefinito del pulsante passato in id (F1 o F12)
    // viene abilitato tramite flag nello std_msg.txt. Se attivo, il comportamento
    // è uguale alla versione precedente
    bool usePredefBtn(uint id);
    P_Body* getModal() { return Modal; }
    void destroyModal();

    virtual void request();

    bool isEnabledBtn(uint idBtn);

    void postOpenKeyboard(class PEdit* pCtrl);

    virtual void releasePress();

    void sendListValue(bool onLoad);
//    void sendListValue(uint id);

    virtual LPCTSTR getPageName() const { return 0; }
    virtual void setPageName(LPCTSTR pageDescr) {}

  protected:
    // se aperto un body modale gli invia il comando di refreshBody()
    P_Body* Modal;

    // serie di finestre non modali
    PVect<P_Body*> modeless;

    // utility per avere una controllo di accesso con livello
    typedef P_Body* (P_Body::*pfzMember)();
    P_Body* runWithPsw(pfzMember fz, int level);


    virtual void evPaint(HDC hdc, const PRect& rect);
//    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
//    virtual void evPaint(HDC hdc);

    // può essere settata dalla classe erede per modificare
    // l'id di inizio di default, per poter usare magari lo
    // stesso file se non si definiscono oggetti, ma solo il menu
    long idInitMenu;

    // usato per verificare se la pagina è stata chiamata col vecchio metodo
    // oppure con quello generico
    int ParentID;

  private:
    bool readOnly;

};
//----------------------------------------------------------------------------
// cerca e ritorna il puntatore al body corrente
P_Body* getCurrBody();
//----------------------------------------------------------------------------
extern void alignWithMonitor(PWin* owner, int& x, int& y, bool act = false);
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

