//------------ prfData.h -----------------------------------------------
//----------------------------------------------------------------------
#ifndef PRFDATA_H_
#define PRFDATA_H_
//----------------------------------------------------------------------
#ifndef DEFIN_H_
  #include "defin.h"
#endif
//----------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------
UINT svMessageBox(PWin* parent, LPCTSTR msg, LPCTSTR title, UINT flag);
//----------------------------------------------------------------------
class prfData
{
  public:
    enum tData {  tNoData,  // nessun tipo definito -  0
                  tBitData, // campo di bit         -  1
                  tBData,   // 8 bit -> 1 byte      -  2
                  tWData,   // 16 bit-> 2 byte      -  3
                  tDWData,  // 32 bit-> 4 byte      -  4
                  tFRData,  // 32 bit float         -  5
                  ti64Data, // 64 bit               -  6
                  tRData,   // 64 bit float         -  7
                  tDateHour,// 64 bit per data + ora-  8
                  tDate,    // 64 bit per data      -  9
                  tHour,     // 64 bit per ora      - 10
                  // valori signed
                  tBsData,   // 8 bit -> 1 byte      -  11
                  tWsData,   // 16 bit-> 2 byte      -  12
                  tDWsData,  // 32 bit-> 4 byte      -  13

                  tStrData,  // struttura formata dalla dimensione del buffer
                             // ed il puntatore (32 + 32 bit)

                  tNegative = 1 << 30,
//                  tArray = 1 << 31,
               };

    enum tResultData {  invalidType,
                        invalidAddress,
                        invalidPerif,

                        // per verificare velocemente se si è in errore

                        // da failed (compreso) andando indietro ci sono gli errori
                        failed = invalidPerif,

                        // da okData (compreso) in poi sono tutti validi,
                        okData,

                        notModified = okData,
                        isChanged
                     };

      // buffer per appoggio dato da caricare

      // se campo di bit l'indirizzo è comunque relativo
      // all'indirizzamento, il numero di bit e l'offset richiesti vanno
      // inseriti in u.dw come MAKELONG(nBit, offset). L'offset è
      // inteso come numero di bit da shiftare a dx prima di prendere i bit
      // che servono, es. se i bit sono tre e l'offset 2
      // (per 16 bit f edc ba9 876 543 210 -> - -fe dcb a98 765 432 -> xx

      // in caso di invio bit occorre usare MAKELONG(MAKEWORD(nBit, offset), val)
      // se il valore da inviare supera la capacità di 16 bit probabilmente avrebbe
      // più senso usarlo come word a parte
    union uData {
        BDATA b;
        WDATA w;
        DWDATA dw;

        BsDATA sb;
        WsDATA sw;
        DWsDATA sdw;

        fREALDATA fw;
        REALDATA rw;
        LARGE_INTEGER li;
        FILETIME ft; // usato sia per data/ora/data_ora
        struct strData {
          DWDATA len; // len non contempla il terminatore stringa che viene
          LPBYTE buff;// comunque allocato, in pratica len(buff) = len + 1
          } str;
        } U;

      // indirizzo logico di lettura, nel buffer
    uint lAddr;
      // indirizzo fisico di lettura, dipendente dalla periferica
    uint pAddr;

      // tipo di dato da leggere
    // tData
    DWDATA typeVar;

    prfData() : lAddr(0), pAddr(0), typeVar(tNoData) { U.li.QuadPart = 0; }
    prfData(uData u, uint p_addr, tData type, uint l_addr);

    prfData(uint lenBuff, LPBYTE Buff = 0);
    prfData(uint lenBuff, LPCTSTR Buff);
    prfData(LPBYTE buff);
    prfData(LPCTSTR buff);

    void setStr(uint len, LPBYTE newBuff = 0);
    void setStr(uint len, LPCTSTR newBuff);

    ~prfData();

    prfData(const prfData& other);
    const prfData& operator =(const prfData& other);

