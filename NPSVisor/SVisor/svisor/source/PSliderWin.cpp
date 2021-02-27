//-------- PSliderWin.cpp --------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <Winuser.h>
#include "PSliderWin.h"
#include "PButton.h"
#include "p_Util.h"
#include "PShowValueWin.h"
//-------------------------------------------------------------------
class myBtn : public PButton
{
  private:
    typedef PButton baseClass;
  public:
    myBtn(PSliderWin* parent, uint id, int x, int y, int w, int h, int dir, PSliderWin::whichDrag set, LPCTSTR text = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, x, y, w, h, text, hinst), Dir(dir), Set(set), P(parent), delay(400) {}
    ~myBtn() { destroy(); }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    PSliderWin* P;
    int Dir;
    PSliderWin::whichDrag Set;
    DWORD delay;
    void sendTrackEv(DWORD flag, DWORD time);
};
//----------------------------------------------------------------------------
#define LF_TOP 1
#define W_BTN 50
#define H_BTN 20

#define OFFS_X(n) (W_BTN * (n) + LF_TOP)
#define OFFS_Y(n) (H_BTN * (n) + LF_TOP)
//----------------------------------------------------------------------------
PSliderWin::PSliderWin(PWin * parent, uint id, const PRect& r, PBitmap* bkg, const PVect<infoVar>& varName,
            const infoTrend& iTrend, DWORD bitShow, LPCTSTR title, HINSTANCE hinst) :
      baseClass(parent, 0, r, title, hinst),
      Bkg(bkg), Left(0), Right(0), onDragging(dNone),
      onAnchor(dNone), ChildPop(0), LineShow(iTrend.LineShow)
{
  Attr.style |= WS_POPUP;
  PRect r2(r);
  int cx = GetSystemMetrics(SM_CXSCREEN);
  int cy = GetSystemMetrics(SM_CYSCREEN);

  int diff = cx - r.right;
  if(diff > cy - r.bottom)
    r2.MoveTo(r.right, r.top);
  else
    r2.MoveTo(r.left, r.bottom);
  PButton* ok = new PButton(this, IDC_BUTTON_F1, LF_TOP, LF_TOP, W_BTN, H_BTN, _T("F1-Ok"));
  ok->Attr.style |= BS_DEFPUSHBUTTON;

  PRect r3(r);
  r3.MoveTo(0, 0);
  new myBtn(this, IDC_BUTTON_F3, OFFS_X(0), r3.bottom - OFFS_Y(1), W_BTN, H_BTN, -1, dLeft, _T("<< F3"));
  new myBtn(this, IDC_BUTTON_F4, OFFS_X(1), r3.bottom - OFFS_Y(1), W_BTN, H_BTN, 1, dLeft, _T("F4 >>"));

  new myBtn(this, IDC_BUTTON_F5, r3.right - OFFS_X(2), r3.bottom - OFFS_Y(1), W_BTN, H_BTN, -1, dRight, _T("<< F5"));
  new myBtn(this, IDC_BUTTON_F6, r3.right - OFFS_X(1), r3.bottom - OFFS_Y(1), W_BTN, H_BTN, 1, dRight, _T("F6 >>"));

  ChildPop = new PShowValueWin(this, 0, varName, iTrend, bitShow, r2);
}
//----------------------------------------------------------------------------
PSliderWin::~PSliderWin ()
{
  destroy();
}
//----------------------------------------------------------------------------
bool PSliderWin::create()
{
  if(!baseClass::create())
    return false;
  fillAndSet(Left, dLeft);
  fillAndSet(Right, dRight);
  SetFocus(*this);
  return true;
}
//----------------------------------------------------------------------------
void PSliderWin::setLimits(int left, int right)
{
  Right = right;
  Left = left;
}
//----------------------------------------------------------------------------
void PSliderWin::movePix(int dir, whichDrag set, bool refresh)
{
  onDragging = set;
  int x = dLeft == set ? Left : Right;
  evKeyMove(x + dir);
  if(refresh) {
    onDragging = dNone;
    InvalidateRect(*this, 0, 0);
    }
}
//-----------------------------------------------------------
bool PSliderWin::preProcessMsg(MSG& msg)
{
  if(WM_KEYDOWN == msg.message) {
    switch(msg.wParam) {
      case VK_F1:
      case VK_ESCAPE:
        DestroyWindow(*this);
        break;
      case VK_F3:
        movePix(-1, dLeft);
        break;
      case VK_F4:
        movePix(1, dLeft);
        break;
      case VK_F5:
        movePix(-1, dRight);
        break;
      case VK_F6:
        movePix(1, dRight);
        break;
      }
    }
  else if(WM_KEYUP == msg.message) {
    switch(msg.wParam) {
      case VK_F3:
      case VK_F4:
      case VK_F5:
      case VK_F6:
        onDragging = dNone;
        InvalidateRect(*this, 0, 0);
        break;
      }
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
LRESULT PSliderWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
          DestroyWindow(*this);
          break;
        }
      break;

    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
      evMouse(message, wParam, lParam);
      break;

    case WM_ERASEBKGND:
      evPaintBkg((HDC)wParam);
      return 1;

    case WM_PAINT:
      if(GetUpdateRect(hwnd, 0, 0)) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        if(hdc)
          evPaint(hdc, toBool(ps.fErase));
        EndPaint(*this, &ps);
        } while(false);
      return 0;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
