//-------------------- pPrintPage.h -------------------------------------------
#ifndef PPRINTPAGE_H_
#define PPRINTPAGE_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
#include "pDialog.h"
#include "pPrinter.h"
#include "p_Vect.h"
#include "pToolBar.h"
#include "pBitmap.h"
//-----------------------------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------------------------
class PPrintPage : public PPrintDocBase//, public PWin
{
  public:
    PPrintPage(PWin* parent, PPrinter* allocated = 0, LPCTSTR title = 0, bool autoDeletePrinter = true);
    virtual ~PPrintPage();


//    virtual LPCTSTR getCaption() { return PWin::getCaption(); }
    virtual LPCTSTR getCaption() { return Title; }
    PWin* getParent() { return Parent; }
    virtual void printPage(HDC hdc, uint which_page);
    void Print(bool showDialog = false);
    void PrintPreview(LPCTSTR alternateBitmapOrID4Tool = 0);

    virtual void initializePrinter(PPrinter *printer);
    PPrinter* getPrinter() { return Printer; }
    HDC getPrnDC() { return Printer->getHDC(); }
    virtual void paintPage(HDC hdc);

    POINT getPhysicalOffset() const { return Printer->getPhysicalOffset(); }
    SIZE getPrintableSize() const { return Printer->getPrintableSize(); }


  protected:
    virtual class PPreviewPage* allocPreviewPage(PWin* parent, PPrintPage* page, bool& print, LPCTSTR alternateBitmapOrID4Tool = 0);
    friend class PPreviewPage;
//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual void getWindowClass(WNDCLASS& wcl);
    uint currPainting;

    PWin* Parent;
    LPCTSTR Title;
    PPrinter* Printer;
    bool preview;
    bool autoDeletePrinter;
    NO_COPY_COSTR_OPER(PPrintPage)
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// codici per file da preparare per stampa
enum print_code {
      PRN_TXT = 1,      // X,Y,testo
      PRN_LINE,         // X1,Y1,X2,Y2
      PRN_POLYLINE,     // nlinee, Xiniz,Yiniz,X1,Y1,X2,Y2,..Xn,Yn
      PRN_BOX,          // X,Y,W,H -> rettangolo vuoto
      PRN_BMP,          // X,Y,W,H,nome_file_BMP
      PRN_SET_FONT,     // H,L,Flag (&1->Italic, &2->Bold, &4->Underlined), nome del font ('*' -> default)
      PRN_SET_FONT2,     // H,L,angolo,Flag,nome del font ('*' -> default)
      PRN_SET_COLOR_FG, // C
      PRN_SET_COLOR_BKG,// C
      PRN_SET_ALIGN,    // ALIGN_HORZ,ALIGN_VERT
      PRN_SET_BKMODE,   // OPAQUE | TRANSPARENT
      PRN_SET_PEN,      // color, width, style
      PRN_SET_BRUSH1,    // color
      PRN_SET_BRUSH2,    // color, style
      PRN_SET_BRUSH3,    // nome_file_BMP

      PRN_RECT,          // X,Y,W.H  -> rettangolo pieno
      PRN_OVAL,          // X,Y,W,H,filled
      PRN_TXT_ON_BOX,    // X,Y,W,H,align,testo

