//----------- newNormal.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "newNormal.h"
#include "svDialog.h"
#include "svEdit.h"
#include "PComboBox.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "def_dir.h"
#include "macro_utils.h"
//----------------------------------------------------------------------------
static PVect<infoVarNorm> InfoVarNorm;
//----------------------------------------------------------------------------
const PVect<infoVarNorm>& getInfoVarNorm()
{
  return InfoVarNorm;
}
//----------------------------------------------------------------------------
void addStringToComboBox(HWND hList, LPCTSTR str, uint addSize)
{
  int wid = extent(hList, str);
  int awi = SendMessage(hList, CB_GETDROPPEDWIDTH, 0, 0);

  if(wid + (int)addSize > awi)
    ::SendMessage(hList, CB_SETDROPPEDWIDTH, wid + addSize, 0);
  ::SendMessage(hList, CB_ADDSTRING, 0, LPARAM(str));
}
//-----------------------------------------------------------
void fillCBPerif(HWND hwnd, int select)
{
  LPCTSTR prf[] = {
    _T("00-Prph.None"),
    _T("01-Prph.Mem"),
    _T("02-Prph.PLC"),
    _T("03-Prph.Gen"),
    _T("04-Prph.Gen"),
    _T("05-Prph.Gen"),
    _T("06-Prph.Gen"),
    _T("07-Prph.Gen"),
    _T("08-Prph.Gen"),
    _T("09-Prph.Gen"),

    _T("10-Prph.Mem+"),
    _T("11-Prph.Mem+"),
    _T("12-Prph.Mem+"),
    _T("13-Prph.Mem+"),
    _T("14-Prph.Mem+"),
    _T("15-Prph.Mem+"),
    _T("16-Prph.Mem+"),
    _T("17-Prph.Mem+"),
    _T("18-Prph.Mem+"),
    _T("19-Prph.Mem+"),
    _T("20-Prph.Mem+"),
    };
  int add = GetSystemMetrics(SM_CXVSCROLL);
//  int add = 0;
  for(uint i = 0; i < SIZE_A(prf); ++i)
    addStringToComboBox(hwnd, prf[i], add);
  if(select < 0 || (uint)select >= SIZE_A(prf))
    select = 0;
  SendMessage(hwnd, CB_SETCURSEL, select, 0);
}
//-----------------------------------------------------------
void fillCBType(HWND hwnd, int select)
{
  LPCTSTR type[] = {
    _T("0-Standard"),
    _T("1-By Var"),
    _T("2-By Bit"),
    };
  int add = 0;
  for(uint i = 0; i < SIZE_A(type); ++i)
    addStringToComboBox(hwnd, type[i], add);
  if(select < 0 || (uint)select >= SIZE_A(type))
    select = 0;
  SendMessage(hwnd, CB_SETCURSEL, select, 0);
}
//----------------------------------------------------------------------------
#define MAX_GROUP 16
#define PAGE_SCROLL_LEN  MAX_GROUP
#define MAX_V_SCROLL (MAX_NORMAL - MAX_GROUP)
//#define MAX_NORMAL 256
#define MAX_NORMAL 1024

#define MAX_TEXT 36
//----------------------------------------------------------------------------
#define MAX_EDIT_DOUBLE 12
#define MAX_EDIT_INT    8
//----------------------------------------------------------------------------
#define MUL_DIV 100000
//----------------------------------------------------------------------------
#define WM_CUSTOM_VSCROLL (WM_APP + 101)
//----------------------------------------------------------------------------
#define OFFSET_MIN    60
#define OFFSET_MAX    (OFFSET_MIN + 60)
#define OFFSET_TYPE   (OFFSET_MAX + 60)
#define OFFSET_PRPH   (OFFSET_TYPE + 60)
#define OFFSET_ADDR   (OFFSET_PRPH + 60)
#define OFFSET_TY_LK  (OFFSET_ADDR + 60)
#define OFFSET_BIT    (OFFSET_TY_LK + 60)
#define OFFSET_DESCR  (OFFSET_BIT + 60)
//----------------------------------------------------------------------------
#define FILE_NORMALIZ _T("norm")
//----------------------------------------------------------------------------
enum typeNorm { etnStd, etnVar, etnBit };
//----------------------------------------------------------------------------
struct extBaseRow
{
  uint prph;
  uint addr;
  uint typeOrLink;
  uint bit;