bool PSliderWin::isOverObject(int curr, const POINT& pt)
{
  return pt.x >= curr - 1 && pt.x <= curr + 1;
}
//----------------------------------------------------------------------------
void PSliderWin::evMouse(UINT message, WPARAM wParam, LPARAM lParam)
{
  POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
  switch(message) {
    case WM_MOUSEMOVE:
      evMouseMove(pt);
      SetFocus(*this);
      break;

    case WM_LBUTTONDOWN:
      evMouseLDown(pt);
      break;

    case WM_LBUTTONUP:
      evMouseLUp(pt);
      break;
    }
}
//----------------------------------------------------------------------------
void PSliderWin::setPointer(uint type)
{
  switch(type) {
    case dNone:
      SetCursor(LoadCursor(NULL, IDC_ARROW));
      break;
    case dRight:
      SetCursor(LoadCursor(getHInstance(), MAKEINTRESOURCE(IDC_CURS_SLIDER_DX)));
      break;
    case dLeft:
      SetCursor(LoadCursor(getHInstance(), MAKEINTRESOURCE(IDC_CURS_SLIDER_SX)));
      break;
    }
  onAnchor = type;
}
//-----------------------------------------------------------
void PSliderWin::evMouseMove(const POINT& pt)
{
  if(dNone == onDragging) {
    if(isOverObject(Left, pt))
      setPointer(dLeft);
    else if(isOverObject(Right, pt))
      setPointer(dRight);
    else if(onAnchor != dNone)
      setPointer(dNone);
    return;
    }

  setPointer(onAnchor);
  switch(onDragging) {
    case dRight:
      if(pt.x <= Left)
        return;
      do {
        PRect r;
        GetClientRect(*this, r);
        if(pt.x >= r.right)
          return;
        } while(false);
      paintSlider(Right);
      Right = pt.x;
      break;
    case dLeft:
      if(pt.x >= Right || pt.x <= 0)
        return;
      paintSlider(Left);
      Left = pt.x;
      break;
    }
  paintSlider(pt.x);

  fillAndSet(pt.x, onDragging);
}
//-----------------------------------------------------------
void PSliderWin::evKeyMove(int x)
{
  switch(onDragging) {
    case dRight:
      if(x <= Left)
        return;
      do {
        PRect r;
        GetClientRect(*this, r);
        if(x >= r.right)
          return;
        } while(false);
      paintSlider(Right);
      Right = x;
      break;
    case dLeft:
      if(x >= Right || x <= 0)
        return;
      paintSlider(Left);
      Left = x;
      break;
    }
  paintSlider(x);
  fillAndSet(x, onDragging);
}
//-----------------------------------------------------------
void PSliderWin::fillAndSet(int x, DWORD which_drag)
{
  PVarTrend* p = getParentWin<PVarTrend>(this);
  freeListPacket& list = getFreePacket();
  PacketInfo* packet = list.get();
//  packet->clear();
  *(LPDWORD)(packet->Value) = x;
  p->fillPacket(packet);
  packet->Side = dLeft == which_drag ? PacketInfo::epi_left : PacketInfo::epi_right;
  if(ChildPop)
    ChildPop->setInfo(packet);
  else
    list.release(packet);
}
//-----------------------------------------------------------
void PSliderWin::evMouseLDown(const POINT& pt)
{
  if(onAnchor == dNone)
    return;

  SetCapture(*this);
  onDragging = onAnchor;
  setPointer(onAnchor);
}
//-----------------------------------------------------------
void PSliderWin::evMouseLUp(const POINT& pt)
{
  if(onAnchor == dNone)
    return;

  evMouseMove(pt);
  InvalidateRect(*this, 0, 0);
  ReleaseCapture();
  onDragging = dNone;
}
//----------------------------------------------------------------------------
#define GRAY(a) RGB((a),(a),(a))
#define C1 GRAY(0xff)
#define C2 GRAY(0xf0)
//#define C2 GRAY(0x3f)
#define C3 GRAY(0xf)
//----------------------------------------------------------------------------
void PSliderWin::paintSlider(int x, HDC hDc)
{
  HDC hdc = hDc ? hDc : GetDC(*this);

  if(!hDc) {
    int old = SetROP2(hdc, R2_XORPEN);
    HPEN pen1 = CreatePen(PS_SOLID, 1, C1);
    HPEN pen2 = CreatePen(PS_SOLID, 3, C2);
    HPEN pen3 = CreatePen(PS_SOLID, 5, C3);
    HGDIOBJ oldPen = SelectObject(hdc, pen3);
    PRect r;
    GetClientRect(*this, r);
    MoveToEx(hdc, x, r.top, 0);
    LineTo(hdc, x, r.bottom);
    SelectObject(hdc, pen2);
    MoveToEx(hdc, x, r.top, 0);
    LineTo(hdc, x, r.bottom);
    SelectObject(hdc, pen1);
    MoveToEx(hdc, x, r.top, 0);
    LineTo(hdc, x, r.bottom);
    SelectObject(hdc, oldPen);
    DeleteObject(pen3);
    DeleteObject(pen2);
    DeleteObject(pen1);
    SetROP2(hdc, old);
    ReleaseDC(*this, hdc);
    }
  else {
    HPEN pen = CreatePen(PS_SOLID, 1, LineShow);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    PRect r;
    GetClientRect(*this, r);
    MoveToEx(hdc, x, r.top, 0);
    LineTo(hdc, x, r.bottom);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
    }
}
//----------------------------------------------------------------------------
void PSliderWin::evPaintBkg(HDC hdc)
{
  POINT pt = { 0, 0 };
  Bkg->draw(hdc, pt);
}
//----------------------------------------------------------------------------
void PSliderWin::evPaint(HDC hdc, bool eraseBkg)
{
  POINT pt = { 0, 0 };
  if(!eraseBkg) {
    POINT pt = { 0, 0 };
    Bkg->draw(hdc, pt);
    }
  paintSlider(Left, hdc);
  paintSlider(Right, hdc);
}
//----------------------------------------------------------------------------
void myBtn::sendTrackEv(DWORD flag, DWORD time)
{
  TRACKMOUSEEVENT tme;
  tme.cbSize = sizeof(tme);
  tme.dwFlags = flag;
  tme.hwndTrack = *this;
  tme.dwHoverTime = time;
  _TrackMouseEvent(&tme);
}
//----------------------------------------------------------------------------
LRESULT myBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      sendTrackEv(TME_CANCEL | TME_HOVER, delay);
      P->onDragging = PSliderWin::dNone;
      InvalidateRect(*P, 0, 0);
      break;
    case WM_LBUTTONDOWN:
      delay = 500;
      // fall through
    case WM_MOUSEHOVER:
      if(MK_LBUTTON & wParam) {
        P->movePix(Dir, Set);
        sendTrackEv(TME_HOVER, delay);
        if(delay > 10)
          delay >>= 1;
        if(WM_LBUTTONDOWN == message)
          break;
        return 0;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
