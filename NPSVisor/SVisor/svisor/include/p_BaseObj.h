//-------- P_BaseObj.h -------------------------------------------------------
#ifndef P_BASEOBJ_H_
#define P_BASEOBJ_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <math.h>
//----------------------------------------------------------------------------
#ifndef DEFIN_H_
  #include "defin.h"
#endif
#ifndef PDEF_H_
  #include "pdef.h"
#endif
#include "p_util.h"
#include "p_vect.h"
#include "p_date.h"
#include "prfdata.h"
#include "sizer.h"
#include "P_Bits.h"
#include "pInfoMoveObj.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class P_BaseBody;
//----------------------------------------------------------------------------
//#ifndef MAXINT
//  #define MAXINT ((int)((~(DWORD)0) >> 1))
//#endif
#define NOT_USE_INDEX -INT_MAX
//----------------------------------------------------------------------------
template <class T>
inline
REALDATA getNormalized(T val, REALDATA nrm)
{
  return (REALDATA)((REALDATA)val * nrm);
}
//----------------------------------------------------------------------------
template <class T>
inline
void getDenormalizedInt(T& target, REALDATA val, REALDATA nrm)
{
  if(0.0 == nrm || 1.0 == nrm)
    target = (T)val;
  else {
    val /= nrm;
    target = (T)ROUND_REAL(val);
    }
}
//----------------------------------------------------------------------------
template <class T>
inline
void getDenormalizedReal(T& target, REALDATA val, REALDATA nrm)
{
  if(0.0 == nrm || 1.0 == nrm)
    target = (T)val;
  else
    target = (T)(val / nrm);
}
//----------------------------------------------------------------------------
extern void makeBinaryString(DWORD val, LPTSTR buffer, size_t lenBuff);
extern void makeHexString(DWORD val, LPTSTR buffer, size_t lenBuff);

extern void makeHexString(DWORD val, LPTSTR buffer, size_t lenBuff);
extern void makeFixedStringConvertPoint(REALDATA val, LPTSTR buffer, size_t lenBuff, uint ndec);
extern void makeFixedStringUConvertPoint(REALDATA val, LPTSTR buffer, size_t lenBuff, uint ndec);

extern void replaceWithPoint(LPTSTR buff);
extern void replaceWithComma(LPTSTR buff);
//----------------------------------------------------------------------------
enum eBase { ebTen, ebBinary, ebHex };
//----------------------------------------------------------------------------
template <class T>
void shiftAndMakeString(T val, int nbit, int offs, int dec, REALDATA nrm, LPTSTR buffer, size_t lenBuff, uint base)
{
  if(nbit) {
    val >>= offs;
    val &= (T)((1 << nbit) - 1);
    }
  REALDATA rVal = getNormalized(val, nrm);
  switch(base) {
    case ebTen:
      makeFixedStringConvertPoint(rVal, buffer, lenBuff, dec);
      break;
    case ebBinary:
      makeBinaryString(ROUND_REAL(rVal), buffer, lenBuff);
      break;
    case ebHex:
      makeHexString(ROUND_REAL(rVal), buffer, lenBuff);
      break;
    }
}
//----------------------------------------------------------------------------
template <class T>
void shiftAndMakeStringU(T val, int nbit, int offs, int dec, REALDATA nrm, LPTSTR buffer, size_t lenBuff, uint base)
{
  if(nbit) {
    val >>= offs;
    val &= (T)((1 << nbit) - 1);
    }
  REALDATA rVal = getNormalized(val, nrm);
  switch(base) {
    case ebTen:
      makeFixedStringUConvertPoint(rVal, buffer, lenBuff, dec);
      break;
    case ebBinary:
      makeBinaryString(ROUND_REAL(rVal), buffer, lenBuff);
      break;
    case ebHex:
      makeHexString(ROUND_REAL(rVal), buffer, lenBuff);
      break;
    }
}
//----------------------------------------------------------------------------
bool verifyRange(uint idNorm, const prfData& data);
//----------------------------------------------------------------------------
void getLocaleDecimalPoint(LPTSTR buff, size_t dim);
void getLocaleThousandSep(LPTSTR buff, size_t dim);
//----------------------------------------------------------------------------
#define GET_SET_ATTR(t, a) \
  public:  \
    void set##a(t val) { a = val; } \
    t get##a() const { return a; } \
  private: \
    t a
