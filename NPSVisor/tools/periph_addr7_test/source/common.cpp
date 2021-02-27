//----------- common.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
//----------------------------------------------------------------------------
static bool IgnoreKillFocus;
//----------------------------------------------------------------------------
bool setIgnoreKillFocus(bool set)
{
  bool old = IgnoreKillFocus;
  IgnoreKillFocus = set;
  return old;
}
//----------------------------------------------------------------------------
bool canIgnoreKillFocus()
{
  return IgnoreKillFocus;
}
//----------------------------------------------------------------------------
static int getDec(TCHAR car)
{
  if(car >= _T('0') && car <= _T('9'))
    return car - _T('0');
  if(car >= _T('a') && car <= _T('f'))
    return car - _T('a') + 10;
  if(car >= _T('A') && car <= _T('F'))
    return car - _T('A') + 10;
  return 0;
}
//----------------------------------------------------------------------------
static int hexToInt(LPCTSTR buff)
{
  int code = 0;
  int len = _tcslen(buff);
  for(int i = len - 1, j = 0; i >= 0; --i, ++j) {
    int val = getDec(buff[i]);
    val <<= j * 4;
    code += val;
    }
  return code;
}
//----------------------------------------------------------------------------
static void intToHex(LPTSTR buff, int val, bool isWord)
{
  if(isWord)
    wsprintf(buff, _T("%04X"), val);
  else
    wsprintf(buff, _T("%02X"), val);
}
//----------------------------------------------------------------------------
static int binToInt(LPCTSTR buff)
{
  int code = 0;
  int len = _tcslen(buff);
  for(int i = len - 1, j = 0; i >= 0; --i) {
    if(_T('1') == buff[i] || _T('0') == buff[i]) {
      if(_T('1') == buff[i])
        code |= 1 << j;
      ++j;
      }
    }
  return code;
}
//----------------------------------------------------------------------------
static void intToBin(LPTSTR buff, int val, int len)
{
  #define MAX_L_BIN 32
  int j = 0;
  buff += len + (len / 4) - !(len % 4);
  *buff-- = 0;
  for(int i = len - 1; i >= 0; --i, ++j) {
    if(j && !(j & 3))
      *buff-- = _T('.');
    if(val & 1)
      *buff-- = _T('1');
    else
      *buff-- = _T('0');
    val >>= 1;
    }
}
//-----------------------------------------------------------------------------
bool PEditCR::evKeyDown(UINT& key)
{
  if(VK_RETURN == key)
    PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(cmdid, 0), 0);
  return false;
}
//----------------------------------------------------------------------------
LRESULT PEditEmpty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_KILLFOCUS:
      invalidateNum();
      break;
    case WM_SETFOCUS:
      SendMessage(hwnd, EM_SETSEL, 0, (LPARAM)-1);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PEditEmpty::invalidateNum() const
{
  if(!canIgnoreKillFocus() && IS_THIS_CHANGED()) {
    uint ix = Attr.id  - IDC_EDIT_BYTE1_1;
    ix %= DEF_OFFSET_CTRL;
    HWND hw = GetDlgItem(*getParent(), ix + IDC_STATICTEXT_1);
    if(hw)
      InvalidateRect(hw, 0, 0);
    InvalidateRect(*this, 0, 1);
    if(alsoSibling) {
      hw = GetDlgItem(*getParent(), Attr.id + alsoSibling);
      if(hw)
        InvalidateRect(hw, 0, 1);
      }
    }
}
//----------------------------------------------------------------------------
static HWND isMyEditCtrl(HWND hwnd)
{
  PWin* w = PWin::getWindowPtr(hwnd);
  return dynamic_cast<PageEdit*>(w) ? hwnd : 0;
}
//----------------------------------------------------------------------------
bool PageEdit::evKeyDown(UINT& key)
{
  switch(key) {
    case VK_NEXT:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
      return true;
    case VK_PRIOR:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
      return true;
    case VK_UP:
      do {
        uint ix = Attr.id  - 1;
        HWND hw = isMyEditCtrl(GetDlgItem(*getParent(), ix));
        if(hw)
          SetFocus(hw);
        } while(false);
      return true;
    case VK_DOWN:
    case VK_RETURN:
      do {
        uint ix = Attr.id  + 1;
        HWND hw = isMyEditCtrl(GetDlgItem(*getParent(), ix));
        if(hw)
          SetFocus(hw);
        } while(false);
      return true;
    case VK_LEFT:
      if(Attr.id  - DEF_OFFSET_CTRL > IDC_EDIT_BYTE1_1) {
        LRESULT sel = SendMessage(*this, EM_GETSEL, 0, 0);
        if(-1 == sel)
          break;
        int init = LOWORD(sel);
        int end  = HIWORD(sel);
        if(!init && !end) {
          uint ix = Attr.id  - DEF_OFFSET_CTRL;
          HWND hw = isMyEditCtrl(GetDlgItem(*getParent(), ix));
          if(hw) {
            SetFocus(hw);
            return true;
            }
          }
        }
      break;
    case VK_RIGHT:
      do {
        LRESULT sel = SendMessage(*this, EM_GETSEL, 0, 0);
        if(-1 == sel)
          break;
        int init = LOWORD(sel);
        int end  = HIWORD(sel);
        sel = SendMessage(*this, WM_GETTEXTLENGTH, 0, 0);

        if(init == end && init == sel) {
          uint ix = Attr.id  + DEF_OFFSET_CTRL;
          HWND hw = isMyEditCtrl(GetDlgItem(*getParent(), ix));
          if(hw) {
            SetFocus(hw);
            return true;
            }
          }
        } while(false);
      break;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT clickEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      if(MK_CONTROL & wParam) {
        TCHAR buff[64];
        TCHAR old[64];
        bool isMod = setIgnoreKillFocus(true);
        GetWindowText(*this, buff, SIZE_A(buff));
        wsprintf(old, _T("[%s]"), buff);
        SetWindowText(*this, old);
        setIgnoreKillFocus(isMod);
        _tcscpy(old, buff);
        SendMessage(*this, EM_SETSEL, 0, (LPARAM) -1);
        if(performChoose(buff) && _tcsicmp(buff, old)) {
          SetWindowText(*this, buff);
          SendMessage(*this, EM_SETMODIFY, TRUE, 0);
          PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(Attr.id, EN_CHANGE), (LPARAM)getHandle());
          invalidateNum();
          }
        else {
          isMod = setIgnoreKillFocus(true);
          SetWindowText(*this, old);
          SendMessage(*this, EM_SETMODIFY, FALSE, 0);
          setIgnoreKillFocus(isMod);
          }
        SendMessage(*this, EM_SETSEL, -1, (LPARAM) -1);
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//#define bkg_Color RGB(0xbc, 0xfe, 0xf0)
#define bkg_Color1 0xC1E1D8
#define bkg_Color2 0xD8E1C1
//----------------------------------------------------------------------------
class genShow : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    genShow(PWin* parent, LPTSTR buff, uint idd, COLORREF color) :
      baseClass(parent, idd), Buff(buff), Color(color), Brush(CreateSolidBrush(color))
      {
        int idBmp[] = { IDB_OK, IDB_CANC };
        int idBtn[] = { IDOK, IDCANCEL };

        HINSTANCE hi = getHInstance();
        POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0, 0, 0xff), 1, GetSysColor(COLOR_BTNTEXT));
        for(uint i = 0; i < SIZE_A(idBmp); ++i) {
          PBitmap* Bmp = new PBitmap(idBmp[i], hi);
          POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], Bmp,  POwnBtnImageStd::wLeft, true);
          btn->setColorRect(cr);
          }
        enableCapture(true);
      }

    ~genShow()
    {
      destroy();
      DeleteObject(Brush);
    }

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    LPTSTR Buff;
    COLORREF Color;
    HBRUSH Brush;
};
//----------------------------------------------------------------------------
LRESULT genShow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
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
    case WM_NCHITTEST:
      SetWindowLong(*this, DWL_MSGRESULT, HTCAPTION);
      return TRUE;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HBRUSH genShow::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  SetBkColor(hdc, Color);
  return (Brush);
}
//----------------------------------------------------------------------------
class ShowType :public genShow
{
  private:
    typedef genShow baseClass;
  public:
    ShowType(PWin* parent, LPTSTR buff) :
      baseClass(parent, buff, IDD_DIALOG_SHOW_TYPE, bkg_Color1) {}

