//------ PVarLed.cpp ---------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarLed.h"
#include "p_basebody.h"
#include "p_util.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
pLed::pLed(PWin* owner, PBitmap **bmp, UINT nBmp, const POINT& pt) :
     PSeqTraspBitmap(owner, bmp, nBmp, pt)
{
//  setCanSaveBkg(false);
}
//----------------------------------------------------------------------------
void pLed::setNum(int n)
{
  if(uint(n) >= MAX_NUM_LED) {
    setEnable(false);
    }
  else if(getCurr() != uint(n) || !isEnabled()) {
    setEnable(true);
    setCurr(n, false);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PVarLed::PVarLed(P_BaseBody* owner, uint id) : baseVar(owner, id),
  nDigit(4), nDec(0), Bkg(0), aBmp(0), Led(0), pointBmp(0)
 {}
//----------------------------------------------------------------------------
PVarLed::~PVarLed()
{
  for(int i = 0; i < nDigit; ++i)
    delete Led[i];
  delete []Led;

  delete pointBmp;
  delete []aBmp;
  delete Bkg;
}
//----------------------------------------------------------------------------
bool PVarLed::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  //x,y,w,h,numero_cifre,numero_decimali,style,rBkg,gBkg,bBkg

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int style = 0;
  int R = 0;
  int G = 0;
  int B = 0;
  int zero = 0;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h, &nDigit, &nDec, &style, &R, &G, &B, &zero);

  showZero = toBool(zero);
  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  p = getOwner()->getPageString(getId() + ADD_INIT_VAR);
  if(!p)
    return false;

  PVect<uint> pIdBmp;
  insertIdBmp(p, pIdBmp, -1);

  Bkg = new PPanel(rect, RGB(R,G,B), (PPanel::bStyle)style);

  int nElem = pIdBmp.getElem();

  aBmp = new PBitmap*[nElem];

  const PVect<PBitmap*>& source = getOwner()->getBmp4Var();

  for(int i = 0; i < nElem; ++i) {
    uint ix = pIdBmp[i];
    aBmp[i] = source[ix];
    }
  SIZE szPoint = aBmp[MAX_NUM_LED]->getSize();
  SIZE szDigit = aBmp[1]->getSize();

  int width = szDigit.cx;

  width *= nDigit;
  if(nDec)
    width += szPoint.cx;

  if(!width)
    width = 1;
  PRect allRect(rect);

/**/
  allRect.Inflate(-2, -2);
  double scaleX = allRect.Width();
  scaleX /= width;
  double scaleY = allRect.Height();
  scaleY /= szDigit.cy;

  Led = new pLed*[nDigit];
  int integer = nDigit - nDec;
  POINT pt = { allRect.left, allRect.top };

  width = (int)(szDigit.cx * scaleX);

  for(int i = 0; i < integer; ++i) {
    Led[i] = new pLed(getOwner(), aBmp, nElem, pt);
    Led[i]->setScale(scaleX, scaleY);
    pt.x += width;
    }
  pointBmp = new PTraspBitmap(getOwner(), aBmp[MAX_NUM_LED], pt);
  pointBmp->setScale(scaleX, scaleY);
//  pointBmp->setCanSaveBkg(false);

  if(nDec)
    pt.x += (LONG)(szPoint.cx * scaleX);

  for(int i = integer; i < nDigit; ++i) {
    Led[i] = new pLed(getOwner(), aBmp, nElem, pt);
    Led[i]->setScale(scaleX, scaleY);
    pt.x += width;
    }

  // i dati per la variabile e visibilità sono sfasati di ADD_INIT_VAR
#if 0
  uint id = getId();
  setId(id + ADD_INIT_VAR);
  bool success = makeStdVars();
  setId(id);
#else
  // anche se difficilmente si arriverà a saturare con oggetti led, visto che
  // la gestione è stata unificata, non è più possibile usare il trick sopra
  // che spostava l'offset dell'id, ma occorre riscrivere il codice per intero
  manageObjId moi(getId(), getBaseId());
  uint idAdd = moi.getSecondExtendId();
  LPCTSTR p1 = getOwner()->getPageString(idAdd);
  idAdd = moi.getThirdExtendId();
  LPCTSTR p2 = getOwner()->getPageString(idAdd);
  allocVisibility(p2);
  makeVar(BaseVar, p1, p2);
#endif
  return true;
}

//----------------------------------------------------------------------------
bool PVarLed::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible())
    return upd;

  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }

  force |= upd;

  if(!force && prfData::notModified == result)
    return upd;


  REALDATA val = BaseVar.getNormalizedResult(data);
  Set(val);
  invalidate();
  return true;
}
//----------------------------------------------------------------------------
void PVarLed::performDraw(HDC hdc)
{
  Bkg->paint(hdc);
  for(int i = 0; i < nDigit; ++i)
    Led[i]->Draw(hdc);
  if(nDec)
    pointBmp->Draw(hdc);
}
//----------------------------------------------------------------------------
static void moveObjOffset(PTraspBitmapBase& obj, HDC hdc, const POINT& offset)
{
  POINT pt = obj.getPoint();
  POINT old = pt;
  pt.x += offset.x;
  pt.y += offset.y;
  obj.moveToSimple(pt);
  obj.Draw(hdc);
  obj.moveToSimple(old);
}
//----------------------------------------------------------------------------
void PVarLed::performDraw(HDC hdc, const POINT& offset)
{
  PRect old(get_Rect());
  PRect r(old);
  r.Offset(offset.x, offset.y);
  Bkg->setRect(r);
  Bkg->paint(hdc);
  Bkg->setRect(old);

  for(int i = 0; i < nDigit; ++i)
    moveObjOffset(*Led[i], hdc, offset);
  if(nDec)
    moveObjOffset(*pointBmp, hdc, offset);
}
//----------------------------------------------------------------------------
static uint pw(uint digit)
{
  uint ret = 1;
  while(digit--)
    ret *= 10;
  return ret;
}
//----------------------------------------------------------------------------
void PVarLed::Set(long val)
{
  bool sign = false;
  if(val < 0) {
    val = -val;
    sign = true;
    }
  val %= pw(nDigit - sign);

  if(!val && !showZero) {
    showEmpty();
    return;
    }

  int num = val;
  int t = nDigit;
  while(num != 0 && t > 0) {
    --t;
    num /= 10;
    }
  int i;
  for(i = nDigit - 1; i >= t; --i) {
    Led[i]->setNum(val % 10);
    val /= 10;
    }
  if(nDec) {
    bool lessOne = nDigit - i <= nDec + 1;
    if(lessOne) {
      while(nDigit - i <= nDec + 1)
        Led[i--]->setNum(0);
      }
    }
  if(sign) {
    Led[i]->setNum(MAX_NUM_LED - 1);
    --i;
    }
  for(; i >= 0; --i)
    Led[i]->setNum(MAX_NUM_LED);
}
//----------------------------------------------------------------------------
void PVarLed::Set(REALDATA val)
{
  for(int i = 0; i < nDec; ++i)
    val *= 10;
  long num = ROUND_REAL(val);
  Set(num);
}
//----------------------------------------------------------------------------
void PVarLed::showEmpty()
{
  for(int i = 0; i < nDigit; ++i)
    Led[i]->setNum(MAX_NUM_LED - 1);
}
//----------------------------------------------------------------------------
