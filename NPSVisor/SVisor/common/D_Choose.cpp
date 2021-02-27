//-------- d_choose.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "p_body.h"
#include "p_util.h"
#include "d_choose.h"
#include "id_btn.h"
#include "1.h"
#include "id_msg_common.h"
#include "Commonlgraph.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class myListChoose : public PListBox
{
  public:
    myListChoose(PWin* parent, setOfString& sStr, int id, int hfont = 18, int len = 255);
    bool create();
  protected:
    virtual void paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
//    virtual void setColor(TDC &DC, DRAWITEMSTRUCT FAR& drawInfo);
  private:
    COLORREF colors[MAX_CHOOSE];
    int heightFont;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class chooseGen : public svDialog
{
  public:
    chooseGen(PWin *parent, uint id = IDD_EXPORT):
      baseClass(parent, id), choose(0) {  }
    int getChoose() { return choose; }
    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void dblClick();
    virtual void fill() = 0;
    virtual void setPersist(int set) = 0;
  private:
    int choose;
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool chooseGen::create()
{
  if(!baseClass::create())
    return false;
  PRect r;
  GetWindowRect(*getParent(), r);

  PRect rThis;
  GetWindowRect(*this, rThis);
  rThis.MoveTo(r.right, r.top);
//  MapWindowPoints(HWND_DESKTOP, *getParent(), (LPPOINT)(LPRECT)rThis, 2);
  setWindowPos(0, rThis, SWP_NOZORDER | SWP_NOSIZE);
  fill();
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F1), ID_DIALOG_OK);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F2), ID_DIALOG_CANC);
  return true;
}
//----------------------------------------------------------------------------
void chooseGen::CmOk()
{
  HWND hw = ::GetDlgItem(*this, IDC_LISTBOX_CHOOSE_EXP);
  if(hw)
    choose = ::SendMessage(hw, LB_GETCURSEL, 0, 0);
  setPersist(choose);
  EndDialog(*this, IDOK);
}
//----------------------------------------------------------------------------
void chooseGen::dblClick()
{
  CmOk();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define EV_BN_CLICKED(id, fz) \
        case id:  \
          fz();   \
          break;
//----------------------------------------------------------------------------
LRESULT chooseGen::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_BUTTON_F1:
        EV_BN_CLICKED(IDOK, CmOk)

        case IDC_BUTTON_F2:
        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
        }
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          dblClick();
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class chooseElement : public chooseGen
{
  public:
    chooseElement(PWin *parent, setOfString& sos, uint id = IDD_EXPORT):
      chooseGen(parent, id), sStr(sos) {  }
  protected:
    virtual void fill();
    void setPersist(int set) { oldSel = set; }
  private:
    static int oldSel;
    setOfString& sStr;
};
//----------------------------------------------------------------------------
int chooseElement::oldSel = 0;
//----------------------------------------------------------------------------
void chooseElement::fill()
{
  HWND hw = ::GetDlgItem(*this, IDC_LISTBOX_CHOOSE_EXP);
  if(hw) {
#if 1
    LPCTSTR pNameTrend = sStr.getString(ID_TREND_NAME);
    uint tot;
    uint id = getInitDataByNameTrend(pNameTrend, tot);
    if(id) {
      for(uint i = 0; i < tot; ++i) {
        bool needDelete;
        LPCTSTR p = getDescrDataTrend(i + id, needDelete);
        smartPointerConstString sp(p, needDelete);
        SendMessage(hw, LB_INSERTSTRING, -1, (LPARAM)p);
        }
      }
#else
    for(int i = ID_INIT_CHOOSE_GRAPH; i < ID_INIT_CHOOSE_GRAPH + MAX_CHOOSE_GRAPH; ++i) {
      LPCTSTR p = sStr.getString(i);
      if(!p)
        break;
      p = findNextParam(p, 2);
      ::SendMessage(hw, LB_INSERTSTRING, -1, (LPARAM)p);
      }
#endif
    ::SendMessage(hw, LB_SETCURSEL, oldSel, 0);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define DIM_ID 3
#define DIM_DESCR1 40
//----------------------------------------------------------------------------
TD_Choose::TD_Choose(PWin* parent, setOfString& sos, int* last_choose, uint resId)
:
    baseClass(parent, resId), lastChoose(last_choose), sStr(sos)
{
  LB_Choose = new myListChoose(this, sos, IDC_LISTBOX_CHOOSE_LG);
  int t[] = {  DIM_ID, DIM_DESCR1};
  LB_Choose->SetTabStop(sizeof(t)/sizeof(t[0]), t, 0);
}
//----------------------------------------------------------------------------
TD_Choose::~TD_Choose()
{
  destroy();
}
//----------------------------------------------------------------------------
static void setID(LPTSTR buff, int code)
{
  fillStr(buff, _T(' '), DIM_ID);
  TCHAR t[20];
  _itot_s(code, t, SIZE_A(t), 10);
  int l = _tcslen(t);
  for(int i = l - 1; i >= 0; --i)
    buff[i] = t[i];
  buff[DIM_ID] = _T('\t');
  buff[DIM_ID + 1] = 0;
}
//----------------------------------------------------------------------------
#define SEP TAB
//----------------------------------------------------------------------------
/*
#define USE_NUM
#ifdef USE_NUM
  #define getP2(i) \
    _itot(code - i + 1, tmp, 10); \
    p2 = tmp;
#else
  #define getP2(i) p2 = sStr.getStringOrId(ID_DESCR_CHOOSE_INIT + code - i);
#endif
*/
//----------------------------------------------------------------------------
bool TD_Choose::addToLB(int code)
{
#if 1
  LPCTSTR pNameTrend = sStr.getString(ID_TREND_NAME);
  uint tot;
  uint id = getInitDataByNameTrend(pNameTrend, tot);
  bool needDelete;
  LPCTSTR p = getDescrDataTrend(code + id - 1, needDelete);
  smartPointerConstString sp(p, needDelete);

#else
  LPCTSTR p = sStr.getStringOrId(ID_INIT_CHOOSE_GRAPH + code - 1);
  p = findNextParam(p, 2);
#endif
  TCHAR t[DIM_ID + 32];
  TCHAR buff[200];

  setID(t, code);

  wsprintf(buff, _T("%s%s"), t, p);
  SendMessage(*LB_Choose, LB_INSERTSTRING, -1, (LPARAM)buff);
  return true;
/*
  LPCTSTR p1;
  LPCTSTR p2;
#ifdef USE_NUM
  TCHAR tmp[20];
#endif
  if(code >= INIT_POWER && code < END_POWER) {
    p1 = sStr.getStringOrId(ID_CHOOSE_POWER);
    getP2(INIT_POWER)
    }
  else if(code >= INIT_SPEED && code < END_SPEED) {
    p1 = sStr.getStringOrId(ID_CHOOSE_SPEED);
    getP2(INIT_SPEED)
//    p2 = sStr.getStringOrId(ID_DESCR_CHOOSE_INIT + code - INIT_SPEED);
    }
  else if(code >= INIT_TEMP && code < END_TEMP) {
    p1 = sStr.getStringOrId(ID_CHOOSE_TEMP);
    getP2(INIT_TEMP)
//    p2 = sStr.getStringOrId(ID_DESCR_CHOOSE_INIT + code - INIT_TEMP);
    }
  else if(code >= INIT_PRESS && code < END_PRESS) {
    p1 = sStr.getStringOrId(ID_CHOOSE_PRESS);
    getP2(INIT_PRESS)
//    p2 = sStr.getStringOrId(ID_DESCR_CHOOSE_INIT + code - INIT_PRESS);
    }
  else {
    p1 = sStr.getStringOrId(ID_INIT_CHOOSE_GRAPH_OTHER + code - INIT_OTHER);
    p2 = _T("");
    }

  TCHAR t[DIM_ID + 4];
  TCHAR buff[200];
  setID(t, code);
  wsprintf(buff, _T("%s%s - %s"), t, p1, p2);
  SendMessage(*LB_Choose, LB_INSERTSTRING, -1, (LPARAM)buff);
  return true;
*/
}
//----------------------------------------------------------------------------
void TD_Choose::fill_LB()
{
  for(int i = 0; i < MAX_CHOOSE; ++i) {
    int last = lastChoose[i];
    if(!last)
      break;
    if(!addToLB(last))
      continue;
    }
  if(MAX_CHOOSE == SendMessage(*LB_Choose, LB_GETCOUNT, 0, 0))
    enableBtn(IDC_BUTTON_F3, false);
  SendMessage(*LB_Choose, LB_SETCURSEL, 0, 0);
  LB_SelChange();
}
//----------------------------------------------------------------------------
void TD_Choose::enableBtn(int id, bool enable)
{
  HWND hw = ::GetDlgItem(*this, id);
  if(hw)
    ::EnableWindow(hw, enable);
}
//----------------------------------------------------------------------------
bool TD_Choose::isEnableBtn(int id)
{
  HWND hw = ::GetDlgItem(*this, id);
  if(hw)
    return toBool(::IsWindowEnabled(hw));
  return false;
}
//----------------------------------------------------------------------------
void TD_Choose::LB_SelChange()
{
  int ix = SendMessage(*LB_Choose, LB_GETCURSEL, 0, 0);
  int count = SendMessage(*LB_Choose, LB_GETCOUNT, 0, 0);
  if(ix < 0 || !count) {
    enableBtn(IDC_BUTTON_F4, false);
    enableBtn(IDC_BUTTON_F5, false);
    enableBtn(IDC_BUTTON_F6, false);
    }
  else {
    if(!ix) {
      enableBtn(IDC_BUTTON_F5, false);
      enableBtn(IDC_BUTTON_F6, count > 1);
      }
    else {
      if(ix == count - 1) {
        enableBtn(IDC_BUTTON_F5, true);
        enableBtn(IDC_BUTTON_F6, false);
        }
      else {
        enableBtn(IDC_BUTTON_F5, true);
        enableBtn(IDC_BUTTON_F6, true);
        }
      }
    enableBtn(IDC_BUTTON_F4, true);
    }
}
//----------------------------------------------------------------------------
#define SET_(id, txt)   ::SetDlgItemText(*this, id, txt)
//----------------------------------------------------------------------------
bool TD_Choose::create()
{
  if(!baseClass::create())
    return false;
#if 1
  setWindowTextByLang(sStr, *this, ID_TITLE_CHOOSE_LGRAPH);
#else
  setCaption(sStr.getStringOrId(ID_TITLE_CHOOSE_LGRAPH));
#endif
  for(int i = 0; i < MAX_BTN_CHOOSE; ++i) {
#if 1
    smartPointerConstString sp = getStringOrIdByLang(sStr, ID_TEXT_CHOOSE_LGRAPH + i);
    SET_(IDC_STATICTEXT_G_F1 + i, &sp);
#else
    SET_(IDC_STATICTEXT_G_F1 + i, sStr.getStringOrId(ID_TEXT_CHOOSE_LGRAPH + i));
#endif
    }
  fill_LB();
  return true;
}
//----------------------------------------------------------------------------
#define EV_BN_CLICKED(id, fz) \
        case id:  \
          fz();   \
          break;
//----------------------------------------------------------------------------
LRESULT TD_Choose::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        EV_BN_CLICKED(IDC_BUTTON_F1, BNClicked_F1)
        EV_BN_CLICKED(IDC_BUTTON_F2, BNClicked_F2)
        EV_BN_CLICKED(IDC_BUTTON_F3, BNClicked_F3)
        EV_BN_CLICKED(IDC_BUTTON_F4, BNClicked_F4)
        EV_BN_CLICKED(IDC_BUTTON_F5, BNClicked_F5)
        EV_BN_CLICKED(IDC_BUTTON_F6, BNClicked_F6)

        EV_BN_CLICKED(IDOK, BNClicked_F1)
        EV_BN_CLICKED(IDCANCEL, BNClicked_F2)
        }
      switch(HIWORD(wParam)) {
        case LBN_SELCHANGE:
          LB_SelChange();
          break;
        }
      break;


    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);

}
//----------------------------------------------------------------------------
void TD_Choose::BNClicked_F1()
{
  int count = SendMessage(*LB_Choose, LB_GETCOUNT, 0, 0);
  int i;
  TCHAR t[500];
  for(i = 0; i < count; ++i) {
    SendMessage(*LB_Choose, LB_GETTEXT, i, (LPARAM)t);
    lastChoose[i] = _ttoi(t);
    }
  for(; i < MAX_CHOOSE; ++i)
    lastChoose[i] = 0;
  EndDialog(*this, IDOK);
}
//----------------------------------------------------------------------------
void TD_Choose::BNClicked_F2()
{
  EndDialog(*this, IDCANCEL);
}
//----------------------------------------------------------------------------
void TD_Choose::BNClicked_F3()
{
  if(!isEnableBtn(IDC_BUTTON_F3))
    return;
  int choose = -1;
  chooseElement ce(this, sStr);
  if(ce.modal() == IDOK) {
    choose = ce.getChoose();
/*
    int init[] = { INIT_POWER , INIT_SPEED, INIT_TEMP, INIT_PRESS };
    int end[] = { END_POWER , END_SPEED, END_TEMP, END_PRESS };
    chooseNum cn(this, sStr.getStringOrId(ID_CHOOSE_POWER + choose), end[choose] - init[choose]);
    if(cn.modal() == IDOK)
        choose = init[choose] + cn.getChoose();
      else
        choose = -1;
*/
    }
  if(choose < 0)
    return;
  addToLB(choose + 1);
  int count = SendMessage(*LB_Choose, LB_GETCOUNT, 0, 0);
  if(count >= MAX_CHOOSE)
    enableBtn(IDC_BUTTON_F3, false);
  SendMessage(*LB_Choose, LB_SETCURSEL, count - 1, 0);
  LB_SelChange();
  SetFocus(*LB_Choose);
}
//---------------------------------------------------------------------
void TD_Choose::BNClicked_F4()
{
  if(!isEnableBtn(IDC_BUTTON_F4))
    return;
  int ix = SendMessage(*LB_Choose, LB_GETCURSEL, 0, 0);
  if(ix < 0)
    return;

  SendMessage(*LB_Choose, LB_DELETESTRING, ix, 0);
  if(SendMessage(*LB_Choose, LB_GETCOUNT, 0, 0) == ix)
    --ix;
  SendMessage(*LB_Choose, LB_SETCURSEL, ix, 0);
  if(!isEnableBtn(IDC_BUTTON_F3))
    enableBtn(IDC_BUTTON_F3, true);

  LB_SelChange();
  SetFocus(*LB_Choose);
}
//---------------------------------------------------------------------
void TD_Choose::BNClicked_F5()
{
  if(!isEnableBtn(IDC_BUTTON_F5))
    return;
  int ix = SendMessage(*LB_Choose, LB_GETCURSEL, 0, 0);
  if(ix <= 0)
    return;
  TCHAR t[200];
  SendMessage(*LB_Choose, LB_GETTEXT, ix, (LPARAM)t);
  SendMessage(*LB_Choose, LB_DELETESTRING, ix, 0);
  SendMessage(*LB_Choose, LB_INSERTSTRING, ix - 1, (LPARAM)t);
  SendMessage(*LB_Choose, LB_SETCURSEL, ix - 1, 0);

  LB_SelChange();
  SetFocus(*LB_Choose);
}
//---------------------------------------------------------------------
void TD_Choose::BNClicked_F6()
{
  if(!isEnableBtn(IDC_BUTTON_F6))
    return;
  int ix = SendMessage(*LB_Choose, LB_GETCURSEL, 0, 0);
  if(ix < 0)
    return;
  TCHAR t[200];
  SendMessage(*LB_Choose, LB_GETTEXT, ix, (LPARAM)t);
  SendMessage(*LB_Choose, LB_DELETESTRING, ix, 0);
  SendMessage(*LB_Choose, LB_INSERTSTRING, ix + 1, (LPARAM)t);
  SendMessage(*LB_Choose, LB_SETCURSEL, ix + 1, 0);
  LB_SelChange();
  SetFocus(*LB_Choose);
}
//-----------------------------------------------------------------
//----------------------------------------------------------------------------
myListChoose::myListChoose(PWin* parent, setOfString& sStr, int id, int hfont, int len):
       PListBox(parent, id, len), heightFont(hfont)
{
  extern COLORREF getColor(LPCTSTR p, int *tick);
  int dummy;
  for(int i = 0; i < MAX_CHOOSE; ++i) {
    LPCTSTR p = sStr.getString(ID_COLOR_1 + i);
    colors[i] = getColor(p, &dummy);
    }
}
//------------------------------------------------------------------
void myListChoose::paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(!ix) {
    uint item = drawInfo.itemID;
    if(item >= MAX_CHOOSE)
      return;
    PRect r2(r);
    r2.Inflate(-1, -2);
    r2.Offset(1, 0);
    HBRUSH hbrush = CreateSolidBrush(colors[item]);
    FillRect(hdc, r2, hbrush);
    DeleteObject(hbrush);
//    COLORREF old = SetBkColor(hdc, colors[item]);
//    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, r2, 0, 0, 0);
//    SetBkColor(hdc, old);
    }
  PListBox::paintCustom(hdc, r, ix, drawInfo);
}
//------------------------------------------------------------------
bool myListChoose::create()
{
  if(!PListBox::create())
    return false;
  setFont(DEF_FONT(heightFont), true);
  return true;
}

