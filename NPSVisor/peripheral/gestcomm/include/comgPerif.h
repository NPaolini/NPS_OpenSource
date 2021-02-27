//------------------ ComgPerif.H ------------------------------
//-----------------------------------------------------------
#ifndef COMGPERIF_H_
#define COMGPERIF_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#ifndef GESTCOMM_H_
  #include "gestComm.h"
#endif
#include "defGPerif.h"
#include "p_file.h"
#include "prfData.h"
#include "p_mappedfile.h"
#include "commidt.h"
#include "p_avl.h"
//-----------------------------------------------------------
// il file di conversione da cui ricavare l'array di address è formato da:
// indirizzo_logico_+_1,db_o_device_o_,indirizzo_periferica,tipo_di_dato
// per ogni riga.
//es. indirizzo zero nel buffer di lavoro, db 110, indirizzo 100 nella prf, tipo word
//1,110,100,3
// l'indirizzo è uno_based perché il valore zero non è ammesso
// se non viene usato un indirizzo, basta ometterlo nel file
// il file è scritto in formato carattere
typedef double aaType;

struct alternAddress
{
// aggiunta
  uint ipAddr; // indirizzo IP o altro
  uint port;
// nuova aggiunta
// se zero non vengono presi in considerazione
  aaType vMin;  // il range viene considerato se vMin != vMax
  aaType vMax;
  aaType vMinP;
  aaType vMaxP;

  uint db; // o device o numero periferica ecc.
  uint addr; // indirizzo corrispondente
  uint dataType; // tipo di dato, vedere il file prfData.h per la numerazione
  uint action;  // azione predefinita -> 0 = nessuna, 1 = lettura continua,
                // 2 = lettura solo all'avvio,
                // 4 = lettura a richiesta,
                // 8 = variabile di setup
                // altri valori possono servire per raggruppare
  alternAddress() : db(0), addr(0), dataType(0), action(0), ipAddr(0), port(0),
      vMin(0), vMax(0), vMinP(0), vMaxP(0) {}

};
// N.B. Il tipo di dato non viene usato nella gestione della periferica, se il dato
//      occupa due locazione devono essere caricate entrambe. Può comunque servire
//      ad eventuali classi derivate. Può servire nel supervisore per sapere la
//      dimensione del dato quando non viene specificato in una pagina, ma usato in
//      gestdata.
//-----------------------------------------------------------
namespace normVerify {
  enum verify { noAction, okRange, subRange, overRange };
};
//-----------------------------------------------------------
// nel programma di creazione del file .ad? l'ip viene memorizzato
// in ordine inverso al suo uso normale di apertura socket. Poiché
// è stato usato anche per altri usi, per mantenere la compatibilità
// occorre usare questa routine per invertire l'ordine dove necessario.
DWORD reverseIP(DWORD ip);
//-----------------------------------------------------------
template <class T>
class normalizer
{
  public:
    // non viene controllato il valore zero nel range, ma solo se sono diversi
    aaType vMin;
    aaType vMax;
    aaType vMinP;
    aaType vMaxP;

    normalizer(aaType vmin = 0, aaType vmax = 0, aaType vminP = 0, aaType vmaxP = 0) :
      vMin(vmin), vMax(vmax), vMinP(vminP), vMaxP(vmaxP) {}

    normalizer(const alternAddress& aa) :
      vMin(aa.vMin), vMax(aa.vMax), vMinP(aa.vMinP), vMaxP(aa.vMaxP) {}

      virtual normVerify::verify getNormalized(T& data) = 0;
    virtual normVerify::verify getDenormalized(T& data) = 0;
};
//-----------------------------------------------------------
template <class T>
class normalizerInt : public normalizer<T>
{
  public:
    normalizerInt(aaType vmin = 0, aaType vmax = 0, aaType vminP = 0, aaType vmaxP = 0) :
      normalizer<T>(vmin, vmax, vminP, vmaxP) {}

