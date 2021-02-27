//--------P_BaseBody.h -----------------------------------------------------------
#ifndef P_BASEBODY_H_
#define P_BASEBODY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASE_H_
  #include "p_base.h"
#endif

#ifndef PPANEL_H_
  #include "ppanel.h"
#endif
#ifndef P_VECT_H_
  #include "p_vect.h"
#endif
#ifndef P_TXT_H_
  #include "p_txt.h"
#endif
#ifndef POWNBTNSELECT_H_
  #include "POwnBtnSelect.h"
#endif

#ifndef PALLOBJ_F_D_H_
  #include "pAllObj_f_d.h"
#endif
#include "sv_dll.h"

#include "setPack.h"
//----------------------------------------------------------------------------
#define ID_TITLE 1
#define ID_INIT_MENU (ID_TITLE + 1)
//#define ID_INIT_MENU 11
#define ID_INIT_ACTION_MENU (ID_INIT_MENU + 12)

#define ID_DEF_SEND_EDIT (ID_INIT_ACTION_MENU + 12)


#define ID_SHOW_SIMPLE_SEND 27

#define ID_PAGE_DYNAMIC_LINK 28
#define ID_PAGE_DYNAMIC_LINK_BITS_OFFS 97

#define ID_PAGE_DYNAMIC_LINK_V2 29

#define ID_OFFSET_X 30
#define ID_OFFSET_Y 31

#define ID_VERSION  32

#define ID_MODAL_RECT   33
#define ID_MODELESS_MEMO_POS 34

#define ID_BMP            35
#define ID_BARGRAPH       36
#define ID_SIMPLE_PANEL   40
#define ID_SIMPLE_TXT     41
#define ID_VAR_TXT        42
#define ID_VAR_EDI        43
#define ID_VAR_BTN        44
#define ID_VAR_BMP        45
#define ID_VAR_LED        46
#define ID_VAR_DIAM       47
#define ID_VAR_LBOX       48
#define ID_VAR_CHOOSE     49
#define ID_VAR_CURVE      50
#define ID_VAR_X_METER    51
#define ID_VAR_CAM        52
#define ID_VAR_SCOPE      53
#define ID_VAR_ALARM      54
#define ID_VAR_TREND      55
#define ID_VAR_SPIN_UPDN  56
#define ID_VAR_TABLE_INFO 57
#define ID_VAR_PLOT_XY    58
#define ID_VAR_SLIDER     59

#define ID_INIT_BMP_4_BTN  151

//#define ID_INIT_BMP_4_VAR  251
#define ID_INIT_BMP_4_VAR  ID_INIT_BMP_4_BTN

// è stato scritto sopra, lo riscrivo qui perché potrebbe essere riallocato per errore non vedendolo
//#define ID_PAGE_DYNAMIC_LINK_BITS_OFFS 97
#define ID_NO_CAPTION_MODAL 98

#define ID_RESOLUTION 99
#define ID_CODE_BTN_FZ_BEHA 999998
#define ID_CODE_PAGE_TYPE 999999

#define ID_SEND_VALUE_ON_OPEN  999996
#define ID_SEND_VALUE_ON_CLOSE 999997

#define ID_INIT_FONT 100
#define MAX_FONT 50

#define ID_INIT_CAM               480

#define ID_INIT_BMP               501
#define ID_INIT_BARGRAPH          601
#define ID_INIT_SIMPLE_PANEL     1001
#define ID_INIT_SIMPLE_TXT       2001
#define ID_INIT_VAR_TXT          3001
#define ID_INIT_VAR_EDI          4001
#define ID_INIT_VAR_BTN          5001
#define ID_INIT_VAR_BMP          6001
#define ID_INIT_VAR_LED          7001
#define ID_INIT_VAR_DIAM         8001
#define ID_INIT_VAR_LBOX         9001
#define ID_INIT_VAR_CHOOSE      10001
#define ID_INIT_VAR_CURVE       11001
#define ID_INIT_VAR_X_METER     12001
#define ID_INIT_VAR_SCOPE       13001
#define ID_INIT_VAR_ALARM       14001
#define ID_INIT_VAR_TREND       15001
#define ID_INIT_VAR_SPIN_UPDN   16001
#define ID_INIT_VAR_TABLE_INFO  17001
#define ID_INIT_VAR_PLOT_XY     18001
#define ID_INIT_VAR_SLIDER      19001

