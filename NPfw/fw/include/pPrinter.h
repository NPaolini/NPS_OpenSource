//-------------------- pPrinter.H ---------------------------------------------
#ifndef PPRINTER_H_
#define PPRINTER_H_
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include <commdlg.h>
//-----------------------------------------------------------------------------
typedef unsigned short uint16;
//-----------------------------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------------------------
class PPrintDocBase
{
  public:
    PPrintDocBase() : totPages(0), currPage(0) {}
    virtual ~PPrintDocBase() { }

    virtual bool hasPage(uint curr_page) { return curr_page <= totPages; }

    // metodo da definire
    virtual void printPage(HDC hdc, uint which_page) = 0;
    virtual LPCTSTR getCaption() = 0;

    virtual void getPageInfo(int& /*minPage*/, int& /*maxPage*/,
                             int& /*fromPage*/, int& /*toPage*/)  { }


    // vengono richiamate nella print per permettere un maggior controllo
    // prima di aprire il dialogo o comunque prima di acquisire la stampante di default
    // viene chiamata subito prima di getPageInfo()
    virtual void preSetup(class PPrinter* /*printer*/) { };
    // subito dopo aver acquisito i dati sulla stampante
    virtual void postSetup(class PPrinter* /*printer*/) { };

    virtual void beginPrinting() {}
    virtual void beginDocument(int /*fromPage*/, int /*toPage*/) {}
    virtual void endDocument() {}
    virtual void endPrinting() {}

    uint getTotPages() { return totPages; }
    uint getCurrPage() { return currPage; }
  protected:
    void setTotPages(uint nPages) { totPages = nPages; }
    void setCurrPage(uint nPage) { currPage = nPage; }
  private:
    uint totPages;
    uint currPage;
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class PPrinter
{
  public:
    PPrinter(PWin* parent);
    virtual ~PPrinter() { }

    virtual bool Print(PPrintDocBase* doc, bool prompt);

    int escape(int esc, int count=0, const void* inData=0,
                       void* outData=0);
    uint QueryEscSupport(int escapeNum);
    int  SetCopyCount(int reqestCount, int& actualCount);
    HDC getHDC() const;

    POINT getPhysicalOffset() const;
    SIZE getPrintableSize() const;

    class infoPrn
    {
      public:
        infoPrn();
        ~infoPrn();

        void Lock();
        void Unlock();

        void            ClearDevMode();
        const DEVMODE*  GetDevMode() const;
        void            SetDevMode(const DEVMODE* devMode);
        void            ClearDevNames();
        const DEVNAMES* GetDevNames() const;
        LPCTSTR         GetDriverName() const;
        LPCTSTR         GetDeviceName() const;
        LPCTSTR         GetOutputName() const;
        void SetDevNames(LPCTSTR driver, LPCTSTR device, LPCTSTR output);

        bool createDC() const;


        int  fromPage;
        int  toPage;
        int  minPage;
        int  maxPage;
        uint  nCopies;
        DWORD Flags;

        // setup per le proprietà della stampante
        DWORD PageSetupFlags;
        POINT PaperSize;
        PRect  MinMargin;
        PRect  Margin;

      private:
        mutable HDC  prnDC;
        HGLOBAL   hDevMode;
        HGLOBAL   hDevNames;
        DEVMODE*  DevMode;
        DEVNAMES* DevNames;

        friend class PPrinter;
        infoPrn(const infoPrn&);
        infoPrn& operator =(const infoPrn&);
    };


    enum PrnSetup { DlgGoPrint, DlgPrintProperty };
    bool setup(PrnSetup type);

    void clearDev();

    infoPrn& getInfo() { return iPrn; }
    bool getDefPrn();

    void setResetPageSetupFlag(DWORD set, DWORD reset);

    int getLastErr() const { return Error; }
  protected:
    virtual PWin* allocAbortWindow(PWin* Parent, PPrinter* printer);

  private:

    PWin* Parent;
    infoPrn iPrn;
    PAGESETUPDLG Psd;
    PRINTDLG      Pd;
    int Error;

    NO_COPY_COSTR_OPER(PPrinter)
};
//-----------------------------------------------------------------------------
// MSG
#define WM_SETNUMBER    WM_USER+100
// WPARAM
#define TO_PAGE   1
#define CURR_PAGE 2
//-----------------------------------------------------------------------------
// attiva l'annullamento della stampa
void setAbort();
//-----------------------------------------------------------------------------
// crea una finestra di dialogo senza template, come una finestra normale
class abortPrinterWin : public PWin
{
  public:
    abortPrinterWin(PWin* parent, PPrinter* printer);
    ~abortPrinterWin();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR getClassName() const { return WC_DIALOG; }
    int totPage;
    virtual LPCTSTR getStringDriver() const { return _T("sulla stampante %s"); }
    virtual LPCTSTR getStringWait() const { return _T("Attendere, preparazione pagine"); }
    virtual LPCTSTR getStringPrinting() const { return _T("Sto stampando la pagina n.%d di %d"); }
  private:
    PPrinter* Printer;
    HFONT Font;
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline int PPrinter::escape(int esc, int count, const void* inData, void* outData)
{
  return ::Escape(getHDC(), esc, count, (const char far*)inData, outData);
                                        // API type cast
}
//-----------------------------------------------------------------------------
inline int PPrinter::SetCopyCount(int requestCopies, int& actualCopies)
{
  return escape(SETCOPYCOUNT, sizeof(int), &requestCopies, &actualCopies);
}
//-----------------------------------------------------------------------------
inline uint PPrinter::QueryEscSupport(int escapeNum)
{
  return escape(QUERYESCSUPPORT, sizeof(int), &escapeNum);
}
//-----------------------------------------------------------------------------
inline void PPrinter::setResetPageSetupFlag(DWORD set, DWORD reset)
{
  Psd.Flags |= set;
  Psd.Flags &= ~reset;
}
//-----------------------------------------------------------------------------
inline const DEVMODE* PPrinter::infoPrn::GetDevMode() const
{
  return DevMode;
}
//-----------------------------------------------------------------------------
inline const DEVNAMES* PPrinter::infoPrn::GetDevNames() const
{
  return DevNames;
}
//-----------------------------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------------------------
#endif
