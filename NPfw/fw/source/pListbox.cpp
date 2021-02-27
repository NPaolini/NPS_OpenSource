//-------------------- pListbox.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pListbox.h"
#include "p_Util.h"
#include <math.h>
//-----------------------------------------------------------
//colori di default

#if 1
//-----------------------------------------------------------
//-----------------------------------------------------------------
// colori di default
// colore sfondo normale
#define _BKCOLOR    RGB(250,250,250)
//#define BKCOLOR_    RGB(234,253,250)
// colore sfondo evidenziato
#define _BKCOLOR_E  RGB(222,222,222)
//#define BKCOLOR_E_  RGB(128,128,255)
// colore testo normale
#define _COLORT     RGB(0,0,169)
// colore testo evidenziato
#define _COLORT_E   RGB(0,0,0)
//#define COLORT_E_   RGB(0xff,0xff,0x0f)

//-----------------------------------------------------------
//-----------------------------------------------------------
#else
//-----------------------------------------------------------
//-----------------------------------------------------------
//#define DEF1
#define COL(a,b) a##b
#ifdef DEF1
  #define _BKCOLOR     COL(COLOR,1)
  #define _BKCOLOR_E   COL(COLOR_E,1)
  #define _COLORT      COL(COLORT,1)
  #define _COLORT_E    COL(COLORT_E,1)
#else
  #define _BKCOLOR     COL(COLOR,2)
  #define _BKCOLOR_E   COL(COLOR_E,2)
  #define _COLORT      COL(COLORT,2)
  #define _COLORT_E    COL(COLORT_E,2)
#endif
// colori sfondo normale
//#define COLOR1
#define COLOR1 RGB(0xff,0xff,0xff)
#define COLOR2 RGB(0xff,0xff,0xcf)
// colori sfondo evidenziato
#define COLOR_E1 RGB(0x0,0x0,0x0)
#define COLOR_E2 RGB(0x1f,0xf,0x0)
// colori testo normale
#define COLORT1 RGB(0x0,0x0,0x0)
#define COLORT2 RGB(0x0,0x0,0x1f)
// colori testo evidenziato
#define COLORT_E1 RGB(0xff,0xff,0xff)
#define COLORT_E2 RGB(0xff,0xff,0x0f)

