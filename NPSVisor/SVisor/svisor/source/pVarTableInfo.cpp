//------ pVarTableInfo.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarTableInfo.h"
#include "p_basebody.h"
#include "p_util.h"
//----------------------------------------------------------------------------
/*
  l'oggetto è formato ad un pannello di sfondo (opzionale, potrebbe essere trasparente)
  da un header con i titoli per le colonne, colori per testo-sfondo (anche qui lo sfondo
      potrebbe essere trasparente), font e tipo di riquadro
  da una serie di righe con impostazioni simili all'header. Nella prima colonna ci va
  un contenuto variabile abbinato ad un bit. Il contenuto può essere dato da un bitmap
  o da un testo. Nel caso di bitmap si può scegliere tra: bit(0-1)
    un solo bitmap -> nascosto-visibile
    un solo bitmap -> fisso-lampeggiante
    due bitmap -> fisso1-fisso2
    due bitmap -> nascosti-alternati 1-2
    due bitmap -> fisso1-alternati 1-2
  nel caso del testo ci deve essere sempre un testo visibile, opzioni raggruppabili:
    0 -> testo fisso
    1 -> cambio colore testo, cambio colore sfondo, cambio testo, alternanza (delle opzioni precedenti)

  Le altre colonne sono di sola visualizzazione e non prevedono alcun cambio.

  Per ora le colonne usate sono tre (o due, impostabile).
  Nella seconda ci va l'indicazione dell'evento, nella terza una descrizione più dettagliata.

codici per l'oggetto:

id,x,y,num_row,offX,offY,idBaseStyle,styleHead,idHeadpanel,idHeadtext,style1RowOff,style1RowOn,idRowpanel,id1BmpOrText,style2_3_row,id2text,id3text,lampTime

dove:
  num_row -> numero di righe di informazione, escluso l'header
  offX,offY -> distanza in x,y tra righe,colonne (compreso l'header)
  idBaseStyle -> pannello di sfondo (può non esserci)
  style... -> style dell'head/rows. Per la prima colonna può essere differente se usa un bitmap di segnalazione
  id...panel -> dimensione del pannello, per le righe contiene solo l'altezza, la larghezza viene presa dall'header
  id...text -> testo visualizzato. Per l'head sono i tre testi della riga, per le righe è una serie per ciascuna colonna
               per la prima colonna può essere l'id da cui caricare le immagini anziché il testo

id+300,standard,
id+600,standard

lo style può essere:
  bit 0-1
  0 -> pannello di sfondo non presente
  1 -> stile UP
  2 -> stile DN
  3 -> stile bordato nero

  bit 2-3
  4 -> testo centrato
  8 -> allineato a sx
 12 -> allineato a dx

  bit 4
+16 -> riempimento

  bit 5
+32 -> usa bmp_txt (questo valore va messo in style1RowOff)
+32 -> usa swapbyte (questo valore va messo nello style puntato da idBaseStyle, dove seguono colori)

nell'idHeadpanel ci vanno:
  h,w1,w2,w3,idFont,RGB_fg,RGB_bg, vanno messe tutte, anche se una colonna non è usata

nell'idRowpanel ci vanno:        1    <---->   2_3
  idFont,RGB_fg,RGB_bg,RGB_fg2,RGB_bg2,     h,idFont,RGB_fg3,RGB_bg3

nell'id...text:
  styleShow,idBmp1,idBmp2,flag -> per la prima colonna, se bitmap
      flag indica se il bitmap va
      0 -> adattato senza distorsione
      1 -> adattato con distorsione
      2 -> senza ridimensionamento

      N.B. il bitmap viene sempre centrato nel rettangolo

  styleShow -> per la prima colonna, se testo
  Primo testo (id + 1)
  Secondo testo (id + 2)

  Primo testo, a seguire (id + 1, ..., n) gli altri -> per i testi normali

styleShow(bitmap) può valere:
  0 -> un solo bitmap, visibile su attivazione del bit
  1 -> un solo bitmap, lampeggiante su attivazione del bit
  2 -> un solo bitmap visibile, lampeggiante su attivazione del bit
  3 -> due bitmap, scambio
  4 -> due bitmap nascosti, alternanti su attivazione del bit
  5 -> due bitmap uno visibile, alternanti su attivazione del bit
 +8 -> usa anche alternanza sfondo del pannello

styleShow(testo) può valere:
  0 -> un solo testo, visibile su attivazione del bit
  1 -> un solo testo, lampeggiante su attivazione del bit
  2 -> un solo testo visibile, lampeggiante su attivazione del bit
  3 -> due testi, scambio
  4 -> due testi nascosti, alternanti su attivazione del bit
  5 -> due testi uno visibile, alternanti su attivazione del bit

*/
//----------------------------------------------------------------------------
#define MAX_DIM_TEXT_INFO 256
#define MAX_COLUMN_TABLE_INFO 3
//----------------------------------------------------------------------------
static LPCTSTR getColor(LPCTSTR p, COLORREF& color)
{
  int r = 0;
  int g = 0;
  int b = 0;
  _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
  color = RGB(r, g, b);
  return findNextParamTrim(p, 3);
}
//----------------------------------------------------------------------------
extern DWORD getGlobalTimeCycle();
//----------------------------------------------------------------------------
namespace varTblInfo
{
//----------------------------------------------------------------------------
  struct common
  {
    COLORREF Fg;
    COLORREF Bg;
    uint Style;
    uint idFont;
    common() : Fg(RGB(0, 0, 0)), Bg(RGB(180, 180, 180)), Style(0), idFont(0)
    {    }
  };
//----------------------------------------------------------------------------
  struct simplePainter
  {
    void paint(P_BaseBody* owner, HDC hdc, const PRect& rect, LPCTSTR txt, const common& obj);
    bool hasPanel(const common& obj) const;
  };
//----------------------------------------------------------------------------
  struct simpleMakeTextPainter
  {
    PTextFixedPanel* make(P_BaseBody* owner, const PRect& rect, LPCTSTR txt, const common& obj);
  };
//----------------------------------------------------------------------------
  struct infoText
  {
    TCHAR Info[MAX_DIM_TEXT_INFO];
    infoText() { ZeroMemory(Info, sizeof(Info));  }
    void load(P_BaseBody* owner, uint id) {
      smartPointerConstString text = owner->getPageStringLang(id);
      if(text)
        copyStrZ(Info, &text, SIZE_A(Info) - 1);
      }
  };
//----------------------------------------------------------------------------
  struct header : public common
  {
    int Height;
    int Width[MAX_COLUMN_TABLE_INFO];
    PVect<PTextFixedPanel*> Text;
    header() : Height(0) {  ZeroMemory(Width, sizeof(Width));   }
    ~header()  {  flushPV(Text);   }