//----------------------------------------------------------------------------
#define GET_SET_ALIAS(tv, v, ta, a) \
  public:  \
    void set##a(ta val) { v = (tv)val; } \
    ta get##a() const { return (ta)v; }
//----------------------------------------------------------------------------
#define PRPH_4_CONST 1000
//----------------------------------------------------------------------------
class pConst
{
  public:
    pConst(class pVariable* owner) : Owner(owner) {}
    void makeConst(LPCTSTR p);

    prfData::tResultData getData(prfData& data);
  private:
    pVariable* Owner;
};
//----------------------------------------------------------------------------
// se Type == string -> nBit = lenString
class pVariable
{
  public:
    pVariable(uint prph, uint addr, uint type, uint nbit = 1, uint offs = 0, __int64 norm = 0,  uint dec = 0, bool neg = false, uint base = 0) :
        Prph(prph), Addr(addr), Type(type), nBit(nbit), Offs(offs), Norm(norm), Dec(dec), Neg(neg),
        BaseNum(base), vConst(0) {}

    pVariable() : Prph(0), Addr(0), Type(0), nBit(0), Offs(0), Norm(0), Dec(0), Neg(false), BaseNum(0), vConst(0)  {}
    ~pVariable() { delete vConst; }
    // carica i dati dalla periferica
    prfData::tResultData getData(P_BaseBody* owner, prfData& data, uint addrOffs = 0);

    // riempie i campi essenziali di data
    void fillAttr(prfData& data, uint addrOffs = 0);

    // scrive in buff la stringa con i dati presenti in data e con le impostazioni correnti
    void makeString(LPTSTR buff, size_t lenBuff, prfData& data);

    double getResult(prfData& data);
    double getNormalizedResult(prfData& data);
    int getIxResult(prfData& data);
    bool getDeNormalized(prfData& target, REALDATA input);
    bool getDeNormalized(P_BaseBody* owner, prfData& target, LPCTSTR source);
    void makeConst(LPCTSTR p);

    GET_SET_ATTR(uint, Prph);
    GET_SET_ATTR(uint, Addr);
    GET_SET_ATTR(uint, nBit);
    GET_SET_ATTR(uint, Type);
    GET_SET_ATTR(uint, Offs);
//    GET_SET_ATTR(__int64, Norm);
    GET_SET_ATTR(uint, Dec);
    GET_SET_ATTR(bool, Neg);
    GET_SET_ATTR(uint, BaseNum);

  public:
    void setNorm(__int64 v) { Norm = v; }
    int getNorm() const { return (int)Norm; }

    // in alcuni oggetti sono usati con nomi e scopi diversi
    void setMaskA(__int64 v) { Norm = v; }
    __int64 getMaskA() const { return Norm; }
//    GET_SET_ALIAS(int, Norm, DWORD, MaskA);

    void setNegA(bool v) { Dec = (uint)v; }
    bool getNegA() const { return toBool(Dec); }
//    GET_SET_ALIAS(uint, Dec, bool, NegA);
  private:
    __int64 Norm;
    pConst* vConst;
};
//----------------------------------------------------------------------------
class pVisibility : public pVariable
{
  private:
    typedef pVariable baseClass;
  public:
    pVisibility(uint prph, uint addr, uint nbit = 1, uint offs = 0, uint neg = 0) :
        baseClass(prph, addr, prfData::tBitData, nbit, offs, 0, neg, false)
        {
        if(!nbit) {
          if(offs > 0xffff)
            setType(prfData::tDWData);
          else
            setType(prfData::tWData);
          }
        }

    bool isVisible(P_BaseBody* owner);
};
//----------------------------------------------------------------------------
class baseColor
{
  public:
    baseColor() {}

