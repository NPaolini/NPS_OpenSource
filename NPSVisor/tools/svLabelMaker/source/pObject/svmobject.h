//-------------------- svmObject.h ----------------------------
//-----------------------------------------------------------
#ifndef SVMOBJECT_H_
#define SVMOBJECT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmProperty.h"
#include "svmHistory.h"
#include "p_txt.h"
#include "common.h"
#include "smartPS.h"
#include "svmDefObj.h"
#include "pPrintPage.h"
//-----------------------------------------------------------
class svmObjHistory;
//-----------------------------------------------------------
bool isMetafile(LPCTSTR filename);
//----------------------------------------------------------------------------
lUDimF writeStringChkUnicode(P_File& pf, LPCTSTR buff);
//-----------------------------------------------------------
class svmObjCount
{
  public:
    svmObjCount();

    DWORD getGlobalIdCount(DWORD add = 1);

    void reset();

  private:
    DWORD counts[4];
};
//-----------------------------------------------------------
svmObjCount& getObjCount();
//-----------------------------------------------------------
LPCTSTR getTypeString(uint ix);
//-----------------------------------------------------------
void normalizeYNeg(PRect& r);
bool intersecYNeg(const PRect& r1, const PRect& r2);
bool isInYNeg(const PRect& r, const POINT& pt);
//-----------------------------------------------------------
class svmObject : public genericSet
{
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

    virtual void DrawObjectLocked(HDC hdc, const PRect& rect);
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

    void setSelectSimple(bool set) { Selected = set ? yesSel : notSel; }

    void standBySel(bool add);

    void chgId(int newId) { Id = newId; }
    int getId() const;
    virtual int getIdType() const;

//    int get_zOrder() const { return zOrder; }
//    void set_zOrder(int z) { zOrder = z; }

    virtual const PRect& getRect() const;
    virtual const PRect& getLinkedRect() const;

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
    virtual void copyPropFrom(const Property* other) { *Prop = *other; }

    virtual void unionBits(DWORD& capable) { Prop->unionBits(capable); }
    virtual void initUnion(unionProperty& uProp) { Prop->initUnion(uProp); }
    virtual void setCommonProperty(const unionProperty& prop);

    const svmObject& operator =(const svmObject& other) { Clone(other); return *this; }

    PWin* getParent() const { return Par; }

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);

    virtual bool save(P_File& pf) { return true; }

    virtual bool load(LPCTSTR row) { return true; }
    virtual smartPointerConstString getTitle() const = 0;

    virtual smartPointerConstString getTitle4Prop() const { return getTitle(); }
    virtual LPCTSTR getObjName() const { return get_objName(IdType); }

    virtual void fillTips(LPTSTR tips, int size);

    // alloca una copia dell'oggetto corrente
    virtual svmObject* makeClone() = 0;
    // copia i dati modificabili da other
    virtual void Clone(const svmObject& other) = 0;

    virtual bool isLocked() const { return locked; }
    virtual void setLock(bool set) { locked = set; }

    virtual bool isVisible() const { return Visible; }
    virtual void setVisible(bool set) { Visible = set; }

    void setLinked() { Linked = true; setLock(true); }
    bool isLinked() const { return Linked; }
    virtual Property* allocProperty() = 0;

    bool isOutScreen();
    void re_InsideRect();

    virtual bool intersec(const PRect& frame) { return intersecYNeg(frame, getRect()); }
    Property* getLast();
    virtual void replaceLast(svmObject* obj);

    virtual bool setOffsetVar(int offs) { return false; }

    virtual bool useFont() const { return false; }
    virtual void resolveFont(const typeFont* tf);
    void saveInfoFont();
    const typeFont* getSaved() const { return Prop->forPaste; }

  protected:
    HDC getDC() const;
    void releaseDC(HDC hdc) const;
    PRect getMinBox() const;

    PWin *Par;
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

    int Selected;

    bool Linked;
    // memorizza la parte sensibile su cui si è effettuato il click
    typeOfAnchor whichPoint;

    virtual void drawHandleUI(HDC hdc);
    virtual void drawRectUI(HDC hdc, const PRect& rtAll);
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
    virtual void reInsideRect(PRect& test, const PRect& area);

    virtual bool performDialogProperty() { return false; }
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
inline const PRect& svmObject::getLinkedRect() const { return Prop->Rect; }
//-----------------------------------------------------------
inline svmObject::typeOfAnchor svmObject::getAnchor() const { return whichPoint; }
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
#endif
