//----------- LimitsDlg.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "LimitsDlg.h"
#include "svEdit.h"
#include "pStatic.h"
#include "p_util.h"
#include "p_file.h"
#include "POwnBtnImage.h"
#include "id_msg_common.h"
//----------------------------------------------------------------------------
#define PAGE_SCROLL_LEN  MAX_GROUP
#define MAX_V_SCROLL (maxShow - MAX_GROUP)
//#define MAX_V_SCROLL (MAX_ADDRESSES - MAX_GROUP)
//----------------------------------------------------------------------------
#define WM_CUSTOM_VSCROLL (WM_APP + 101)
//----------------------------------------------------------------------------
#define IS_CHECKED(id) (BST_CHECKED == SendMessage(GetDlgItem(*this, (id)), BM_GETCHECK, 0, 0))
//----------------------------------------------------------------------------
#define OFFSET_V_MIN   0
#define OFFSET_V_MAX   (OFFSET_V_MIN + 100)
#define OFFSET_DESCR  (OFFSET_V_MAX + 100)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_EDIT_fREALDATA 12
//----------------------------------------------------------------------------
class PRow_ld
{
  public:

    PRow_ld(PWin* parent, uint first_id);
    virtual ~PRow_ld() {}

    void getDescr(LPTSTR buff) const;
    void setDescr(LPCTSTR buff) const;
    fREALDATA getMin();
    void setMin(fREALDATA v);
    fREALDATA getMax();
    void setMax(fREALDATA v);

    void getAll(fREALDATA& vmin, fREALDATA& vmax);
    void setAll(fREALDATA vmin, fREALDATA vmax);
    void disable();
  protected:
    svEdit* eMin;
    svEdit* eMax;
    PStatic* Descr;

    uint firstId;

    fREALDATA getGen(PEdit* ctrl);
    void setGen(PEdit* ctrl, fREALDATA val);
};
//----------------------------------------------------------------------------
class PageEdit_ld : public svEdit
{
  public:
    PageEdit_ld(PWin * parent, uint resid, int textlen = 255, HINSTANCE hinst = 0) :
          svEdit(parent, resid, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
bool PageEdit_ld::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_NEXT:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
      break;
    case VK_PRIOR:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
      break;
    }
  return svEdit::evKeyDown(key);
}
//----------------------------------------------------------------------------
class firstEdit_ld : public PageEdit_ld
{
  public:
    firstEdit_ld(PWin * parent, uint resid, int textlen = 255, HINSTANCE hinst = 0) :
          PageEdit_ld(parent, resid, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
bool firstEdit_ld::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return PageEdit_ld::evKeyDown(key);
}
//----------------------------------------------------------------------------
class lastEdit_ld : public PageEdit_ld
{
  public:
    lastEdit_ld(PWin * parent, uint resid, int textlen = 255, HINSTANCE hinst = 0) :
          PageEdit_ld(parent, resid, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
bool lastEdit_ld::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
    PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
    return true;
    }
  return PageEdit_ld::evKeyDown(key);
}
//----------------------------------------------------------------------------
class PFirstRow_ld : public PRow_ld
{
  public:

    PFirstRow_ld(PWin* parent, uint first_id) : PRow_ld(parent, first_id)
    {
      delete eMin;
      eMin = new firstEdit_ld(parent, firstId, MAX_EDIT_fREALDATA);
    }
};
//----------------------------------------------------------------------------
class PLastRow_ld : public PRow_ld
{
  public:

