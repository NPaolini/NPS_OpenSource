//-------- exp_BaseContainer.cpp ------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "exp_BaseContainer.h"
#include "P_Express.h"
#include "POwnBtnImageStd.h"
#include <shlwapi.h>
#include <commdlg.h>
#include <limits.h>
#include "p_util.h"
#include "macro_utils.h"
#include  "headerMsg.h"
#include "PButton.h"
//-------------------------------------------------------------------
#define H_TIT 22
#define W_NUMB 48

#define DEF_BACKCOLOR RGB(0xaa, 0xdf, 0xff)
//-------------------------------------------------------------------
class mySPlitContainer : public PSplitWin
{
  private:
    typedef PSplitWin baseClass;
  public:
    mySPlitContainer(PWin* parent, uint id) : baseClass(parent, id, 10, 0, PSplitter::PSW_HORZ) {}

    virtual ~mySPlitContainer() { destroy(); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//-------------------------------------------------------------------
LRESULT mySPlitContainer::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(*this)
    SendMessage(*getParent(), message, wParam, lParam);
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
class myStatic : public PStatic
{
  private:
    typedef PStatic baseClass;
  public:
    myStatic(PWin* parent, PWin* ed, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, r, title, hinst), Ed(ed)
    {
      Attr.style |= SS_NOTIFY;
      Attr.style &= ~WS_TABSTOP;
    }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PWin* Ed;
};
//-------------------------------------------------------------------
LRESULT myStatic::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      SetFocus(*Ed);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
#define BGK_TIT RGB(255, 255, 180)
//-------------------------------------------------------------------
exp_BaseContainer::exp_BaseContainer(PWin* parent, uint id, LPCTSTR tit, infoEdit* iEdit, bool readonly) :
    baseClass(parent, id, 100, 3, PSplitter::PSW_VERT), Dirty(false), currRow(-2),
    Tit(0), Ed(0), lastRow(-1), lastColor(-1), bkgTit(0), onMySetting(false),
    ireo(0), itd(0)

{
  PSplitWin* spw = new mySPlitContainer(this, id + 10);
  spw->Attr.style |= WS_DLGFRAME;
  spw->Attr.style &= ~WS_TABSTOP;
  Ed = new InsideEditFirst(spw, ID_EDIT + id, iEdit, readonly);
//  Ed = new InsideEdit(spw, ID_EDIT + id, iEdit, readonly);
  Tit = new myStatic(this, Ed, ID_STATIC + id, PRect(0, 0, 10, 10), tit);
  NumberRow = new PNumberRow(spw, ID_NUMBER + id, (InsideEdit*)Ed);
  ((InsideEdit*)Ed)->setManRow((PNumberRow*)NumberRow);

  spw->setWChild(NumberRow, Ed);
  PSplitter* split = spw->getSplitter();
  split->setLock(PSplitter::PSL_FIRST);
  split->setDim1(W_NUMB);
  split->setDrawStyle(PSplitter::PSD_NONE);

  setWChild(Tit, spw);
  split = getSplit();
  split->setLock(PSplitter::PSL_FIRST);
  split->setDim1(H_TIT);
  PBitmap bmp(IDB_BAR, getHInstance());
  bkgTit = CreatePatternBrush(bmp);

}
//-------------------------------------------------------------------
exp_BaseContainer::~exp_BaseContainer()
{
  if(itd)
    itd->Release();
  if(ireo)
    ireo->Release();
  destroy();
  DeleteObject(bkgTit);
}
//-------------------------------------------------------------------
const infoEdit& exp_BaseContainer::getInfo() const
{
  return ((InsideEdit*)Ed)->getInfo();
}
//-------------------------------------------------------------------
void exp_BaseContainer::setInfo(const infoEdit& ed) { ((InsideEdit*)Ed)->setInfo(ed); }
//-------------------------------------------------------------------
bool exp_BaseContainer::create()
{
  Attr.style |= WS_CLIPCHILDREN;
  Attr.exStyle |= WS_EX_CONTROLPARENT;
  if(!baseClass::create())
    return false;
  SendMessage(*Ed, EM_SETTEXTMODE, TM_RICHTEXT | TM_MULTILEVELUNDO, 0);
  Tit->setFont(D_FONT(22, 0, 0, _T("Comic Sans MS")));
  SendMessage(*Ed, EM_SETEVENTMASK, 0, (LPARAM)(ENM_CHANGE | ENM_SCROLL | ENM_SCROLLEVENTS | ENM_KEYEVENTS | ENM_MOUSEEVENTS));
  SendMessage(*Ed, EM_SETUNDOLIMIT, 50, 0);
  return true;
}
//-------------------------------------------------------------------
void exp_BaseContainer::setReadOnly(bool set)
{
  ((InsideEdit*)Ed)->setReadOnly(set);
}
//-------------------------------------------------------------------
void exp_BaseContainer::setUndoAction(uint action)
{
  if(!ireo) {
    if(SendMessage(*Ed, EM_GETOLEINTERFACE, 0, (LPARAM)&ireo)) {
      if(!FAILED(ireo->QueryInterface(IID_ITextDocument, (LPVOID*)&itd))) {
        }
      }
    }
  if(itd)
    itd->Undo(action, NULL);
}
//-------------------------------------------------------------------
void exp_BaseContainer::disableEvent()
{
  setUndoAction(tomSuspend);
}
//-------------------------------------------------------------------
void exp_BaseContainer::restoreEvent()
{
  setUndoAction(tomResume);
}
//-------------------------------------------------------------------
LRESULT exp_BaseContainer::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case EN_SETFOCUS:
          ((InsideEdit*)Ed)->setOnFocus(true);
          InvalidateRect(*Tit, 0, 0);
          break;
        case EN_KILLFOCUS:
          ((InsideEdit*)Ed)->setOnFocus(false);
          InvalidateRect(*Tit, 0, 0);
          break;
        case EN_CHANGE:
          if(onMySetting)
            break;
          Dirty = true;
          if(checkRow()) {
            disableEvent();
            colorizeSingle();
            restoreEvent();
            }
          break;
        }
      break;
    case WM_MY_CUSTOM:
      switch(LOWORD(wParam)) {
        case CM_COLORIZE:
          colorizePaste((CHARRANGE*)lParam);
          break;
        case CM_FOCUS:
          InvalidateRect(*Tit, 0, 0);
          break;
        }
      break;
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    case WM_DESTROY:
      if(itd) {
        itd->Release();
        itd = 0;
        }
      if(ireo) {
        ireo->Release();
        ireo = 0;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//http://msdn.microsoft.com/en-us/library/bb787877%28v=vs.85%29.aspx
//-------------------------------------------------------------------
void exp_BaseContainer::parseMsgFilter(MSGFILTER* msg)
{
  if(GetFocus() != *Ed)
    SetFocus(*Ed);
}
//------------------------------------------------------------------------------
HBRUSH exp_BaseContainer::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  if(WM_CTLCOLORSTATIC == ctlType) {
    if(((InsideEdit*)Ed)->hasFocus()) {
      SetBkMode(hdc, TRANSPARENT);
//      SetBkColor(hdc, BGK_TIT);
      return bkgTit;
      }
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    return GetSysColorBrush(COLOR_BTNFACE);
    }
  return 0;
}
//-------------------------------------------------------------------
bool exp_BaseContainer::checkRow()
{
  int curr = SendMessage(*Ed, EM_LINEFROMCHAR, -1, 0);
  if(curr != lastRow) {
    lastRow = curr;
    lastColor = -1;
    }
  return true;
}
//-------------------------------------------------------------------
static int isFunct(LPCTSTR m)
{
  while(*m && (_istalnum (*m) || _T('_') == *m))
    ++m;
  while(*m && _T(' ') >= *m)
    ++m;
  if(*m && _T('(') == *m)
    return eRichGoto;
  if(*m && _T('=') == *m)
    return eRichBase;
  return eRichLabel;
}
//-------------------------------------------------------------------
extern bool isReturn(LPCTSTR p, int& pos);
//-------------------------------------------------------------------
static int checkOther(LPCTSTR m)
{
  int pos;
  if(isReturn(m, pos))
    return eRichGoto;
  switch(m[0]) {
    case _T('{'):
    case _T('}'):
      return eRichGoto;
    case VAR_FUNCT_CHAR:
      if(_istalpha(m[1]))
        return isFunct(m + 2);
    case VAR_CHAR:
      return eRichBase;
    }
  return eRichLabel;
}
//-------------------------------------------------------------------
static bool match(LPCTSTR m1, LPCTSTR m2)
{
  while(*m1) {
    if(*m1 != _totlower(*m2))
      return false;
    ++m1;
    ++m2;
    }
  while(*m2) {
    if(_T(' ') != *m2)
      return _T('=') == *m2;
    ++m2;
    }
  return true;
}
//-------------------------------------------------------------------
static int findMark(LPCTSTR buff, int nchar)
{
// eRichBase, eRichGoto, eRichLabel, eRichComment
  while(*buff && (unsigned)*buff <= _T(' '))
    ++buff;
  switch(*buff) {
    case _T('#'):
      return eRichLabel;
    case _T(';'):
      return eRichComment;
    case VAR_CHAR:
    case VAR_FUNCT_CHAR:
      if(match(_T("_skip"), buff + 1))
        return eRichGoto;
      if(match(_T("_abort"), buff + 1))
        return eRichGoto;
    }
  return checkOther(buff);
}
//-------------------------------------------------------------------
LPTSTR exp_BaseContainer::getSel()
{
  CHARRANGE cr;
  SendMessage(*Ed, EM_EXGETSEL, 0, (LPARAM)&cr);
  int ixLine =  SendMessage(*Ed, EM_LINEFROMCHAR, cr.cpMin, 0);
  TCHAR buff[4096];
  *(LPWORD)buff = SIZE_A(buff);
  int nchar = SendMessage(*Ed, EM_GETLINE, ixLine, (LPARAM)buff);
  uint len = min(nchar, cr.cpMax - cr.cpMin);
  LPTSTR result = new TCHAR[len + 1];
  int ix = SendMessage(*Ed, EM_LINEINDEX, ixLine, 0);
  copyStrZ(result, buff + cr.cpMin - ix, len);
  return result;
}
//-------------------------------------------------------------------
void exp_BaseContainer::colorizePaste(CHARRANGE* cr)
{
  int ixInit =  SendMessage(*Ed, EM_LINEFROMCHAR, cr->cpMin, 0);
  int ixEnd =  SendMessage(*Ed, EM_LINEFROMCHAR, cr->cpMax, 0);
  delete cr;
  disableEvent();
  while(ixInit <= ixEnd) {
    lastColor = -1;
    colorizeSingle(ixInit);
    ++ixInit;
    }
  Dirty = true;
  restoreEvent();
}
//-------------------------------------------------------------------
void exp_BaseContainer::performColorizeLine(int ixLine, int colortype, LPTSTR buff, int nchar)
{
  int ix = SendMessage(*Ed, EM_LINEINDEX, ixLine, 0);
  CHARRANGE cr;
  cr.cpMin = ix;
  cr.cpMax = ix + nchar;
  SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&cr);
  CHARFORMAT2 cf;
  ZeroMemory(&cf, sizeof(cf));
  cf.cbSize = sizeof(cf);
#ifdef ALSO_BACKGROUND
  cf.dwMask = CFM_COLOR | CFM_BACKCOLOR;
  if(ixLine == currRow)
    cf.crBackColor = DEF_BACKCOLOR;
  else
    cf.crBackColor = getInfo().bg;
#else
  cf.dwMask = CFM_COLOR;
#endif
  cf.crTextColor = getInfo().fg[colortype];
  SendMessage(*Ed, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
  if(eRichBase == colortype || eRichGoto == colortype)
    colorizeFunction(cr, buff, ixLine);
}
//-------------------------------------------------------------------
static
LPTSTR checkInitFunct(LPTSTR p)
{
  if(!p || !*p)
    return 0;

  while(*p && (unsigned)*p <= _T(' '))
    ++p;
  switch(*p) {
    case _T('#'):
      return p;

    case _T('$'):
    case _T('@'):
      ++p;
      while(*p && _istalnum(*p) || _T('_') == *p)
        ++p;
      return checkInitFunct(p);

    default:
      if(_istalpha(*p))
        return p;
      while(*p) {
        if(_istalpha(*p))
          return p;
        if(_T('#') == *p)
          return p;

        if(_T('$') == *p || _T('@') == *p)
          return checkInitFunct(p);
        ++p;
        }
      break;
    }
  return 0;
}
//-------------------------------------------------------------------
static
LPTSTR checkEndFunct(LPTSTR p)
{
  if(_T('#') == *p)
    ++p;
  while(*p && (_istalnum(*p) || _T('_') == *p))
    ++p;
  if(*p)
    return p;
  return 0;
}
//-------------------------------------------------------------------
static
bool myCmpText(LPTSTR p, uint len, LPCTSTR match, uint lenMatch)
{
  if(len < lenMatch)
    len = lenMatch;
  for(uint i = 0; i < len; ++i)
    if(_totlower(p[i]) != match[i])
      return false;
  return true;
}
//-------------------------------------------------------------------
static
bool isNullText(LPTSTR p, uint len)
{
  TCHAR nullStr[] = _T("null");
  return myCmpText(p, len, nullStr,  SIZE_A(nullStr) - 1);
}
//-------------------------------------------------------------------
static
LPTSTR isReturnText(LPTSTR p)
{
  TCHAR nullStr[] = _T("return ");
  if(myCmpText(p, SIZE_A(nullStr) - 1, nullStr,  SIZE_A(nullStr) - 1))
    return p + SIZE_A(nullStr) - 1;
  return 0;
}
//-------------------------------------------------------------------
void exp_BaseContainer::colorizeFunction(CHARRANGE& cr, LPTSTR buff, int lineNum)
{
  LPTSTR pb = buff;
  while(*pb && (unsigned)*pb <= _T(' '))
    ++pb;
  LPTSTR p2 = isReturnText(pb);
  if(p2)
    pb = p2;
  else {
    while(*pb && *pb != _T('='))
      ++pb;
    if(*pb)
      ++pb;
    }
  while(pb && *pb) {
    LPTSTR p = checkInitFunct(pb);
    if(!p)
      break;
    LPTSTR p2 = checkEndFunct(p + 1);
    uint len = p2 ? p2 - p : _tcslen(p);
    richEditStyle ix = eRichLabel;
    if(isNullText(p, len))
      ix = eRichComment;
    colorizePart(p - buff, len, cr, ix, lineNum);
    pb = p2;
    }
}
//-------------------------------------------------------------------
void exp_BaseContainer::colorizePart(int offset, int len, const CHARRANGE& cr, richEditStyle ix, int lineNum)
{
  CHARRANGE cr2 = cr;
  cr2.cpMin += offset;
  cr2.cpMax = cr2.cpMin + len;
  SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&cr2);
  CHARFORMAT2 cf;
  ZeroMemory(&cf, sizeof(cf));
  cf.cbSize = sizeof(cf);
#ifdef ALSO_BACKGROUND
  cf.dwMask = CFM_COLOR | CFM_BACKCOLOR;
  if(lineNum == currRow)
    cf.crBackColor = DEF_BACKCOLOR;
  else
    cf.crBackColor = getInfo().bg;
#else
  cf.dwMask = CFM_COLOR;
#endif
  cf.crTextColor = getInfo().fg[ix];
  SendMessage(*Ed, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}
//-------------------------------------------------------------------

//-------------------------------------------------------------------
void exp_BaseContainer::colorizeSingle(int ix_line)
{
  TCHAR buff[4096];
  currRow = SendMessage(*Ed, EM_LINEFROMCHAR, -1, 0);
  int ixLine = ix_line >= 0 ? ix_line : currRow;
  *(LPWORD)buff = SIZE_A(buff);
  int nchar = SendMessage(*Ed, EM_GETLINE, ixLine, (LPARAM)buff);
  buff[nchar] = 0;
  int colortype = findMark(buff, nchar);
  lastColor = colortype;
  CHARRANGE crSave;
  setRedraw::off(*Ed);
  SendMessage(*Ed, EM_EXGETSEL, 0, (LPARAM)&crSave);
  performColorizeLine(ixLine, colortype, buff, nchar);
  SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&crSave);
  setRedraw::on(*Ed);
#if 1
  InvalidateRect(*Ed, 0, 0);
#else
  PRect r = ((PNumberRow*)NumberRow)->getRectLine(ixLine);
  PRect r2;
  GetWindowRect(*Ed, r2);
  r.right = r2.Width();
  InvalidateRect(*Ed, r, 0);
#endif
}
//-------------------------------------------------------------------
void exp_BaseContainer::colorize()
{
  onMySetting = true;
  disableEvent();
  LONG old = GetWindowLong(*Ed, GWL_STYLE);
  SetWindowLong(*Ed, GWL_STYLE, old & ~ES_NOHIDESEL);
  CHARRANGE crSave;
  SendMessage(*Ed, EM_EXGETSEL, 0, (LPARAM)&crSave);
  POINT pt;
  SendMessage(*Ed, EM_GETSCROLLPOS, 0, (LPARAM)&pt);
  int nLine =  SendMessage(*Ed, EM_GETLINECOUNT, 0, 0);
  SetCursor(LoadCursor(0, IDC_WAIT));
  TCHAR buff[4096];
  setRedraw::off(*Ed);
//  LockWindowUpdate(*Ed);
  currRow = SendMessage(*Ed, EM_LINEFROMCHAR, -1, 0);
  for(int i = 0; i < nLine; ++i) {
    *(LPWORD)buff = SIZE_A(buff);
    int nchar = SendMessage(*Ed, EM_GETLINE, i, (LPARAM)buff);
    buff[nchar] = 0;
    int colortype = findMark(buff, nchar);
    performColorizeLine(i, colortype, buff, nchar);
    }

  SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&crSave);
  SendMessage(*Ed, EM_SETSCROLLPOS, 0, (LPARAM)&pt);
  InvalidateRect(*((InsideEdit*)Ed)->getNumbRow(), 0, true);
  setRedraw::on(*Ed);
  InvalidateRect(*Ed, 0, true);