//-----------------------------------------------------------
#endif
#define BKCOLOR     C_Bkg
#define BKCOLOR_E   C_BkgSel
#define COLORT      C_Text
#define COLORT_E    C_TextSel
//-----------------------------------------------------------
//-----------------------------------------------------------
PListBox::PListBox(PWin * parent, uint id, int x, int y, int w, int h,
                    int /*len*/, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, 0, hinst), pxTabs(0), nTabs(0), Align(0),
      Buff(0), ShowField(0), MaxLen(0), C_Text(_COLORT), C_TextSel(_COLORT_E),
           C_Bkg(_BKCOLOR), C_BkgSel(_BKCOLOR_E), first(true), hItem(0),
           charTabs(0), pixelWidthChar(AUTO_PIX_WIDTH)
{
    Attr.style = LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS |
                 LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE |
//                 WS_BORDER |
                 WS_VSCROLL | WS_HSCROLL | WS_TABSTOP;
    Attr.exStyle |= WS_EX_CLIENTEDGE;
}
//-----------------------------------------------------------
PListBox::PListBox(PWin* parent, uint id, int /*len*/, HINSTANCE hinst) :
   PControl(parent, id, hinst), pxTabs(0), nTabs(0), Align(0), Buff(0),
      ShowField(0), MaxLen(0), C_Text(_COLORT), C_TextSel(_COLORT_E),
           C_Bkg(_BKCOLOR), C_BkgSel(_BKCOLOR_E), first(true), hItem(0),
           charTabs(0), pixelWidthChar(AUTO_PIX_WIDTH)
{
}
//-----------------------------------------------------------
PListBox::PListBox(PWin * parent, uint id, const PRect& r, int /*len*/, HINSTANCE hinst) :
      PControl(parent, id, r, 0, hinst), pxTabs(0), nTabs(0), Align(0),
      Buff(0), ShowField(0), MaxLen(0), C_Text(_COLORT), C_TextSel(_COLORT_E),
           C_Bkg(_BKCOLOR), C_BkgSel(_BKCOLOR_E), first(true), hItem(0), charTabs(0),
           pixelWidthChar(AUTO_PIX_WIDTH)
{
    Attr.style = LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS |
                 LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE |
//                 WS_BORDER |
                 WS_VSCROLL | WS_HSCROLL | WS_TABSTOP;
    Attr.exStyle |= WS_EX_CLIENTEDGE;
}
//-----------------------------------------------------------
PListBox::~PListBox ()
{
  destroy();

  delete []charTabs;
  delete []Buff;
  delete []Align;
  delete []ShowField;
  delete []pxTabs;
//  destroy();
}
//-----------------------------------------------------------
void PListBox::copyColorFrom(PListBox* other)
{
  SetColor(other->C_Text, other->C_Bkg);
  SetColorSel(other->C_TextSel, other->C_BkgSel);
}
//-----------------------------------------------------------
int PListBox::retrieveHeightFont()
{
  SIZE sz = getSizeFont();

  return sz.cy / SIMUL_DEC_FONT;
/*
  HDC hdc = GetDC(getHandle());
  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);
  ReleaseDC(getHandle(), hdc);
  return tm.tmHeight;
*/
}
//-----------------------------------------------------------
LRESULT PListBox::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
//    case WM_HSCROLL:
//      InvalidateRect(*this, 0, 0);
//      break;
    case WM_MEASUREITEM:
      if(!hItem)
        hItem = retrieveHeightFont();
      if(hItem) {
        MEASUREITEMSTRUCT *lpmis = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
        lpmis->itemHeight = hItem + ADD_HEIGHT;
        return true;
        }
      break;
    case WM_DRAWITEM:
//      if(wParam == Attr.id)
      DrawItem (*(LPDRAWITEMSTRUCT) lParam);
      return true;
//      break;
    case WM_PAINT:
      if(!nTabs || !pxTabs)
        break;
      if(SendMessage(hwnd, LB_GETCOUNT, 0, 0) <= 0) {
#if 1
        PAINTSTRUCT Paint;
        BeginPaint(hwnd, &Paint);
        EndPaint(hwnd, &Paint);
#else
        PRect rect;
        GetUpdateRect(hwnd, rect, true);
        ValidateRect(hwnd, rect);
#endif
        }
      else
        break;
    case WM_ERASEBKGND:
