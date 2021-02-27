//-------- printImage.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PRINTIMAGE_H_
#define PRINTIMAGE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_File.h"
#include "svPrinter.h"
//----------------------------------------------------------------------------
#define DEF_BMP_NAME_4_PRINT _T("~print~.bmp")
//----------------------------------------------------------------------------
#define LEN_BMPFILEHEADER (sizeof(WORD) + sizeof(DWORD) + sizeof(WORD) + sizeof(WORD) + sizeof(DWORD))
//----------------------------------------------------------------------------
class printImage
{
  public:
    printImage(PWin* par, svPrinter* printer, LPCTSTR filename);
    virtual ~printImage();

    virtual bool beginDoc(bool showSetup);
//    virtual bool endDoc();

    void Print() { run(false); }
    void Preview()  { run(true); }
  protected:
    PWin *Parent;
    LPCTSTR Filename;
    svPrinter* Printer;
    P_File* File;
    offsetOfPage OOP;

//    SIZE pageSize;

    int currPage;
/*
    int X;
    int Y;  // coordinata y corrente
    int oldY; // coordinata y precedente, da cui far partire le linee verticali
    int minDim; // dimensione del file quando non vengono scritti dati
*/
//    bool firstRow;
    virtual bool makeHeader();
    virtual bool makeBody();
    virtual bool makeFooter();
  private:
    void run(bool preview);

    virtual LPCTSTR getTitle();

};
//----------------------------------------------------------------------------
#endif
