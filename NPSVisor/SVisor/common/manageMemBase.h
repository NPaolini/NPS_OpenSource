//---------- manageMemBase.h --------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MANAGEMEMBASE_H_
#define MANAGEMEMBASE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "gestPrf.h"
#include "log_ord.h"
#include "P_Bits.h"
#include "PCrt_lck.h"
#include "p_vect.h"
#include "mainClient.h"
#include "p_mappedfile.h"
//-------------------------------------------------------------
// attualmente 8191 variabili
// spostata in mainClient.h
//#define SIZE_OTHER_DATA ((4096 * 4 - sizeof(DWDATA)) / sizeof(DWDATA))
//-------------------------------------------------------------
bool okOnEmerg(mainClient *par);
//------------------------------------------------------------------
class retrieveValue;
//------------------------------------------------------------------
class manageValue
{
  public:
    manageValue(mainClient* owner, uint id);
    ~manageValue();
    uint getNumValue() const { return Set.getElem(); }
    void fillData(fREALDATA* buff, bool onQuit);
  private:
    PVect<retrieveValue*> Set;
    mainClient* Owner;
};
//-------------------------------------------------------------
struct job_file
{
  DWDATA data[SIZE_OTHER_DATA];
};
//-------------------------------------------------------------
class ImplJobMemBase
{
  public:
    ImplJobMemBase(mainClient *parent);
    virtual ~ImplJobMemBase();
    virtual void init();
    virtual void end();
    // notifica ad ogni tick del timer
    virtual void notify();

    // usare questa anziché notify()
    virtual void derivedNotify() {}

    // notifica per effettuare un salvataggio di tutti i dati di log
    // esclusi quelli standard
    virtual void notifySaveLog(bool onQuit = false);
    virtual void commit();

     // gestisce i cambiamenti nell'ordine attivo
     // accetta solo oSuspended, oEnded, oSave
    virtual log_status::errEvent chgOrd(ordStat cause);

//    void save(const job_file& newJob);
    virtual void save();
    virtual void load();
//    job_file &get();

    virtual void setDirty() { StatusBits.U.b.needRefresh = true; }

    // gestione manutenzioni
//    IMaint *getMaint() { return G_M; }

    prfData::tResultData get(prfData& target) const;
    prfData::tResultData getset(prfData& data, bool noCommit = false);
    prfData::tResultData set(const prfData& data, bool noCommit = false);

    void checkTimerTrend();
    bool isReadOnly() const { return StatusBits.U.b.readOnly; }
    void setReadOnly(bool set) { StatusBits.U.b.readOnly = set; }

    class save_trend
    {
      public:
        save_trend(criticalSect& cSect, uint code, setOfString* set = 0) :
              notifyByTime(0), mainCode(code), buff(0), manValue(0),
              needSave(false), cSect(cSect), Set(set) {}

        ~save_trend();

        bool init(ImplJobMemBase *owner);
        bool fillData(mainClient *parent, bool onQuit = false);

        // ritorna l'abilitazione al salvataggio
        bool getEnableSave(mainClient *parent);
        // costruisce il nome del file, setta nel parametro se usa lo storico,
        // sottrae il numero passato per costruire un nome storico (es. del mese precedente)
        // In pratica si richiama la prima volta normalmente, se in useHistory torna true
        // lo si richiama impostando 1 in subIfHistory e si verifica l'esistenza del file
        // spostandolo nello storico in caso di successo
        bool makeName(mainClient *parent, LPTSTR target, bool& useHistory, int subIfHistory = 0) const;

        bool makeName(LPTSTR target, const FILETIME& ft) const;
        bool makeCurrentName(mainClient *parent, LPTSTR target) const;

        bool makeFindableName(LPTSTR target, bool& useHistory) const;
        DWDATA getLenRec() const { return lenRec; }
        bool exportHeader(P_File& pf) const;
        bool exportData(P_File& pf, LPBYTE buff) const;
        int getType() const;

        bool canSave() const { return needSave; }
        void resetSave();

        uint setTime(ImplJobMemBase *owner, uint time);
        uint getTime() const;
        void checkTimerTrend(ImplJobMemBase *owner);

        bool save(P_File& pf);

        // esegue un ciclo di N letture sul file per verificare la distanza temporale
        // tra i record. Torna > 0 se il valore trovato è valido, 0 se ci sono incongruenze
        // e < 0 se si è raggiunta la fine del file
        int checkTimeRes(P_File& pfSource, DWORD& resolution);

