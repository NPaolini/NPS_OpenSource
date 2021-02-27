//-------------------- svmObject.h ----------------------------
//-----------------------------------------------------------
#ifndef SVMOBJECT_H_
#define SVMOBJECT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "groupObject.h"
#include "svmMainClient.h"
#include "svmProperty.h"
#include "svmHistory.h"
#include "svmDefObj.h"
#include "language_util.h"
//-----------------------------------------------------------
#define DATA_NOT_USED -1
//-----------------------------------------------------------
#define ONLY_CONFIRM 1000
#define ONLY_CONFIRM_ONE_CHAR 9
//-----------------------------------------------------------
class svmObjHistory;
class P_File;
//void resetGlobalId();
//DWORD getGlobalIdCount(DWORD add = 1);
//----------------------------------------------------------------------------
#define PRPH_4_CONST 1000
#define PRPH_4_CONST_CB_SEL 21
//-----------------------------------------------------------
extern bool getConstValue(LPCTSTR p, DWORD& value, uint type);
//-----------------------------------------------------------
#define FIRST_ID_MSG 800001
#define FIRST_ID_FUNCTION_KEY 100001
#define FIRST_ID_CHG_PAGE     110001
//-----------------------------------------------------------
bool isMetafile(LPCTSTR filename);
//----------------------------------------------------------------------------
class manageObjId
{
  public:
    manageObjId(uint id, uint base_id) : Id(id), baseId(base_id) {}
    manageObjId(uint id) : Id(id), baseId(calcBase(id)) {}

    uint getId() const { return Id; }
    uint getBaseId() const { return baseId; }

    uint calcBaseExtendId(uint ix);
    uint calcAndSetExtendId(uint ix);

    uint getFirstExtendId();
    uint getSecondExtendId();
    uint getThirdExtendId();

    uint calcBase(uint id);
  private:
    uint Id;
    uint baseId;
};
//-----------------------------------------------------------
class svmObjCount
{
  public:
    svmObjCount();

    DWORD getGlobalIdCount(DWORD add = 1);
    DWORD getFunctionKeyIdCount(DWORD add = 1);
    DWORD getIdCount(objType which, DWORD add = 1);
    DWORD getChgPageIdCount(DWORD add = 1);

    void reset();

  private:
    DWORD counts[oMAX_OBJ_TYPE + 3];
};
//-----------------------------------------------------------
svmObjCount& getObjCount();
//-----------------------------------------------------------
class dual
{
  public:
    dual(int initCount) : currCount(initCount) {}
    virtual ~dual() {}

    virtual int addAndGetId(LPCTSTR txt)= 0;

    virtual int findId(LPCTSTR txt) const = 0;
    virtual LPCTSTR getTxt(int id) const = 0;
    virtual void reset() = 0;
    virtual bool save(P_File& pf) = 0;
    virtual bool load(setOfString& set) = 0;
  protected:
    int currCount;
};
//-----------------------------------------------------------
dual* getDualBmp4Btn();
//-----------------------------------------------------------
dual* getDualBmp4BtnLinked();
//-----------------------------------------------------------
LPCTSTR getTypeString(uint ix);
//-----------------------------------------------------------
struct infoAd4
{
  TCHAR objName[64];
  uint id;
  TCHAR infoVar[64];
  uint prph;
  uint addr;
  uint type;
  uint nBit;
  uint offs;

