//-------------------- POwnBtnImageStd.h ---------------------------
//-----------------------------------------------------------
#ifndef POWNBTNIMAGESTD_H_
#define POWNBTNIMAGESTD_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnBtn.h"
#include <uxtheme.h>
//#include <tmschema.h>
#include <vssym32.h>
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
typedef HRESULT(__stdcall *ptr_DrawThemeParentBackground)(HWND hwnd, HDC hdc, const RECT *pRect);
typedef BOOL(__stdcall *ptr_IsThemeBackgroundPartiallyTransparent)(HTHEME hTheme, int iPartId, int iStateId);

typedef HRESULT(__stdcall *ptr_CloseThemeData)(HTHEME hTheme);
typedef HRESULT(__stdcall *ptr_DrawThemeBackground)(HTHEME hTheme, HDC hdc,
              int iPartId, int iStateId, const RECT *pRect,  const RECT *pClipRect);
typedef HTHEME(__stdcall *ptr_OpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (__stdcall *ptr_DrawThemeText)(HTHEME hTheme, HDC hdc, int iPartId,
              int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags,
              DWORD dwTextFlags2, const RECT *pRect);
typedef HRESULT (__stdcall *ptr_GetThemeBackgroundContentRect)(HTHEME hTheme,  HDC hdc,
              int iPartId, int iStateId,  const RECT *pBoundingRect,
              RECT *pContentRect);

typedef HRESULT (__stdcall *ptr_DrawThemeEdge)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
              const RECT *pDestRect, UINT uEdge, UINT uFlags, RECT *pContentRect);

typedef HRESULT (__stdcall *ptr_GetThemeColor)(HTHEME hTheme, int iPartId, int iStateId,
              int iPropId, COLORREF *pColor);

typedef COLORREF (__stdcall *ptr_GetThemeSysColor)(HTHEME hTheme, int iColorID);
//-----------------------------------------------------------
#define STAT_PROC(a) static ptr_##a a
//-----------------------------------------------------------
class manageXP_Theme
{
  public:
    manageXP_Theme() { themeChanged(); }
    ~manageXP_Theme() { closeAll(); }
    static void themeChanged();

    STAT_PROC(OpenThemeData);
    STAT_PROC(CloseThemeData);

    STAT_PROC(DrawThemeBackground);
    STAT_PROC(DrawThemeText);
    STAT_PROC(GetThemeBackgroundContentRect);

    STAT_PROC(IsThemeBackgroundPartiallyTransparent);
    STAT_PROC(DrawThemeParentBackground);
    STAT_PROC(DrawThemeEdge);
    STAT_PROC(GetThemeColor);
    STAT_PROC(GetThemeSysColor);

    static bool isValid() { return success; }
  private:
    static HMODULE hModThemes;

    static manageXP_Theme dummy;

    static bool success;

    static void closeAll();

};
//-----------------------------------------------------------
/*
  Disegna un pulsante standard con il bitmap passato, il flag determina
  la posizione del testo e dell'immagine. L'immagine non viene scalata.
  Se non viene passato alcun testo, l'immagine viene posta al centro.
  Nel caso di wCenter sia l'immagine che il testo vengono centrati.
*/
//-----------------------------------------------------------
class IMPORT_EXPORT POwnBtnImageStd : public POwnBtn
{
  private:
    typedef POwnBtn baseClass;
  public:

    enum where { wLeft, wTop, wRight, wBottom, wCenter };

    POwnBtnImageStd(PWin * parent, uint id, int x, int y, int w, int h,
          PBitmap* image, uint fPos = wLeft, bool autoDelete = false,
          LPCTSTR text = 0, HINSTANCE hinst = 0);
    POwnBtnImageStd(PWin * parent, uint id, const PRect& r, PBitmap* image,
          uint fPos = wLeft, bool autoDelete = false, LPCTSTR text = 0, HINSTANCE hinst = 0);
    POwnBtnImageStd(PWin * parent, uint resid, PBitmap* image,
          uint fPos = wLeft, bool autoDelete = false, HINSTANCE hinst = 0);


    POwnBtnImageStd(PWin * parent, uint id, int x, int y, int w, int h,
          PBitmap* image, bool autoDelete, uint fPos = wLeft,
          LPCTSTR text = 0, HINSTANCE hinst = 0);
    POwnBtnImageStd(PWin * parent, uint id, const PRect& r, PBitmap* image,
          bool autoDelete, uint fPos = wLeft, LPCTSTR text = 0, HINSTANCE hinst = 0);
    POwnBtnImageStd(PWin * parent, uint resid, PBitmap* image,
          bool autoDelete, uint fPos = wLeft, HINSTANCE hinst = 0);

