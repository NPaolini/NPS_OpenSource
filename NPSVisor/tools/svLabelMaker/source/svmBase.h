//------------------ svmBase.h ------------------------------
//-----------------------------------------------------------
#ifndef SVMBASE_H_
#define SVMBASE_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "p_list.h"
#include "svmObject.h"
//-----------------------------------------------------------
#include "headerMsg.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmListObj : public P_List_Gen
{
  private:
    typedef P_List_Gen baseClass;
  public:
    svmListObj();
    ~svmListObj() { Flush(); }


    virtual bool Add(ListData);
    virtual bool atEnd(ListData);

    virtual bool insert(ListData, bool before);

    virtual ListData Rem();

    int getElem() const { return nElem; }
    virtual void Flush() { baseClass::Flush(); nElem = 0; }
  protected:
    virtual void freeData(ListData d);
    virtual int cmp(ListData a, ListData b);
    int nElem;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class PListHistory : public P_List_Gen
{
  public:
    PListHistory();
    ~PListHistory() { Flush(); }
  protected:
    virtual void freeData(ListData d);
    virtual int cmp(ListData a, ListData b);
};
//-----------------------------------------------------------
class svmBase
{
  public:
    svmBase(PWin* owner);
    virtual ~svmBase();

    void addObject(svmObject* obj, HDC hdc = 0);
    void addObjectLinked(svmObject* obj);

    enum action { none, select, moving, sizing, panning };
    int findPoint(HDC hdc, const POINT&pt, int keyFlag);
    void findObjects(const PRect& frame, int keyFlag);
    void showSelected(HDC hdc);
    void unselectAll(HDC hdc);

    svmObject::typeOfAnchor getAnchor(const POINT& pt, LPTSTR tips, int size, bool& changed, bool& onSelected);

    svmObject::typeOfAnchor beginXor(const POINT& pt);
    void drawXorBlock(HDC hdc, const POINT& pt);
    void drawXorBlockFinal(HDC hdc, const POINT& pt);
    void drawXorObject(HDC hdc, const POINT& pt);
    void drawXorObjectFinal(HDC hdc, const POINT& pt);

    void beginXorSizeByKey();
    void drawResizeAllObject(HDC hdc, const POINT& pt);
    void drawResizeAllObjectFinal(HDC hdc);

    void deleteSelected();

    void Undo();
    bool canUndo();

    enum align { aLeft, aTop, aRight, aBottom };
    void Align(align how);
    bool canAlign();

    bool canAlignSpace();
    void AlignSpace(align how);

    enum order { oTop, oPrevious, oNext, oBottom, oMax };
    void Order(order how);
    void orderByFirst(order how);
    void orderByLast(order how);
    bool canOrder();
    void enableWichOrder(bool enable[]);

    bool canProperty();

    void paint(HDC hdc, const PRect& rect, bool all);

    void setProperty(bool editSingle);
    bool savePage(P_File& pfTemplate, P_File& pfVarDef);

    void flushHistory() { History.Flush(); }
    void flushObject()  { List.Flush(); flushLinked(); }
    void flushLinked()  { ListLinked.Flush(); LinkedStat = 0; }

    void setOffset(const POINT& pt);
    void setActionToBkg(bool set);
    bool setActionLock(bool set);
    bool setActionHide(bool set);

    void flushSel();

    void flushSelected4Paste();
    void pasteCopied(HDC hdc);
    void copySelected();

    bool canPaste();
    bool setLockObj(bool hide);

    int hasLinked() const { return LinkedStat; }

    enum eBtnStat { nonePressed, eBS_Lock, eBS_Hide, eBS_LockAndHide, eBS_hasProject = 4 };
    DWORD getBtnStat() const { return oldBtnStat; }
    void setBtnStat(DWORD newStat)  { oldBtnStat = newStat; }
    void addBtnStat(DWORD newStat)  { oldBtnStat |= newStat; }
    void remBtnStat(DWORD newStat)  { oldBtnStat &= ~newStat; }

    void findBtnStat();

    void invalidate() { freeBkg(); InvalidateRect(*Owner, 0, 0); }

    void ReInside();
    bool canReInside();

    bool setActionOffset();

    bool save(P_File& pf, LPCTSTR fileVars, uint* ids);

    bool isUsedFontId(uint idfont);
    void getUsedFontId(PVect<uint>& set);
    bool decreaseFontIfAbove(uint id_font);
    bool resetFontIfAbove(uint id_font);
    bool isFontAbove(uint id_font);

  protected:
    svmListObj List;
    svmListObj ListLinked;
    int LinkedStat; // 0 -> nessun oggetto, 1 -> oggetti visibili, -1 -> oggetti nascosti
    PListHistory History;

    PWin* Owner;

    int findPointAdd(HDC hdc, const POINT&pt);

    DWORD oldBtnStat;

  private:
    int getAlignVal(align how, svmObject* obj);
    void unionAlign(align how, svmObject* obj, int& pt);
    void setAlign(align how, svmObject* obj, int pt);
    svmObjHistory::typeOfAction commonProperty(unionProperty& prop);

    static PVect<svmObject*> selected4Paste;

    void makeBkg();
    void freeBkg();

    HBITMAP hBmpWork;
    HDC mdcWork;
    HGDIOBJ oldObj;
};
//-----------------------------------------------------------
inline bool svmBase::canUndo()
{
  return History.getFirst() != 0;
}
//-----------------------------------------------------------
inline void svmBase::flushSel()
{
//  selectedBkg.reset();
//  selectedOther.reset();
}
//-----------------------------------------------------------
#endif
