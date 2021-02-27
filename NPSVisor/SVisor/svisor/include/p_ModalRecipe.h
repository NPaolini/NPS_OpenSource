//--------P_ModalRecipe.h ------------------------------------------------------
#ifndef P_ModalRecipe_H_
#define P_ModalRecipe_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "recipe.h"
#include "mainClient.h"
#include "p_ModalBody.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class P_ModalRecipe : virtual public P_ModalBody, virtual public TD_Recipe
{
  public:
    P_ModalRecipe(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);

    virtual ~P_ModalRecipe();

    bool create();
    void setReady(bool first);

    P_Body* pushedBtn(int idBtn);

    virtual void refreshBody();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class P_ModelessRecipe : public P_ModalRecipe
{
  public:
    P_ModelessRecipe(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);

    virtual ~P_ModelessRecipe();

    void setReady(bool first);

    P_Body* pushedBtn(int idBtn);

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    void savePos(HWND hwnd);

};
//----------------------------------------------------------------------------
class manageModal
{
  public:
    manageModal() : Owner(0) {}
    virtual ~manageModal() {}
    void setOwner(P_Body* owner) { Owner = owner; }

    virtual bool windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result);
    virtual bool pushedBtn(int idBtn);
    virtual bool acceptChildPage(int idBtn);
    virtual void create();
    bool canRefresh() { return !exiting; }
  protected:
    bool exiting;
    P_Body* Owner;
};

//----------------------------------------------------------------------------
class baseModal
{
  public:
    P_ModalBody(PWin* parent, LPCTSTR pageDescr);
    P_ModalBody(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);

    virtual ~P_ModalBody();

    bool create();
    void setReady(bool first);

    P_Body* pushedBtn(int idBtn);

    virtual void refresh();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR findChildPage(int idBtn, int& idPar, int& pswLevel, bool& predefinite);
    bool noCaption;
    bool Transp;
    void makeRegion();
    bool exiting;
  private:
    bool getSize(int& w, int& h);

    typedef P_DefBody baseClass;
};


//----------------------------------------------------------------------------
bool manageModal::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
        case IDC_BUTTON_F2:
        case IDC_BUTTON_F3:
        case IDC_BUTTON_F4:
        case IDC_BUTTON_F5:
        case IDC_BUTTON_F6:
        case IDC_BUTTON_F7:
        case IDC_BUTTON_F8:
        case IDC_BUTTON_F9:
        case IDC_BUTTON_F10:
        case IDC_BUTTON_F11:
        case IDC_BUTTON_F12:
          pushedBtn(LOWORD(wParam) - IDC_BUTTON_F1 + ID_F1);
          result = 1;
          return true;
        }
      break;
    case WM_NCHITTEST:
      if(noCaption) {
        result = HTCAPTION;
        return true;
        }
      break;
    case WM_NCMOUSEMOVE:
      if(noCaption)
        SetCursor(LoadCursor(Owner->getHInstance(),  MAKEINTRESOURCE(IDC_CURS_SLIDER_DX)));
      break;
    case WM_NCLBUTTONDOWN:
      if(noCaption)
        SetCursor(LoadCursor(Owner->getHInstance(),  MAKEINTRESOURCE(IDC_CURS_SLIDER_DX)));
      break;
    case WM_NCLBUTTONUP:
      if(noCaption)
        SetCursor(LoadCursor(Owner->getHInstance(),  MAKEINTRESOURCE(IDC_CURS_SLIDER_DX)));
      break;
    }
  return false;
}
//----------------------------------------------------------------------------
bool manageModal::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F1:
      Owner->sendListValue(false);
      Owner->refresh();
      exiting = true;
      if(isFlagSet(pfModal))
        EndDialog(*Owner, IDOK);
      else
        DestroyWindow(*Owner);
      return 0;

    default:
      break;
    }
  return true;
}
//----------------------------------------------------------------------------
bool manageModal::acceptChildPage(int idBtn)
{
  int id = idBtn + MAX_BTN;
  LPCTSTR p = Owner->getPageString(id);

  if(!p)
    return false;
  id = _ttoi(p);
  switch(id) {
    case ID_OPEN_PAGE:
    case ID_OPEN_TREND:
    case ID_OPEN_RECIPE:
    case ID_OPEN_RECIPE_ROW:
    case ID_OPEN_MODELESSBODY:

    case ID_GOTO_PREVIOUS_PAGE:
    case ID_SHUTDOWN:
    case ID_SHOW_TREEVIEW:
    case ID_PRINT_TREEVIEW:
    case ID_SHOW_ALARM:
//    case ID_PRINT_SCREEN:
      return false;

    }
  return true;
}