    virtual ~POwnBtnImageStd();

    // il secondo parametro è solo per compatibilità
    PBitmap* replaceBmp(PBitmap* other, uint pos = 0);
    void themeChanged();

    bool create();
    bool enabledTheme() const { return !forceNoTheme; }
    void disableTheme(bool set) { forceNoTheme = set; loadTheme(); }

    enum imgStyle { eisFixed, eisScaled, eisStretched };
    void setImgStyle(imgStyle set) { scaleImg = set; }
  protected:
    PBitmap* Bmp;
    uint flagPos;
    bool autoDeleteBmp;

    HTHEME hTheme;
    bool forceNoTheme;
    bool mouseOverButton;
    imgStyle scaleImg;
    bool isThemed() const { return toBool(hTheme); }

    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual bool drawItem(uint idCtrl, LPDRAWITEMSTRUCT dis);
    virtual void drawText(HDC hdc, const PRect& rect, POwnBtn::howDraw style);
    virtual void drawCustom(HDC hdc, const PRect& rect, POwnBtn::howDraw style);
    virtual void mouseEnter(const POINT& pt, uint flag);
    virtual void mouseLeave(const POINT& pt, uint flag);
    virtual void invalidateIf();
    virtual PRect getRectDrawButton(bool withFocus) const;
    virtual void drawFocusAfter(HDC hdc, LPDRAWITEMSTRUCT dis);
    virtual void drawFocusThemed(HDC hdc, COLORREF c, const PRect& r);

    virtual void makeRectImageButton(PRect& r);
    virtual void adjustRectText(const PRect& rect, PRect& calc);

  private:
    void loadTheme();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline
POwnBtnImageStd::POwnBtnImageStd(PWin * parent, uint id, int x, int y, int w, int h,
            PBitmap* image, uint fPos, bool autoDelete, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, text, hinst),
      Bmp(image), flagPos(fPos), autoDeleteBmp(autoDelete), hTheme(0), mouseOverButton(false),
      forceNoTheme(false), scaleImg(eisFixed)
{
}
//-----------------------------------------------------------
inline
POwnBtnImageStd::POwnBtnImageStd(PWin * parent, uint id, const PRect& r,
            PBitmap* image, uint fPos, bool autoDelete, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, r, text, hinst),
      Bmp(image), flagPos(fPos), autoDeleteBmp(autoDelete), hTheme(0), mouseOverButton(false),
      forceNoTheme(false), scaleImg(eisFixed)
{
}
//-----------------------------------------------------------
inline POwnBtnImageStd::POwnBtnImageStd(PWin * parent, uint resid,
            PBitmap* image, uint fPos, bool autoDelete, HINSTANCE hinst) :
        baseClass(parent, resid, hinst),
        Bmp(image), flagPos(fPos), autoDeleteBmp(autoDelete), hTheme(0), mouseOverButton(false),
      forceNoTheme(false), scaleImg(eisFixed)
{
}
//-----------------------------------------------------------
//-----------------------------------------------------------
inline
POwnBtnImageStd::POwnBtnImageStd(PWin * parent, uint id, int x, int y, int w, int h,
            PBitmap* image, bool autoDelete, uint fPos, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, text, hinst),
      Bmp(image), flagPos(fPos), autoDeleteBmp(autoDelete), hTheme(0), mouseOverButton(false),
      forceNoTheme(false), scaleImg(eisFixed)
{
}
//-----------------------------------------------------------
inline
POwnBtnImageStd::POwnBtnImageStd(PWin * parent, uint id, const PRect& r,
            PBitmap* image, bool autoDelete, uint fPos, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, r, text, hinst),
      Bmp(image), flagPos(fPos), autoDeleteBmp(autoDelete), hTheme(0), mouseOverButton(false),
      forceNoTheme(false), scaleImg(eisFixed)
{
}
//-----------------------------------------------------------
inline POwnBtnImageStd::POwnBtnImageStd(PWin * parent, uint resid,
            PBitmap* image, bool autoDelete, uint fPos, HINSTANCE hinst) :
        baseClass(parent, resid, hinst),
        Bmp(image), flagPos(fPos), autoDeleteBmp(autoDelete), hTheme(0), mouseOverButton(false),
      forceNoTheme(false), scaleImg(eisFixed)
{
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
