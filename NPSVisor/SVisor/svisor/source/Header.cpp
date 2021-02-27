//----------- pheader.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <process.h>
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "header.h"
#include "ptextpanel.h"
#include "mainclient.h"

#include "sizer.h"
#include "pcrt_lck.h"
#include "1.h"
#include "p_date.h"
//-------------------------------------------------------------
class gestTime
{
  public:
    gestTime(Header *par);
    ~gestTime();
    void stop();
  private:
    Header *Par;

    HANDLE hEvent;
    DWORD idThread;

    void resetThread()  { idThread = 0; }

    typedef unsigned ( __stdcall* pfThread )( void * );

//    static DWORD FAR PASCAL gestTimeProc(LPSTR);
    static unsigned __stdcall gestTimeProc(void*);
};
//-----------------------------------------------------
#define H_HEADER (DIM_Y_LOGO + OFFS_Y_LOGO * 3)

// dimensioni del logo per schermo standard 800x600
#define DIM_X_LOGO 80
#define DIM_Y_LOGO 110
//#define DIM_Y_LOGO 106

#define OFFS_X_LOGO 10
#define OFFS_Y_LOGO 5

#define HEIGHT_FONT_TIME 23

#define Y1_TIME (OFFS_Y_LOGO + 6)
//#define X1_TIME (X2_TITLE + 2)
#define X1_TIME 786
#define W_TIME 170
#define H_TIME H_TITLE
//#define H_TIME 26
#define Y2_TIME (Y1_TIME + H_TIME)
#define X2_TIME (X1_TIME + W_TIME)

#define Y1_BOX_ALARM (Y2_TITLE + 5)
//#define Y1_BOX_ALARM (Y2_TIME + 5)
#define X1_BOX_ALARM 690
#define W_BOX_ALARM  90
#define H_BOX_ALARM  60
#define Y2_BOX_ALARM (Y1_BOX_ALARM + H_BOX_ALARM)
#define X2_BOX_ALARM (X1_BOX_ALARM + W_BOX_ALARM)

#define Y1_ALARM (Y2_BOX_ALARM - H_ALARM - 10)
#define X1_ALARM 700
#define W_ALARM 70
#define H_ALARM 26
#define Y2_ALARM (Y1_ALARM + H_ALARM)
#define X2_ALARM (X1_ALARM + W_ALARM)

#define HEIGHT_FONT_TITLE 30

#define Y1_TITLE OFFS_Y_LOGO
#define X1_TITLE 100
#define W_TITLE 460
//#define W_TITLE 480
#define H_TITLE (HEIGHT_FONT_TITLE + 5)
#define Y2_TITLE (Y1_TITLE + H_TITLE)
#define X2_TITLE (X1_TITLE + W_TITLE)


#define INIT_X_INFO X1_TITLE
#define INIT_Y_INFO (Y2_TITLE + 5)
//--------------------------------------------------------------
static void invalidate(PWin* owner, PPanel*obj);
//---------------------------------------------------------------
#define _REG(a) const UINT WM_##a = RegisterWindowMessage(_T("WM_") _T(#a))

  _REG(SV_HEADER_MANAGE_TIME);

  #define EV_TIMER 1
  #define EV_ALARM 2
