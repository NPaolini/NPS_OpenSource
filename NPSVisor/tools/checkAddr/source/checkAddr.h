//----------- checkAddr.h ----------------------------------------------------
#ifndef CHECKADDR_H_
#define CHECKADDR_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "pedit.h"
#include "p_file.h"
#include "file_lck.h"
#include "p_avl.h"
#include "p_txt.h"
#include "POwnBtnImage.h"
#include "pDialog.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "popensave.h"
#include "pregistry.h"
#include "pCommonFilter.h"
#include "P_FreePacketQueue.h"
#include "common.h"
//----------------------------------------------------------------------------
#include "resource.h"
//----------------------------------------------------------------------------
#define MAX_LEVEL 20
//----------------------------------------------------------------------------
class manageObjId
{
  public:
    manageObjId(uint id, uint base_id) : Id(id), baseId(base_id) {}
    manageObjId(uint id) : Id(id), baseId(calcBase(id)) {}

    uint getId() const { return Id; }
    uint getBaseId() const { return baseId; }

    uint calcBaseExtendId(uint ix);
    uint calcAndSetExtendId(uint ix);

    uint getFirstExtendId();
    uint getSecondExtendId();
    uint getThirdExtendId();

    uint calcBase(uint id);
  private:
    uint Id;
    uint baseId;
};
//----------------------------------------------------------------------------
extern void getPath(LPTSTR path);
//----------------------------------------------------------------------------
class infoPrph;
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
    result addPage(LPCTSTR page, const infoPages*  infoParent, const infoPrph* openedBy, uint level, infoPages** target);

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
  virtual ~myStr() {}
    LPCTSTR P;
    bool operator<(const myStr& other) const { return P && other.P ? _tcsicmp(P, other.P) < 0 : true; }
    bool operator>(const myStr& other) const { return P && other.P ? _tcsicmp(P, other.P) > 0 : false; }
};
//----------------------------------------------------------------------------
// nessun distruttore, serve solo per poter effettuare la comparazione in PVect
class myStrImage : public myStr
{
  private:
  typedef myStr baseClass;
  public:
    myStrImage(LPCTSTR p = 0, LPCTSTR page = 0) : baseClass(p), Page(page) {}

    LPCTSTR Page;
  void fillBuff(LPTSTR buff) const;
};
//----------------------------------------------------------------------------
class setOfImages
{
  public:
    setOfImages() {}
    ~setOfImages();

    // se result != failed torna il puntatore all'oggetto (nuovo o esistente)
    // level, in caso di esistenza, viene aggiornato al livello più alto (valore minore)
    void addImage(LPCTSTR image, LPCTSTR page);

    uint getElem() const { return Images.getElem(); }
    const myStrImage& getImage(uint pos) const;
    bool exist(LPCTSTR image) const;
    void reset();
  private:
    PVect<myStrImage> Images;
};
//----------------------------------------------------------------------------
// mantiene le informazioni sui percorsi di apertura.
//----------------------------------------------------------------------------
class infoPages
{
  public:
    infoPages(LPCTSTR PageCurr, const infoPages*  info_parent, const infoPrph* openedBy, uint level);
    ~infoPages();

    void addChild(infoPages* child);
    void addParent(const infoPages* parent);

    LPCTSTR getPageCurr() const { return PageCurr; }
    const PVect<infoPages*>& getChild() const { return PageChild; }
    const PVect<const infoPages*>& getPageParent() const { return infoParent; }
    const infoPrph* getInfoOpenedBy()const { return openedBy; }

    const infoPages* findChild(LPCTSTR page);

    void setLevel(uint level) { if(Level > level) Level = level; }
    uint getLevel() const { return Level; }

    uint getPageType() const { return pageType; }
    void setPageType(uint newType) { pageType = newType; }

  private:
    LPCTSTR PageCurr;
    // serve per mantenere la profondità della pagina, se durante la visualizzazione
    // viene richiesta questa pagina e questa ha un livello più alto (valore minore) non viene espansa
    // ulteriolmente
    uint Level;

    uint pageType;
    PVect<const infoPages*>  infoParent;
    const infoPrph* openedBy;
    PVect<infoPages*> PageChild;

};
//----------------------------------------------------------------------------
// Contiene le informazioni sulla singola variabile
//----------------------------------------------------------------------------
#define SET_GET_V(t, v) \
  public:\
    t get##v() const { return v; }\
    void set##v(t vv) { v = vv; }\
  private:\
    t v;
