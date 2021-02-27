//-------- P_ModListFiles.cpp ---------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <commctrl.h>
#include "mainwin.h"
#include "P_ModListFiles.h"
#include "p_Util.h"
#include "p_date.h"
#include "resource.h"
#include "P_file.h"
#include "P_txt.h"
#include <stdio.h>
#include "smartPS.h"
//------------------------------------------------------------------
//------------------------------------------------------------------
static int getDateFormat()
{
  static int dateFormat = -1;
  if(dateFormat < 0) {
    dateFormat = European;
    TCHAR buff[80];
    int res = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, buff, SIZE_A(buff));
    if(res) {
      switch(buff[0]) {
        case _T('d'):
        case _T('D'):
        default:
          dateFormat = European;
          break;
        case _T('m'):
        case _T('M'):
          dateFormat = American;
          break;
          return American;
        case _T('y'):
        case _T('Y'):
          dateFormat = Other;
          break;
        }
      }
    }
  return dateFormat;
}
//-------------------------------------------------------------------
#define LV_TXT_COLOR RGB(0, 0, 0xb3)
//#define LV_BKG_COLOR RGB(0xf8, 0xe4, 0xb3)
#define LV_BKG_COLOR RGB(0xf9, 0xf8, 0xe8)
//-------------------------------------------------------------------
static void performCopyFile(P_File& pft, P_File& pfs)
{
  BYTE buff[4096 * 2];
  pfs.P_seek(0);
  pft.P_seek(0);
  long dim = (long)pfs.get_len();
  while(dim > 0) {
    int toread = min(dim, sizeof(buff));
    pfs.P_read(buff, toread);
    pft.P_write(buff, toread);
    dim -= toread;
    }
}
//-------------------------------------------------------------------
#define D_BUFF 256
static void performCopyFile(P_File& pft, P_File& pfs, size_t szRec, size_t fromS, size_t to, size_t fromT)
{
  P_SmartPointer<LPBYTE> buff(new BYTE[szRec * D_BUFF], true);
  pfs.P_seek(fromS * szRec);
  pft.P_seek(fromT * szRec);
  long dim = (long)pfs.get_len() - fromS * szRec;
  while(dim > 0 && fromS < to) {
    long toread = min(D_BUFF, to - fromS);
    fromS += toread;
    toread *= szRec;
    pfs.P_read(buff, toread);
    pft.P_write(buff, toread);
    dim -= toread;
    }
}
//-------------------------------------------------------------------
struct infoColumn
{
  int ncol;
  int pos;
  infoColumn(int ncol = 0, int pos = 0) : ncol(ncol), pos(pos) {}
};
//-------------------------------------------------------------------
#define SIZE_REC(ncol) (sizeof(FILETIME) + sizeof(float) * (ncol))
//-------------------------------------------------------------------
static void performAddColumnFile(P_File& pft, P_File& pfs, const infoColumn& ic)
{
  int szRec = SIZE_REC(ic.ncol);
  P_SmartPointer<LPBYTE> rec(new BYTE[szRec], true);
  pfs.P_seek(0);
  pft.P_seek(0);
  long dim = (long)pfs.get_len();
  int offs = SIZE_REC(ic.pos);
  int remain = SIZE_REC(ic.ncol) - offs;
  float t = 0;
  while(dim > 0) {
    int toread = min(dim, szRec);
    pfs.P_read(rec, toread);
    pft.P_write(rec, offs);
    pft.P_write(&t, sizeof(t));
    pft.P_write(rec + offs, remain);
    dim -= toread;
    }
}
//-------------------------------------------------------------------
static void performRemColumnFile(P_File& pft, P_File& pfs, const infoColumn& ic)
{
  int szRec = SIZE_REC(ic.ncol);
  P_SmartPointer<LPBYTE> rec(new BYTE[szRec], true);

  pfs.P_seek(0);
  pft.P_seek(0);
  long dim = (long)pfs.get_len();
  int offs = SIZE_REC(ic.pos);
  int remain = SIZE_REC(ic.ncol) - offs - sizeof(float);
  while(dim > 0) {
    int toread = min(dim, szRec);
    pfs.P_read(rec, toread);
    pft.P_write(rec, offs);
    pft.P_write(rec + offs + sizeof(float), remain);
    dim -= toread;
    }
}
//-------------------------------------------------------------------
struct infoRow
{
  int ncol;
  PVect<int> pos;
  infoRow(int ncol = 0) : ncol(ncol) {}
};
//-------------------------------------------------------------------
static void performAddRowFile(P_File& pft, P_File& pfs, const infoColumn& ic)
{
  int szRec = SIZE_REC(ic.ncol);
  P_SmartPointer<LPBYTE> rec(new BYTE[szRec], true);
  pfs.P_seek(0);
  pft.P_seek(0);
  long dim = (long)pfs.get_len();
  if(!ic.pos) {
    int toread = min(dim, szRec);
    pfs.P_read(rec, toread);
    FILETIME ft = *(FILETIME*)&rec;
    ft -= SECOND_TO_I64 / 10 * 2;
    P_SmartPointer<LPBYTE> rec2(new BYTE[szRec], true);
    ZeroMemory(rec2, szRec);
    *(FILETIME*)&rec2 = ft;
    pft.P_write(rec2, toread);
    pft.P_write(rec, toread);
    dim -= toread;
    }
  else {
    float t = 0;
    int row = 0;
    while(dim > 0) {
      int toread = min(dim, szRec);
      pfs.P_read(rec, toread);
      pft.P_write(rec, toread);
      dim -= toread;
      if(++row >= ic.pos)
        break;
      }
    if(dim) {
      FILETIME ft = *(FILETIME*)&rec;
      int toread = min(dim, szRec);
      pfs.P_read(rec, toread);
      FILETIME ft2 = *(FILETIME*)&rec;
      ft = I64_TO_FT((MK_I64(ft) + MK_I64(ft2)) / 2);
      if(ft > ft2)
        ft += SECOND_TO_I64 / 10 * 2;
      ZeroMemory(rec, szRec);
      *(FILETIME*)&rec = ft;
      pft.P_write(rec, toread);
      pfs.P_seek(-szRec, SEEK_CUR_);
      }
    else {
      FILETIME ft = *(FILETIME*)&rec;
      ft += SECOND_TO_I64 / 10 * 2;
      ZeroMemory(rec, szRec);
      *(FILETIME*)&rec = ft;
      pft.P_write(rec, szRec);
      }
    }
  while(dim > 0) {
    int toread = min(dim, szRec);
    pfs.P_read(rec, toread);
    pft.P_write(rec, toread);
    dim -= toread;
    }
}
//-------------------------------------------------------------------
static void performRemRowFile(P_File& pft, P_File& pfs, const infoRow& ir)
{
  int szRec = SIZE_REC(ir.ncol);
  P_SmartPointer<LPBYTE> rec(new BYTE[szRec], true);
  pfs.P_seek(0);
  pft.P_seek(0);
  long dim = (long)pfs.get_len();
  int nrec = dim / szRec;
  uint ix = 0;
  long i = 0;
  for(; i < nrec; ++i) {
    pfs.P_read(rec, szRec);
    if(i == ir.pos[ix]) {
      if(++ix >= ir.pos.getElem())
        break;
      continue;
      }
    pft.P_write(rec, szRec);
    }
  for(; i < nrec; ++i) {
    pfs.P_read(rec, szRec);
    pft.P_write(rec, szRec);
    }
}
//-------------------------------------------------------------------
#define IDC_LIST1 101
//-----------------------------------------------------------
class myPCustomListViewFullEdit : public PCustomListViewFullEdit
{
  private:
    typedef PCustomListViewFullEdit baseClass;
  public:
    myPCustomListViewFullEdit(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, text, hinst), modDate(false) {  }

