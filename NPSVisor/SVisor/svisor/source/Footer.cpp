//--------- footer.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "footer.h"
#include "p_body.h"
#include "p_date.h"
#include "pstatic.h"
#include "pTextpanel.h"
#include "sizer.h"
#include "p_util.h"
#include "1.h"
//--------------------------------------------------------------------
// se si deriva da footer una nuova classe occorre modificare questa
Footer *allocFooter(mainClient *parent)
{
  Footer *ft = new Footer(parent);
  return ft;
}
//--------------------------------------------------------------------
Footer::Footer(mainClient* parent, uint resId, HINSTANCE hinstance)
:
    P_Base(parent, resId, hinstance), H_Txt(0), showAlert(0), Font(0),
    FontStat(0)
{
  Attr.style |= WS_VISIBLE | WS_CHILD;
  isBody = false;
  makeAll();
}
//--------------------------------------------------------------------
Footer::~Footer()
{
  destroy();

  // INSERT>> Your destructor code here.
  for(int i=0; i < MAX_BTN; ++i)
    delete Txt[i];
  delete PStatus;
  delete PVersion;
  if(Font)
    DeleteObject(Font);
  if(FontStat)
    DeleteObject(FontStat);
}
//--------------------------------------------------------------------
bool Footer::create()
{
  if(!P_Base::create())
    return false;
  for(int i = 0; i < MAX_BTN; ++i)
    Btn[i]->setFont(FontStat);
  LPCTSTR p = ::getString(ID_HIDE_PART);
  bool hided = false;
  if(p) {
    int code = _ttoi(p);
    if(code & 2)
      hided = true;
    }
  ShowWindow(*this, hided ? SW_HIDE : SW_SHOWNORMAL);
  return true;
}
//--------------------------------------------------------------------
void Footer::BnClicked(int btn)
{
  Par->pushBtn(btn);
}
//------------------------------------
#define DELTA 5
#define DELTA_X 3
#define Y_TXT 5
#define Y_BTN (Y_TXT *2 + H_Txt)
#define Y_STAT (Y_BTN + H_Btn + Y_TXT/2)
//--------------------------------------------------------------------
#define DIM_H_FONT 16
#define DIV_FONT (DEF_Y_SCREEN / DIM_H_FONT + 0.5)
#define DEF_H_BTN 20
#define REAL_H_BTN ((sizer::getHeight() / 600.0) * DEF_H_BTN)
//----------------------------------
#define NAME_FONT _T("arial")
void Footer::makeAll()
{

  Height();

  SIZE szOld;
  sizer::eSizer currSize = sizer::getDefault(szOld);
  sizer::setDefault(sizer::s800x600);

  int l = sizer::getWidth() - DELTA * 2;
  int step = l / MAX_BTN;
  int dx = (l - step * MAX_BTN) / 2;

#if 0
  Font = CreateFont(R__Y(DIM_H_FONT), R__Y(DIM_H_FONT) / 3, 0, 0, FW_NORMAL, 0, 0, 0,
          ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
          DEFAULT_QUALITY, FF_SWISS, NAME_FONT);

  FontStat = CreateFont(R__Y(DIM_H_FONT), 0, 0, 0, FW_NORMAL, 0, 0, 0,
          ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
          DEFAULT_QUALITY, FF_SWISS, NAME_FONT);
#else
  Font = D_FONT(R__Y(DIM_H_FONT), R__Y(DIM_H_FONT) / 3, 0, NAME_FONT);

  FontStat = D_FONT(R__Y(DIM_H_FONT), 0, 0, NAME_FONT);
#endif

  int x1 = DELTA + dx;
  int x2 = x1 + step - DELTA_X;

  PRect rTxt(x1, Y_TXT, x2, Y_TXT + H_Txt);
  PRect rBtn(x1, Y_BTN, x2, Y_BTN + H_Btn);

  for(int i = 0; i < MAX_BTN; ++i) {
    Txt[i] = new PTextFixedPanel(_T(" "), rTxt, Font, RGB(0, 0, 0xff), PPanel::DN_FILL);
    Txt[i]->setAlign(TA_CENTER);

    TCHAR f[10];
    wsprintf(f, _T("F%d"), i + 1);
    Btn[i] = new PButton(this, IDC_BUTTON_F1 + i, rBtn, f);
    Btn[i]->setFont(FontStat);
    rTxt.Offset(step,0);
    rBtn.Offset(step,0);
    }
  PRect rStat(DELTA+dx,Y_STAT,rBtn.right-step,Y_STAT + H_Stat);
  PRect rVer(rStat);
  rStat.right -= R__Y(150);
  rVer.left = rStat.right + 2;
  PStatus = new PTextFixedPanel(_T(" "), rStat, FontStat, RGB(0, 0, 0), PPanel::DN_FILL);
  PVersion = new PTextFixedPanel(_T(" "), rVer, FontStat, RGB(0, 0, 0), PPanel::DN_FILL);
  PVersion->setAlign(TA_CENTER);

  sizer::setDefault(currSize, &szOld);
}
//--------------------------------------------------------------------
int Footer::trueHeight()
{
  SIZE szOld;
  sizer::eSizer currSize = sizer::getDefault(szOld);
  sizer::setDefault(sizer::s800x600);

  double ratio = sizer::getHeight();
  ratio /= DEF_Y_SCREEN;

  int H_Row = (int)(DIM_H_FONT * ratio);
  H_Txt = (int)(H_Row * 3.5);
  H_Btn = (int)REAL_H_BTN;
  H_Stat = (int)(H_Row * 1.5);

  int h = H_Txt + H_Btn + H_Stat;
  h = (int)( h + ratio * Y_TXT * 4);

  sizer::setDefault(currSize, &szOld);

  return h;
}
//--------------------------------------------------------------------
int Footer::Height()
{
  int h = trueHeight();

  LPCTSTR p = getString(ID_HIDE_PART);
  int hidePart = 0;
  if(p)
    hidePart = _ttoi(p);
  if(hidePart & 2)
    return 0;
  return h;
}
//--------------------------------------------------------------------
void Footer::setVisual(forceVis type)
{
  ForceVis = type;
}
//--------------------------------------------------------------------
void Footer::setInfoBtn(setOfString& set, long idInit)
//void Footer::setInfoBtn(const long info[MAX_BTN])
{
  for(int i = 0; i < MAX_BTN; ++i) {
#if 1
  smartPointerConstString p(getStringByLang(set, idInit + i));
//  smartPointerConstString p = getStringByLangGlob(idInit + i);
#else
    LPCTSTR p = set.getString(idInit + i);
#endif
    if(!p) {
      Txt[i]->setText(_T(" "));
      ::EnableWindow(*Btn[i], !i);
      }
    else {
      Txt[i]->setText(p);
      ::EnableWindow(*Btn[i], true);
      }
    }
  makeRefresh(rInfo);
}
//--------------------------------------------------------------------
void Footer::setNewText(int idBtn, LPCTSTR newText)
{
  idBtn -= ID_F1;
  if(static_cast<unsigned>(idBtn) < MAX_BTN) {
    Txt[idBtn]->setText(newText);
    ::EnableWindow(*Btn[idBtn], *newText);
    }
}
//--------------------------------------------------------------------
void Footer::setMsgStatus(LPCTSTR status, int wait)
{
  PStatus->setText(status);
  makeRefresh(rStatus);
  int len = _tcslen(status);
  if(!len || (1 == len && _T(' ') == status[0]))
    wait = 1;
  showAlert = wait;
}
//--------------------------------------------------------------------
void Footer::setVersion(DWORD applVersion, DWORD bodyVersion)
{
  TCHAR buff[100];
  int internal = LOWORD(applVersion);
  int appl = HIWORD(applVersion);
  wsprintf(buff, _T("Ver. %d.%d.%d.%d - %d.%d"),
        HIBYTE(internal),
        LOBYTE(internal),
        HIBYTE(appl),
        LOBYTE(appl),
        HIBYTE(bodyVersion),
        LOBYTE(bodyVersion));
  PVersion->setText(buff);
  PVersion->draw2(getHandle());
}
//--------------------------------------------------------------------
void Footer::makeRefresh(tRefresh type)
{
  if(!IsWindowVisible(*this))
    return;
  if(rStatus == type)
    PStatus->draw2(getHandle());
  else
    for(int i = 0; i < MAX_BTN; ++i)
      Txt[i]->draw2(getHandle());
}
//--------------------------------------------------------------------
void Footer::evPaint(HDC hdc)
{
  for(int i = 0; i < MAX_BTN; ++i)
    Txt[i]->draw(hdc);
  PStatus->draw(hdc);
  PVersion->draw(hdc);
}
//--------------------------------------------------------------------
void Footer::refresh()
{
  if(!IsWindowVisible(*this))
    return;
  static bool status;
  if(showAlert > 0) {
    --showAlert;
    if(!showAlert) {
      PStatus->setBkgColor(GetSysColor(COLOR_BTNFACE));
      PStatus->setText(_T(" "));
      }
    else {
      if(status)
        PStatus->setBkgColor(GetSysColor(COLOR_BTNFACE));
      else
        PStatus->setBkgColor(GetSysColor(COLOR_BTNSHADOW));
      }
    PStatus->draw2(getHandle());
    status = !status;
    }

}
//--------------------------------------------------------------------
#define MIN_TIME 3000000L
#define MAX_TIME (MIN_TIME * 2)
//--------------------------------------------------------------------
bool Footer::idle(DWORD count)
{
/*
  if(!IsWindowVisible(*this))
    return P_Base::idle(count);

  static FILETIME last;
  static bool first = true;
  HWND hfocus = GetFocus();
  if(hfocus) {
    PWin* firstChild = getFirstChild();
    PWin* child = firstChild;
    if(getHandle() == hfocus)
      child = this;
    if(child)
      do {
        if(child->getHandle() == hfocus) {
          if(first) {
            last = getFileTimeCurr();
            first = false;
            break;
            }
          P_Body* body = getCurrBody();
          if(body) {
            PWin* b = body->getFirstChild();
            if(b) {
              FILETIME curr = getFileTimeCurr();
              __int64 diff = cMK_I64(curr) - cMK_I64(last);
              if(diff > MIN_TIME) {
                if(diff < MAX_TIME)
                  SetFocus(*b);
                first = true;
                }
              }
            }
          break;
          }
        child = child->Next();
        } while(child && child != firstChild);
    }
*/
  return P_Base::idle(count);
}
//--------------------------------------------------------------------
void Footer::enableBtn(int idBtn, bool enable)
{
  idBtn -= ID_F1;
  if(static_cast<unsigned>(idBtn) < MAX_BTN)
    ::EnableWindow(*Btn[idBtn], enable);
}
//--------------------------------------------------------------------
bool Footer::isEnabledBtn(int idBtn)
{
  idBtn -= ID_F1;
  if(static_cast<unsigned>(idBtn) < MAX_BTN)
    return toBool(IsWindowEnabled(*Btn[idBtn]));
  return false;
}
//--------------------------------------------------------------------
LRESULT Footer::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT:
      if(GetUpdateRect(hwnd, 0, 0)) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(*this, &ps);
        if(!hdc) {
          EndPaint(*this, &ps);
          break;
          }

