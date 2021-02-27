//--------- cust_gen.cpp --------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "cust_gen.h"
#include "infoText.h"
//----------------------------------------------------------------------------
#define PAGE_SCROLL_LEN  MAX_GROUP
#define MAX_V_SCROLL (MAX_ROWS - MAX_GROUP)
//----------------------------------------------------------------------------
#define WM_CUSTOM_VSCROLL (WM_APP + 101)
//----------------------------------------------------------------------------
#define OFFSET_DESCR  (IDC_EDIT_CC_V_1 - IDC_EDIT_CC_C_1)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class cust_FirstRow : public cust_Row
{
  public:

    cust_FirstRow(PWin* parent, uint first_id) : cust_Row(parent, first_id)
    {
      delete ID;
      ID = new firstEdit(parent, firstId);
    }
};
//----------------------------------------------------------------------------
class cust_LastRow : public cust_Row
{
  public:

    cust_LastRow(PWin* parent, uint first_id) : cust_Row(parent, first_id)
    {
      delete Descr;
      Descr = new lastEdit(parent, firstId + OFFSET_DESCR, MAX_TEXT - 1);
    }
};
//----------------------------------------------------------------------------
cust_Row::cust_Row(PWin* parent, uint first_id) :
    firstId(first_id)
{
  ID = new PageEdit(parent, firstId);
  Descr = new PageEdit(parent, firstId + OFFSET_DESCR, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
void cust_Row::getDescr(LPTSTR buff) const
{
  GetWindowText(*Descr, buff, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
inline
void cust_Row::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
}
//----------------------------------------------------------------------------
inline
DWORD cust_Row::getID()
{
  TCHAR buff[50];
  GetWindowText(*ID, buff, SIZE_A(buff));
  return _tstol(buff);
}
//----------------------------------------------------------------------------
inline
void cust_Row::setID(DWORD val)
{
  TCHAR buff[50];
  wsprintf(buff, _T("%d"), val);
  SetWindowText(*ID, buff);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool runCustomGen(PWin* parent)
{
  return IDOK == cust_gen(parent).modal();
}
//----------------------------------------------------------------------------
cust_gen::cust_gen(PWin* parent, uint resId, HINSTANCE hinstance) :
    PModDialog(parent, resId, hinstance),
        Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
        Brush3(CreateSolidBrush(bkgColor3)), currPos(0)
{
  Rows[0] = new cust_FirstRow(this, IDC_EDIT_CC_C_1);

  int i;
  for(i = 1; i < MAX_GROUP - 1; ++i)
    Rows[i] = new cust_Row(this, IDC_EDIT_CC_C_1 + i);

  Rows[i] = new cust_LastRow(this, IDC_EDIT_CC_C_1 + i);
}
//-------------------------------------------------------------------
cust_gen::~cust_gen()
{
  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
  DeleteObject(HGDIOBJ(Brush3));
  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
bool cust_gen::create()
{

  if(!baseClass::create())
    return false;

  setOfString set;
  loadTmpCustom(set);

  if(set.setFirst()) {
    uint ix = 0;
    TCHAR tmp[1000] = _T("\0");
    do {
      Saved[ix].id = set.getCurrId();
      LPCTSTR p = set.getCurrString();
      if(p)
        translateFromCRNL(tmp, p);
      _tcscpy_s(Saved[ix].text, MAX_TEXT, tmp);
      if(++ix >= MAX_ROWS)
        break;

      } while(set.setNext());
    }
  loadData();

  HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_CC);
  if(hsb) {
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nPage = PAGE_SCROLL_LEN;
    si.nMin = 0;
    si.nMax = MAX_ROWS - 1;
    SendMessage(hsb, SBM_SETSCROLLINFO, 1, (LPARAM)&si);
    }

  return true;
}
//----------------------------------------------------------------------------
LRESULT cust_gen::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CUSTOM_VSCROLL:
      lParam = (LPARAM)GetDlgItem(*this, IDC_SCROLLBAR_CC);
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
//------------------------------------------------------------------------------
void cust_gen::evVScrollBar(HWND child, int flags, int pos)
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
void cust_gen::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    Saved[pos].id = Rows[i]->getID();
    Rows[i]->getDescr(Saved[pos].text);
    }
}
//------------------------------------------------------------------------------
void cust_gen::loadData()
{
  int pos = currPos;

  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {

    Rows[i]->setID(Saved[pos].id);
    Rows[i]->setDescr(Saved[pos].text);
    }
}
//----------------------------------------------------------------------------
HBRUSH cust_gen::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    SetBkColor(hdc, bkgColor3);
    return (Brush3);

    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, bkgColor);
    return (Brush);
    }
  return 0;
}
//-------------------------------------------------------------------
void cust_gen::CmOk()
{
  saveCurrData();
  P_File* pf = makeFileTmpCustom(false);
  delete pf;
  pf = makeFileTmpCustom(true);

  if(!pf) {
    delete pf;
    return;
    }
  bool success = true;
  for(uint i = 0; i < SIZE_A(Saved); ++i) {
    if(*Saved[i].text && Saved[i].id >= FIRST_CUSTOM_ID) {
      TCHAR buff[20];
      wsprintf(buff, _T("%d,"), Saved[i].id);
      if(!writeStringChkUnicode(*pf, buff)) {
        success = false;
        break;
        }
      if(!writeStringForceUnicode(*pf, Saved[i].text)) {
        success = false;
        break;
        }
      if(!writeStringForceUnicode(*pf, _T("\r\n"))) {
        success = false;
        break;
        }
      }
    }
  delete pf;
  if(success)
    baseClass::CmOk();
}
//----------------------------------------------------------------------------