//  LockWindowUpdate(0);
  SetWindowLong(*Ed, GWL_STYLE, old);
  restoreEvent();
  SetCursor(LoadCursor(0, IDC_ARROW));
  onMySetting = false;
}
//-------------------------------------------------------------------
static bool isValidChar(TCHAR c)
{
  if(_istalnum(c) || _T('_') == c)
    return false;
  return true;
}
//-------------------------------------------------------------------
static bool isValidDelimiter(LPCTSTR result, uint len, LPCTSTR buff)
{
  if(result > buff) {
    if(!isValidChar(*(result - 1)))
      return false;
    }
  result += len;
  if(!*result)
    return true;
  return isValidChar(*result);
}
//-------------------------------------------------------------------
//#define REQ(a) a##_DEFAULT
#define REQ(a) a##_USECRLF
//-------------------------------------------------------------------
bool exp_BaseContainer::find(uint offs, LPCTSTR word, DWORD flag, CHARRANGE& cr)
{
  FINDTEXTEXW fte;
  ZeroMemory(&fte, sizeof(fte));
  fte.chrg.cpMin = offs;
  fte.chrg.cpMax = -1;
  fte.lpstrText = word;
  DWORD wParam = FR_DOWN;
  if(flag & Case)
    wParam |= FR_MATCHCASE;
  if(flag & Wholeword)
    wParam |= FR_WHOLEWORD;

  int result = SendMessage(*Ed, EM_FINDTEXTEXW, wParam, (LPARAM)&fte);
  if(result < 0)
    return false;
  cr = fte.chrgText;
  return true;
}
//-------------------------------------------------------------------
#define ED_C  ((InsideEditFirst*)Ed)
//-------------------------------------------------------------------
bool exp_BaseContainer::select(LPCTSTR word, DWORD flag, bool next)
{
//  int ev = disableEvent(*Ed);
  CHARRANGE cr = { 0, 0 };
  if(next)
    SendMessage(*Ed, EM_EXGETSEL, 0, (LPARAM)&cr);
  bool found = find(cr.cpMax, word, flag, cr);
  if(found) {
    SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&cr);
    ED_C->reloadCurr();
    InvalidateRect(*NumberRow, 0, 0);
    }