#define ID_INIT_OBJ_LAST    ID_INIT_VAR_SLIDER

#define MAX_OBJ 22
//----------------------------------------------------------------------------
// usato per i testi semplici
#define ADD_MIDDLE_VAR        400
//----------------------------------------------------------------------------
#define ADD_INIT_VAR          300
#define ADD_INIT_SECOND       300
#define ADD_INIT_BITFIELD     ADD_INIT_SECOND
//----------------------------------------------------------------------------
#define OFFS_INIT_VAR_EXT     250000
#define ADD_INIT_VAR_EXT      2500
#define ADD_INIT_SECOND_EXT   2500
#define ADD_INIT_THIRD_EXT    ADD_INIT_SECOND_EXT
//----------------------------------------------------------------------------
#define ADD_INIT_SIMPLE_BMP 200000
//----------------------------------------------------------------------------
#define OFFS_INIT_SIMPLE_BMP (ADD_INIT_SIMPLE_BMP - 50000)

#define OFFSET_BTN_OPEN_MODELES  (ADD_INIT_SIMPLE_BMP + 1000)
#define MAX_NORMAL_OPEN_MODELESS (ID_INIT_VAR_BTN + 99)
//----------------------------------------------------------------------------
// range di utilizzo normale dei barGraph
#define GRAPH_VISIBILITY_OFFS 200

// nel OFFSET_BTN_OPEN_MODELES viene aggiunto anche l'id, quindi il valore reale
// diventa OFFSET_BTN_OPEN_MODELES + ID_INIT_VAR_BTN.
// qui lo usiamo come base e quindi è effettivo
#define ADD_INIT_GRAPH       (ADD_INIT_SIMPLE_BMP + 1000)
#define GRAPH_VISIBILITY_OFFS_EXT 2000
//----------------------------------------------------------------------------
#define PRF_MEMORY  1
#define PRF_PLC     2
//----------------------------------------------------------------------------
#define ID_OPEN_PAGE_BY_CHOOSE 119999
//----------------------------------------------------------------------------
enum fBmp {
  bUSE_COORD,
  bTILED,
  bCENTERED,
  bSCALED,
  bFILLED,
  };
//----------------------------------------------------------------------------
struct infoVarReq
{
  uint idBase;
  uint idList;
  uint offs;
};
//----------------------------------------------------------------------------
extern void getFileStrCheckRemote(LPTSTR target, LPCTSTR file);
extern LPCTSTR insertIdBmp(LPCTSTR p, PVect<uint>& target, int numBmp);
extern LPCTSTR getColor(LPCTSTR p, PVect<COLORREF>& col, int num = 2);
extern LPCTSTR insertId(LPCTSTR p, PVect<uint>& target, int num);
extern LPCTSTR getFont(LPCTSTR p, uint& id);
//----------------------------------------------------------------------------
// per default dopo circa trenta secondi annulla l'edit e rilegge i dati
#define DEFAULT_TIMEOUT_EDIT 30

// La finestra principale mantiene il puntatore alla finestra attiva.
// All'attivazione di un'altra finestra, tornata dal metodo pushedBtn(),
// la finestra principale provvede a chiudere e deallocare le risorse per
// la finestra precedentemente attiva.
//----------------------------------------------------------------------------
class P_BaseBody : public P_Base
{
  private:
    typedef P_Base baseClass;

  public:
    P_BaseBody(PWin* parent, uint resId, HINSTANCE hInstance = 0);
    virtual ~P_BaseBody();

    virtual void refresh();

    // viene chiamata dalla client principale al termine della
    // creazione e successiva inizializzazione titolo e info pulsanti
    // può servire per disabilitare temporaneamente alcuni pulsanti
    // o altro.
    // Crea tutti gli oggetti, se si ridefinisce occorre richiamarla
    virtual void setReady(bool first);

    PVarText* getVarFromId(uint id);
    PVarEdit* getEditFromId(uint id);
    PVarListBox* getLBoxFromId(uint id);