  uint Type; // typeNorm

  extBaseRow() : Type(etnStd), prph(0), addr(0), typeOrLink(0), bit(0) {}
};
//----------------------------------------------------------------------------
struct baseRow
{
  double val;
  double vMin;
  double vMax;
  TCHAR text[MAX_TEXT];
  extBaseRow ext;

};
//----------------------------------------------------------------------------
class PRow
{
  public:

    PRow(PWin* parent, uint first_id);
    virtual ~PRow() {}

    void getDescr(LPTSTR buff) const;
    void setDescr(LPCTSTR buff) const;
    double getVal();
    void setVal(double val);

    double getMin();
    void setMin(double Min);
    double getMax();
    void setMax(double Max);
    void getAll(double& val, double& Min, double& Max);

    void setAll(double val, double Min, double Max);

    void getAll(baseRow& val);
    void setAll(const baseRow& val);
    void fillCBPerif(const baseRow& val);
    void fillCBType(const baseRow& val);

    void enableRow(bool enable);
    void rePos(int left, int y, int h, double scaleX);
    void setNDec(int ndec) { nDec = ndec; }
    int getNDec() const { return nDec; }
  protected:
    svEdit* Val;
    svEdit* Min;
    svEdit* Max;
    svEdit* Descr;

    PComboBox* Type;
    PComboBox* Prph;
    svEdit* Addr;
    svEdit* typeOrLink;
    svEdit* Bit;
    uint firstId;
    int nDec;
    double getGen(PEdit* ctrl);
    void setGen(PEdit* ctrl, double val);
};
//----------------------------------------------------------------------------
class PageEdit : public svEdit
{
  public:
    PageEdit(PWin * parent, uint resid, const PRect& r, int textlen = 255, HINSTANCE hinst = 0) :
          svEdit(parent, resid, r, 0, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
bool PageEdit::evKeyDown(WPARAM& key)
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
class firstEdit : public PageEdit
{
  public:
    firstEdit(PWin * parent, uint resid, const PRect& r, int textlen = 255, HINSTANCE hinst = 0) :
          PageEdit(parent, resid, r, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
bool firstEdit::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return PageEdit::evKeyDown(key);
}
//----------------------------------------------------------------------------
class lastEdit : public PageEdit
{
  public:
    lastEdit(PWin * parent, uint resid, const PRect& r, int textlen = 255, HINSTANCE hinst = 0) :
          PageEdit(parent, resid, r, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
bool lastEdit::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
    PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
    return true;
    }
  return PageEdit::evKeyDown(key);
}
//----------------------------------------------------------------------------
class PFirstRow : public PRow
{
  public:

    PFirstRow(PWin* parent, uint first_id) : PRow(parent, first_id)
    {
      PRect r = Val->getRect();
      delete Val;
      Val = new firstEdit(parent, firstId, r, MAX_EDIT_DOUBLE);
      HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
      Val->setFont(hf, false);
    }
};
//----------------------------------------------------------------------------
class PLastRow : public PRow
{
  public:

    PLastRow(PWin* parent, uint first_id) : PRow(parent, first_id)
    {
      PRect r = Descr->getRect();
      delete Descr;
      Descr = new lastEdit(parent, firstId + OFFSET_DESCR, r, MAX_TEXT);
      HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
      Descr->setFont(hf, false);
    }
};
//----------------------------------------------------------------------------
class PD_Normalz : public PModDialog
{
  public:
    PD_Normalz(PWin* parent, LPCTSTR filename, uint resId = IDD_NORMALZ, HINSTANCE hinstance = 0);
    virtual ~PD_Normalz();

    virtual bool create();

  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
//    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    LPTSTR Filename;
    baseRow Cfg[MAX_NORMAL];
    PRow* Rows[MAX_GROUP];
    int currPos;
    int nDec;

    void saveCurrData();
    void saveData();
    void loadData();
    void reload();
    void evVScrollBar(HWND child, int flags, int pos);
};
//----------------------------------------------------------------------------
#define H_FIELD 20
//----------------------------------------------------------------------------
#define OFFS_H 14
//----------------------------------------------------------------------------
#define D_VAL   60
#define D_MIN   D_VAL
#define D_MAX   D_VAL

#define D_TYPE  60
#define D_PRPH  60
#define D_ADDR  D_VAL
#define D_TY_LK 50
#define D_BIT   40

#define D_DESCR 200
//----------------------------------------------------------------------------
#define O_VAL   58
#define O_MIN   (O_VAL + D_VAL + 1)
#define O_MAX   (O_MIN + D_MIN + 1)

#define O_TYPE  (O_MAX + D_MAX + 1)
#define O_PRPH  (O_TYPE + D_TYPE  +1)
#define O_ADDR  (O_PRPH + D_PRPH + 1)
#define O_TY_LK (O_ADDR + D_ADDR + 1)
#define O_BIT   (O_TY_LK + D_TY_LK + 1)

#define O_DESCR (O_BIT + D_BIT + 1)

#define MAX_DIM (O_DESCR + D_DESCR + 1)
//----------------------------------------------------------------------------
PRow::PRow(PWin* parent, uint first_id) :
    firstId(first_id)
{
  HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

  PRect r(0, 0, D_VAL, H_FIELD);
  r.MoveTo(O_VAL, OFFS_H + (first_id - IDC_EDIT_VAL_1 + 1) * H_FIELD);

  Val = new PageEdit(parent, firstId, r, MAX_EDIT_DOUBLE);
  Val->setFont(hf, false);

  r.Offset(O_MIN - O_VAL, 0);
  Min = new PageEdit(parent, firstId + OFFSET_MIN, r, MAX_EDIT_DOUBLE);
  Min->setFont(hf, false);
  r.Offset(O_MAX - O_MIN, 0);
  Max = new PageEdit(parent, firstId + OFFSET_MAX, r, MAX_EDIT_DOUBLE);
  Max->setFont(hf, false);

  r.Offset(O_TYPE - O_MAX, 0);
  r.right = r.left + D_TYPE;
  PRect r2(r);
  r2.bottom += 300;
  Type = new PComboBox(parent, firstId + OFFSET_TYPE, r2);
  Type->setFont(hf, false);

  r.Offset(O_PRPH - O_TYPE, 0);
  r.right = r.left + D_PRPH;
  r2 = r;
  r2.bottom += 1000;
  Prph = new PComboBox(parent, firstId + OFFSET_PRPH, r2);
  Prph->setFont(hf, false);

  r.Offset(O_ADDR - O_PRPH, 0);
  r.right = r.left + D_ADDR;
  Addr = new PageEdit(parent, firstId + OFFSET_ADDR, r, MAX_EDIT_INT);
  Addr->setFont(hf, false);

  r.Offset(O_TY_LK - O_ADDR, 0);
  r.right = r.left + D_TY_LK;
  typeOrLink = new PageEdit(parent, firstId + OFFSET_TY_LK, r, MAX_EDIT_INT);
  typeOrLink->setFont(hf, false);

  r.Offset(O_BIT - O_TY_LK, 0);
  r.right = r.left + D_BIT;
  Bit = new PageEdit(parent, firstId + OFFSET_BIT, r, MAX_EDIT_INT);
  Bit->setFont(hf, false);

  r.Offset(O_DESCR - O_BIT, 0);
  r.right = r.left + D_DESCR;
  Descr = new PageEdit(parent, firstId + OFFSET_DESCR, r, MAX_TEXT);
  Descr->setFont(hf, false);
}
//----------------------------------------------------------------------------
void PRow::rePos(int x, int y, int h, double scaleX)
{
  PWin* ctrl[] = { Val, Min, Max, Type, Prph, Addr, typeOrLink, Bit, Descr };
  for(uint i = 0; i < SIZE_A(ctrl); ++i) {
    PRect r;
    GetWindowRect(*ctrl[i], r);
    r.MoveTo(x, y);
    uint dim = ROUND_REAL(r.Width() * scaleX);
    r.right = r.left + dim;
#if 1
    r.bottom = r.top + h;
#else
    switch(i) {
      case 3:
      case 4:
        r.bottom = r.top + h - 2;
        break;
      default:
        r.bottom = r.top + h;
        break;
      }
#endif
    SetWindowPos(*ctrl[i], 0, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);
    x += r.Width() + 1;
    }
}
//----------------------------------------------------------------------------
void PRow::enableRow(bool enable)
{
  EnableWindow(*Val, enable);
  EnableWindow(*Min, enable);
  EnableWindow(*Max, enable);
  EnableWindow(*Type, enable);
  EnableWindow(*Prph, enable);
  EnableWindow(*Addr, enable);
  EnableWindow(*typeOrLink, enable);
  EnableWindow(*Bit, enable);
  EnableWindow(*Descr, enable);
}
//----------------------------------------------------------------------------
void PRow::fillCBPerif(const baseRow& val)
{
  ::fillCBPerif(*Prph, val.ext.prph);
}
//----------------------------------------------------------------------------
void PRow::fillCBType(const baseRow& val)
{
  ::fillCBType(*Type, val.ext.Type);
}
//----------------------------------------------------------------------------
inline
void PRow::getAll(baseRow& val)
{
  val.val = getGen(Val);
  val.vMin = getGen(Min);
  val.vMax = getGen(Max);
  val.ext.Type = SendMessage(*Type, CB_GETCURSEL, 0, 0);
  val.ext.prph = SendMessage(*Prph, CB_GETCURSEL, 0, 0);

  if(val.ext.Type < 0)
    val.ext.Type = 0;
  if(val.ext.prph < 0)
    val.ext.prph = 0;

  GetWindowText(*Addr, val.text, MAX_TEXT);
  val.ext.addr = _ttoi(val.text);

  GetWindowText(*typeOrLink, val.text, MAX_TEXT);
  val.ext.typeOrLink = _ttoi(val.text);

  GetWindowText(*Bit, val.text, MAX_TEXT);
  val.ext.bit = _ttoi(val.text);

  GetWindowText(*Descr, val.text, MAX_TEXT);
}
//----------------------------------------------------------------------------
inline
void PRow::setAll(const baseRow& val)
{
  setGen(Val, val.val);
  setGen(Min, val.vMin);
  setGen(Max, val.vMax);
  SendMessage(*Type, CB_SETCURSEL, val.ext.Type, 0);
  SendMessage(*Prph, CB_SETCURSEL, val.ext.prph, 0);

  TCHAR buff[50];
  wsprintf(buff, _T("%d"), val.ext.addr);
  SetWindowText(*Addr, buff);

  wsprintf(buff, _T("%d"), val.ext.typeOrLink);
  SetWindowText(*typeOrLink, buff);

  wsprintf(buff, _T("%d"), val.ext.bit);
  SetWindowText(*Bit, buff);

  SetWindowText(*Descr, val.text);
}
//----------------------------------------------------------------------------
inline
void PRow::getDescr(LPTSTR buff) const
{
  GetWindowText(*Descr, buff, MAX_TEXT);
}
//----------------------------------------------------------------------------
inline
void PRow::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
}
//----------------------------------------------------------------------------
inline
double PRow::getGen(PEdit* ctrl)
{
  TCHAR buff[50];
  GetWindowText(*ctrl, buff, SIZE_A(buff));
  return _tstof(buff);
}
//----------------------------------------------------------------------------
inline
void PRow::setGen(PEdit* ctrl, double val)
{
  TCHAR buff[50];
  if(nDec)
    _stprintf_s(buff, SIZE_A(buff), _T("%.*f"), nDec, val);
  else
    _stprintf_s(buff, SIZE_A(buff), _T("%.5f"), val);
  zeroTrim(buff);
  SetWindowText(*ctrl, buff);
}
//----------------------------------------------------------------------------
inline
double PRow::getVal()
{
  return getGen(Val);
}
//----------------------------------------------------------------------------
inline
void PRow::setVal(double val)
{
  setGen(Val, val);
}
//----------------------------------------------------------------------------
inline
double PRow::getMin()
{
  return getGen(Min);
}
//----------------------------------------------------------------------------
inline
void PRow::setMin(double vMin)
{
  setGen(Min, vMin);
}
//----------------------------------------------------------------------------
inline
double PRow::getMax()
{
  return getGen(Max);
}
//----------------------------------------------------------------------------
inline
void PRow::setMax(double vMax)
{
  setGen(Max, vMax);
}
//----------------------------------------------------------------------------
inline
void PRow::getAll(double& val, double& vMin, double& vMax)
{
  TCHAR buff[50];
  GetWindowText(*Val, buff, SIZE_A(buff));
  val = _tstof(buff);
  GetWindowText(*Min, buff, SIZE_A(buff));
  vMin = _tstof(buff);
  GetWindowText(*Max, buff, SIZE_A(buff));
  vMax = _tstof(buff);
}
//----------------------------------------------------------------------------
inline
void PRow::setAll(double val, double vMin, double vMax)
{
  setVal(val);
  setMin(vMin);
  setMax(vMax);
}
//----------------------------------------------------------------------------
LPCTSTR checkNewVersion(extBaseRow& ext, LPCTSTR p)
{
#if 0
  return p;
#else
  LPCTSTR p2 = findNextParam(p, 1);
  if(!p2)
    return p;

  ext.Type = _ttoi(p);
  p =  findNextParam(p2, 1);
  if(!p)
    return p2;
  ext.prph = _ttoi(p2);
  p2 =  findNextParam(p, 1);
  if(!p2)
    return p;
  ext.addr = _ttoi(p);
  p =  findNextParam(p2, 1);
  if(!p)
    return p2;
  ext.typeOrLink = _ttoi(p2);
  p2 =  findNextParam(p, 1);
  if(!p2)
    return p;
  ext.bit = _ttoi(p);
  return p2;
#endif
}
//----------------------------------------------------------------------------
static int calcNDec(LPCTSTR p, int ndec)
{
  while(*p && (unsigned)*p <= _T(' '))
    ++p;
  while(*p && *p != _T('.')) {
    if(*p == _T(','))
      return ndec;
    ++p;
    }
  ++p;
  int i = 0;
  while(*p && *p != _T(',')) {
    ++i;
    ++p;
    }
  return max(i, ndec);
}
//----------------------------------------------------------------------------
PD_Normalz::PD_Normalz(PWin* parent, LPCTSTR filename, uint resId, HINSTANCE hinstance) :
    PModDialog(parent, resId, hinstance), Filename(str_newdup(filename)), currPos(0), nDec(0)
{

  for(uint i = 0; i < SIZE_A(Cfg); ++i) {
    Cfg[i].val = 1.0;
    Cfg[i].vMin = 0.0;
    Cfg[i].vMax = 0.0;
    Cfg[i].text[0] = 0;
    }

  setOfString Set(Filename);
  if(Set.setFirst())
    do {
      long id = Set.getCurrId();
      if(id > 0 && id <= MAX_NORMAL) {
        LPCTSTR p = Set.getCurrString();
        --id;
        Cfg[id].val = _tstof(p);
        nDec = calcNDec(p, nDec);
        p = findNextParam(p, 1);
        if(p && (_T('-') == *p || _istdigit((unsigned)*p))) {
          Cfg[id].vMin = _tstof(p);

          p = findNextParam(p, 1);
          if(p)
            Cfg[id].vMax = _tstof(p);

          p = findNextParam(p, 1);
          }
        if(p) {

          p = checkNewVersion(Cfg[id].ext, p);
          if(p) {
            int len = _tcslen(p);
            if(len >= MAX_TEXT)
              len = MAX_TEXT - 1;
            copyStr(Cfg[id].text, p, len);
            Cfg[id].text[len] = 0;
            }
          }
        }
      } while(Set.setNext());
  if(nDec < 5)
    nDec = 5;
}
//----------------------------------------------------------------------------
PD_Normalz::~PD_Normalz()
{
  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
  delete []Filename;
}
//----------------------------------------------------------------------------
bool PD_Normalz::create()
{
  Rows[0] = new PFirstRow(this, IDC_EDIT_VAL_1);

  int i;
  for(i = 1; i < MAX_GROUP - 1; ++i)
    Rows[i] = new PRow(this, IDC_EDIT_VAL_1 + i);

  Rows[i] = new PLastRow(this, IDC_EDIT_VAL_1 + i);

  if(!PModDialog::create())
    return false;
#if 1
  PRect r1;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1), r1);
  PRect r2;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_16), r2);

  double h = r2.top - r1.top;
  h /= 15;
  MapWindowPoints(0, *this, (LPPOINT)(LPRECT)r1, 2);
  double y = r1.top;

  GetWindowRect(GetDlgItem(*this, IDC_GROUPBOX_1), r1);
  MapWindowPoints(0, *this, (LPPOINT)(LPRECT)r1, 2);
  GetWindowRect(GetDlgItem(*this, IDC_EDIT_VAL_1), r2);

  int left = r1.left + 4;
  int right = r1.right - 4;

  GetWindowRect(GetDlgItem(*this, IDC_EDIT_VAL_1 + OFFSET_DESCR), r1);

  double scaleX = right - left;
  scaleX /= r1.right - r2.left;

  for(int i = 0; i < MAX_GROUP; ++i, y += h) {
    Rows[i]->rePos(left, (int)y, (int)h - 1, scaleX);
    Rows[i]->setNDec(nDec);
    }