    ~ShowType()
    {
      destroy();
    }

    bool create();
  protected:
    void CmOk();
};
//----------------------------------------------------------------------------
bool ShowType::create()
{
  if(!baseClass::create())
    return false;
  uint v = _ttoi(Buff);
  SET_CHECK(IDC_CHECK_TYPE1 + v);
  return true;
}
//----------------------------------------------------------------------------
void ShowType::CmOk()
{
  for(uint i = 0; i <= tStrData; ++i) {
    if(IS_CHECKED(IDC_CHECK_TYPE1 + i)) {
      wsprintf(Buff, _T("%d"), i);
      break;
      }
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
class ShowAction :public genShow
{
  private:
    typedef genShow baseClass;
  public:
    ShowAction(PWin* parent, LPTSTR buff) :
      baseClass(parent, buff, IDD_DIALOG_SHOW_ACTION, bkg_Color2) {}

    ~ShowAction()
    {
      destroy();
    }

    bool create();
  protected:
    void CmOk();
};
//----------------------------------------------------------------------------
bool ShowAction::create()
{
  if(!baseClass::create())
    return false;
  uint v = _ttoi(Buff);
  if(v & 1)
    SET_CHECK(IDC_CHECK_READ);
  if(v & 2)
    SET_CHECK(IDC_CHECK_WRITE);
  if(v & 4)
    SET_CHECK(IDC_CHECK_REQUEST);
  if(v & 8)
    SET_CHECK(IDC_CHECK_SETUP);
  return true;
}
//----------------------------------------------------------------------------
void ShowAction::CmOk()
{
  uint v = 0;
  if(IS_CHECKED(IDC_CHECK_READ))
    v |= 1;
  if(IS_CHECKED(IDC_CHECK_WRITE))
    v |= 2;
  if(IS_CHECKED(IDC_CHECK_REQUEST))
    v |= 4;
  if(IS_CHECKED(IDC_CHECK_SETUP))
    v |= 8;

  wsprintf(Buff, _T("%d"), v);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool clickEditType::performChoose(LPTSTR buff)
{
  return IDOK == ShowType(this, buff).modal();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool clickEditAction::performChoose(LPTSTR buff)
{
  return IDOK == ShowAction(this, buff).modal();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool firstEdit::evKeyDown(UINT& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool lastEdit::evKeyDown(UINT& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool clickEditTypeFirst::evKeyDown(UINT& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool clickEditActionFirst::evKeyDown(UINT& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool clickEditTypeLast::evKeyDown(UINT& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool clickEditActionLast::evKeyDown(UINT& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
PEditEmpty* makeFactory(PWin* parent, uint id, const PRect& r, int len, int whichRow)
{
  switch(whichRow) {
    case -1:
      return new firstEdit(parent, id, r, len);
    case 1:
      return new lastEdit(parent, id, r, len);
    default:
      return new PageEdit(parent, id, r, len);
    }
}
//----------------------------------------------------------------------------
PEditEmpty* makeFactoryType(PWin* parent, uint id, const PRect& r, int len, int whichRow)
{
  switch(whichRow) {
    case -1:
      return new clickEditTypeFirst(parent, id, r, len);
    case 1:
      return new clickEditTypeLast(parent, id, r, len);
    default:
      return new clickEditType(parent, id, r, len);
    }
}
//----------------------------------------------------------------------------
PEditEmpty* makeFactoryAction(PWin* parent, uint id, const PRect& r, int len, int whichRow)
{
  switch(whichRow) {
    case -1:
      return new clickEditActionFirst(parent, id, r, len);
    case 1:
      return new clickEditActionLast(parent, id, r, len);
    default:
      return new clickEditAction(parent, id, r, len);
    }
}
//----------------------------------------------------------------------------
PRow::PRow(PWin* parent, uint first_id, POINT pt, int whichRow) :
    firstId(first_id), realAddr(0)
{
  PRect r(0, 0, W_BYTE, H_EDIT);
  r.MoveTo(pt.x, pt.y);
  Byte1 = makeFactory(parent, firstId, r, MAX_EDIT_BYTE, whichRow);
  r.Offset(r.Width(), 0);
  Byte2 = makeFactory(parent, firstId + OFFSET_BYTE2, r, MAX_EDIT_BYTE, whichRow);
  r.Offset(r.Width(), 0);
  Byte3 = makeFactory(parent, firstId + OFFSET_BYTE3, r, MAX_EDIT_BYTE, whichRow);
  r.Offset(r.Width(), 0);
  Byte4 = makeFactory(parent, firstId + OFFSET_BYTE4, r, MAX_EDIT_BYTE, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_PORT;
  Port = makeFactory(parent, firstId + OFFSET_PORT, r, MAX_EDIT_DWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_DB;
  Db = makeFactory(parent, firstId + OFFSET_DB, r, MAX_EDIT_DWORD, whichRow);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_ADDR;
  Addr = makeFactory(parent, firstId + OFFSET_ADDR, r, MAX_EDIT_DWORD, whichRow);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_TYPE;
  Type = makeFactoryType(parent, firstId + OFFSET_TYPE, r, MAX_EDIT_DWORD, whichRow);
  Type->setSibling(-DEF_OFFSET_CTRL);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_ACTION;
  Action = makeFactoryAction(parent, firstId + OFFSET_ACTION, r, MAX_EDIT_DWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_MIN;
  Min = makeFactory(parent, firstId + OFFSET_VMIN, r, MAX_EDIT_REAL, whichRow);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_MAX;
  Max = makeFactory(parent, firstId + OFFSET_VMAX, r, MAX_EDIT_REAL, whichRow);
  Max->setSibling(-DEF_OFFSET_CTRL);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_MIN;
  MinP = makeFactory(parent, firstId + OFFSET_VMIN_P, r, MAX_EDIT_REAL, whichRow);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_MAX;
  MaxP = makeFactory(parent, firstId + OFFSET_VMAX_P, r, MAX_EDIT_REAL, whichRow);
  MaxP->setSibling(-DEF_OFFSET_CTRL);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_DESCR;
  Descr = makeFactory(parent, firstId + OFFSET_DESCR, r, MAX_TEXT - 1, whichRow);

  PEdit* setNumb[] = {
    Byte1, Byte2, Byte3, Byte4,
    Port, Db, Type, Action //, Addr
    };
  for(uint i = 0; i < SIZE_A(setNumb); ++i)
    setNumb[i]->Attr.style |= ES_NUMBER;

  PEdit* setReal[] = { Min, Max, MinP, MaxP };
  for(uint i = 0; i < SIZE_A(setReal); ++i)
    setReal[i]->setFilter(new PNumbFilter);
}
//----------------------------------------------------------------------------
bool PRow::hasId(uint id)
{
  PEdit* set[] = { Byte1, Byte2, Byte3, Byte4, Port, Db, Addr, Type, Action,
    Descr, Min, Max, MinP, MaxP
    };
  for(uint i = 0; i < SIZE_A(set); ++i) {
    if(id == set[i]->Attr.id)
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void PRow::invalidate()
{
  PEdit* set[] = { Byte1, Byte2, Byte3, Byte4, Port, Db, Addr, Type, Action,
    Descr, Min, Max, MinP, MaxP
    };
  for(uint i = 0; i < SIZE_A(set); ++i)
    InvalidateRect(*set[i], 0, 1);
}
//----------------------------------------------------------------------------
DWORD PRow::getIpAddr()
{
  TCHAR buff[50];
  GetWindowText(*Byte1, buff, SIZE_A(buff));
  DWORD b1 = _ttol(buff);

  GetWindowText(*Byte2, buff, SIZE_A(buff));
  DWORD b2 = _ttol(buff);

  GetWindowText(*Byte3, buff, SIZE_A(buff));
  DWORD b3 = _ttol(buff);

  GetWindowText(*Byte4, buff, SIZE_A(buff));
  DWORD b4 = _ttol(buff);

  DWORD dw;
  IP_TO_DWORD(b1, b2, b3, b4, dw);
  return dw;
}
//----------------------------------------------------------------------------
void PRow::setIpAddr(DWORD dw)
{
  TCHAR buff[50];
  DWORD b1;
  DWORD b2;
  DWORD b3;
  DWORD b4;
  DWORD_TO_IP(b1, b2, b3, b4, dw);
  wsprintf(buff, _T("%d"), b1);
  SetWindowText(*Byte1, buff);

  wsprintf(buff, _T("%d"), b2);
  SetWindowText(*Byte2, buff);

  wsprintf(buff, _T("%d"), b3);
  SetWindowText(*Byte3, buff);

  wsprintf(buff, _T("%d"), b4);
  SetWindowText(*Byte4, buff);
  invalidateNum(Byte1);
  invalidateNum(Byte2);
  invalidateNum(Byte3);
  invalidateNum(Byte4);
}
//----------------------------------------------------------------------------
#include "adrDlg.h"
//----------------------------------------------------------------------------
bool PRow::isHex()
{
  PD_Adr* pdt = dynamic_cast<PD_Adr*>(Addr->getParent());
  if(pdt)
    return pdt->isHex();
  return false;
}
//----------------------------------------------------------------------------
DWORD PRow::getAddr(DWORD baseAddr)
{
  if(IS_CHANGED(*Addr)) {
    if(isHex())
      return getGenHex(Addr) * baseAddr;
    return getGen(Addr) * baseAddr;
    }
  return realAddr;
}
//----------------------------------------------------------------------------
void PRow::setAddr(DWORD vAddr, DWORD base)
{
  realAddr = vAddr;
  if(isHex())
    setGenHex(Addr, vAddr / base);
  else
    setGen(Addr, vAddr / base);
}
//----------------------------------------------------------------------------
void PRow::setMin(double val)
{
  TCHAR buff[50];
  _stprintf_s(buff, _T("%f"), val);
  zeroTrim(buff);
  SetWindowText(*Min, buff);
  invalidateNum(Min);
}
//----------------------------------------------------------------------------
void PRow::setMax(double val)
{
  TCHAR buff[50];
  _stprintf_s(buff, _T("%f"), val);
  zeroTrim(buff);
  SetWindowText(*Max, buff);
  invalidateNum(Max);
}
//----------------------------------------------------------------------------
void PRow::setMinP(double val)
{
  TCHAR buff[50];
  _stprintf_s(buff, _T("%f"), val);
  zeroTrim(buff);
  SetWindowText(*MinP, buff);
  invalidateNum(MinP);
}
//----------------------------------------------------------------------------
void PRow::setMaxP(double val)
{
  TCHAR buff[50];
  _stprintf_s(buff, _T("%f"), val);
  zeroTrim(buff);
  SetWindowText(*MaxP, buff);
  invalidateNum(MaxP);
}
//----------------------------------------------------------------------------
void PRow::offset(int x, int y, double scaleX, double scaleY)
{
  PEdit* ctrl[] = {
     Byte1, Byte2, Byte3, Byte4, Port, Db, Addr,
     Type, Action, Descr, Min, Max, MinP, MaxP
     };

  x = ROUND_REAL(scaleX * x);
  for(uint i = 0; i < SIZE_A(ctrl); ++i) {
    PRect r;
    GetWindowRect(*ctrl[i], r);
    r.Offset(x, y);
    MapWindowPoints(HWND_DESKTOP, *ctrl[i]->getParent(), (LPPOINT)(LPRECT)r, 2);
    uint dim = ROUND_REAL(r.Width() * scaleX);
    r.right = r.left + dim;
    dim = ROUND_REAL(r.Height() * scaleY);
    r.bottom = r.top + dim;
    SetWindowPos(*ctrl[i], 0, r.left, r.top, r.Width(), r.Height(), /*SWP_NOSIZE |*/ SWP_NOZORDER);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_EDIT_IP_PORT 20
#define MAX_EDIT_DDWORD  20
#define MAX_EDIT_RANGE   20
//#define MAX_EDIT_REAL   20
//----------------------------------------------------------------------------
PRowTest::PRowTest(PWin* parent, uint first_id, POINT pt, int whichRow) :
    firstId(first_id), Value(0), Owner(parent)
{
  HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  PRect r(0, 0, W_IP_PORT, H_EDIT);
  r.MoveTo(pt.x, pt.y);
  PRect r2(r);
  r2.Inflate(0, -1);
  IP_Port = new PStatic(parent, firstId, r2);
  IP_Port->setFont(hf);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_DB_ADDR;
  r2 = r;
  r2.Inflate(0, -1);
  Db_Addr = new PStatic(parent, firstId + OFFSET_DB_ADDR, r2);
  Db_Addr->setFont(hf);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_TYPE_ACTION;
  r2 = r;
  r2.Inflate(0, -1);
  Type_Action = new PStatic(parent, firstId + OFFSET_TYPE_ACTION, r2);
  Type_Action->setFont(hf);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_RANGE;
  r2 = r;
  r2.Inflate(0, -1);
  Range = new PStatic(parent, firstId + OFFSET_VRANGE, r2);
  Range->setFont(hf);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_RANGEP;
  r2 = r;
  r2.Inflate(0, -1);
  RangeP = new PStatic(parent, firstId + OFFSET_VRANGEP, r2);
  RangeP->setFont(hf);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_VALUE;

  if(-1 == whichRow)
    Value = new firstEdit(parent, firstId + OFFSET_VALUE, r, MAX_EDIT_REAL);
  else if(1 == whichRow)
    Value = new lastEdit(parent, firstId + OFFSET_VALUE, r, MAX_EDIT_REAL);
  else
    Value = new PageEdit(parent, firstId + OFFSET_VALUE, r, MAX_EDIT_REAL);
  Value->setSibling(-DEF_OFFSET_CTRL_TEST);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_DESCR_TEST;
  r2 = r;
  r2.Inflate(0, -1);
  Descr = new PStatic(parent, firstId + OFFSET_DESCR_TEST, r2);
  Descr->setFont(hf);
}
//----------------------------------------------------------------------------
bool PRowTest::hasId(uint id)
{
  PWin* set[] = { IP_Port, Value, Db_Addr, Type_Action, Descr, Range, RangeP };
  for(uint i = 0; i < SIZE_A(set); ++i) {
    if(id == set[i]->Attr.id)
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void PRowTest::invalidate()
{
  PWin* set[] = { IP_Port, Value, Db_Addr, Type_Action, Descr, Range, RangeP };
  for(uint i = 0; i < SIZE_A(set); ++i)
    InvalidateRect(*set[i], 0, 1);
}
//----------------------------------------------------------------------------
void PRowTest::offset(int x, int y, double scaleX, double scaleY)
{
  PWin* ctrl[] = {
     IP_Port, Db_Addr, Type_Action, Range, RangeP, Value, Descr
     };

  x = ROUND_REAL(x * scaleX);
  for(uint i = 0; i < SIZE_A(ctrl); ++i) {
    PRect r;
    GetWindowRect(*ctrl[i], r);
    r.Offset(x, y);
    MapWindowPoints(HWND_DESKTOP, *ctrl[i]->getParent(), (LPPOINT)(LPRECT)r, 2);
    uint dim = ROUND_REAL(r.Width() * scaleX);
    r.right = r.left + dim;
    dim = ROUND_REAL(r.Height() * scaleY);
    r.bottom = r.top + dim;
    SetWindowPos(*ctrl[i], 0, r.left, r.top, r.Width(), r.Height(), /*SWP_NOSIZE |*/ SWP_NOZORDER);
    }
}
//----------------------------------------------------------------------------
DWORD PRowTest::getIpAddr_Port(DWORD& port)
{
  TCHAR buff[100];
  GetWindowText(*IP_Port, buff, SIZE_A(buff));
  DWORD b1 = 0;
  DWORD b2 = 0;
  DWORD b3 = 0;
  DWORD b4 = 0;
  port = 0;
  _stscanf(buff, _T("%d.%d.%d.%d:%d"), &b1, &b2, &b3, &b4, &port);

  DWORD dw;
  IP_TO_DWORD(b1, b2, b3, b4, dw);
  return dw;
}
//----------------------------------------------------------------------------
void PRowTest::setIpAddr_Port(DWORD addr, DWORD port)
{
  TCHAR buff[100];
  DWORD b1;
  DWORD b2;
  DWORD b3;
  DWORD b4;
  DWORD_TO_IP(b1, b2, b3, b4, addr);
  wsprintf(buff, _T("%d.%d.%d.%d:%d"), b1, b2, b3, b4, port);
  SetWindowText(*IP_Port, buff);
  invalidateNum(IP_Port);
}
//----------------------------------------------------------------------------
DWORD PRowTest::getDB_Addr(DWORD& addr)
{
  TCHAR buff[100];
  GetWindowText(*Db_Addr, buff, SIZE_A(buff));
  DWORD db = 0;
  addr = 0;
  _stscanf(buff, _T("%d-%d"), &db, &addr);
  return db;
}
//----------------------------------------------------------------------------
void PRowTest::setDB_Addr(DWORD db, DWORD addr)
{
  TCHAR buff[100];
  wsprintf(buff, _T("%d-%d"), db, addr);
  SetWindowText(*Db_Addr, buff);
  invalidateNum(Db_Addr);
}
//----------------------------------------------------------------------------
DWORD PRowTest::getType_Action(DWORD& action)
{
  TCHAR buff[100];
  GetWindowText(*Type_Action, buff, SIZE_A(buff));
  DWORD type = 0;
  action = 0;
  _stscanf(buff, _T("%d-%d"), &type, &action);
  return type;
}
//----------------------------------------------------------------------------
void PRowTest::setType_Action(DWORD type, DWORD action)
{
  TCHAR buff[100];
  wsprintf(buff, _T("%d-%d"), type, action);
  SetWindowText(*Type_Action, buff);
  invalidateNum(Type_Action);
}
//----------------------------------------------------------------------------
static void removeZeroRange(PWin* ctrl, double v1, double v2)
{
  TCHAR t1[40];
  TCHAR t2[40];
  TCHAR buff[100];
  _stprintf_s(t1, _T("%f"), v1);
  _stprintf_s(t2, _T("%f"), v2);
  zeroTrim(t1);
  zeroTrim(t2);
  wsprintf(buff, _T("%s-%s"), t1, t2);
  SetWindowText(*ctrl, buff);
}
//----------------------------------------------------------------------------
rangeLP PRowTest::getRange()
{
  TCHAR buff[100];
  GetWindowText(*Range, buff, SIZE_A(buff));
  rangeLP rlp;
  ZeroMemory(&rlp, sizeof(rlp));

  _stscanf(buff, _T("%lf-%lf"), &rlp.minL, &rlp.maxL);

  GetWindowText(*RangeP, buff, SIZE_A(buff));
  _stscanf(buff, _T("%lf-%lf"), &rlp.minP, &rlp.maxP);
  return rlp;
}
//----------------------------------------------------------------------------
void PRowTest::setRange(const rangeLP& rlp)
{
  removeZeroRange(Range, rlp.minL, rlp.maxL);
  removeZeroRange(RangeP, rlp.minP, rlp.maxP);
  invalidateNum(Range);
  invalidateNum(RangeP);
}
//----------------------------------------------------------------------------
#include "testDlg.h"
//----------------------------------------------------------------------------
bool PRowTest::isHex()
{
  PD_Test* pdt = dynamic_cast<PD_Test*>(Value->getParent());
  if(pdt)
    return pdt->isHex();
  return false;
}
//----------------------------------------------------------------------------
DWORD PRowTest::getValue()
{
  TCHAR buff[100];
  GetWindowText(*Value, buff, SIZE_A(buff));
  DWORD dummy;
  DWORD type = getType_Action(dummy);
  DWORD value = 0;

  if(isHex()) {
    switch(type) {
      case tBitData:
      case tBData:
      case tWData:
      case tDWData:
      case tBsData:
      case tWsData:
      case tDWsData:
      default:
        _stscanf(buff, _T("%X"), &value);
        break;

      case tFRData:
      case tRData:
        _stscanf(buff, _T("%f"), (float*)&value);
        break;
      }

    }
  else {
    switch(type) {
      case tBitData:
        value = binToInt(buff);
        break;
      case tBData:
      case tWData:
      case tDWData:
      case tBsData:
      case tWsData:
      case tDWsData:
      default:
        _stscanf(buff, _T("%d"), &value);
        break;

      case tFRData:
      case tRData:
        _stscanf(buff, _T("%f"), (float*)&value);
        break;
      case tStrData:
        do {
          uint len = min(_tcslen(buff), 4);
          copyStr((LPSTR)&value, buff, len);
          } while(false);
        break;
      }
    }
  return value;
}
//----------------------------------------------------------------------------
uint PRowTest::getLenBin()
{
  PD_Test* pdt = dynamic_cast<PD_Test*>(Value->getParent());
  if(pdt)
    return pdt->getNByte() * 8;
  return MAX_L_BIN;
}
//----------------------------------------------------------------------------
void PRowTest::setValue(DWORD value)
{
  DWORD dummy;
  DWORD type = getType_Action(dummy);

  TCHAR buff[100] = { 0 };
  if(isHex()) {
    switch(type) {
      case tBData:
      case tBsData:
        wsprintf(buff, _T("%02X"), value);
        break;
      case tWData:
      case tWsData:
        wsprintf(buff, _T("%04X"), value);
        break;

      case tBitData:
      case tDWData:
      default:
        wsprintf(buff, _T("%08X"), value);
        break;

      case tFRData:
      case tRData:
        _stprintf_s(buff, _T("%f"), *(float*)&value);
        zeroTrim(buff);

        break;
      }
    }
  else {
    switch(type) {
      case tBitData:
        intToBin(buff, value, getLenBin());
//        intToBin(buff, value, MAX_L_BIN);
        break;
      case tBData:
      case tWData:
      case tDWData:
      default:
        wsprintf(buff, _T("%u"), value);
        break;
      case tBsData:
      case tWsData:
      case tDWsData:
        wsprintf(buff, _T("%d"), (long)value);
        break;

      case tFRData:
      case tRData:
        _stprintf_s(buff, _T("%f"), *(float*)&value);
        zeroTrim(buff);
        break;
      case tStrData:
        do {
          LPCSTR p = (LPCSTR)&value;
          for(uint i = 0; i < 4 && *p; ++i, ++p)
            buff[i] = *p;
          } while(false);
        break;
      }
    }

  SetWindowText(*Value, buff);
  invalidateNum(Value);
}
//----------------------------------------------------------------------------
bool PBinGroupFilter::accept(UINT& key, HWND ctrl)
{
  if(!baseClass::accept(key, ctrl) && _T('.') != key)
    return false;

  int init;
  int end;
  if(!getSel(ctrl, init, end))
    return false;

  int pos = 7;
  for(int i = 0; i < 3; ++i, pos += 8) {
    if(pos == init) {
      if(_T('.') == key)
        return true;
      TCHAR buff[3];
      buff[0] = key;
      buff[1] = _T('.');
      buff[2] = 0;
      addString(ctrl, buff, init, end);
      // torna false perché è già stato caricato sia il carattere
      // che il separatore
      return false;
      }
    }
  // il separatore era stato accettato inizialmente, ma si trova in una
  // posizione sbagliata
  if(_T('.') == key)
    return false;
  return true;
}
//----------------------------------------------------------------------------
int getNumField(LPCTSTR p)
{
  int nf = 0;
  int len = _tcslen(p);
  for(int i = 0; i < len; ++i) {
    int v = (int)(unsigned)p[i];
    if(_T(',') == v)
      ++nf;
    else if(!_istdigit(v))// && _T('-') != p[i])// && _T('.') != p[i])
      break;
    }
  return nf;
}
//---------------------------------------------------------------------------
int getNumField2(LPCTSTR p)
{
  int nf = 0;
  int len = _tcslen(p);
  for(int i = 0; i < len; ++i) {
    int v = (int)(unsigned)p[i];
    if(_T(',') == v)
      ++nf;
    else if(!_istdigit(v) && _T('-') != v && _T('.') != v)
      break;
    }
  return nf;
}
//----------------------------------------------------------------------------
// numero di campi penultima modifica
#define DEF_NUM_FIELD 6
//----------------------------------------------------------------------------
void //PD_Test::
addField(setOfString& Set, uint id, LPCTSTR p, int nField)
{
  TCHAR t[2000];
  LPCTSTR p2 = findNextParam(p, nField);
  int len;
  if(p2)
    len = p2 - p;
  else
    len = _tcslen(p);

  copyStr(t, p, len);
  t[len] = 0;
  for(int i = nField; i < DEF_NUM_FIELD; ++i)
    _tcscat(t, _T("0,"));

  int nf = 0;
  if(p2) {
    nf = getNumField2(p2);
    if(nf) {
      LPCTSTR p3 = findNextParam(p2, 2);
      if(p3) {
        LPTSTR pt = t + _tcslen(t);
        int l = p3 - p2;
        copyStr(pt, p2, l);
        pt[l] = 0;
//        _tcscat(pt, _T("0.0,"));
        double vm = _tstof(p2);
        LPCTSTR pp = findNextParam(p2, 1);
        double vM = _tstof(pp);
        double norm = _tstof(p3);
        if(vM - vm && norm) {
//          norm *= vM - vm;
          vM *= norm;
          vm *= norm;
          TCHAR b[50];
          _stprintf_s(b, _T("%f,"), vm);
          zeroTrim(b);
          _tcscat(pt, b);
          _stprintf_s(b, _T("%f,"), vM);
          zeroTrim(b);
          _tcscat(pt, b);
          p3 = findNextParam(p3, 1);
          _tcscat(pt, p3);
          }
        else {
          _tcscat(pt, _T("0.0,"));
          _tcscat(pt, p3);
          }
        Set.replaceString(id, str_newdup(t), true);
        return;
        }
      }
    }

  _tcscat(t, _T("0.0,0.0,0.0,0.0,"));
  if(p2) {
    p2 = findNextParam(p2, nf);
    _tcscat(t, p2);
    }
  Set.replaceString(id, str_newdup(t), true);
}
//----------------------------------------------------------------------------
bool //PD_Test::
convertToLatest(setOfString& Set, LPTSTR Filename)
{
  if(!Set.setFirst())
    return false;
  setOfString set2(Filename);

  Set.reset();
  set2.setFirst();
  int nField = getNumField(set2.getCurrString());
  if(nField >= DEF_NUM_FIELD)
    nField = DEF_NUM_FIELD - 1;
  do {
    LPCTSTR p = set2.getCurrString();
    addField(Set, set2.getCurrId(), p, nField);
    } while(set2.setNext());

  Set.replaceString(ID_DEF_DATATYPE, str_newdup(_T("4")), true);
  Set.replaceString(ID_DEF_VER, str_newdup(STR_VER), true);
  do {
    P_File pf(Filename);
    pf.appendback();
    } while(false);

  P_File::chgExt(Filename, _T(".ad7"));
  P_File pf(Filename, P_CREAT);
  if(!pf.P_open())
    return false;
  Set.setFirst();
  do {
    Set.writeCurrent(pf);
    } while(Set.setNext());
  return true;
}
//----------------------------------------------------------------------------