    PLastRow_ld(PWin* parent, uint first_id) : PRow_ld(parent, first_id)
    {
      delete eMax;
      eMax = new lastEdit_ld(parent, firstId + OFFSET_V_MAX, MAX_EDIT_fREALDATA);
    }
};
//----------------------------------------------------------------------------
PRow_ld::PRow_ld(PWin* parent, uint first_id) :
    firstId(first_id)
{
  eMin = new PageEdit_ld(parent, firstId, MAX_EDIT_fREALDATA);
  eMax = new PageEdit_ld(parent, firstId + OFFSET_V_MAX, MAX_EDIT_fREALDATA);
  Descr = new PStatic(parent, firstId + OFFSET_DESCR);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
void PRow_ld::getDescr(LPTSTR buff) const
{
  GetWindowText(*Descr, buff, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
inline
void PRow_ld::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
}
//----------------------------------------------------------------------------
inline
fREALDATA PRow_ld::getGen(PEdit* ctrl)
{
  TCHAR buff[50];
  GetWindowText(*ctrl, buff, SIZE_A(buff));
  return (fREALDATA)_tstof(buff);
}
//----------------------------------------------------------------------------
inline
void PRow_ld::setGen(PEdit* ctrl, fREALDATA val)
{
  TCHAR buff[50];
  _stprintf_s(buff, SIZE_A(buff), _T("%0.4f"), val);
  Z_END(buff);

  SetWindowText(*ctrl, buff);
}
//----------------------------------------------------------------------------
inline
fREALDATA PRow_ld::getMin()
{
  return getGen(eMin);
}
//----------------------------------------------------------------------------
inline
void PRow_ld::setMin(fREALDATA val)
{
  setGen(eMin, val);
}
//----------------------------------------------------------------------------
inline
fREALDATA PRow_ld::getMax()
{
  return getGen(eMax);
}
//----------------------------------------------------------------------------
inline
void PRow_ld::setMax(fREALDATA val)
{
  setGen(eMax, val);
}
//----------------------------------------------------------------------------
#define IS_CHANGED(hCtrl) SendMessage(hCtrl, EM_GETMODIFY, 0, 0)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
void PRow_ld::getAll(fREALDATA& vmin, fREALDATA& vmax)
{
  vmin = getMin();
  vmax = getMax();
}
//----------------------------------------------------------------------------
inline
void PRow_ld::setAll(fREALDATA vmin, fREALDATA vmax)
{
  setMin(vmin);
  setMax(vmax);
}
//----------------------------------------------------------------------------
inline
void PRow_ld::disable()
{
  EnableWindow(*eMin, false);
  EnableWindow(*eMax, false);
  SetWindowText(*eMin, _T(""));
  SetWindowText(*eMax, _T(""));
  SetWindowText(*Descr, _T(""));
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
#define CYAN_COLOR  RGB(0xcf,0xcf,0xff)
#define bkgColor2 CYAN_COLOR
//----------------------------------------------------------------------------
PD_Limits::PD_Limits(LPCTSTR filename, setOfString& sos, PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  currPos(0),
    dirty(false), maxShow(0), scrollByEdit(false),
    Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
    FileName(filename), SOS(sos)
{
  int idBmp[] = { IDB_DONE, IDB_SAVE, };
  int idBtn[] = { IDCANCEL, IDC_BUTTON_SAVE, };

  PVect<PBitmap*> vBmp;
  for(int i = 0; i < SIZE_A(Bmp); ++i) {
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());
    vBmp[0] = Bmp[i];
    new POwnBtnImage(this, idBtn[i], vBmp, false);
    }

  P_File f(FileName, P_READ_ONLY);
  if(f.P_open())
    f.P_read(&rSet, sizeof(rSet));
  else
    memset(&rSet, 0, sizeof(rSet));

  LPCTSTR pNameTrend = SOS.getString(ID_TREND_NAME);
  uint tot;
  uint id = getInitDataByNameTrend(pNameTrend, tot);
  if(id) {
    for(uint i = 0; i < tot; ++i) {
      smartPointerConstString pt = getDescrDataTrend(i + id);

      copyStr(Cfg[maxShow].text, &pt, MAX_DESCR);
      Cfg[maxShow].text[MAX_DESCR] = 0;
      Cfg[maxShow].vMin = rSet.rMin[maxShow];
      Cfg[maxShow].vMax = rSet.rMax[maxShow];
      ++maxShow;
      }
    }
}
//----------------------------------------------------------------------------
PD_Limits::~PD_Limits()
{
  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
  for(int i = 0; i < SIZE_A(Bmp); ++i)
    delete Bmp[i];
  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
}
//----------------------------------------------------------------------------
bool PD_Limits::create()
{
  int i = 1;
  if(maxShow <= MAX_GROUP)
    i = 0;
  else
    Rows[0] = new PFirstRow_ld(this, IDC_EDIT_V_MIN_1);

  int totRow = min(maxShow, MAX_GROUP);
  for(; i < totRow - 1; ++i)
    Rows[i] = new PRow_ld(this, IDC_EDIT_V_MIN_1 + i);

  if(maxShow <= MAX_GROUP)
    Rows[i] = new PRow_ld(this, IDC_EDIT_V_MIN_1 + i);
  else
    Rows[i] = new PLastRow_ld(this, IDC_EDIT_V_MIN_1 + i);

  for(++i; i < MAX_GROUP; ++i)
    Rows[i] = new PRow_ld(this, IDC_EDIT_V_MIN_1 + i);

  if(!baseClass::create())
    return false;

  for(i = totRow; i < MAX_GROUP; ++i)
    Rows[i]->disable();

  loadData();
  setScrollRange();

  if(maxShow <= MAX_GROUP)
    EnableWindow(GetDlgItem(*this, IDC_SCROLLBAR_COUNT_NUM), false);

  setTitle();

  HWND firstEdit_ld = GetDlgItem(*this, IDC_EDIT_V_MIN_1);
  SetFocus(firstEdit_ld);
  return true;
}
//----------------------------------------------------------------------------
void PD_Limits::setScrollRange()
{
  HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_COUNT_NUM);
  if(hsb) {
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nPage = PAGE_SCROLL_LEN;
    si.nMin = 0;
    si.nMax = maxShow - 1;
    SendMessage(hsb, SBM_SETSCROLLINFO, 1, (LPARAM)&si);
    }
}
//----------------------------------------------------------------------------
/**/
// Il dialogbox non si serve del ciclo dei messaggi gestito dal pAppl, ma ha un
// suo ciclo interno quindi questa routine è inutile.
// A meno di creare il dialogo modeless e gestire il ciclo dei messaggi, vedere
// nel file pDialoh.h un esempio
bool PD_Limits::preProcessMsg(MSG& msg)
{
  switch(msg.message) {
    case WM_KEYDOWN:
      switch(msg.wParam) {
        case VK_NEXT:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
          break;
        case VK_PRIOR:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
          break;
        case VK_HOME:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_LEFT, 0);
          break;
        case VK_END:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_RIGHT, 0);
          break;
        }
      break;
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
/**/
//----------------------------------------------------------------------------
LRESULT PD_Limits::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
//        case IDOK:
//          return 0;
        case IDC_BUTTON_F1:
        case IDCANCEL:
          if(!saveOnExit())
            return 0;
          EndDialog(getHandle(), IDCANCEL);
          break;

        case IDC_BUTTON_F2:
        case IDC_BUTTON_SAVE:
          saveData();
          break;
        }
      switch(HIWORD(wParam)) {
        case EN_CHANGE:
          dirty = true;
          break;
        }
      break;

    case WM_CUSTOM_VSCROLL:
      lParam = (LPARAM)GetDlgItem(*this, IDC_SCROLLBAR_COUNT_NUM);
      scrollByEdit = true;
      // passa sotto
    case WM_VSCROLL:
      evVScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
      break;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
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
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool PD_Limits::saveOnExit()
{
  if(dirty) {
    int choose = svMessageBox(this, _T("Do you want to save changed data?"),
                                   _T("Data is changed"), MB_YESNOCANCEL | MB_ICONINFORMATION);
    switch(choose) {
      case IDCANCEL:
        return false;
      case IDYES:
        if(!saveData())
          return false;
        break;
      }
    }
  return true;
}
//------------------------------------------------------------------------------
void PD_Limits::evVScrollBar(HWND child, int flags, int pos)
{
  if(!IsWindowEnabled(child))
    return;
  int curr = SendMessage(child, SBM_GETPOS, 0, 0);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = MAX_V_SCROLL;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - PAGE_SCROLL_LEN;
      break;
    case SB_PAGERIGHT:
      pos = curr + PAGE_SCROLL_LEN;
      break;
    }
  if(pos < 0) {
    if(scrollByEdit) {
      shift_click(VK_TAB);
      return;
      }
    else
      pos = 0;
    }
  else if(pos > MAX_V_SCROLL) {
    if(scrollByEdit) {
      click(VK_TAB);
      return;
      }
    else
      pos = MAX_V_SCROLL;
    }
//  else if(pos > maxShow - MAX_GROUP)//MAX_V_SCROLL)
//    return;
  SendMessage(child, SBM_SETPOS, pos, true);

  saveCurrData();
  currPos = pos;
  loadData();
  scrollByEdit = false;
}
//------------------------------------------------------------------------------
bool PD_Limits::saveData()
{
  saveCurrData();

  P_File file(FileName, P_CREAT);
  if(file.P_open()) {
    for(int i = 0; i < maxShow; ++i) {
      rSet.rMin[i] = Cfg[i].vMin;
      rSet.rMax[i] = Cfg[i].vMax;
      }
    file.P_write(&rSet, sizeof(rSet));
    dirty = false;
    loadData();
    return true;
    }
  return false;
}
//------------------------------------------------------------------------------
void PD_Limits::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    Cfg[pos].vMin = Rows[i]->getMin();
    Cfg[pos].vMax = Rows[i]->getMax();
    Rows[i]->getDescr(Cfg[pos].text);
    }
}
//------------------------------------------------------------------------------
void PD_Limits::loadData()
{
  int pos = currPos;

  bool oldDirty = dirty;
//  int focus = 0;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos + 1);
    SetDlgItemText(*this, IDC_STATICTEXT_NUM_1 + i, buff);

    Rows[i]->setAll(Cfg[pos].vMin, Cfg[pos].vMax);
    Rows[i]->setDescr(Cfg[pos].text);
    }
  dirty = oldDirty;
}
//----------------------------------------------------------------------------
HBRUSH PD_Limits::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    for(int i = 0; i < MAX_GROUP && i < maxShow; ++i) {
      if(GetDlgItem(*this, IDC_STATIC_DESCR_LIMITS_1 + i) == hWndChild) {
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
      }
    return 0;
    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, bkgColor);
    return (Brush);
    }
  return 0;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define DEF_TITLE _T("Address converter")
void PD_Limits::setTitle()
{
//  TCHAR title[_MAX_PATH * 2];
//  wsprintf(title, _T("%s - %s"), DEF_TITLE, Filename);
//  setCaption(title);
}
//------------------------------------------------------------------------------
