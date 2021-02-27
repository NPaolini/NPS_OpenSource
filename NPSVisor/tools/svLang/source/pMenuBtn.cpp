//-------- pMenuBtn.cpp ---------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pMenuBtn.h"
#include "POwnBtnImageStd.h"
#include "pCheckBox.h"
#include "resource.h"
#include "macro_Utils.h"
//-------------------------------------------------------------------
//#define USE_FLAT
#define USE_NEW_FLAT
//-------------------------------------------------------------------
class hilightBtn : public POwnBtnImageStd
{
  private:
    typedef POwnBtnImageStd baseClass;
  public:
    hilightBtn(PWin * parent, uint id, int x, int y, int w, int h,
          PBitmap* image, PBitmap* image2, LPCTSTR text = 0, HINSTANCE hinst = 0) :
    baseClass(parent, id, x, y, w, h, image, wTop, true, text, hinst), old(0),
      lightImg(image2) { init(); }

    hilightBtn(PWin * parent, uint id, const PRect& r, PBitmap* image, PBitmap* image2,
          LPCTSTR text = 0, HINSTANCE hinst = 0) :
    baseClass(parent, id, r, image, wTop, true, text, hinst), old(0),
      lightImg(image2) { init(); }

    hilightBtn(PWin * parent, uint resid, PBitmap* image, PBitmap* image2, HINSTANCE hinst = 0) :
    baseClass(parent, resid, image, wTop, true, hinst), old(0),
      lightImg(image2) { init(); }

    ~hilightBtn();
    bool create();
  protected:
    PBitmap* lightImg;
    PBitmap* old;
#ifdef USE_FLAT
    colorRect oldC;
    void drawFocusAfter(HDC hdc, LPDRAWITEMSTRUCT /*dis*/);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif
    void init();
    void mouseEnter(const POINT& pt, uint flag);
    void mouseLeave(const POINT& pt, uint flag);
    void mouseMove(const POINT& pt, uint flag);

    void restore();
};
//----------------------------------------------------------------------------
hilightBtn::~hilightBtn()
{
  if(old)
    replaceBmp(old);
  delete lightImg;
  destroy();
}
//----------------------------------------------------------------------------
void hilightBtn::init()
{
  Attr.style |= WS_TABSTOP;
  enableCapture();
}
//----------------------------------------------------------------------------
bool hilightBtn::create()
{
#ifdef USE_FLAT
  setDrawingEdge(false);
  oldC = getColorRect();
  colorRect t = oldC;
  t.bkg = -1;
  setColorRect(t);
#endif
#ifdef USE_NEW_FLAT
  setFlat(true);
#endif
  return baseClass::create();
}
//----------------------------------------------------------------------------
#ifdef USE_FLAT
void hilightBtn::drawFocusAfter(HDC hdc, LPDRAWITEMSTRUCT dis)
{
  if(!old) {
    old = replaceBmp(lightImg);
    InvalidateRect(*this, 0, 1);
    }
}
//----------------------------------------------------------------------------
LRESULT hilightBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_KILLFOCUS:
      restore();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