  infoAd4() : id(0), prph(0), addr(0), type(0), nBit(0), offs(0) { objName[0] = 0; infoVar[0] = 0; }
};
//-----------------------------------------------------------
class manageInfoAd4
{
  public:
    manageInfoAd4() {}
    ~manageInfoAd4();
    void add(infoAd4* iad4);
    const PVect<infoAd4*>& getSet() { return Set; }
  private:
    PVect<infoAd4*> Set;
};
//----------------------------------------------------------------------------
struct infoOffsetReplace
{
  int offs;
  DWORD flag;
  int prphFrom;
  int prphTo;
  int offsTxt;
  int prphFilterOnAddr;
  int bitOffs;
  int prphFrom2;
  int prphTo2;
  int addrFrom;
  int addrTo;
};
//-----------------------------------------------------------
inline bool isSetBitf(uint offs, DWORD flag) { return toBool((1 << offs) & flag); }
//-----------------------------------------------------------
inline void setBitf(DWORD& flag, uint offs) { flag |= (1 << offs); }
//-----------------------------------------------------------
namespace stylePanel {
enum stylePanel {
    trasp,
    up_fill,
    dn_fill,
    fill,
    up_trasp,
    dn_trasp,
    border_fill,
    border_trasp
    };
}
//-----------------------------------------------------------
enum eMirrorType { emtNone, emtHorz, emtVert, emtBoth };
//-----------------------------------------------------------
class svmObject : public svmBaseObject
{
  private:
    typedef svmBaseObject baseClass;
  public:
    svmObject(PWin* par, int id, int idType, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObject(PWin* par, int id, int idType, const PRect& r);

    svmObject(PWin* par, setOfString& sos, int id, int idType, int idCode);

    virtual ~svmObject() { delete Prop; delete oldProp; }


    enum typeOfAnchor {
        Extern = 0,     // esterno alla figura
        LeftTop = 1,  // uno dei punti intorno
        Top,
        RightTop,
        Right,
        RightBottom,
        Bottom,
        LeftBottom,
        Left,
        Inside        // interno alla figura
        };
    virtual bool canDistorce() const { return true; }

    // per evitare accavallamenti strani la Draw() si spezza in due in modo
    // che si possano usare due cicli. Nel primo si disegna l'oggetto, nel
    // secondo ciclo si disegna la selezione
    virtual void DrawOnlyObject(HDC hdc, const PRect& rect);
    virtual void DrawOnlySelect(HDC hdc, const PRect& rect);

    virtual void DrawObjectLinked(HDC hdc, const PRect& rect);

    virtual void Select(HDC hdc);
    virtual void Unselect(HDC hdc);

    typeOfAnchor beginDrag(const POINT& pt);
    void Drag(HDC hdc, const POINT& pt);
    virtual bool endDrag(HDC hdc, const POINT& pt);

    void beginDragSizeByKey();
    void DragOnResizeByKey(HDC hdc, const POINT& pt);
    bool endDragOnResizeByKey(HDC hdc);

    virtual typeOfAnchor pointIn(const POINT& pt) const;
    virtual typeOfAnchor getAnchor() const;
    void setAnchor(typeOfAnchor anchor, const POINT& pt);

    enum tSelect { notSel, yesSel, signalRemove, signalAdd };
    tSelect isSelected() const;

    void setSelectSimple(bool set) {   baseClass::setSelectSimple(set); Selected = set ? yesSel : notSel; }

    void standBySel(bool add);

    void chgId(int newId) { Id = newId; }
    int getId() const;
    virtual int getIdType() const;

    int get_zOrder() const { return zOrder; }
    void set_zOrder(int z) { zOrder = z; }

    virtual const PRect& getRect() const;

    virtual void setRect(const PRect& rect);

    uint getFontId() const;
    void setFont(uint id_font);
    bool decreaseFontIfAbove(uint id_font);
    bool resetFontIfAbove(uint id_font);
    bool isFontAbove(uint id_font);

    virtual svmObjHistory* makeHistoryObject(svmObjHistory::typeOfAction, svmObject* prev = 0, bool useOld = false);
    virtual void restoreByHistoryObject(const svmObjHistory& history);

    virtual const Property* getProp() const { return Prop; }
    virtual Property* getProp() { return Prop; }
    // se selezionati più oggetti si possono modificare solo proprietà comuni,
    // i bit di flagModified indicano le modifiche compatibili tra gli oggetti
    virtual void setCommonProperty(const Property* prop, DWORD flagModified, LPCTSTR simpleText);

    const svmObject& operator =(const svmObject& other) { Clone(other); return *this; }

    PWin* getParent() const { return Par; }
    void invalidate();

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true)
    { return svmObjHistory::None; }

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order) {
      return baseClass::save(pfCript, pfClear, order);
      }

    virtual bool load(uint id, setOfString& set) { return true; }
    virtual smartPointerConstString getTitle() const = 0;

    virtual smartPointerConstString getTitle4Prop() const { return getTitle(); }
    virtual LPCTSTR getObjName() const { return get_objName(IdType); }
    virtual uint getStyle() const;
    bool saveVar(P_File& pf, uint baseId, uint addCod = 0);
    void loadVar(uint id, setOfString& set, uint* addCod = 0);

    void loadOnlyVisVar(uint id, setOfString& set, uint* addCod = 0);
    void loadOnlyVisVar(setOfString& set, uint trueId, uint* addCod = 0);

    bool saveOnlyVisVar(P_File& pf, uint baseId, uint addCod = 0);
    bool saveOnlyVisVar(uint trueId, P_File& pf, uint addCod = 0);

    virtual void fillTips(LPTSTR tips, int size);

    // alloca una copia dell'oggetto corrente
    virtual svmObject* makeClone() = 0;
    // copia i dati modificabili da other
    virtual void Clone(const svmObject& other) = 0;

    virtual bool isLocked() const { return locked; }
    virtual void setLock(bool set);

    virtual bool isVisible() const { return Visible; }
    virtual void setVisible(bool set);

    void setLinked() { Linked = true; setLock(true); }
    virtual Property* allocProperty() = 0;

    virtual void addInfoAd4(manageInfoAd4& set) { addInfoAd4Base(set); addInfoAd4Other(set); addInfoAd4Vis(set); addInfoAd4Move(set); }
    bool isOutScreen();
    void re_InsideRect();

    enum eWhichOffset { eoOnlyBase, eoAlsoVisibility, eoAlsoOther, eoExcludePrph1, eoUseAddr, eoUsePrph, eoAlsoPrphVisibility,
                        eoUseSimpleText, eoOnlyOnePrphOnUseAddr, eoProgrBitOffs, eoProgrAddr, eoSubstVar };
    virtual void setOffsetAddr(const infoOffsetReplace& ior);
    bool isValid4OffsetAddr(uint prph, uint addr, bool noPrph1);
    bool isValidPrph4OffsetAddr(uint prph, const infoOffsetReplace& ior);
    virtual bool useFont() const { return true; }
    void resolveFont(const typeFont* tf);
    void saveInfoFont();
    const typeFont* getSaved() const { return Prop->forPaste; }

    virtual void setMirror(uint vert_horz, SIZE sz);
  protected:

    PWin *Par;
