//-------------------- pPrintPage.cpp -----------------------------------------
//-----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------------
#include <stdio.h>
#include "pPrintPage.h"
#include "pDialog.h"
#include "pWinTool.h"
#include "defin.h"
#include "p_file.h"
#include "p_util.h"
#include "pTextPanel.h"
#include "resource.h"
//-----------------------------------------------------------------------------
//#define NDEBUG
#ifdef _DEBUG
  #include <assert.h>
  #define ASSERT__(a) assert(a)
#else
  #define ASSERT__(a) if(!(a)) return
#endif
#include <stdlib.h>
//-----------------------------------------------------------------------------
static void reScale(HDC hdc, HDC prnDc);
//-----------------------------------------------------------------------------
LPCTSTR get_next(LPCTSTR p, int &dim)
{
  LPCTSTR  t= _tcschr(p, _T(','));// ,dim);
  if(t) {
    dim -= int(t - p);
    ++t;
    }
  return t;
}
//-----------------------------------------------------------------------------
LPTSTR get_line(LPTSTR buff,int len)
{
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if((unsigned)*p <= 13)
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  ++i;
  if((unsigned)*p <= 13 && *p != c) {
    ++p;
    ++i;
    }
  return(i < len ? p : 0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PPrintPage::PPrintPage(PWin* parent, PPrinter* allocated, LPCTSTR title, bool autoDeletePrinter)//, HINSTANCE hinstance)
:
//    PWin(parent, title, hinstance),
    Parent(parent), Title(str_newdup(title)),
    currPainting(-1),
    preview(false), Printer(allocated ? allocated : new PPrinter(parent)),
    autoDeletePrinter(allocated ? autoDeletePrinter : true)

{
  // INSERT>> Your constructor code here.
//  Attr.x = 10;
//  Attr.y = 10;
//  Attr.w = 10000;
//  Attr.h = 10000;
//  Attr.style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
}
//-----------------------------------------------------------------------------
PPrintPage::~PPrintPage()
{
//  destroy();
  delete []Title;
//  delete PreviewWin;
//  PreviewWin = 0;
  if(autoDeletePrinter)
    delete Printer;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//#define TEST_LAYOUT
#ifdef TEST_LAYOUT
void PPrintPage::initializePrinter(PPrinter *printer)
{
  const DEVMODE *devMode = printer->getInfo().GetDevMode();

  int dim = devMode->dmSize + devMode->dmDriverExtra;
  char *pDev = new char[dim];
  DEVMODE *dev = (DEVMODE*) pDev;
  memcpy(dev, devMode, dim);

  // attiva l'orientamento orizzontale
  dev->dmOrientation = DMORIENT_LANDSCAPE;

  // attiva l'orientamento verticale
//  dev->dmOrientation = DMORIENT_PORTRAIT;

  // attiva il settaggio delle dimensioni personalizzate
  dev->dmFields |= DM_PAPERLENGTH | DM_PAPERWIDTH | DM_ORIENTATION;

  // attiva il formato personalizzato e la dimensione
  dev->dmPaperSize = DMPAPER_USER;
  dev->dmPaperLength = 4500;
  dev->dmPaperWidth = 2100;

  printer->getInfo().SetDevMode(dev);
  delete []pDev;
}
//-----------------------------------------------------------------------------
#else
//-----------------------------------------------------------------------------
void PPrintPage::initializePrinter(PPrinter* /*printer*/)
{
/*
  const DEVMODE *devMode = printer->getInfo().GetDevMode();

  int dim = devMode->dmSize + devMode->dmDriverExtra;
  char *pDev = new char[dim];
  DEVMODE *dev = (DEVMODE*) pDev;
  memcpy(dev, devMode, dim);

  // attiva l'orientamento orizzontale
  dev->dmOrientation = DMORIENT_LANDSCAPE;

  // attiva l'orientamento verticale
//  dev->dmOrientation = DMORIENT_PORTRAIT;

  // disattiva le dimensioni personalizzate
  dev->dmFields &= ~(DM_PAPERLENGTH | DM_PAPERWIDTH);

  // attiva il formato fisso e la dimensione in A4
  dev->dmFields |= DM_PAPERSIZE | DM_ORIENTATION;
  dev->dmPaperSize = DMPAPER_A4;

  printer->getInfo().SetDevMode(dev);
  delete []pDev;
*/
}
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void PPrintPage::Print(bool showDialog)
{
  if(!Printer)
    return;
  initializePrinter(Printer);
  Printer->Print(this, showDialog);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void PPrintPage::paintPage(HDC hdc)
{
  HDC prnDC = getPrnDC();
  SetMapMode(prnDC, MM_LOMETRIC);
  if(hdc != prnDC)
    reScale(hdc, prnDC);
}
//-----------------------------------------------------------------------------
void PPrintPage::printPage(HDC hdc, uint which_page)
{
  currPainting = which_page;
  paintPage(hdc);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// deve essere creato e distrutto nello spazio di vita del pDC
class print_token
{
  public:
  enum tk_error { NO_ERR, NO_TOKEN, END_BUFF };
   print_token(const POINT& pt, HDC hdc, bool preview) : pDC(hdc), Offset(pt), Bmp(0), Font(0),
          Pen(0), Brush(0), preview(preview) { }
   ~print_token();

    tk_error elab_token(LPCTSTR t,int dim);

  private:
    void paint_txt(LPCTSTR p, int dim);
    void paint_line(LPCTSTR p, int dim);
    void paint_polyline(LPCTSTR p, int dim);
    void paint_box(LPCTSTR p, int dim);
    void paint_bmp(LPCTSTR p, int dim);
    void set_font(LPCTSTR p, int dim);
    void set_font2(LPCTSTR p, int dim);
    void set_color_fg(LPCTSTR p, int dim);
    void set_color_bkg(LPCTSTR p, int dim);
    void set_align(LPCTSTR p, int dim);
    void set_bkmode(LPCTSTR p, int dim);
    void set_pen(LPCTSTR p, int dim);
    void set_brush1(LPCTSTR p, int dim);
    void set_brush2(LPCTSTR p, int dim);
    void set_brush3(LPCTSTR p, int dim);

    void paint_txt_onBox(LPCTSTR p, int dim);
    void paint_rect(LPCTSTR p, int dim);
    void paint_oval(LPCTSTR p, int dim);

    void paint_null_brush(LPCTSTR p, int dim);
    void paint_round_rect(LPCTSTR p, int dim);
    void paint_arc(LPCTSTR p, int dim);
    void paint_pie(LPCTSTR p, int dim);
    void paint_cord(LPCTSTR p, int dim);

    HDC pDC;
//    PWin *W;
    POINT Offset;
    PBitmap *Bmp;
    HGDIOBJ Font;
    HGDIOBJ Pen;
    HGDIOBJ Brush;
    bool preview;
};
//-----------------------------------------------------------------------------
print_token::~print_token()
{
  delete Bmp;
  if(Font)
     DeleteObject(SelectObject(pDC, (HGDIOBJ)Font));

  if(Pen)
     DeleteObject(SelectObject(pDC, (HGDIOBJ)Pen));

  if(Brush)
     DeleteObject(SelectObject(pDC, (HGDIOBJ)Brush));
}
//-----------------------------------------------------------------------------
void print_token::paint_txt(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  TextOut(pDC, x, -y, p, (int)_tcslen(p));
}
//-----------------------------------------------------------------------------
void print_token::paint_txt_onBox(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;

  p = get_next(p, dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;

  p = get_next(p, dim);
  ASSERT__(p != NULL);
  int w = _ttoi(p);

  p = get_next(p, dim);
  ASSERT__(p != NULL);
  int h = _ttoi(p);

  p = get_next(p, dim);
  ASSERT__(p != NULL);
  int align = _ttoi(p);
  align |= DT_WORDBREAK | DT_EXPANDTABS;

  p = get_next(p, dim);
  ASSERT__(p != NULL);

  PRect rect(x, -y, x + w, -(y + h));
  DrawText(pDC, p, -1, rect, align);
}
//----------------------------------------------------------
void print_token::paint_line(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  MoveToEx(pDC, x, -y, 0);
  x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  y = _ttoi(p) - Offset.y;
  LineTo(pDC, x, -y);
}
//-----------------------------------------------------------------------------
void print_token::paint_polyline(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int nLine = _ttoi(p);
  p = get_next(p, dim);
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;
  MoveToEx(pDC, x, -y, 0);
  for(int i = 0; i < nLine; ++i) {
    p = get_next(p,dim);
    ASSERT__(p != NULL);
    x = _ttoi(p) - Offset.x;
    p = get_next(p,dim);
    ASSERT__(p != NULL);
    y = _ttoi(p) - Offset.y;
    LineTo(pDC, x, -y);
    }
}
//-----------------------------------------------------------------------------
//----------------------------------------------------------
void print_token::paint_rect(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int w = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int h = _ttoi(p);

  Rectangle(pDC, x, -y, x + w, -(y + h));
}
//----------------------------------------------------------
void print_token::paint_oval(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;

  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;

  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int w = _ttoi(p);

  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int h = _ttoi(p);

  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int filled = _ttoi(p);

  if(filled)
    Ellipse(pDC, x, -y, x + w, -(y + h));
  else
    Arc(pDC, x, -y, x + w, -(y + h), 0, 0, 0, 0);
}
//----------------------------------------------------------
void print_token::paint_box(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int w = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int h = _ttoi(p);
#if 0
  MoveToEx(pDC, x, y, 0);
  LineTo(pDC, x + w, y);
  LineTo(pDC, x + w, (y + h));
  LineTo(pDC, x, (y + h));
  LineTo(pDC, x, y);
#else
  MoveToEx(pDC, x, -y, 0);
  LineTo(pDC, x + w, -y);
  LineTo(pDC, x + w, -(y + h));
  LineTo(pDC, x, -(y + h));
  LineTo(pDC, x, -y);
#endif
}
//-----------------------------------------------------------------------------
static int reverseColor(int c)
{
  int r = uint(c >> 16) & 0xff;
  int g = c & 0xff00;
  int b = uint(c & 0xff) << 16;
  return r | g | b;
}
//-----------------------------------------------------------------------------
template <typename T>
LPCTSTR get_color_(LPCTSTR p, T& color, uint hasEnoughParam = 0)
{
  color = 0;
  if(_istdigit(*p)) {
    color = _ttol(p);
    bool checkOther = true;
    if(hasEnoughParam) {
      LPCTSTR p2 = findNextParam(p, hasEnoughParam);
      if(!p2)
        checkOther = false;
      }
    if(checkOther) {
      p = findNextParamTrim(p);
      if(p) {
        color += _ttol(p) << 8;
        p = findNextParamTrim(p);
        if(p)
          color += _ttol(p) << 16;
        }
      }
    }
  else if(_T('#') == *p) {
    _stscanf_s(p + 1, _T("%x"), &color);
    color = reverseColor(color);
    }
  return p;
}
//-----------------------------------------------------------------------------
void print_token::set_pen(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  DWORD color;
  p = get_color_(p, color, 4);
  p = get_next(p, dim);
  ASSERT__(p != NULL);
  int width = _ttoi(p);
  p = get_next(p, dim);
  ASSERT__(p != NULL);
  int style = _ttoi(p);
  HPEN pen = CreatePen(style, width, color);
  if(Pen)
    DeleteObject(SelectObject(pDC, (HGDIOBJ)pen));
  else
    Pen = SelectObject(pDC, (HGDIOBJ)pen);
}
//-----------------------------------------------------------------------------
void print_token::set_brush1(LPCTSTR p, int /*dim*/)
{
  ASSERT__(p != NULL);
  DWORD color;
  p = get_color_(p, color);
  HBRUSH brush = CreateSolidBrush(color);
  if(Brush)
    DeleteObject(SelectObject(pDC, (HGDIOBJ)brush));
  else
    Brush = SelectObject(pDC, (HGDIOBJ)brush);
}
//-----------------------------------------------------------------------------
void print_token::set_brush2(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  DWORD color;
  p = get_color_(p, color, 3);
  p = get_next(p, dim);
  ASSERT__(p != NULL);
  int style = _ttoi(p);
  HBRUSH brush = CreateHatchBrush(style, color);
  if(Brush)
    DeleteObject(SelectObject(pDC, (HGDIOBJ)brush));
  else
    Brush = SelectObject(pDC, (HGDIOBJ)brush);
}
//-----------------------------------------------------------------------------
void print_token::set_brush3(LPCTSTR p, int /*dim*/)
{
  ASSERT__(p != NULL);
  delete Bmp;
  Bmp = new PBitmap(p);
  if(Bmp->isValid()) {
    HBRUSH brush = CreatePatternBrush(*Bmp);
    if(Brush)
      DeleteObject(SelectObject(pDC, (HGDIOBJ)brush));
    else
      Brush = SelectObject(pDC, (HGDIOBJ)brush);
    }
}
//-----------------------------------------------------------------------------
void print_token::paint_null_brush(LPCTSTR p, int dim)
{
  LOGBRUSH lb = { BS_NULL, 0, 0 };

  HBRUSH brush = CreateBrushIndirect(&lb);
  if(Brush)
    DeleteObject(SelectObject(pDC, (HGDIOBJ)brush));
  else
    Brush = SelectObject(pDC, (HGDIOBJ)brush);
}
//-----------------------------------------------------------------------------
void print_token::paint_round_rect(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int w = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int h = _ttoi(p);

  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int cx = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int cy = _ttoi(p);

  RoundRect(pDC, x, -y, x + w, -(y + h), cx, cy);
}
//-----------------------------------------------------------------------------
typedef BOOL (__stdcall *fzFig)(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect,
      int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2);
//-----------------------------------------------------------------------------
static
void paint_Fig(HDC pDC, LPCTSTR p, int dim, const POINT& Offset, fzFig fzP)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int w = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int h = _ttoi(p);

  int ofX = x + w / 2;
  int ofY = y + h / 2;

  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int cx1 = _ttoi(p) + ofX;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int cy1 = -_ttoi(p) + ofY;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int cx2 = _ttoi(p) + ofX;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int cy2 = -_ttoi(p) + ofY;


  fzP(pDC, x, -y, x + w, -(y + h), cx1, -cy1, cx2, -cy2);
}
//-----------------------------------------------------------------------------
void print_token::paint_arc(LPCTSTR p, int dim)
{
  paint_Fig(pDC, p, dim, Offset, Arc);
}
//-----------------------------------------------------------------------------
void print_token::paint_pie(LPCTSTR p, int dim)
{
  paint_Fig(pDC, p, dim, Offset, Pie);
}
//-----------------------------------------------------------------------------
void print_token::paint_cord(LPCTSTR p, int dim)
{
  paint_Fig(pDC, p, dim, Offset, Chord);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**/
#define GDIPLUS_PRINT
//-----------------------------------------------------------------------------
#ifndef GDIPLUS_PRINT
//-----------------------------------------------------------------------------
WORD DibNumColors (VOID FAR * pv)
{
  INT bits;
  LPBITMAPINFOHEADER  lpbi;
  LPBITMAPCOREHEADER  lpbc;
  lpbi = ((LPBITMAPINFOHEADER)pv);
  lpbc = ((LPBITMAPCOREHEADER)pv);

  if (lpbi->biSize != sizeof(BITMAPCOREHEADER)) {
    if (lpbi->biClrUsed != 0)
      return (WORD)lpbi->biClrUsed;
    bits = lpbi->biBitCount;
    }
  else
    bits = lpbc->bcBitCount;
  switch (bits) {
    case 1:
      return 2;
    case 4:
      return 16;
    case 8:
      return 256;
    default:
      return 0;
    }
}
//-----------------------------------------------------------------------------
WORD PaletteSize(VOID FAR * pv)
{
  LPBITMAPINFOHEADER lpbi;
  WORD NumColors;
  lpbi = (LPBITMAPINFOHEADER)pv;
  NumColors = DibNumColors(lpbi);
  if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
    return (WORD)(NumColors * sizeof(RGBTRIPLE));
  else
    return (WORD)(NumColors * sizeof(RGBQUAD));
}
//-----------------------------------------------------------------------------
#define WIDTHBYTES(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
//-----------------------------------------------------------------------------
HANDLE DibFromBitmap(HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal)
{
  BITMAP bm;
  BITMAPINFOHEADER     bi;
  BITMAPINFOHEADER FAR *lpbi;
  DWORD                dwLen;
  HANDLE               hdib;
  HANDLE               h;
  HDC                  hdc;
  if (!hbm)
    return NULL;
  if (hpal == NULL)
    hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
  GetObject(hbm,sizeof(bm),(LPSTR)&bm);
  if (biBits == 0)
    biBits =  (WORD)(bm.bmPlanes * bm.bmBitsPixel);
  bi.biSize               = sizeof(BITMAPINFOHEADER);
  bi.biWidth              = bm.bmWidth;
  bi.biHeight             = bm.bmHeight;
  bi.biPlanes             = 1;
  bi.biBitCount           = biBits;
  bi.biCompression        = biStyle;
  bi.biSizeImage          = 0;
  bi.biXPelsPerMeter      = 0;
  bi.biYPelsPerMeter      = 0;
  bi.biClrUsed            = 0;
  bi.biClrImportant       = 0;
  dwLen  = bi.biSize + PaletteSize(&bi);
  hdc = GetDC(NULL);
  hpal = SelectPalette(hdc,hpal,FALSE);
  RealizePalette(hdc);
  hdib = GlobalAlloc(GHND,dwLen);
  if (!hdib){
    SelectPalette(hdc,hpal,FALSE);
    ReleaseDC(NULL,hdc);
    return NULL;
    }
  lpbi = (BITMAPINFOHEADER FAR *)(VOID FAR *)GlobalLock(hdib);
  *lpbi = bi;

  GetDIBits(hdc, hbm, 0L, (DWORD)bi.biHeight, (LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);
  bi = *lpbi;
  GlobalUnlock(hdib);
  if (bi.biSizeImage == 0) {
    bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;
    if (biStyle != BI_RGB)
      bi.biSizeImage = (bi.biSizeImage * 3) / 2;
    }
  dwLen = bi.biSize + PaletteSize(&bi) + bi.biSizeImage;
  if((h = GlobalReAlloc(hdib,dwLen,0)) != 0)
    hdib = h;
  else {
    GlobalFree(hdib);
    hdib = NULL;
    SelectPalette(hdc,hpal,FALSE);
    ReleaseDC(NULL,hdc);
    return hdib;
    }
  lpbi = (BITMAPINFOHEADER FAR *)(VOID FAR *)GlobalLock(hdib);
  if (GetDIBits( hdc, hbm, 0L, (DWORD)bi.biHeight, (LPBYTE)lpbi + (WORD)lpbi->biSize +
            PaletteSize(lpbi), (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS) == 0) {
    GlobalUnlock(hdib);
//    hdib = NULL;
    SelectPalette(hdc,hpal,FALSE);
    ReleaseDC(NULL,hdc);
    return NULL;
    }
  bi = *lpbi;
  GlobalUnlock(hdib);
  SelectPalette(hdc,hpal,FALSE);
  ReleaseDC(NULL,hdc);
  return hdib;
}
//-----------------------------------------------------------------------------
BOOL StretchDibBlt(HDC hdc, INT x, INT y, INT dx, INT dy, HANDLE hdib, INT dx0, INT dy0)
{
  LPBITMAPINFOHEADER lpbi;
  LPSTR        pBuf;
  BOOL         f;
  if (!hdib)
    return PatBlt(hdc, x, y, dx, dy, SRCCOPY);
  lpbi = (LPBITMAPINFOHEADER)(VOID FAR *)GlobalLock(hdib);
  if (!lpbi)
    return FALSE;
  pBuf = (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi);
  f = StretchDIBits(hdc, x, y, dx, dy, 0, 0, dx0, dy0, pBuf, (LPBITMAPINFO)lpbi,
        DIB_RGB_COLORS, SRCCOPY);
  GlobalUnlock(hdib);
  return f;
}
/**/
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void print_token::paint_bmp(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int x = _ttoi(p) - Offset.x;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int y = _ttoi(p) - Offset.y;
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int w = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int h = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);

  PBitmap bmp(p);
  if(!bmp.isValid())
    return;
  SIZE sz = { w, -h };
  POINT pt = { x, -y };
#if 0
  int cap = GetDeviceCaps(pDC, RASTERCAPS);
  #define T_T(a) { a, #a }
  struct {
    int c;
    LPCTSTR t;
    } caps[] = {
   T_T(RC_BANDING),
   T_T(RC_BITBLT),
   T_T(RC_BITMAP64),
   T_T(RC_DI_BITMAP),
   T_T(RC_DIBTODEV),
   T_T(RC_FLOODFILL),
   T_T(RC_GDI20_OUTPUT),
   T_T(RC_PALETTE),
   T_T(RC_SCALING),
   T_T(RC_STRETCHBLT),
   T_T(RC_STRETCHDIB)
   };

  for(int i = 0; i < SIZE_A(caps); ++i) {
    LPCTSTR has;
    if(cap & caps[i].c)
      has = _T("Capable ");
    else
      has = _T("Not Capable ");

    MessageBox(0, caps[i].t, has, MB_OK);
    }
#endif
#ifdef GDIPLUS_PRINT
  if(preview)
    bmp.draw(pDC, pt, sz);
  else {
#define dMM_TO_PRINTER_X(h) (float((h) * 300.0 / dpiX))
#define dMM_TO_PRINTER_Y(h) (float((h) * 300.0 / dpiY))
    Gdiplus::Graphics graphics(pDC);
    Gdiplus::REAL dpiX = graphics.GetDpiX();
    Gdiplus::REAL dpiY = graphics.GetDpiY();

    Gdiplus::RectF r(dMM_TO_PRINTER_X(pt.x), dMM_TO_PRINTER_Y(pt.y), dMM_TO_PRINTER_X(sz.cx), dMM_TO_PRINTER_Y(sz.cy));
    graphics.SetPageUnit(Gdiplus::UnitDocument);
    Gdiplus::ImageAttributes imAtt;
    imAtt.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    const p_gdiplusBitmap m_pBitmap = bmp.getManBmp();
    graphics.DrawImage(m_pBitmap, r, 0, 0, (Gdiplus::REAL)m_pBitmap->GetWidth(), (Gdiplus::REAL)m_pBitmap->GetHeight(), Gdiplus::UnitPixel, &imAtt);
    }
#else
  if(preview)
    bmp.draw(pDC, pt, sz);
  else {
    SIZE szBmp = bmp.getSize();
    HANDLE hdib = DibFromBitmap(bmp, 0, 0, 0);
    if(hdib) {
      StretchDibBlt(pDC, pt.x, pt.y, sz.cx, sz.cy, hdib, szBmp.cx, szBmp.cy);
      GlobalFree(hdib);
      }
    }
#endif
}
//-----------------------------------------------------------------------------
#define INCH_TO_MM(h) MulDiv((h), 254, 100)
//-----------------------------------------------------------------------------
void print_token::set_font(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int h = _ttoi(p);
  h = -INCH_TO_MM(h);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int l = _ttoi(p);
  l = INCH_TO_MM(l);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int flag = _ttoi(p);
  p = get_next(p, dim);
  if(!p || *p == '*')
    p = _T("arial");
  HFONT font = D_FONT(h, l, flag, p);

  if(Font)
    DeleteObject(SelectObject(pDC, (HGDIOBJ)font));
  else
    Font = SelectObject(pDC, (HGDIOBJ)font);
}
//-----------------------------------------------------------------------------
void print_token::set_font2(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int h = _ttoi(p);
  h = -INCH_TO_MM(h);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int l = _ttoi(p);
  l = INCH_TO_MM(l);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int escp = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  int flag = _ttoi(p);

  p = get_next(p, dim);
  if(!p || *p == '*')
    p = _T("arial");

  HFONT font = D_FONT_ORIENT(h, l, escp, flag, p);

  if(Font)
    DeleteObject(SelectObject(pDC, (HGDIOBJ)font));
  else
    Font = SelectObject(pDC, (HGDIOBJ)font);
}
//-----------------------------------------------------------------------------
void print_token::set_color_fg(LPCTSTR p, int /*dim*/)
{
  ASSERT__(p != NULL);
  DWORD color;
  p = get_color_(p, color);
  SetTextColor(pDC, color);
}
//-----------------------------------------------------------------------------
void print_token::set_color_bkg(LPCTSTR p, int /*dim*/)
{
  ASSERT__(p != NULL);
  DWORD color;
  p = get_color_(p, color);
  SetBkColor(pDC, color);
}
//-----------------------------------------------------------------------------
void print_token::set_align(LPCTSTR p, int dim)
{
  ASSERT__(p != NULL);
  int horz,vert;
  horz = _ttoi(p);
  p = get_next(p,dim);
  ASSERT__(p != NULL);
  vert = _ttoi(p);
  SetTextAlign(pDC, horz | vert);
}
//-----------------------------------------------------------------------------
void print_token::set_bkmode(LPCTSTR p, int /*dim*/)
{
  ASSERT__(p != NULL);
  int mode = _ttoi(p);
  SetBkMode(pDC, mode);
//  if(mode == OPAQUE)
//    pDC->SetBkColor(TColor::White);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PPrintFile::PPrintFile(LPCTSTR file, offsetOfPage& page, PWin* parent,
                PPrinter* allocated, LPCTSTR title, bool autoDeletePrinter)
:
    PPrintPage(parent, allocated, title, autoDeletePrinter),
    Page(page), Buff(0), dimPage(0), maxDimPage(0)
{
  File = new P_File(file, P_READ_ONLY);
  File->P_open();
  if(!Page.getElem())
    fillOffsetPage();
  setTotPages(Page.getElem());
}
//-----------------------------------------------------------------------------
PPrintFile::~PPrintFile()
{
//  destroy();

  delete File;
  delete []Buff;
}
//-----------------------------------------------------------------------------
void PPrintFile::fillOffsetPage()
{
  TCHAR buff[BUFF_DIM_PAGE + 1];
  buff[BUFF_DIM_PAGE] = 0;
  long curr = 0;
  int pages = 0;
  while(true) {
    if(BUFF_DIM_PAGE_BYTE != File->P_read(buff, BUFF_DIM_PAGE_BYTE))
      break;
    long dim = _ttol(buff);
    Page[pages] = curr;
    ++pages;
    curr += dim + BUFF_DIM_PAGE_BYTE;
    if(curr >= (long)File->get_len())
      break;
    File->P_seek(curr);
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define HANDLE_TOKEN(a, b)  \
        case a:             \
          b(p, dim);        \
          break;
//-----------------------------------------------------------------------------
print_token::tk_error print_token::elab_token(LPCTSTR t, int dim)
{
  LPCTSTR p;
  p = get_next(t, dim);
  if(!p) // errore
    return END_BUFF;
  print_code code = static_cast<print_code>(_ttoi(t));
  switch(code) {
    HANDLE_TOKEN(PRN_TXT,           paint_txt)
    HANDLE_TOKEN(PRN_LINE,          paint_line)
    HANDLE_TOKEN(PRN_POLYLINE,      paint_polyline)
    HANDLE_TOKEN(PRN_BOX,           paint_box)
    HANDLE_TOKEN(PRN_BMP,           paint_bmp)
    HANDLE_TOKEN(PRN_SET_FONT,      set_font)
    HANDLE_TOKEN(PRN_SET_FONT2,     set_font2)
    HANDLE_TOKEN(PRN_SET_COLOR_FG,  set_color_fg)
    HANDLE_TOKEN(PRN_SET_COLOR_BKG, set_color_bkg)
    HANDLE_TOKEN(PRN_SET_BKMODE,    set_bkmode)
    HANDLE_TOKEN(PRN_SET_ALIGN,     set_align)
    HANDLE_TOKEN(PRN_SET_PEN,       set_pen)
    HANDLE_TOKEN(PRN_SET_BRUSH1,    set_brush1)
    HANDLE_TOKEN(PRN_SET_BRUSH2,    set_brush2)
    HANDLE_TOKEN(PRN_SET_BRUSH3,    set_brush3)

    HANDLE_TOKEN(PRN_RECT,          paint_rect)
    HANDLE_TOKEN(PRN_OVAL,          paint_oval)
    HANDLE_TOKEN(PRN_TXT_ON_BOX,    paint_txt_onBox)

    HANDLE_TOKEN(PRN_SET_NULL_BRUSH,paint_null_brush)
    HANDLE_TOKEN(PRN_ROUND_RECT,    paint_round_rect)
    HANDLE_TOKEN(PRN_ARC,           paint_arc)
    HANDLE_TOKEN(PRN_PIE,           paint_pie)
    HANDLE_TOKEN(PRN_CORD,          paint_cord)

    default:
      return NO_TOKEN;
    }
  return NO_ERR;
}
//-----------------------------------------------------------------------------
void PPrintFile::paintPage(HDC hdc)
{
  PPrintPage::paintPage(hdc);

  if(getCurrPage() != currPainting) {
    setCurrPage(currPainting);

    const long& cpos = Page[currPainting - 1];
    long pos = cpos;
    TCHAR t[BUFF_DIM_PAGE + 2];
    t[BUFF_DIM_PAGE] = 0;
    File->P_seek(pos);
    File->P_read(t, BUFF_DIM_PAGE * sizeof(t[0]));
    dimPage = _ttoi(t);
    if(dimPage > maxDimPage) {
      delete []Buff;
      Buff = new TCHAR[dimPage + 4];
      maxDimPage = dimPage;
      }
    memset(Buff, 0, maxDimPage * sizeof(TCHAR));
    File->P_read(Buff, dimPage * sizeof(TCHAR));
    }
  HDC hdcPrn = getPrnDC();
  POINT offset = { GetDeviceCaps(hdcPrn, PHYSICALOFFSETX),
      -GetDeviceCaps(hdcPrn, PHYSICALOFFSETY) };
  DPtoLP(hdcPrn, &offset, 1);
  print_token p_Token(offset, hdc, preview);
  int dim = dimPage;
  LPTSTR p1 = Buff;
  while(dim > 0) {
    LPTSTR p2 = get_line(p1, dim); // p2 punta alla riga successiva
    int len = int(p2 - p1);
    if(!p2)
      len = dim;
    print_token::tk_error err = p_Token.elab_token(p1, len);
    if(err == print_token::END_BUFF)
      break;
    dim -= len;
    p1 = p2;
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void PPrintFile::getPageInfo(int& minPage, int& maxPage, int& selFromPage, int& selToPage)
{
  PPrintPage::getPageInfo(minPage, maxPage, selFromPage, selToPage);
  minPage = 1;
  maxPage = getTotPages();
  selFromPage = minPage;
  selToPage = maxPage;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**/
#define FIRST_ID_COMMAND IDM_DONE
#define LAST_ID_COMMAND IDM_PRINT

//-----------------------------------------------------------------------------
#ifndef TBSTYLE_FLAT
  // Borland nella struttura dichiara un BYTE reserved[2] ????
  #define MY_TBSTYLE1 TBSTYLE_CHECKGROUP, { 0 }
  #define MY_TBSTYLE2 TBSTYLE_BUTTON, { 0 }
#else
  #define MY_TBSTYLE1 TBSTYLE_CHECKGROUP
  #define MY_TBSTYLE2 TBSTYLE_BUTTON
#endif
//-----------------------------------------------------------------------------
#define TIP(a) (IDM_DONE + (a))
TBBUTTON tbButtons[] = {
  { 0, IDM_DONE,   TBSTATE_ENABLED, MY_TBSTYLE2, 0L, TIP(0) },
  { 1, IDM_PREVIOUS,  TBSTATE_ENABLED, MY_TBSTYLE2, 0L, TIP(1)},
  { 2, IDM_NEXT,   TBSTATE_ENABLED, MY_TBSTYLE2, 0L, TIP(2)},
  { 3, IDM_ZOOM_IN,   TBSTATE_ENABLED, MY_TBSTYLE2, 0L, TIP(3)},
  { 4, IDM_ZOOM_OUT,    TBSTATE_ENABLED, MY_TBSTYLE2, 0L, TIP(4)},
  { 5, IDM_PRINT,    TBSTATE_ENABLED, MY_TBSTYLE2, 0L, TIP(5)},
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// identifiers
#define ID_TOOLBAR         1000

// other constants
#define BMP_CX        20
#define BMP_CY        20
//-----------------------------------------------------------------------------
static LPCTSTR tipsTools[SIZE_A_c(tbButtons)] = {
  _T("Done"),
  _T("Previous"),
  _T("Next"),
  _T("Zoom In"),
  _T("Zoom Out"),
  _T("Print"),
  };
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PToolBarInfo iTools = {
    ID_TOOLBAR,
    tbButtons,
    SIZE_A(tbButtons),
    IDB_TOOLBAR_PREVIEW,
    1,
    { BMP_CX, BMP_CY },
    { BMP_CX, BMP_CY },
    0,
    tipsTools,
    0
    };
//-----------------------------------------------------------------------------
static LRESULT eraseBkg(HWND hwnd, HDC hdc, HBRUSH brush)
{
  PRect rect;
  if(!GetUpdateRect(hwnd, rect, false))
    GetClientRect(hwnd, rect);
  FillRect(hdc , rect, brush);
  return true;
}
//-----------------------------------------------------------------------------
class PPaperWin : public PWin
{
  public:
    PPaperWin(PWin* parent);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//-----------------------------------------------------------------------------
PPaperWin::PPaperWin(PWin* parent) : PWin(parent)
{
  Attr.style = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS;
}
//-----------------------------------------------------------------------------
LRESULT PPaperWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ERASEBKGND:
//      return eraseBkg(hwnd, (HDC)wParam, (HBRUSH) GetStockObject(BLACK_BRUSH));
      return eraseBkg(hwnd, (HDC)wParam, (HBRUSH) GetStockObject(LTGRAY_BRUSH));
    }
  return PWin::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------------------------
class PWorkWin : public PWin
{
  public:
    PWorkWin(PWin* parent, PPrintPage* forward);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void restoreExtend(HDC hdc);
    PPrintPage* forwardDraw;
};
//-----------------------------------------------------------------------------
PWorkWin::PWorkWin(PWin* parent, PPrintPage* forward) : PWin(parent), forwardDraw(forward)
{
  Attr.style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
//  Attr.style = WS_CHILD | WS_BORDER | WS_VISIBLE;// | WS_CLIPSIBLINGS;
}
//-----------------------------------------------------------------------------
void PWorkWin::restoreExtend(HDC hdc)
{
  PRect r;
  GetWindowRect(*this, r);
  SetMapMode(hdc, MM_ANISOTROPIC);
  SetWindowExtEx(hdc, r.Width(), r.Height(), 0);
  SetViewportExtEx(hdc, r.Width(), r.Height(), 0);
}
//-----------------------------------------------------------------------------
LRESULT PWorkWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ERASEBKGND:
      return eraseBkg(hwnd, (HDC)wParam, (HBRUSH) GetStockObject(WHITE_BRUSH));

    case WM_PAINT:
      do {
#if 0
          HDC hdc = GetDC(*this);
          restoreExtend(hdc);
          forwardDraw->paintPage(hdc);
          ReleaseDC(*this, hdc);
          ValidateRect(*this, 0);
#else
          PAINTSTRUCT ps;
          HDC hdc = BeginPaint(*this, &ps);
          restoreExtend(hdc);
          forwardDraw->paintPage(hdc);
          EndPaint(*this, &ps);
#endif
        } while(false);

      return 0;
//      break;
    }
  return PWin::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*
class PPreviewPage : public PDialog
{
  public:
    PPreviewPage(PWin* parent, PPrintPage* page);
    ~PPreviewPage() { destroy(); }

    virtual bool create();
    virtual bool preProcessMsg(MSG& msg);

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void runPage(HDC hdc, int numPage);
    int totPage;

    void makePage(int numPage, bool zoomed = false);
  private:
    PWinTools* Tools;
    PPrintPage* Page;
    PWorkWin* workPage;
    PPaperWin* paperPage;
    int heightTool;

    int maxScroll;
    int pageScroll;

    void evMouseWheel(short delta, short x, short y);

    void HScroll(int type, int pos);
    void VScroll(int type, int pos);
    void genScroll(bool isVert, int action, int pos);

    POINT PtOffsPage;

    bool Zoomed;

    typedef PDialog baseClass;
};
*/
//-----------------------------------------------------------------------------
class myWin_Tools : public PWinTools
{
  public:
    myWin_Tools(PWin * parent, struct PToolBarInfo *info, LPCTSTR title,
            UINT idBkg, HINSTANCE hInstance) :
      PWinTools(parent, info, title, idBkg, hInstance) {}

    void setCurr(uint curr) { Curr = curr; refreshPage(); }
    void setTot(uint tot)   { Tot = tot;   refreshPage(); }
    void setAll(uint curr, uint tot)  { Curr = curr; Tot = tot;   refreshPage(); }
  protected:
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
  private:
    typedef PWinTools baseClass;
    uint Curr;
    uint Tot;
    void evPaint(HDC hdc);
    void refreshPage();
};
//-----------------------------------------------------------------------------
LRESULT myWin_Tools::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      do {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        PRect r(ps.rcPaint);
        if(r.Width() && r.Height()) {
          evPaint(hdc);
          }
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------------------------
void myWin_Tools::evPaint(HDC hdc)
{
  PRect r;
  GetClientRect(*this, r);
  r.left = r.right - 100;
  r.Inflate(-10, -2);
  TCHAR buff[40];
  wsprintf(buff, _T("%d / %d"), Curr, Tot);
  HFONT hf = D_FONT(18, 0, fBOLD, _T("arial"));
  PTextFixedPanel txt(buff, r, hf, RGB(0, 0, 0), PPanel::DN_FILL);
  txt.setAlign(TA_CENTER);
  txt.draw(hdc);
  DeleteObject(hf);
}
//-----------------------------------------------------------------------------
void myWin_Tools::refreshPage()
{
  if(!getHandle())
    return;
  HDC hdc = GetDC(*this);
  evPaint(hdc);
  ReleaseDC(*this, hdc);
}
//-----------------------------------------------------------------------------
static UINT_PTR getId(LPCTSTR source)
{
  UINT_PTR dw = (UINT_PTR)source;
  if(dw <= 0xffff)
    return dw;
  return 0;
}
//-----------------------------------------------------------------------------
static LPCTSTR makeIdOrStr(LPCTSTR source)
{
  if(!source)
    return 0;
  UINT_PTR dw = getId(source);
  if(dw)
    return (LPCTSTR)dw;
  return str_newdup(source);
}
//-----------------------------------------------------------------------------
static
void chgToolsInfo(PBitmap* bmp, PToolBarInfo& info)
{
  SIZE sz = bmp->getSize();
  sz.cx /= info.nButtons;
  info.szBmp = sz;
  info.szBtn = sz;
  if(!info.idBmp) {
    HBITMAP tmpHbmp = *bmp;
    info.hBitmap = tmpHbmp;
    }
}
//-----------------------------------------------------------------------------
static
PBitmap* chgTools(PWin* owner, PToolBarInfo& dup, LPCTSTR alternateToolBitmap)
{
  if(!alternateToolBitmap)
    return 0;
  DWORD dw = getId(alternateToolBitmap);
  if(dw) {
    dup.idBmp = 0;
    PBitmap* bmp = new PBitmap(dw, owner->getHInstance());
    chgToolsInfo(bmp, dup);
    return bmp;
    }
#ifdef GDIPLUS_PRINT
  PBitmap* bmp = new PBitmap(alternateToolBitmap);
  if(!bmp->isValid()) {
    delete bmp;
    return 0;
    }
  chgToolsInfo(bmp, dup);
  return bmp;

#else
  uint len =0;
  LPBYTE buff = 0;
  do {
    P_File pf(alternateToolBitmap, P_READ_ONLY);
    if(!pf.P_open())
      return 0;
    len = (uint)pf.get_len();
    buff = new BYTE[len];
    pf.P_read(buff, len);
    } while(false);

  PALETTEENTRY pe[16];
  HPALETTE hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
  UINT np = GetPaletteEntries(hpal, 0, SIZE_A(pe), pe);
  BITMAPINFOHEADER* lpbi = (BITMAPINFOHEADER*)(buff + sizeof(BITMAPFILEHEADER));
  LPBYTE pPal = buff + sizeof(BITMAPFILEHEADER) + lpbi->biSize;
  uint NumColors = DibNumColors(lpbi);
#define PALETTE_BKG 7

  NumColors = min(NumColors, np);
  if(NumColors <= PALETTE_BKG) {
    delete []buff;
    PBitmap tbmp(alternateToolBitmap);
    PBitmap* bmp = new PBitmap(tbmp);
    dup.idBmp = 0;
    chgToolsInfo(bmp, dup);
    return bmp;
    }
  COLORREF bkg = GetSysColor(COLOR_BTNFACE);

  if(lpbi->biSize == sizeof(BITMAPCOREHEADER)) {
    pPal += PALETTE_BKG * 3;
    *pPal++ = GetBValue(bkg);
    *pPal++ = GetGValue(bkg);
    *pPal++ = GetRValue(bkg);
    }
  else {
    pPal += PALETTE_BKG * 4;
    *pPal++ = GetBValue(bkg);
    *pPal++ = GetGValue(bkg);
    *pPal++ = GetRValue(bkg);
    }
  do {
    P_File pft(_T("~tmp.bmp"), P_CREAT);
    if(!pft.P_open()) {
      delete []buff;
      return 0;
      }
    pft.P_write(buff, len);
    } while(false);

  delete []buff;
  PBitmap* bmp = new PBitmap(_T("~tmp.bmp"));

  dup.idBmp = 0;
  chgToolsInfo(bmp, dup);
  return bmp;
#endif
}
//-----------------------------------------------------------------------------
PPreviewPage::PPreviewPage(PWin* parent, PPrintPage* page, bool& print, LPCTSTR bitmapOrID) : baseClass(parent, IDD_PREVIEW),
        Page(page), heightTool(0), maxScrollV(0), pageScrollV(0), maxScrollH(0), pageScrollH(0),
        Zoomed(0), choosePrint(print), alternateToolBitmap(makeIdOrStr(bitmapOrID)), alternateBitmap(0)
{
  dupTools = iTools;
  alternateBitmap = chgTools(this, dupTools, alternateToolBitmap);
  Tools = new myWin_Tools(this, &dupTools, _T("Tools"), IDB_BKG_TOOLBAR, getHInstance());
  Tools->Attr.style &= ~(WS_CAPTION | WS_OVERLAPPED | WS_POPUP);
  Tools->Attr.style |= WS_CHILD;
  Tools->Attr.exStyle &= ~WS_EX_TOOLWINDOW;
  Tools->setAll(1, Page->getTotPages());
  workPage = new PWorkWin(this, Page);
  paperPage = new PPaperWin(this);
}
//-----------------------------------------------------------------------------
PPreviewPage::~PPreviewPage()
{
  destroy();
  if(!getId(alternateToolBitmap))
    delete []alternateToolBitmap;
  delete alternateBitmap;
}
//-----------------------------------------------------------------------------
#define LIMIT_PAGE 5
//-----------------------------------------------------------------------------
inline
static void getPaperArea(PWin* win, PRect& r, int upperOffset)
{
  GetClientRect(*win, r);
  r.top = upperOffset;
  r.Inflate(-LIMIT_PAGE, -LIMIT_PAGE);
}
//-----------------------------------------------------------------------------
static void reScale(HDC hdc, HDC prnDc)
{
  SIZE scrve;
  ::GetViewportExtEx(hdc, &scrve);

  POINT scrwe = { GetDeviceCaps(prnDc, HORZRES), GetDeviceCaps(prnDc, VERTRES) };
//  POINT scrwe = { GetDeviceCaps(prnDc, PHYSICALWIDTH), GetDeviceCaps(prnDc, PHYSICALHEIGHT) };
  DPtoLP(prnDc, &scrwe, 1);

  ::SetMapMode(hdc, MM_ANISOTROPIC);
  ::SetWindowExtEx(hdc, scrwe.x, scrwe.y, 0);
  ::SetViewportExtEx(hdc, scrve.cx, scrve.cy, 0);

/**/
  POINT origin;
  GetViewportOrgEx(prnDc, &origin);
  DPtoLP(prnDc, &origin, 1);
  LPtoDP(hdc, &origin, 1);
  ::SetViewportOrgEx(hdc, origin.x, origin.y, 0);

  GetWindowOrgEx(prnDc, &origin);
  ::SetWindowOrgEx(hdc, origin.x, origin.y, 0);
/**/
}
//-----------------------------------------------------------------------------
void PPreviewPage::runPage(HDC hdc, int pageNum)
{
  Page->beginPrinting();
  Page->beginDocument(pageNum, pageNum);

  Page->printPage(hdc, pageNum);

  Page->endDocument();
  Page->endPrinting();
}
//-----------------------------------------------------------------------------
static void setSB(HWND hwnd, const PRect& r, uint maxS)
{
  SendMessage(hwnd, SBM_SETRANGE, 0, maxS);
//  SendMessage(hwnd, SBM_SETPOS, 0, 0);
  SetWindowPos(hwnd, 0, r.left, r.top, r.Width(), r.Height(), SWP_SHOWWINDOW);
  EnableWindow(hwnd, true);
}
//-----------------------------------------------------------------------------
inline
float getPercZoom(int zoom)
{
  float v = (float)zoom ;
  v *= 0.6f;
  return v >= 1 ? v : 1;
}
//-----------------------------------------------------------------------------
inline
int mulZoom(int v, float f)
{
  return (int)((float)v * f);
}
//-----------------------------------------------------------------------------
inline
int divZoom(int v, float f)
{
  return (int)((float)v / f);
}
//-----------------------------------------------------------------------------
void PPreviewPage::makePage(int numPage, uint zoomed)
{
  HDC hdcPrn = Page->getPrnDC();
  if(!hdcPrn)
    return;
  Zoomed = zoomed;

  SIZE paperExtent =
    { GetDeviceCaps(hdcPrn, PHYSICALWIDTH), GetDeviceCaps(hdcPrn, PHYSICALHEIGHT) };
  SIZE printExtent =
    { GetDeviceCaps(hdcPrn, HORZRES), GetDeviceCaps(hdcPrn, VERTRES) };
  SIZE logicExtent =
    { GetDeviceCaps(hdcPrn, LOGPIXELSX), GetDeviceCaps(hdcPrn, LOGPIXELSY) };
  if (logicExtent.cx != logicExtent.cy) {
    double scale = logicExtent.cx;
    scale /= logicExtent.cy;
    printExtent.cy = (LONG)(printExtent.cy * scale);
    }
  PRect r;
  getPaperArea(this, r, heightTool);

  double ratioScreen = r.Height();
  ratioScreen /= r.Width();

  double ratioDevice = paperExtent.cy;
  ratioDevice /= paperExtent.cx;

  if(zoomed) {
    int wSB_V = GetSystemMetrics(SM_CXVSCROLL);
    int wSB_H = GetSystemMetrics(SM_CYHSCROLL);
    PRect rsbV(r);
    PRect rsbH(r);
    rsbV.Inflate(LIMIT_PAGE / 2 - 1, 0);
    rsbH.Inflate(0, LIMIT_PAGE);

    rsbV.left = rsbV.right - wSB_V;
    rsbH.top = rsbH.bottom - wSB_H;

    r.right -= wSB_V + LIMIT_PAGE;
    r.bottom -= wSB_H + LIMIT_PAGE;
    int rangeV;
    int rangeH;
    float pZ = getPercZoom(Zoomed + 1);

    if(ratioScreen > ratioDevice) {
      int h = mulZoom(r.Width(), pZ);
      h = (int)(h * ratioDevice);
      h = r.Height() - h;
      rangeV = r.Height();
      rangeH = r.Width();
      r.right = r.left + mulZoom(rangeH, pZ);
      int top = r.top;
      r.Inflate(0, -(h / 2));
//      if((int)r.Height() - rangeV > 0)
        r.MoveTo(r.left, top);
      }
    else {
      int w = mulZoom(r.Height(), pZ);
      w = (int)( w / ratioDevice);
      w  = r.Width() - w;
      rangeV = r.Height();
      rangeH = r.Width();
      r.bottom = r.top + mulZoom(rangeV, pZ);
      int left = r.left;
      r.Inflate(-(w / 2), 0);
//      if((int)r.Width() - rangeH > 0)
        r.MoveTo(left, r.top);
      }
    maxScrollV = r.Height() - rangeV;
    pageScrollV = divZoom(rangeV, pZ);
    if(maxScrollV <= 0) {
      maxScrollV = 0;
      pageScrollV = 0;
      }
    maxScrollH = r.Width() - rangeH;
    pageScrollH = divZoom(rangeH, pZ);
    if(maxScrollH <= 0) {
      maxScrollH = 0;
      pageScrollH = 0;
      }

    if(maxScrollV && maxScrollH) {
      rsbV.bottom -= wSB_H;
      rsbH.right -= wSB_V;
      }
    setSB(GetDlgItem(*this, IDC_SCROLLBAR_VERT_PREVIEW), rsbV, maxScrollV);
    setSB(GetDlgItem(*this, IDC_SCROLLBAR_HORZ_PREVIEW), rsbH, maxScrollH);

    if(!maxScrollH)
      ShowWindow(GetDlgItem(*this, IDC_SCROLLBAR_HORZ_PREVIEW), SW_HIDE);
    if(!maxScrollV)
      ShowWindow(GetDlgItem(*this, IDC_SCROLLBAR_VERT_PREVIEW), SW_HIDE);

    PostMessage(*this, WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), 0);
    }
  else {
    if(ratioScreen > ratioDevice) {
      int h = r.Width();
      h = (int)(h * ratioDevice);
      h = r.Height() - h;
      r.Inflate(0, -(h / 2));
      }
    else {
      int w = r.Height();
      w = (int)( w / ratioDevice);
      w  = r.Width() - w;
      r.Inflate(-(w / 2), 0);
      }
    ShowWindow(GetDlgItem(*this, IDC_SCROLLBAR_HORZ_PREVIEW), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_SCROLLBAR_VERT_PREVIEW), SW_HIDE);
    EnableWindow(GetDlgItem(*this, IDC_SCROLLBAR_HORZ_PREVIEW), false);
    EnableWindow(GetDlgItem(*this, IDC_SCROLLBAR_VERT_PREVIEW), false);
    }

  paperPage->setWindowPos(0, r, SWP_NOZORDER);

  double ratioXPaper = paperExtent.cx;
  ratioXPaper /= printExtent.cx;

  double ratioYPaper = paperExtent.cy;
  ratioYPaper /= printExtent.cy;

  int width = (int)(r.Width() / ratioXPaper);
  int height = (int)(r.Height() / ratioYPaper);

#if 1
  double offsX = GetDeviceCaps(hdcPrn, PHYSICALOFFSETX);
  offsX /= paperExtent.cx;
  offsX *= width;

  double offsY = GetDeviceCaps(hdcPrn, PHYSICALOFFSETY);
  offsY /= paperExtent.cy;
  offsY *= height;
#else
  double offsX = GetDeviceCaps(hdcPrn, PHYSICALOFFSETX);
  offsX /= paperExtent.cx;
  offsX *= r.Width();

  double offsY = GetDeviceCaps(hdcPrn, PHYSICALOFFSETY);
  offsY /= paperExtent.cy;
  offsY *= r.Height();
#endif

  // da aggiungere lo scambio del physicalOffsetX quando
  // la pagina è ruotata (orizzontale). Viene riportato
  // il limite fisico inferiore perché la pagina è ruotata
  // di 90° in senso orario e quindi il nuovo left
  // corrisponde al vecchio bottom

  PtOffsPage.x = (LONG)offsX;
  PtOffsPage.y = (LONG)offsY;

  r.Offset((LONG)offsX, (LONG)offsY);
  r.right = r.left + width;
  r.bottom = r.top + height;
  workPage->setWindowPos(0, r, SWP_NOZORDER);

  DPtoLP(hdcPrn, (POINT*)&printExtent, 1);

  HDC hdc = GetDC(*workPage);
  SetMapMode(hdc, MM_ANISOTROPIC);
  SetWindowExtEx(hdc, r.Width(), r.Height(), 0);
  SetViewportExtEx(hdc, r.Width(), r.Height(), 0);

  reScale(hdc, hdcPrn);

  runPage(hdc, numPage);

  ReleaseDC(*workPage, hdc);

}
//-----------------------------------------------------------------------------
bool PPreviewPage::create()
{
  if(!baseClass::create())
    return false;

  PRect rect(-2, -2, GetSystemMetrics(SM_CXSCREEN) + 4, GetSystemMetrics(SM_CYSCREEN) + 4);

  setWindowPos(0, rect, SWP_NOZORDER | SWP_SHOWWINDOW);

  GetWindowRect(*Tools, rect);
  heightTool = rect.Height();
  PRect r;
//  GetWindowRect(*this, r);
  GetClientRect(*this, r);
  r.bottom = heightTool;
  Tools->setWindowPos(0, r, SWP_NOZORDER);

  Tools->enableBtn(IDM_ZOOM_OUT - IDM_DONE, false);
  Tools->enableBtn(IDM_PREVIOUS - IDM_DONE, false);
  bool enable = Page->getTotPages() > 1;
  Tools->enableBtn(IDM_NEXT - IDM_DONE, enable);

  SendMessage(GetDlgItem(*this, IDC_SCROLLBAR_VERT_PREVIEW), SBM_SETPOS, 0, 0);
  SendMessage(GetDlgItem(*this, IDC_SCROLLBAR_HORZ_PREVIEW), SBM_SETPOS, 0, 0);

  makePage(1, 0);

  return true;
}
//-----------------------------------------------------------------------------
/*
#define POST_AND_SEL(hwnd, id) \
          PostMessage(hwnd, WM_C_CHANGED_SELECTION, id, 0); \
          Tools->setHotItem(id - IDM_DONE);
*/
//-----------------------------------------------------------------------------
#define POST_AND_SEL(hwnd, id) postAndSel(id)
//-----------------------------------------------------------------------------
void PPreviewPage::postAndSel(uint id)
{
  PostMessage(*this, WM_C_CHANGED_SELECTION, id, 0);
  Tools->setHotItem(id - IDM_DONE);
}
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//----------------------------------------------------------------------------
#define VK_OEM_PLUS       0xBB   // '+' any country
#define VK_OEM_MINUS      0xBD   // '-' any country
//----------------------------------------------------------------------------
bool PPreviewPage::preProcessMsg(MSG& msg)
{
  switch (msg.message) {
    case WM_KEYDOWN:
      switch(msg.wParam) {
        case VK_ADD:
        case VK_OEM_PLUS:
          POST_AND_SEL(*this, IDM_ZOOM_IN);
          return true;

        case VK_SUBTRACT:
        case VK_OEM_MINUS:
          POST_AND_SEL(*this, IDM_ZOOM_OUT);
          return true;
      }
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//-----------------------------------------------------------------------------
LRESULT PPreviewPage::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ERASEBKGND:
      return eraseBkg(hwnd, (HDC)wParam, (HBRUSH) GetStockObject(GRAY_BRUSH));

    case WM_MOUSEWHEEL:
      evMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
      break;

    case WM_HSCROLL:
      HScroll(LOWORD (wParam), HIWORD (wParam));
      break;

    case WM_VSCROLL:
      VScroll(LOWORD (wParam), HIWORD (wParam));
      break;

    case WM_COMMAND:
      switch(wParam) {
        case IDCANCEL:
//        case IDC_BUTTON_F1:
          POST_AND_SEL(hwnd, IDM_DONE);
          break;
/*
        case IDC_BUTTON_F2:
          POST_AND_SEL(hwnd, IDM_PREVIOUS);
          break;
        case IDC_BUTTON_F3:
          POST_AND_SEL(hwnd, IDM_NEXT);
          break;
        case IDC_BUTTON_F4:
          POST_AND_SEL(hwnd, IDM_ZOOM_IN);
          break;
        case IDC_BUTTON_F5:
          POST_AND_SEL(hwnd, IDM_ZOOM_OUT);
          break;
*/
        default:
          break;
        }
      break;

    case WM_C_CHANGED_SELECTION:
      switch(wParam) {
        case IDM_PRINT:
          choosePrint = true;
          // fall through
        case IDM_DONE:
#if 1
          DestroyWindow(hwnd);
#else
          EndDialog(hwnd, IDOK);
#endif
          return 0;
        case IDM_PREVIOUS:
          if(Page->currPainting > 1) {
            --Page->currPainting;
            Tools->setCurr(Page->currPainting);
            Tools->enableBtn(IDM_NEXT - IDM_DONE, true);
            if(1 == Page->currPainting)
              Tools->enableBtn(IDM_PREVIOUS - IDM_DONE, false);
            InvalidateRect(*workPage, 0 , 1);
            }
          break;
        case IDM_NEXT:
          if(Page->currPainting < Page->getTotPages()) {
            ++Page->currPainting;
            Tools->setCurr(Page->currPainting);
            Tools->enableBtn(IDM_PREVIOUS - IDM_DONE, true);
            if(Page->getTotPages() == Page->currPainting)
              Tools->enableBtn(IDM_NEXT - IDM_DONE, false);
            InvalidateRect(*workPage, 0 , 1);
            }
          break;
        case IDM_ZOOM_IN:
          if(!Zoomed || Zoomed < 5) {
            if(5 == Zoomed + 1)
              Tools->enableBtn(IDM_ZOOM_IN - IDM_DONE, false);
            Tools->enableBtn(IDM_ZOOM_OUT - IDM_DONE, true);
            InvalidateRect(*workPage, 0, 1);
            makePage(Page->getCurrPage(), Zoomed + 1);
            }
          break;
        case IDM_ZOOM_OUT:
          if(Zoomed > 0) {
            if(1 == Zoomed)
              Tools->enableBtn(IDM_ZOOM_OUT - IDM_DONE, false);
            Tools->enableBtn(IDM_ZOOM_IN - IDM_DONE, true);
            InvalidateRect(*workPage, 0, 1);
            makePage(Page->getCurrPage(), Zoomed - 1);
            }
          break;
        }
      SetFocus(hwnd);
      do {
        HWND child = GetDlgItem(hwnd, IDC_SCROLLBAR_VERT_PREVIEW);
        if(IsWindowEnabled(child)) {
          SetFocus(child);
          break;
          }
        child = GetDlgItem(hwnd, IDC_SCROLLBAR_HORZ_PREVIEW);
        if(IsWindowEnabled(child))
          SetFocus(child);
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void PPreviewPage::evMouseWheel(short delta, short /*x*/, short /*y*/)
{
  int tD = delta;
  tD /= WHEEL_DELTA / 40;
//  tD *= 10;
  uint msg = WM_VSCROLL;
  HWND child = GetDlgItem(*this, IDC_SCROLLBAR_VERT_PREVIEW);
  int maxScroll = maxScrollV;
  if(!IsWindowEnabled(child)) {
    msg = WM_HSCROLL;
    child = GetDlgItem(*this, IDC_SCROLLBAR_HORZ_PREVIEW);
    maxScroll = maxScrollH;
    }

  if(IsWindowEnabled(child)) {
    int curr = (int)SendMessage(child, SBM_GETPOS, 0, 0);
    curr -= tD;
    if(curr < 0)
      curr = 0;
    else if(curr > maxScroll)
      curr = maxScroll;
    SendMessage(*this, msg, MAKEWPARAM(SB_THUMBTRACK, curr), 0);
    }
}
//-----------------------------------------------------------------------------
PPreviewPage* PPrintPage::allocPreviewPage(PWin* /*parent*/, PPrintPage* /*page*/, bool& print, LPCTSTR alternateBitmapOrID4Tool)
{
  return new PPreviewPage(getParent(), this, print, alternateBitmapOrID4Tool);
}
//-----------------------------------------------------------------------------
void PPrintPage::PrintPreview(LPCTSTR alternateBitmapOrID4Tool)
{
  if(!Printer)
    return;
  preview = true;

  initializePrinter(Printer);
  bool print = false;
#if 1
  EnableWindow(*getParent(), false);

  PPreviewPage* ppp = allocPreviewPage(getParent(), this, print, alternateBitmapOrID4Tool);
  if(ppp->create()) {
    while(ppp->getHandle()) {
      ::WaitMessage();
      getAppl()->pumpMessages();
      }
    }
  EnableWindow(*getParent(), true);
  SetForegroundWindow(*getParent());
  delete ppp;
  if(print) {
    preview = false;
    Print(true);
    }
#else
  HWND oldWin = GetActiveWindow();
  PPreviewPage(getParent(), this).modal();
  SetActiveWindow(oldWin);
#endif
}
//-----------------------------------------------------------------------------
void PPreviewPage::HScroll(int type, int pos)
{
  genScroll(false, type, pos);
}
//-----------------------------------------------------------------------------
void PPreviewPage::VScroll(int type, int pos)
{
  genScroll(true, type, pos);
}
//-----------------------------------------------------------------------------
void PPreviewPage::genScroll(bool isVert, int action, int pos)
{
  int maxScroll = maxScrollV;
  int pageScroll = pageScrollV;

  HWND hsb;
  if(isVert)
    hsb = GetDlgItem(*this, IDC_SCROLLBAR_VERT_PREVIEW);
  else {
    hsb = GetDlgItem(*this, IDC_SCROLLBAR_HORZ_PREVIEW);
    maxScroll = maxScrollH;
    pageScroll = pageScrollH;
    }

  int curr = (int)SendMessage(hsb, SBM_GETPOS, 0, 0);
  switch(action) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
//    case SB_TOP:
      pos = 0;
      break;
    case SB_RIGHT:
//    case SB_BOTTOM:
      pos = maxScroll;
      break;
    case SB_LINELEFT:
//    case SB_LINEUP:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
//    case SB_LINEDOWN:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
//    case SB_PAGEUP:
      pos = curr - pageScroll;
      break;
    case SB_PAGERIGHT:
//    case SB_PAGEDOWN:
      pos = curr + pageScroll;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if(pos > maxScroll)
    pos = maxScroll;

  SendMessage(hsb, SBM_SETPOS, pos, true);

  PRect r;
  getPaperArea(this, r, heightTool);

  if(isVert) {
    int curr = (int)SendMessage(GetDlgItem(*this, IDC_SCROLLBAR_HORZ_PREVIEW), SBM_GETPOS, 0, 0);
    r.MoveTo(r.left - curr, r.top - pos);
    }
  else {
    int curr = (int)SendMessage(GetDlgItem(*this, IDC_SCROLLBAR_VERT_PREVIEW), SBM_GETPOS, 0, 0);
    r.MoveTo(r.left - pos, r.top - curr);
    }
  paperPage->setWindowPos(0, r, SWP_NOZORDER | SWP_NOSIZE);

  r.Offset(PtOffsPage.x, PtOffsPage.y);
  workPage->setWindowPos(0, r, SWP_NOZORDER | SWP_NOSIZE);
}