        uint getCode() const { return mainCode; }
      private:
        // puntatore all'oggetto che gestisce il timer
        class saveByTime *notifyByTime;
        // indirizzo principale in std_msg delle definizioni
        DWDATA mainCode;
        // dimensione record
        DWDATA lenRec;
        // puntatore al buffer necessario per contenere tutti i dati
        LPBYTE buff;
        // variabile settata dalla classe del timer
        bool needSave;
        criticalSect& cSect;

        bool makeNameByDate(LPTSTR target, uint type,  LPCTSTR p, int subIfHistory) const;
        bool makeNameByVar(mainClient *parent, LPTSTR target, LPCTSTR p) const;

        bool makeFindableNameByDate(LPTSTR target, uint type,  LPCTSTR p) const;
        bool makeFindableNameByVar(LPTSTR target, LPCTSTR p) const;

        mutable PVect<int> nDecForExport;

        // modifica, se Set != 0 anziché leggere da std_msg legge da Set e non inizializza
        // il timer per il salvataggio
        // si può creare un saveSet al volo per l'export.
        //
        setOfString* Set;

        // risoluzione per visualizzazione time, viene settata nell'exportHeader e
        // dipende dal (nuovo) valore aggiunto in coda al Code secondario. Se zero (o non
        // esiste) non viene usato, si può usare il metodo checkTimeRes() per eseguire un
        // check automatico. Deve essere 1000
        mutable DWORD timeRes;

        manageValue* manValue;
        bool makeHaderName(LPTSTR target) const;
        bool makeHeader(P_File& pf)   const;
      };

    int getNumOfTrend() const { return numTrend; }
    // prima di richiamarla verificare che ix sia nel range
    const save_trend& getSaveTrend(int ix) const { return *SaveTrend[ix]; }
    const save_trend* getSaveTrendByCode(uint code) const;

    virtual void dontSaveNow();

    virtual void getBlockData(LPDWORD target, uint ndw, uint startAddr);
    virtual void copyBlockData(LPDWORD source, uint ndw, uint startAddr);
    virtual void fillBlockData(DWORD value, uint ndw, uint startAddr);
    bool started() const { return Started; }
  protected:
//  private:
    mainClient *Par;
    bool Started;

    job_file jobFile;
    // punta a jobFile.data se file fisico, altrimenti al file mappato
    LPDWORD currData;
    // per confronto con i dati su jobFile
    DWDATA oldData[SIZE_OTHER_DATA];
    P_Bits BitsNoCommit;

    p_MappedFile* pMF;
    virtual void openMapping(bool useFileMapped);

//    IMaint *G_M;

    void saveFileTrend();


    void allocTrend(const PVect<uint>& trendCode);

    // numero di trend da gestire (ricavati da std_msg)
    uint numTrend;
    save_trend** SaveTrend;

    criticalSect cSect;
    mutable criticalSect cSectSet;

    struct currStatus
    {
      union {
        DWORD allBits;
        struct {
          DWORD needRefresh   : 1;
          DWORD accessingFile : 1;
          DWORD needSaveJob   : 1;
          DWORD saveLater     : 1;
          DWORD onProcessing  : 1;
          DWORD readOnly      : 1; // è in lettura da remoto
          DWORD readOnlyWrite : 1; // ma i dati sono tutti in locale
          } b;
        } U;
      currStatus() { U.allBits = 0; U.b.needRefresh = 1; }
    } StatusBits;
    PVect<int> vNoReload;
    DWORD lastTick;
};
//---------------------------------------------------------------------
//---------------------------------------------------------------------
#endif

