//-------------------- clientWin.h ---------------------------
//-----------------------------------------------------------
#ifndef clientWin_H_
#define clientWin_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pNumberImg.h"
//-------------------------------------------------------------------
#define TEXT_FONT _T("text_font")
#define TEXT_COLOR _T("text_color")
#define TEXT_BORDER _T("text_border")
#define TEXT_MARGIN _T("text_margin")
#define TEXT_BASE _T("text_")

#define IMG_INFO _T("img_info")
#define IMG_INFO_PERC _T("img_info_perc")
#define IMG_INFO_TOP _T("img_info_top")
//-----------------------------------------------------------
struct infoText
{
  uint nBorder;   // numero di bordi (0, 1, 2) il più interno è nero, l'altro è bianco
  LPCTSTR fontname;
  uint padding;
  COLORREF background; // se (DWORD)-1 allora è trasparente, altrimenti il colore passato
  COLORREF color;
  infoText() : nBorder(0), fontname(0), padding(0), background((DWORD)-1), color(0) {}
};
//-----------------------------------------------------------
class pInfoText
{
  public:
    pInfoText(const infoText& info) : Info(info) {}
    virtual void draw(HDC hdc, const PVect<LPCTSTR>& txt, const POINT& pt, const SIZE& sz);
    virtual void draw(HDC hdc, LPCTSTR txt, const POINT& pt, const SIZE& sz);
  protected:
    const infoText& Info;
};
//-----------------------------------------------------------
class pInfoImg
{
  public:
    pInfoImg() {}
    virtual void draw(HDC hdc, PBitmap* bmp, const PRect& r);
};
//-------------------------------------------------------------------
class clientWin : public fullView
{
  private:
    typedef fullView baseClass;
  public:
    clientWin(PWin* parent, LPCTSTR title = 0, HINSTANCE hInst = 0) :
        baseClass(parent, title, hInst), imgPath(0), img(0), changed(false) { }

    clientWin(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst), imgPath(0), img(0), changed(false) { }

    clientWin(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst), imgPath(0), img(0), changed(false) { }

    ~clientWin();

    bool create();
    void setBackground(LPCTSTR path);
    PBitmap* getImage() { return img; }
    bool isChanged();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evPaint(HDC hdc, const PRect& r);
    virtual void resize();
    LPCTSTR imgPath;
    pMDC_custom mdc;
    PBitmap* img;
    bool changed;
};
//-----------------------------------------------------------
#endif