    // nel tipo bit torna zero. Nel caso (bit) la dimensione del tipo
    // viene impostata dalla periferica e quindi non si può stabilire a priori.
    // Nel caso (string) torna uno.
    static int getNBit(tData type);
    static int getNByte(tData type);

      // se lo stesso errore si è già verificato torna false e non visualizza nulla
    bool showMessageError(class PWin* win, uint prf, tResultData result) const;
  private:

    //usati come campi di bit
    static struct tagErrBuff {
      WORD Type;
      WORD Addr;
      WORD Perif;
      tagErrBuff() : Type(0), Addr(0), Perif(0) { }

      // la periferica viene considerata uno based.
      // Secondo le specifiche, la periferica uno identifica la memoria,
      // ma viene comunque considerata
      bool hasErr(uint perif, tResultData type);
      void setErr(uint perif, tResultData type);
      } errBuff;
};
//----------------------------------------------------------------------
// utility per ricavare il valore normalizzato o denormalizzato
bool getNormalizedData(REALDATA& val, const prfData& data, REALDATA norm);
bool getDenormalizedData(prfData& data, REALDATA val, REALDATA norm);
//----------------------------------------------------------------------
#define GET_NBIT(a) prfData::getNBit((prfData::tData)(a))
#define GET_NBYTE(a) prfData::getNByte((prfData::tData)(a))
//----------------------------------------------------------------------
/*
  NOTE:
    Il tipo di dato stringa può usarsi anche come array di byte, in dipendenza
    del contenuto. Viene comunque aggiunto uno zero terminatore oltre
    la dimensione data da len (nel caso si usi come stringa normale ed i suoi
    dati occupino tutto il buffer).

    L'aggiunta del tipo stringa rende la classe MOLTO FRAGILE.
    Poiché è stata usata in molte parti accedendo direttamente ai suoi dati
    l'incapsulamento degli stessi comporterebbe, per ora, una revisione di
    tutto il codice. Anche di quello precedente che non usa questo tipo di dato.

    Se non si usa il tipo stringa valgono le operazioni di prima.
    Se si usa il tipo stringa non si può modificare direttamente la variabile
    'typeVar' o la variabile 'U.str.len'.
    Per ora ho inserito semplicemente il metodo 'setStr()' da usarsi:

    esempio nel caso di richiesta di get()
    void fnz(int type, int len, int addr, HWND hwnd, .....)
    {
      ......
      prfData data;
      data.lAddr = addr;
      //--------------------------------
      if(prfData::tStrData == type)
        data.setStr(len);
      else
        data.typeVar = type;
      //--------------------------------
      prf->get(data);


      if(prfData::tStrData == type) {
        LPBYTE buff = data.U.str.buff;

        SetWindowTextA(hwnd, buff);
        }
      else {
        ..
        }
      ........
    }

    esempio nel caso di richiesta di set()
    void fnz(int type, int len, int addr, LPBYTE buffer.....)
    {
      ......
      prfData data;
      data.lAddr = addr;
      //--------------------------------
      if(prfData::tStrData == type)
        data.setStr(len, buffer);
      else {
        data.typeVar = type;
        switch(type) {
          .......
            data.U.?? = ??;
          }
        }
      //--------------------------------
      prf->set(data);
      .......
    }
    oppure nel caso di richiesta di set()
    void fnz(int type, int len, int addr, HWND hwnd, .....)
    {
      ......
      prfData data;
      data.lAddr = addr;
      //--------------------------------
      if(prfData::tStrData == type) {
        data.setStr(len);
        GetWindowTextA(hwnd, data.U.str.buffer, len + 1);
        }
      else {
        data.typeVar = type;
        switch(type) {
          .......
            data.U.?? = ??;
          }
        }
      //--------------------------------
      prf->set(data);
      .......
    }

  Naturalmente, se si usa il tipo stringa nelle pagine di testo è il
  motore che si occupa di tutto quanto necessario.

*/
//----------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------
#endif