//----------------------------------------------------------------------------
void baseModal::makeRegion()
{
  HRGN hrgn = 0;
  addRegionObj(Txt, hrgn);
  addRegionObj(Edi, hrgn);
  addRegionObj(Btn, hrgn);
  addRegionObj(Diam, hrgn);
  addRegionObj(LBox, hrgn);
  addRegionObj(Choose, hrgn);
  addRegionObj(Bmp, hrgn);
  addRegionObj(Led, hrgn);
  addRegionObj(barGraph, hrgn);
  addRegionObj(Curve, hrgn);
  addRegionObj(Cam, hrgn);
  addRegionObj(XMeter, hrgn);
  addRegionObj(Scope, hrgn);
  addRegionObj(oAlarm, hrgn);
  addRegionObj(oTrend, hrgn);
  addRegionObj(oSpin, hrgn);
  addRegionObjCheckBmp(Simple, hrgn);
  addRegionObjCheckBmp(SimpleWithHide, hrgn);
  if(hrgn) {
    SetWindowRgn(*this, hrgn, false);
    if(isWinXP_orLater()) {
      typedef int (WINAPI *pGetWindowRgnBox)(HWND hWnd, LPRECT lprc);
      HMODULE h = LoadLibrary(_T("USER32.DLL"));
      if(h) {
        FARPROC f = GetProcAddress(h, "GetWindowRgnBox");
        if(f) {
          PRect r;
          if(ERROR != ((pGetWindowRgnBox)f)(*this, r)) {
            Attr.w = r.Width();
            Attr.h = r.Height();
            }
          }
        FreeLibrary(h);
        }
      }
    }
}
//----------------------------------------------------------------------------
bool baseModal::create()
{
  smartPointerConstString tit = getTitle();
  if(!baseClass::create())
    return false;
  Attr.id = IDD_MODAL_BODY;
  setReady(false);
  ManModal.create);
  SetWindowText(*this, tit);
  return true;
}
//----------------------------------------------------------------------------
void manageModal::create()
{
  LPCTSTR p = Owner->getPageString(ID_NO_CAPTION_MODAL);
  int subW = 0;
  int subH = 0;
  if(p && 1 == _ttoi(p)) {
    p = findNextParamTrim(p);
    if(p && 1 == _ttoi(p)) {
      Transp = true;
      makeRegion();
      }
    Owner->Attr.style &= ~WS_CAPTION;
    SetWindowLongPtr(*Owner, GWL_STYLE, GetWindowLongPtr(*Owner, GWL_STYLE) & ~WS_CAPTION);
    noCaption = true;
    if(!Transp || !isWinXP_orLater()) {
      subW = R__X(5);
      subH = GetSystemMetrics(SM_CYCAPTION) + R__X(5);
      }
    }

  int x = 0;
  int y = 0;
  int w = GetSystemMetrics(SM_CXSCREEN);
  int h = GetSystemMetrics(SM_CYSCREEN);
  alignWithMonitor(*Owner, x, y);
  if(w < Owner->Attr.x + Owner->Attr.w - x)
    Owner->Attr.x = w - Owner->Attr.w + x;
  else if(Owner->Attr.x < 0)
    Owner->Attr.x = 0;
  if(h < Owner->Attr.y + Owner->Attr.h - y)
    Owner->Attr.y = h - Owner->Attr.h + y;
  else if(Owner->Attr.y < 0)
    Owner->Attr.y = 0;
  SetWindowPos(*Owner, 0, Owner->Attr.x, Owner->Attr.y, Owner->Attr.w - subW, Owner->Attr.h - subH, SWP_NOZORDER);

  InvalidateRect(*Owner, 0, 1);
  SetWindowText(*Owner, tit);
  ShowWindow(*Owner, SW_SHOWNORMAL);
  return true;
}
//----------------------------------------------------------------------------
bool baseModal::getSize(int& w, int& h)
{
  int x = 0;
  int y = 0;
  LPCTSTR p = getPageString(ID_OFFSET_X);
  if(p)
    x = _ttoi(p);
  p = getPageString(ID_OFFSET_Y);
  if(p)
    y = _ttoi(p);

  PRect area;
  calcArea(Txt, area);
  calcArea(Edi, area);
  calcArea(Btn, area);
  calcArea(Diam, area);
  calcArea(LBox, area);
  calcArea(Choose, area);
  calcArea(Bmp, area);
  calcArea(Led, area);
  calcArea(barGraph, area);
  calcAreaCheckBmp(Simple, area);
  calcAreaCheckBmp(SimpleWithHide, area);
  calcArea(Curve, area);
  calcArea(Cam, area);
  calcArea(XMeter, area);
  calcArea(Scope, area);
  calcArea(oAlarm, area);
  calcArea(oTrend, area);
  calcArea(oSpin, area);
  calcArea(oTableInfo, area);

  area.Offset(x, y);

  if(!area.Width())
    area.right = area.left + 300;
  if(!area.Height())
    area.bottom = area.top + 200;
  if(w < 0)
    w = area.right + R__X(10);
  if(h < 0)
    h = area.bottom + R__Y(10) + GetSystemMetrics(SM_CYCAPTION);
  return true;
}

//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