#endif
  SET_INT(IDC_EDIT_NUM_DEC, nDec);
  loadData();
  for(i = 0; i < MAX_GROUP; ++i) {
    Rows[i]->fillCBPerif(Cfg[i]);
    Rows[i]->fillCBType(Cfg[i]);
    }

  HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
  if(hsb)
    SendMessage(hsb, SBM_SETRANGE , 0, MAX_V_SCROLL);

  HWND firstEdit = GetDlgItem(*this, IDC_EDIT_VAL_1);
  SetFocus(firstEdit);

//  Rows[0]->setFocus(IDC_SCROLLBAR_RED);

  return true;
}
//----------------------------------------------------------------------------
bool PD_Normalz::preProcessMsg(MSG& msg)
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
    return PModDialog::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
LRESULT PD_Normalz::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          saveData();
          break;
        case IDC_BUTTON_RELOAD_DEC:
          reload();
          break;
        }
      break;

    case WM_CUSTOM_VSCROLL:
      lParam = (LPARAM)GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
      // passa sotto
    case WM_VSCROLL:
      evVScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
      break;
/*
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
*/
    }
  return PModDialog::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PD_Normalz::evVScrollBar(HWND child, int flags, int pos)
{
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
  if(pos < 0)
    pos = 0;
  else if(pos > MAX_V_SCROLL)
    pos = MAX_V_SCROLL;
  SendMessage(child, SBM_SETPOS, pos, true);

  saveCurrData();
  currPos = pos;
  loadData();
}
//------------------------------------------------------------------------------
void PD_Normalz::reload()
{
  GET_INT(IDC_EDIT_NUM_DEC, nDec);
  if(nDec == Rows[0]->getNDec())
    return;
//  saveCurrData();
  for(uint i = 0; i < MAX_GROUP; ++i)
    Rows[i]->setNDec(nDec);
  loadData();
}
//------------------------------------------------------------------------------
void PD_Normalz::saveData()
{
  saveCurrData();
  P_File file(Filename, P_CREAT);
  if(file.P_open()) {
    for(int i = 0; i < MAX_NORMAL; ++i) {
      if(Cfg[i].val != 1.0 || Cfg[i].text[0] || Cfg[i].vMin != 0.0 || Cfg[i].vMax != 0.0) {
        TCHAR buff[500];
        if(Cfg[i].val == 0.0)
          Cfg[i].val = 1.0;
        if(!nDec)
          nDec = 5;
        TCHAR v[3][64];
        _stprintf_s(v[0], SIZE_A(v[0]), _T("%.*f"), nDec, Cfg[i].val);
        _stprintf_s(v[1], SIZE_A(v[1]), _T("%.*f"), nDec, Cfg[i].vMin);
        _stprintf_s(v[2], SIZE_A(v[2]), _T("%.*f"), nDec, Cfg[i].vMax);
        for(uint j = 0; j < 3; ++j)
          zeroTrim(v[j]);

        _stprintf_s(buff, SIZE_A(buff), _T("%d,%s,%s,%s,%d,%d,%d,%d,%d,%s\r\n"),
              i + 1, v[0], v[1],v[2],
              Cfg[i].ext.Type, Cfg[i].ext.prph, Cfg[i].ext.addr, Cfg[i].ext.typeOrLink,
              Cfg[i].ext.bit, Cfg[i].text);
        file.P_writeString(buff);
        }
      }
    }
}
//------------------------------------------------------------------------------
void PD_Normalz::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
#if 1
    Rows[i]->getAll(Cfg[pos]);