//  restoreEvent(*Ed, ev);
  return found;
}
//-------------------------------------------------------------------
bool exp_BaseContainer::replace(LPCTSTR newWord, LPCTSTR word, DWORD flag, bool next)
{
//  int ev = disableEvent(*Ed);
  CHARRANGE cr;
  SendMessage(*Ed, EM_EXGETSEL, 0, (LPARAM)&cr);

  int len = _tcslen(newWord);
  bool found = find(cr.cpMin, word, flag, cr);
  if(found) {
    SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&cr);
    SendMessage(*Ed, EM_REPLACESEL, TRUE, (LPARAM)newWord);
    ED_C->reloadCurr();
    InvalidateRect(*Ed, 0, 0);
    Dirty = true;
    cr.cpMax = cr.cpMin + len;
    cr.cpMin = cr.cpMax;
    SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&cr);
    if(next) {
      if(find(cr.cpMax, word, flag, cr))
        SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&cr);
      else {
        cr.cpMin = cr.cpMax;
        SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&cr);
        }
      ED_C->reloadCurr();
      }
    InvalidateRect(*NumberRow, 0, 0);
    }
//  restoreEvent(*Ed, ev);
  return found;
}
//-------------------------------------------------------------------
bool exp_BaseContainer::replaceAll(LPCTSTR newWord, LPCTSTR word, DWORD flag)
{
//  int ev = disableEvent(*Ed);
  CHARRANGE cr = { 0, 0 };
  bool found = false;
  setRedraw::off(*Ed);
  while(find(cr.cpMax, word, flag, cr)) {
    SendMessage(*Ed, EM_EXSETSEL, 0, (LPARAM)&cr);
    SendMessage(*Ed, EM_REPLACESEL, TRUE, (LPARAM)newWord);
    found = true;
    }
  Dirty = found;
  InvalidateRect(*NumberRow, 0, 0);
  setRedraw::on(*Ed);
  InvalidateRect(*Ed, 0, true);
  ED_C->reloadCurr();
//  restoreEvent(*Ed, ev);
  return found;
}
//-------------------------------------------------------------------
uint exp_BaseContainer::getTextLen()
{
  GETTEXTLENGTHEX gtle = { REQ(GTL), 1200 };
  uint dim = SendMessage(*Ed, EM_GETTEXTLENGTHEX, (WPARAM)&gtle, 0);
  return dim;
}
//-------------------------------------------------------------------
LPTSTR exp_BaseContainer::getText(uint& dim)
{
  GETTEXTLENGTHEX gtle = { REQ(GTL), 1200 };
  dim = SendMessage(*Ed, EM_GETTEXTLENGTHEX, (WPARAM)&gtle, 0);
  LPTSTR buff = new TCHAR[dim + 2];
  getText(buff, dim);
  return buff;
}
//-------------------------------------------------------------------
void exp_BaseContainer::resetDirty()
{
  setUndoAction(tomFalse); //Prevents Undo and empties buffer.
  setUndoAction(tomTrue); //Restarts Undo again.
  Dirty = false;
}
//-------------------------------------------------------------------
void exp_BaseContainer::setText(LPCTSTR buff)
{
  onMySetting = true;
#ifdef USE_RICH
  int ev = disableEvent(*Ed);
  SETTEXTEX ste = { ST_DEFAULT, 1200 };
  SendMessage(*Ed, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)buff);
  restoreEvent(*Ed, ev);