void hilightBtn::mouseEnter(const POINT& pt, uint flag)
{
#ifdef USE_FLAT
  setColorRect(oldC);
  setDrawingEdge(true);
  if(!old)
    old = replaceBmp(lightImg);
#else
  old = replaceBmp(lightImg);
#endif
  InvalidateRect(*this, 0, 0);
  baseClass::mouseEnter(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
void hilightBtn::mouseLeave(const POINT& pt, uint flag)
{
  restore();
  baseClass::mouseLeave(pt, flag);
}
//----------------------------------------------------------------------------
void hilightBtn::restore()
{
#ifdef USE_FLAT
  colorRect t = oldC;
  t.bkg = -1;
  setColorRect(t);
  setDrawingEdge(false);
  if(old)
    replaceBmp(old);
#else
  replaceBmp(old);
#endif
  old = 0;
#ifdef USE_FLAT
  PRect r = getRect();
  r.Inflate(4, 4);

  InvalidateRect(*getParent(), r, 1);
#endif
  InvalidateRect(*this, 0, 0);
}
//----------------------------------------------------------------------------
void hilightBtn::mouseMove(const POINT& pt, uint flag)
{
  baseClass::mouseMove(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//-------------------------------------------------------------------
#define ICD_CHECK_UNICODE 5000
//-------------------------------------------------------------------
bool pMenuBtn::saveOnUnicode()
{
  return true;
//  return IS_CHECKED(ICD_CHECK_UNICODE);
}
//-------------------------------------------------------------------
#define DB_BTN(a, t, i) { a, a, a##2, t, i }
#define NEW_BTN new PBitmap(iBtn[i].idBtn, getHInstance())
#define NEW_BTN2 new PBitmap(iBtn[i].idBtn2, getHInstance())
#define TXT_BTN getStringOrDef(iBtn[i].idLang, iBtn[i].Text)
//-------------------------------------------------------------------
bool pMenuBtn::create()
{
  PRect r(OFFS_BTN, OFFS_BTN + 10, OFFS_BTN + SIZE_BTN, OFFS_BTN + SIZE_BTN);
  struct infoBtn
  {
    uint id;
    uint idBtn;
    uint idBtn2;
    LPCTSTR Text;
    uint idLang;
  };
  infoBtn iBtn[] = {
    DB_BTN(IDB_BITMAP_M_DONE, _T("Finito"), 1),
    DB_BTN(IDB_BITMAP_M_UPD, _T("Aggiorna"), 2),
    DB_BTN(IDB_BITMAP_M_FOLDER, _T("Sfoglia..."), 3),
    DB_BTN(IDB_BITMAP_M_SAVE, _T("Salva"), 4),
    DB_BTN(IDB_BITMAP_M_EXP, _T("Esporta"), 5),
    DB_BTN(IDB_BITMAP_M_IMP, _T("Importa"), 6),
    };
#ifdef USE_FLAT
  for(uint i = 0; i < SIZE_A(iBtn); ++i) {
    hilightBtn* btn = new hilightBtn(this, iBtn[i].id, r, NEW_BTN, NEW_BTN2, TXT_BTN);
    r.Offset(0, SIZE_BTN);
    }
#else
  POwnBtn::colorRect cr;
  cr.focus = RGB(0x3f, 0x7f,0xff);
  cr.tickness = 3;
  for(uint i = 0; i < SIZE_A(iBtn); ++i) {
    hilightBtn* btn = new hilightBtn(this, iBtn[i].id, r, NEW_BTN, NEW_BTN2, TXT_BTN);
    btn->setColorRect(cr);
    r.Offset(0, SIZE_BTN);
    }
#endif
  r.Offset(-r.left + 3, -5);
//  new PCheckBox(this, ICD_CHECK_UNICODE, r, getStringOrDef(7, _T("Unicode")));
  if(baseClass::create()) {
    setUnicode();
    return true;
    }
  return false;
}
//---------------------------------------------------------
void pMenuBtn::setUnicode()
{
//  SET_CHECK(ICD_CHECK_UNICODE);
}
//---------------------------------------------------------
LRESULT pMenuBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDB_BITMAP_M_DONE:
          PostMessage(*getParent(), WM_CUSTOM_BY_BTN, MAKEWPARAM(CM_BTN_DONE, 0), 0);
          break;
        case IDB_BITMAP_M_UPD:
          PostMessage(*getParent(), WM_CUSTOM_BY_BTN, MAKEWPARAM(CM_BTN_UPD, 0), 0);
          break;
        case IDB_BITMAP_M_FOLDER:
          PostMessage(*getParent(), WM_CUSTOM_BY_BTN, MAKEWPARAM(CM_BTN_FOLDER, 0), 0);
          break;
        case IDB_BITMAP_M_SAVE:
          PostMessage(*getParent(), WM_CUSTOM_BY_BTN, MAKEWPARAM(CM_BTN_SAVE, 0), 0);
          break;
        case IDB_BITMAP_M_EXP:
          PostMessage(*getParent(), WM_CUSTOM_BY_BTN, MAKEWPARAM(CM_BTN_EXPORT, 0), 0);
          break;
        case IDB_BITMAP_M_IMP:
          PostMessage(*getParent(), WM_CUSTOM_BY_BTN, MAKEWPARAM(CM_BTN_IMPORT, 0), 0);
          break;

        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