#define USE_MEMDC
#ifdef USE_MEMDC

#define DEF_COLOR_BKG GetSysColor(COLOR_BTNFACE)
//#define DEF_COLOR_BKG RGB(192, 192, 192)
        PRect r;
        GetClientRect(*this, r);
        HBITMAP hBmpTmp = CreateCompatibleBitmap(hdc, r.Width(), r.Height());

        HDC mdc = CreateCompatibleDC(hdc);
        HGDIOBJ oldObj = SelectObject(mdc, hBmpTmp);

        HBRUSH br = CreateSolidBrush(DEF_COLOR_BKG);
        FillRect(mdc, r, br);
        DeleteObject(HGDIOBJ(br));

        evPaint(mdc);

        BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdc, 0, 0, SRCCOPY);
        SelectObject(mdc, oldObj);

        DeleteDC(mdc);

        DeleteObject(hBmpTmp);

#else
        evPaint(hdc);
#endif
        EndPaint(*this, &ps);
        } while(false);
      return 0;
      }

  LRESULT result = P_Base::windowProc( hwnd, message, wParam, lParam);

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
          BnClicked(wParam - IDC_BUTTON_F1 + ID_F1);
          break;
        case IDC_BUTTON_CTRL_F1:
          BnClicked(ID_FZ_SETUP);
          break;
        }
      }
  return result;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------

