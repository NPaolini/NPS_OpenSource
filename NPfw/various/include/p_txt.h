//------ P_Txt.h -------------------------------------------------------------
#ifndef P_TXT_H_
#define P_TXT_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "setPack.h"
//----------------------------------------------------------------------------
class manageRowOfBuff
{
  public:
    manageRowOfBuff(LPCTSTR filename);
    // da un buffer allocato
    manageRowOfBuff(uint len, LPSTR buffer, bool autodelete = false);
    manageRowOfBuff(uint len, LPWSTR buffer, bool autodelete = false);
    virtual ~manageRowOfBuff();

    bool run();
  protected:
    virtual bool manageLine(LPCTSTR row) = 0;

  private:
    LPTSTR Buff;
    uint Len;
    bool autoDelete;

    NO_COPY_COSTR_OPER(manageRowOfBuff)
};
//----------------------------------------------------------------------------
// classe virtuale pura per gestione unica dei due tipi di file
//----------------------------------------------------------------------------
class codeSetOfString
{
  public:
    codeSetOfString(LPCTSTR filename = 0) {}
    // da un buffer allocato
    codeSetOfString(uint len, LPSTR buffer, bool autodelete = false) {}
    codeSetOfString(uint len, LPWSTR buffer, bool autodelete = false) {}
    virtual ~codeSetOfString() {}

    // aggiunge un altro set (non devono esserci numeri uguali)
    // usato anche quando non si ha la possibilità di impostare
    // il nome del file da caricare nel costruttore
    virtual void add(LPCTSTR filename) = 0;
    // da un buffer allocato
    virtual void add(uint len, LPSTR buffer, bool autodelete = false) = 0;
    virtual void add(uint len, LPWSTR buffer, bool autodelete = false) = 0;

    // recupera la stringa, restituisce zero se non trova il numero
    virtual LPCTSTR getString(long id) const = 0;

    // come sopra, ma restituisce una stringa formata dall'id passato
    // se non trova il numero
    virtual LPCTSTR getStringOrId(long id) const = 0;

    virtual uint getNumElem() = 0;

    // addIfNotExist = true -> aggiunge la nuova stringa se non esiste,
    // deleteOld = true -> rimuove quella esistente
    virtual void replaceString(long id, LPTSTR newString, bool addIfNotExist = false, bool deleteOld = true) = 0;

    virtual bool setFirst() = 0;
    virtual bool setLast() = 0;
    virtual bool setNext() = 0;
    virtual bool setPrev() = 0;

    virtual long getCurrId() const = 0;
    virtual LPCTSTR getCurrString() const = 0;

    // per scrivere i dati su file compatibili con la lettura, usa translateFromCRNL()
    virtual bool writeCurrent(class P_File& pf, bool unicode = false) = 0;

    virtual void reset() = 0;

    NO_COPY_COSTR_OPER(codeSetOfString)
};
//----------------------------------------------------------------------------
// carica da un file di testo tutte le stringhe inizianti con un
// numero e memorizza la parte seguente. E' possibile poi recuperare
// la stringa dal numero.

// Costruisce il set nel ctor e viene distrutto nel dtor

// se si compila in UNICODE verificare se anche i file sono in UNICODE
// in caso affermativo occorre ricompilare il sorgente attivando la
// direttiva USE_UNICODE_FILES

// nella lettura, sia da buffer che da file, esegue la trasformazione di alcuni
// caratteri, vedere translateToCRNL()

class setOfString : public codeSetOfString
{
  public:
    setOfString(LPCTSTR filename = 0);
    // da un buffer allocato
    setOfString(uint len, LPSTR buffer, bool autodelete = false);
    setOfString(uint len, LPWSTR buffer, bool autodelete = false);
    virtual ~setOfString();

