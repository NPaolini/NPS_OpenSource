//----------- replaceVarName.h ----------------------------
#ifndef replaceVarName_H_
#define replaceVarName_H_
//---------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------
// inizio variabili locali nelle funzioni
#define MAX_PARAM_ID 1000
//------------------------------------------------------------------
// quattro perché i primi sono skip e abort, poi id è la funzione e l'id è uno based
#define OFFS_PARAM 4
#define ID_FUNCT_NAME 3
//---------------------------------------------------------
class replaceVarName
{
  public:
    replaceVarName() : LeftValue(0) {}
    virtual ~replaceVarName() {}

    virtual LPCTSTR getName(uint id) = 0;
    virtual uint getId(LPCTSTR name) = 0;
    virtual void setStartId(uint id) {}
    virtual void resetStartId() {}

    virtual uint getIdAdd(LPCTSTR name) = 0;

    // Sia len che canDelete sono parametri di ingresso/uscita e
    // si riferiscono a buff. Non dovrebbe succedere, ma se il
    // buffer non riesce a contenere i nuovi dati viene distrutto
    // (se canDelete in ingresso vale true) e riallocato (canDelete
    // varrà true in uscita). len, in uscita, contiene sempre la
    // nuova dimensione
    // leftValue indica se si deve gestire solo la parte sinistra (es. in vars o init)
    // se zero indica che è la zona funct o calc, se uno è la zona vars, maggiore di uno le altre
    // con il valore uno si controlla che la variabile non sia duplicata
    virtual LPTSTR run(LPTSTR buff, uint &len, bool& canDelete, int leftValue, bool first) = 0;

    virtual void flushWork() = 0;
  protected:
    int LeftValue;
};
//---------------------------------------------------------
extern
replaceVarName* allocReplaceVarName();

// il blocco funzioni deve essere parsato sia da quella sopra (generale)
// che da questa sotto, ogni funzione deve allocare il proprio
extern
replaceVarName* allocReplaceVarName4Funct();
//---------------------------------------------------------
#endif