    // richiamate all'interno del metodo di azione del pulsante.

    // Dopo aver verificato il pulsante ma prima di caricare la variabile
    virtual bool preActionBtn(PVarBtn*) { return true; }
    // Dopo aver caricato la variabile ma prima di inviare
    virtual bool preSendBtn(class baseActive*, prfData&) { return true; }
    // Richiamato come ultima azione
    virtual void postSendBtn(class baseActive*, prfData&) {  }

    // NO_SEND blocca l'invio dei dati
    // YES_SEND permette l'invio dei dati
    // WAIT_SEND permette la modifica dei dati a video ma non invia
    enum preSend { NO_SEND, YES_SEND, WAIT_SEND };
    // richiamate all'interno del metodo di azione dell'edit.
    // Si comportano allo stesso modo dei pulsanti
    virtual preSend preSendEdit(PVarEdit*, prfData&) { return YES_SEND; }
    virtual void postSendEdit(PVarEdit*) {  }

    virtual void preUpdateGraph(PVarGraph* /*graph*/, int& /*perc*/) { }
    // ignora controllo perif se -1
    bool getData(prfData& target, PVarEdit* edi, int perif = -1);
    // se tipo bit deve aggiustarlo prima di spedirlo
//    bool adjuctValToBitType(prfData& data, PVarEdit* edi);

    virtual bool sendData(const prfData& data, uint perif);

    // richiesta per la prima volta del body, il setReady() avviene prima che siano
    // pronte le periferiche
    virtual void request();

    LPCTSTR getPageString(uint id);
    void replacePageString(uint id, LPCTSTR newText);

//    LPCTSTR getPageString(uint id, bool* needDelete = 0);
    smartPointerConstString getPageStringLang(uint id);
    PBitmap* getBmp4Btn(uint ix);
    const PVect<PBitmap*>& getBmp4Btn();

    #define getBmp4Var getBmp4Btn

    void resetTimeEdit() { timeoutEdit = 0; }

    void reloadText();

    WORD getVersionBody();

    virtual bool checkObjectPsw(uint id, bool* changed = 0);

    // se qualche pagina predefinita gestisce a modo suo alcuni campi può filtrare
    // gli eventuali errori per periferiche o dati non gestiti a livello generale
    virtual void ShowErrorData(uint idprf, const prfData& data, prfData::tResultData result)
    { data.showMessageError(this, idprf, result); }

    void resetCache();

    HFONT getFont(uint id);

    const POINT& getOffset() const { return Offset; }

    genericPerif* getGenPerif(uint Prph);

    // richiede il nome del file da passare alla setOfString
    virtual void getFileStr(LPTSTR path) = 0;

    virtual void requestOneRead(uint prph, uint startAddr, uint num);
    void removeAllBmpWork(uint bits);

    void simulActionBtn(uint idBtn) { actionBtn(idBtn, false); }
    bool showWhileSend() const { return !toBool(notShowWhileSimpleSend); }
    bool isDirtyBkg() const { return DirtyBkg; }
    void setDirtyBkg() { DirtyBkg = true; }

    void removeFromClipped(const PRect& r);
    bool addToClipped(const PRect& r);
    virtual LPCTSTR getPageName() const { return 0; }

    HWND setListBoxCallBack(infoCallBackSend* fz, uint ixLB);

    bool isBodyReady() { return isReady; }

  protected:

    bool preProcessMsg(MSG& msg);
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void evPaint(HDC hdc, const PRect& rect);
    virtual void evBkgPaint(HDC hdc, const PRect& rect);

    void evPaintSimple(HDC hdc, const PRect& rect);
    void evPaint2(HDC hdc, const PRect& rect);

    // torna false se va ricaricato, true se i dati non sono più vecchi della pagina
    virtual bool getCacheFile(LPTSTR target, uint sz) = 0;

     // inizia il timer in secondi per l'editing, trascorso tale tempo si annulla
     // l'editing e si rilegge dalla periferica il valore
    void initTimeEdit(int sec = DEFAULT_TIMEOUT_EDIT);
    bool mustResetEdit() { return 0 == timeoutEdit; }

