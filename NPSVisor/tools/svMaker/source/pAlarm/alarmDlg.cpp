//------------ alarmDlg.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "alarmDlg.h"
#include "alarmEditorDlg.h"
#include "common.h"
#include "fillAlarmAssoc.h"
//----------------------------------------------------------------------------
PD_Alarm::PD_Alarm(PWin* parent, uint idPrph, uint resId, HINSTANCE hinstance) :
    baseClass(parent, idPrph, resId, hinstance)
{
  for(int i = 0; i < MAX_GROUP; ++i)
    Rows[i] = 0;
  ClearAll();
  setFileName();
}
//----------------------------------------------------------------------------
PD_Alarm::~PD_Alarm()
{
  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
}
//----------------------------------------------------------------------------
void PD_Alarm::setFileName()
{
  wsprintf(Filename, _T("#alarm_%d")PAGE_EXT, idPrph);
}
//----------------------------------------------------------------------------
void PD_Alarm::Clear(int from, int to)
{
  dirty = true;
  from = min((uint)from, SIZE_A(Cfg) - 1);
  to = min((uint)to, SIZE_A(Cfg) - 1);
  for(int i = from; i <= to; ++i)
    Cfg[i].clear();
}
//----------------------------------------------------------------------------
bool PD_Alarm::create()
{
  POINT pt = { X_INIT, Y_INIT };
  Rows[0] = new PRow_Alarm(this, IDC_EDIT_FIRST_FIELD, pt, -1);

  pt.y += H_EDIT - 1;
  for(int i = 1; i < MAX_GROUP - 1; ++i) {
    Rows[i] = new PRow_Alarm(this, IDC_EDIT_FIRST_FIELD + i, pt, 0);
    pt.y += H_EDIT - 1;
    }

  Rows[MAX_GROUP - 1] = new PRow_Alarm(this, IDC_EDIT_FIRST_FIELD + MAX_GROUP - 1, pt, 1);

  if(!baseClass::create())
    return false;
  PRect r;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1), r);
  int y = r.top;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_2), r);
  int h = r.top - y;

  PVect<infoPosCtrl> ipc;

  uint idc[] = { IDC_STATIC_GROUP, IDC_STATIC_FILTER, IDC_STATIC_ASSOC, IDC_STATIC_DESCR };
  for(uint i = 0; i < SIZE_A(idc); ++i) {
    GetWindowRect(GetDlgItem(*this, idc[i]), r);
    ipc[i].x = r.left;
    ipc[i].y = y;
    ipc[i].w = r.Width();
    ipc[i].h = h - 1;
    }

  Rows[0]->offset(ipc);
  for(uint j = 1; j < MAX_GROUP; ++j) {
    GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1 + j), r);
    for(uint i = 0; i < SIZE_A(idc); ++i)
      ipc[i].y = r.top;
    Rows[j]->offset(ipc);
    }

  ClearAll();
  setFileName();
  loadFile();
  loadData();
  setScrollRange();

  return true;
}
//----------------------------------------------------------------------------
extern void gRunSectionAlarmText(PWin* owner);
//----------------------------------------------------------------------------
LRESULT PD_Alarm::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_EDIT_SECTION:
          gRunSectionAlarmText(this);
          break;
        case IDC_BUTTON_PASTE_FROM_CLIP:
          copyFromClipboard();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