    normalizerInt (const alternAddress& aa) : normalizer<T>(aa) {}
    // se esiste il normalizzatore in data viene memorizzato l'equivalente binario
    // di un float, altrimenti il tipo di dato <T>
    virtual normVerify::verify getNormalized(T& data);
    // se esiste il normalizzatore da data viene preso l'equivalente binario
    // di un float
    virtual normVerify::verify getDenormalized(T& data);
};
//-----------------------------------------------------------
template <class T>
class normalizerReal : public normalizer<T>
{
  public:
    normalizerReal(aaType vmin = 0, aaType vmax = 0, aaType vminP = 0, aaType vmaxP = 0) :
      normalizer<T>(vmin, vmax, vminP, vmaxP) {}

    normalizerReal (const alternAddress& aa) : normalizer<T>(aa) {}

    virtual normVerify::verify getNormalized(T& data);
    virtual normVerify::verify getDenormalized(T& data);
};
//-----------------------------------------------------------
//typedef normalizerInt<WDATA>     wNormalizer;
typedef normalizerInt<DWDATA>    dwNormalizer;
typedef normalizerReal<fREALDATA> fNormalizer;
typedef normalizerReal<REALDATA>  rNormalizer;
//-----------------------------------------------------------
//typedef normalizerInt<WsDATA>     wsNormalizer;
typedef normalizerInt<DWsDATA>    dwsNormalizer;
//-----------------------------------------------------------
// nuova struttura per precaricare dati a lettura continua o comunque a richiesta
// per evitare la richiesta di una word alla volta.

// in caso di lettura necessita di un vettore contenente i corrispondenti indirizzi logici
// su cui copiare le word lette
// in caso di scrittura il buffer deve essere già preparato con le word in successione fisica
//-----------------------------------------------------------
#if 0
// per una eventuale modifica, si utilizzano due tipi di strutture, una quando
// i dati sono minori o uguali a (MIN_BUFF_TRANSFER/2), l'altra quando sono di più
// contiene fino a 20 indirizzi
#define MIN_BUFF_TRANSFER 40
//-----------------------------------------------------------
// contiene fino a 2000 indirizzi
#define MAX_BUFF_TRANSFER 4000
//-----------------------------------------------------------
#else
//-----------------------------------------------------------
// Modificato. Se i dati da contenere eccedono la capacità ne viene allocato
// uno successivo
#define MAX_BUFF_TRANSFER (512 - sizeof(uint) * 6)
//#define MAX_BUFF_TRANSFER (512 - sizeof(alternAddress))
//#define MAX_BUFF_TRANSFER 1000
//-----------------------------------------------------------
#endif
//-----------------------------------------------------------
struct addrToComm
{
//  virtual ~addrToComm() {}
// aggiunta
  uint ipAddr; // indirizzo IP o altro
  uint port;
  uint db;
  uint addr;
  uint len; // numero di dati (la dimensione dipende dal tipo)
  uint type;
  addrToComm(uint db = 0, uint addr = 0, uint len = 0, uint type = 0, uint ipAddr = 0, uint port = 0, bool onCycleReading = false) :
      db(db), addr(addr), len(len), type(type), ipAddr(ipAddr), port(port), onCycleReading(onCycleReading) {}

  void Clear() { db = 0; addr = 0; len = 0; type = 0; ipAddr = 0; port = 0; onCycleReading = false; }
// In scrittura il buffer contiene i dati pronti per essere spediti.
// In lettura il buffer (convertito in puntatore a WORD) contiene gli indirizzi logici
// corrispondenti agli indirizzi fisici
  BYTE buff[MAX_BUFF_TRANSFER];
  bool onCycleReading;

