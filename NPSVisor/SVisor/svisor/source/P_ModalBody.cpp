//------ P_ModalBody.cpp -------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "p_ModalBody.h"
#include "pAllObj.h"
#include "id_btn.h"
#include "password.h"
#include "p_util.h"
//#include "def_dir.h"
#include "lnk_body.h"
//#include "newnormal.h"
#include "sizer.h"
//----------------------------------------------------------------------------
#include "1.h"
//---------------------------------------------------------------------
static void makeTraspRegion(HRGN region, PSimpleBmp* obj)
{
  HDC hdc = GetDC(0);
  PRect r = obj->getTrueRect();
  POINT offs = { r.left, r.top };
  HBITMAP hBmp = CreateCompatibleBitmap(hdc, r.Width(), r.Height());
  if(hBmp) {
    HDC mdc = CreateCompatibleDC(hdc);
    HGDIOBJ oldObj = SelectObject(mdc, hBmp);
    obj->drawAtZero(mdc, r);
    COLORREF pixTransp = GetPixel(mdc, 0, 0);


    bool bInTransparency = false;  // Already inside a transparent part?
    int start_x = -1;        // Start of transparent part
    r.MoveTo(0, 0);

    // For all rows of the bitmap ...
    for (int y = 0; y < r.bottom; y++) {
      // For all pixels of the current row ...
      // (To close any transparent region, we go one pixel beyond the
      // right boundary)
      for (int x = 0; x <= r.right; x++) {
        bool bTransparent = false; // Current pixel transparent?

        // Check for positive transparency within image boundaries
        if((x < r.right) && (pixTransp == GetPixel(mdc, x, y)))
          bTransparent = true;

        // Does transparency change?
        if (bInTransparency != bTransparent) {
          if (bTransparent) {
            // Transparency starts. Remember x position.
            bInTransparency = true;
            start_x = x;
            }
          else  {
            // Transparency ends (at least beyond image boundaries).
            // Create a region for the transparency, one pixel high,
            // beginning at start_x and ending at the current x, and
            // subtract that region from the current bitmap region.
            // The remainding region becomes the current bitmap region.
            HRGN hrgnTransp = CreateRectRgn(start_x + offs.x, y + offs.y, x + offs.x, y + 1 + offs.y);
            CombineRgn(region, region, hrgnTransp, RGN_DIFF);
            DeleteObject(hrgnTransp);

            bInTransparency = false;
            }
          }
        }
      }
    DeleteObject(SelectObject(mdc, oldObj));
    DeleteDC(mdc);
    }
  ReleaseDC(0, hdc);
}
//----------------------------------------------------------------------------
template <typename T>
void createRegionObj(T& obj, HRGN region)
{
  PRect r = obj->get_Rect();
  HRGN hrgn = CreateRectRgn(r.left, r.top, r.right, r.bottom);
  CombineRgn(region, region, hrgn, RGN_OR);
  DeleteObject(hrgn);
}
//---------------------------------------------------------------------
template <typename T>
void addRegionObj(T& obj, HRGN& region)
{
  int nObj = obj.getElem();
  if(!nObj)
    return;
  int start = 0;
  if(!region) {
    PRect r = obj[0]->get_Rect();
    region = CreateRectRgn(r.left, r.top, r.right, r.bottom);
    ++start;
    }
  for(int i = start; i < nObj; ++i)
    createRegionObj(obj[i], region);
}
//---------------------------------------------------------------------
template <typename T>
void addRegionObjCheckBmp(T& obj, HRGN& region)
{
  int nObj = obj.getElem();
  if(!nObj)
    return;
  int start = 0;
  if(!region) {
    PSimpleBmp* bmp = dynamic_cast<PSimpleBmp*>(obj[0]);
    if(bmp) {
      PRect r = bmp->getTrueRect();
      region = CreateRectRgn(r.left, r.top, r.right, r.bottom);
      if(bmp->isTransp())
        makeTraspRegion(region, bmp);
      }
    else {
      PRect r = obj[0]->get_Rect();
      region = CreateRectRgn(r.left, r.top, r.right, r.bottom);
      }
    ++start;
    }
  for(int i = start; i < nObj; ++i) {
    PSimpleBmp* bmp = dynamic_cast<PSimpleBmp*>(obj[i]);
    if(bmp) {
      PRect r = bmp->getTrueRect();
      HRGN hrgn = CreateRectRgn(r.left, r.top, r.right, r.bottom);
      if(bmp->isTransp())
        makeTraspRegion(hrgn, bmp);
      CombineRgn(region, region, hrgn, RGN_OR);
      DeleteObject(hrgn);
      }
    else
      createRegionObj(obj[i], region);
    }
}
//----------------------------------------------------------------------------
template <class T>
void calcArea(T& set, PRect& area)
{
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i) {
    PRect r = set[i]->get_Rect();
    area |= r;
    }
}
//----------------------------------------------------------------------------
template <class T>
void calcAreaCheckBmp(T& set, PRect& area)
{
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i) {
    PSimpleBmp* bmp = dynamic_cast<PSimpleBmp*>(set[i]);
    if(bmp)
      area |= bmp->getTrueRect();
    else
      area |= set[i]->get_Rect();
    }
}
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
          Owner->pushedBtn(LOWORD(wParam) - IDC_BUTTON_F1 + ID_F1);
          result = 1;
          return true;

        case ID_SHIFT_F1:
        case ID_SHIFT_F2:
        case ID_SHIFT_F3:
        case ID_SHIFT_F4:
        case ID_SHIFT_F5:
        case ID_SHIFT_F6:
        case ID_SHIFT_F7:
        case ID_SHIFT_F8:
        case ID_SHIFT_F9:
        case ID_SHIFT_F10:
        case ID_SHIFT_F11:
        case ID_SHIFT_F12:
        case ID_CTRL_F1:
        case ID_CTRL_F2:
        case ID_CTRL_F3:
        case ID_CTRL_F4:
        case ID_CTRL_F5:
        case ID_CTRL_F6:
        case ID_CTRL_F7:
        case ID_CTRL_F8:
        case ID_CTRL_F9:
        case ID_CTRL_F10:
        case ID_CTRL_F11:
        case ID_CTRL_F12:
          Owner->pushedBtn(LOWORD(wParam));
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
bool manageModal::acceptChildPage(int idBtn)
{
  int id = idBtn + MAX_BTN;
  LPCTSTR p = Owner->getPageString(id);

  if(!p)
    return false;
  id = _ttoi(p);
  switch(id) {
    case P_DefBody::ID_OPEN_PAGE:
    case P_DefBody::ID_OPEN_TREND:
    case P_DefBody::ID_OPEN_RECIPE:
    case P_DefBody::ID_OPEN_RECIPE_ROW:
    case P_DefBody::ID_OPEN_MODELESSBODY:

    case P_DefBody::ID_GOTO_PREVIOUS_PAGE:
    case P_DefBody::ID_SHUTDOWN:
    case P_DefBody::ID_SHOW_TREEVIEW:
    case P_DefBody::ID_PRINT_TREEVIEW:
    case P_DefBody::ID_SHOW_ALARM:
//    case P_DefBody::ID_PRINT_SCREEN:
      return false;

    }
  return true;
}
//----------------------------------------------------------------------------
void manageModal::addToRegion(HRGN hrgn)
{
  ((P_ModalBody*)Owner)->addToRegion(hrgn);
}
//----------------------------------------------------------------------------
void manageModal::calcAllArea(PRect& area)
{
  ((P_ModalBody*)Owner)->calcAllArea(area);
}
//----------------------------------------------------------------------------
void manageModal::makeRegion()
{
  HRGN hrgn = 0;
  addToRegion(hrgn);
  if(hrgn) {
    SetWindowRgn(*Owner, hrgn, false);
    if(isWinXP_orLater()) {
      typedef int (WINAPI *pGetWindowRgnBox)(HWND hWnd, LPRECT lprc);
      HMODULE h = LoadLibrary(_T("USER32.DLL"));
      if(h) {
        FARPROC f = GetProcAddress(h, "GetWindowRgnBox");
        if(f) {
          PRect r;
          if(ERROR != ((pGetWindowRgnBox)f)(*Owner, r)) {
            Owner->Attr.w = r.Width();
            Owner->Attr.h = r.Height();
            }
          }
        FreeLibrary(h);
        }
      }
    }
}
//----------------------------------------------------------------------------
void manageModal::create()
{
  LPCTSTR p = Owner->getPageString(ID_NO_CAPTION_MODAL);
  int subW = -pt_offs.x;
  int subH = -pt_offs.y;
  if(p && 1 == _ttoi(p)) {
    p = findNextParamTrim(p);
    bool Transp = false;
    if(p && 1 == _ttoi(p)) {
      Transp = true;
      makeRegion();
      }
    Owner->Attr.style &= ~WS_CAPTION;
    SetWindowLongPtr(*Owner, GWL_STYLE, GetWindowLongPtr(*Owner, GWL_STYLE) & ~WS_CAPTION);
    noCaption = true;
//    if(!Transp || !isWinXP_orLater()) {
      subW = GetSystemMetrics(SM_CXBORDER) * 2;
      subH = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYBORDER) * 2;
//      }
    }

  int x = 0;
  int y = 0;
  int w = GetSystemMetrics(SM_CXSCREEN);
  int h = GetSystemMetrics(SM_CYSCREEN);
  alignWithMonitor(Owner, x, y);
  if(w < Owner->Attr.x + Owner->Attr.w - x)
    Owner->Attr.x = w - Owner->Attr.w + x;
  else if(Owner->Attr.x < 0)
    Owner->Attr.x = 0;
  if(h < Owner->Attr.y + Owner->Attr.h - y)
    Owner->Attr.y = h - Owner->Attr.h + y;
  else if(Owner->Attr.y < 0)
    Owner->Attr.y = 0;