    // aggiunge un altro set (non devono esserci numeri uguali)
    // usato anche quando non si ha la possibilità di impostare
    // il nome del file da caricare nel costruttore
    void add(LPCTSTR filename);
    // da un buffer allocato
    void add(uint len, LPSTR buffer, bool autodelete = false);
    void add(uint len, LPWSTR buffer, bool autodelete = false);

    // recupera la stringa, restituisce zero se non trova il numero
    LPCTSTR getString(long id) const;

    // come sopra, ma restituisce una stringa formata dall'id passato
    // se non trova il numero
    LPCTSTR getStringOrId(long id) const;

    uint getNumElem();

    // implementata per verificare la consistenza del set (albero AVL)
    // salva sul file passato i dati riguardanti l'id, il nodo, la posizione
    // del nodo sia in profondità che in larghezza e la stringa abbinata
    void saveTree(LPCTSTR filename);

    // addIfNotExist = true -> aggiunge la nuova stringa se non esiste,
    // deleteOld = true -> rimuove quella esistente
    void replaceString(long id, LPTSTR newString, bool addIfNotExist = false, bool deleteOld = true);

    bool setFirst();
    bool setLast();
    bool setNext();
    bool setPrev();

    long getCurrId() const;
    LPCTSTR getCurrString() const;

    // per scrivere i dati su file compatibili con la lettura, usa translateFromCRNL()
    bool writeCurrent(class P_File& pf, bool unicode = false);

    void reset();
  protected:
    // classe effettiva di implementazione, per nascondere vari
    // dettagli implementativi
    class impl_set* ImplSet;

    NO_COPY_COSTR_OPER(setOfString)
};
//----------------------------------------------------------------------------
// trasforma le coppie ('\\' + 'n') ('\\' + 't') in ('\r\n') ('\t') ed elimina
// la prima barra rovescia negli altri casi
LPTSTR translateToCRNL(LPTSTR target, LPCTSTR source);
//----------------------------------------------------------------------------
// trasformazione inversa, da ('\r\n') ('\t') in('\\' + 'n') ('\\' + 't') e
// raddoppia la barra rovescia
LPTSTR translateFromCRNL(LPTSTR target, LPCTSTR source);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// simile a quella sopra implementa un altro tipo di file
// ogni codice è racchiuso tra [] su una riga, tutto quello che segue,
// fino al termine o fino a che non trova un altro codice fa parte del blocco
// ogni riga che inizia con # è considerata commento e non viene inclusa
//----------------------------------------------------------------------------
class setOfBlockString : public codeSetOfString
{
  public:
    setOfBlockString(LPCTSTR filename = 0);
    // da un buffer allocato
    setOfBlockString(uint len, LPSTR buffer, bool autodelete = false);
    setOfBlockString(uint len, LPWSTR buffer, bool autodelete = false);
    virtual ~setOfBlockString();

    // aggiunge un altro set (non devono esserci numeri uguali)
    // usato anche quando non si ha la possibilità di impostare
    // il nome del file da caricare nel costruttore
    void add(LPCTSTR filename);
    // da un buffer allocato
    void add(uint len, LPSTR buffer, bool autodelete = false);
    void add(uint len, LPWSTR buffer, bool autodelete = false);

    // recupera la stringa, restituisce zero se non trova il numero
    LPCTSTR getString(long id) const;

    // come sopra, ma restituisce una stringa formata dall'id passato
    // se non trova il numero
    LPCTSTR getStringOrId(long id) const;

    uint getNumElem();

    void replaceString(long id, LPTSTR newString, bool addIfNotExist = false, bool deleteOld = true);

    bool setFirst();
    bool setLast();
    bool setNext();
    bool setPrev();

    long getCurrId() const;
    LPCTSTR getCurrString() const;

    // per scrivere i dati su file compatibili con la lettura
    bool writeCurrent(class P_File& pf, bool unicode = false);

    void reset();
  protected:
    // classe effettiva di implementazione, per nascondere vari
    // dettagli implementativi
    class impl_blockSet* ImplSet;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif
