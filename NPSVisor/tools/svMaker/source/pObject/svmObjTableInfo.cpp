//-------------------- svmObjTableInfo.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------
#include "resource.h"
#include "p_File.h"
#include "pModDialog.h"
#include "svmObjTableInfo.h"
#include "svmDefObj.h"
#include "svmPropertyTableInfo.h"
#include "PTraspBitmap.h"
#include "common.h"
#include "ptextpanel.h"
//-----------------------------------------------------------
//----------------------------------------------------------------------------
namespace varTblInfo
{
//----------------------------------------------------------------------------
#define MAX_DIM_TEXT_INFO 256
#define MAX_COLUMN_TABLE_INFO 3
//----------------------------------------------------------------------------
  struct common
  {
    COLORREF Fg;
    COLORREF Bg;
    uint Style;
    uint idFont;
    uint alignText;
    common() : Fg(RGB(0, 0, 0)), Bg(RGB(180, 180, 180)), Style(0), idFont(0), alignText(0)
    {    }
  };
//----------------------------------------------------------------------------
  struct simplePainter
  {
    void paint(HDC hdc, const PRect& rect, LPCTSTR txt, const common& obj, bool linked);
    bool hasPanel(const common& obj) const;
  };
//----------------------------------------------------------------------------
  struct simpleMakeTextPainter
  {
    PTextFixedPanel* make(const PRect& rect, LPCTSTR txt, const common& obj, bool linked);
  };
//----------------------------------------------------------------------------
  struct header : public common
  {
    static SIZE calcSize(const PropertyTableInfo& pt);
    void paint(HDC hdc, const PRect& rect, const PropertyTableInfo& pti, bool linked);
  };
//----------------------------------------------------------------------------
#define MAX_ROWS 16
//----------------------------------------------------------------------------
  struct firstColumn
  {
    firstColumn(const PropertyTableInfo& pti, bool linked) : useBmp(false), styleShow(0), bmpFlag(0),
        pti(pti), linked(linked)
        { load();  }

    void paint(HDC hdc, const PRect& rect, bool active);

    private:
      common Data[2];
      bool useBmp;
      uint styleShow;
      uint bmpFlag;
      const PropertyTableInfo& pti;
      bool linked;
      void paintText(HDC hdc, const PRect& rect, bool active);
      void paintBmp(HDC hdc, const PRect& rect, bool active);
      void load();
  };
//----------------------------------------------------------------------------
  class manRow : public common
  {
    public:
      manRow(const PropertyTableInfo& owner, bool linked) : Owner(owner),
          fCol(owner, linked), Linked(linked)   {  }
      ~manRow() {  }
      void paint(HDC hdc, const PRect& rect, WORD bitStat);
      static SIZE calcSize(const PropertyTableInfo& pt);
    private:
      header Head;
      firstColumn fCol;

      const PropertyTableInfo& Owner;
      bool Linked;
      void paintFirst(HDC hdc, const PRect& rect, bool active);
};
//----------------------------------------------------------------------------
}; // end namespace
//-----------------------------------------------------------
static PropertyTableInfo staticPT;
//-----------------------------------------------------------
inline PRect makeRectCheck(int x, int y)
{
  if(DATA_NOT_USED == x)
    x = 10;
  if(DATA_NOT_USED == y)
    y = 10;
  return PRect(x, y, x + 30, y + 30);
}
//-----------------------------------------------------------
svmObjTableInfo::svmObjTableInfo(PWin* par, int id, int x, int y) :
  svmObject(par, id, oTBL_INFO, makeRectCheck(x, y))
{
  init();
}
//-----------------------------------------------------------
svmObjTableInfo::svmObjTableInfo(PWin* par, int id, const PRect& r) :
  svmObject(par, id, oTBL_INFO, makeRectCheck(r.left, r.top))
{
  init();
}
//-----------------------------------------------------------
PRect svmObjTableInfo::calcRect(int x, int y) const
{
  const PropertyTableInfo* pti = dynamic_cast<const PropertyTableInfo*>(Prop);
  SIZE sz = varTblInfo::manRow::calcSize(*pti);
  PRect r(0, 0, sz.cx, sz.cy);
  r.MoveTo(x, y);
  return r;
}
//-----------------------------------------------------------
void svmObjTableInfo::init()
{
  Prop = allocProperty();
  Prop->Rect = getFirstRect();
  Prop->Rect = calcRect(Prop->Rect.left, Prop->Rect.top);
}
//-----------------------------------------------------------
const PRect& svmObjTableInfo::getRect() const
{
  Prop->Rect = calcRect(Prop->Rect.left, Prop->Rect.top);
  return baseClass::getRect();
}
//-----------------------------------------------------------
void svmObjTableInfo::setRect(const PRect& rect)
{
  PRect r = calcRect(rect.left, rect.top);
  baseClass::setRect(r);
}
//-----------------------------------------------------------
svmObject::typeOfAnchor svmObjTableInfo::pointIn(const POINT& pt) const
{
// si è copiato qui il codice per evitare controlli inutili, altrimenti
// per un più corretto uso sarebbe dovuto essere: se il ritorno dalla classe
// base è diverso da Extern, tornare Inside, altrimenti Extern
  int x = pt.x;
  int y = pt.y;
  if(x < Prop->Rect.left || y < Prop->Rect.top)
    return Extern;
  if(x > Prop->Rect.right || y > Prop->Rect.bottom)
    return Extern;

  return Inside;
}
//----------------------------------------------------------------------------
void svmObjTableInfo::DrawObject(HDC hdc)
{
  PRect r = getRect();
  varTblInfo::manRow(*dynamic_cast<PropertyTableInfo*>(Prop), Linked).paint(hdc, r, 0x50af);
}
//-----------------------------------------------------------
smartPointerConstString svmObjTableInfo::getTitle() const
{
  return smartPointerConstString(0, 0);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PRect svmObjTableInfo::getRectTitle(HDC, LPCTSTR /*title*/) const
{
  return getRect();
}
//-----------------------------------------------------------
svmObject* svmObjTableInfo::makeClone()
{
  svmObjTableInfo* obj = new svmObjTableInfo(getParent(), getId(), getRect());
  *obj->Prop = *Prop;
  return obj;
}
//-----------------------------------------------------------
void svmObjTableInfo::Clone(const svmObject& other)
{
  *Prop = *other.getProp();
}
//-----------------------------------------------------------
Property* svmObjTableInfo::allocProperty() { return new PropertyTableInfo; }
//-----------------------------------------------------------
svmObjHistory::typeOfAction svmObjTableInfo::dialogProperty(bool onCreate)
{
  if(onCreate) {
    if(staticPT.Rect.Width()) {
      staticPT.Rect.MoveTo(Prop->Rect.left, Prop->Rect.top);
      if(Prop->getFullCloneOnCreate())
        *Prop = staticPT;
      else
        (dynamic_cast<PropertyTableInfo*>(Prop))->cloneMinusProperty(staticPT);
      }
    else {
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      }
    }
  else {
    if(!oldProp)
      oldProp = allocProperty();
    *oldProp = *Prop;
    if(IDOK == svmDialogTableInfo(this, Prop, getParent()).modal()) {
      if(Prop->getFullCloneOnCreate())
        staticPT = *Prop;
      else
        staticPT.cloneMinusProperty(*Prop);
      return svmObjHistory::Change;
      }
    }
  return baseClass::dialogProperty(onCreate);
}
//-----------------------------------------------------------
uint svmObjTableInfo::saveBasePanel(P_File& pf)
{
  svmObjCount& objCount = getObjCount();
  uint idBaseStyle = objCount.getGlobalIdCount();

  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  TCHAR buff[1024];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d\r\n"),
    idBaseStyle, pt->style,
    GetRValue(pt->background), GetGValue(pt->background), GetBValue(pt->background),
    pt->useSwap);
  writeStringChkUnicode(pf, buff);
  return idBaseStyle;
 }
//-----------------------------------------------------------
uint svmObjTableInfo::saveHeadPanel(P_File& pf)
{
  svmObjCount& objCount = getObjCount();
  uint idPanel = objCount.getGlobalIdCount();

  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  TCHAR buff[1024];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
    idPanel, REV__Y(pt->HeightHeader),
    REV__X(pt->Width[0]), REV__X(pt->Width[1]), REV__X(pt->Width[2]),
    pt->idFont + ID_INIT_FONT,
    GetRValue(pt->foreground), GetGValue(pt->foreground), GetBValue(pt->foreground),
    GetRValue(pt->type2), GetGValue(pt->type2), GetBValue(pt->type2)
    );
  writeStringChkUnicode(pf, buff);
  return idPanel;
}
//-----------------------------------------------------------
uint svmObjTableInfo::saveHeadText(P_File& pf, P_File& pfCrypt)
{
  svmObjCount& objCount = getObjCount();
  uint idText = objCount.getGlobalIdCount(NCOL_FULL);

  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  TCHAR buff[4096];
  for(uint i = 0; i < NCOL_FULL; ++i) {
    LPCTSTR p = pt->textHeader[i];
    if(p) {
      wsprintf(buff, _T("%d,%s\r\n"), idText + i, p);
      writeStringChkUnicode(pf, pfCrypt, buff);
      }
    }
  return idText;
}
//-----------------------------------------------------------
//nell'idRowpanel ci vanno:        1    <---->   2_3
//idFont,RGB_fg,RGB_bg,RGB_fg2,RGB_bg2,     h,idFont,RGB_fg3,RGB_bg3

uint svmObjTableInfo::saveRowPanel(P_File& pf)
{
  svmObjCount& objCount = getObjCount();
  uint idPanel = objCount.getGlobalIdCount();

  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  TCHAR buff[4096];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n"),
    idPanel, pt->infoFirst[0].idFont + ID_INIT_FONT,
    GetRValue(pt->infoFirst[0].fg), GetGValue(pt->infoFirst[0].fg), GetBValue(pt->infoFirst[0].fg),
    GetRValue(pt->infoFirst[0].bg), GetGValue(pt->infoFirst[0].bg), GetBValue(pt->infoFirst[0].bg),
    GetRValue(pt->infoFirst[1].fg), GetGValue(pt->infoFirst[1].fg), GetBValue(pt->infoFirst[1].fg),
    GetRValue(pt->infoFirst[1].bg), GetGValue(pt->infoFirst[1].bg), GetBValue(pt->infoFirst[1].bg),

    REV__Y(pt->heightRows), pt->infoRows.idFont + ID_INIT_FONT,
    GetRValue(pt->infoRows.fg), GetGValue(pt->infoRows.fg), GetBValue(pt->infoRows.fg),
    GetRValue(pt->infoRows.bg), GetGValue(pt->infoRows.bg), GetBValue(pt->infoRows.bg)
    );
  writeStringChkUnicode(pf, buff);
  return idPanel;
}
//-----------------------------------------------------------
/*
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
*/
uint svmObjTableInfo::saveFirstText(P_File& pfCrypt, P_File& pfClear)
{
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  svmObjCount& objCount = getObjCount();
  uint style = pt->infoFirst[0].style;
  bool useBmp = toBool(style & USE_BMP);
  uint idText = objCount.getGlobalIdCount(3);

  TCHAR buff[4096];
  if(useBmp) {
    dual* dualBmp = getDualBmp4Btn();
    uint idBmp1 = pt->First[0] ? dualBmp->addAndGetId(pt->First[0]) : 0;
    uint idBmp2 = pt->First[1] ? dualBmp->addAndGetId(pt->First[1]) : 0;
    wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"), idText, pt->styleShow, idBmp1, idBmp2, pt->infoFirst[1].alignText);
    writeStringChkUnicode(pfCrypt, buff);
    }
  else {
    wsprintf(buff, _T("%d,%d\r\n"), idText, pt->styleShow);
    writeStringChkUnicode(pfCrypt, buff);
    LPCTSTR p0 = pt->First[0];
    LPCTSTR p1 = pt->First[1];
    wsprintf(buff, _T("%d,%s\r\n"), idText + 1, p0 ? p0 : _T(" "));
    writeStringChkUnicode(pfClear, pfCrypt, buff);
    wsprintf(buff, _T("%d,%s\r\n"), idText + 2, p1 ? p1 : _T(" "));
    writeStringChkUnicode(pfClear, pfCrypt, buff);
    }
  return idText;
}
//-----------------------------------------------------------
uint svmObjTableInfo::saveInfoText(P_File& pf, P_File& pfCrypt, PVect<LPCTSTR>& list)
{
  svmObjCount& objCount = getObjCount();
  uint idText = objCount.getGlobalIdCount(NROW_FULL);

  TCHAR buff[4096];
  for(uint i = 0; i < NROW_FULL; ++i) {
    LPCTSTR p = list[i];
    if(p) {
      wsprintf(buff, _T("%d,%s\r\n"), idText + i, p);
      writeStringChkUnicode(pf, pfCrypt, buff);
      }
    }
  return idText;
}
//-----------------------------------------------------------
static uint combineBits(uint border, uint align, DWORD preserveMask = 0)
{
  DWORD t = border;
  border &= ~preserveMask;
  preserveMask &= t;
  uint style = 0;
  if(border & Property::UP)
    style = 1;
  else if(border & Property::DN)
    style = 2;
  else if(border & Property::BORDER)
    style = 3;
  if(border & Property::FILL)
    style |= 16;

  style |= (align + 1) << 2;
  style |= preserveMask;
  return style;
}
//-----------------------------------------------------------
static void explodeBits(uint style, uint& border, uint& align, DWORD preserveMask = 0)
{
  DWORD t = style;
  style &= ~preserveMask;
  preserveMask &= t;
  align = ((style >> 2) & 3) - 1;
  style &= ~(3 << 2);
  border = 0;
  if(style & 16) {
    border = Property::FILL;
    style &= ~16;
    }
  switch(style) {
    case 1:
      border |= Property::UP;
      break;
    case 2:
      border |= Property::DN;
      break;
    case 3:
      border |= Property::BORDER;
      break;
    }
  border |= preserveMask;
}
//-----------------------------------------------------------
bool svmObjTableInfo::save(P_File& pfCrypt, P_File& pfClear, uint order)
{
  manageObjId moi(0, ID_INIT_VAR_TABLE_INFO);
  uint id = moi.calcAndSetExtendId(order);
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);

  uint idBaseStyle = saveBasePanel(pfCrypt);
  uint idHeadPanel = saveHeadPanel(pfCrypt);
  uint idHeadText = saveHeadText(pfClear, pfCrypt);
  uint idRowPanel = saveRowPanel(pfCrypt);

  uint idFirstText = saveFirstText(pfCrypt, pfClear);
  uint idSecondText = saveInfoText(pfClear, pfCrypt, pt->Second);
  uint idThirdText = saveInfoText(pfClear, pfCrypt, pt->Third);

  svmObjCount& objCount = getObjCount();

  uint styleHead = combineBits(Prop->type1, pt->alignText);
  uint styleOff = combineBits(pt->infoFirst[0].style, pt->infoFirst[0].alignText, USE_BMP);
  uint styleOn = combineBits(pt->infoFirst[1].style, pt->infoFirst[0].alignText);
  uint styleRows = combineBits(pt->infoRows.style, pt->infoRows.alignText);

  TCHAR buff[4096];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,-%d\r\n"),
            id, REV__X(Prop->Rect.left), REV__Y(Prop->Rect.top),
            pt->nRow, REV__X(pt->offsX), REV__Y(pt->offsY),
            idBaseStyle, styleHead, idHeadPanel, idHeadText,
            styleOff, styleOn,
            idRowPanel, idFirstText, styleRows,
            idSecondText, idThirdText, pt->timeCycle
            );
  writeStringChkUnicode(pfCrypt, buff);

  saveVar(pfCrypt, id);

  return baseClass::save(pfCrypt, pfClear, id);
}
//-----------------------------------------------------------
void svmObjTableInfo::loadBasePanel(uint id, setOfString& set)
{
  LPCTSTR p = set.getString(id);
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  uint r;
  uint g;
  uint b;
  uint useSwap;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &pt->style, &r, &g, &b, &useSwap);
  pt->useSwap = toBool(useSwap);
  pt->background = RGB(r, g, b);
}
//-----------------------------------------------------------
void svmObjTableInfo::loadHeadPanel(uint id, setOfString& set)
{
  LPCTSTR p = set.getString(id);
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);

  uint r;
  uint g;
  uint b;
  uint r2;
  uint g2;
  uint b2;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
    &pt->HeightHeader, &pt->Width[0], &pt->Width[1], &pt->Width[2],
    &pt->idFont,
    &r, &g, &b, &r2, &g2, &b2
    );

  pt->idFont -= ID_INIT_FONT;
  pt->foreground = RGB(r, g, b);
  pt->type2 = RGB(r2, g2, b2);

  pt->HeightHeader = R__Y(pt->HeightHeader);
  for(uint i = 0; i < SIZE_A(pt->Width); ++i)
    pt->Width[i] = R__X(pt->Width[i]);
}
//-----------------------------------------------------------
void svmObjTableInfo::loadHeadText(uint id, setOfString& set)
{
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  for(uint i = 0; i < NCOL_FULL; ++i) {
    LPCTSTR p = set.getString(id + i);
    pt->textHeader[i] = p ? str_newdup(p) : 0;
    }
}
//-----------------------------------------------------------
void svmObjTableInfo::loadRowPanel(uint id, setOfString& set)
{
  LPCTSTR p = set.getString(id);
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);

  uint C[6][3];
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
    &pt->infoFirst[0].idFont,
    &C[0][0], &C[0][1], &C[0][2],
    &C[1][0], &C[1][1], &C[1][2],
    &C[2][0], &C[2][1], &C[2][2],
    &C[3][0], &C[3][1], &C[3][2],
    &pt->heightRows, &pt->infoRows.idFont,
    &C[4][0], &C[4][1], &C[4][2],
    &C[5][0], &C[5][1], &C[5][2]
    );

  pt->infoFirst[0].idFont -= ID_INIT_FONT;
  pt->infoRows.idFont -= ID_INIT_FONT;

  pt->heightRows = R__Y(pt->heightRows);
  pt->infoFirst[0].fg = RGB(C[0][0], C[0][1], C[0][2]);
  pt->infoFirst[0].bg = RGB(C[1][0], C[1][1], C[1][2]);
  pt->infoFirst[1].fg = RGB(C[2][0], C[2][1], C[2][2]);
  pt->infoFirst[1].bg = RGB(C[3][0], C[3][1], C[3][2]);
  pt->infoRows.fg =     RGB(C[4][0], C[4][1], C[4][2]);
  pt->infoRows.bg =     RGB(C[5][0], C[5][1], C[5][2]);
}
//-----------------------------------------------------------
void svmObjTableInfo::loadFirstText(uint id, setOfString& set)
{
  LPCTSTR p = set.getString(id);
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);

  uint style = pt->infoFirst[0].style;
  bool useBmp = toBool(style & USE_BMP);

  if(useBmp) {
    uint id1;
    uint id2;
    _stscanf_s(p, _T("%d,%d,%d,%d"), &pt->styleShow, &id1, &id2, &pt->infoFirst[1].alignText);
    dual* dualBmp = getDualBmp4Btn();
    LPCTSTR name = getOnlyName(dualBmp->getTxt(id1), true);
    pt->First[0] = name ? str_newdup(name) : 0;
    name = getOnlyName(dualBmp->getTxt(id2), true);
    pt->First[1] = name ? str_newdup(name) : 0;
    }
  else {
    pt->styleShow = _ttoi(p);
    LPCTSTR name = set.getString(id + 1);
    pt->First[0] = name ? str_newdup(name) : 0;
    name = set.getString(id + 2);
    pt->First[1] = name ? str_newdup(name) : 0;
    }
}
//-----------------------------------------------------------
void svmObjTableInfo::loadInfoText(uint id, PVect<LPCTSTR>& list, setOfString& set)
{
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  for(uint i = 0; i < NROW_FULL; ++i) {
    LPCTSTR p = set.getString(id + i);
    list[i] = p ? str_newdup(p) : 0;
   }
}
//-----------------------------------------------------------
//id,x,y,num_row,offX,offY,idBaseStyle,styleHead,idHeadpanel,idHeadtext,style1RowOff,style1RowOn,
//      idRowpanel,id1BmpOrText,style2_3_row,id2text,id3text

bool svmObjTableInfo::load(uint id, setOfString& set)
{
  int x = 0;
  int y = 0;

  LPCTSTR p = set.getString(id);
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);

  uint idBaseStyle = 0;
  uint idHeadPanel = 0;
  uint idHeadText = 0;
  uint idRowPanel = 0;
  uint idFirstText = 0;
  uint idSecondText = 0;
  uint idThirdText = 0;

  uint styleHead = 0;
  uint styleOff = 0;
  uint styleOn = 0;
  uint styleRows = 0;

  int nscan = _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
       &x, &y,
       &pt->nRow, &pt->offsX, &pt->offsY,
       &idBaseStyle, &styleHead, &idHeadPanel, &idHeadText,
       &styleOff, &styleOn,
       &idRowPanel, &idFirstText, &styleRows,
       &idSecondText, &idThirdText, &pt->timeCycle
       );
  pt->timeCycle = -(int)pt->timeCycle;

  explodeBits(styleHead, Prop->type1, pt->alignText);
  explodeBits(styleOff, pt->infoFirst[0].style, pt->infoFirst[0].alignText, USE_BMP);
  explodeBits(styleOn, pt->infoFirst[1].style, pt->infoFirst[0].alignText);
  explodeBits(styleRows, pt->infoRows.style, pt->infoRows.alignText);

  loadBasePanel(idBaseStyle, set);
  loadHeadPanel(idHeadPanel, set);
  loadHeadText(idHeadText, set);
  loadRowPanel(idRowPanel, set);
  loadFirstText(idFirstText, set);
  loadInfoText(idSecondText, pt->Second, set);
  loadInfoText(idThirdText, pt->Third, set);

  Prop->Rect = PRect(0, 0, 100, 100);
  Prop->Rect.MoveTo(R__X(x), R__Y(y));
  pt->offsX = R__X(pt->offsX);
  pt->offsY = R__Y(pt->offsY);
//  calcRect();
  loadVar(id, set);

  return true;
}


//----------------------------------------------------------------------------
namespace varTblInfo
{
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PTextFixedPanel* simpleMakeTextPainter::make(const PRect& rect, LPCTSTR txt, const common& obj, bool linked)
{
  svmFont& setFont = linked ? getFontObjLinked() : getFontObj();
  HFONT font = setFont.getFont(obj.idFont);
  uint align = TA_CENTER;
  switch(obj.alignText) {
    case Property::aLeft:
      align = TA_LEFT;
      break;
    case Property::aRight:
      align = TA_RIGHT;
      break;
    }
  PTextFixedPanel* t = new PTextFixedPanel(txt, rect, font, obj.Fg, (PPanel::bStyle)obj.Style, obj.Bg);
  t->setAlign(align);
  t->setVAlign(DT_VCENTER);
  return t;
}
//----------------------------------------------------------------------------
bool simplePainter::hasPanel(const common& obj) const
{
  return (PPanel::FILL & obj.Style) || (PPanel::NO != obj.Style);
}
//----------------------------------------------------------------------------
void simplePainter::paint(HDC hdc, const PRect& rect, LPCTSTR txt, const common& obj, bool linked)
{
  smartPointerConstString spcs = getStringByLangSimple(txt);
  PTextFixedPanel* t = simpleMakeTextPainter().make(rect, spcs, obj, linked);
  t->draw(hdc);
  delete t;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
SIZE header::calcSize(const PropertyTableInfo& pt)
{
  SIZE sz = { 0, pt.HeightHeader };
  for(uint i = 0; i < SIZE_A(pt.Width); ++i)
    sz.cx += pt.Width[i] ? pt.Width[i] + pt.offsX : 0;
  sz.cx -= pt.offsX;
  return sz;
}
//----------------------------------------------------------------------------
void header::paint(HDC hdc, const PRect& rect, const PropertyTableInfo& pti, bool linked)
{
  Fg = pti.foreground;
  Bg = pti.type2;
  Style = pti.type1;
  idFont = pti.idFont;
  alignText = pti.alignText;

  PTextFixedPanel* t = simpleMakeTextPainter().make(rect, _T(""), *this, linked);

  uint offs = 0;
  for(uint i = 0; i < SIZE_A(pti.Width); ++i) {
    if(!pti.Width[i])
      continue;
    PRect r(0, 0, pti.Width[i], pti.HeightHeader);
    r.MoveTo(rect.left + offs, rect.top);
    t->setRect(r);
    smartPointerConstString spcs = getStringByLangSimple(pti.textHeader[i]);
    t->drawText(hdc, spcs);
    offs += pti.Width[i] + pti.offsX;
    }
  delete t;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void firstColumn::load()
{
  useBmp = toBool(pti.infoFirst[0].style & USE_BMP);
  styleShow = pti.styleShow;
  bmpFlag = pti.infoFirst[1].alignText;

  Data[0].Fg = pti.infoFirst[0].fg;
  Data[0].Bg = pti.infoFirst[0].bg;
  Data[0].Style = pti.infoFirst[0].style & ~USE_BMP;
  Data[0].idFont = pti.infoFirst[0].idFont;
  Data[0].alignText = pti.infoFirst[0].alignText;

  Data[1].Fg = pti.infoFirst[1].fg;
  Data[1].Bg = pti.infoFirst[1].bg;
  Data[1].Style = pti.infoFirst[1].style;
  Data[1].idFont = pti.infoFirst[0].idFont;
  Data[1].alignText = pti.infoFirst[0].alignText;
}
//----------------------------------------------------------------------------
void firstColumn::paintBmp(HDC hdc, const PRect& rect, bool active)
{
  LPCTSTR txt = _T("");
  bool useSwapPanel = toBool(pti.styleShow & 8);
  uint ix = useSwapPanel ? active : 0;
  int id = -1;
  switch(pti.styleShow & ~8) {
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
    case 1:
    default:
      if(!active)
        break;
      // fall through
    case 2:
      id = 0;
      break;

    case 3:
    case 4:
    case 5:
      id = active;
      break;
    }
  simplePainter sP;
  if(sP.hasPanel(Data[ix]))
    sP.paint(hdc, rect, _T(""), Data[ix], linked);

  if(!pti.First[id])
    return;
  TCHAR bmpname[_MAX_PATH];
  makeFileImagePath(bmpname, SIZE_A(bmpname), pti.First[id]);

  PBitmap bmp(bmpname);

  SIZE sz = bmp.getSize();
  if(!sz.cx)
    return;
  POINT pt = { (rect.right + rect.left - sz.cx) / 2, (rect.bottom + rect.top - sz.cy) / 2 };
  PTraspBitmap tBmp(0, &bmp, pt);
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
void firstColumn::paintText(HDC hdc, const PRect& rect, bool active)
{
  int ix = -1;
  switch(pti.styleShow) {
/*
  0 -> un solo testo, visibile su attivazione del bit
  1 -> un solo testo, lampeggiante su attivazione del bit
  2 -> un solo testo visibile, lampeggiante su attivazione del bit
  3 -> due testi, scambio
  4 -> due testi nascosti, alternanti su attivazione del bit
  5 -> due testi uno visibile, alternanti su attivazione del bit
*/
    case 0:
    case 1:
    default:
      if(!active)
        break;
      // fall through
    case 2:
      ix = 0;
      break;

    case 3:
    case 4:
    case 5:
      ix = active;
      break;

    }
  if(ix >= 0)
    simplePainter().paint(hdc, rect, pti.First[ix], Data[ix], linked);
  else
    simplePainter().paint(hdc, rect, _T(""), Data[0], linked);
}
//----------------------------------------------------------------------------
void firstColumn::paint(HDC hdc, const PRect& rect, bool active)
{
  if(useBmp)
    paintBmp(hdc, rect, active);
  else
    paintText(hdc, rect, active);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
SIZE manRow::calcSize(const PropertyTableInfo& pt)
{
  SIZE sz = header::calcSize(pt);
  sz.cy += (pt.heightRows + pt.offsY) * pt.nRow +  + pt.offsY * 2;
  sz.cx += pt.offsX * 2;
  return sz;
}
//----------------------------------------------------------------------------
void manRow::paintFirst(HDC hdc, const PRect& rect, bool active)
{
  fCol.paint(hdc, rect, active);
}
//----------------------------------------------------------------------------
static WORD swap16(WORD v)
{
  return (v >> 8) | ((v & 0xff) << 8);
}
//----------------------------------------------------------------------------
void manRow::paint(HDC hdc, const PRect& rect_, WORD bitStat)
{
  Fg = Owner.infoRows.fg;
  Bg = Owner.infoRows.bg;
  Style = Owner.infoRows.style;
  idFont = Owner.infoRows.idFont;
  alignText = Owner.infoRows.alignText;
  PRect rect(rect_);

  PPanel Panel(rect, Owner.background, (PPanel::bStyle)Owner.style);
  Panel.draw(hdc);

  rect.Inflate(-(long)Owner.offsX, -(long)Owner.offsY);
  Head.paint(hdc, rect, Owner, Linked);
  POINT Pt = { rect.left, rect.top };

  int y = Pt.y + Owner.HeightHeader + Owner.offsY;
  uint nElem = Owner.nRow;
  int nInfo = MAX_COLUMN_TABLE_INFO - 1;

  PTextFixedPanel* Text = simpleMakeTextPainter().make(rect, _T(" "), *this, Linked);

  if(Owner.useSwap)
    bitStat = swap16(bitStat);
  PVect<LPCTSTR> Info[] = { Owner.Second, Owner.Third };
  for(uint i = 0; i < nElem; ++i) {
    PRect r(0, 0, Owner.Width[0], Owner.heightRows);
    r.MoveTo(Pt.x, y);
    if(Owner.Width[0])
      paintFirst(hdc, r, toBool(bitStat & (1 << i)));
    else
      r.right -= Owner.offsX;

    for(int j = 0; j < nInfo; ++j) {
      if(Owner.Width[j + 1]) {
        r.MoveTo(r.right + Owner.offsX, r.top);
        r.right = r.left + Owner.Width[j + 1];
        Text->setRect(r);
        smartPointerConstString spcs = getStringByLangSimple(Info[j][i]);

        Text->drawText(hdc, spcs);
        }
      }
    y += Owner.heightRows + Owner.offsY;
    }
  delete Text;
}
//----------------------------------------------------------------------------
}; // end namespace