/*
  SET_GET_V(uint, Prph)

  viene espansa in:

  public:
    uint getPrph() const { return Prph; }
    void setPrph(uint vv) { Prph = vv; }
  private:
    uint Prph;

*/
//----------------------------------------------------------------------------
union dualOffset
{
  struct {
    int offsAddr : 23;
    int alsoVisib:  1;
    int offsBit  :  8;
    };
  int offset;
};
//----------------------------------------------------------------------------
#define SET_pO(o, v, vv) (o)->set##v(vv)
//----------------------------------------------------------------------------
#define CAST_pIPRPH(o) ((const infoPrph*)(o))
#define GET_pO(o, v, vv) vv = CAST_pIPRPH(o)->get##v()
#define GET_pV(o, v) CAST_pIPRPH(o)->get##v()
//----------------------------------------------------------------------------
#define SET_rO(o, v, vv) (o).set##v(vv)
#define GET_rO(o, v, vv) vv = (o).get##v()
#define GET_rV(o, v) (o).get##v()
//----------------------------------------------------------------------------
#define COORD_BY_FUNCT -1000
//----------------------------------------------------------------------------
#define COORD_BY_TIMER_TREND  -2000
#define COORD_BY_ENABLE_TREND -2001
#define COORD_BY_PRINT_SCREEN -2002
#define COORD_BY_PRINT_VARS   -2003
#define COORD_BY_FILTER_ALARM -2004
#define COORD_BY_MAN_STOP     -2005

#define COORD_BY_DLL_RELOAD   -2006
#define COORD_BY_RECIPE_PAGE  -2007
#define COORD_BY_RECIPE_ROW_PAGE  -2008

#define COORD_BY_NORM_LINK_VAR -2009
#define COORD_BY_NORM_LINK_BIT -2010

#define COORD_BY_MAN_STOP_OPER -2011

#define COORD_BY_CURR_DATETIME -2012

#define COORD_BY_SAVE_SCREEN   -2013


// non definire valori tra -2013 e -2025, servono per
// indirizzare la periferica allarme a cui sono associate le variabili
#define COORD_BY_FILTER_ALARM_ASSOC -2025

#define COORD_BY_INITIALIZED   -2026
#define COORD_BY_PSW           -2027

#define COORD_BY_ASSOC_FILENAME -2028
//----------------------------------------------------------------------------
#define IMG_OK_TYPE   23
#define IMG_WARN_TYPE 22
//----------------------------------------------------------------------------
extern int getIxImageByIdObj(int ixObj);
//----------------------------------------------------------------------------
class infoPrph : public genericSet
{
  public:
    // se si rende virtuale, gli elementi vanno azzerati uno alla volta,
    // this contiene anche l'indirizzo della tabella virtuale
    infoPrph() { init(); }
    enum ePgType { noPg, eBody, eHeader, eModeless, eModal, eLink1, eLink2, eDll, eStdMsg, eFooter, eNorm, eAlarmAssoc };
    enum eObjType { noObj, eBtn, eTxt, eEdit, ePanel, eBmp, eBarGraph, eLed, eListBox,
                    eDiam, eChoose, eCurve, ePlotXY, eCam, eMeter, eXScope, eSpin, eSlider,
                    eObjAlarm, eObjTblInfo, eObjTrend,
                    eTrend = eObjTrend, ePrint, eAlarm, eManStop, eFunct, eManStopAddrOper,
                    eManStopAddrListBox, eMaxObj };
    enum eAction { noAct, eOpenBody, eOpenTrend, eOpenRecipe, eOpenRecipeRow, eOpenMaint,
                    eOpenModal, eOpenModeless, eOpenLink1, eOpenLink2, eVisibility,
                    eSendOnOpen, eSendOnClose, eActionSpin, eActionMove, eMaxAction };

    SET_GET_V(uint, Prph)
    SET_GET_V(uint, Addr)
    SET_GET_V(uint, dataType)
    SET_GET_V(uint, nBit)
    SET_GET_V(uint, Offs)

    SET_GET_V(int, OffsLink)

    SET_GET_V(LPCTSTR, Page)
    SET_GET_V(uint, pageType)
    SET_GET_V(uint, objType)
    SET_GET_V(uint, action)
    SET_GET_V(LPCTSTR, openPage)
    SET_GET_V(int, X)
    SET_GET_V(int, Y)

