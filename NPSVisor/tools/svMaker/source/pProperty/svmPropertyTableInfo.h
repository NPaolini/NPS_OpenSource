//-------------------- svmPropertyTableInfo.h ---------------
//-----------------------------------------------------------
#ifndef svmPropertyTableInfo_H_
#define svmPropertyTableInfo_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjTableInfo.h"
//-----------------------------------------------------------
struct infoTblCommon
{
  COLORREF fg;
  COLORREF bg;
  UINT style;
  UINT idFont;
  UINT alignText;
  infoTblCommon() : fg(0), bg(RGB(255, 255, 255)), style(0), idFont(0), alignText(0) {}
};
//-----------------------------------------------------------
#define NROW_FULL 16
#define NCOL_FULL 3
#define NSTATE_FIRST 2
//-----------------------------------------------------------
#define USE_BMP 32
//-----------------------------------------------------------
class PropertyTableInfo : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyTableInfo();
    virtual ~PropertyTableInfo();

    const PropertyTableInfo& operator=(const PropertyTableInfo& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    void cloneMinusProperty(const Property& other);
    // comuni
    uint offsX;
    uint offsY;
    bool useSwap;
    // pannello base, usa type1 e type2

    uint timeCycle;

    // header
    uint Width[NCOL_FULL]; // larghezza colonne
    uint HeightHeader;
    PVect<LPCTSTR> textHeader;

    // prima colonna
    PVect<LPCTSTR> First;
//    bool useBmp;
    // infoFirst[0].alignText è per il testo
    // infoFirst[1].alignText è per il bitmap
    infoTblCommon infoFirst[NSTATE_FIRST];
    uint styleShow;

    // successive colonne
    PVect<LPCTSTR> Second;
    PVect<LPCTSTR> Third;
    infoTblCommon infoRows;
    uint heightRows;
    uint nRow;

  protected:
    virtual void clone(const Property& other);

  private:
    PropertyTableInfo(const PropertyTableInfo& other);
};
/*
nella base

    COLORREF foreground; -> usato per l'header
    COLORREF background; -> sfondo
    UINT style;  -> sfondo
    UINT idFont;   -> usato per l'header
    UINT alignText; -> usato per l'header

    // dipendenti dall'oggetto
    UINT type1; -> style per l'header
    UINT type2; -> COLORREF background; per l'header

    // quasi tutti gli oggetti sono agganciati a varibili
    UINT perif;
    UINT addr;
    UINT typeVal;
    UINT nDec;
    UINT nBits;
    UINT offset;
    int normaliz;
    bool negative;

    // nuova specifica per password abbinata a variabile (solo per campi di input)
    uint pswLevel;

    // nuove specifche, dati per gestione visibilità
    UINT visPerif;
    UINT visAddr;
    UINT visnBits;
    UINT visOffset;
    bool visNegative;

    PRect Rect;

*/
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmDialogTableInfo : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogTableInfo(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_TABLE_INFO_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst) {  }
    ~svmDialogTableInfo()    {   destroy();   }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
};
//-----------------------------------------------------------
#endif