#else
  SetWindowText(*Ed, buff);
#endif
  onMySetting = false;
}
//-------------------------------------------------------------------
void exp_BaseContainer::getText(LPTSTR buff, int dim)
{
#ifdef USE_RICH
  GETTEXTEX gte = { dim, REQ(GT), 1200, 0, 0 };
  LRESULT success = SendMessage(*Ed, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)buff);
#else
  GetWindowText(*Ed, buff, dim);
#endif
}
//------------------------------------------------------------------
double dPRECISION = 0.00001;
static int nDec = 5;
//------------------------------------------------------------------
static void calcDecimal()
{
  nDec = 0;
  double prec = dPRECISION;
  while(prec < 1.0) {
    prec *= 10;
    ++nDec;
    }
}
//------------------------------------------------------------------
int getDecimal()
{
  return nDec;
}
//------------------------------------------------------------------
static void setPrecision(double v)
{
  dPRECISION = v;
  calcDecimal();
}
//------------------------------------------------------------------
extern void getKeyPrec(LPTSTR path);
extern void setKeyPrec(LPTSTR path);
//-------------------------------------------------------------------
PDialogBtn::PDialogBtn(PWin* parent, uint id, HINSTANCE hInst) :
  baseClass(parent, id, hInst)
{
  int idBmp[] = { IDB_RUN, IDB_SAVE, IDB_SAVE, IDB_FOLDER, IDB_SETUP, IDB_SEARCH, IDB_DONE };
  int idBtn[] = { IDC_BUTTON_RUN, IDC_BUTTON_SAVE, IDC_BUTTON_SAVEAS, IDC_BUTTON_SEARCH_FILE,
                  IDC_BUTTON_SETUP, IDC_BUTTON_SEARCH, IDC_BUTTON_DONE };

  for(uint i = 0; i < SIZE_A(idBtn); ++i) {
    PBitmap* bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], bmp, POwnBtnImageStd::wLeft, true);
    }
  Attr.exStyle |= WS_EX_CONTROLPARENT;
}
//-------------------------------------------------------------------
bool PDialogBtn::create()
{
  if(!baseClass::create())
    return false;
  TCHAR prec[_MAX_PATH];
  getKeyPrec(prec);
  if(!*prec)
    _tcscpy_s(prec, _T("0.00001"));
  SET_TEXT(IDC_EDIT_PREC, prec);
  setPrecision(_tstof(prec));
  return true;
}
//-------------------------------------------------------------------
bool PDialogBtn::preProcessMsg(MSG& msg)
{
  return baseClass::preProcessMsg(msg);
}
//-------------------------------------------------------------------
LRESULT PDialogBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_RUN:
        case IDC_BUTTON_SAVE:
        case IDC_BUTTON_SAVEAS:
        case IDC_BUTTON_SEARCH_FILE:
        case IDC_BUTTON_SETUP:
        case IDC_BUTTON_SEARCH:
        case IDC_BUTTON_DONE:
        case IDC_CHECK_READ_ONLY:
          PostMessage(*getAppl()->getMainWindow(), WM_COMMAND, MAKEWPARAM(LOWORD(wParam), 0), 0);
          break;
        case IDC_EDIT_PREC:
          switch(HIWORD(wParam)) {
            case EN_KILLFOCUS:
              do {
                TCHAR t[128];
                GET_TEXT(IDC_EDIT_PREC, t);
                setPrecision(_tstof(t));
                } while(false);
              break;
            }
          break;
        }
    case WM_DESTROY:
      do {
        TCHAR prec[64];
        GetWindowText(GetDlgItem(hwnd, IDC_EDIT_PREC), prec, SIZE_A(prec));
        setKeyPrec(prec);
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
bool PDialogBtn::isCheckedReadOnly()
{
  return IS_CHECKED(IDC_CHECK_READ_ONLY);
}
//-------------------------------------------------------------------
void PDialogBtn::enableSave(bool set)
{
  ENABLE(IDC_BUTTON_SAVEAS, set);
  ENABLE(IDC_BUTTON_SAVE, set);
}
//-------------------------------------------------------------------
HWND PDialogBtn::getEdit()
{
  return GetDlgItem(*this, IDC_EDIT_FILENAME);
}
//-------------------------------------------------------------------
void PDialogBtn::setText(LPCTSTR buff)
{
  SetDlgItemText(*this, IDC_EDIT_FILENAME, buff);
}
//-------------------------------------------------------------------
void PDialogBtn::getText(LPTSTR buff, int dim)
{
  GetDlgItemText(*this, IDC_EDIT_FILENAME, buff, dim);
}
//-------------------------------------------------------------------
uint PDialogBtn::getTextLen()
{
  return SendMessage(GetDlgItem(*this, IDC_EDIT_FILENAME), WM_GETTEXTLENGTH, 0, 0);
}
//-------------------------------------------------------------------
LPTSTR PDialogBtn::getText(uint& dim)
{
  dim = getTextLen();
  LPTSTR buff = new TCHAR[dim + 2];
  getText(buff, dim);
  return buff;
}