#else
    Cfg[pos].val = Rows[i]->getVal();
    Cfg[pos].vMin = Rows[i]->getMin();
    Cfg[pos].vMax = Rows[i]->getMax();
    Rows[i]->getDescr(Cfg[pos].text);
#endif
    }
}
//------------------------------------------------------------------------------
void PD_Normalz::loadData()
{
  int pos = currPos;
//  int focus = 0;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos + 1);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);
#if 1
    Rows[i]->setAll(Cfg[pos]);
    Rows[i]->enableRow(pos > 1);
#else
    Rows[i]->setAll(Cfg[pos].val, Cfg[pos].vMin, Cfg[pos].vMax);
    Rows[i]->setDescr(Cfg[pos].text);
#endif
    }
}
//----------------------------------------------------------------------------
/*
HBRUSH PD_Normalz::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(
      WM_CTLCOLORSTATIC == ctlType ||
//      WM_CTLCOLORBTN == ctlType ||
//      WM_CTLCOLORDLG == ctlType ||
//      WM_CTLCOLORLISTBOX == ctlType ||
//      WM_CTLCOLORMSGBOX == ctlType ||
//      WM_CTLCOLORSCROLLBAR == ctlType ||
      WM_CTLCOLOREDIT == ctlType
      ) {
    bool isThis = false;
    for(int i = 0; i < MAX_GROUP; ++i) {
      PColorBox* box = Rows[i]->getBox();
      if(*box == hWndChild) {
        SetBkColor(hdc, box->getColor());
        return box->getBrush();
        }
      }
    if(isThis) {
      SetTextColor(hdc, RGB(0, 0, 0));
      SetBkColor(hdc, RGB(0xff, 0xff, 0xff));
      return (HBRUSH)GetStockObject(WHITE_BRUSH);
      }
    for(int i = 0; i < SIZE_A(BaseColor); ++i)
      if(*BaseColor[i] == hWndChild) {
        SetBkColor(hdc, BaseColor[i]->getColor());
        return BaseColor[i]->getBrush();
        }
    }
  return 0;
}

*/
//----------------------------------------------------------------------------
class newNormal
{
  public:
    struct nrmData {
      fREALDATA Val;
      fREALDATA Min;
      fREALDATA Max;
      extBaseRow ext;
      nrmData() : Val(1.0), Min(0.0), Max(0.0) {}
      };
    static const nrmData& get(uint ix);