  bool operator ==(const addrToComm& other) {
    return ipAddr == other.ipAddr && db == other.db && addr == other.addr && len == other.len && port == other.port;
    }
};
//----------------------------------------------------------------------------
#define CHECK_MIN_SUP(a) if(a > other.a) return true; if(a < other.a) return false
//-----------------------------------------------------------------------------
class addrConv : public genericSet
{
  public:
    addrConv(uint ipAddr = 0, uint port = 0, uint db = 0, uint pAddr = 0, uint lAddr = 0,
            uint type = 0, bool onCycleReading = false) :
          ipAddr(ipAddr), port(port), db(db), pAddr(pAddr), lAddr(lAddr), type(type),
          onCycleReading(onCycleReading) {}
    uint ipAddr;
    uint port;
    uint db;
    uint pAddr;
    uint lAddr;
    uint type;
    bool onCycleReading;
    bool operator >(const addrConv& other) {
      CHECK_MIN_SUP(ipAddr);
      CHECK_MIN_SUP(port);
      CHECK_MIN_SUP(db);
      CHECK_MIN_SUP(pAddr);
      return false;
      }
};
//-----------------------------------------------------------------------------
typedef genericAvl<addrConv> orderedPAddr;
//-----------------------------------------------------------------------------
// nuove specifiche, il buffer è sempre a 32bit mentre il tipo di dato nella periferica
// viene ricavato dal file di testo con id 1000000 (es. 1000000,2 per tipo word)
#define DEF_TYPE_SIZE 4
#define ID_DATATYPE     1000000
#define ID_VER_ADR      1000001
#define ID_TIMER_TICK   1000002
//-----------------------------------------------------------
#define CUR_VERSION 7
#define CUR_VERSION_TXT _T("7")
//-----------------------------------------------------------------
#define CRYPT_HEAD "CRYPNPS"
#define DIM_HEAD sizeof(CRYPT_HEAD)
//-----------------------------------------------------------------
#define CRYPT_STEP     1
#define DIM_KEY_CRYPT  8
//-----------------------------------------------------------------
bool load_CriptedTxt(LPCBYTE key, class setOfString& set, LPCTSTR name);
//-----------------------------------------------------------------------------
class gestCommgPerif : public gestComm
{
  private:
    typedef gestComm baseClass;
  public:
	gestCommgPerif(LPCTSTR file, WORD flagCommand = 0);
    virtual ~gestCommgPerif();

    enum Perif_TypeCommand { tRead = 1, tWrite, tWriteBits };
    enum actionFromFile { NO, ALWAYS_READ, FIRST_READ, REQ_ONLY_READ = 4, VAR_4_SETUP = 8 };

    // noAction deve essere sempre zero. Quando il contenitore per i comandi
    // è vuoto torna zero che diventa, dopo un cast, noAction
    enum gPerif_Command { noAction = 0, readCustom, writeCustom,  writeNextCustom, writeBits, writeBits16 };
    // buffer è usato solo per il send e può contenere i dati (anziché essere letti da file). lenFile ne indica
    // la dimensione
    virtual bool addCommand(gPerif_Command cmd, WORD p1, DWORD p2, DWORD id_req, actionFromFile action = NO, LPBYTE buffer = 0);
//    virtual bool addCommand(gPerif_Command cmd, WORD p1, DWORD p2, DWORD id_req, actionFromFile action = NO);

    bool addReadCustom(int init, int end, WORD p/* = OK_READ*/);

    virtual ComResult Init();

    int getNumBlock() const { return numBuffBlock; }

    bool addReadReq(WORD wP, DWORD lP);
    bool addOneReadReq(WORD wP, DWORD lP);
    virtual uint getTimerTick() const;
    virtual uint getMoreCycleCount() const;

    // per invio dato singolo, è senza risposta
    virtual void addSingleWrite(DWORD data, WORD addr);

    virtual bool hasRequestVar() { return requestVarPresent; }
  protected:
    bool requestVarPresent;