    SIZE calcSize(uint offX);
    bool load(P_BaseBody* owner, const POINT& pt, int offX, uint style, uint idPanel, uint idText);

    void paint(P_BaseBody* owner, HDC hdc, const PRect& rect);
  };
//----------------------------------------------------------------------------
  struct row
  {
    infoText Info[MAX_COLUMN_TABLE_INFO - 1];
    row()  {   }
  };
//----------------------------------------------------------------------------
#define MAX_ROWS 16
//----------------------------------------------------------------------------
  struct firstColumn
  {
    TCHAR Info[2][MAX_DIM_TEXT_INFO];
    uint idBmp[2];
    common Data[2];
    bool useBmp;
    uint styleShow;
    bool oldState[MAX_ROWS];
    uint bmpFlag;
    firstColumn() : useBmp(false), styleShow(0), bmpFlag(0)
        { ZeroMemory(Info, sizeof(Info));  ZeroMemory(idBmp, sizeof(idBmp)); ZeroMemory(oldState, sizeof(oldState));  }

    bool load(P_BaseBody* owner, uint styleOff, uint styleOn, uint idPanel, uint idText_bmp);

    void paint(P_BaseBody* owner, HDC hdc, const PRect& rect, bool active, uint ix);

    void paintText(P_BaseBody* owner, HDC hdc, const PRect& rect, bool active, uint ix);
    void paintBmp(P_BaseBody* owner, HDC hdc, const PRect& rect, bool active, uint ix);
    bool useAlternate() const;

    void swapOldState(DWORD bits, uint tot);
    void resetState(DWORD bits, uint tot);

  };
//----------------------------------------------------------------------------
  class manRow : public common
  {
    public:
      manRow(P_BaseBody* owner) : Owner(owner), hRow(0), Panel(0), Text(0), UseSwap(false),
          offX(0), offY(0), Alternate(false), timeCycle(0), lastTime(0) { Pt.x = 0; Pt.y = 0; }
      ~manRow() { delete Panel; delete Text; }
      bool load(uint id);
      void paint(HDC hdc, const PRect& rect, DWORD bitStat);
      void invalidateByBit(DWORD bitStat);
      PRect getRect();
      P_BaseBody* getOwner() { return Owner; }
      bool useSwap() const { return UseSwap; }
      bool useAlternate() const { return Alternate; }
      bool canShow();
      bool resetTime(DWORD bits);
      void swapOldState(DWORD BitStat);

    private:
      header Head;
      firstColumn fCol;
      PVect<row> Rows;
      uint hRow;
      POINT Pt;
      bool UseSwap;
      bool Alternate;
      uint offX;
      uint offY;

      PPanel* Panel;
      PTextFixedPanel* Text;
      P_BaseBody* Owner;
      PRect calcSize();
      void makePanel(int id);
      void paintFirst(HDC hdc, const PRect& rect, bool active, uint ix);
      DWORD timeCycle;
      DWORD lastTime;
};
//----------------------------------------------------------------------------
}; // end namespace
//----------------------------------------------------------------------------
using namespace varTblInfo;
//----------------------------------------------------------------------------
pVarTableInfo::pVarTableInfo(P_BaseBody* owner, uint id) :  baseVar(owner, id), BitStat(0),
    ManageRow(new manRow(owner))  { }
//----------------------------------------------------------------------------
pVarTableInfo::~pVarTableInfo()
{
  delete ManageRow;
}
//----------------------------------------------------------------------------
bool pVarTableInfo::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  if(!ManageRow->load(getId()))
    return false;

  PRect rect = ManageRow->getRect();

  setRect(rect);

  if(!makeStdVars())
    return false;

  return true;
}
//----------------------------------------------------------------------------
void pVarTableInfo::draw(HDC hdc, const PRect& rect)
{
  if(!isVisible())
    return;
  ManageRow->paint(hdc, rect, BitStat);
}
//----------------------------------------------------------------------------
static WORD swap16(WORD v)
{
  return (v >> 8) | ((v & 0xff) << 8);
}
//----------------------------------------------------------------------------
static DWORD swap32(DWORD v)
{
  DWORD low = swap16(WORD(v >> 16));
  DWORD hi = swap16(WORD(v & 0xffff));
  return (hi << 16) | low;
}
//----------------------------------------------------------------------------
static DWORD alignNBitOffs(DWORD val, uint nbit, uint offs)
{
  if(!nbit) {
    if(!offs)
      return val;
    else
      nbit = 8 * sizeof(DWORD);
    }
  val >>= offs;
  val &= (1 << nbit) - 1;
  return val;
}
//----------------------------------------------------------------------------
bool pVarTableInfo::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible())
    return upd;
  force |= upd;

  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }

  if(!force && prfData::notModified == result) {
    if(ManageRow->useAlternate() && ManageRow->canShow()) {
      ManageRow->invalidateByBit(BitStat);
      ManageRow->swapOldState(BitStat);
      return true;
      }
    return upd;
    }
  bool needSwap = ManageRow->useSwap();

  data.U.dw = alignNBitOffs(data.U.dw, BaseVar.getnBit(),  BaseVar.getOffs());

  DWORD v = BitStat;
  if(needSwap) {
    if(prfData::tWData == data.typeVar || prfData::tWsData == data.typeVar)
      BitStat = swap16(data.U.w);
    else if(prfData::tDWData == data.typeVar || prfData::tDWsData == data.typeVar)
      BitStat = swap32(data.U.dw);
    else
      BitStat = data.U.dw;
    }
  else
    BitStat = data.U.dw;

  force |= ManageRow->resetTime(BitStat);
  if(force)
    v = (DWORD)-1;
  else if(ManageRow->useAlternate())
    v = BitStat;
  else
    v ^= BitStat;
  ManageRow->invalidateByBit(v);
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
namespace varTblInfo
{
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PTextFixedPanel* simpleMakeTextPainter::make(P_BaseBody* owner, const PRect& rect, LPCTSTR txt, const common& obj)
{
  uint typePanel = obj.Style & 3;
  bool filled = toBool(obj.Style & (1 << 4));

  uint type = filled ? PPanel::FILL : 0;
  switch(typePanel) {
    case 0:
    default:
      type |= PPanel::NO;
      break;
    case 1:
      type |= PPanel::UP;
      break;
    case 2:
      type |= PPanel::DN;
      break;
    case 3:
      type |= PPanel::BORDER;
      break;
    }

  uint align = (obj.Style >> 2) & 3;

  HFONT font = owner->getFont(obj.idFont);

  PTextFixedPanel* t = new PTextFixedPanel(txt, rect, font, obj.Fg, (PPanel::bStyle)type, obj.Bg);

  switch(align) {
    case 1:
    default:
      t->setAlign(TA_CENTER);
      break;
    case 2:
      t->setAlign(TA_LEFT);
      break;
    case 3:
      t->setAlign(TA_RIGHT);
      break;
    }
  t->setVAlign(DT_VCENTER);
  return t;
}
//----------------------------------------------------------------------------
bool simplePainter::hasPanel(const common& obj) const
{
  uint typePanel = obj.Style & 3;
  bool filled = toBool(obj.Style & (1 << 4));
  return filled || typePanel != PPanel::NO;
}
//----------------------------------------------------------------------------
void simplePainter::paint(P_BaseBody* owner, HDC hdc, const PRect& rect, LPCTSTR txt, const common& obj)
{
  PTextFixedPanel* t = simpleMakeTextPainter().make(owner, rect, txt, obj);
  t->draw(hdc);
  delete t;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool header::load(P_BaseBody* owner, const POINT& pt, int offX, uint style, uint idPanel, uint idText)
{
  Style = style;
  LPCTSTR p = owner->getPageString(idPanel);
  if(!p)
    return false;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &Height, Width, Width + 1, Width + 2, &idFont);

  // la prima colonna deve essere sempre presente, se manca la seconda si scambia con la terza
/*
  if(!Width[1]) {
    Width[1] = Width[2];
    Width[2] = 0;
    }
*/
  for(uint i = 0; i < SIZE_A(Width); ++i)
    Width[i] = R__X(Width[i]);
  Height = R__Y(Height);

  idFont -= ID_INIT_FONT;
  p = findNextParamTrim(p, 5);
  if(!p)
    return false;
  p = getColor(p, Fg);
  if(!p)
    return false;
  p = getColor(p, Bg);

  uint offs = 0;
  for(int i = 0; i < SIZE_A(Width); ++i) {
    PRect r(0, 0, Width[i], Height);
    r.MoveTo(pt.x + offs, pt.y);
    infoText c;
    c.load(owner, idText + i);
    Text[i] = simpleMakeTextPainter().make(owner, r, c.Info, *this);
    offs += Width[i] ? Width[i] + offX : 0;
    }
  return true;
//  h,w1,w2,w3,idFont,RGB_fg,RGB_bg, vanno messe tutte, anche se una colonna non è usata
}
//----------------------------------------------------------------------------
SIZE header::calcSize(uint offX)
{
  uint nElem = Text.getElem();
  SIZE sz = { 0, Height };
  for(uint i = 0; i < nElem; ++i)
    sz.cx += Width[i] ? Width[i] + offX : 0;
  sz.cx -= offX;
  return sz;
}
//----------------------------------------------------------------------------
void header::paint(P_BaseBody* owner, HDC hdc, const PRect& rect)
{
  uint nElem = Text.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(!Text[i]->getRect().Width())
      continue;
    if(rect.Intersec(Text[i]->getRect()))
      Text[i]->draw(hdc);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//style1Row, idRowpanel, id1BmpOrText
bool firstColumn::load(P_BaseBody* owner, uint styleOff, uint styleOn, uint idPanel, uint idText_bmp)
{
  useBmp = toBool(styleOff & (1 << 5));
  styleOff &= ~(1 << 5);
  Data[0].Style = styleOff;
  Data[1].Style = styleOn;

  //idPanel->idFont,RGB_fg,RGB_bg,RGB_fg2,RGB_bg2
  LPCTSTR p = owner->getPageString(idPanel);
  if(!p)
    return false;
  Data[0].idFont = _ttoi(p);
  Data[0].idFont -= ID_INIT_FONT;
  Data[1].idFont = Data[0].idFont;
  p = findNextParamTrim(p);
  if(!p)
    return false;

  p = getColor(p, Data[0].Fg);
  if(!p)
    return false;
  p = getColor(p, Data[0].Bg);
  if(!p)
    return false;

  p = getColor(p, Data[1].Fg);
  if(!p)
    return false;
  p = getColor(p, Data[1].Bg);

  p = owner->getPageString(idText_bmp);
  if(!p)
    return false;
  styleShow = _ttoi(p);
  if(useBmp) {
    p = findNextParamTrim(p);
    if(!p)
      return false;
    idBmp[0] = _ttoi(p);
    p = findNextParamTrim(p);
    if(p)
      idBmp[1] = _ttoi(p);
    p = findNextParamTrim(p);
    if(p)
      bmpFlag = _ttoi(p);
    }
  else {
    smartPointerConstString text = owner->getPageStringLang(idText_bmp + 1);
    if(text)
      copyStrZ(Info[0], &text, SIZE_A(Info[0]) - 1);
    text = owner->getPageStringLang(idText_bmp + 2);
    if(text)
      copyStrZ(Info[1], &text, SIZE_A(Info[1]) - 1);
    }
  return true;
}
//----------------------------------------------------------------------------
bool firstColumn::useAlternate() const
{
  switch(styleShow & ~8) {
    case 1:
    case 2:
    case 4:
    case 5:
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void firstColumn::paintBmp(P_BaseBody* owner, HDC hdc, const PRect& rect, bool active, uint ixRow)
{
  LPCTSTR txt = _T("");
  bool useSwapPanel = toBool(styleShow & 8);
  uint ix = useSwapPanel ? active : 0;
  int id = -1;
  switch(styleShow & ~8) {
/*
  0 -> un solo bitmap, visibile su attivazione del bit
  1 -> un solo bitmap, lampeggiante su attivazione del bit
  2 -> un solo bitmap visibile, lampeggiante su attivazione del bit
  3 -> due bitmap, scambio
  4 -> due bitmap nascosti, alternanti su attivazione del bit
  5 -> due bitmap uno visibile, alternanti su attivazione del bit
 +8 -> usa anche alternanza sfondo del pannello
*/
    case 0:
    default:
      if(active)
        id = idBmp[0];
      break;
    case 2:
      if(!active) {
        id = idBmp[0];
        break;
        }
      // fall through
    case 1:
      if(active) {
        if(!oldState[ixRow]) {
          id = idBmp[0];
          ix = 0;
          }
        }
      break;
    case 3:
      id = idBmp[active];
      break;
    case 5:
      if(!active) {
        id = idBmp[0];
        break;
        }
      // fall through
    case 4:
      if(active) {
        id = idBmp[oldState[ixRow]];
        if(useSwapPanel)
          ix = oldState[ixRow];
        }
      break;
    }
  simplePainter sP;
  if(sP.hasPanel(Data[ix]))
    sP.paint(owner, hdc, rect, _T(""), Data[ix]);
  id -= ID_INIT_BMP_4_BTN;
  if(id < 0)
    return;
  PBitmap* bmp = owner->getBmp4Var(id);
  if(!bmp)
    return;

  SIZE sz = bmp->getSize();
  POINT pt = { (rect.right + rect.left - sz.cx) / 2, (rect.bottom + rect.top - sz.cy) / 2 };
  PTraspBitmap tBmp(owner, bmp, pt);
  switch(bmpFlag) {
    case 0:
      do {
        double rX = rect.Width();
        rX /= sz.cx;
        double rY = rect.Height();
        rY /= sz.cy;

        if(rX < rY) {
          tBmp.setScale(rX);
          pt.x = rect.left;
          pt.y += LONG(sz.cy * (1 - rX) / 2);
          }
        else {
          tBmp.setScale(rY);
          pt.y = rect.top;
          pt.x += LONG(sz.cx * (1 - rY) / 2);
          }
        } while(false);
      break;
    case 1:
      do {
        double rX = rect.Width();
        rX /= sz.cx;
        double rY = rect.Height();
        rY /= sz.cy;

        tBmp.setScale(rX, rY);
        pt.x = rect.left;
        pt.y = rect.top;
        } while(false);
      break;
    case 2:
    default:
      break;
    }
  tBmp.moveToSimple(pt);
  tBmp.Draw(hdc);
}
//----------------------------------------------------------------------------
void firstColumn::paintText(P_BaseBody* owner, HDC hdc, const PRect& rect, bool active, uint ixRow)
{
  LPCTSTR txt = _T("");
  uint ix = 0;
  switch(styleShow) {
/*
  0 -> un solo testo, visibile su attivazione del bit
  1 -> un solo testo, lampeggiante su attivazione del bit
  2 -> un solo testo visibile, lampeggiante su attivazione del bit
  3 -> due testi, scambio
  4 -> due testi nascosti, alternanti su attivazione del bit
  5 -> due testi uno visibile, alternanti su attivazione del bit
*/
    case 0:
    default:
      if(active)
        txt = Info[0];
      break;
    case 2:
      if(!active) {
        txt = Info[0];
        break;
        }
      // fall through
    case 1:
      if(active) {
        if(!oldState[ixRow])
          txt = Info[0];
        }
      break;
    case 3:
      txt = Info[active];
      ix = active;
      break;
    case 5:
      if(!active) {
        txt = Info[0];
        break;
        }
      // fall through
    case 4:
      if(active) {
        txt = Info[oldState[ixRow]];
        ix = oldState[ixRow];
//        ix = active;
        }
      break;
    }
  simplePainter().paint(owner, hdc, rect, txt, Data[ix]);
}
//----------------------------------------------------------------------------
void firstColumn::paint(P_BaseBody* owner, HDC hdc, const PRect& rect, bool active, uint ix)
{
  if(useBmp)
    paintBmp(owner, hdc, rect, active, ix);
  else
    paintText(owner, hdc, rect, active,ix );
}
//----------------------------------------------------------------------------
void firstColumn::resetState(DWORD bits, uint tot)
{
  for(uint i = 0; i < tot; ++i, bits >>= 1)
    oldState[i] = bits & 1;
}
//----------------------------------------------------------------------------
void firstColumn::swapOldState(DWORD bits, uint tot)
{
  for(uint i = 0; i < tot; ++i, bits >>= 1)
    if(bits & 1)
      oldState[i] = !oldState[i];
}
//----------------------------------------------------------------------------
void manRow::swapOldState(DWORD BitStat)
{
  uint nElem = Rows.getElem();
  fCol.swapOldState(BitStat, nElem);
}
//----------------------------------------------------------------------------
bool manRow::resetTime(DWORD bits)
{
  bool ret = toBool(lastTime);
  lastTime = 0;
  uint nElem = Rows.getElem();
  fCol.resetState(bits, nElem);
  return ret;
}
//----------------------------------------------------------------------------
bool manRow::canShow()
{
  DWORD curr = GetTickCount();
  if(curr - lastTime < timeCycle)
    return false;
  lastTime = curr;
  return true;
}
//----------------------------------------------------------------------------
bool manRow::load(uint id)
{
  LPCTSTR p = getOwner()->getPageString(id);
//id,x,y,num_row,offX,offY,idBaseStyle,styleHead,idHeadpanel,idHeadtext,style1RowOff,style1RowOn,idRowpanel,id1BmpOrText,style2_3_row,id2text,id3text

  int x = 0;
  int y = 0;
  int num_row = 0;
  int idBaseStyle = 0;
  int styleHead = 0;
  int idHeadpanel = 0;
  int idHeadtext = 0;
  int style1RowOff = 0;
  int style1RowOn = 0;
  int idRowpanel = 0;
  int id1BmpOrText = 0;
  int style2_3_row = 0;
  int id2text = 0;
  int id3text= 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &num_row, &offX, &offY, &idBaseStyle,
                    &styleHead, &idHeadpanel, &idHeadtext,
                    &style1RowOff, &style1RowOn, &idRowpanel, &id1BmpOrText,
                    &style2_3_row, &id2text, &id3text, &timeCycle);

  if(!timeCycle)
    timeCycle = getGlobalTimeCycle();
  else
    timeCycle = -(int)timeCycle;
  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  Pt.x = R__X(x);
  Pt.y = R__Y(y);

  offX = R__X(offX);
  offY = R__Y(offY);

//  int numCol = int(toBool(id2text)) + int(toBool(id3text)) + 1;
  // info header
  if(!Head.load(getOwner(), Pt, offX, styleHead, idHeadpanel, idHeadtext))
    return false;

  // info prima colonna
  if(!fCol.load(getOwner(), style1RowOff, style1RowOn, idRowpanel, id1BmpOrText))
    return false;

  p = getOwner()->getPageString(idRowpanel);
  p = findNextParamTrim(p, 13);
  if(!p)
    return false;

  // info restanti colonne per tutte le righe
  Style = style2_3_row;
  hRow = R__Y(_ttoi(p));
  p = findNextParamTrim(p);
  if(!p)
    return false;

  idFont = _ttoi(p) - ID_INIT_FONT;
  p = findNextParamTrim(p);
  if(!p)
    return false;

  p = getColor(p, Fg);
  if(!p)
    return false;
  p = getColor(p, Bg);

  int ix = 0;
  if(id2text) {
    for(int i = 0; i < num_row; ++i)
      Rows[i].Info[0].load(getOwner(), id2text + i);
    ++ix;
    }
  if(id3text) {
    for(int i = 0; i < num_row; ++i)
      Rows[i].Info[ix].load(getOwner(), id3text + i);
    ++ix;
    }
  PRect r;
  // coordinate-dimensioni e testo verranno assegnate al volo
  Text = simpleMakeTextPainter().make(getOwner(), r, _T(" "), *this);
  // va fatto al termine, deve usare le dimensioni da calcolare
  makePanel(idBaseStyle);

  Alternate = fCol.useAlternate();
  return true;
}
//----------------------------------------------------------------------------
void manRow::makePanel(int id)
{
  LPCTSTR p = getOwner()->getPageString(id);
  if(!p)
    return;
  uint style = _ttoi(p);
  p = findNextParamTrim(p);
  COLORREF bg;
  if(!p)
    bg = RGB(255, 255, 255);
  else
    p = getColor(p, bg);

  if(p && _ttoi(p))
    UseSwap = true;

  uint typePanel = style & 3;
  bool filled = toBool(style & (1 << 4));

  uint type = filled ? PPanel::FILL : 0;
  switch(typePanel) {
    case 0:
    default:
      type |= PPanel::NO;
      break;
    case 1:
      type |= PPanel::UP;
      break;
    case 2:
      type |= PPanel::DN;
      break;
    case 3:
      type |= PPanel::BORDER;
      break;
    }
  PRect rect = calcSize();
  rect.Inflate(offX, offY);
  rect.Offset(offX, offY);
  Panel = new PPanel(rect, bg, (PPanel::bStyle)style);
//  Panel = new PPanel(rect, bg, (PPanel::bStyle)type);
}
//----------------------------------------------------------------------------
PRect manRow::getRect()
{
  return Panel ? Panel->getRect() : calcSize();
}
//----------------------------------------------------------------------------
PRect manRow::calcSize()
{
  SIZE sz = Head.calcSize(offX);
  sz.cy += (hRow + offY) * Rows.getElem();
  // aumenta i bordi per il panel di sfondo
  PRect r(0, 0, sz.cx + offX * 2, sz.cy + offY * 2);
  r.MoveTo(Pt.x, Pt.y);
  return r;
}
//----------------------------------------------------------------------------
void manRow::paintFirst(HDC hdc, const PRect& rect, bool active, uint ix)
{
  fCol.paint(getOwner(), hdc, rect, active, ix);
}
//----------------------------------------------------------------------------
void manRow::paint(HDC hdc, const PRect& rect, DWORD bitStat)
{
  if(Panel && rect.Intersec(Panel->getRect()))
    Panel->draw(hdc);

  Head.paint(getOwner(), hdc, rect);
  int y = Pt.y + Head.Height + offY;
  uint nElem = Rows.getElem();
  int nInfo = Head.Text.getElem() - 1;
  for(uint i = 0; i < nElem; ++i) {
    PRect r(0, 0, Head.Width[0], hRow);
    r.MoveTo(Pt.x, y);
    if(Head.Width[0]) {
      if(r.Intersec(rect))
        paintFirst(hdc, r, toBool(bitStat & (1 << i)), i);
      }
    else
      r.right -= offX;
    const row& Rw = Rows[i];
    for(int j = 0; j < nInfo; ++j) {
      if(Head.Width[j + 1]) {
        r.MoveTo(r.right + offX, r.top);
        r.right = r.left + Head.Width[j + 1];
        if(r.Intersec(rect)) {
          Text->setRect(r);
          Text->drawText(hdc, Rw.Info[j].Info);
          }
        }
      }
    y += hRow + offY;
    }
}
//----------------------------------------------------------------------------
void manRow::invalidateByBit(DWORD bits)
{
  uint nElem = Rows.getElem();
  int y = Pt.y + Head.Height + offY;
  PRect r(0, 0, Head.Width[0], hRow);
  for(uint i = 0; i < nElem; ++i) {
    if(bits & (1 << i)) {
      r.MoveTo(Pt.x, y);
      InvalidateRect(*getOwner(), r, true);
      }
    y += hRow + offY;
    }
}
//----------------------------------------------------------------------------
}; // end namespace
