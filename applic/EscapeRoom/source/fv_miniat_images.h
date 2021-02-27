//-------- fv_miniat_images.h -----------------------------------------
//-------------------------------------------------------------------
#ifndef fv_miniat_images_H
#define fv_miniat_images_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "fullView.h"
#include "Pminiat.h"
#include "pBitmap.h"
#include "p_Vect.h"
#include "p_util.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class miniat_image : public Pminiat
{
  private:
    typedef Pminiat baseClass;
  public:
    miniat_image(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0,
          LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, x, y, w, h, title, hinst), Bmp(0)  {}

    miniat_image(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
    baseClass(parent, id,r, title, hinst), Bmp(0) {}

    virtual ~miniat_image() { destroy(); delete Bmp; }

    virtual SIZE getBlockSize() { SIZE sz = { 160 * 2, 90 * 2 }; return sz;}
    void loadImage(LPCTSTR basePath, LPCTSTR image);
  protected:
    virtual void evPaint(HDC hdc);
    virtual HBRUSH createBkg(bool& needDestroy);
    virtual void paintSel(HDC hdc);
    PBitmap* Bmp;
};
//----------------------------------------------------------------------------
class client_miniat_images : public PclientScrollMiniat
{
  private:
    typedef PclientScrollMiniat baseClass;
  public:

    client_miniat_images(PWin * parent, uint id, uint nVert, uint nHorz, int x = 0, int y = 0, int w = 0, int h = 0,
          LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, nVert, nHorz, x, y, w, h, title, hinst), basePath(0) {}

    client_miniat_images(PWin * parent, uint id, uint nVert, uint nHorz, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, nVert, nHorz, r, title, hinst), basePath(0) {}

    virtual ~client_miniat_images();

    virtual void recreate_miniat();
    virtual void setBasePath(LPCTSTR newPath);

    virtual bool create();

    void setSelected(uint idc);

  protected:
    virtual Pminiat* makeMiniat(uint ix, const PRect& r);
    PVect<LPCTSTR> images;
    LPCTSTR basePath;
    void fillList();
};
//----------------------------------------------------------------------------
class container_miniat_images : public PscrollContainer
{
  private:
    typedef PscrollContainer baseClass;
  public:

    container_miniat_images(PWin * parent, uint id, const PscrollInfo& iScroll,
          int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
            baseClass(parent, id, iScroll, x, y, w, h, title, hinst) {}

    container_miniat_images(PWin * parent, uint id, const PscrollInfo& iScroll,
          const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
            baseClass(parent, id, iScroll, r, title, hinst) {}

    virtual ~container_miniat_images() { destroy(); }

    virtual void setBasePath(LPCTSTR newPath);
    int getSelected() const;
    void setSelected(uint idc);

  protected:
    virtual PclientScrollWin* makeClient() { return new client_miniat_images(this, 222, IScroll.getMaxVert(), IScroll.getMaxHorz()); }
};
//-------------------------------------------------------------------
class fv_miniat_images : public fullView
{
  private:
    typedef fullView baseClass;
  public:
    fv_miniat_images(PWin* parent, LPCTSTR title = 0, HINSTANCE hInst = 0) : baseClass(parent, title, hInst), List(0) { }

    fv_miniat_images(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst), List(0)  { }

    fv_miniat_images(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst), List(0)  { }

    ~fv_miniat_images() { destroy(); }

    bool create();
    virtual void setBasePath(LPCTSTR newPath) { List->setBasePath(newPath); }
    int getSelected() const { return List->getSelected(); }
    void setSelected(uint idc) { List->setSelected(idc); }
  protected:
    virtual HWND getHWClient() { return *List; }

    container_miniat_images* List;
};
//-------------------------------------------------------------------
#endif