  public:
    int getNorm() const { return Norm; }
    void setNorm(int n) { if(n < -2) Norm = -n;  else if(n <= 2)  Norm = 0; else Norm = n; }


  private:
    void init();
/*
  protected:
    uint Prph;        // dati standard della variabile
    uint Addr;
    uint dataType;
    uint nBit;
    uint Offs;


    LPCTSTR Page;     // pagina in cui viene usata
    uint pageType;    // header, footer, body, modeless, modal, inclusa_con_offset, inclusa_con_link
    uint objType;     // oggetto che usa la variabile (edit, btn, ecc.)
    uin action:       // eventuale azione associata alla variabile
    LPCTSTR openPage; // eventuale pagina che viene aperta
    int X;            // coordinate dell'oggetto, -1000 se non ha oggetti associati (es. tasti funzione)
    int Y;            // nel caso di tasti funzione contiene il tasto (zero -> F1)
*/
    int Norm;
};
//----------------------------------------------------------------------------
class setOfPrphByPage : public P_Avl
{
  private:
    typedef P_Avl baseClass;
  public:
    setOfPrphByPage() : P_Avl(false) {}
    virtual ~setOfPrphByPage() {}
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
};
//----------------------------------------------------------------------------
class setOfPrphByPrph : public P_Avl
{
  private:
    typedef P_Avl baseClass;
  public:
    setOfPrphByPrph() : P_Avl(true) {}
    virtual ~setOfPrphByPrph() {}
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
};
//----------------------------------------------------------------------------
class setOfPrphOther : public P_Avl
{
  private:
    typedef P_Avl baseClass;
  public:
    setOfPrphOther() : P_Avl(false) {}
    virtual ~setOfPrphOther() {}
    virtual bool Add(infoPrph* prph);
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
};
//----------------------------------------------------------------------------
class setOfPrphNorm : public P_Avl
{
  private:
    typedef P_Avl baseClass;
  public:
    setOfPrphNorm() : P_Avl(false) {}
    virtual ~setOfPrphNorm() {}
    virtual bool Add(infoPrph* prph);
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
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

  Caricamento variabili:
    carica set,
    controlla tasti funzione
    per ciascun tipo di oggetto
      controlla le variabili associate
*/
//----------------------------------------------------------------------------
#define MAX_NORMAL 256
#define MAX_PRPH    20
//----------------------------------------------------------------------------
class setOfPrph
{
  private:
    setOfPrph() { ZeroMemory(Norm, sizeof(Norm)); ZeroMemory(Prph, sizeof(Prph)); }
//  public:
    void add(infoPrph* prph)
    {
      sPbPage.Add(prph);
      sPbPrph.Add(prph);
      sPOther.Add(prph);
      sPNorm.Add(prph);
      uint v = GET_pV(prph, Norm);
      if(v < MAX_NORMAL)
        Norm[v] = true;
      v = GET_pV(prph, Prph);
      if(v < MAX_PRPH)
        Prph[v] = true;
    }
    void reset() {
      sPbPage.Flush(); sPbPrph.Flush(); sPOther.Flush(); sPNorm.Flush();
      ZeroMemory(Norm, sizeof(Norm)); ZeroMemory(Prph, sizeof(Prph));
      }
  private:
    setOfPrphByPage sPbPage;
    setOfPrphByPrph sPbPrph;
    setOfPrphOther  sPOther;
    setOfPrphNorm   sPNorm;
    bool Norm[MAX_NORMAL];
    bool Prph[MAX_PRPH];

    friend class managePages;
};
//----------------------------------------------------------------------------
class wrapText
{
  public:
    TCHAR txt[_MAX_PATH];
};
//----------------------------------------------------------------------------
class managePages;
//----------------------------------------------------------------------------
typedef P_FreePacketQueue<wrapText, 10> infoPacket;
//----------------------------------------------------------------------------
class infoDialog : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    infoDialog(PWin* parent, managePages* owner);
    ~infoDialog() { destroy(); }
    virtual bool create();
    void setTextInfo(wrapText* txt);
    void finished(bool success);
    managePages* getOwner() { return Owner; }
    wrapText* getFree();
    void releaseFree(wrapText* txt);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void showText();
  private:
    managePages* Owner;
    infoPacket freeTxt;
};
//----------------------------------------------------------------------------
class managePages
{
  public:
    managePages(LPCTSTR path, LPCTSTR firstPage);

    virtual ~managePages();

