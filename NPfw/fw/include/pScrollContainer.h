//--------- PscrollContainer.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PscrollContainer_H_
#define PscrollContainer_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
class PscrollInfo
{
  public:
    enum scrollType { stNone, stVert, stHorz, stBoth };
    PscrollInfo() : ScrollType(stNone), MaxVert(100), MaxHorz(100) {}
    PscrollInfo(scrollType t, uint maxV, uint maxH) : ScrollType(t), MaxVert(maxV), MaxHorz(maxH) {}

    uint getMaxVert() const { return MaxVert; }
    uint getMaxHorz() const { return MaxHorz; }
    scrollType getScrollType() const { return ScrollType; }

    void setMaxVert(uint m) { MaxVert = m; }
    void setMaxHorz(uint m) { MaxHorz = m; }
    void setScrollType(scrollType t) { ScrollType = t; }

  private:
    uint MaxVert;
    uint MaxHorz;
    scrollType ScrollType;
};
//----------------------------------------------------------------------------
class PclientScrollWin : public PWin
{
  private:
    typedef PWin baseClass;
  public:

    PclientScrollWin(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0,
          LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, x, y, w, h, title, hinst), FirstV(0), FirstH(0) {}

    PclientScrollWin(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, r, title, hinst), FirstV(0), FirstH(0) {}

    virtual ~PclientScrollWin() { destroy(); }

    virtual bool create();
    // deve ritornare l'unità di visualizzazione (es. dimensione miniatura)
    virtual SIZE getBlockSize() { SIZE sz = { 1, 1 }; return sz; }

    virtual void setHPos(uint first) {  FirstH = first;  setPos();  }

    virtual void setVPos(uint first) {  FirstV = first;  setPos();  }
    virtual void resetScroll();
    virtual void setScrollPos(uint x, uint y) { FirstH = x; FirstV = y;  setPos(); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    uint FirstV;
    uint FirstH;

    virtual void resize() {}
    virtual void setPos();
};
//----------------------------------------------------------------------------
class PscrollContainer : public PWin
{
  private:
    typedef PWin baseClass;
  public:

    PscrollContainer(PWin * parent, uint id, const PscrollInfo& iScroll,
          int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0);
    PscrollContainer(PWin * parent, uint id, const PscrollInfo& iScroll,
          const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0);

    virtual ~PscrollContainer();

    virtual bool create();
    virtual void resetScroll();

    virtual void setInfo(const PscrollInfo& iScroll) { IScroll = iScroll; resize(); }
    const PscrollInfo& getInfo() { return IScroll; }

  protected:
    PscrollInfo IScroll;

    uint maxShowVert;
    uint maxShowHorz;
    PclientScrollWin* client;

    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evMouseWheel(short delta, short x, short y);
    virtual PclientScrollWin* makeClient() = 0;
    void resize();
    void evVScrollBar(HWND child, int flags, int pos);
    void evHScrollBar(HWND child, int flags, int pos);

    void checkMarginX(int& pos);
    void checkMarginY(int& pos);

    bool disabledScroll;

  private:
    void init();
};
//----------------------------------------------------------------------------
#endif

