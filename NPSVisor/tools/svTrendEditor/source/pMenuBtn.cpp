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
#ifdef USE_NEW_FLAT
  setFlat(true);
#endif
  return baseClass::create();
}
//----------------------------------------------------------------------------
void hilightBtn::mouseEnter(const POINT& pt, uint flag)
{
  old = replaceBmp(lightImg);
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
  replaceBmp(old);
  old = 0;
  InvalidateRect(*this, 0, 0);
}
//----------------------------------------------------------------------------
void hilightBtn::mouseMove(const POINT& pt, uint flag)
{
  baseClass::mouseMove(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//-------------------------------------------------------------------
#define DB_BTN(id, a, t) { id, a, a##2, t }
#define NEW_BTN new PBitmap(iBtn[i].idBtn, getHInstance())
#define NEW_BTN2 new PBitmap(iBtn[i].idBtn2, getHInstance())
#define TXT_BTN iBtn[i].Text
//-------------------------------------------------------------------
bool pMenuBtn::create()
{
  PRect r(OFFS_BTN, OFFS_BTN + 10, OFFS_BTN + SIZE_BTN_H, OFFS_BTN + SIZE_BTN_V + 10);
  struct infoBtn
  {
    uint id;
    uint idBtn;
    uint idBtn2;
    LPCTSTR Text;
  };

  infoBtn iBtn[] = {
    DB_BTN(CM_BTN_FOLDER, IDB_BITMAP_M_FOLDER, getStringOrDef(IDT_BROWSE, _T("Sfoglia..."))),
    DB_BTN(CM_BTN_ADD, IDB_BITMAP_M_ADD, getStringOrDef(IDT_ADD, _T("Aggiungi..."))),
    DB_BTN(CM_BTN_REM, IDB_BITMAP_M_REM, getStringOrDef(IDT_REM, _T("Rimuovi..."))),
    DB_BTN(CM_BTN_COPY, IDB_BITMAP_M_COPY, getStringOrDef(IDT_COPY, _T("Copia"))),
    DB_BTN(CM_BTN_PASTE, IDB_BITMAP_M_PASTE, getStringOrDef(IDT_PASTE, _T("Incolla"))),
    DB_BTN(CM_BTN_UPD, IDB_BITMAP_M_UPD, getStringOrDef(IDT_RELOAD, _T("Ricarica"))),
    DB_BTN(CM_BTN_SAVE, IDB_BITMAP_M_SAVE, getStringOrDef(IDT_SAVE, _T("Salva"))),
    DB_BTN(CM_BTN_EXPORT, IDB_BITMAP_M_EXPORT, getStringOrDef(IDT_EXPORT, _T("Esporta"))),
//    DB_BTN(CM_BTN_IMPORT, IDB_BITMAP_M_IMPORT, _T("Importa")),
    DB_BTN(CM_BTN_DONE, IDB_BITMAP_M_DONE, getStringOrDef(IDT_DONE, _T("Finito"))),
    };
  POwnBtn::colorRect cr;
  cr.focus = RGB(0x3f, 0x7f,0xff);
  cr.tickness = 3;
  for(uint i = 0; i < SIZE_A(iBtn); ++i) {
    hilightBtn* btn = new hilightBtn(this, iBtn[i].id, r, NEW_BTN, NEW_BTN2, TXT_BTN);
    btn->setColorRect(cr);
    r.Offset(0, SIZE_BTN_V);
    }
  r.Offset(-r.left + 3, 0);
  new PCheckBox(this, ICD_CHECK_DATE, r, getStringOrDef(IDT_MOD_DATE, _T("Mod.data")));

  if(baseClass::create())
    return true;

  return false;
}

//---------------------------------------------------------
LRESULT pMenuBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case CM_BTN_FOLDER:
        case CM_BTN_ADD:
        case CM_BTN_REM:
        case CM_BTN_COPY:
        case CM_BTN_PASTE:
        case CM_BTN_UPD:
        case CM_BTN_SAVE:
        case CM_BTN_EXPORT:
//        case CM_BTN_IMPORT:
        case CM_BTN_DONE:
          PostMessage(*getParent(), WM_CUSTOM_BY_BTN, MAKEWPARAM(LOWORD(wParam), 0), 0);
          break;
        case ICD_CHECK_DATE:
          PostMessage(*getParent(), WM_CUSTOM_BY_BTN, MAKEWPARAM(LOWORD(wParam), 0), IS_CHECKED(ICD_CHECK_DATE));
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------
void pMenuBtn::enableDisable(uint flag)
{
  uint ids[] = { CM_BTN_ADD, CM_BTN_REM /*, CM_BTN_EXPORT */ };

  ENABLE(CM_BTN_SAVE, !(flag & 2));
  ENABLE(CM_BTN_UPD, !(flag & 4));
  ENABLE(CM_BTN_COPY, !(flag & 8));
  ENABLE(CM_BTN_PASTE, !(flag & 16));
  bool enable = !(flag & ~31);
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], enable);
}
//-------------------------------------------------------------------