//    svmObject* Next;
    COLORREF XoredColor;

    // viene inizializzata a zero, le classi derivate devono preoccuparsi di
    // allocare la proprietà giusta
    Property* Prop;

    // usata per memorizzare le informazioni da immettere in history
    Property* oldProp;

    // rettangolo per le coordinate durante la fase di trascinamento o
    // di ridimensionamento (se trascinamento, il punto di aggancio è
    // memorizzato nella parte left-top del rettangolo)
    PRect RectDragging;
    // memorizza il primo punto del beginDrag()
    POINT firstPoint;

    // identificatore, deve essere univoco
    int Id;

    // tipo di oggetto
    int IdType;

    int zOrder;

    int Selected;

    // memorizza la parte sensibile su cui si è effettuato il click
    typeOfAnchor whichPoint;

    bool Linked;

    virtual void addInfoAd4Move(manageInfoAd4& set);
    virtual void addInfoAd4Vis(manageInfoAd4& set);
    virtual void addInfoAd4Base(manageInfoAd4& set);
    virtual void addInfoAd4Other(manageInfoAd4& set) {}
    virtual LPCTSTR getTextInfoVis() { return _T("Visibilità"); }
    virtual LPCTSTR getTextInfoBase() { return _T("Base"); }

    void drawSel(HDC hdc);

    virtual void drawHandleUI(HDC hdc);
    virtual void drawBoxes(HDC hdc, const PRect& all);
    virtual COLORREF getXoredColor() const;

    virtual void DrawTitle(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    // viene richiamata da drawTitle()
    virtual void DrawRectTitle(HDC hdc, LPCTSTR title, PRect rect);

    virtual void DrawObject(HDC hdc) = 0;

    const PRect& getFirstRect() const { return Rect; }

    static int howVerifyOutScreen;
    void verifyOutScreen();

    virtual void addTipsVis(LPTSTR tips, int size);


#if 0 // le classi eredi devono specificarlo
    virtual uint getBaseId() const = 0;
    virtual uint getFirstExtendId() { return manageObjId(getId(), getBaseId()).getFirstExtendId(); }
    virtual uint getSecondExtendId() { return manageObjId(getId(), getBaseId()).getSecondExtendId(); }
    virtual uint getThirdExtendId() { return manageObjId(getId(), getBaseId()).getThirdExtendId(); }
#else // le classi eredi non hanno bisogno di specificarlo, ma aumentano i costi
    virtual uint getBaseId() const { return manageObjId(getId()).getBaseId(); }
    virtual uint getFirstExtendId() { return manageObjId(getId()).getFirstExtendId(); }
    virtual uint getSecondExtendId() { return manageObjId(getId()).getSecondExtendId(); }
    virtual uint getThirdExtendId() { return manageObjId(getId()).getThirdExtendId(); }
#endif
    virtual uint getTrueId(uint ix, uint base) const { return manageObjId(0, base).calcBaseExtendId(ix); }

  private:
    // rettangolo per le dimensioni
    PRect Rect;

    void getDelta(int& dx, int& dy, const POINT& pt);
    void calcNewPoint(const POINT& pt);
    void calcNewPointOnResize(const POINT& pt);

    bool locked;
    bool Visible;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline svmObject::tSelect svmObject::isSelected() const
{
  return (svmObject::tSelect)Selected;
}
//-----------------------------------------------------------
inline int svmObject::getId() const { return Id; }
//-----------------------------------------------------------
inline int svmObject::getIdType() const { return IdType; }
//-----------------------------------------------------------
inline const PRect& svmObject::getRect() const { return Prop->Rect; }
//-----------------------------------------------------------
inline svmObject::typeOfAnchor svmObject::getAnchor() const { return whichPoint; }
//-----------------------------------------------------------
inline uint svmObject::getFontId() const { return Prop->idFont; }
inline void svmObject::setFont(uint id_font) { Prop->idFont = id_font; }
//inline void svmObject::setFont(HFONT font) { Font = font; }
inline bool svmObject::decreaseFontIfAbove(uint id_font)
{
  if(Prop->idFont > id_font) {
    --Prop->idFont;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
inline bool svmObject::isFontAbove(uint id_font) { return Prop->idFont > id_font; }
//-----------------------------------------------------------
inline bool svmObject::resetFontIfAbove(uint id_font)
{
  if(Prop->idFont > id_font) {
    Prop->idFont = 0;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
inline COLORREF svmObject::getXoredColor() const
{
  extern bool isColoredXor();
  return isColoredXor() ? XoredColor : RGB(0xcf, 0xcf, 0xcf);
}
//-----------------------------------------------------------
inline PRect svmObject::getRectTitle(HDC, LPCTSTR) const { return Prop->Rect; }
//-----------------------------------------------------------
inline void svmObject::setRect(const PRect& rect)
{
//  Rect = rect;
  if(Prop)
    Prop->Rect = rect;
}
//-----------------------------------------------------------
#endif