      PRN_SET_NULL_BRUSH,
      PRN_ROUND_RECT,    // X,Y,W.H,cx,cy  -> rettangolo bordato pieno
      PRN_ARC,           // X,Y,W.H,rx1,ry1,rx2,ry2
      PRN_PIE,           // X,Y,W.H,rx1,ry1,rx2,ry2
      PRN_CORD,          // X,Y,W.H,rx1,ry1,rx2,ry2
      };
//-----------------------------------------------------------------------------
#define BUFF_DIM_PAGE 12
//#define BUFF_DIM_PAGE 6
#define BUFF_DIM_PAGE_BYTE (BUFF_DIM_PAGE * sizeof(TCHAR))
// il file è diviso in pagine ed è scritto in ASCII
// i primi BUFF_DIM_PAGE TCHAR (in ASCII) indicano la dimensione della pagina
// (servono per allocare la memoria per la pagina)
// poi segue il codice e i dati specifici, ogni riga termina con un CR+NL
//
//-----------------------------------------------------------------------------
LPCTSTR get_next(LPCTSTR p, int &dim);
//-----------------------------------------------------------------------------
LPTSTR get_line(LPTSTR buff, int len);
//-----------------------------------------------------------------------------
class offsetOfPage : public PVect<long>
{
  public:
    offsetOfPage() : PVect<long>(2) {  }
};
//-----------------------------------------------------------------------------
// page può essere lasciato vuoto, verrà riempito dalla classe e può quindi essere riutilizzato
// in caso di non cambiamento nel file
//-----------------------------------------------------------------------------
class PPrintFile : public PPrintPage
{
  public:
    PPrintFile(LPCTSTR file, offsetOfPage &page, PWin* parent, PPrinter* allocated = 0,
                  LPCTSTR title = 0, bool autoDeletePrinter = true);
      //,    HINSTANCE hinstance = 0);
    virtual ~PPrintFile();
    void getPageInfo(int& minPage, int& maxPage, int& selFromPage, int& selToPage);
  protected:
    virtual void paintPage(HDC hdc);
    void fillOffsetPage();

    class P_File *File;
    offsetOfPage &Page;

    int dimPage;
    int maxDimPage;
    LPTSTR Buff;
};
//-----------------------------------------------------------------------------
enum idCmd {  IDM_DONE = 100,
              IDM_PREVIOUS,
              IDM_NEXT,
              IDM_ZOOM_IN,
              IDM_ZOOM_OUT,
              IDM_PRINT,
            };
//-----------------------------------------------------------------------------
class PPreviewPage : public PDialog
{
  public:
    PPreviewPage(PWin* parent, PPrintPage* page, bool& print, LPCTSTR alternateBitmapOrID4Tool = 0);
    ~PPreviewPage();

    virtual bool create();
    virtual bool preProcessMsg(MSG& msg);

    void postAndSel(uint id);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void runPage(HDC hdc, int numPage);
    int totPage;

    void makePage(int numPage, uint zoomed = 0);
  private:
    class myWin_Tools* Tools;
    PPrintPage* Page;
    class PWorkWin* workPage;
    class PPaperWin* paperPage;
    int heightTool;
    bool& choosePrint;

    int maxScrollV;
    int pageScrollV;

    int maxScrollH;
    int pageScrollH;

    void evMouseWheel(short delta, short x, short y);

    void HScroll(int type, int pos);
    void VScroll(int type, int pos);
    void genScroll(bool isVert, int action, int pos);

    POINT PtOffsPage;

    uint Zoomed;

    LPCTSTR alternateToolBitmap;
    PBitmap* alternateBitmap;
    PToolBarInfo dupTools;

