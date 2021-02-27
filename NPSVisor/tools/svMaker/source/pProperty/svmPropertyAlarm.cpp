//--------------- svmPropertyAlarm.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "svmPropertyAlarm.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "common.h"
//-----------------------------------------------------------
static LPCTSTR lbA_listText[] = {
    _T("Allarmi attivi"),
    _T("Allarmi attivi riconosciuti"),
    _T("Eventi attivi"),
    _T("Eventi attivi riconosciuti"),
    _T("Allarmi/Eventi non attivi"),
    };
//-----------------------------------------------------------
LPCTSTR getStrAlarm(uint ix)
{
  if(ix >= SIZE_A(lbA_listText))
    return _T("valore errato");
  return lbA_listText[ix];
}
//-----------------------------------------------------------
class myListBox_Alarm : public PListBox
{
  private:
    typedef PListBox baseClass;
  public:
//    myListBox_Alarm(PWin* parent, uint idc, PropertyAlarm::lineColor* &lColor, const PRect& r, int len = 255, HINSTANCE hinst = 0) :
//      baseClass(parent, idc, r, len, hinst) : LineColor(lColor) {}
    myListBox_Alarm(PWin* parent, uint idc, PropertyAlarm::lineColor* lColor, int len = 255, HINSTANCE hinst = 0) :
      baseClass(parent, idc, len, hinst), LineColor(lColor), vFocus(-1) {}
    bool create();
    void set_pColor(PropertyAlarm::lineColor* lColor) { LineColor = lColor; }
    void setVirtualFocus(int set) { vFocus = set; }
  protected:
    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
  private:
    PropertyAlarm::lineColor* LineColor;
    int vFocus;
};
//-----------------------------------------------------------
bool myListBox_Alarm::create()
{
  int tab[] = { 30 };
  SetTabStop(SIZE_A(tab), tab, 0);
  if(!baseClass::create())
    return false;
  for(uint i = 0; i < SIZE_A(lbA_listText); ++i)
    SendMessage(*this, LB_ADDSTRING, 0, (LPARAM)lbA_listText[i]);
  return true;
}
//-----------------------------------------------------------
HPEN myListBox_Alarm::setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(-1 == drawInfo.itemID || MAX_LINE_COLOR <= drawInfo.itemID || !LineColor)
    return baseClass::setColor(hDC, drawInfo);

  PropertyAlarm::lineColor* lCol = &LineColor[drawInfo.itemID];
  PRect Rect(drawInfo.rcItem);

  COLORREF cText;
  COLORREF cBkg;

  if((vFocus == drawInfo.itemID)|| drawInfo.itemState & ODS_FOCUS) {
    cText = lCol->fg_s;
    cBkg = lCol->bg_s;
    }
  else {
    cText = lCol->fg_n;
    cBkg = lCol->bg_n;
    }

  SetBkColor(hDC, cBkg);
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, Rect, 0, 0, 0);

  SetTextColor(hDC, cText);
  HPEN pen = CreatePen(PS_SOLID, 1, cText);
  HPEN oldPen = (HPEN) SelectObject(hDC, pen);
  return oldPen;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PropertyAlarm::~PropertyAlarm()
{
}
//-----------------------------------------------------------
#define CLONE_V(a) a = po->a
//-----------------------------------------------------------
void PropertyAlarm::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyAlarm* po = dynamic_cast<const PropertyAlarm*>(&other);
  if(po && po != this) {

    for(uint i = 0; i < SIZE_A(DataPrf); ++i)
      CLONE_V(DataPrf[i]);
    for(uint i = 0; i < SIZE_A(lColor); ++i)
      CLONE_V(lColor[i]);
    for(uint i = 0; i < SIZE_A(percWidth); ++i)
      CLONE_V(percWidth[i]);
    for(uint i = 0; i < SIZE_A(columnPos); ++i)
      CLONE_V(columnPos[i]);
    }
}
//-----------------------------------------------------------
void PropertyAlarm::cloneMinusProperty(const Property& other)
{
  clone(other);
}
//-----------------------------------------------------------
struct idcColors {
  uint idcS;
  uint idcE[3];
};
//-----------------------------------------------------------
static idcColors alColors[] = {
    { IDC_STATIC_AL_CL_FG, { IDC_EDIT_AL_COLOR_R_FG, IDC_EDIT_AL_COLOR_G_FG, IDC_EDIT_AL_COLOR_B_FG } },
    { IDC_STATIC_AL_CL_BG, { IDC_EDIT_AL_COLOR_R_BG, IDC_EDIT_AL_COLOR_G_BG, IDC_EDIT_AL_COLOR_B_BG } },
    { IDC_STATIC_AL_CL_FG2, { IDC_EDIT_AL_COLOR_R_FG2, IDC_EDIT_AL_COLOR_G_FG2, IDC_EDIT_AL_COLOR_B_FG2 } },
    { IDC_STATIC_AL_CL_BG2, { IDC_EDIT_AL_COLOR_R_BG2, IDC_EDIT_AL_COLOR_G_BG2, IDC_EDIT_AL_COLOR_B_BG2 } },
    };
