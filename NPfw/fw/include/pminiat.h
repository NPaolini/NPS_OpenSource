//--------- Pminiat.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef Pminiat_H_
#define Pminiat_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "PscrollContainer.h"
#include "p_vect.h"
#include "pMDC_custom.h"
//----------------------------------------------------------------------------
#define BORDER_MINIAT 6
//----------------------------------------------------------------------------
class Pminiat;
//----------------------------------------------------------------------------
class PclientScrollMiniat : public PclientScrollWin
{
  private:
    typedef PclientScrollWin baseClass;
  public:

    PclientScrollMiniat(PWin * parent, uint id, uint nVert, uint nHorz, int x = 0, int y = 0, int w = 0, int h = 0,
          LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, x, y, w, h, title, hinst),  nVert(nVert), nHorz(nHorz), currSel(0), clients(0) {}

    PclientScrollMiniat(PWin * parent, uint id, uint nVert, uint nHorz, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, r, title, hinst),  nVert(nVert), nHorz(nHorz), currSel(0), clients(0) {}

    virtual ~PclientScrollMiniat();

    virtual bool create();

    virtual SIZE getBlockSize();

    virtual void setSelected(uint idc);
    int getSelected() const { return currSel; }

  protected:
    uint nVert;
    uint nHorz;

    int currSel;

    PVect<Pminiat*> clients;

    virtual Pminiat* makeMiniat(uint ix, const PRect& r) = 0;
//    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void resize();
};
//----------------------------------------------------------------------------
class Pminiat : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    Pminiat(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0,
          LPCTSTR title = 0, HINSTANCE hinst = 0);
    Pminiat(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0);

    virtual ~Pminiat();

    void setSelected(bool set);
    bool isSelected() const { return Selected; }

//    virtual bool create();

    virtual SIZE getBlockSize() = 0;

    uint getId() const { return Attr.id; }

  protected:
    bool Selected;

    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual void evPaint(HDC hdc) = 0;

    // richiamata dopo la evPaint per disegnare l'eventuale selezione
    virtual void paintSel(HDC hdc);

    virtual HBRUSH createBkg(bool& needDestroy) = 0;
    pMDC_custom mdc_custom;

};
//----------------------------------------------------------------------------
PclientScrollMiniat* getScrollMiniat(PWin* w);
//----------------------------------------------------------------------------
#endif