    typedef PDialog baseClass;
};
//----------------------------------------------------------------------------
// alcune macro di aiuto
/*
  N.B. il file che verrà caricato dalla PPrintPage deve essere in unicode
  se si sta compilando in unicode.
  Quindi è conveniente usare le macro sotto.
  La prima serie non verifica la dimensione del buffer, la seconda richiede
  la dimensione del buffer, la terza viene usata per vettori (non allocati
  dinamicamente per cui è possibile usare il sizeof)
*/
//----------------------------------------------------------------------------
#define RESULT(file, buff) \
  uint len = _tcslen(buff) * sizeof(buff[0]);       \
  result = file->P_write(buff, len) == len;

//    result = toBool(file->P_writeString(buff));

//----------------------------------------------------------
#define PRINT0(file, buff) \
  { RESULT(file, buff)  }
//----------------------------------------------------------
#define PRINT1(file, buff, formt, a) \
  { wsprintf(buff, formt, a);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT2(file, buff, formt, a, b) \
  { wsprintf(buff, formt, a, b);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT3(file, buff, formt, a, b, c) \
  { wsprintf(buff, formt, a, b, c);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT4(file, buff, formt, a, b, c, d) \
  { wsprintf(buff, formt, a, b, c, d);  \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT5(file, buff, formt, a, b, c, d, e) \
  { wsprintf(buff, formt, a, b, c, d, e);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT6(file, buff, formt, a, b, c, d, e, f) \
  { wsprintf(buff, formt, a, b, c, d, e, f);  \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT7(file, buff, formt, a, b, c, d, e, f, g) \
  { wsprintf(buff, formt, a, b, c, d, e, f, g);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT8(file, buff, formt, a, b, c, d, e, f, g, h) \
  { wsprintf(buff, formt, a, b, c, d, e, f, g, h);  \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
//----------------------------------------------------------
#define PRINT_CR_NL(file, buff)  PRINT0(file, _T("\r\n"))
//----------------------------------------------------------
#define PRINT1_S(file, buff, size, formt, a) \
  { _stprintf_s(buff, size, formt, a);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT2_S(file, buff, size, formt, a, b) \
  { _stprintf_s(buff, size, formt, a, b);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT3_S(file, buff, size, formt, a, b, c) \
  { _stprintf_s(buff, size, formt, a, b, c);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT4_S(file, buff, size, formt, a, b, c, d) \
  { _stprintf_s(buff, size, formt, a, b, c, d);  \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT5_S(file, buff, size, formt, a, b, c, d, e) \
  { _stprintf_s(buff, size, formt, a, b, c, d, e);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT6_S(file, buff, size, formt, a, b, c, d, e, f) \
  { _stprintf_s(buff, size, formt, a, b, c, d, e, f);  \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT7_S(file, buff, size, formt, a, b, c, d, e, f, g) \
  { _stprintf_s(buff, size, formt, a, b, c, d, e, f, g);   \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
#define PRINT8_S(file, buff, size, formt, a, b, c, d, e, f, g, h) \
  { _stprintf_s(buff, size, formt, a, b, c, d, e, f, g, h);  \
    RESULT(file, buff)  \
  }
//----------------------------------------------------------
//----------------------------------------------------------
#define PRINT1_A_S(file, buff, formt, a) \
  { PRINT1_S(file, buff, SIZE_A(buff), formt, a)  }
//----------------------------------------------------------
#define PRINT2_A_S(file, buff, formt, a, b) \
  { PRINT2_S(file, buff, SIZE_A(buff), formt, a, b)  }
//----------------------------------------------------------
#define PRINT3_A_S(file, buff, formt, a, b, c) \
  { PRINT3_S(file, buff, SIZE_A(buff), formt, a, b, c)  }
//----------------------------------------------------------
#define PRINT4_A_S(file, buff, formt, a, b, c, d) \
  { PRINT4_S(file, buff, SIZE_A(buff), formt, a, b, c, d)  }
//----------------------------------------------------------
#define PRINT5_A_S(file, buff, formt, a, b, c, d, e) \
  { PRINT5_S(file, buff, SIZE_A(buff), formt, a, b, c, d, e)  }
//----------------------------------------------------------
#define PRINT6_A_S(file, buff, formt, a, b, c, d, e, f) \
  { PRINT6_S(file, buff, SIZE_A(buff), formt, a, b, c, d, e, f)  }
//----------------------------------------------------------
#define PRINT7_A_S(file, buff, formt, a, b, c, d, e, f, g) \
  { PRINT7_S(file, buff, SIZE_A(buff), formt, a, b, c, d, e, f, g)  }
//----------------------------------------------------------
#define PRINT8_A_S(file, buff, formt, a, b, c, d, e, f, g, h) \
  { PRINT8_S(file, buff, SIZE_A(buff), formt, a, b, c, d, e, f, g, h)  }
//----------------------------------------------------------
//-----------------------------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------------------------
#endif