    bool makeAll(PWin* owner);

    // gestione variabili
    enum order { no, byPrph, byPages, byOther, byNorm };
    void setOrder(order which) { currOrder = which; }
    order getOrder() const { return currOrder; }

    const bool* getListPrph() const { return soPrph.Prph; }
    const bool* getListNorm() const { return soPrph.Norm; }

    bool setFirstPrph();
    bool setLastPrph();
    bool setPrevPrph();
    bool setNextPrph();

    bool find(const infoPrph* target);
    bool find(const infoPages* target);

    // gestione (deposito) pagine
    bool setFirstPage();
    bool setLastPage();
    bool setPrevPage();
    bool setNextPage();
    //
    bool getCurr(const infoPrph* &target);
    bool getCurr(const infoPages* &target);

    // per il movimento nella navigazione si torna la radice,
    // poi va gestito manualmente
    const infoPages* getRoot() const { return Root; }

    const infoPages* getHeader() const { return Header; }

    LPCTSTR getPath() const { return Path; }

    uint getIdVarScript(LPCTSTR varName);
    LPCTSTR getVarNameScript(uint id);

    const setOfPages& getSetPage() { return soPages; }
    const setOfImages& getSetImage() { return soImages; }
    const PVect<myStr>& getStdPage() { return stdPages; }
  private:
    LPCTSTR Path;
    LPCTSTR firstPage;

    uint realPlc;

    TCHAR oldPath[_MAX_PATH];
    // deposito pagine
    setOfPages soPages;
    // deposito periferiche con doppio ordinamento
    setOfPrph  soPrph;
    // ordinamento corrente
    order currOrder;
    // radice per albero di navigazione pagine
    infoPages* Root;
    infoPages* Header;

    setOfImages soImages;

    // per individuare se si sta navigando sui body (0) o sull'header (1)
    infoPrph::ePgType defPageType;
    // posizione corrente
    uint currPage;

    // mantiene la lista dei file di testo usati dallo std_msg
    PVect<myStr> stdPages;

    int use64bit4Alarmfilter;

    void addGlobalPrph(infoPrph* prph);
    void addFromFunct(LPCTSTR name, infoPages* iP, infoPrph::eAction action, uint level, int x, int y, infoPrph::eObjType obj);
    infoPrph* addInfoPrphByVars(infoPages* iP, setOfString& sos, uint idBase, uint ix, infoPrph::eObjType obj, uint pgType);
    infoPrph* addInfoPrphByAltText(infoPages* iP, setOfString& sos, uint idBase, uint ix, infoPrph::eObjType obj, uint pgType, int offsAltText);

    infoPrph* addInfoPrphByCurve(infoPages* iP, setOfString& sos, uint ix, uint pgType);
    infoPrph* addInfoPrphByXScope(infoPages* iP, setOfString& sos, uint ix, uint pgType);
    infoPrph* addInfoPrphByObjAlarm(infoPages* iP, setOfString& sos, uint ix, uint pgType);
    infoPrph* addInfoPrphByObjTrend(infoPages* iP, setOfString& sos, uint ix, uint pgType);
    infoPrph* addInfoPrphBySpin(infoPages* iP, setOfString& sos, uint ix, uint pgType);
    infoPrph* addInfoPrphByObjTblInfo(infoPages* iP, setOfString& sos, uint ix, uint pgType);

    void checkMenu(infoPages* iP, setOfString& sos, uint level);
    void checkLinked(infoPages* iP, setOfString& sos, uint level);

    infoPrph* addSingleObj(setOfString& sos, LPCTSTR p1, LPCTSTR p2, infoPages* iP, uint obj, int x, int y, uint nVar = 1);
    void addVisibility(LPCTSTR p, infoPages* iP, uint obj, int x, int y);
    void addMovement(setOfString& sos, LPCTSTR p, infoPages* iP, uint obj, int x, int y);
    void addSingleObjByMov(LPCTSTR p1, LPCTSTR p2, infoPages* iP, uint obj, int x, int y);

    void checkSimple(infoPages* iP, setOfString& sos, uint idGroup, uint idInit, infoPrph::eObjType obj);
    void checkVars(infoPages* iP, setOfString& sos, uint idGroup, uint idBase, infoPrph::eObjType obj);
    void checkVarsAlt(infoPages* iP, setOfString& sos, uint idGroup, uint idBase, infoPrph::eObjType obj, int offsAltText);

