//--------- d_mntnce.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"

#include <stdlib.h>
//#include <stdio.h>

#include "lnk_body.h"
#include "id_btn.h"
#include "sizer.h"
#include "p_Util.h"
#include "1.h"
#include "d_mntnce.h"
#include "file_lck.h"
#include "p_date.h"
#include "def_dir.h"

#include "dsettime.h"

//#define NAME_STR _T("pageMaint.txt")
#define NAME_STR getPageName()

//#define TRACE
#ifdef TRACE
  #define __MSG__ \
    { TCHAR buff[255]; \
      wsprintf(buff, _T("linea %d"), __LINE__);\
      MessageBox(_T("Lock"), buff); \
      }
#else
  #define __MSG__ ;
#endif
// definizioni per stringhe da caricare
#define _M(a) (idInitMenu + (a) - 1)

//---------------------------------------------------------------
P_Body *getManut(int idPar, PWin *parent)
{
  return new TD_Maintenance(idPar, parent);
}
//-------------------------------------------------------------------------
class listBoxMaint : public PVarListBox
{
  public:
    listBoxMaint(int alert, int warn, P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen = 255, HINSTANCE hinst = 0);
    bool hasFont() const { return toBool(getFont()); }
  protected:
    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
  private:
    int alertHours;
    int warningHours;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static int IdParent;
static int tDIM_INFO;

#define DIM_EDIT 8
#define DIM_PROGR 5
//#define DIM_INFO 80
#define DIM_INFO 70

#define DIM_TOT (DIM_PROGR+1 + DIM_INFO+1 + DIM_EDIT+1 + DIM_EDIT+4 + 4)

#define H_ED 24
#define Y1_ED1 20
#define Y2_ED1 (Y1_ED1 + H_ED)

#define Y1_ED2 Y1_ED1
#define Y2_ED2 Y2_ED1

#define W_ED1   470
#define X1_ED1 (X1_LB + 25)
#define X2_ED1 (X1_ED1 + W_ED1)

#define W_ED2 60
#define X1_ED2 (X2_ED1 + 1)
#define X2_ED2 (X1_ED2 + W_ED2)


#define W_LB 640
#define X1_LB ((DEF_X_SCREEN - W_LB) / 2)
#define X2_LB (DEF_X_SCREEN - X1_LB)

#define H_LB 250
#define Y1_LB (Y2_ED1 + 10)
#define Y2_LB (Y1_LB + H_LB)

#define H_FONT_LB 16
//----------------------------------------------------------------------------
uint calcLenMaintName()
{
  bool needDelete;
  uint currLen = 0;
  for(uint i = 0; i < MAX_MAINT; ++i) {
    LPCTSTR p = getStringByLangGlob(ID_FIRST_MAINT + i, needDelete);
    if(p) {
      uint len = _tcslen(p);
      if(len > currLen)
        currLen = len;
      if(currLen > DIM_INFO) {
        currLen = DIM_INFO;
        break;
        }
      if(needDelete)
        delete []p;
      }
    }
  if(currLen < DIM_INFO - 2)
    currLen += 2;
  return currLen;
}
//----------------------------------------------------------------------------
static int Offs1 = DIM_PROGR + DIM_INFO + 2;
static int Offs2 = DIM_PROGR + DIM_INFO + DIM_EDIT + 3;
//----------------------------------------------------------------------------
TD_Maintenance::TD_Maintenance(int idPar, PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(idPar, parent, resId, hinst), currSel(0), currId(0),
    //fEdit(0), Font(0)
    LB(0), info(0), ore(0)
{
  if(idPar)
    IdParent = idPar;

  tDIM_INFO = calcLenMaintName();
/*
  bool needDelete;
  for(uint i = 0; i < MAX_MAINT; ++i) {
    LPCTSTR p = getStringByLangGlob(ID_FIRST_MAINT + i, needDelete);
    if(p) {
      uint len = _tcslen(p);
      if(len > tDIM_INFO)
        tDIM_INFO = len;
      if(tDIM_INFO > DIM_INFO) {
        tDIM_INFO = DIM_INFO;
        break;
        }
      if(needDelete)
        delete []p;
      }
    }
  if(tDIM_INFO < DIM_INFO - 2)
    tDIM_INFO += 2;
*/
  Offs1 = DIM_PROGR + tDIM_INFO + 2;
  Offs2 = DIM_PROGR + tDIM_INFO + DIM_EDIT + 3;

  gestJobData *Job = Par->getData();
  IMaint *g_m = Job->getMaint();
  G_M = dynamic_cast<gest_maint*>(g_m);
}
//----------------------------------------------------------------------------
TD_Maintenance::~TD_Maintenance()
{
  destroy();
}
//----------------------------------------------------------------------------
#define F_1 10
#define F_2 (F_1 + 1)
#define F_3 (F_2 + 4)
#define F_4 (F_3 + 4)
//----------------------------------------------------------------------------
#define SET_FORMAT \
    TCHAR formt[] = _T("%02d   \t%-20s\t%6d\t%6d:%02d\t");\
    formt[F_1] = tDIM_INFO / 10 + _T('0');\
    formt[F_2] = (tDIM_INFO % 10) + _T('0');\
    formt[F_3] = DIM_EDIT + _T('0');\
    formt[F_4] = DIM_EDIT + _T('1');
//----------------------------------------------------------------------------
#define __DA_FARE__
#ifdef __DA_FARE__
static void addSign(TCHAR *buff, TCHAR sign)
{
  for(int i = 0; i < DIM_EDIT + 1; ++i)
    if(buff[i] >= _T('0')) {
      buff[i - 1] = sign;
      break;
      }
}
#endif
//----------------------------------------------------------------------------
static bool formatLine(TCHAR *buff, int id, const g_maint &gm)
{
  TCHAR t[DIM_INFO + 1];
#if 1
  bool needDelete;
  LPCTSTR p = getStringByLangGlob(ID_FIRST_MAINT + id, needDelete);
  if(p) {
    uint len = _tcslen(p);
    if((int)len > tDIM_INFO)
      len = tDIM_INFO;
    _tcsncpy_s(t, SIZE_A(t), p, len);
    if(needDelete)
      delete []p;
#else
  LPCTSTR p = getString(ID_FIRST_MAINT + id);
  if(p) {
    uint len = _tcslen(p);
    if(len > DIM_INFO)
      len = DIM_INFO;
    _tcsncpy(t, p, len);
#endif
    t[len] = 0;
    SET_FORMAT
#ifdef __DA_FARE__
    g_mem_maint mt(id, gm);
    __int64 time = mt.remain();
    bool neg = time < 0;
    if(neg)
      time = - time;
#else
    __int64 time = cMK_I64(gm.ft);
#endif
    time /= MINUTE_TO_I64;
    int minute = int(time % 60);
    int ore = int(time / 60);
    wsprintf(buff, formt, id + 1, t, gm.hours, ore, minute);
#ifdef __DA_FARE__
    if(neg)
      addSign(buff + Offs2, _T('+'));
    else
      addSign(buff + Offs2, _T('-'));
#endif
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#ifdef __DA_FARE__
// è negativo se ha il segno '+' che indica che ha sorpassato il tempo
// stabilito per la manutenzione
static bool isNeg(LPCTSTR buff)
{
  for(int i = 0; i < DIM_EDIT + 1; ++i) {
    if(_T('+') == buff[i])
      return true;
    if(_T('-') == buff[i])
      return false;
    if(buff[i] >= _T('0'))
      return false;
    }
  return false;
}
#endif
//----------------------------------------------------------------------------
static int getRemain(LPTSTR buff)
{
  int totHours = _ttoi(buff + Offs1);
  if(!totHours)
    return 10000;  // un valore sicuramente superiore al warning
  int otherHours = _ttoi(buff + Offs2);
#ifdef __DA_FARE__
  if(otherHours <= 0)  // è negativo se non ancora raggiunto la scadenza
    if(isNeg(buff + Offs2))
      return -1;
  if(!otherHours)
    return 1;
  return -otherHours;
#else
  otherHours = totHours - otherHours;
  return otherHours;
#endif
}
//----------------------------------------------------------------------------
void TD_Maintenance::fillLb()
{
  ::SendMessage(*LB, LB_RESETCONTENT, 0 ,0);
  TCHAR buff[DIM_TOT + 2];
  fillStr(buff, _T(' '), SIZE_A(buff));
  for(int i = 0; i < MAX_MAINT; ++i)
    if(formatLine(buff, i, fgm[i]))
      ::SendMessage(*LB, LB_ADDSTRING, 0, (WPARAM)buff);
  ::SetFocus(*LB);
  ::SendMessage(*LB, LB_SETCURSEL, currSel, 0);
}
//--------------------------------------------------------------------
bool TD_Maintenance::create()
{
  if(!baseClass::create())
    return false;

//  fEdit = DEF_FONT(R__Y(H_FONT_LB));

//  load();
#if 0
  ::EnableWindow(*ore, false);

  info->setFont(fEdit, false);
  ore->setFont(fEdit, false);

  // diminuisce lo spazio per la descrizione
//  LB->rescale(1, 1, 0.8);

  int count = ::SendMessage(*LB, LB_GETCOUNT, 0, 0);

  PRect r(R__X(X1_LB), R__Y(Y1_LB), R__X(X2_LB), R__Y(Y2_LB));

  int h = LB->getHItem();
  int w = ::SendMessage(*LB, LB_GETHORIZONTALEXTENT, 0, 0);

  if(r.Height() / h < count)
    w += GetSystemMetrics(SM_CYVSCROLL);
  w += GetSystemMetrics(SM_CYEDGE) * 2;

  r.right = r.left + w;

  w = int(sizer::getWidth() - r.Width()) / 2;

  if(w < 0) {
    w = R__X(5);
    r.left = w;
    r.right = sizer::getWidth() - R__X(10);
    }
  r.MoveTo(w, r.top);

  LB->setWindowPos(0, r, SWP_NOZORDER | SWP_SHOWWINDOW);

  LB->setIntegralHeight();

  int tabs[4];
  for(int i = 0; i < 4; ++i)
    tabs[i] = LB->getLenTab(i);

  PRect r1(tabs[0] + r.left, R__Y(Y1_ED1), tabs[1] + r.left, R__Y(Y2_ED1));
  info->setWindowPos(0, r1, SWP_NOZORDER | SWP_SHOWWINDOW);

  PRect r2(r1.right + 1, r1.top, tabs[2] + r.left + 10, r1.bottom);
  ore->setWindowPos(0, r2, SWP_NOZORDER | SWP_SHOWWINDOW);
#endif
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define ID_SAVE ID_F4
#define ID_ANN  ID_F2
#define ID_MOD  ID_F3
//----------------------------------------------------------------------------
#define CLICK_BY_MIN (1000 * 60 / Par->getTimeForTimer())
//----------------------------------------------------------------------------
// se dopo 10 minuti non ha ancora salvato, perde tutte le modifiche
#define RESET_EDIT (CLICK_BY_MIN * 10)
//----------------------------------------------------------------------------
// aggiorna ogni MINUTES minuti, il salvataggio automatico
// avviene ogni 5 minuti (definito in mntnce.cpp)
#define MINUTES 3
#define TIME_UPDATE (CLICK_BY_MIN * MINUTES)
//----------------------------------------------------------------------------
void TD_Maintenance::refreshBody()
{
  baseClass::refreshBody();
  if(!LB || !info || !ore)
    return;

  if(Par->isEnabledBtn(ID_ANN) && IsWindowEnabled(*ore) && mustResetEdit()) {
    ::EnableWindow(*ore, false);
    pushedBtn(ID_ANN);
    initTimeEdit(-1);
    }

  if(!Par->isEnabledBtn(ID_ANN) && G_M->isChanged())
    load();
}
//----------------------------------------------------------------------------
static
bool hasFzText(setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  return p && *p;
}
//----------------------------------------------------------------------------
#define CHECK_FZ_ENABLED(btn) \
  if(!hasFzText(sStr, btn)) \
    return 0;
//----------------------------------------------------------------------------
P_Body* TD_Maintenance::pushedBtn(int idBtn)
{
  extern void runPerif(PWin* main);

  switch(idBtn) {
    case ID_F2:
      CHECK_FZ_ENABLED(ID_F2)
      load();
      return 0;

    case ID_F3:
      CHECK_FZ_ENABLED(ID_F3)
      modify();
      return 0;

    case ID_F4:
      CHECK_FZ_ENABLED(ID_F4)
      save();
      return 0;

/*
    case ID_F6:
      G_M->start();
      if(G_M->isRunning()) {
        enableBtn(ID_F6, false);
        enableBtn(ID_F7);
        }
      break;
    case ID_F7:
      G_M->stop();
      enableBtn(ID_F7, false);
      enableBtn(ID_F6);
      load();
      break;

    case ID_F8:
      runPerif(Par);
      break;
*/
    case ID_F10:
      CHECK_FZ_ENABLED(ID_F10)
      threadSet();
      return 0;

    case ID_F11:
      CHECK_FZ_ENABLED(ID_F11)
      Reset();
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
void TD_Maintenance::Reset()
{
  if(msgBoxByLangGlob(this, ID_CONFIRM_RESET, ID_TITLE_CONFIRM_RESET, MB_YESNO | MB_ICONSTOP) != IDYES)
    return;
  G_M->reset();
  load();
}
//----------------------------------------------------------------------------
void TD_Maintenance::threadSet()
{
  if(!LB || !info || !ore)
    return;
  int n = ::SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(n < 0)
    return;
  TCHAR t[DIM_TOT + 2];
  ::SendMessage(*LB, LB_GETTEXT, n, (LPARAM)t);
  n = _ttoi(t) - 1;
  if(n < 0)
    return;
  int hour = 0;
  int minute = 0;
  if(IDOK == TD_SetTime(this, hour, minute).modal()) {
    __int64 v = hour * HOUR_TO_I64 + minute * MINUTE_TO_I64;
    G_M->setLeft(n, v);
    load();
    }
}
//----------------------------------------------------------------------------
static void enaBtnEnd(P_Body *w, bool ena)
{
  w->enableBtn(ID_F1, ena);
  w->enableBtn(ID_MOD, ena);
  w->enableBtn(ID_SAVE, !ena);
  w->enableBtn(ID_ANN, !ena);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_Maintenance::getFileStr(LPTSTR path)
{
  getFileStrCheckRemote(path, NAME_STR);
//  _tcscpy(path, NAME_STR);
//  getPath(path, dSystem);
}
//----------------------------------------------------------------------------
void TD_Maintenance::setReady(bool first)
{
  baseClass::setReady(true);

  if(LBox.getElem())
    LB = LBox[0];

  if(Edi.getElem()) {
    info = Edi[0];
    ore = Edi[1];
    EnableWindow(*info, false);
    EnableWindow(*ore, false);
    }
  load();
  enaBtnEnd(this, true);
  enableBtn(ID_F9, false);
  if(sStr.getString(_M(6)) && sStr.getString(_M(7))) {
    if(G_M->isRunning()) {
      enableBtn(ID_F6, false);
      enableBtn(ID_F7);
      }
    else {
      enableBtn(ID_F7, false);
      enableBtn(ID_F6);
      }
    }
  refresh();
}
//----------------------------------------------------------------------------
/*
const long *TD_Maintenance::getInfo()
{
  return MenuMaint;
}
//----------------------------------------------------------------------------
long TD_Maintenance::getTitle()
{
  enaBtnEnd(this, true);
  enableBtn(ID_F9, false);
  if(getString(_M(6)) && getString(_M(7))) {
    if(G_M->isRunning()) {
      enableBtn(ID_F6, false);
      enableBtn(ID_F7);
      }
    else {
      enableBtn(ID_F7, false);
      enableBtn(ID_F6);
      }
    }
  return ID_TITLE_MAINT;
}
*/
//----------------------------------------------------------------------------
HBRUSH TD_Maintenance::evCtlColor(HDC dc, HWND hWndChild, uint /*ctlType*/)
{
  if(!info || !ore)
    return 0;
  if(hWndChild == *info || hWndChild == *ore) {
    ::SetBkColor(dc, RGB(0xff,0xff,0xff));
    ::SetTextColor(dc, RGB(0,0,0));
    return HBRUSH(::GetStockObject(WHITE_BRUSH));
    }
  return 0;
}
//----------------------------------------------------------------------------
void TD_Maintenance::ENKillfocusOre()
{
  if(!LB || !info || !ore)
    return;
  TCHAR buff[DIM_TOT + 2];
  ::GetWindowText(*ore, buff, SIZE_A(buff));
  fgm[currId].hours = _ttoi(buff);
  ::EnableWindow(*ore, false);
  fillStr(buff, _T(' '), SIZE_A(buff));
  formatLine(buff, currId, fgm[currId]);
  ::SendMessage(*LB, LB_DELETESTRING, currSel, 0);
  ::SendMessage(*LB, LB_INSERTSTRING, currSel, (LPARAM)buff);
  enableBtn(ID_MOD);
  ::SendMessage(*LB, LB_SETCURSEL, currSel, 0);
}
//----------------------------------------------------------------------------
void TD_Maintenance::load()
{
  if(!LB || !info || !ore)
    return;
  currSel = ::SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(currSel < 0)
    currSel = 0;
  if(G_M->getAll(fgm)) {
    enaBtnEnd(this, true);
    fillLb();
    }
}
//----------------------------------------------------------------------------
void TD_Maintenance::modify()
{
  if(!LB || !info || !ore)
    return;
  currSel = ::SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(currSel >= 0) {
    TCHAR t[DIM_TOT + 2];
    ::SendMessage(*LB, LB_GETTEXT, currSel, (LPARAM)t);
    currId = _ttoi(t) - 1;
    t[DIM_PROGR + tDIM_INFO] = 0;
    ::SetWindowText(*info, t + DIM_PROGR + 1);
    t[DIM_PROGR + tDIM_INFO + DIM_EDIT + 2] = 0;
    TCHAR* p = t + DIM_PROGR + tDIM_INFO + 2;
    TCHAR* d = t;
    int i;
    for(i = 0; i < DIM_EDIT; ++i, ++p)
      if(*p > _T(' '))
        break;
    for(; i < DIM_EDIT; ++i, ++p) {
      if(*p <= _T(' '))
        break;
      *d++ = *p;
      }
    *d = 0;

    ::SetWindowText(*ore, t);
    ::EnableWindow(*ore, true);
    ::SetFocus(*ore);
    enaBtnEnd(this, false);
    initTimeEdit(RESET_EDIT);
    }
}
//----------------------------------------------------------------------------
void TD_Maintenance::evPaint(HDC hdc, const PRect& rect)
{
  baseClass::evPaint(hdc, rect);
/*
  HICON hIcon = (HICON)LoadImage( getHInstance(),
            MAKEINTRESOURCE(IDI_ICON_TIMER),
            IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);

  ::DrawIcon(hdc, 5, 5, hIcon);

  ::DestroyIcon(hIcon);
*/
}
//----------------------------------------------------------------------------
void TD_Maintenance::save()
{
  G_M->setHours(fgm);
  load();
}
//----------------------------------------------------------------------------
LRESULT TD_Maintenance::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case EN_KILLFOCUS:
          if(ore && GetWindowLongPtr(*ore, GWL_ID) == LOWORD(wParam))
            ENKillfocusOre();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PVarListBox* TD_Maintenance::allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen, HINSTANCE hinst)
{
  int alertHours;
  LPCTSTR p = getString(ID_ALERT_HOURS);
  if(!p)
    alertHours = 2;
  else
    alertHours = _ttoi(p);
  if(!alertHours)
    alertHours = 2;

  int warningHours;
  p = getString(ID_WARNING_HOURS);
  if(!p)
    warningHours = 12;
  else
    warningHours = _ttoi(p);
  if(!warningHours)
    warningHours = 12;
  listBoxMaint* lb = new listBoxMaint(alertHours, warningHours, parent, style, id, rect, textlen, hinst);

  if(!lb->hasFont()) {
    p = getString(ID_SIZE_FONT_ALARM);
    int h_font = H_FONT_LB;
    if(p)
      h_font = _ttoi(p);
    if(!h_font)
      h_font = H_FONT_LB;

    p = getString(ID_NAME_FONT_ALARM);
    if(!p)
      p = _T("arial");
    HFONT Font = D_FONT(R__Y(h_font), 0, 0, p);
    lb->setFont(Font, true);

    p = getString(ID_FIXED_WIDTH_FONT_ALARM);
    if(p) {
      double fix = _tstof(p);
      if(fix > 0)
        lb->setPixelWidthChar(R__Xf(fix));
      }
    }
  int t[] = {  -DIM_PROGR, tDIM_INFO, -DIM_EDIT, -(DIM_EDIT + 3) };
  lb->SetTabStop(SIZE_A(t), t, 0);
  return lb;
}
//----------------------------------------------------------------------------
listBoxMaint::listBoxMaint(int alert, int warn, P_BaseBody* parent, uint style, uint id,
              const PRect& rect, int len, HINSTANCE hinst) :
      PVarListBox(parent, style, id, rect, len, hinst), baseActive(parent, id),
//      PListBox(parent, id, len, hinst),
      alertHours(alert),
      warningHours(warn)
{
}
//----------------------------------------------------------------------------
#if 0
  #define BKCOLOR_MAX_ALERT_E RGB(0xff, 0x7f, 0x7f)
  #define BKCOLOR_MAX_ALERT   RGB(0xff, 0x7f, 0x7f)
  #define COLORT_MAX_ALERT_E  RGB(0xcf, 0xff, 0xff)
  #define COLORT_MAX_ALERT    RGB(0xcf, 0xff, 0xff)
#else
  #define BKCOLOR_MAX_ALERT_E RGB(0xcf, 0xcf, 0xcf)
  #define BKCOLOR_MAX_ALERT   RGB(0xdf, 0xdf, 0xdf)
  #define COLORT_MAX_ALERT_E  RGB(0xcf, 0x00, 0x00)
  #define COLORT_MAX_ALERT    RGB(0xaf, 0x00, 0x00)
#endif
//----------------------------------------------------------------------------
#define BKCOLOR_ALERT_E RGB(0xff, 0xaf, 0xaf)
#define BKCOLOR_ALERT   RGB(0xff, 0xcf, 0xcf)
//----------------------------------------------------------------------------
#define BKCOLOR_WARNING_E RGB(0xdf, 0xdf, 0xaf)
#define BKCOLOR_WARNING   RGB(0xff, 0xff, 0xcf)
//----------------------------------------------------------------------------
#define BKCOLOR_NORM_E  RGB(0xaf, 0xff, 0xaf)
#define BKCOLOR_NORM    RGB(0xcf, 0xff, 0xcf)
//----------------------------------------------------------------------------
#define COLORT    RGB(0, 0, 0)
#define COLORT_E  RGB(0, 0, 0)
//----------------------------------------------------------------------------
HPEN listBoxMaint::setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo)
{
  int ore = getRemain(Buff);
  COLORREF cText;
  COLORREF cBkg;
  if (drawInfo.itemState & ODS_SELECTED)  {
    if(ore <= 0) {
      cText = COLORT_MAX_ALERT_E;
      cBkg = BKCOLOR_MAX_ALERT_E;
      }
    else if(ore <= alertHours) {
      cText = COLORT_E;
      cBkg = BKCOLOR_ALERT_E;
      }
    else if(ore <= warningHours) {
      cText = COLORT_E;
      cBkg = BKCOLOR_WARNING_E;
      }
    else {
      cText = COLORT_E;
      cBkg = BKCOLOR_NORM_E;
      }
    }
  else {
    if(ore <= 0) {
      cText = COLORT_MAX_ALERT;
//      cText = COLORT_E;
      cBkg = BKCOLOR_MAX_ALERT;
      }
    else if(ore <= alertHours) {
      cText = COLORT_E;
      cBkg = BKCOLOR_ALERT;
      }
    else if(ore <= warningHours) {
      cText = COLORT_E;
      cBkg = BKCOLOR_WARNING;
      }
    else {
      cText = COLORT_E;
      cBkg = BKCOLOR_NORM;
      }
    }
  PRect Rect(drawInfo.rcItem);

  SetBkColor(hDC, cBkg);
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, Rect, 0, 0, 0);

  SetTextColor(hDC, cText);
  HPEN pen = CreatePen(PS_SOLID, 1, COLORT);
  HPEN oldPen = (HPEN) SelectObject(hDC, pen);
  return oldPen;
}
//----------------------------------------------------------------------------
