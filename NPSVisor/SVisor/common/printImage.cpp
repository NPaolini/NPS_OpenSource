//--------------- printImage.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "printImage.h"
#include "p_file.h"
#include "p_util.h"
#include "p_txt.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
printImage::printImage(PWin* par, svPrinter* printer, LPCTSTR filename) :
    Parent(par), Filename(str_newdup(filename)), Printer(printer), File(0),
    currPage(0)
{
}
//----------------------------------------------------------------------------
printImage::~printImage()
{
  delete []Filename;
  delete File;

}
//----------------------------------------------------------------------------
bool printImage::beginDoc(bool showSetup)
{
  if(!Printer)
    return false;
  delete File;
  File = new P_File(Filename, P_CREAT);
  if(!File->P_open())
    return false;

  OOP.reset();
  OOP[0] = 0;
  bool result = true;
  if(showSetup) {
    Printer->setResetPageSetupFlag(0, PSD_RETURNDEFAULT);
    result = Printer->setup(PPrinter::DlgPrintProperty);
    }
  currPage = 0;

  return result;
}
//----------------------------------------------------------------------------
bool printImage::makeHeader()
{
  OOP[currPage] = (long)File->get_len();

  TCHAR t[BUFF_DIM_PAGE + 2];
  fillStr(t, 0, BUFF_DIM_PAGE);
  File->P_write(t, BUFF_DIM_PAGE * sizeof(t[0]));
  return true;
}
//----------------------------------------------------------------------------
int toDecMm(int value, bool hMm)
{
  if(hMm)
    return value / 10;
  value *= 254;
  return value / 1000;
}
//----------------------------------------------------------------------------
#define CONV_TO_DMM(v) v = toDecMm((v), isOnHMm)
//----------------------------------------------------------------------------
LPCTSTR setMargin(LPCTSTR p, int& m)
{
  if(p) {
    int v = _ttoi(p);
    if(v > 0)
      m = v;
    p = findNextParam(p, 1);
    }
  return p;
}
//----------------------------------------------------------------------------
bool printImage::makeBody()
{
  PPrinter::infoPrn& ip = Printer->getInfo();
  PRect r(ip.Margin);
  bool isOnHMm = !(PSD_INTHOUSANDTHSOFINCHES & ip.PageSetupFlags);
  CONV_TO_DMM(r.left);
  CONV_TO_DMM(r.right);
  CONV_TO_DMM(r.top);
  CONV_TO_DMM(r.bottom);

  LPCTSTR p = getString(ID_MARGIN_PRINTSCREEN);
  p = setMargin(p, r.left);
  p = setMargin(p, r.top);
  p = setMargin(p, r.right);
  p = setMargin(p, r.bottom);

  POINT psz = { r.left, r.top };
#if 0
  HDC hdcPrn = Printer->getHDC();
  POINT offset = { GetDeviceCaps(hdcPrn, PHYSICALOFFSETX),
      -GetDeviceCaps(hdcPrn, PHYSICALOFFSETY) };
  DPtoLP(hdcPrn, &offset, 1);
  CONV_TO_DMM(offset.x);
  CONV_TO_DMM(offset.y);

  psz.x += offset.x;
  psz.y += offset.y;
#endif
  SIZE sz = *(SIZE*)&ip.PaperSize;
  CONV_TO_DMM(sz.cx);
  CONV_TO_DMM(sz.cy);
  sz.cx -= psz.x + r.right;
  sz.cy -= psz.y + r.bottom;
  double pScale = sz.cy;
  pScale /= (double)sz.cx;

  int iHeight;
  int iWidth;
  do {
    P_File pf(DEF_BMP_NAME_4_PRINT, P_READ_ONLY);
    if(!pf.P_open())
      return false;
    BITMAPFILEHEADER bmfh;
    pf.P_read(&bmfh, LEN_BMPFILEHEADER);

    BITMAPINFOHEADER bmih;
    pf.P_read(&bmih, sizeof(BITMAPINFOHEADER));

    iWidth = bmih.biWidth;
    iHeight = bmih.biHeight;
    } while(false);
  if(iHeight <= 0 || iWidth <= 0)
    return false;
  double iScale = iHeight;
  iScale /= (double)iWidth;

  int x = psz.x;
  int y = psz.y;
  int w;
  int h;
  if(pScale > iScale) {
    w = sz.cx;
    h = (int)(w * iScale);
    }
  else {
    h = sz.cy;
    w = (int)(h / iScale);
    }
  int result;
  TCHAR buff[512];
  PRINT6_A_S(File, buff, _T("%d,%d,%d,%d,%d,%s\r\n"), PRN_BMP, x, y, w, h, DEF_BMP_NAME_4_PRINT)
//  wsprintf(buff, _T("%d,%d,%d,%d,%d,%s\r\n"), PRN_BMP, x, y, w, h, DEF_BMP_NAME_4_PRINT);
//  File->P_write(buff, _tcslen(buff) * sizeof(buff[0]));
  return true;
}
//----------------------------------------------------------------------------
bool printImage::makeFooter()
{
  long pos = OOP[currPage];
  ++currPage;
  File->P_seek(pos);

  long dim = (long)File->get_len() - pos - BUFF_DIM_PAGE  * sizeof(TCHAR);
  TCHAR t[BUFF_DIM_PAGE + 2];
  wsprintf(t, _T("%d"), dim);
  File->P_write(t, BUFF_DIM_PAGE * sizeof(t[0]));
  File->P_seek(0, SEEK_END_);
  return true;
}
//----------------------------------------------------------------------------
void printImage::run(bool preview)
{
  do {
    makeHeader();
    makeBody();
    makeFooter();
    delete File;
    File = 0;
    } while(false);

  svPrintFile ppf(Filename, OOP, Parent, Printer, getTitle(), false);
  if(preview)
    ppf.PrintPreview();
  else
    ppf.Print();
//    ppf.Print(true);

  DeleteFile(DEF_BMP_NAME_4_PRINT);
}
//----------------------------------------------------------------------------
LPCTSTR printImage::getTitle()
{
  static TCHAR title[] = _T("NPS_PrintImage");
  return title;
}