    static void load();
  private:
    // per evitare di creare un oggetto ad ogni invocazione
    // si rende tutto statico. Però all'inizializzazione le
    // directory non sono ancora settate.
    // Occorre quindi richiamare la load() dopo aver settato le dir
    newNormal();
    static nrmData Buff[MAX_NORMAL];
    static newNormal initializer;
};
//----------------------------------------------------------------------------
newNormal::nrmData newNormal::Buff[MAX_NORMAL];
newNormal newNormal::initializer;
//----------------------------------------------------------------------------
#define _SET_1(a) { Buff[a].Val = 1.0; Buff[a].Min = 0.0; Buff[a].Max = 0.0; extBaseRow ebr; Buff[a].ext = ebr; }
//----------------------------------------------------------------------------
newNormal::newNormal()
{
  load();
}
//----------------------------------------------------------------------------
const newNormal::nrmData& newNormal::get(uint ix)
{
  if(ix && ix <= SIZE_A(Buff))
    return Buff[ix - 1];
  static newNormal::nrmData d;
  return d;
}
//----------------------------------------------------------------------------
void newNormal::load()
{
  InfoVarNorm.reset();

  for(int i = 0; i < MAX_NORMAL; ++i)
    _SET_1(i);

  TCHAR file[_MAX_PATH];
  makePath(file, FILE_NORMALIZ, dSystem, false);

  setOfString Set(file);
  if(Set.setFirst())
    do {
      long id = Set.getCurrId();
      if(id > 0 && id <= MAX_NORMAL) {
        LPCTSTR p = Set.getCurrString();

        do {
          --id;
          Buff[id].Val = (fREALDATA)_tstof(p);
          if(Buff[id].Val == 0.0)
            Buff[id].Val = 1.0;
          p = findNextParam(p, 1);
          if(!p)
            break;
          Buff[id].Min = (fREALDATA)_tstof(p);
          p = findNextParam(p, 1);
          if(!p)
            break;
          Buff[id].Max = (fREALDATA)_tstof(p);
          p = findNextParam(p, 1);
          if(!p)
            break;
          Buff[id].ext.Type = (typeNorm)_ttoi(p);
          p = findNextParam(p, 1);
          if(!p)
            break;
          Buff[id].ext.prph = _ttoi(p);
          p = findNextParam(p, 1);
          if(!p)
            break;
          Buff[id].ext.addr = _ttoi(p);
          p = findNextParam(p, 1);
          if(!p)
            break;
          Buff[id].ext.typeOrLink = _ttoi(p);
          p = findNextParam(p, 1);
          if(!p)
            break;
          Buff[id].ext.bit = _ttoi(p);
          } while(false);
        if(Buff[id].ext.Type) {
          infoVarNorm ivn = { Buff[id].ext.prph, Buff[id].ext.addr };
          uint nElem = InfoVarNorm.getElem();
          InfoVarNorm[nElem] = ivn;
          }

        }
      } while(Set.setNext());
}
//----------------------------------------------------------------------------
extern
fREALDATA getNormByVar(uint prph, uint addr, uint typeVar);
//----------------------------------------------------------------------------
extern
bool isSetBitNorm(uint prph, uint addr, uint bit);
//----------------------------------------------------------------------------
fREALDATA getNorm(uint ix)
{
  const newNormal::nrmData& data = newNormal::get(ix);
  switch(data.ext.Type) {
    case etnStd:
      return data.Val;

    case etnVar:
      do {
        fREALDATA result = getNormByVar(data.ext.prph, data.ext.addr, data.ext.typeOrLink);
        if(!result)
          return data.Val;
        return result;
        } while(false);
      break;
    case etnBit:
      if(isSetBitNorm(data.ext.prph, data.ext.addr, data.ext.bit))
        return getNorm(data.ext.typeOrLink);
      return data.Val;
    }
  return 1.0f;
}
//----------------------------------------------------------------------------
bool isOnRange(fREALDATA val, uint ix)
{
  const newNormal::nrmData& nd = newNormal::get(ix);
  if(etnBit == nd.ext.Type) {
    if(isSetBitNorm(nd.ext.prph, nd.ext.addr, nd.ext.bit))
      return isOnRange(val, nd.ext.typeOrLink);
    }

  if(0.0 == nd.Min && 0.0 == nd.Max)
    return true;
  return nd.Min <= val && val <= nd.Max;
}
//----------------------------------------------------------------------------
bool isOnRange(DWDATA val, uint ix)
{
  const newNormal::nrmData& nd = newNormal::get(ix);

  if(etnBit == nd.ext.Type) {
    if(isSetBitNorm(nd.ext.prph, nd.ext.addr, nd.ext.bit))
      return isOnRange(val, nd.ext.typeOrLink);
    }

  if(0.0 == nd.Min && 0.0 == nd.Max)
    return true;

  fREALDATA rVal = nd.Val;
  if(etnVar == nd.ext.Type)
    rVal = getNormByVar(nd.ext.prph, nd.ext.addr, nd.ext.typeOrLink);

  rVal *= val;
  return nd.Min <= rVal && rVal <= nd.Max;
}
//----------------------------------------------------------------------------
void getMinMax(fREALDATA& vMin, fREALDATA& vMax, uint ix)
{
  const newNormal::nrmData& nd = newNormal::get(ix);
  vMin = nd.Min;
  vMax = nd.Max;
}
//----------------------------------------------------------------------------
bool setupNormal(PWin* parent)
{
  TCHAR file[_MAX_PATH];
  makePath(file, FILE_NORMALIZ, dSystem, false);

  if(IDOK == PD_Normalz(parent, file).modal()) {
    newNormal::load();
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void initNormal()
{
  newNormal::load();
}