    void makeColors(P_BaseBody* owner, uint idInit);

    bool getColors(COLORREF& fg, COLORREF& bg, uint ix);
//    bool getFgColor(COLORREF& color, uint ix);
//    bool getBgColor(COLORREF& color, uint ix);
  private:
    struct colors
    {
      COLORREF fg;
      COLORREF bg;
    };
    PVect<colors> Colors;
};
//----------------------------------------------------------------------------
template <typename T, typename V>
class findIxByVal
{
  public:
    findIxByVal(const PVect<T>& set, V value) : Set(set), Value(value) {}
    int getIx();
    int getPos();
    int getExact();
  private:
    const PVect<T>& Set;
    V Value;
};
//----------------------------------------------------------------------------
template <typename T, typename V>
int findIxByVal<T, V>::getIx()
{
  uint nElem = Set.getElem();
  if(!nElem)
    return -1;
  if(Value < nElem)
    return Value;
  return nElem - 1;
}
//----------------------------------------------------------------------------
template <typename T, typename V>
int findIxByVal<T, V>::getPos()
{
  uint nElem = Set.getElem();
  if(!nElem)
    return -1;
  for(uint i = 0; i < nElem; ++i)
    if(Value <= Set[i].Val)
      return i;
  return nElem - 1;
}
//----------------------------------------------------------------------------
#define dPRECISION 0.0000001
//----------------------------------------------------------------------------
#define IS_EQU(v1, v2) (fabs((v1) - (v2)) < dPRECISION)
//----------------------------------------------------------------------------
template <typename T, typename V>
int findIxByVal<T, V>::getExact()
{
  uint nElem = Set.getElem();
  if(!nElem)
    return -1;
  for(uint i = 0; i < nElem; ++i)
    if(IS_EQU(Value, Set[i].Val))
      return i;
  return -1;
}
//----------------------------------------------------------------------------
class baseVarColor
{
  public:
    baseVarColor() : lastIx(-1) {}

    void makeColors(P_BaseBody* owner, uint idInit);
    void makeColorsOnRow(P_BaseBody* owner, uint idInit, uint step = 4);
    void makeFullColorsOnRow(P_BaseBody* owner, uint idInit, uint step = 7);

    bool getReverseColors(COLORREF& fg, COLORREF& bg, double val);
    bool getColors(COLORREF& fg, COLORREF& bg, double val, P_BaseBody* owner = 0, uint idText = 0, LPTSTR buff = 0, size_t lenBuff = 0);
    bool getExactColors(COLORREF& fg, COLORREF& bg, double val, P_BaseBody* owner = 0, uint idText = 0, LPTSTR buff = 0, size_t lenBuff = 0);
//    bool getFgColor(COLORREF& color, double val);
//    bool getBgColor(COLORREF& color, double val);
    double getValByPos(uint pos) { if(pos >= ValColors.getElem()) return 0; return ValColors[pos].Val; }

    int getLastIx() const { return lastIx; }
    void resetLastIx() { lastIx = -1; }
  private:
    struct val_colors
    {
      COLORREF fg;
      COLORREF bg;
      double Val;
    };
    PVect<val_colors> ValColors;
    int lastIx;
};
//----------------------------------------------------------------------------
class manageObjId
{
  public:
    manageObjId(uint id, uint base_id) : Id(id), baseId(base_id) { }
    manageObjId(uint id) : Id(id), baseId(calcBase(id)) { }

    uint getId() const { return Id; }
    uint getBaseId() const { return baseId; }

    uint calcBaseExtendId(uint ix);
    uint calcAndSetExtendId(uint ix);

    uint getFirstExtendId();
    uint getSecondExtendId();
    uint getThirdExtendId();