static
LPTSTR get_line(LPTSTR buff, int len)
{
  if(len < 0)
    return 0;
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(; i < len; ++i, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
void PD_Alarm::fillRowByClipboard(LPTSTR lptstr)
{
  int offs = 0;
  int dim = _tcslen(lptstr);
  for(int pos = currPos; lptstr && pos < MAX_ADDRESSES; ++pos) {
    LPTSTR next = get_line(lptstr, dim - offs);
    _tcscpy_s(Cfg[pos].text, lptstr);
    int len = next ? next - lptstr : dim - offs;
    offs += len;
    lptstr = next;
    }
  loadData();
  setDirty();
}
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define myCF_TEXT CF_UNICODETEXT
#else
  #define myCF_TEXT CF_TEXT
#endif
//----------------------------------------------------------------------------
void PD_Alarm::copyFromClipboard()
{
  if(!IsClipboardFormatAvailable(myCF_TEXT))
    return;
  if(!OpenClipboard(*this))
    return;
  HGLOBAL hglb = GetClipboardData(myCF_TEXT);
  while(hglb) {
    LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
    if(!lptstr)
      break;
    fillRowByClipboard(lptstr);
    GlobalUnlock(hglb);
    break;
    }
  CloseClipboard();
}
//----------------------------------------------------------------------------
bool myIcmp(LPCTSTR str1, LPCTSTR str2, uint len)
{
  for(uint i = 0; i < len; ++i, ++str1, ++str2)
    if(_totlower(*str1) != *str2)
      return false;
  return true;
}
//----------------------------------------------------------------------------
bool foundText(LPCTSTR text, LPCTSTR toFind, uint len)
{
  if(!*text)
    return false;
  int l = _tcslen(text) - len + 1;
  for(int i = 0; i < l; ++i)
    if(myIcmp(text + i, toFind, len))
      return true;
  return false;
}
//----------------------------------------------------------------------------
static int gSearchText(HWND edit, const baseRowAlarm* cfg, int currPos)
{
  static int last = -1;
  TCHAR t[512];
  GetWindowText(edit, t, SIZE_A(t));
  trim(lTrim(t));
  if(!*t)
    return -1;
  if(currPos == last)
    ++currPos;
  last = currPos;
  uint len = 0;
  for(; t[len]; ++len)
    t[len] = _totlower(t[len]);

  for(uint i = 0; i < MAX_ADDRESSES; ++i) {
    int pos = (currPos + i) % MAX_ADDRESSES;
    if(foundText(cfg[pos].text, t, len)) {
      last = pos;
      return pos;
      }
    }
  return -1;
}
//----------------------------------------------------------------------------
void PD_Alarm::searchText()
{
  int pos = gSearchText(GetDlgItem(*this, IDC_EDIT_SEARCH_TEXT), Cfg, currPos);
  if(pos >= 0)
    evVScrollBar(GetDlgItem(*this, IDC_SCROLLBAR_COUNT), SB_THUMBTRACK, pos);
}
//----------------------------------------------------------------------------
void PD_Alarm::Copy() {}
//----------------------------------------------------------------------------
void PD_Alarm::fill()
{
  baseRowAlarm* tmpCfg = new baseRowAlarm[MAX_ADDRESSES];
  for(int i = 0; i < MAX_ADDRESSES; ++i)
    tmpCfg[i] = Cfg[i];

  if(IDOK == PD_FillAlarm(tmpCfg, this).modal())  {
    for(int i = 0; i < MAX_ADDRESSES; ++i)
      Cfg[i] = tmpCfg[i];

    loadData();
    dirty = true;
    }
  delete []tmpCfg;
}
//------------------------------------------------------------------------------
const DWORD offsAssoc = 100000;
const DWORD offsText  = 200000;
const DWORD firstIdAssocText = 300001;
//------------------------------------------------------------------------------
bool PD_Alarm::saveData()
{
  PD_alarmEditorDlg* aed = getParentWin<PD_alarmEditorDlg>(this);
  PD_Assoc* assoc = aed->getAssoc(this);
  saveCurrData();
  assoc->saveCurrData();
  if(!isDirty() && !assoc->isDirty())
    return true;
  setFileName();
  TCHAR path[_MAX_PATH];
  makeSystemFilename(path, Filename + 1);
  do {
    P_File pf(path);
    pf.appendback();
    } while(false);

  P_File fileClear(path, P_CREAT);

  makeSystemFilename(path, Filename);
  do {
    P_File pf(path);
    pf.appendback();
    } while(false);
  manageCryptPage mcp;
  bool crypted = mcp.isCrypted();
  if(crypted)
    mcp.makePathCrypt(path);

  P_File fileCrypt(path, P_CREAT);

  bool success = fileCrypt.P_open() && fileClear.P_open();
  if(success) {
    const baseRowAssoc* aRows = assoc->getRows();

    const DWORD offsAssoc = 100000;
    const DWORD offsText  = 200000;
    DWORD idAssocText = firstIdAssocText;

    LPTSTR buff = new TCHAR[4096];
    for(uint i = 0; i < MAX_ADDRESSES; ++i) {
      if(Cfg[i].grp || Cfg[i].text[0] || Cfg[i].filter) {
        wsprintf(buff, _T("%d,%d,%d,%d\r\n"), i + 1, Cfg[i].filter, Cfg[i].grp, i + 1 + offsText);
        writeStringChkUnicode(fileCrypt, buff);
        wsprintf(buff, _T("%d,%s\r\n"), i + 1 + offsText, Cfg[i].text);
        writeStringChkUnicode(fileClear, fileCrypt, buff);

        if(aRows[i].prph) {
          TCHAR t = charFromDw(aRows[i].assocType);
          if(aRows[i].assocFile[0]) {
            TCHAR t2[_MAX_PATH];
            translateFromCRNL(t2, aRows[i].assocFile);
            wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%c,%s\r\n"), i + 1 + offsAssoc, aRows[i].prph,
                aRows[i].addr, aRows[i].type, aRows[i].nBit, aRows[i].offset, t, t2);
            }
          else
            wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%c\r\n"), i + 1 + offsAssoc, aRows[i].prph,
                aRows[i].addr, aRows[i].type, aRows[i].nBit, aRows[i].offset, t);
          writeStringChkUnicode(fileCrypt, buff);
          }
        }
      }
    delete []buff;
    resetDirty();
    assoc->resetDirty();
    }
  fileCrypt.P_close();
  if(crypted) {
    makeSystemFilename(path, Filename);
    mcp.releaseFileCrypt(path);
    }
  return success;
}
//------------------------------------------------------------------------------
void PD_Alarm::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    dirty |= Rows[i]->isDirty();
    Cfg[pos].grp = Rows[i]->getGroup();
    Cfg[pos].filter = Rows[i]->getFilter();
    Rows[i]->getDescr(Cfg[pos].text, SIZE_A(Cfg[pos].text));
    }
}
//------------------------------------------------------------------------------
void PD_Alarm::loadData()
{
  int pos = currPos;

  bool oldDirty = dirty;

  PD_alarmEditorDlg* aed = getParentWin<PD_alarmEditorDlg>(this);
  PD_Assoc* assoc = aed->getAssoc(this);
  baseRowAssoc* aRows = assoc->getRows();

  for(int i = 0; i < MAX_GROUP && pos < MAX_ADDRESSES; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos + 1);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);
    buff[0] = charFromDw(aRows[pos].assocType);
    buff[1] = 0;
    Rows[i]->setAll(Cfg[pos].grp, Cfg[pos].filter, buff);
    Rows[i]->setDescr(Cfg[pos].text);
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + i), 0, 0);
    Rows[i]->resetDirty();
    }
  dirty = oldDirty;
}
//----------------------------------------------------------------------------
bool PD_Alarm::isRowEmpty(uint pos)
{
  pos += IDC_EDIT_FIRST_FIELD;
  for(int i = 0; i < 2; ++i, pos += DEF_OFFSET_CTRL)
    if(GetDlgItemInt(*this, pos, 0, true))
      return false;

  TCHAR buff[500];
  for(int i = 2; i < 3; ++i, pos += DEF_OFFSET_CTRL) {
    GetWindowText(GetDlgItem(*this, pos), buff, SIZE_A(buff));
    if(*buff)
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
uint getValidNumber(LPCTSTR p)
{
  uint len = _tcslen(p);
  for(uint i = 0; i < len; ++i)
    if(!_istdigit(p[i]))
      return 0;
  uint numb = _ttol(p);

  return numb > MAX_ADDRESSES ? numb : 0;
}
//----------------------------------------------------------------------------
void PD_Alarm::loadAssocFile(long id, setOfString& Set, setOfString& Set2, baseRowAssoc* aRows, LPCTSTR p)
{
  id -= offsAssoc;
  uint prph = _ttoi(p);
  if(!prph)
    return;
  baseRowAssoc& bra = aRows[id - 1];
  _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &bra.prph, &bra.addr, &bra.type, &bra.nBit, &bra.offset);
  p = findNextParamTrim(p, 5);
  TCHAR assocType = *p;
  switch(assocType) {
    case _T('-'):
    case _T('N'):
    case _T('V'):
    case _T('B'):
    case _T('n'):
    case _T('v'):
    case _T('b'):
      break;
    default:
      assocType = _T('-');
      break;
    }
  bra.assocType = dwFromChar(assocType);
  p = findNextParamTrim(p);
  if(p)
    _tcscpy_s(bra.assocFile, p);
}
//----------------------------------------------------------------------------
bool PD_Alarm::saveOnExit()
{
  PD_alarmEditorDlg* aed = getParentWin<PD_alarmEditorDlg>(this);
  PD_Assoc* assoc = aed->getAssoc(this);
  saveCurrData();
  assoc->saveCurrData();

  dirty |= assoc->isDirty();
  return baseClass::saveOnExit();
}
//----------------------------------------------------------------------------
bool PD_Alarm::loadFile()
{
  setOfString Set;
  setOfString Set2;
  makeSystemSet(Set, Filename);
  makeSystemSet(Set2, Filename + 1);
  if(!Set.setFirst()) {
    makeSystemSet(Set, Filename + 1);
    if(!Set.setFirst())
      return false;
    }
  ClearAll();
  PD_alarmEditorDlg* aed = getParentWin<PD_alarmEditorDlg>(this);
  PD_Assoc* assoc = aed->getAssoc(this);
  assoc->ClearAll();
  baseRowAssoc* aRows = assoc->getRows();
  do {
    long id = Set.getCurrId();
    if(id > 0 && id <= MAX_ADDRESSES) {
      LPCTSTR p = Set.getCurrString();
      Cfg[id - 1].filter = _ttol(p);
      p = findNextParamTrim(p, 1);
      if(p) {
        Cfg[id - 1].grp = _ttol(p);
        p = findNextParamTrim(p, 1);
        }

      if(p) {
        uint ix = getValidNumber(p);
        if(ix)
          p = Set2.getString(ix);

        if(p) {
          int len = _tcslen(p);
          if(len >= MAX_TEXT)
            len = MAX_TEXT - 1;
          copyStr((LPTSTR)Cfg[id - 1].text, p, len);
          Cfg[id - 1].text[len] = 0;
          }
        }
      else {
        Cfg[id - 1].grp =  Cfg[id - 1].filter;
        Cfg[id - 1].filter = 0;
        p = Set.getCurrString();
        p = findNextParamTrim(p);
        if(p) {
          int len = _tcslen(p);
          if(len >= MAX_TEXT)
            len = MAX_TEXT - 1;
          copyStr((LPTSTR)Cfg[id - 1].text, p, len);
          Cfg[id - 1].text[len] = 0;
          }
        }
      }
    else if(id > offsAssoc && id <= offsAssoc + MAX_ADDRESSES)
      loadAssocFile(id, Set, Set2, aRows, Set.getCurrString());

    } while(Set.setNext());
  dirty = false;
  assoc->resetDirty();
  return true;
}
//------------------------------------------------------------------------------
