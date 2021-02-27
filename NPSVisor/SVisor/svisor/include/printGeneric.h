//-------- printGeneric.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PRINTGENERIC_H_
#define PRINTGENERIC_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_File.h"
#include "svPrinter.h"
//----------------------------------------------------------------------------
/*
  Per mantenere le impostazioni di stampa preferite allocare e rendere
  permanente printer.
*/
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class printGeneric
{
  public:
    printGeneric(PWin* par, svPrinter* printer, LPCTSTR filename);
    virtual ~printGeneric();

    void Setup();
    static void Setup(svPrinter* printer);

    void Print(bool setup) { run(false, setup, 0); }
    void Preview(bool setup, LPCTSTR alternateBitmapOrID4Tool = 0)  { run(true, setup, alternateBitmapOrID4Tool); }

  protected:
    PWin *Parent;
    LPCTSTR Filename;
    svPrinter* Printer;
    P_File* File;
    offsetOfPage OOP;

    PRect pageForm;

    int currPage;

    int Y;      // coordinata y corrente
    int MAX_Y;  // viene inizialmente impostato a pageForm.bottom

    // se servono si aggiungono nelle classi derivate
//    int oldY;   // coordinata y precedente, da cui far partire eventali linee verticali
//    bool firstRow;

    // se derivata va chiamata prima di aggiungere dati
    virtual bool makeHeader();

    // non si dovrebbe derivarla
    virtual bool makeBody();

    // se derivata va chiamata dopo aver aggiunto dati
    virtual bool makeFooter();

    // richiamata nel makeBody() deve tornare 0 se errore e annulla la stampa,
    // < 0 se è terminata e non deve richiamare la makeHeader(), 1 per proseguire
    // per forzare un salto pagina porre Y >= MAX_Y
    virtual int addRow() = 0;

    // prepara il file ed imposta pageForm ai margini settati
    virtual bool beginDoc(bool showSetup);

    // segnaposto, viene chiamata prima di uscire dal beginDoc()
    virtual bool initializePrinter(svPrinter* /*printer*/) { return true; }

    // carica i margini impostati nella stampante, non quelli relativi a pageform.
    // Va usata in combinazione con setMargin() per evitare di modificare quelli che
    // non necessitano di modifiche
    void getMargin(PRect& r);
    PRect getMargin() { PRect r; getMargin(r); return r; }

    // imposta pageForm con i margini passati in r (in dmm), va chiamata dopo beginDoc(),
    // reimposta MAX_Y a pageForm.bottom
    void setMargin(const PRect& r);

    virtual void run(bool preview, bool setup, LPCTSTR alternateBitmapOrID4Tool);
  private:

    virtual LPCTSTR getTitle();

};
//----------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------
#endif
