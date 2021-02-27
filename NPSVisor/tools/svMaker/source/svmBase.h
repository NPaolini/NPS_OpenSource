//------------------ svmBase.h ------------------------------
//-----------------------------------------------------------
#ifndef SVMBASE_H_
#define SVMBASE_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "p_list.h"
#include "svmObject.h"
#include "svmDefObj.h"
//-----------------------------------------------------------
#include "headerMsg.h"
//-----------------------------------------------------------
class svmMainClient;
//-----------------------------------------------------------
//-----------------------------------------------------------
class allowSelect
{
  public:
    enum allow { asNone, asOnlyBkg, asOtherThenBkg, asNoBkgAndLocked, asAllObject  };
    allowSelect() : Allowed(asOtherThenBkg) {}

    allow get() const { return Allowed; }
    void set(allow which) { Allowed = which; }
  private:
    allow Allowed;
};
//-----------------------------------------------------------
allowSelect& getAllow();
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
    virtual bool insert(ListData d, bool before, pList nearTo);
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
#if 1
  // se si usa la stessa classe occorre invertire i flag add e rem.
  typedef PListHistory PListRedo;
#else
class PListRedo : public PListHistory
{
  private:
    typedef PListHistory baseClass;
  public:

  protected:
    virtual void freeData(ListData d);
};
#endif
//-----------------------------------------------------------
typedef bool (*fz_isObjGen)(svmObject* t);
typedef svmObject* (*fz_allocObjGen)(svmObject* ed, int id);
//-----------------------------------------------------------
class svmBase
{
  public:
    svmBase(PWin* owner);
    virtual ~svmBase();

    void addObject(svmObject* obj, HDC hdc);
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

    void Redo();
    bool canRedo();

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
    bool savePage(P_File& pfCript, P_File& pfClear, int nBtn);

    void flushHistory() { History.Flush(); RedoHistory.Flush(); }
    void flushObject()  { List.Flush(); ListLinked.Flush(); }

    void setOffset(const POINT& pt);
    void setOffsetLinked(const POINT& pt);
    void setActionToBkg(bool set);
    bool setActionLock(bool set, bool toBkg);
    bool setActionHide(bool set, bool toBkg);
    void setActionMirror(bool all, uint choose);
    void flushSel();

    void flushSelected4Paste();
    void pasteCopied(HDC hdc);
    void copySelected();
    void manageAd4();
    void manageCheckAddr(bool refresh);

    bool canPaste();
    bool setLockObj(bool hide);

    int hasLinked() const { return LinkedStat; }
    void hideShowLinked(bool show);

    enum eBtnStat { nonePressed, eBS_Lock, eBS_Hide, eBS_LockAndHide, eBS_Bkg };
    DWORD getBtnStat() const { return oldBtnStat; }
    void setBtnStat(DWORD newStat)  { oldBtnStat = newStat; }
    void addBtnStat(DWORD newStat)  { oldBtnStat |= newStat; }
    void remBtnStat(DWORD newStat)  { oldBtnStat &= ~newStat; }

    void findBtnStat(bool onBkg);

    bool pasteFromBkg() const { return selectedFromBkg; }
    void setBkg(PBitmap* newBkg);
    void invalidate() { freeBkg(); InvalidateRect(*Owner, 0, 0); }

    void ReInside();
    bool canReInside();

    bool setActionOffset();
    bool canGroup();
    bool canUngroup();

    void actionGroup();
    void actionUngroup(bool all);

    bool canToggleEdTxt();
    bool canToggleChooseTxt();
    void toggleEdTxt();
    void toggleChooseTxt();

    bool canCenter();
    void centerVert();
    void centerHorz();

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
    PListRedo RedoHistory;
    PWin* Owner;

    int findPointAdd(HDC hdc, const POINT&pt);
    int oldFindPoint(HDC hdc, const POINT& pt);

    DWORD oldBtnStat;

  private:
    int getAlignVal(align how, svmObject* obj);
    void unionAlign(align how, svmObject* obj, int& pt);
    void setAlign(align how, svmObject* obj, int pt);
    svmObjHistory::typeOfAction commonProperty(Property* prop, DWORD& bits, LPTSTR& simpleText);

    bool saveNumObjects(P_File& pf, int type, int num);

    static PVect<svmObject*> selected4Paste;
    static bool selectedFromBkg;

    class PBitmap* BmpBkg;

    void makeBkg();
    void freeBkg();

    HBITMAP hBmpWork;
    HDC mdcWork;
    HGDIOBJ oldObj;

    void toggleObjGen(fz_isObjGen fz_one, fz_allocObjGen fz_allocOne, fz_isObjGen fz_two, fz_allocObjGen fz_allocTwo);
};
//-----------------------------------------------------------
inline bool svmBase::canUndo()
{
  return History.getFirst() != 0;
}
//-----------------------------------------------------------
inline bool svmBase::canRedo()
{
  return RedoHistory.getFirst() != 0;
}
//-----------------------------------------------------------
inline void svmBase::flushSel()
{
}
//-----------------------------------------------------------
#endif