//-----------------------------------------------------------
static uint idcTrackBar[] = { IDC_SLIDER_AL_R, IDC_SLIDER_AL_G, IDC_SLIDER_AL_B };
//-----------------------------------------------------------
#define INFO_COLOR RGB(230, 215, 240)
//-----------------------------------------------------------
svmDialogAlarm::svmDialogAlarm(svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(owner, prop, parent, id, hInst), lastSel(-1), idTimer(0), needRefreshLb(false),
      hBrInfo(CreateSolidBrush(INFO_COLOR)), onMyModify(false)
{
  for(uint i = 0; i < SIZE_A(colorEdit); ++i)
    colorEdit[i] = new P_SimpleColorRow(this, alColors[i].idcS, alColors[i].idcE);
  CurrColor = new PColorBox(this, 0, IDC_STATIC_AL_CURR_COLOR);
}

//-----------------------------------------------------------
svmDialogAlarm::~svmDialogAlarm()
{
  destroy();
  for(uint i = 0; i < SIZE_A(colorEdit); ++i)
    delete colorEdit[i];
  DeleteObject(hBrInfo);
}
//-----------------------------------------------------------
bool svmDialogAlarm::create()
{
  myListBox_Alarm* lbA = new myListBox_Alarm(this, IDC_LIST_AL_COLOR_SAMPLE, 0);
  if(!baseClass::create())
    return false;
  PropertyAlarm* pa = dynamic_cast<PropertyAlarm*>(tmpProp);
  lbA->set_pColor(pa->lColor);
  uint idcsW[] = {
    IDC_EDIT_AL_PERC_1,
    IDC_EDIT_AL_PERC_2,
    IDC_EDIT_AL_PERC_3,
    IDC_EDIT_AL_PERC_4,
    IDC_EDIT_AL_PERC_5,
    IDC_EDIT_AL_PERC_6,
    IDC_EDIT_AL_PERC_7,
    IDC_EDIT_AL_PERC_8,
    IDC_EDIT_AL_PERC_9,
    };
  for(uint i = 0; i < SIZE_A(pa->percWidth); ++i)
    if(pa->percWidth[i])
      SET_INT(idcsW[i], pa->percWidth[i]);

  uint idcsP[] = {
    IDC_EDIT_AL_POS_1,
    IDC_EDIT_AL_POS_2,
    IDC_EDIT_AL_POS_3,
    IDC_EDIT_AL_POS_4,
    IDC_EDIT_AL_POS_5,
    IDC_EDIT_AL_POS_6,
    IDC_EDIT_AL_POS_7,
    IDC_EDIT_AL_POS_8,
    IDC_EDIT_AL_POS_9,
    };
  for(uint i = 0; i < SIZE_A(pa->columnPos); ++i)
    SET_INT(idcsP[i], pa->columnPos[i]);

  fillPrf2();
  fillPrf2Grp();
  fillPrf2Txt();
  SET_INT(IDC_EDIT_ADDR_FLT, pa->DataPrf[0].addr);
  SET_INT(IDC_EDIT_ADDR_FLT_GRP, pa->DataPrf[1].addr);
  SET_INT(IDC_EDIT_ADDR_FLT_TXT, pa->DataPrf[2].addr);
  SET_INT(IDC_EDIT_ADDR_FLT_TXT_LEN, pa->DataPrf[2].dec);

  for(int i = 0; i < 3; ++i) {
    HWND hsb = GetDlgItem(*this, idcTrackBar[i]);
    if(hsb) {
      SendMessage(hsb, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 255));
      SendMessage(hsb, TBM_SETPAGESIZE, 0, (LPARAM)10);
      SendMessage(hsb, TBM_SETSEL, TRUE, (LPARAM)MAKELONG(0, 255));
      }
    }

  uint idcs[] = {
    IDC_CHECK_AL_ID,
    IDC_CHECK_AL_STAT,
    IDC_CHECK_AL_DATE,
    IDC_CHECK_AL_TIME,
    IDC_CHECK_AL_FILTER,
    IDC_CHECK_AL_ID_GRP,
    IDC_CHECK_AL_DESCR_GRP,
    IDC_CHECK_AL_PRPH,
    IDC_CHECK_AL_DESCR_AL,

    IDC_CHECK_AL_USE_LBOX,
    };
//  SET_CHECK(IDC_CHECK_AL_ID);

  for(uint i = 0; i < SIZE_A(idcs); ++i) {
    if(Prop->PA_def_show & (1 << i))
      SET_CHECK(idcs[i]);
    }
  SET_INT(IDC_EDIT_SIMPLE_TEXT, Prop->PA_fix_filter);
  SendMessage(*lbA, LB_SETCURSEL, 0, 0);
//  colorEdit[0]->setFocus(idcTrackBar);
  selChgColor();
  return true;
}
//-----------------------------------------------------------
void svmDialogAlarm::fillPrf2()
{
  PropertyAlarm* pt = dynamic_cast<PropertyAlarm*>(Prop);
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_FLT);
  fillCBPerif(hwnd, pt->DataPrf[0].perif);
}
//-----------------------------------------------------------
void svmDialogAlarm::fillPrf2Grp()
{
  PropertyAlarm* pt = dynamic_cast<PropertyAlarm*>(Prop);
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_FLT_GRP);
  fillCBPerif(hwnd, pt->DataPrf[1].perif);
}
//-----------------------------------------------------------
void svmDialogAlarm::fillPrf2Txt()
{
  PropertyAlarm* pt = dynamic_cast<PropertyAlarm*>(Prop);
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_FLT_TXT);
  fillCBPerif(hwnd, pt->DataPrf[2].perif);
}
//-----------------------------------------------------------
void svmDialogAlarm::updateData()
{
  if(lastSel < 0 || lastSel >= MAX_LINE_COLOR)
    return;
  if(onMyModify)
    return;
  PropertyAlarm* pt = dynamic_cast<PropertyAlarm*>(tmpProp);
  PropertyAlarm::lineColor& lc = pt->lColor[lastSel];
  lc.fg_n = colorEdit[0]->getColor();
  lc.bg_n = colorEdit[1]->getColor();
  lc.fg_s = colorEdit[2]->getColor();
  lc.bg_s = colorEdit[3]->getColor();
}
//-----------------------------------------------------------
void svmDialogAlarm::refreshLb()
{
  needRefreshLb = true;
  updateData();
  SetTimer(*this, (idTimer = 1212), 100, 0);
}
//-----------------------------------------------------------
LRESULT svmDialogAlarm::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_INITDIALOG:
      SetFocus(GetDlgItem(hwnd, IDC_EDIT_AL_COLOR_R_FG));
      return FALSE;
    case WM_DESTROY:
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      break;
    case WM_TIMER:
      if(needRefreshLb) {
        needRefreshLb = false;
        InvalidateRect(GetDlgItem(*this, IDC_LIST_AL_COLOR_SAMPLE), 0, 1);
        }
      break;

    case WM_CUSTOM_SET_FOCUS:
      do {
        HWND focus = GetFocus();
        for(uint i = 0; i < SIZE_A(colorEdit); ++i)
          colorEdit[i]->setFocus(idcTrackBar, focus);
        checkChangeEdit(wParam);
        } while(false);
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_LIST_AL_COLOR_SAMPLE:
          if(LBN_SELCHANGE == HIWORD(wParam))
            selChgColor();
          break;
        }
      switch(HIWORD(wParam)) {
        case EN_CHANGE:
          checkChangeEdit(LOWORD(wParam));
          break;
        }
      break;
    case WM_HSCROLL:
      evHScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
      break;