    void checkModelessByBtn(infoPages* iP, setOfString& sos, uint level, uint idBase, uint ix, uint pgType);
    void addFromBtn(LPCTSTR name, infoPages* iP, infoPrph::eAction action, uint level, int x, int y);
    void checkActionBtn(infoPages* iP, setOfString& sos, uint level, uint idBase, uint ix);



    void checkBmp(infoPages* iP, setOfString& sos);
    void checkBarGraph(infoPages* iP, setOfString& sos);
    void checkPanel(infoPages* iP, setOfString& sos);
    void checkEdit(infoPages* iP, setOfString& sos);
    void checkTxt(infoPages* iP, setOfString& sos);
    void checkSimpleTxt(infoPages* iP, setOfString& sos);
    void checkBtn(infoPages* iP, setOfString& sos, uint level);
    void checkChoise(infoPages* iP, setOfString& sos);

    void checkLed(infoPages* iP, setOfString& sos);
    infoPrph* addInfoPrphByVarsLed(infoPages* iP, setOfString& sos, uint ix, uint pgType);

    void checkDiam(infoPages* iP, setOfString& sos);
    void checkCurve(infoPages* iP, setOfString& sos);
    void checkXMeter(infoPages* iP, setOfString& sos);
    void checkXScope(infoPages* iP, setOfString& sos);
    void checkCam(infoPages* iP, setOfString& sos);
    void checkSpin(infoPages* iP, setOfString& sos);
    void checkSlider(infoPages* iP, setOfString& sos);

    void checkObjAlarm(infoPages* iP, setOfString& sos);
    void checkObjTrend(infoPages* iP, setOfString& sos);
    void checkObjTnlInfo(infoPages* iP, setOfString& sos);
    void checkPlotXY(infoPages* iP, setOfString& sos);

    uint getNumFieldFromTrendByName(LPCTSTR trendName);

    void checkAllVars(infoPages* iP, setOfString& sos, uint level);

    infoPages* loadPage(LPCTSTR filename, infoPages* iP_Parent, infoPrph* openedBy, uint level);

    void checkBitmap(infoPages* iP, setOfString& sos, LPCTSTR page);

    setOfString sosStdMsg;
    void loadStdMsg(setOfString& sos);
    void loadHeader(const setOfString& sos);

    void loadStdMsgFilterAlarm(const setOfString& sos);
    void loadStdMsgAlarmGroup(const setOfString& sos);
    void loadStdMsgLineStat(const setOfString& sos);
    void loadStdMsgPrintScreen(const setOfString& sos);
    void loadStdMsgSaveScreen(const setOfString& sos);
    void loadStdMsgPrintVars(const setOfString& sos);
    void loadStdMsgTrend(const setOfString& sos);
    void loadStdMsgManageStop(const setOfString& sos);
    void loadStdMsgDll(const setOfString& sos);
    void loadStdMsgCurrDateTime(const setOfString& sos);
    void loadStdMsgInitialized(const setOfString& sos);
    void loadStdMsgSystray(const setOfString& sos);
    void loadStdMsgPswManage(const setOfString& sos);
    bool makeNameDllText(LPTSTR path, LPCTSTR p);
    bool loadRowVals(LPCTSTR row,infoPages* iP, bool& insideVars);
    void performLoadExpressScript(LPCTSTR filescript, infoPages* iP);
    void performLoadExpressDll(LPCTSTR path, LPCTSTR name);
    void performLoadGenericDll(LPCTSTR path, LPCTSTR name);

    void checkRecipe(infoPages* iP, setOfString& sos);
    void checkRecipeRow(infoPages* iP, setOfString& sos);
    void checkDefaultPages(infoPages* iP, setOfString& sos);

    struct scriptMan
    {
      uint id;
      LPCTSTR name;

      scriptMan() : id(0), name(0) {}
      ~scriptMan() { delete []name; }
    };

    PVect<scriptMan*> setScript;

    void loadNorm();

    void loadReserved(uint idPrph);

    void addAlarmAssoc(int prphAl);

    LPCTSTR performLoadSendOnOpen_Close(infoPages* iP, LPCTSTR p, infoPrph::eAction act, LPCTSTR txt);
    void loadSendOnOpen_Close(infoPages* iP, const setOfString& sos);

    friend unsigned FAR PASCAL LoadAllProc(void*);

    bool thPerformMakeAll();
    infoDialog* Dlg;
};
//----------------------------------------------------------------------------
#endif
