//--------------- svPrinter.h ---------------------------------------------
//----------------------------------------------------------------------------
#ifndef SVPRINTER_H_
#define SVPRINTER_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pprintPage.h"
#include "1.h"
#include "p_name.h"
#include "mainClient.h"
#include "language.h"

#include "setPack.h"
//-----------------------------------------------------------------------------
class svAbortPrinterWin : public abortPrinterWin
{
#if 1
  public:
    svAbortPrinterWin(PWin* parent, PPrinter* printer) : abortPrinterWin(parent, printer),
    id_PRINTER_DRIVER(0), id_PRINT_PAGE_WAIT(0), id_PRINT_PAGE(0)
    { }
    ~svAbortPrinterWin() {  delete id_PRINTER_DRIVER; delete id_PRINT_PAGE_WAIT; delete id_PRINT_PAGE; }
  protected:
    mutable smartPointerConstString* id_PRINTER_DRIVER;
    mutable smartPointerConstString* id_PRINT_PAGE_WAIT;
    mutable smartPointerConstString* id_PRINT_PAGE;

    LPCTSTR retString(smartPointerConstString* &sps, uint id) const
    {
      delete sps;
      sps = new smartPointerConstString(getStringByLangGlob(id));
      return *sps;
    }
    virtual LPCTSTR getStringDriver() const
    {
      if(!retString(id_PRINTER_DRIVER, ID_MSG_PRINTER_DRIVER))
        return abortPrinterWin::getStringDriver();
      return *id_PRINTER_DRIVER;
    }
    virtual LPCTSTR getStringWait() const
    {
      if(!retString(id_PRINT_PAGE_WAIT, ID_MSG_PRINT_PAGE_WAIT))
        return abortPrinterWin::getStringWait();
      return *id_PRINT_PAGE_WAIT;
    }
    virtual LPCTSTR getStringPrinting() const
    {
      if(!retString(id_PRINT_PAGE, ID_MSG_PRINT_PAGE))
        return abortPrinterWin::getStringPrinting();
      return *id_PRINT_PAGE;
    }
#else
  public:
    svAbortPrinterWin(PWin* parent, PPrinter* printer) : abortPrinterWin(parent, printer) {}
  protected:
    virtual LPCTSTR getStringDriver() const { return getStringOrId(ID_MSG_PRINTER_DRIVER); }
    virtual LPCTSTR getStringWait() const { return getStringOrId(ID_MSG_PRINT_PAGE_WAIT); }
    virtual LPCTSTR getStringPrinting() const { return getStringOrId(ID_MSG_PRINT_PAGE); }
#endif
};
//-----------------------------------------------------------------------------
class svPrinter : public PPrinter
{
  public:
    svPrinter(PWin* parent) : PPrinter(parent) {}
  protected:
    virtual PWin* allocAbortWindow(PWin* Parent, PPrinter* printer)
    {  return  new svAbortPrinterWin(Parent, printer);  }
};
//-----------------------------------------------------------------------------
class svPreviewPage : public PPreviewPage
{
  public:
    svPreviewPage(PWin* parent, PPrintPage* page, bool& print, LPCTSTR alternateBitmapOrID4Tool = 0)
        : PPreviewPage(parent, page, print, alternateBitmapOrID4Tool)
    {
      mainClient* mcl = getMain();
      if(mcl)
        mcl->enableDialogFunctionKey();
    }
    ~svPreviewPage()
    {
      mainClient* mcl = getMain();
      if(mcl)
        mcl->disableDialogFunctionKey();
      destroy();
    }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    typedef PPreviewPage baseClass;
};
//-----------------------------------------------------------------------------
class svPrintPage : public PPrintPage
{
  public:
    svPrintPage(PWin* parent, PPrinter* allocated = 0, LPCTSTR title = 0) :
        PPrintPage(parent, allocated, title) {}
  protected:
    virtual class PPreviewPage* allocPreviewPage(PWin* parent, PPrintPage* page, bool& print, LPCTSTR alternateBitmapOrID4Tool = 0);
};
//-----------------------------------------------------------------------------
class svPrintFile : public PPrintFile
{
  public:
    svPrintFile(LPCTSTR file, offsetOfPage &page, PWin* parent, PPrinter* allocated = 0, LPCTSTR title = 0,
        bool autoDeletePrinter = true) :
        PPrintFile(file, page, parent, allocated, title, autoDeletePrinter) {}
  protected:
    virtual class PPreviewPage* allocPreviewPage(PWin* parent, PPrintPage* page, bool& print, LPCTSTR alternateBitmapOrID4Tool = 0);
};
//-----------------------------------------------------------------------------
#include "restorePack.h"
#endif