//      if(!nTabs || !pxTabs)
//        break;
      do {
        PRect rect;
#if 1
        if(!GetUpdateRect(hwnd, rect, false)) {
          int count = (int)SendMessage(hwnd, LB_GETCOUNT, 0, 0);
          PRect r;
          GetClientRect(hwnd, r);
          count *= getHItem();
          if(count < r.Height())
            rect = r;
          }
#else
        GetClientRect(hwnd, rect);
#endif
        HBRUSH hbrush = CreateSolidBrush(BKCOLOR);
        HDC hdc = (HDC)wParam;
        if(!hdc)
          hdc = GetDC(hwnd);
        FillRect(hdc, rect, hbrush);
        DeleteObject(hbrush);
        if(!wParam)
          ReleaseDC(hwnd, hdc);
        return true;
        } while(false);
      }

  return PControl::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void PListBox::setAlign(int n, tAlign al)
{
  if(Align && n < nTabs)
    Align[n] = al;
}
//-----------------------------------------------------------
void PListBox::setFont(HFONT font, bool autoDelete)
{
  hItem = 0;
  PControl::setFont(font, autoDelete);
  if(getHandle())
    SetTab();
  setHorzExt();
}
//-----------------------------------------------------------
void PListBox::setFontNoReduce(HFONT font, bool autoDelete)
{
  if(!pxTabs)
    return;
  // salva le dimensioni correnti
  int* oldTabs = new int[nTabs];
  for(int i = 0; i < nTabs; ++i)
    oldTabs[i] = pxTabs[i];

  setFont(font, autoDelete);

  // se il nuovo font causa un restringimento
  // ripristina le vecchie misure e riaggiusta la larghezza massima
  if(pxTabs[nTabs - 1] < oldTabs[nTabs - 1]) {
    for(int i = 0; i < nTabs; ++i)
      pxTabs[i] = oldTabs[i];
    setHorzExt();
    }
  delete []oldTabs;
}
//-----------------------------------------------------------
HPEN PListBox::setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo)
{
  COLORREF txt;
  COLORREF bkg;
  if (drawInfo.itemState & ODS_SELECTED)  {
    bkg = BKCOLOR_E;
    txt = COLORT_E;
    }
  else  {
    bkg = BKCOLOR;
    txt = COLORT;
    }

  PRect Rect(drawInfo.rcItem);

  SetBkColor(hDC, bkg);
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, Rect, 0, 0, 0);

  SetTextColor(hDC, txt);
  HPEN pen = CreatePen(PS_SOLID, 1, txt);
  HPEN oldPen = (HPEN) SelectObject(hDC, pen);
  return oldPen;
}
//-----------------------------------------------------------
void PListBox::paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
#if 1
  if(!pxTabs)
    return;

#define DEF_STYLE (DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER)
  uint format = DEF_STYLE;
  int right = pxTabs[ix];
  LPTSTR t = 0;
  switch(Align[ix]) {
    case aRight:
      format |= DT_RIGHT;
      do {
        size_t len = _tcslen(buff);
        t = new TCHAR[len + 2];
        _tcscpy_s(t, len + 1, buff);
        t[len] = _T(' ');
        t[len + 1] = 0;
        } while(false);
      break;
    case aLeft:
      format |= DT_LEFT;
      break;
    default:
      format |= DT_CENTER;
      break;
    }
  PRect r(x, y, right, drawInfo.rcItem.bottom);
  r.Inflate(-3, 0);
  if(t) {
    DrawText(hDC, t, (int)_tcslen(t), r, format);
    delete []t;
    }
  else
    DrawText(hDC, buff, (int)_tcslen(buff), r, format);

#else
  TextOut(hDC, x, y, buff, _tcslen(buff));
