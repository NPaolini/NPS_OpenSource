//-------------------- pPrinter.cpp -------------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
//#ifdef __BORLANDC__
  #include <cderr.h>
//#endif
#include <commdlg.h>
#include "pStatic.h"
#include "pButton.h"
#include "pPrinter.h"
//#include "1.h"
//#include "p_name.h"
#include "defin.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static bool Aborted = false;
void setAbort() { Aborted = true; }
//-----------------------------------------------------------------------------
int CALLBACK printerAbortProc(HDC hDC, int code);
//-----------------------------------------------------------------------------
PPrinter::PPrinter(PWin* parent) : Parent(parent)
{
  memset(&Pd, 0, sizeof Pd);
  Pd.lStructSize = sizeof Pd;
  Pd.hwndOwner = parent ? parent->getHandle() : 0;
  Pd.hInstance = PAppl::getHinstance();
  Pd.Flags = iPrn.Flags;
  Pd.Flags &= ~(PD_RETURNDEFAULT | PD_ENABLEPRINTTEMPLATE | PD_ENABLESETUPTEMPLATE);

  Pd.nFromPage = (uint16)iPrn.fromPage;
  Pd.nToPage = (uint16)iPrn.toPage;
  Pd.nMinPage = (uint16)iPrn.minPage;
  Pd.nMaxPage = (uint16)iPrn.maxPage;
  Pd.nCopies = (uint16)iPrn.nCopies;

  memset(&Psd, 0, sizeof Psd);
  Psd.lStructSize = sizeof Psd;
  Psd.hwndOwner = Pd.hwndOwner;
  Psd.hInstance = Pd.hInstance;
  Psd.Flags = iPrn.PageSetupFlags;
  Psd.Flags &= ~PSD_ENABLEPAGESETUPTEMPLATE;

  Psd.ptPaperSize = iPrn.PaperSize;
  Psd.rtMinMargin = *(RECT*)iPrn.MinMargin;
  Psd.rtMargin = *(RECT*)iPrn.Margin;

  getDefPrn();
}
//-----------------------------------------------------------------------------
PWin* PPrinter::allocAbortWindow(PWin *parent, PPrinter* printer)
{
  return new abortPrinterWin(parent, printer);
}
//-----------------------------------------------------------------------------
HDC PPrinter::getHDC() const
{
  if(!iPrn.prnDC)
    iPrn.createDC();
  return iPrn.prnDC;
}
//-----------------------------------------------------------------------------
bool PPrinter::Print(PPrintDocBase* doc, bool prompt)
{

  Error = 1;

  HCURSOR hOrigCursor = ::SetCursor(::LoadCursor(0, IDC_WAIT));

  doc->preSetup(this);

  doc->getPageInfo(iPrn.minPage, iPrn.maxPage, iPrn.fromPage, iPrn.toPage);

  if (-1 == iPrn.fromPage) {
    iPrn.Flags |= PD_NOPAGENUMS;
    iPrn.Flags &= ~PD_PAGENUMS;
    }
  else {
    iPrn.Flags |= PD_PAGENUMS;
    iPrn.Flags &= ~PD_NOPAGENUMS;
    }
  if (prompt) {
    iPrn.Flags |= PD_RETURNDC;
//    iPrn.Flags &= ~(PD_RETURNDEFAULT | PD_PRINTSETUP);

#if 1
    Pd.Flags = iPrn.Flags;
#endif

    bool ok = setup(DlgGoPrint);

    if (!ok && Error == PDERR_DEFAULTDIFFERENT) {
      clearDev();
      ok = setup(DlgGoPrint);
      }

    if (!ok)
      return false;

    }
  else
    iPrn.createDC();

  if (!iPrn.prnDC)
    return false;

  doc->postSetup(this);

  PWin* abortWin = allocAbortWindow(Parent, this);
  abortWin->create();

  ::SetCursor(hOrigCursor);
  if(Parent)
    EnableWindow(*Parent, false);

  Aborted = false;

  SetAbortProc(iPrn.prnDC, printerAbortProc);

  int CopiesPerPass = 1;
//  int tempCopiesPerPass = iPrn.nCopies;

  if (!(iPrn.Flags & PD_COLLATE))
    SetCopyCount(iPrn.nCopies, CopiesPerPass);
  if(CopiesPerPass <= 0)
    CopiesPerPass = 1;
  int fromPage;
  int toPage;
  if (prompt && (iPrn.Flags & PD_PAGENUMS)) {
    fromPage = iPrn.fromPage;
    toPage = iPrn.toPage;
    }
  else if (iPrn.minPage) {
    fromPage = iPrn.minPage;
    toPage = iPrn.maxPage;
    }
  else {
    fromPage = 1;
    toPage = 0x7fff;
    }

  doc->beginPrinting();

  Error = 1;
  for (int copies = iPrn.nCopies; copies > 0 && Error > 0; copies -= CopiesPerPass) {
//  for (int copies = iPrn.nCopies; copies > 0 && Error > 0; copies -= tempCopiesPerPass) {

    if (CopiesPerPass > 1 && copies < CopiesPerPass)
      SetCopyCount(copies, CopiesPerPass);

    DOCINFO di = { sizeof(di), doc->getCaption(), 0, 0, 0 };

    Error = StartDoc(iPrn.prnDC, &di);

    doc->beginDocument(fromPage, toPage);

    SendMessage(*abortWin, WM_SETNUMBER, TO_PAGE, toPage);
    for (int pageNum = fromPage;
         Error > 0 && pageNum <= toPage && doc->hasPage(pageNum);
         pageNum++) {

      SendMessage(*abortWin, WM_SETNUMBER, CURR_PAGE, pageNum);

      Error = StartPage(iPrn.prnDC);

      if(Error > 0)
        doc->printPage(iPrn.prnDC, pageNum);

      if (Error > 0) {
        Error = EndPage(iPrn.prnDC);
        if (Error == 0)
          Error = 1;
        }
      }

    if (Error > 0)
      EndDoc(iPrn.prnDC);

    doc->endDocument();

    }

  doc->endPrinting();

  if (CopiesPerPass > 1)
    SetCopyCount(1, CopiesPerPass);

  if(Parent)
    EnableWindow(*Parent, true);

  delete abortWin;

  ::DeleteDC(iPrn.prnDC);
  iPrn.prnDC = 0;

  return Error > 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool PPrinter::setup(PrnSetup type)
{
  if(DlgPrintProperty == type) {

    iPrn.Unlock();

    Psd.hDevMode = iPrn.hDevMode;
    Psd.hDevNames = iPrn.hDevNames;
//    if(!(Psd.Flags & PSD_RETURNDEFAULT))
//      Psd.Flags = iPrn.PageSetupFlags;
//      Psd.ptPaperSize = iPrn.PaperSize;
//    if(!(Psd.Flags & PSD_DEFAULTMINMARGINS)) {
//      Psd.rtMinMargin = *(RECT*)iPrn.MinMargin;
//      Psd.rtMargin = *(RECT*)iPrn.Margin;
//      }

    int ret = ::PageSetupDlg(&Psd);
    if (ret) {
      setResetPageSetupFlag(PSD_MARGINS, PSD_DEFAULTMINMARGINS);
      iPrn.PageSetupFlags = Psd.Flags;
//      iPrn.PageSetupFlags |= PSD_MARGINS;
//      iPrn.PageSetupFlags &= ~PSD_DEFAULTMINMARGINS;
      Error = 0;
      iPrn.PaperSize = Psd.ptPaperSize;
      iPrn.MinMargin = Psd.rtMinMargin;
      iPrn.Margin = Psd.rtMargin;
    }
    else {
      Error = ::CommDlgExtendedError();
    }

    iPrn.hDevMode = Psd.hDevMode;
    iPrn.hDevNames = Psd.hDevNames;

    iPrn.Lock();

    return toBool(ret);
    }

  iPrn.Unlock();
  Pd.hDevMode = iPrn.hDevMode;
  Pd.hDevNames = iPrn.hDevNames;
  Pd.nFromPage = iPrn.fromPage;
  Pd.nToPage = iPrn.toPage;

  Pd.nMinPage = iPrn.minPage;
  Pd.nMaxPage = iPrn.maxPage;
  int ret = ::PrintDlg(&Pd);
  if (ret) {
    iPrn.Flags = Pd.Flags;
    Error = 0;
    iPrn.prnDC = Pd.hDC;
    iPrn.fromPage = Pd.nFromPage;
    iPrn.toPage = Pd.nToPage;
    iPrn.nCopies = Pd.nCopies;
    }
  else {
    Error = ::CommDlgExtendedError();
    }
  iPrn.hDevMode = Pd.hDevMode;
  iPrn.hDevNames = Pd.hDevNames;
  iPrn.Lock();

  return toBool(ret);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void PPrinter::clearDev()
{
  iPrn.ClearDevMode();
  iPrn.ClearDevNames();
}
//-----------------------------------------------------------------------------
bool PPrinter::getDefPrn()
{
  Psd.Flags |= PSD_RETURNDEFAULT;
  clearDev();
  return setup(DlgPrintProperty);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
POINT PPrinter::getPhysicalOffset() const
{
  HDC hdcPrn = getHDC();
  POINT offset = { GetDeviceCaps(hdcPrn, PHYSICALOFFSETX),
      GetDeviceCaps(hdcPrn, PHYSICALOFFSETY) };
  uint old = SetMapMode(hdcPrn, MM_LOMETRIC);
  DPtoLP(hdcPrn, &offset, 1);
  SetMapMode(hdcPrn, old);
  offset.y = -offset.y;
  return offset;
}
//-----------------------------------------------------------------------------
SIZE PPrinter::getPrintableSize() const
{
  HDC hdcPrn = getHDC();
  SIZE size = { GetDeviceCaps(hdcPrn, HORZSIZE), GetDeviceCaps(hdcPrn, VERTSIZE) };
  size.cx *= 10;
  size.cy *= 10;
/*
  uint old = SetMapMode(hdcPrn, MM_LOMETRIC);
  DPtoLP(hdcPrn, (POINT*)&size, 1);
  SetMapMode(hdcPrn, old);
*/
  return size;
}
//-----------------------------------------------------------------------------
//#define DEF_FLAG (PD_NOSELECTION | PD_COLLATE)
#define DEF_FLAG (PD_NOSELECTION | PD_ALLPAGES | PD_COLLATE)
//-----------------------------------------------------------------------------
PPrinter::infoPrn::infoPrn() :
    fromPage(-1), toPage(-1), minPage(-1), maxPage(-1),
    nCopies(1), Flags(DEF_FLAG),
    PageSetupFlags(PSD_DEFAULTMINMARGINS),
    prnDC(0), hDevMode(0), hDevNames(0),
    DevMode(0), DevNames(0)
{
  PaperSize.x = 0;
  PaperSize.y = 0;
}
//-----------------------------------------------------------------------------
PPrinter::infoPrn::~infoPrn()
{
  if(hDevMode) {
    GlobalUnlock(hDevMode);
    GlobalFree(hDevMode);
    }
  if(hDevNames) {
    GlobalUnlock(hDevNames);
    GlobalFree(hDevNames);
    }
  if(prnDC)
    ::DeleteDC(prnDC);
}
//-----------------------------------------------------------------------------
void PPrinter::infoPrn::Lock()
{
  if (hDevMode)
    DevMode = (DEVMODE far*)::GlobalLock(hDevMode);
  else
    DevMode = 0;
  if (hDevNames)
    DevNames = (DEVNAMES far*)::GlobalLock(hDevNames);
  else
    DevNames = 0;
}
//-----------------------------------------------------------------------------
void PPrinter::infoPrn::Unlock()
{
  if (hDevMode) {
    ::GlobalUnlock(hDevMode);
    DevMode = 0;
  }
  if (hDevNames) {
    ::GlobalUnlock(hDevNames);
    DevNames = 0;
  }
  if (prnDC) {
    ::DeleteDC(prnDC);
    prnDC = 0;
  }
}
//-----------------------------------------------------------------------------
void PPrinter::infoPrn::ClearDevMode()
{
  if (hDevMode) {
    ::GlobalUnlock(hDevMode);
    ::GlobalFree(hDevMode);
    hDevMode = 0;
    DevMode = 0;
  }
}
//-----------------------------------------------------------------------------
void PPrinter::infoPrn::SetDevMode(const DEVMODE far* devMode)
{
  ClearDevMode();
  if (devMode) {
    int size = devMode->dmSize + devMode->dmDriverExtra;
    hDevMode = ::GlobalAlloc(GHND, size);
    DevMode = (DEVMODE far*)::GlobalLock(hDevMode);
    memcpy(DevMode, devMode, size);
  }
}
//-----------------------------------------------------------------------------
void PPrinter::infoPrn::ClearDevNames()
{
  if (hDevNames) {
    ::GlobalUnlock(hDevNames);
    ::GlobalFree(hDevNames);
    hDevNames = 0;
    DevNames = 0;
  }
}
//-----------------------------------------------------------------------------
LPCTSTR PPrinter::infoPrn::GetDriverName() const
{
  return DevNames ? (LPTSTR)DevNames + DevNames->wDriverOffset : 0;
}
//-----------------------------------------------------------------------------
LPCTSTR PPrinter::infoPrn::GetDeviceName() const
{
  return DevNames ? (LPTSTR)DevNames + DevNames->wDeviceOffset : 0;
}
//-----------------------------------------------------------------------------
LPCTSTR PPrinter::infoPrn::GetOutputName() const
{
  return DevNames ? (LPTSTR)DevNames + DevNames->wOutputOffset : 0;
}
//-----------------------------------------------------------------------------
void PPrinter::infoPrn::SetDevNames(LPCTSTR driver, LPCTSTR device, LPCTSTR output)
{
  ClearDevNames();
  if (!driver || !device || !output)
    return;

  size_t size1 = _tcslen(driver) + 1;
  size_t size2 = _tcslen(device) + 1;
  size_t size3 = _tcslen(output) + 1;
  hDevNames = ::GlobalAlloc(GHND, sizeof(DEVNAMES) + size1 + size2 + size3);
  DevNames = (DEVNAMES far*)::GlobalLock(hDevNames);
  LPTSTR p = (LPTSTR)(DevNames + 1);

  DevNames->wDriverOffset = uint16(p - (LPTSTR)DevNames);
  if (driver)
    while (*driver)
      *p++ = *driver++;
  *p++ = 0;

  DevNames->wDeviceOffset = uint16(p - (LPTSTR)DevNames);
  if (device)
    while (*device)
      *p++ = *device++;
  *p++ = 0;

  DevNames->wOutputOffset = uint16(p - (LPTSTR)DevNames);
  if (output)
    while (*output)
      *p++ = *output++;
  *p = 0;

  DevNames->wDefault = false;
}
//-----------------------------------------------------------------------------
bool PPrinter::infoPrn::createDC() const
{
  if (prnDC) {
    ::DeleteDC(prnDC);
    prnDC = 0;
  }
  prnDC = ::CreateDC(GetDriverName(), GetDeviceName(),
                     GetOutputName(), GetDevMode());
  return toBool(prnDC);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CALLBACK printerAbortProc(HDC /*hDC*/, int code)
{
  getAppl()->pumpMessages();

  if(Aborted)
    return false;

  return code == 0 || code == SP_OUTOFDISK;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
abortPrinterWin::abortPrinterWin(PWin* parent, PPrinter* printer) :
  PWin(parent), Printer(printer), totPage(0), Font(D_FONT(16, 0, 0, _T("arial")))
  { }
//-----------------------------------------------------------------------------
abortPrinterWin::~abortPrinterWin()
{
  destroy();
  DeleteObject(Font);
}
//-----------------------------------------------------------------------------
enum idcCtrl { IDC_ABORT = 100, IDC_PAGE_NUM, IDC_DRIVER };

bool abortPrinterWin::create()
{
  const int w = 250;
  const int h = 120;

  int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
  int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

  Attr.x = x;
  Attr.y = y;
  Attr.w = w;
  Attr.h = h;

  Attr.style = WS_POPUP | WS_VISIBLE | WS_DLGFRAME;

  PRect r(0, 10, w, 50);

  TCHAR buff[200];

  LPCTSTR p = getStringWait();
  PStatic* txt = new PStatic(this, IDC_PAGE_NUM, r, p);
  txt->Attr.style = WS_VISIBLE | SS_CENTER | WS_CHILD;
  txt->setFont(Font);

  r.Offset(0, 20);

  p = getStringDriver();
  PPrinter::infoPrn& iPrn = Printer->getInfo();
  wsprintf(buff, p, iPrn.GetDeviceName());
  txt = new PStatic(this, IDC_DRIVER, r, buff);
  txt->Attr.style = WS_VISIBLE | SS_CENTER | WS_CHILD;
  txt->setFont(Font);

  int wBtn = w / 4;
  x = (w - wBtn) / 2;
  const int hBtn = 24;

  r.Offset(0, 55);
  r.left = x;
  r.right = x + wBtn;
  r.bottom = r.top + hBtn;

  PButton* btn = new PButton(this, IDC_ABORT, r, _T("Abort"));
  btn->setFont(Font);

  if(!PWin::create())
    return false;
  return true;
}
//-----------------------------------------------------------------------------
LRESULT abortPrinterWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_ABORT:
          setAbort();
          break;
        }
      break;
    case WM_SETNUMBER:
      do {
        int curr = 0;
        switch(wParam) {
          case TO_PAGE:
            totPage = (int)lParam;
            break;
          case CURR_PAGE:
            curr = (int)lParam;
            break;
          }
        TCHAR buff[200];
        LPCTSTR p = getStringPrinting();
        wsprintf(buff, p, curr, totPage);
        SetDlgItemText(*this, IDC_PAGE_NUM, buff);
        } while(false);
      break;
    }
  return PWin::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------------------------