/*
  Specifiche file di trend.

  Il codice 1000 indica i codici che contengono le informazioni.
  es. 1000,1111,1122,2123,ecc

  I codici che contengono informazioni sono nella forma:
  cod,addrForTimer,dataCod,type,prefix,ext,cod2

  es. 1111,10,1112,1,MH,.TRD

  addrForTimer è sempre preso dalla periferica memoria.
  Per compatibilità è stato lasciato in lGraph2.txt la possibilità di inserire
  la periferica, ma DEVE essere sempre usata quella in memoria (lGraph2 non effettua
  alcuna azione se non è la periferica giusta).

  dataCod è il codice contenente le informazioni sui dati da salvare.

  type può essere:
    0 -> trend giornaliero, il nome del file è formato da prefisso, la data e l'estensione
    1 -> trend mensile, il nome del file è formato da prefisso, la data e l'estensione
    2 -> trend annuale, il nome del file è formato da prefisso, la data e l'estensione

    3 -> trend abbinato a variabile, il nome del file è formato da prefisso,
         valori caricati da cod2 e l'estensione

    es. type 0 -> 'prefix_2002_06_09.ext'
    es. type 1 -> 'prefix_2002_06.ext'
    es. type 2 -> 'prefix_2002.ext'

    prefix non deve contenere spazi né virgole né altri segni di punteggiatura.
    ext deve essere di tre caratteri più il punto iniziale (non obbligatorio).

  cod2, dove richiesto è formato da:
    cod2,idprf,addr,typeOfData,minLen,idprf2,addr2,typeOfData2,minLen2,ecc.
    Se ci sono più blocchi, nella costruzione del nome, questi vengono separati dal segno '_'.
    es. cod2,1,5,3,3,1,6,4,4 supponendo che all'indirizzo 5 ci sia 44 e all'indirizzo 6 ci sia 23
        il prefisso sia 'pp' e l'estensione '.xxx' (naturalmente il type è 3) il risultato
        diventa 'pp_044_0023.xxx'
    se il numero di caratteri necessari supera la lunghezza minima questa viene estesa
    es. come sopra, ma all'indirizzo 5 c'è 1234 il risultato è 'pp_1234_0023.xxx'

    i tipi ammessi sono solo numeri interi senza segno (byte, word e dword) e stringa


  cod2 può essere omesso dove non richiesto. Nel caso non sia presente nel tipo 3 il nome
  viene semplicemente formato dal prefisso più l'estensione

  Nel caso di trend temporali (type 0, 1, 2) sommando 100 al type si attiva la gestione dello
  storico. In pratica al cambiamento del nome del file quello vecchio viene spostato nella
  cartella dello storico

  Formato dei dati nel file:

  Il primo dato è sempre FILETIME e contiene la data_ora del salvataggio.
  Gli altri dati sono sempre fREALDATA (float 32bit) e vengono ricavati da dataCod il
  cui formato è:
  dataCod,numOfData,firstCode,codeForEnableSave
  firstCode,idprf,addr,dataType,idNorm,nDec4Export,HeaderExport
  firstCode+1,idprf,addr,dataType,idNorm,nDec4Export,HeaderExport
  firstCode+2,idprf,addr,dataType,idNorm,nDec4Export,HeaderExport
  ....
  firstCode+n,idprf,addr,dataType,idNorm,nDec4Export,HeaderExport

  se non esiste nDec4Export viene impostato a 5 e poi vengono eliminati gli zeri non necessari.
  Altrimenti il numero viene esportato con i decimali richiesti (max 5)
  Viene comunque considerata l'intestazione del campo, se esiste
 es. dataCod,4,11234,11240
      11234,2,100,4,1,2,primo campo
      11235,2,104,5,0,1,secondo
      11236,2,110,4,3,2,terzo
      11237,2,10,4,2,2,quarto

 equivalente come dimensione a
  struct stat_file
  {
    FILETIME ft;
    fREALDATA val[4];
  };

  codeForEnableSave indica una variabile da cui ricavare se la deve salvare o meno
  il file di trend (es. se la macchina è ferma è inutile salvare)
  se omesso salva sempre (si può sempre, da programma, modificare il timer)
  Il formato è: idprf,addr,type,nbit,offset.
  Nel programma viene salvato lo stato di run nella variabile zero in memoria
  nel primo bit, quindi per usarla occorre usare:
    11240,1,0,1,1,0

esempio completo:
1000,1001,1002

mensile
1001,10,1100,1,MH,.TRD
1100,4,1101
1101,2,100,4,1
1102,2,104,5,0
1103,2,110,4,3
1104,2,10,4,2

da variabile numerica
1002,11,1200,3,trx,dat,1250
1200,6,1201,1251
1201,2,100,4,1
1202,2,104,5,0
1203,2,110,4,3
1204,2,10,4,2.
1205,2,210,4,2
1206,2,55,4,2

1250,1,24,4,4
1251,1,25,4,0

NOTE: (*)
  Nel salvataggio viene preso il valore corrente del dato.
  Se questo fosse zero (potrebbe dipendere da casi particolari) viene preso il
  valore precedente. Il valore corrente viene comunque memorizzato e, quindi,
  se il successivo è ancora zero viene ripreso quello precedente che però ora
  vale zero.
  Da verificare se in caso di doppio zero è opportuno ricaricare l'ultimo valore
  salvato e riportarlo a zero.

  (*) Non viene più controllato, si salva quello che c'è.


  Poiché si può usare una variabile per l'abilitazione del salvataggio, nella pagina
  di visualizzazione dei trend è stato rimosso il blocco al set del timer in caso di
  macchina ferma.
*/