    // stringhe pertinenti della pagina, non è possibile caricarla
    // nel costruttore in quanto mancante del nome del file,
    // lo si farà nella setReady() o nel costruttore della derivata
    setOfString sStr;

    // viene richiamata per riempire sStr, verifica se il file è criptato
    void fillPageStr(LPCTSTR filename);

    PVect<PVarText*> Txt;
    PVect<PVarEdit*> Edi;
    PVect<PBitmap*> BmpForBtn;
    PVect<PVarBtn*> Btn;
    PVect<PVarDiam*> Diam;
    PVect<PVarListBox*> LBox;
    PVect<PVarChoose*> Choose;
    PVect<PVarCurve*> Curve;
    PVect<PVarCam*> Cam;
    PVect<PVarXMeter*> XMeter;
    PVect<baseSimple*> Simple;
    PVect<baseSimple*> SimpleWithHide;
    PVect<PVarBmp*> Bmp;
    PVect<PVarLed*> Led;
    PVect<PVarGraph*> barGraph;
    PVect<PVarScope*> Scope;
    PVect<pVarListAlarm*> oAlarm;
    PVect<PVarTrend*> oTrend;
    PVect<PVarSpin*> oSpin;
    PVect<pVarTableInfo*> oTableInfo;
    PVect<PVarPlotXY*> PlotXY;
    PVect<PVarSlider*> Slider;

    PVect<HFONT> Font;

//    PVect<uint> idObjCurr[MAX_OBJ];

    void manageObjLink(LPCTSTR p);

    POINT Offset;

    void makeEdi();
    void makeBtn();
    void makeTxt();
    void makeVarTxt();
    void makeBar();
    void makeSimple();
    void makePanel();
    void makeSimpleTxt();
    void makeSimpleBmp();
    void makeBitmap();
    void makeLed();

    void makeDiam();
    void makeLBox();
    void makeChoose();
    void makeCurve();
    void makeCam();

    void makeXMeter();
    void makeScope();
    void makeAlarm();
    void makeTrend();
    void makeSpin();
    void makeTableInfo();
    void makePlotXY();
    void makeSlider();

    virtual PVarListBox* allocLBox(LPCTSTR p, int id);
    virtual PVarListBox* allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen = 255, HINSTANCE hinst = 0);

    void allocFont();

    void actionBtn(uint idBtn, bool toggle);
    void actionSpin(uint idSpin, int up);

    bool forwardKeyToBtn(DWORD where, HWND hbtn);
    void actionChoose(uint idBtn);

    virtual void handleLBFocus(HWND hwndCtrl, bool set, HDC hDc);
    virtual void handleLBChange(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void makeListReqRead(uint idPrph, PVect<DWORD>& set, const infoVarReq& ir);
    void addCamToReq(uint idPrph, PVect<DWORD>& set);
    void makeListReqRead(uint idPrph, PVect<DWORD>& set);
    void makeListReqRead();

    void makeListReqRead(PVect<DWORD>& set, P_Bits& bits);
    void makeListReqRead(PVect<P_Bits*>& allBits);

    setOfString BodyCache;

    LPCTSTR getStringOrIdByLang(uint id, bool& needDelete);
    void setWindowTextByLang(HWND hwnd, uint id);
    uint msgBoxByLang(PWin* win, uint idMsg, uint idTitle, uint flag);
    smartPointerConstString getStringOrIdByLang(uint id);
    LPCTSTR getStringByLang(uint id, bool& needDelete);
    smartPointerConstString getStringByLang(uint id);

    LPCTSTR sendValue(LPCTSTR p);
  private:
    int timeoutEdit; // usata per annullare l'editing dopo un certo tempo

    int notShowWhileSimpleSend;


    HBITMAP hBmpWork;
    HDC mdcWork;
    HGDIOBJ oldObjWork;

    bool isReady;

    HBITMAP hBmpBkgWork;
    HDC mdcBkgWork;
    HGDIOBJ oldObjBkgWork;

    bool allocBmpWork(HDC hdc);
    bool allocBmpBkgWork(HDC hdc);

    void flushAll();

    bool DirtyBkg;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