    // usiamo due code, quella in scrittura ha sempre priorità su quella in lettura
    // in quella in scrittura si accoda sempre, in quella di lettura dipende dal ritorno desiderato
    class CustomQueue* CustomDataRead;
    class CustomQueue* CustomDataWrite;
    UDimF lenFile; // dimensione file in lettura di dati custom o dimensione del buffer

    // buffer temporaneo per lettura dati da Periferica
    BDATA Buff_Scratch[MAX_DIM_PERIF];

    // buffer di lavoro per lettura dati da Periferica
    BDATA Buff_Lav[MAX_DIM_PERIF];

    // buffer di lavoro per scrittura dati da Periferica
    BDATA Buff_Write[MAX_DIM_PERIF];

    // buffer contenente tutti i dati della periferica, per confronto e salvataggio su file
    // modifica: se viene usato un filemapped currBuff punta al file
    // altrimenti a memoria allocata
    BDATA* Buff_Perif;

    p_MappedFile* pMF;
    void openMapping(bool useFileMapped);

    // nome del file che contiene i dati e le direttive da spedire alla Periferica
    // Deve essere composto da:
    //  1) la prima word contiene il valore flagCommand
    //  2) due word per l'indirizzo di inizio e fine dati (compreso)
    //  3) n byte di dati (fine - inizio + 1) da trasmettere

    DWORD CurrAction;
    DWORD flagCommand;

    // se non esiste il file di conversione indirizzi del svisor in indirizzi fisici
    // Buff_Addr deve essere NULL, altrimenti deve contenere
    //  (MAX_DIM_PERIF/sizeof(tipo_di_dato_perif)) dati.
    // Ogni indirizzo del svisor (diviso per la dimensione del dato della periferica)
    // corrisponde all'offset nel vettore.
    // Il file di conversione ha lo stesso nome del file di dati, ma con estensione .ADR
    alternAddress* Buff_Addr;

    // richiamata dopo aver caricato il file, ma prima di aver inizializzato Buff_Addr
    // può servire per ricavare la dimensione di default da tornare poi in getTypeSize()

    // poiché viene usata praticamente da tutte le classi derivate è stata implementata
    // e spostata in libreria
    virtual void verifySet(class setOfString& set);

    virtual void performEndInit(class setOfString& set) {}

    virtual uint getTypeSize() const { return typeSize; }

    uint typeSize;

    virtual bool isSimilar(uint type1, uint type2);

    // definisce funzioni pure della classe base
    virtual ComResult SendCommand();
    virtual ComResult DoData(ComResult);
    virtual ComResult write(ComResult);

    // routine di default per la SendCommand() e la DoData()
    // richiamate quando è un comando diverso da quelli base
    // ## Attenzione!! Se viene inserito un comando nella coda,
    // occorre scaricarlo, non viene fatto automaticamente
//    virtual ComResult defSendCommand() { return gestComm::NoErr; }

    virtual ComResult defDoData(ComResult last) { return last; }
    virtual void defMakeAction(class customData *);
//-----------------------------------------------------------------------------
    // l'indirizzo finale e' compreso nella lettura, quindi (dim = end - init +1)
    // gli indirizzi sono già specifici per la periferica, quindi non è necessaria
    // alcun altra conversione

    // invia il comando [type] per i dati puntati da [pAddr]
    virtual ComResult Send(const addrToComm* pAddr, Perif_TypeCommand type) = 0;

    // spedisce i dati puntati da [pAddr]
    virtual ComResult SendData(addrToComm* pAddr) = 0;

    // riceve i dati puntati da [pAddr] nel buffer [buff]
    virtual ComResult Receive(const addrToComm* pAddr, BDATA* buff) = 0;
//-----------------------------------------------------------------------------