/*
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
*/
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
static void verifyTrueOrder(uint set[], uint nelem)
{
  DWORD t = 0;
  for(uint i = 0; i < nelem; ++i) {
    if(!set[i] || set[i] > nelem)
      set[i] = i + 1;
    if(t & (1 << (set[i] - 1))) {
      for(uint j = 0; j < nelem; ++j) {
        uint k = (j + i + 1) % nelem;
        if(!(t & (1 << k))) {
          t |= 1 << k;
          set[i] = k + 1;
          break;
          }
        }
      }
    else
      t |= 1 << (set[i] - 1);
    }
}
//-----------------------------------------------------------
void svmDialogAlarm::CmOk()
{
  PropertyAlarm* pt = dynamic_cast<PropertyAlarm*>(tmpProp);
  if(pt) {

    HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_FLT);
    pt->DataPrf[0].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    GET_INT(IDC_EDIT_ADDR_FLT, pt->DataPrf[0].addr);

    hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_FLT_GRP);
    pt->DataPrf[1].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    GET_INT(IDC_EDIT_ADDR_FLT_GRP, pt->DataPrf[1].addr);

    hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_PERIFS_FLT_TXT);
    pt->DataPrf[2].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    GET_INT(IDC_EDIT_ADDR_FLT_TXT, pt->DataPrf[2].addr);
    GET_INT(IDC_EDIT_ADDR_FLT_TXT_LEN, pt->DataPrf[2].dec);
    }

  uint idcsW[] = {
    IDC_EDIT_AL_PERC_1,
    IDC_EDIT_AL_PERC_2,
    IDC_EDIT_AL_PERC_3,
    IDC_EDIT_AL_PERC_4,
    IDC_EDIT_AL_PERC_5,
    IDC_EDIT_AL_PERC_6,
    IDC_EDIT_AL_PERC_7,
    IDC_EDIT_AL_PERC_8,
    IDC_EDIT_AL_PERC_9,
    };
  for(uint i = 0; i < SIZE_A(pt->percWidth); ++i)
    GET_INT(idcsW[i], pt->percWidth[i]);

  uint idcsP[] = {
    IDC_EDIT_AL_POS_1,
    IDC_EDIT_AL_POS_2,
    IDC_EDIT_AL_POS_3,
    IDC_EDIT_AL_POS_4,
    IDC_EDIT_AL_POS_5,
    IDC_EDIT_AL_POS_6,
    IDC_EDIT_AL_POS_7,
    IDC_EDIT_AL_POS_8,
    IDC_EDIT_AL_POS_9,
    };
  for(uint i = 0; i < SIZE_A(pt->columnPos); ++i)
    GET_INT(idcsP[i], pt->columnPos[i]);

  verifyTrueOrder(pt->columnPos, SIZE_A(pt->columnPos));

  uint idcs[] = {
    IDC_CHECK_AL_ID,
    IDC_CHECK_AL_STAT,
    IDC_CHECK_AL_DATE,
    IDC_CHECK_AL_TIME,
    IDC_CHECK_AL_FILTER,
    IDC_CHECK_AL_ID_GRP,
    IDC_CHECK_AL_DESCR_GRP,
    IDC_CHECK_AL_PRPH,
    IDC_CHECK_AL_DESCR_AL,

    IDC_CHECK_AL_USE_LBOX,
    };

  tmpProp->PA_def_show = 0;
  for(uint i = 0; i < SIZE_A(idcs); ++i) {
    if(IS_CHECKED(idcs[i]))
      tmpProp->PA_def_show |= 1 << i;
    }
  GET_INT(IDC_EDIT_SIMPLE_TEXT, tmpProp->PA_fix_filter);


  baseClass::CmOk();
}
//----------------------------------------------------------------------------
HBRUSH svmDialogAlarm::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    for(uint i = 0; i < SIZE_A(colorEdit); ++i) {
      PColorBox* box = colorEdit[i]->getBox();
      if(*box == hWndChild) {
        SetBkColor(hdc, box->getColor());
        return box->getBrush();
        }
      }
    if(*CurrColor == hWndChild) {
      SetBkColor(hdc, CurrColor->getColor());
      return CurrColor->getBrush();
      }
    if(GetDlgItem(*this, IDC_STATIC_CURR_AL_TEXT) == hWndChild) {
      SetTextColor(hdc, RGB(0, 0, 180));
      SetBkColor(hdc, INFO_COLOR);
      return hBrInfo;
      }
    }
  return 0;
}
//------------------------------------------------------------------------------
void svmDialogAlarm::evHScrollBar(HWND child, int flags, int pos)
{
  int curr = SendMessage(child, TBM_GETPOS, 0, 0);
  switch(flags) {
    case TB_THUMBPOSITION:
    case TB_THUMBTRACK:
      break;
    case TB_ENDTRACK:
      pos = curr;
      break;
    case TB_TOP:
      pos = 0;
      break;
    case TB_BOTTOM:
      pos = 255;
      break;
    case TB_LINEUP:
      pos = curr - 1;
      break;
    case TB_LINEDOWN:
      pos = curr + 1;
      break;
    case TB_PAGEUP:
      pos = curr - 16;
      break;
    case TB_PAGEDOWN:
      pos = curr + 16;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if(pos > 255)
    pos = 255;
  SendMessage(child, TBM_SETPOS, true, pos);
//  SendMessage(child, TBM_SETPOS, pos, true);

  int val[3];
  for(int i = 0; i < 3; ++i) {
    HWND hsb = GetDlgItem(*this, idcTrackBar[i]);
    val[i] = SendMessage(hsb, TBM_GETPOS, 0, 0);
    }
  P_SimpleColorRow* cr = 0;
  for(uint i = 0; i < SIZE_A(colorEdit); ++i) {
    if(colorEdit[i]->hasFocus()) {
      cr = colorEdit[i];
      break;
      }
    }
  if(cr) {
    cr->setColor(RGB(val[0], val[1], val[2]));
    CurrColor->set(RGB(val[0], val[1], val[2]));
    }
  refreshLb();
}
//-----------------------------------------------------------
void svmDialogAlarm::checkChangeEdit(uint idc)
{
  for(uint i = 0; i < SIZE_A(alColors); ++i) {
    for(uint j = 0; j < SIZE_A(alColors[i].idcE); ++j) {
      if(idc == alColors[i].idcE[j]) {
        colorEdit[i]->setColorBySelf(idcTrackBar);
        setVirtualFocus(i >= SIZE_A(alColors) / 2 ? lastSel : -1);
        setCurrColor(colorEdit[i]->getColor());
        refreshLb();
        return;
        }
      }
    }
}
//-----------------------------------------------------------
void svmDialogAlarm::setCurrColor(COLORREF c)
{
  CurrColor->set(c);
}
//-----------------------------------------------------------
void svmDialogAlarm::setVirtualFocus(int set)
{
  HWND hw = GetDlgItem(*this, IDC_LIST_AL_COLOR_SAMPLE);
  myListBox_Alarm* lba = dynamic_cast<myListBox_Alarm*>(PWin::getWindowPtr(hw));
  lba->setVirtualFocus(set);
}
//-----------------------------------------------------------
void svmDialogAlarm::selChgColor()
{
  int sel = SendMessage(GetDlgItem(*this, IDC_LIST_AL_COLOR_SAMPLE), LB_GETCURSEL, 0, 0);
  if(sel < 0 || sel == lastSel)
    return;
  lastSel = sel;
  PropertyAlarm* pt = dynamic_cast<PropertyAlarm*>(tmpProp);
  if(!pt)
    return;
  onMyModify = true;
  PropertyAlarm::lineColor& lCol = pt->lColor[sel];
  colorEdit[0]->setColor(lCol.fg_n);
  colorEdit[1]->setColor(lCol.bg_n);
  colorEdit[2]->setColor(lCol.fg_s);
  colorEdit[3]->setColor(lCol.bg_s);
  SendMessage(*this, WM_CUSTOM_SET_FOCUS, alColors[0].idcE[0], 0);
  colorEdit[2]->setFocus(idcTrackBar);
  CurrColor->set(colorEdit[2]->getColor());

  setVirtualFocus(lastSel);
  SET_TEXT(IDC_STATIC_CURR_AL_TEXT, getStrAlarm(lastSel));
  onMyModify = false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//----------------------------------------------------------------------------
P_SimpleColorRow::P_SimpleColorRow(PWin* parent, uint idcStatic, uint* idcEdit) :
    Focus(false)
{
  for(uint i = 0; i < SIZE_A(rgb); ++i) {
    rgb[i] = new P_EditFocus(parent, idcEdit[i]);
    rgb[i]->setFilter(new PRGBFilter);
    }
  cbx = new PColorBox(parent, 0, idcStatic);
}
//----------------------------------------------------------------------------
P_SimpleColorRow::checkFocus P_SimpleColorRow::setFocus(uint* IdSB, HWND hwnd)
{
  bool focus = hwnd ? false : true;
  if(!focus)
    for(uint i = 0; i < SIZE_A(rgb); ++i)
      if(hwnd == *rgb[i]) {
        focus = true;
        break;
        }
  if(!focus && Focus) {
    Focus = false;
    return P_SimpleColorRow::LOSS;
    }

  if(!hwnd || focus && !Focus) {
    PWin* parent = rgb[0]->getParent();
    for(int i = 0; i < 3; ++i) {
      HWND hsb = GetDlgItem(*parent, IdSB[i]);
      int val[3];
      if(hsb) {
        TCHAR buff[20];
        GetWindowText(*rgb[i], buff, SIZE_A(buff));
        val[i] = _ttoi(buff);
        SendMessage(hsb, TBM_SETPOS, true, val[i]);
        }
      svmDialogAlarm* sda = dynamic_cast<svmDialogAlarm*>(parent);
      if(sda)
        sda->setCurrColor(RGB(val[0], val[1], val[2]));
      }
    Focus = true;
    return P_SimpleColorRow::GET;
    }
  return P_SimpleColorRow::NOCHANGE;
}
//----------------------------------------------------------------------------
COLORREF P_SimpleColorRow::getColor()
{
  int val[3];
  TCHAR buff[10];
  for(int i = 0; i < 3; ++i) {
    GetWindowText(*rgb[i], buff, SIZE_A(buff));
    val[i] = _ttoi(buff);
    }
  return RGB(val[0], val[1], val[2]);
}
//----------------------------------------------------------------------------
void P_SimpleColorRow::setColorBySelf(uint* IdSB)
{
  int val[3];
  TCHAR buff[10];
  PWin* parent = rgb[0]->getParent();
  for(int i = 0; i < 3; ++i) {
    GetWindowText(*rgb[i], buff, SIZE_A(buff));
    val[i] = _ttoi(buff);

    HWND hsb = GetDlgItem(*parent, IdSB[i]);
    if(hsb)
      SendMessage(hsb, TBM_SETPOS, true, val[i]);
    }
  cbx->set(RGB(val[0], val[1], val[2]));
}
//----------------------------------------------------------------------------
void P_SimpleColorRow::setColor(COLORREF color)
{
  int val[3] = {
      GetRValue(color),
      GetGValue(color),
      GetBValue(color)
      };
  TCHAR buff[10];
  for(int i = 0; i < 3; ++i) {
    wsprintf(buff, _T("%d"), val[i]);
    SetWindowText(*rgb[i], buff);
    }
  cbx->set(color);
}
//----------------------------------------------------------------------------
void P_SimpleColorRow::setAll(LPCTSTR p)
{
  int r = 0;
  int g = 0;
  int b = 0;
  _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
  setColor(RGB(r, g, b));
}
//----------------------------------------------------------------------------