//---------------------------------------------------------------
Header::Header(PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), time(0), alarm(0), boxAlarm(0),
    statAlarm(0), title(0), gt(0),
    FontTitle(0), FontTime(0), Hided(false)
{
  isBody = false;
}
//--------------------------------------------------------------
Header::~Header()
{
  destroy();

  // INSERT>> Your destructor code here.
  delete gt;
  delete time;
  delete alarm;
  delete title;
  delete boxAlarm;
  if(FontTitle)
    DeleteObject(FontTitle);
  if(FontTime)
    DeleteObject(FontTime);
}
//----------------------------------------------------------------------------
//--------------------------------------------------------------------
bool Header::create()
{
  if(!baseClass::create())
    return false;
  LPCTSTR p = ::getString(ID_HIDE_PART);
  bool hided = false;
  if(p) {
    int code = _ttoi(p);
    if(code & 1)
      hided = true;
    }
  ShowWindow(*this, hided ? SW_HIDE : SW_SHOWNORMAL);
  Hided = hided;
  if(hided) {
    p =  ::getString(ID_HIDE_TIME);
    if(!p || !_ttoi(p))
      hided = false;
    }

  if(!hided)
    gt = new gestTime(this);

  makeAll();
  return true;
}
//--------------------------------------------------------------
static void formatTime(LPTSTR buff, SYSTEMTIME& Time)
{
  smartPointerConstString month(getStringByLangGlob(ID_FIRST_MONTH + Time.wMonth - 1));
  switch(whichData()) {
    case American:
      wsprintf(buff, _T("%s %02d %04d  %02d:%02d:%02d"),
        &month ? &month : _T("???"), Time.wDay, Time.wYear,
        Time.wHour, Time.wMinute, Time.wSecond);
      break;
    case Other:
      wsprintf(buff, _T("%04d %s %02d  %02d:%02d:%02d"),
        Time.wYear, &month ? &month : _T("???"), Time.wDay,
        Time.wHour, Time.wMinute, Time.wSecond);
      break;
    default:
      wsprintf(buff, _T("%02d %s %04d  %02d:%02d:%02d"),
        Time.wDay, &month ? &month : _T("???"), Time.wYear,
        Time.wHour, Time.wMinute, Time.wSecond);
      break;
    }
}
//--------------------------------------------------------------
void Header::refreshAlarm(HDC hdc)
{
  if(statAlarm > 0) {
    --statAlarm;
    if(statAlarm & 1) {
      if(gestPerif::sAlarm == Which)
        alarm->setBkgColor(RGB(0xff, 0, 0));
      else
        alarm->setBkgColor(RGB(0xff, 0xff, 0));
      }
    else
      alarm->setBkgColor(GetSysColor(COLOR_BTNFACE));
    alarm->draw(hdc);
    }
  const perifsSet *p = Par->getPerifs();
  gestPerif::statAlarm al = p->hasAlarm();
  if(gestPerif::sNoAlarm == al) {
    if(statAlarm > 0)
      resetAlarm();
    }
  else {
    if(statAlarm > 0)
      Which = al;
    else
      setAlarm(al);
    }
}
//--------------------------------------------------------------------
bool Header::getCacheFile(LPTSTR target, uint sz)
{
  target[0] = 0;
  return false;
}
//--------------------------------------------------------------
void Header::refresh()
{
  baseClass::refresh();
}
//----------------------------------------------------------------------------
LRESULT Header::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_SV_HEADER_MANAGE_TIME == message) {
    switch(LOWORD(wParam)) {
      case EV_TIMER:
        if(toBool(HIWORD(wParam))) {
					P_Body* bd = getCurrBody();
					if(bd)
          	InvalidateRect(*bd, time->getRect(), 0);
					}
        else
          InvalidateRect(*this, time->getRect(), 0);
        break;
      case EV_ALARM:
        do {
          HDC hdc = GetDC(*this);
          refreshAlarm(hdc);
          ReleaseDC(*this, hdc);
          } while(false);
        break;
      }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//--------------------------------------------------------------
static void invalidate(PWin* owner, PPanel*obj)
{
  InvalidateRect(*owner, obj->getRect(), 0);
}
//--------------------------------------------------------------
static void paintIf(HDC hdc, PPanel*obj, const PRect& rect)
{
  if(rect.Intersec(obj->getRect()))
    obj->draw(hdc);
}
//--------------------------------------------------------------
void Header::evPaint(HDC hdc, const PRect& rect)
{
  baseClass::evPaint(hdc, rect);

  paintIf(hdc, title, rect);
  paintIf(hdc, boxAlarm, rect);
  paintIf(hdc, alarm, rect);
  paintIf(hdc, time, rect);

  SIZE szOld;
  sizer::eSizer currSize = sizer::getDefault(szOld);
  sizer::setDefault(sizer::s800x600);

  SIZE sz = { R__X(DIM_X_LOGO), R__Y(DIM_Y_LOGO) };
  POINT pt = { R__X(OFFS_X_LOGO), R__Y(OFFS_Y_LOGO) };

  int id = sizer::getWidth() < DEF_X_SCREEN ?
            IDB_BITMAP_LOGO640x480 : IDB_BITMAP_LOGO;

  PBitmap Logo(id, getHInstance());
  PRect r(pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy);
  if(r.Intersec(rect))
    Logo.draw(hdc, pt, sz);

  sizer::setDefault(currSize, &szOld);
}
//--------------------------------------------------------------
int Header::trueHeight()
{
  SIZE szOld;
  sizer::eSizer currSize = sizer::getDefault(szOld);
  sizer::setDefault(sizer::s800x600);
  int h = R__Y(H_HEADER);
  sizer::setDefault(currSize, &szOld);
  return h;
}
//--------------------------------------------------------------
int Header::Height()
{
  LPCTSTR p = getString(ID_HIDE_PART);
  int hidePart = 0;
  if(p)
    hidePart = _ttoi(p);
  if(hidePart & 1)
    return 0;
  return trueHeight();
}
//--------------------------------------------------------------
void Header::remakeByLang()
{
  resetCache();
  sStr.reset();
  removeAllBmpWork(2);
  setReady(true);
  makeAll();
  InvalidateRect(*this, 0, 0);
}
//--------------------------------------------------------------
void Header::makeAll()
{

  SIZE szOld;
  sizer::eSizer currSize = sizer::getDefault(szOld);
  sizer::setDefault(sizer::s800x600);

//  #define USE_BOLD fBOLD
  #define USE_BOLD 0

  if(!FontTitle)
    FontTitle = D_FONT(R__Y(HEIGHT_FONT_TITLE), 0, USE_BOLD, _T("times new roman"));
  if(!FontTime)
    FontTime = D_FONT(R__Y(HEIGHT_FONT_TIME), 0, 0, _T("arial"));

  if(!time) {
    time = new PTextAutoPanel(_T(" "), R__X(X1_TIME), R__Y(Y1_TIME),
        FontTime, RGB(0, 0, 0xff), PPanel::DN_FILL, GetSysColor(COLOR_BTNFACE),
        PTextPanel::All);
    time->setAlign(TA_TOP | TA_RIGHT);
    }

  if(!alarm) {
    PRect rect(R__X(X1_ALARM), R__Y(Y1_ALARM), R__X(X2_ALARM), R__Y(Y2_ALARM));
    alarm = new PPanel(rect, GetSysColor(COLOR_BTNFACE), PPanel::UP_FILL);
    }

  if(!title) {
    PRect tit(R__X(X1_TITLE), R__Y(Y1_TITLE), R__X(X2_TITLE), R__Y(Y2_TITLE));
    title = new PTextFixedPanel(0, tit, FontTitle, RGB(0, 0, 0xff),
                PPanel::DN_FILL, RGB(0xcf, 0xef, 0xff), PTextPanel::All);
    title->setAlign(TA_TOP | TA_CENTER);
    }

  smartPointerConstString p(getStringOrIdByLangGlob(ID_ALARM));
  if(!boxAlarm) {
    PRect rectbx(R__X(X1_BOX_ALARM), R__Y(Y1_BOX_ALARM), R__X(X2_BOX_ALARM), R__Y(Y2_BOX_ALARM));
    boxAlarm = new PTextFixedPanel(p, rectbx, FontTime, RGB(0xff, 0, 0),
                  PPanel::DN, GetSysColor(COLOR_BTNFACE), PTextPanel::All);
    boxAlarm->setAlign(TA_TOP | TA_CENTER);
    }
  else
    boxAlarm->setText(p);

  sizer::setDefault(currSize);
}
//--------------------------------------------------------------
void Header::setTitle(LPCTSTR tit)
{
  makeAll();
  title->setText(tit);
  invalidate(this, title);
//  title->draw2(*this);
}
//--------------------------------------------------------------
void Header::setAlarm(gestPerif::statAlarm which, unsigned repeat)
{
  statAlarm = repeat;
  Which = which;
}
//---------------------------------------------------------------
void Header::resetAlarm()
{
  statAlarm = 1;
}
//---------------------------------------------------------------
//---------------------------------------------------------------
gestTime::gestTime(Header *par) :  Par(par), hEvent(0),  idThread(0)
{
#if 0
 HANDLE hThread = (HANDLE)_beginthreadex(0, 0, (pfThread)gestTimeProc, this, 0, (unsigned*) &idThread);
#else
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)gestTimeProc,
                  this, 0, &idThread);