#if true  
  MoveWindow(*Owner, Owner->Attr.x, Owner->Attr.y, Owner->Attr.w - subW, Owner->Attr.h - subH, true);
#else
  SetWindowPos(*Owner, 0, Owner->Attr.x, Owner->Attr.y, Owner->Attr.w - subW, Owner->Attr.h - subH, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
  InvalidateRect(*Owner, 0, 1);
#endif
  ShowWindow(*Owner, SW_SHOWNORMAL);
}
//----------------------------------------------------------------------------
void manageModal::getSize(int& w, int& h)
{
  int x = 0;
  int y = 0;
  LPCTSTR p = Owner->getPageString(ID_OFFSET_X);
  if(p)
    x = _ttoi(p);
  p = Owner->getPageString(ID_OFFSET_Y);
  if(p)
    y = _ttoi(p);

  PRect area(10000, 10000, 0, 0);
  calcAllArea(area);
  pt_offs.x = area.left;
  pt_offs.y = area.top;
  area.Offset(x, y);

  if(!area.Width())
    area.right = area.left + 300;
  if(!area.Height())
    area.bottom = area.top + 200;
  if(w < 0)
    w = area.Width() + pt_offs.x * 2;
  if(h < 0)
    h = area.Height() + GetSystemMetrics(SM_CYCAPTION) + pt_offs.y * 2;
}
//----------------------------------------------------------------------------
inline int center(int w, int m)
{
  return (m - w) / 2;
}
//----------------------------------------------------------------------------
void manageModal::adjustSize()
{
  int x = -1;
  int y = -1;
  int w = -1;
  int h = -1;
  LPCTSTR p = Owner->getPageString(ID_MODAL_RECT);
  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d"), &x, &y, &w, &h);
  if(x > 0)
    x = R__X(x);
  if(y > 0)
    y = R__Y(y);
  if(w > 0)
    w = R__X(w);
  if(h > 0)
    h = R__Y(h);
  if(w < 0 || h < 0)
    getSize(w, h);
  if(x < 0)
    x = center(w, sizer::getWidth());
  if(y < 0)
    y = center(h, sizer::getHeight());

  alignWithMonitor(Owner, x, y);
  Owner->Attr.x = x;
  Owner->Attr.y = y;
  Owner->Attr.w = w;
  Owner->Attr.h = h;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void manageRecipe::addToRegion(HRGN hrgn)
{
  ((P_ModalRecipe*)Owner)->addToRegion(hrgn);
}
//----------------------------------------------------------------------------
void manageRecipe::calcAllArea(PRect& area)
{
  ((P_ModalRecipe*)Owner)->calcAllArea(area);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
P_ModalBody::P_ModalBody(int /*idParent*/, PWin* parent, uint resId, HINSTANCE hInstance) :
    baseClass(0, parent, resId, hInstance), ManModal(0)
{
  ManModal = allocManModal();
}
//----------------------------------------------------------------------------
P_ModalBody::P_ModalBody(PWin* parent, LPCTSTR pageDescr) :
    baseClass(parent, pageDescr), ManModal(0)
{
  Attr.id = 0;
  Attr.style = WS_POPUP | WS_CAPTION;
  ManModal = allocManModal();
}
//----------------------------------------------------------------------------
P_ModalBody::~P_ModalBody()
{
  ManModal->setExiting();
  destroy();
  delete ManModal;
}
//----------------------------------------------------------------------------
LRESULT P_ModalBody::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT result;
  if(ManModal->windowProc(hwnd, message, wParam, lParam, result))
    return result;
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
P_Body* P_ModalBody::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F1:
      sendListValue(false);
      refresh();
      ManModal->setExiting();
      if(isFlagSet(pfModal))
        EndDialog(*this, IDOK);
      else
        DestroyWindow(*this);
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
LPCTSTR P_ModalBody::findChildPage(int idBtn, int& idChild, int& pswLevel, bool& predefinite)
{
  if(ManModal->acceptChildPage(idBtn))
    return baseClass::findChildPage(idBtn, idChild, pswLevel, predefinite);
  return 0;
}
//----------------------------------------------------------------------------
void P_ModalBody::addToRegion(HRGN hrgn)
{
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
}
//----------------------------------------------------------------------------
bool P_ModalBody::create()
{
  smartPointerConstString tit = getTitle();
  if(!baseClass::create())
    return false;
  Attr.id = IDD_MODAL_BODY;
  setReady(false);
  ManModal->create();
  SetWindowText(*this, tit);
  return true;
}
//----------------------------------------------------------------------------
void P_ModalBody::refresh()
{
  if(ManModal->canRefresh())
    baseClass::refresh();
}
//----------------------------------------------------------------------------
void P_ModalBody::calcAllArea(PRect& area)
{
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
  calcArea(Slider, area);
}
//----------------------------------------------------------------------------
void P_ModalBody::setReady(bool first)
{
  baseClass::setReady(true);
  ManModal->adjustSize();
  refresh();
}
//----------------------------------------------------------------------------
struct rememberPos
{
  LPCTSTR Page;
  POINT Pt;
  rememberPos() : Page(0) { Pt.x = 0; Pt.y = 0; }
  rememberPos(POINT pt, LPCTSTR p = 0) : Page(str_newdup(p)) { Pt = pt; }
  ~rememberPos() { delete []Page; }
  rememberPos(const rememberPos& other) : Page(0) { clone(other); }
  const rememberPos& operator=(const rememberPos& other) { clone(other); return *this; }
private:
    void clone(const rememberPos& other);
};
//----------------------------------------------------------------------------
void rememberPos::clone(const rememberPos& other)
{
  if(&other == this)
    return;
  delete []Page;
  Page = other.Page;
  rememberPos& ot = const_cast<rememberPos&>(other);
  ot.Page = 0;
  Pt = other.Pt;
}
//----------------------------------------------------------------------------
static PVect<rememberPos> RememberPos;
//----------------------------------------------------------------------------
void getRememberPos(LPCTSTR page, int& x, int& y)
{
  int nElem = RememberPos.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(!_tcsicmp(page, RememberPos[i].Page)) {
      POINT pt = RememberPos[i].Pt;
      x = pt.x;
      y = pt.y;
      break;
      }
    }
}
//----------------------------------------------------------------------------
void setRememberPos(HWND hwnd, LPCTSTR page)
{
  int nElem = RememberPos.getElem();
  int found = -1;
  for(int i = 0; i < nElem; ++i) {
    if(!_tcsicmp(page, RememberPos[i].Page)) {
      found = i;
      break;
      }
    }
  PRect r;
  GetWindowRect(hwnd, r);

  POINT pt = { r.left, r.top };

  if(found < 0) {
    rememberPos rp(pt, page);
    RememberPos[nElem] = rp;
    // non esiste costruttore di copia, quindi viene copiato il puntatore
    // però il buffer per il testo viene ditrutto all'uscita, quindi ponendolo
    // a zero resta allocato quello nel vettore
    rp.Page = 0;
    }
  else
    RememberPos[found].Pt = pt;
}
//----------------------------------------------------------------------------
P_ModelessBody::P_ModelessBody(int /*idParent*/, PWin* parent, uint resId, HINSTANCE hInstance) :
    baseClass(0, parent, resId, hInstance)
{
}
//----------------------------------------------------------------------------
P_ModelessBody::P_ModelessBody(PWin* parent, LPCTSTR pageDescr) :
    baseClass(parent, pageDescr)
{
  Attr.id = 0;
}
//----------------------------------------------------------------------------
P_ModelessBody::~P_ModelessBody()
{
  ManModal->setExiting();
  destroy();
}
//----------------------------------------------------------------------------
LRESULT P_ModelessBody::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ACTIVATE:
    case WM_POST_ACTIVATE:
      // bypassa il controllo del modal
      return P_DefBody::windowProc(hwnd, message, wParam, lParam);
    case WM_DESTROY:
      savePos(hwnd);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
P_Body* P_ModelessBody::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F1:
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
void P_ModelessBody::setReady(bool first)
{
  Attr.id = IDD_MODELESS_BODY;
  baseClass::setReady(true);
  LPCTSTR p = getPageString(ID_MODELESS_MEMO_POS);
  if(p && _ttoi(p))
    return;

  LPCTSTR page = getPageName();
  getRememberPos(page, Attr.x, Attr.y);
  refresh();
}
//----------------------------------------------------------------------------
void P_ModelessBody::savePos(HWND hwnd)
{
  LPCTSTR p = getPageString(ID_MODELESS_MEMO_POS);
  if(p && _ttoi(p))
    return;

  LPCTSTR page = getPageName();
  setRememberPos(hwnd, page);
}


//----------------------------------------------------------------------------
P_ModalRecipe::P_ModalRecipe(int /*idParent*/, PWin* parent, uint resId, HINSTANCE hInstance) :
    baseClass(0, parent, resId, hInstance), ManModal(0)
{
  Attr.style = WS_POPUP | WS_CAPTION;
  ManModal = allocManModal();
//  setFlag(pfWrapped4HWND);
}
//----------------------------------------------------------------------------
P_ModalRecipe::~P_ModalRecipe()
{
  ManModal->setExiting();
  destroy();
  delete ManModal;
}
//----------------------------------------------------------------------------
LRESULT P_ModalRecipe::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT result;
  if(ManModal->windowProc(hwnd, message, wParam, lParam, result))
    return result;
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
P_Body* P_ModalRecipe::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F1:
      sendListValue(false);
      refresh();
      ManModal->setExiting();
      if(isFlagSet(pfModal))
        EndDialog(*this, IDOK);
      else
        DestroyWindow(*this);
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
LPCTSTR P_ModalRecipe::findChildPage(int idBtn, int& idChild, int& pswLevel, bool& predefinite)
{
  if(ManModal->acceptChildPage(idBtn))
    return baseClass::findChildPage(idBtn, idChild, pswLevel, predefinite);
  return 0;
}
//----------------------------------------------------------------------------
void P_ModalRecipe::addToRegion(HRGN hrgn)
{
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
}
//----------------------------------------------------------------------------
bool P_ModalRecipe::create()
{
  if(!baseClass::create())
    return false;
  Attr.id = IDD_MODAL_BODY;
  setReady(false);
  ManModal->create();
  smartPointerConstString tit = getTitle();
  SetWindowText(*this, tit);
  return true;
}
//----------------------------------------------------------------------------
void P_ModalRecipe::refresh()
{
  if(ManModal->canRefresh())
    baseClass::refresh();
}
//----------------------------------------------------------------------------
void P_ModalRecipe::calcAllArea(PRect& area)
{
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
}
//----------------------------------------------------------------------------
void P_ModalRecipe::setReady(bool first)
{
  baseClass::setReady(true);
  ManModal->adjustSize();
  refresh();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
P_ModelessRecipe::P_ModelessRecipe(int /*idParent*/, PWin* parent, uint resId, HINSTANCE hInstance) :
    baseClass(0, parent, resId, hInstance)
{
}
//----------------------------------------------------------------------------
P_ModelessRecipe::~P_ModelessRecipe()
{
  ManModal->setExiting();
  destroy();
}
//----------------------------------------------------------------------------
LRESULT P_ModelessRecipe::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ACTIVATE:
    case WM_POST_ACTIVATE:
      // bypassa il controllo del modal
      return TD_Recipe::windowProc(hwnd, message, wParam, lParam);
    case WM_DESTROY:
      savePos(hwnd);
      break;
    }
  LRESULT result;
  if(ManModal->windowProc(hwnd, message, wParam, lParam, result))
    return result;
  return TD_Recipe::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
P_Body* P_ModelessRecipe::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F1:
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
void P_ModelessRecipe::setReady(bool first)
{
  Attr.id = IDD_MODELESS_BODY;
  baseClass::setReady(true);
  LPCTSTR p = getPageString(ID_MODELESS_MEMO_POS);
  if(p && _ttoi(p))
    return;

  LPCTSTR page = getPageName();
  getRememberPos(page, Attr.x, Attr.y);
  refresh();
}
//----------------------------------------------------------------------------
void P_ModelessRecipe::savePos(HWND hwnd)
{
  LPCTSTR p = getPageString(ID_MODELESS_MEMO_POS);
  if(p && _ttoi(p))
    return;

  LPCTSTR page = getPageName();
  setRememberPos(hwnd, page);
}