    uint calcBase(uint id);
    // se l'id passato non è il firstId, con questa si può correggere
    void adjustId();
  private:
    uint Id;
    uint baseId;
};
//----------------------------------------------------------------------------
class blinkingObj
{
  public:
    enum blnkg { hide, notShow, onShow };
    blinkingObj(DWORD tick) : Tick(tick), Show(onShow), lastTick(0) {}
    bool update();
    void setShow(bool show) { Show = show ? onShow : hide; }
    uint getStatus() const { return Show; }
  private:
    blnkg Show;
    DWORD Tick;
    DWORD lastTick;
    void toggle();
};
//----------------------------------------------------------------------------
template <typename T>
void drawObjOffset(T& obj, HDC hdc, const POINT& offset)
{
  PRect old(obj.getRect());
  PRect r(old);
  r.Offset(offset.x, offset.y);
  obj.setRect(r);
  obj.draw(hdc);
  obj.setRect(old);
}
//----------------------------------------------------------------------------
class pInfoMoveObj;
//----------------------------------------------------------------------------
class baseObj
{
  public:
    baseObj(P_BaseBody* owner, uint id);
    virtual ~baseObj();

    virtual bool allocObj(LPVOID param = 0) = 0;

    virtual bool update(bool force) { return checkVisibility(force); }

    virtual void draw(HDC hdc, const PRect& rect);
    virtual const PRect& get_Rect() const { return Rect; }
    virtual PRect getRectMove() { return Rect; }
    uint getId() const { return Id; }

    bool canHide() { return toBool(Visibility) || toBool(MoveInfo); }
    bool needVisible() { return !Visibility || Visibility->isVisible(getOwner()); }
    bool isVisible() const { return Visible; }
    virtual void setVisibility(bool set) { Visible = set; }

    P_BaseBody* getOwner() { return Owner; }
    void setOwner(P_BaseBody* owner) { Owner = owner; }

    virtual void invalidate(bool alsoBkg = false);
    virtual void invalidate(const PRect& r, bool alsoBkg = false);

    void allocBlink(DWORD tick);

    virtual void addReqVar(PVect<P_Bits*>& allBits);
    virtual void addReqVar2(PVect<P_Bits*>& allBits, const pVariable& pv, int offs_addr = 0);
    virtual void addReqVar2(PVect<P_Bits*>& allBits, int prph, uint addr);

    virtual void MoveTo(const POINT& pt) { Rect.MoveTo(pt.x, pt.y); }
    virtual bool canMove() { return Visible; }
  protected:
    P_BaseBody* Owner;
    pInfoMoveObj* MoveInfo;
    virtual void performDraw(HDC hdc) = 0;
    virtual void performDraw(HDC hdc, const POINT& offset);
    void setRect(const PRect& r);

    void allocVisibility(LPCTSTR p);
    bool checkVisibility(bool force);

    void setId(uint newId) { Id = newId; }

#if 1 // le classi eredi devono specificarlo
    virtual uint getBaseId() const = 0;
    virtual uint getFirstExtendId() { return manageObjId(getId(), getBaseId()).getFirstExtendId(); }
    virtual uint getSecondExtendId() { return manageObjId(getId(), getBaseId()).getSecondExtendId(); }
    virtual uint getThirdExtendId() { return manageObjId(getId(), getBaseId()).getThirdExtendId(); }
#else // le classi eredi non hanno bisogno di specificarlo, ma aumentano i costi
    virtual uint getBaseId() const { return manageObjId(getId()).getBaseId()); }
    virtual uint getFirstExtendId() { return manageObjId(getId()).getFirstExtendId(); }
    virtual uint getSecondExtendId() { return manageObjId(getId()).getSecondExtendId(); }
    virtual uint getThirdExtendId() { return manageObjId(getId()).getThirdExtendId(); }
#endif
  private:
    uint Id;
    PRect Rect;
    pVisibility* Visibility;
    blinkingObj* BlinkObj;
    bool Visible;
};
//----------------------------------------------------------------------------
class baseSimple : public baseObj
{
  private:
    typedef baseObj baseClass;
  public:
    baseSimple(P_BaseBody* owner, uint id) : baseClass(owner, id) {}
};
//----------------------------------------------------------------------------
class baseVar : public baseObj
{
  private:
    typedef baseObj baseClass;
  public:
    baseVar(P_BaseBody* owner, uint id) : baseClass(owner, id), offsAddr(0) {}