#endif
}
//-----------------------------------------------------------
LPTSTR PListBox::trim(TCHAR *buff)
{
  for(int l = (int)_tcslen(buff) - 1; l > 0; --l) // elimina gli spazi finali
    if((unsigned)buff[l] <= _T(' '))
      buff[l] = 0;
    else
      break;
  while(*buff && (unsigned)*buff <= _T(' ')) // elimina gli spazi iniziali
    ++buff;
  return buff;
}
//-----------------------------------------------------------
void PListBox::paintCustom(HDC hDC, PRect& r, int /*ix*/, DRAWITEMSTRUCT FAR& /*drawInfo*/)
{
  MoveToEx(hDC, r.right, r.top, 0);
  LineTo(hDC, r.right, r.bottom - 2);
  int x = r.left + 2;
  if(x < 0)
    x = 0;
  LineTo(hDC, x, r.bottom - 2);
}
//-----------------------------------------------------------
bool PListBox::retrieveBuff(DRAWITEMSTRUCT FAR& drawInfo)
{
  int len = (int)SendMessage(getHandle(), LB_GETTEXTLEN, drawInfo.itemID, 0);
  if(len > MaxLen) {
    MaxLen = len + 1;
    delete []Buff;
    Buff = new TCHAR[MaxLen+2];
    }
  SendMessage(getHandle(), LB_GETTEXT, drawInfo.itemID, (LPARAM)Buff);
  return true;
}
//-----------------------------------------------------------
void PListBox::DrawItem (DRAWITEMSTRUCT FAR& drawInfo)
{
  if(!pxTabs)
    return;

  if(drawInfo.itemID == -1) {
    if (drawInfo.itemState & ODS_FOCUS)
      DrawFocusRect(drawInfo.hDC, &drawInfo.rcItem);
    return;
    }
  else  {
    HDC hdc = drawInfo.hDC;
    if(!retrieveBuff(drawInfo))
      return;
    PRect r(drawInfo.rcItem);
    HBITMAP hBmpWork = CreateCompatibleBitmap(hdc, r.right, r.bottom);
    HDC mdcWork = CreateCompatibleDC(hdc);
    HGDIOBJ oldObjWork = SelectObject(mdcWork, hBmpWork);

    HBRUSH br = CreateSolidBrush(BKCOLOR);

    FillRect(mdcWork, r, br);
    DeleteObject(br);

    HPEN oldPen = setColor(mdcWork, drawInfo);

    TCHAR *t2;
    t2 = Buff;
    int x = drawInfo.rcItem.left;
    int y = drawInfo.rcItem.top;
    HGDIOBJ oldFont = 0;
    HFONT fnt = getFont();
    if(!fnt)
      fnt = (HFONT)SendMessage(*this, WM_GETFONT, 0, 0);
    if(fnt)
      oldFont = SelectObject(mdcWork, fnt);

    for(int i = 0; i < nTabs; ++i) {
      TCHAR *t3 = get_next(t2);
      if(!t3)
        break;
      if(ShowField[i]) {
        t2 = trim(t2);
        paintTxt(mdcWork, x, y, t2, i, drawInfo);
        PRect r(x, y, pxTabs[i], drawInfo.rcItem.bottom);
        paintCustom(mdcWork, r, i, drawInfo);
        x = pxTabs[i];
        }
      t2 = t3;
      }
    DeleteObject(SelectObject(mdcWork, oldPen));

    if (drawInfo.itemState & ODS_FOCUS)
      DrawFocusRect(mdcWork, &drawInfo.rcItem);

    if(oldFont)
      SelectObject(mdcWork, oldFont);
    BitBlt(hdc, r.left, r.top, r.Width(), r.Height(), mdcWork, r.left, r.top, SRCCOPY);
    SelectObject(mdcWork, oldObjWork);
    DeleteDC(mdcWork);
    DeleteObject(hBmpWork);
    }
}
//-----------------------------------------------------------
bool PListBox::create()
{
   bool result = PControl::create();
   SetTab();
   return result;
}
//-----------------------------------------------------------
void PListBox::SetTabStop(int ntabs, int *tabs, int *show)
{
  nTabs = ntabs;
  delete []charTabs;
  charTabs = new int[ntabs];
  memcpy(charTabs, tabs, ntabs * sizeof(int));
  delete []ShowField;
  ShowField = new bool[ntabs];
  delete Align;
  Align = new tAlign[nTabs];
//  delete []pxTabs;
//  pxTabs = new int[nTabs];
  for(int i = 0; i < nTabs; ++i) {
    ShowField[i] = !show ? true : toBool(show[i]);
    if(ShowField[i]) {
      if(charTabs[i] < 0) {
        Align[i] = aRight;
        charTabs[i] = -charTabs[i];
        }
      else
        Align[i] = aLeft;
      }
//    pxTabs[i] = i + 1;
    }
}
//-----------------------------------------------------------
void PListBox::setHorzExt()
{
  if(getHandle() && nTabs && pxTabs)
    SendMessage(getHandle(), LB_SETHORIZONTALEXTENT, pxTabs[nTabs - 1] + 2, 0);
}
//-----------------------------------------------------------
void PListBox::SetTab()
{
  double DlgUnits = pixelWidthChar;
  if(AUTO_PIX_WIDTH == pixelWidthChar)
    DlgUnits = (double)getSizeFont().cx / SIMUL_DEC_FONT;

  int oldLen = MaxLen;
  MaxLen = 0;
  if(charTabs) {
    delete []pxTabs;
    pxTabs = new int[nTabs];
    int offs = 0;
    for(int i = 0; i < nTabs; ++i) {
      if(ShowField[i]) {
        int len = charTabs[i];
        MaxLen += len + 1;
#if 1
        double t = 1.0 / (len * len) + 1.0;
        pxTabs[i] = (int)(len * DlgUnits * t + 0.5);
#else
        if(len > 10)
          pxTabs[i] = (len - 3) * DlgUnits;
        else if(len > 3)
          pxTabs[i] = (len - 1) * DlgUnits;
        else
          pxTabs[i] = len * DlgUnits;
        if(len <= 10) {
          double t = sqrt((double)pxTabs[i]);
          pxTabs[i] += t * 3.0;
          }
#endif
        pxTabs[i] += offs;
//        pxTabs[i] = calc_len(len, !i ? 0 : pxTabs[i - 1], DlgUnits);
        }
      else if(i)
        pxTabs[i] = pxTabs[i - 1];
      else
        pxTabs[i] = 0;
      offs = pxTabs[i];
      }
    if(MaxLen > oldLen) {
      delete []Buff;
      Buff = new TCHAR[MaxLen + 2];
      }
    fillStr(Buff, 0, MaxLen + 2);
    setHorzExt();
    }
}
//-----------------------------------------------------------
void PListBox::rescale(int init, int end, double scale)
{
  if(!getHandle() || !pxTabs)
    return;

  int sum = 0;
  int i;
  for(i = init; i <= end && i < nTabs; ++i) {
    int currLen = i ? pxTabs[i] - pxTabs[i - 1] - sum : pxTabs[i];
    int diff = currLen;
    currLen = (int)(currLen * scale);
    diff -= currLen;
    pxTabs[i] -= diff + sum;
    sum += diff;

    }
  for(; i < nTabs; ++i)
    pxTabs[i] -= sum;

  setHorzExt();
}
//-----------------------------------------------------------
inline
int PListBox::calc_len(int len, int offs, double mult)
{
  return (int)(len * mult + offs);
}
//-----------------------------------------------------------
#define TAB _T('\t')
TCHAR *PListBox::get_next(TCHAR *str)
{
  if(!str)
    return 0;
  while(*str && *str != TAB)
    ++str;
  *str++ = 0;

  return str;
}
//-----------------------------------------------------------
void PListBox::setIntegralHeight()
{
  if(!hItem)
    hItem = retrieveHeightFont();
  if(!hItem)
    return;

  RECT r;
  ::GetWindowRect(*this, &r);
  PRect rect(r);
  int origWidth = rect.Width();
  ::GetClientRect(*this, &r);
  int hScroll = GetSystemMetrics(SM_CYHSCROLL);

  if(!(GetWindowLongPtr(*this, GWL_STYLE) & WS_HSCROLL))
    hScroll = 0;

  int vScroll = GetSystemMetrics(SM_CXVSCROLL);

  if(!(GetWindowLongPtr(*this, GWL_STYLE) & WS_VSCROLL))
    vScroll = 0;

  int width = (int)SendMessage(*this, LB_GETHORIZONTALEXTENT, 0, 0);
  if(width < rect.Width() - vScroll)
    hScroll = 0;
  int hBorder = GetSystemMetrics(SM_CYEDGE);
  int diff = (rect.Height() - hScroll - hBorder * 2) % (hItem + ADD_HEIGHT) - hBorder;

  rect.right = rect.left + origWidth;
  if(diff) {
    rect.bottom -= diff;
    setWindowPos(0, rect, SWP_NOZORDER | SWP_NOMOVE);
    }
}
//-----------------------------------------------------------