    myPCustomListViewFullEdit(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, text, hinst), modDate(false) { }

    myPCustomListViewFullEdit(PWin * parent, uint resid, HINSTANCE hinst = 0) :
         baseClass(parent, resid, hinst), modDate(false)
         {  }
    bool ProcessNotify(LPARAM lParam, LRESULT& result);
    void setModifyDate(bool set) { modDate = set; }

  protected:
    virtual bool canEditItem(int item, int subItem) const { return item != -1 && subItem > (1 - modDate); }
    bool modDate;
};
//-----------------------------------------------------------
bool myPCustomListViewFullEdit::ProcessNotify(LPARAM lParam, LRESULT& result)
{
  LPNMHDR lpnmHdr = (LPNMHDR)lParam;
  switch(lpnmHdr->code) {
    case LVN_ENDLABELEDIT:
      do {
        NMLVDISPINFO * lvdisp = (NMLVDISPINFO*)lParam;
        LVITEM * lvItem = &lvdisp->item;
        if(iSubItem > (1 - modDate)) {
          lvItem->iSubItem = iSubItem;
          P_ModListFiles* par = getParentWin<P_ModListFiles>(getParent());
          if(par)
            par->modifyRec(lvItem);
          lvItem->iSubItem = 0;
          }
        } while(false);
      break;
    }
  return baseClass::ProcessNotify(lParam, result);
}
//-------------------------------------------------------------------
P_ModListFiles::~P_ModListFiles()
{
  destroy();
}
//-------------------------------------------------------------------
bool P_ModListFiles::save(LPCTSTR file)
{
  if(!pF)
    return false;
  if(!_tcsicmp(file, pF->get_name()))
    return false;
  do {
    P_File pf(file);
    pf.appendback();
    } while(false);
  P_File* pf = new P_File(file, P_CREAT);
  if(!pf->P_open())
    return false;
  performCopyFile(*pf, *pF);
  Dirty = false;
  pF->P_close();
  int len = _tcslen(pF->get_name());
  if(!_tcsicmp(_T(".tmp"), pF->get_name() + len - 4))
    DeleteFile(pF->get_name());
  delete pF;
  pF = pf;
  return true;
}
//----------------------------------------------------------------------------
static
int myExtent(HWND hWnd, LPCTSTR lpString)
{
  return max(extent(hWnd, lpString), 100);
}
//-------------------------------------------------------------------
bool P_ModListFiles::create()
{
  List = new myPCustomListViewFullEdit(this, IDC_LIST1, PRect(0, 0, 100, 100));
  List->Attr.style |= LVS_SHOWSELALWAYS | LVS_OWNERDATA;
  if(!baseClass::create())
    return false;
  List->setFont(D_FONT(18, 0, 0, _T("Arial")), true);

  DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  ListView_SetExtendedListViewStyle(*List, dwExStyle);


  ListView_SetBkColor(*List, LV_BKG_COLOR);
  ListView_SetTextBkColor(*List, LV_BKG_COLOR);

  ListView_SetTextColor(*List, LV_TXT_COLOR);

  LVCOLUMN lvc;
  lvc.mask = LVCF_WIDTH | LVCF_TEXT;
  lvc.pszText = (LPTSTR)getStringOrDef(IDT_HEAD_NUM, _T("Num"));
  lvc.cx = 60;
  if(-1 == ListView_InsertColumn(*List, 0, &lvc))
    return false;
  lvc.pszText = (LPTSTR)getStringOrDef(IDT_HEAD_DATE, _T("Date-time"));
  lvc.cx = myExtent(*List, _T("00-00-0000  00:00:00.000")); // 200;
  if(-1 == ListView_InsertColumn(*List, 1, &lvc))
    return false;
  ListView_SetItemCountEx(*List, 0, LVSICF_NOINVALIDATEALL);
  nCol = 2;
  return true;
}
//-------------------------------------------------------------------
void P_ModListFiles::setModifyDate(bool set)
{
  myPCustomListViewFullEdit* l = dynamic_cast<myPCustomListViewFullEdit*>(List);
  l->setModifyDate(set);
}
//-------------------------------------------------------------------
void P_ModListFiles::makeHead(LPCTSTR file, int n_col)
{
  TCHAR text[_MAX_PATH];
  _tcscpy_s(text, file);
  dirName(text);
  int len = _tcslen(text);
  LPCTSTR p = file + len;
  for(; *p; ++p, ++len) {
    if(_T('_') == *p)
      break;
    text[len] = *p;
    }
  text[len] = 0;
  p = file + _tcslen(file) - 1;
  for(; *p; --p) {
    if(_T('.') == *p)
      break;
    }
  _tcscat_s(text, p);
  _tcscat_s(text, _T(".txt"));

  setOfString set(text);

  for(int i = nCol - 1; i >= 2; --i)
    ListView_DeleteColumn(*List, i);

  LVCOLUMN lvc;
  lvc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT;
  lvc.fmt = LVCFMT_CENTER;
  lvc.cx = 60;
  TCHAR buff[64];

  for(int i = 2; i < n_col; ++i) {
    p = set.getString(i);
    if(!p) {
      wsprintf(buff, _T("[%d]"), i - 1);
      p = buff;
      }
    lvc.pszText = (LPTSTR)p;
    lvc.cx = myExtent(*List, lvc.pszText);
    if(-1 == ListView_InsertColumn(*List, i, &lvc))
      return;
    }
}
//-------------------------------------------------------------------
void P_ModListFiles::load(LPCTSTR file, int n_col)
{
  szRec = 0;
  nRec = 0;

  makeHead(file, n_col);
  nCol = n_col;
  delete pF;
  pF = new P_File(file);
  if(!pF->P_open())
    return;
  szRec = sizeof(FILETIME) + (n_col - 2) * sizeof(float);
  nRec = int(pF->get_len() / szRec);
  ListView_SetItemCountEx(*List, nRec, 0);
  Dirty = false;
//  ListView_SetItemCountEx(*List, nRec, LVSICF_NOINVALIDATEALL);
}
//-------------------------------------------------------------------
bool P_ModListFiles::copyFile()
{
  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, pF->get_name());
  _tcscat_s(file, _T(".tmp"));
  P_File* pf2 = new P_File(file, P_CREAT);
  if(!pf2->P_open())
    return false;
  performCopyFile(*pf2, *pF);
  delete pF;
  pF = pf2;
  return true;
}
//-------------------------------------------------------------------
void P_ModListFiles::modifyRec(LV_ITEM* pItem)
{
  if(!isDirty()) {
    if(!copyFile())
      return;
    Dirty = true;
    }
  if(pItem->mask & LVIF_TEXT) {
    P_SmartPointer<LPBYTE> buff(new BYTE[szRec], true);
    pF->P_seek(pItem->iItem * szRec);
    pF->P_read(buff, szRec);
    if(1 == pItem->iSubItem) {
      FILETIME ft;
      unformat_data(pItem->pszText, ft, (eDataFormat)getDateFormat());
      FILETIME* fl = (FILETIME*)&buff;
      *fl = ft;
      pF->P_seek(pItem->iItem * szRec);
      pF->P_write(buff, szRec);
      }
    if(pItem->iSubItem > 1) {
      float v = (float)_tstof(pItem->pszText);
      float* fl = (float*)(buff + sizeof(FILETIME) + sizeof(float) * (pItem->iSubItem - 2));
      *fl = v;
      pF->P_seek(pItem->iItem * szRec);
      pF->P_write(buff, szRec);
      }
    }
}
//----------------------------------------------------------------
static void set_format_data(LPTSTR buff, size_t lenBuff, const FILETIME& ft, LPCTSTR sep = _T("  "))
{
  SYSTEMTIME stm;
  FileTimeToSystemTime(&ft, &stm);
  int ftd = getDateFormat();
  switch(ftd) {
    case  European:
    default:
      _stprintf_s(buff, lenBuff, _T("%02d-%02d-%04d%s%02d:%02d:%02d.%03d"),
              stm.wDay, stm.wMonth, stm.wYear, sep,
              stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);
      break;
    case  American:
      _stprintf_s(buff, lenBuff, _T("%02d-%02d-%04d%s%02d:%02d:%02d.%03d"),
              stm.wMonth, stm.wDay, stm.wYear, sep,
              stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);
      break;
    case  Other:
      _stprintf_s(buff, lenBuff, _T("%04d-%02d-%02d%s%02d:%02d:%02d.%03d"),
              stm.wYear, stm.wMonth, stm.wDay, sep,
              stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);
      break;
    }
}
//-------------------------------------------------------------------
void P_ModListFiles::loadRec(NMLVDISPINFO *pdi)
{
  LV_ITEM* pItem = &pdi->item;
  if(pItem->mask & LVIF_TEXT) {
    P_SmartPointer<LPBYTE> buff(new BYTE[szRec], true);
    pF->P_seek(pItem->iItem * szRec);
    pF->P_read(buff, szRec);
    TCHAR t[64];
    if(!pItem->iSubItem)
      wsprintf(t, _T("%d"), pItem->iItem + 1);
    else if(1 == pItem->iSubItem)
      set_format_data(t, SIZE_A(t), *(FILETIME*)&buff);
    else {
      float fl = *(float*)(buff + sizeof(FILETIME) + sizeof(float) * (pItem->iSubItem - 2));
      _stprintf_s(t, _T("%0.5f"), fl);
      zeroTrim(t);
      }
    _tcscpy_s(pItem->pszText, pItem->cchTextMax, t);
    }
}
//-------------------------------------------------------------------
LRESULT P_ModListFiles::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

  switch(message) {
    case WM_NOTIFY:
      if(LOWORD(wParam) == IDC_LIST1) {
        if(LVN_GETDISPINFO == ((LPNMHDR)lParam)->code) {
          if(nRec)
            loadRec((NMLVDISPINFO*)lParam);
          }
        else {
          LRESULT res;
          if(List->ProcessNotify(lParam, res))
            return res;
          }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void P_ModListFiles::fill(fillInfo& fi)
{
  load(fi.file, fi.nCol + 2);
}
//----------------------------------------------------------------------------
P_File* P_ModListFiles::commonColumnRow()
{
  if(!pF)
    return 0;
  if(!isDirty()) {
    if(!copyFile())
      return 0;
    Dirty = true;
    }
  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, pF->get_name());
  pF->appendback();

  P_File* pf = new P_File(file, P_CREAT);
  if(!pf->P_open()) {
    delete pf;
    return 0;
    }
  return pf;
}
//----------------------------------------------------------------------------
bool P_ModListFiles::addColumn(int pos)
{
  P_File* pf = commonColumnRow();
  infoColumn ic(nCol - 2, pos);
  performAddColumnFile(*pf, *pF, ic);
  pF->P_close();
  DeleteFile(pF->get_name());
  delete pF;
  pF = pf;
  reloadCurr(true);
  return true;
}
//----------------------------------------------------------------------------
bool P_ModListFiles::remColumn(int pos)
{
  P_File* pf = commonColumnRow();
  infoColumn ic(nCol - 2, pos);
  performRemColumnFile(*pf, *pF, ic);
  pF->P_close();
  DeleteFile(pF->get_name());
  delete pF;
  pF = pf;
  reloadCurr(false);
  return true;
}
//----------------------------------------------------------------------------
void P_ModListFiles::reloadCurr(int n_col)
{
  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, pF->get_name());
  load(file, n_col);
  Dirty = true;
}
//----------------------------------------------------------------------------
void P_ModListFiles::reloadCurr(bool forAdd)
{
  int ncol = nCol + (forAdd ? 1 : -1);
  reloadCurr( ncol);
}
//----------------------------------------------------------------------------
static void getCurrSel(HWND hwlv, PVect<int>& res)
{
  int pos = -1;
  uint nElem = 0;
  do {
    pos = ListView_GetNextItem(hwlv, pos, LVNI_ALL | LVNI_SELECTED);
    if(pos < 0)
      break;
    ++nElem;
    }  while(true);
  res.setDim(nElem);
  pos = -1;
  do {
    pos = ListView_GetNextItem(hwlv, pos, LVNI_ALL | LVNI_SELECTED);
    if(pos < 0)
      break;
    res[res.getElem()] = pos;
    }  while(true);
}
//----------------------------------------------------------------------------
static void getRangeSel(HWND hwlv, PVect<int>& res)
{
  int pos = ListView_GetNextItem(hwlv, -1, LVNI_ALL | LVNI_SELECTED);
  if(pos < 0)
    return;
  res[res.getElem()] = pos;
  int lastPos = pos;
  do {
    pos = ListView_GetNextItem(hwlv, pos, LVNI_ALL | LVNI_SELECTED);
    if(pos < 0) {
      res[res.getElem()] = lastPos;
      break;
      }
    if(pos - lastPos > 1) {
      res[res.getElem()] = lastPos;
      res[res.getElem()] = pos;
      }
    lastPos = pos;
    }  while(true);
}
//----------------------------------------------------------------------------
bool P_ModListFiles::hasSelected()
{
  return ListView_GetNextItem(*List, -1, LVNI_ALL | LVNI_SELECTED) >= 0;
}
//------------------------------------------------------------------
static TCHAR getDecSep()
{
  TCHAR buff[4];
  int res = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, buff, SIZE_A(buff));
  if(res)
    return buff[0];
  return _T(',');
}
//------------------------------------------------------------------
static void fromSepToPoint(LPTSTR t, LPCTSTR s, TCHAR sep)
{
  while(*s) {
    if(sep == *s)
      *t++ = _T('.');
    else
      *t++ = *s;
    ++s;
    }
  *t = 0;
}
//------------------------------------------------------------------
static void fromPointToSep(LPTSTR t, LPCTSTR s, TCHAR sep)
{
  while(*s) {
    if(_T('.') == *s)
      *t++ = sep;
    else
      *t++ = *s;
    ++s;
    }
  *t = 0;
}
//------------------------------------------------------------------
static void fromPointToSep(LPTSTR t, TCHAR sep)
{
  while(*t) {
    if(_T('.') == *t)
      *t = sep;
    ++t;
    }
}
//----------------------------------------------------------------------------
void P_ModListFiles::formatRowClipboard(int sel, LPTSTR buff, size_t dim)
{
  P_SmartPointer<LPBYTE> rec(new BYTE[szRec], true);
  pF->P_seek(sel * szRec);
  pF->P_read(rec, szRec);
  set_format_data(buff, dim, *(FILETIME*)&rec, _T("\t"));
  LPBYTE rec2 = rec;
  rec2 += sizeof FILETIME;
  LPTSTR p = buff + _tcslen(buff);
  LPTSTR p2 = p;
  int nc = getCurrNumCol();
  for(int i = 0; i < nc; ++i) {
    _stprintf_s(p, dim - (p - buff), _T("\t%f"), *(float*)rec2);
    rec2 += sizeof(float);
    p += _tcslen(p);
    }
  fromPointToSep(p2, getDecSep());
  _tcscat_s(p, dim - (p - buff), _T("\r\n"));
}
//----------------------------------------------------------------------------
void P_ModListFiles::copyCurrent()
{
  if(!OpenClipboard(*this))
    return;
  EmptyClipboard();

  PVect<int> selected;
  getCurrSel(*List, selected);
  uint nElem = selected.getElem();
  PVect<LPCTSTR> rows;
  rows.setDim(nElem);
  TCHAR buff[4096 * 4];
  uint dim = 0;
  for(uint i = 0; i < nElem; ++i) {
    int sel = selected[i];
    formatRowClipboard(sel, buff, SIZE_A(buff));
    dim += _tcslen(buff) + 2;
    rows[i] = str_newdup(buff);
    }
  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (dim + 16) * sizeof(TCHAR));
  if(hglbCopy) {
    LPTSTR b = (LPTSTR)GlobalLock(hglbCopy);
    LPTSTR pb = b;
    int d = dim + 16;
    for(uint i = 0; i < nElem; ++i) {
      _tcscpy_s(pb, d - (pb - b), rows[i]);
      pb += _tcslen(pb);
      }
    GlobalUnlock(hglbCopy);
    SetClipboardData(myCF_TEXT, hglbCopy);
    }
  CloseClipboard();
  flushPAV(rows);
}
//----------------------------------------------------------------------------
void P_ModListFiles::addRow(uint type) // 0 -> posizione corrente, 1 -> prima riga, 2 -> appende
{
  infoColumn ic(nCol - 2, 0);
  switch(type) {
    case 0:
      do {
        PVect<int> selected;
        getCurrSel(*List, selected);
        if(selected.getElem())
          ic.pos = selected[0];
        } while(false);
      break;
    case 2:
      ic.pos = nRec;
      break;
    }
  P_File* pf = commonColumnRow();
  performAddRowFile(*pf, *pF, ic);
  pF->P_close();
  DeleteFile(pF->get_name());
  delete pF;
  pF = pf;
  ++nRec;
  ListView_SetItemCountEx(*List, nRec, 0);

  ListView_SetItemState(*List, ic.pos, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
  ListView_EnsureVisible(*List, ic.pos, TRUE);
}
//----------------------------------------------------------------------------
void P_ModListFiles::remRow()
{
  infoRow ir(nCol - 2);
  getCurrSel(*List, ir.pos);
  int nElem = ir.pos.getElem();
  if(!nElem)
    return;
  // se selezionati tutti, ne lascia almeno uno
  if(nElem >= nRec) {
    ir.pos.remove(nElem - 1);
    if(! ir.pos.getElem())
      return;
    }
  P_File* pf = commonColumnRow();
  performRemRowFile(*pf, *pF, ir);
  pF->P_close();
  DeleteFile(pF->get_name());
  delete pF;
  pF = pf;
  nRec -= ir.pos.getElem();
  ListView_SetItemCountEx(*List, nRec, 0);
  ListView_SetItemState(*List, ir.pos[0], LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
  ListView_EnsureVisible(*List, ir.pos[0], TRUE);
}
//------------------------------------------------------------------
class myRows : public  manageRowOfBuff
{
  private:
    typedef  manageRowOfBuff baseClass;
  public:
    myRows(uint len, LPTSTR buffer, pvvv& target) : baseClass(len, buffer), target(target) {}
  protected:
    virtual bool manageLine(LPCTSTR row);
  private:
     pvvv& target;
};
//----------------------------------------------------------------------------
bool myRows::manageLine(LPCTSTR row)
{
  int n = target.getElem();
  splitParam(target[n], row, _T('\t'));
  return true;
}
//----------------------------------------------------------------------------
void P_ModListFiles::copyRow( P_File& pf, const pvvChar& row, LPBYTE buff)
{
  int nElem = row.getElem() - 1;
  nElem = min(nElem, getCurrNumCol());
  if(nElem < getCurrNumCol())
    ZeroMemory(buff, szRec);
  TCHAR t[64];
  _tcscpy_s(t, &row[0]);
  _tcscat_s(t, _T("-"));
  _tcscat_s(t, &row[1]);
  unformat_data(t, *(FILETIME*)buff, (eDataFormat)getDateFormat());
  float* b = (float*)(buff + sizeof(FILETIME));
  nElem += 2;
  TCHAR sep = getDecSep();
  for(int i = 2; i < nElem; ++i, ++b) {
    fromSepToPoint(t, &row[i], sep);
    float v = (float)_tstof(t);
    *b = v;
    }
  pf.P_write(buff, szRec);
}
//----------------------------------------------------------------------------
void P_ModListFiles::fillRowByClipboard(const pasteInfo& pi, const  pvvv& target)
{
  P_File* pf = commonColumnRow();
  size_t after = pi.after;
  bool append = !(pi.type & 4);
  if(append) {
    if(pi.type & 8)
      after = nRec;
    else {
      after = 0;
      append = false;
      }
    }
  performCopyFile(*pf, *pF, szRec, 0, after, 0);
  uint nElem = target.getElem();
  P_SmartPointer<LPBYTE> buff(new BYTE[szRec], true);
  for(uint i = 0; i < nElem; ++i)
    copyRow(*pf, target[i], buff);
  size_t from = after;
  if(!append && toBool(pi.type & 2))
    from += nElem;
  performCopyFile(*pf, *pF, szRec, from, nRec, after + nElem);

  pF->P_close();
  DeleteFile(pF->get_name());
  delete pF;
  pF = pf;
  reloadCurr(nCol);
}
//----------------------------------------------------------------------------
void P_ModListFiles::copyRowCols(P_File& pf, const pvvChar& row, LPBYTE buffS, LPBYTE buffT, int offset, bool overwrite)
{
  int nElem = row.getElem();

  pF->P_read(buffS, szRec);
  memcpy_s(buffT, szRec, buffS, szRec);

  float* b = (float*)(buffT + sizeof(FILETIME) + offset * sizeof(float));
  int szRecT = szRec;

  TCHAR sep = getDecSep();
  TCHAR t[64];
  if(overwrite) {
    nElem = min(nElem, getCurrNumCol() - offset);
    for(int i = 0; i < nElem; ++i, ++b) {
      fromSepToPoint(t, &row[i], sep);
      float v = (float)_tstof(t);
      *b = v;
      }
    }
  else {
    szRecT += nElem * sizeof(float);
    for(int i = 0; i < nElem; ++i, ++b) {
      fromSepToPoint(t, &row[i], sep);
      float v = (float)_tstof(t);
      *b = v;
      }
    if(offset < getCurrNumCol()) {
      float* bS = (float*)(buffS + sizeof(FILETIME) + offset * sizeof(float));
      nElem = getCurrNumCol();
      for(int i = offset; i < nElem; ++i)
        *b++ = *bS++;
      }
    }
  pf.P_write(buffT, szRecT);
}
//----------------------------------------------------------------------------
void P_ModListFiles::fillByClipboard(const pasteInfo& pI, const  pvvv& target)
{
  if(pI.type & 1)
    fillRowByClipboard(pI, target);
  else {
    int pasteCol = target[0].getElem();
    DWORD after = pI.after;
    if(!(pI.type & 4)) {
      if(pI.type & 8)
        after = getCurrNumCol();
      else
        after = 0;
      }
    bool overwrite = toBool(pI.type & 2);

    P_File* pf = commonColumnRow();
    // può essere solo overwrite e allora non cambia la dimensione del record
    if(pI.fromRow)
      performCopyFile(*pf, *pF, szRec, 0, pI.fromRow, 0);
    // non controllo se sono più di quelli presenti, vengono aggiunti record con i dati
    // non presenti copiati dall'ultimo record sorgente
    uint nElem = target.getElem();
    P_SmartPointer<LPBYTE> buff(new BYTE[szRec], true);
    int szRecT = szRec + (overwrite ? 0 : pasteCol * sizeof(float));
    P_SmartPointer<LPBYTE> buffT(new BYTE[szRecT], true);
    for(uint i = 0; i < nElem; ++i)
      copyRowCols(*pf, target[i], buff, buffT, after, overwrite);
    long offset = pI.fromRow + nElem;
    if(offset < nRec) {
      if(overwrite)
        performCopyFile(*pf, *pF, szRec, offset, nRec, offset);
      else
        for(int i = offset; i < nRec; ++i)
          copyRowCols(*pf, target[nElem - 1], buff, buffT, after, false);
      }
    pF->P_close();
    DeleteFile(pF->get_name());
    delete pF;
    pF = pf;
    int n_col = nCol;
    if(!overwrite)
      n_col += pasteCol;
    reloadCurr(n_col);
    }
}
//----------------------------------------------------------------------------
#ifdef UNICODE
  #define myCF_TEXT CF_UNICODETEXT
#else
  #define myCF_TEXT CF_TEXT
#endif
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void P_ModListFiles::paste(const pasteInfo& pi)
{
  if(!IsClipboardFormatAvailable(myCF_TEXT))
    return;
  if(!OpenClipboard(*this))
    return;
  HGLOBAL hglb = GetClipboardData(myCF_TEXT);
  LPTSTR str = 0;
  while(hglb) {
    LPCTSTR lptstr = (LPCTSTR)GlobalLock(hglb);
    if(!lptstr)
      break;
    str = str_newdup(lptstr);
    GlobalUnlock(hglb);
    break;
    }
  CloseClipboard();
  if(str) {
    pvvv target;
    myRows(_tcslen(str), str, target).run();
    delete []str;
    fillByClipboard(pi, target);
    }
}
//----------------------------------------------------------------------------
void P_ModListFiles::getHeaderText(LPTSTR target, uint sz, uint colnum)
{
  LVCOLUMN lvc;
  lvc.mask = LVCF_TEXT;
  lvc.pszText = target;
  lvc.cchTextMax = sz;
  if(!ListView_GetColumn(*List, colnum + 2, &lvc))
    wsprintf(target, _T("value_%d"), colnum + 1);
}
//----------------------------------------------------------------------------
static P_File* createFileExp(LPCTSTR filename, LPCTSTR ext)
{
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, filename);
  _tcscat_s(path, ext);
  P_File* pf = new P_File(path, P_CREAT);
  if(!pf->P_open()) {
    delete pf;
    // msg
    return 0;
    }
  return pf;
}
//----------------------------------------------------------------------------
class export_
{
  public:
    export_(P_ModListFiles* owner, const exportInfo& eI, P_File* curr, int currCol) :
          Owner(owner), eI(eI), Curr(curr), currCol(currCol) {}
    virtual ~export_() {}
    virtual bool run();
  protected:
    P_ModListFiles* Owner;
    const exportInfo& eI;
    P_File* Curr;
    int currCol;
    virtual LPCTSTR getExt() const = 0;
    virtual void performExportHead(P_File& pf, int nField) = 0;
    virtual void performExport(P_File& pf, LPCTSTR source, int nField) = 0;
    virtual void performExportRange(P_File& pf, P_File& source, int nField, int from, int to) = 0;

    void msgNoOpen(LPCTSTR filename, bool create);
};
//----------------------------------------------------------------------------
class export_tab : public export_
{
  private:
    typedef export_ baseClass;
  public:
    export_tab(P_ModListFiles* owner, const exportInfo& eI, P_File* curr, int currCol) : baseClass(owner, eI, curr, currCol) {}
  protected:
    virtual LPCTSTR getExt() const { return _T(".tab"); }
    virtual void performExportHead(P_File& pf, int nField);
    virtual void performExport(P_File& pf, LPCTSTR source, int nField);
    virtual void performExportRange(P_File& pf, P_File& pfs, int nField, int from,  int to);
};
//----------------------------------------------------------------------------
class export_sql : public export_
{
  private:
    typedef export_ baseClass;
  public:
    export_sql(P_ModListFiles* owner, const exportInfo& eI, P_File* curr, int currCol) : baseClass(owner, eI, curr, currCol) {}
  protected:
    virtual LPCTSTR getExt() const { return _T(".sql"); }
    virtual void performExportHead(P_File& pf, int nField);
    virtual void performExport(P_File& pf, LPCTSTR source, int nField);
    virtual void performExportRange(P_File& pf, P_File& pfs, int nField, int from,  int to);
};
//----------------------------------------------------------------------------
void P_ModListFiles::fillSelected(PVect<int>& range)
{
  range.reset();
  getRangeSel(*List, range);
}
//----------------------------------------------------------------------------
void P_ModListFiles::exportTab(const exportInfo& eI)
{
  export_tab eTab(this, eI, pF, getCurrNumCol());
  eTab.run();
}
//----------------------------------------------------------------------------
void P_ModListFiles::exportSql(const exportInfo& eI)
{
  export_sql eSql(this, eI, pF, getCurrNumCol());
  eSql.run();
}
//----------------------------------------------------------------------------
void P_ModListFiles::exportRec(const exportInfo& eI)
{
  switch(eI.type & 1) {
    case 0:
      exportTab(eI);
      break;
    case 1:
      exportSql(eI);
      break;
    }
}
//----------------------------------------------------------------------------
void export_::msgNoOpen(LPCTSTR filename, bool create)
{
  TCHAR msg[_MAX_PATH * 2];
  if(create)
    wsprintf(msg, getStringOrDef(IDT_EXP_ERR_NO_CREATE, _T("Impossibile creare il file\r\n%s.%s")), filename, getExt());
  else
    wsprintf(msg, getStringOrDef(IDT_EXP_ERR_NO_OPEN, _T("Impossibile aprire il file\r\n%s")), filename);
  MessageBox(*Owner, msg, getStringOrDef(IDT_ERROR, _T("Errore")), MB_OK | MB_ICONSTOP);
}
//----------------------------------------------------------------------------
bool export_::run()
{
  P_File* pf = 0;
  if(eI.type & 2) { // multiple
    int nElem = eI.fileSet.getElem();
    pf = createFileExp(eI.fileSet[0], getExt());
    if(!pf) {
      msgNoOpen(eI.fileSet[0], true);
      return false;
      }
    performExportHead(*pf, eI.fieldNumSet[0]);
    if(eI.type & 4) { // unica destinazione
      for(int i = 0; i < nElem; ++i)
        performExport(*pf, eI.fileSet[i], eI.fieldNumSet[i]);
      }
    else {
      performExport(*pf, eI.fileSet[0], eI.fieldNumSet[0]);
      for(int i = 1; i < nElem; ++i) {
        delete pf;
        pf = createFileExp(eI.fileSet[i], getExt());
        if(!pf) {
          msgNoOpen(eI.fileSet[i], true);
          return false;
          }
        performExportHead(*pf, eI.fieldNumSet[i]);
        performExport(*pf, eI.fileSet[i], eI.fieldNumSet[i]);
        }
      }
    }
  else {
    pf = createFileExp(Curr->get_name(),getExt());
    if(!pf) {
      msgNoOpen(Curr->get_name(), true);
      return false;
      }
    performExportHead(*pf, currCol);
    if(eI.type & 8) { // esporta le selezioni
      int nElem = eI.fieldNumSet.getElem() / 2;
      P_File pfs(Curr->get_name(), P_READ_ONLY);
      if(!pfs.P_open()) {
        msgNoOpen(Curr->get_name(), false);
        return false;
        }
      for(int i = 0, j = 0; i < nElem; ++i, j += 2)
        performExportRange(*pf, pfs, currCol, eI.fieldNumSet[j],  eI.fieldNumSet[j + 1]);
      }
    else
      performExport(*pf, Curr->get_name(), currCol);
    }
  delete pf;
  return true;
}
//----------------------------------------------------------------------------
#define DIM_BUFF_EXP (4096 * 4)
//----------------------------------------------------------------------------
void export_tab::performExportHead(P_File& pf, int nField)
{
  TCHAR dt[] = _T("Date\tHour");
  pf.P_writeString(dt);
  TCHAR head[256 + 4] = _T("\t");
  for(int i = 0; i < nField; ++i) {
    Owner->getHeaderText(head + 1, 256, i);
    pf.P_writeString(head);
    }
  pf.P_writeString(_T("\r\n"));
}
//----------------------------------------------------------------------------
void export_tab::performExport(P_File& pf, LPCTSTR source, int nField)
{
  P_File pfs(source, P_READ_ONLY);
  if(!pfs.P_open()) {
    msgNoOpen(source, false);
    return;
    }
  int szRec = SIZE_REC(nField);
  int nRow = (int)(pfs.get_len() / szRec);
  performExportRange(pf, pfs, nField, 0, nRow - 1);
}
//----------------------------------------------------------------------------
void export_tab::performExportRange(P_File& pf, P_File& pfs, int nField, int from,  int to)
{
  int szRec = SIZE_REC(nField);
  P_SmartPointer<LPBYTE> rec(new BYTE[szRec], true);
  P_SmartPointer<LPTSTR> buff(new TCHAR[DIM_BUFF_EXP], true);
  TCHAR sep = getDecSep();
  pfs.P_seek(from * szRec);
  for(; from <= to; ++from) {
    pfs.P_read(rec, szRec);
    LPBYTE rec2 = rec;
    set_format_data(buff, DIM_BUFF_EXP, *(FILETIME*)rec2, _T("\t"));
    rec2 += sizeof FILETIME;
    LPTSTR p = buff + _tcslen(buff);
    LPTSTR p2 = p;
    for(int i = 0; i < nField; ++i) {
      _stprintf_s(p, DIM_BUFF_EXP - (p - buff), _T("\t%f"), *(float*)rec2);
      rec2 += sizeof(float);
      p += _tcslen(p);
      }
    fromPointToSep(p2, sep);
    _tcscat_s(p, DIM_BUFF_EXP - (p - buff), _T("\r\n"));
    pf.P_writeString(buff);
    }
}
//----------------------------------------------------------------------------
#define MAX_NAME_HEADER 18
//----------------------------------------------------------------------------
static void makeSafeHeaderText(LPSTR target, uint sz)
{
  sz = min(sz, 512);
  char tmp[512 + 1];
  copyStrZ(tmp, target, sz);
  int init = 0;
  if(isdigit(*tmp)) {
    *target = '_';
    ++init;
    --sz;
    }
  for(uint i = 0; i < sz && tmp[i]; ++i, ++init) {
    if(isalnum(tmp[i]))
      target[init] = tolower(tmp[i]);
    else
      target[init] = '_';
    }
  target[init] = 0;
}
//----------------------------------------------------------------------------
#define SINGLE_INSERT
//----------------------------------------------------------------------------
void export_sql::performExportHead(P_File& pf, int nField)
{
  pf.P_writeToAnsi("-- fdate->yyyymmdd\r\n");
  pf.P_writeToAnsi("-- ftime->hhmmssmmm\r\n");
  pf.P_writeToAnsi("-- DROP TABLE IF EXISTS svTrend;\r\n");
  pf.P_writeToAnsi("CREATE TABLE svTrend (\r\n   fdate char(8),\r\n   ftime char(9)");

  char buff[256];
  TCHAR head[256 + 4];
  char headA[256 + 4];
  for(int i = 0; i < nField; ++i) {
    Owner->getHeaderText(head, 256, i);
    int len = _tcslen(head);
    if(_istdigit(*head)) {
      headA[0] = '_';
      copyStrZ(headA + 1, head);
      ++len;
      }
    else
      copyStrZ(headA, head);
    if(len > MAX_NAME_HEADER) {
      pf.P_writeToAnsi(",\r\n-- ");
      pf.P_writeToAnsi(headA);
      makeSafeHeaderText(headA, 256);

      if(nField < 10)
        headA[MAX_NAME_HEADER - 1] = '1' + i;
      else {
        headA[MAX_NAME_HEADER - 2] = '0' + (i + 1) / 10;
        headA[MAX_NAME_HEADER - 1] = '0' + (i + 1) % 10;
        }
      headA[MAX_NAME_HEADER] = 0;
      }
    else
      makeSafeHeaderText(headA, 256);
    wsprintfA(buff, ",\r\n   %s float", headA);
    pf.P_writeToAnsi(buff);
    }
  pf.P_writeToAnsi("\r\n);\r\n\r\n");
#ifndef SINGLE_INSERT
  pf.P_writeToAnsi("INSERT INTO svTrend VALUES\r\n");
#endif
}
//----------------------------------------------------------------------------
static void set_format_data_sql(LPSTR buff, FILETIME& ft)
{
  SYSTEMTIME stm;
  FileTimeToSystemTime(&ft, &stm);
  wsprintfA(buff, "\'%04d%02d%02d\', \'%02d%02d%02d%03d\'",
              stm.wYear, stm.wMonth, stm.wDay,
              stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);
}
//----------------------------------------------------------------------------
void export_sql::performExport(P_File& pf, LPCTSTR source, int nField)
{
/*
  ('20110323', '1234567', 12.345, 23.456, 34.567),
*/
  P_File pfs(source, P_READ_ONLY);
  if(!pfs.P_open()) {
    msgNoOpen(source, false);
    return;
    }
  int szRec = SIZE_REC(nField);
  int nRow = (int)(pfs.get_len() / szRec);
  performExportRange(pf, pfs, nField, 0,  nRow - 1);
}
//----------------------------------------------------------------------------
void export_sql::performExportRange(P_File& pf, P_File& pfs, int nField, int from,  int to)
{
  int szRec = SIZE_REC(nField);
  P_SmartPointer<LPBYTE> rec(new BYTE[szRec], true);
  P_SmartPointer<LPSTR> buff(new char[DIM_BUFF_EXP], true);
  pfs.P_seek(from * szRec);
  for(; from <= to; ++from) {
    pfs.P_read(rec, szRec);
    buff[0] = '(';
    LPBYTE rec2 = rec;
    set_format_data_sql(buff + 1, *(FILETIME*)rec2);
    rec2 += sizeof FILETIME;
    LPSTR p = buff + strlen(buff);
    for(int i = 0; i < nField; ++i) {
      sprintf_s(p,  DIM_BUFF_EXP - (p - buff), ", \'%0.3f\'", *(float*)rec2);
      rec2 += sizeof(float);
      p += strlen(p);
      }
#ifndef SINGLE_INSERT
    if(from < to)
      strcat_s(buff, DIM_BUFF_EXP, "),\r\n");
    else
      strcat_s(buff, DIM_BUFF_EXP, ");\r\n");
#else
    pf.P_writeToAnsi("INSERT INTO svTrend VALUES\r\n");
    strcat_s(buff, DIM_BUFF_EXP, ");\r\n");
#endif
    pf.P_writeToAnsi(buff);
    }
}