#endif

  if(hThread)
    CloseHandle(hThread);
}
//-------------------------------------------------------------------------
gestTime::~gestTime()
{
  stop();
}
//----------------------------------------------------------------------------
void gestTime::stop()
{
  while(idThread) {
    if(hEvent)
    // segnala al thread di chiudere
      SetEvent(hEvent);
    Sleep(100);
    }
}
//----------------------------------------------------------------------------
#define TIME_FOR_TIME 500
//#define TIME_FOR_TIME 1000
//DWORD FAR PASCAL gestTime::gestTimeProc(LPSTR cl)
unsigned __stdcall gestTime::gestTimeProc(void* cl)
{
  gestTime *pGest = reinterpret_cast<gestTime*>(cl);
  pGest->hEvent = CreateEvent( 0, true, false, 0);
    // se non riesce a creare l'evento
  if(!pGest->hEvent) {
    pGest->resetThread();
    return true;
    }
  bool toggled = true;
  while(true) {

  // attesa di segnalazione da parte della classe o timeout
    DWORD result = WaitForSingleObject(pGest->hEvent, TIME_FOR_TIME);
    if(WAIT_TIMEOUT != result)
      break;
    if(pGest->Par->getHandle()) {
      bool hided = pGest->Par->isHided();
      if(toggled) {
        SYSTEMTIME newTime;
        GetLocalTime(&newTime);
        TCHAR buff[40];
        formatTime(buff, newTime);
        pGest->Par->time->setText(buff);
        if(hided) {
          mainClient* mc = getMain();
          PTextAutoPanel* time = mc->getTimeBox();
          if(time)
            PostMessage(pGest->Par->getHandle(), WM_SV_HEADER_MANAGE_TIME, MAKEWPARAM(EV_TIMER, hided), 0);
          }
        else
          PostMessage(pGest->Par->getHandle(), WM_SV_HEADER_MANAGE_TIME, MAKEWPARAM(EV_TIMER, hided), 0);
        }
      if(!hided)
        PostMessage(pGest->Par->getHandle(), WM_SV_HEADER_MANAGE_TIME, MAKEWPARAM(EV_ALARM, 0), 0);
      toggled = !toggled;

      }
    }
  CloseHandle(pGest->hEvent);
  pGest->hEvent = 0;
  pGest->resetThread();
  return true;
}