    // legge da file i dati da spedire alla Periferica, buff deve essere
    // almeno MAX_WORD_FILE per poter gestire il caso di lettura di
    // tutto la Periferica con blocchi multipli.
    // E' virtuale perche' potrebbe essere necessario avvisare il chiamante
    // dell'avvenuta lettura
    virtual ComResult read(BDATA *buff, uint id);
//-----------------------------------------------------------------------------
    // richiama la Receive() per leggere i dati correnti poi aggiorna la prima dword
    // del buffer contenuto in pAddr (usando la mask contenuta nella seconda dword).
    // Se la periferica consente l'invio in formato bit si può derivare per inibire
    // questa routine. Viene richiamata nella WriteCustom() prima del SendData().
    virtual void prepareSendBits(addrToComm* pAddr);

//    virtual void getSwapped(BDATA* value, uint size) {}
    uint TimerTick;
    bool orderSet(orderedPAddr& set, int init, int end, uint action);
    virtual void setInitError();
  private:
    // routine che estrae i dati custom da CustomData e li invia alla Periferica
    ComResult Send4NextCustom(Perif_TypeCommand type);
    ComResult Send4ReadCustom() { return Send4NextCustom(tRead); }
    ComResult Send4WriteCustom() { return Send4NextCustom(tWrite); }

    ComResult ReadCustom();
    ComResult WriteCustom();

    addrToComm* BuffBlock;
    int numBuffBlock;
    // per la lettura continua a richiesta
    addrToComm* BuffBlockReq;
    int numBuffBlockReq;

    // per lettura singola a richiesta, viene eliminata dopo la lettura
    struct oneRead {
      addrToComm* BlockReq;
      int numBlockReq;
      oneRead() : BlockReq(0), numBlockReq(0) {}
      // non viene implementato il distruttore per non implementare i costruttori di copia e assegnazione
      // necessari per l'uso in PVect
      };
    // viene usato un vettore perché potrebbero arrivare più richieste prima
    // dell'elaborazione
    PVect<oneRead> OneRead;

    bool checkUseReqRead();
    bool useBlockReq;

    // lista con le variabili attualmente in lettura
    PVect<uint> onReqRead;

    // eliminato dall'ereditarietà. Immette in coda i dati da leggere quando questa si vuota
    ComResult  defSendCommand();

    // il buffer passato alla classe finale viene travasato in quello di lavoro
    // dopo la ricezione secondo la corrispondenza
    // indirizzo fisico(init == 0 nel buffer) <-> indirizzo logico
    void ScratchToLav(addrToComm* LToP);

    // riempie LToP con i dati presi da Buff_Addr, maxVal indica il numero di
    // strutture preallocate e ne viene tornato il numero effettivamente caricate
    // se il buffer non è sufficiente torna false.
    // action indica il tipo di caricamento, se zero si caricano tutti quelli compresi
    // nell'intervallo, altrimenti quelli con lo stesso valore
    bool LAddrToPAddrReceive(int init, int end, addrToComm* LToP,
                int& maxVal, uint action);

    // apre il file dei comandi e riempie LToP
    // se buffer != null lo usa al posto del file
    bool LAddrToPAddrSend(addrToComm* LToP, int& maxVal, LPBYTE buffer, uint id);

    void tranferToBuffWrite(int addr, LPBYTE pbuff, int len);

    // esegue l'azione effettiva di riempimento, rec indica se per ricezione o send
    bool performLAddrToPAddr(orderedPAddr& set, addrToComm* LToP,
                int& maxVal, bool rec);

    virtual int getStep(uint type) { return prfData::getNByte((prfData::tData)type); }

    bool addBitsCommand(WORD p1, DWORD p2, DWORD id_req, bool on16);

    class manageSetupPrph* MSP;

    virtual void waitForReaded() {}

//------------------------------------------------------------------
    void deleteFile4Write(uint id);
    void makeFile4Write(LPTSTR path, uint id);
};
//-----------------------------------------------------------------------------
void stringToAddr(long ix, long nElem, LPCTSTR p, alternAddress* Buff_Addr);
//-----------------------------------------------------------------------------
#endif
