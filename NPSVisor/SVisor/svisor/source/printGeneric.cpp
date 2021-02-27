//--------------- printGeneric.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "printGeneric.h"
#include "p_file.h"
#include "p_util.h"
#include "p_txt.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
printGeneric::printGeneric(PWin* par, svPrinter* printer, LPCTSTR filename) :
    Parent(par), Filename(str_newdup(filename)), Printer(printer), File(0),
    currPage(0)
{
}
//----------------------------------------------------------------------------
printGeneric::~printGeneric()
{
  delete []Filename;
  delete File;

}
//----------------------------------------------------------------------------
static
int toDecMm(int value, bool hMm)
{
  if(hMm)
    return value / 10;
  value *= 254;
  return value / 1000;
}
//----------------------------------------------------------------------------
static void rotateRect(PRect& r, bool clockwise)
{
  if(clockwise) {
    int t = r.left;
    r.left = r.bottom;
    r.bottom = r.right;
    r.right = r.top;
    r.top = t;
    }
  else {
    int t = r.left;
    r.left = r.top;
    r.top = r.right;
    r.right = r.bottom;
    r.bottom = t;
    }
}
//----------------------------------------------------------------------------
#define CONV_TO_DMM(v) v = toDecMm((v), isOnHMm)
//----------------------------------------------------------------------------
bool printGeneric::beginDoc(bool showSetup)
{
  if(!Printer)
    return false;
  delete File;
  File = new P_File(Filename, P_CREAT);
  if(!File->P_open())
    return false;

  OOP.reset();
  OOP[0] = 0;
  if(showSetup) {
    Printer->setResetPageSetupFlag(0, PSD_RETURNDEFAULT);
    Printer->setup(PPrinter::DlgPrintProperty);
    }
  currPage = 0;

  PPrinter::infoPrn& ip = Printer->getInfo();
  PRect r(ip.Margin);

#if 0
  const DEVMODE *devMode = Printer->getInfo().GetDevMode();

  if(DMORIENT_LANDSCAPE == devMode->dmOrientation)
    rotateRect(r, true);
#endif
  bool isOnHMm = !(PSD_INTHOUSANDTHSOFINCHES & ip.PageSetupFlags);
  CONV_TO_DMM(r.left);
  CONV_TO_DMM(r.right);
  CONV_TO_DMM(r.top);
  CONV_TO_DMM(r.bottom);

  SIZE sz = *(SIZE*)&ip.PaperSize;
  CONV_TO_DMM(sz.cx);
  CONV_TO_DMM(sz.cy);

  pageForm.left = r.left;
  pageForm.top  = r.top;
  pageForm.right = sz.cx - r.right;
  pageForm.bottom = sz.cy - r.bottom;

  MAX_Y = pageForm.bottom;

  return initializePrinter(Printer);
}
//----------------------------------------------------------------------------
void printGeneric::getMargin(PRect& r)
{
  PPrinter::infoPrn& ip = Printer->getInfo();
  r = ip.Margin;
  bool isOnHMm = !(PSD_INTHOUSANDTHSOFINCHES & ip.PageSetupFlags);
  CONV_TO_DMM(r.left);
  CONV_TO_DMM(r.right);
  CONV_TO_DMM(r.top);
  CONV_TO_DMM(r.bottom);
}
//----------------------------------------------------------------------------
void printGeneric::setMargin(const PRect& r)
{
  PPrinter::infoPrn& ip = Printer->getInfo();
  SIZE sz = *(SIZE*)&ip.PaperSize;
  bool isOnHMm = !(PSD_INTHOUSANDTHSOFINCHES & ip.PageSetupFlags);

  CONV_TO_DMM(sz.cx);
  CONV_TO_DMM(sz.cy);

  pageForm.left = r.left;
  pageForm.top  = r.top;
  pageForm.right = sz.cx - r.right;
  pageForm.bottom = sz.cy - r.bottom;

  MAX_Y = pageForm.bottom;
}
//----------------------------------------------------------------------------
bool printGeneric::makeHeader()
{
  OOP[currPage] = (long)File->get_len();

  TCHAR t[BUFF_DIM_PAGE + 2];
  fillStr(t, 0, BUFF_DIM_PAGE);
  File->P_write(t, BUFF_DIM_PAGE * sizeof(t[0]));
  Y = pageForm.top;
  return true;
}
//----------------------------------------------------------------------------
bool printGeneric::makeBody()
{
  if(!makeHeader())
    return false;
  int result = 0;
  while(result >= 0) {
    result = addRow();
    if(!result)
      return false;
    if(Y >= MAX_Y) {
      if(!makeFooter())
        return false;
      if(result >= 0)
        if(!makeHeader())
          return false;
      }
    }
  return true;
}
//----------------------------------------------------------------------------
bool printGeneric::makeFooter()
{
  long pos = OOP[currPage];
  ++currPage;
  File->P_seek(pos);

  long dim = (long)File->get_len() - pos - BUFF_DIM_PAGE  * sizeof(TCHAR);
  TCHAR t[BUFF_DIM_PAGE + 2];
  ZeroMemory(t, sizeof(t));
  wsprintf(t, _T("%d"), dim);
  File->P_write(t, BUFF_DIM_PAGE * sizeof(t[0]));
  File->P_seek(0, SEEK_END_);
  return true;
}
//----------------------------------------------------------------------------
void printGeneric::Setup()
{
  Printer->setResetPageSetupFlag(0, PSD_RETURNDEFAULT);
  Printer->setup(PPrinter::DlgPrintProperty);
}
//----------------------------------------------------------------------------
void printGeneric::Setup(svPrinter* printer)
{
  printer->setResetPageSetupFlag(0, PSD_RETURNDEFAULT);
  printer->setup(PPrinter::DlgPrintProperty);
}
//----------------------------------------------------------------------------
void printGeneric::run(bool preview, bool setup, LPCTSTR alternateBitmapOrID4Tool)
{
  do {
    if(!beginDoc(setup))
      return;
    bool success = makeBody();
    delete File;
    File = 0;
    if(!success)
      return;
    } while(false);

  svPrintFile ppf(Filename, OOP, Parent, Printer, getTitle(), false);

  if(preview)
    ppf.PrintPreview(alternateBitmapOrID4Tool);
  else
    ppf.Print();
}
//----------------------------------------------------------------------------
LPCTSTR printGeneric::getTitle()
{
  static TCHAR title[] = _T("NPS_PrintDoc");
  return title;
}
