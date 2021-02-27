//----------- loadPage.h ----------------------------------------------------
#ifndef loadPage_H_
#define loadPage_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_avl.h"
#include "p_txt.h"
#include "popensave.h"
#include "p_basebody.h"
#include "resource.h"
//----------------------------------------------------------------------------
extern void getPath(LPTSTR path);
//----------------------------------------------------------------------------
class infoPages;
//----------------------------------------------------------------------------
// Serve per mantenere un posto unico per le pagine allocate, per evitare loop di allocazioni
//----------------------------------------------------------------------------
class setOfPages
{
  public:
    setOfPages() {}
    ~setOfPages();

    enum result { added, exist, failed };

    // se result != failed torna il puntatore all'oggetto (nuovo o esistente)
    // level, in caso di esistenza, viene aggiornato al livello più alto (valore minore)
    result addPage(LPCTSTR page, const infoPages*  infoParent, uint level, uint pswLevel, infoPages** target);

    uint getElem() const { return Pages.getElem(); }
    LPCTSTR getPage(uint pos) const;

    LPCTSTR getPage(LPCTSTR name) const;
    const infoPages* getPageInfo(LPCTSTR name) const;
    const infoPages* getPageInfo(uint pos) const;
    void reset() {   flushPV(Pages); }
    bool existPage(LPCTSTR page) const;
  private:
    PVect<infoPages*> Pages;
};
//----------------------------------------------------------------------------
// nessun distruttore, serve solo per poter effettuare la comparazione in PVect
// P va distrutta a mano
class myStr
{
  public:
    myStr(LPCTSTR p = 0) : P(p) {}

    LPCTSTR P;
    bool operator<(const myStr& other) const { return P && other.P ? _tcsicmp(P, other.P) < 0 : true; }
    bool operator>(const myStr& other) const { return P && other.P ? _tcsicmp(P, other.P) > 0 : false; }
};
//----------------------------------------------------------------------------
// mantiene le informazioni sui percorsi di apertura.
//----------------------------------------------------------------------------
class infoPages
{
  public:
    infoPages(LPCTSTR PageCurr, const infoPages*  info_parent, uint level, uint pswLevel);
    ~infoPages();

    void addChild(infoPages* child);
    void addParent(const infoPages* parent);

    LPCTSTR getPageCurr() const { return PageCurr; }
    const PVect<infoPages*>& getChild() const { return PageChild; }
    const PVect<const infoPages*>& getPageParent() const { return infoParent; }

    const infoPages* findChild(LPCTSTR page);

    void setLevel(uint level) { if(Level > level) Level = level; }
    uint getLevel() const { return Level; }

    void setPswLevel(uint level);
    uint getPswLevel() const { return pswLevel; }
  private:
    LPCTSTR PageCurr;
    // serve per mantenere la profondità della pagina, se durante la visualizzazione
    // viene richiesta questa pagina e questa ha un livello più alto (valore minore) non viene espansa
    // ulteriolmente
    uint Level;

    uint pswLevel;

    PVect<const infoPages*>  infoParent;
    PVect<infoPages*> PageChild;

};
/*

  Le pagine discendenti dalla pagina principale possono essere richiamate da:
    1) tasti funzione,
    2) pulsante con tipo comando 6 (apertura modeless) o periferica zero (simile ai tasti funzione),
    3)

  Specifiche caricamento variabili:
  Si apre la pagina (iniziale 'page1.txt'), si caricano tutte le variabili della pagina
  se ci sono comandi di apertura pagina (body, modal, modeless) si memorizza il nome, il tipo
  e li si aggiunge a una lista. Terminate le variabili si passa ad aprire la lista.

*/
//----------------------------------------------------------------------------
class managePages
{
  public:
    managePages(P_BaseBody* owner, LPCTSTR firstPage);

    virtual ~managePages();

    enum eAction { noAct, eOpenBody, eOpenTrend, eOpenRecipe, eOpenRecipeRow, eOpenMaint };

    bool makeAll();

    bool find(const infoPages* target);

    // gestione (deposito) pagine
    bool setFirstPage();
    bool setLastPage();
    bool setPrevPage();
    bool setNextPage();
    //
    bool getCurr(const infoPages* &target);

    // per il movimento nella navigazione si torna la radice,
    // poi va gestito manualmente
    const infoPages* getRoot() const { return Root; }

    const setOfPages& getSetPage() { return soPages; }
  private:
    P_BaseBody* Owner;
    LPCTSTR firstPage;


    // deposito pagine
    setOfPages soPages;
    // radice per albero di navigazione pagine
    infoPages* Root;

    // posizione corrente
    uint currPage;

    void addFromFunct(LPCTSTR name, infoPages* iP, uint level, uint pswLevel);
    void checkMenu(infoPages* iP, setOfString& sos, uint level);

    void addFromBtn(LPCTSTR name, infoPages* iP, uint level, uint pswLevel);
    void checkActionBtn(infoPages* iP, setOfString& sos, uint level, uint idInit);

    void checkBtn(infoPages* iP, setOfString& sos, uint level);

    void checkAllVars(infoPages* iP, setOfString& sos, uint level);

    infoPages* loadPage(LPCTSTR filename, infoPages* iP_Parent, uint level, uint pswLevel);

    setOfString sosStdMsg;
    void loadStdMsg(setOfString& sos);

    void checkRecipe(infoPages* iP, setOfString& sos);
    void checkRecipeRow(infoPages* iP, setOfString& sos);
    void checkDefaultPages(infoPages* iP, setOfString& sos);

};
//----------------------------------------------------------------------------
#endif