    void setOffsAddr(int newOffs) { offsAddr = newOffs; }
    void setDeltaOffsAddr(int delta) { offsAddr += delta; }
    int getOffs() const { return offsAddr; }
    int getIdPrph() const { return BaseVar.getPrph(); }
    int getIdNorm() const { return BaseVar.getNorm(); }
    uint getAddr() const { return BaseVar.getAddr(); }

    uint getType() const { return BaseVar.getType(); }
    virtual void addReqVar(PVect<P_Bits*>& allBits) { baseClass::addReqVar(allBits); addReqVar2(allBits, BaseVar, offsAddr); }
  protected:
    pVariable BaseVar;
    int offsAddr;

    virtual void makeVar(pVariable& target, LPCTSTR p1, LPCTSTR p2, bool useRealForNorm = false);
    bool makeStdVars(bool useRealForNorm = false);
    virtual void makeOtherVar(pVariable& var, uint id);
};
//----------------------------------------------------------------------------
class baseActive;
typedef PVect<baseActive*> pVActive;
//----------------------------------------------------------------------------
struct bestRect
{
  PRect r;
  bool found;
  baseActive *obj;
  bestRect() : found(false) {}
};
//----------------------------------------------------------------------------
struct rectAround
{
  bestRect left;
  bestRect top;
  bestRect right;
  bestRect bottom;
};
//----------------------------------------------------------------------------
class wrapObjX
{
  public:
    wrapObjX(baseActive* owner = 0) : Owner(owner) {}
    bool operator <(const wrapObjX& other) const;
    baseActive* Owner;
};
//----------------------------------------------------------------------------
class wrapObjY
{
  public:
    wrapObjY(baseActive* owner = 0) : Owner(owner) {}
    bool operator <(const wrapObjY& other) const;
    baseActive* Owner;
};
//----------------------------------------------------------------------------
class pAround
{
  public:
    pAround();

//    void findAround(baseActive* sender, pVActive& edi, pVActive& btn);

    void findAround(baseActive* sender, const PVect<wrapObjX>& wrapX, const PVect<wrapObjY>& wrapY);
    void findAround(baseActive* sender, PVect<pVActive>& objSet);
//    void findAround(baseActive* sender, pVActive& obj);

    enum around { aLeft, aTop, aRight, aBottom, maxAround };

    void setFocus(around where);
    void setJoin(around where, baseActive* sender, baseActive* join);
    bool hasAround(around where) const { return toBool(Obj[where]); }

    HWND getHwnd(around where);

  private:
    baseActive* Obj[maxAround];

    void findLeft(struct rectAround& rA, const PRect& rectSender, baseActive* testing);
    void findRight(struct rectAround& rA, const PRect& rectSender, baseActive* testing);
    void findTop(struct rectAround& rA, const PRect& rectSender, baseActive* testing);
    void findBottom(struct rectAround& rA, const PRect& rectSender, baseActive* testing);

};
//----------------------------------------------------------------------------
class baseActive : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    baseActive(P_BaseBody* owner, uint id) : baseClass(owner, id) {}

//    void findAround(pVActive& obj) { Around.findAround(this, obj); }
    void findAround(const PVect<wrapObjX>& wrapX, const PVect<wrapObjY>& wrapY) { Around.findAround(this, wrapX, wrapY); }

    virtual void setVisibility(bool set)
    {
      if(getHwnd())
        ShowWindow(getHwnd(), set ? SW_NORMAL : SW_HIDE);
      baseClass::setVisibility(set);
    }
    virtual bool update(bool force);

    virtual void draw(HDC hdc, const PRect& rect);

    virtual void invalidate(bool alsoBkg = false);
    virtual void invalidateWithBorder();
  protected:
    virtual void performDraw(HDC hdc) {}
    virtual void performDraw(HDC hdc, const POINT& offset);
    virtual HWND getHwnd() = 0;
    friend class pAround;
    pAround Around;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#include "p_basebody.h"
//----------------------------------------------------------------------------
#endif

