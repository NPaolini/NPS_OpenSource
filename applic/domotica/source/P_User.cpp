//---------- P_User.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_User.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "base64.h"
#include "p_param.h"
#include "p_param_v.h"
#include "p_util.h"
//----------------------------------------------------------------------------
class myLV : public PCustomListViewFullEdit
{
  private:
    typedef PCustomListViewFullEdit baseClass;
  public:
    myLV(PWin * parent, uint resid, HINSTANCE hinst = 0) :
         baseClass(parent, resid, hinst)   {  }
  protected:
    virtual bool canEditItem(int item, int subItem) const { return item >= 0 && subItem > 0; }
    virtual void personalizeEdit(int item, int subItem, LPTSTR txt, size_t dim);
    virtual bool personalizeEditEnd(int item, int subItem, LPTSTR txt, size_t dim);
};
//----------------------------------------------------------------------------
void myLV::personalizeEdit(int item, int subItem, LPTSTR txt, size_t dim)
{
  if(3 != subItem || !txt)
    return;
  int len = _tcslen(txt) - DIM_VER;
  if(len <= 0)
    return;
  TCHAR t[1024];
  copyStrZ(t, txt);
  int i = DIM_HEADER;
  for(; i < len; ++i)
    if(_T('0') != t[i])
      break;
  len -= i;
  copyStr(txt, t + i, len);
  txt[len] = 0;
}
//----------------------------------------------------------------------------
bool myLV::personalizeEditEnd(int item, int subItem, LPTSTR txt, size_t dim)
{
  if(3 != subItem)
    return false;
  int len = _tcslen(txt);
  if(len <= 0)
    return false;
  setting s;
  fillSetting(s);
  int i = DIM_CODE - len;
  for(int j = 0; i < DIM_CODE; ++i, ++j)
    s.Code[DIM_HEADER + i] = txt[j];
  copyStrZ(txt, s.Code);
  return true;
}
//----------------------------------------------------------------------------
P_User::P_User(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(conn, parent, resId, hinstance)
{
  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_SEND, IDB_BITMAP_DONE };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDOK, Bmp[0]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[1]);
}
//----------------------------------------------------------------------------
P_User::~P_User()
{
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
void P_User::loadCurr()
{
  ListView_DeleteAllItems(*lv);
  char buff[4096] = "cmd=getuser:#";
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
  buff[readed] = 0;
  parseBuff(buff);
}
//----------------------------------------------------------------------------
#define MAX_USER 10
#define MAX_COLUMN 4
//----------------------------------------------------------------------------
void P_User::appendText(LPSTR buff, size_t dim, uint ix)
{
  TCHAR t[265];
  LPSTR p = buff + strlen(buff);
  wsprintfA(p, "%d,", ix);
  p += strlen(p);

  LVITEM item;
  ZeroMemory(&item, sizeof(item));
  item.mask = LVIF_TEXT;
  item.cchTextMax = SIZE_A(t) - 1;
  item.iItem = ix;
  for(int i = 1; i < MAX_COLUMN; ++i) {
    item.iSubItem = i;
    item.pszText = t;
    ListView_GetItem(*lv, &item);
    if(!*item.pszText) {
      *p++ = ' ';
      *p++ = ',';
      *p = 0;
      }
    else {
      int sz = dim - (p - buff) - 1;
      copyStrZ(p, item.pszText, sz);
      strcat_s(p, sz, ",");
      p += strlen(p);
      }
    }
}
//----------------------------------------------------------------------------
static LPCTSTR MSG_SHOW_SEND_USER = _T("Sending Users ...\n\rbyte %d of %d");
#define BARCOLOR_CHAR RGB(0x3f, 0xff, 0x10)
//----------------------------------------------------------------------------
void P_User::BNClickedOk()
{
  char buff[4096] = "cmd=user:";
  for(uint i = 0; i < MAX_USER; ++i)
    appendText(buff, SIZE_A(buff), i);
  int len = strlen(buff);
  buff[len] = '#';
  buff[len + 1] = 0;

  progressBar bar(this, MSG_SHOW_SEND_USER, len, BARCOLOR_CHAR);
  bar.create();
  UpdateWindow(bar);
  connSend cs(Conn, &bar);
  cs.send(buff, strlen(buff));
  Sleep(2000);
  loadCurr();
}
//----------------------------------------------------------------------------
bool P_User::create()
{
  lv = new myLV(this, IDC_LIST_USER);
  if(!baseClass::create())
    return false;

  struct infoHeader { int sz; LPCTSTR text; };
  infoHeader ie[] = {
    { 50, _T("Index") },
    { 100, _T("Username") },
    { 100, _T("Password") },
    { 150, _T("RFID Code") }
    };
  LVCOLUMN lvc;
  lvc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT;
  lvc.fmt = LVCFMT_CENTER;
  for(uint i = 0; i < SIZE_A(ie); ++i) {
    lvc.pszText = (LPTSTR)ie[i].text;
    lvc.cx = ie[i].sz;
    if(-1 == ListView_InsertColumn(*lv, i, &lvc))
      return false;
    }

  Conn->passThrough(cReset, 0);
  loadCurr();
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_User::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          if(lv->getEditHandle() == HWND(lParam))
            return 1;
          BNClickedOk();
          break;
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        }
    break;
    case WM_NOTIFY:
      if(((NMHDR*)lParam)->idFrom == IDC_LIST_USER) {
        LRESULT result;
        if(lv->ProcessNotify(lParam, result)) {
          SetWindowLongPtr(*this, DWLP_MSGRESULT, result);
          return result;
          }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MATCH_USER "r=getuser:"
#define DIM_MATCH_USER (strlen(MATCH_USER))
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void P_User::parseRow(LPCSTR* v)
{
  LVITEM lvi;
  ZeroMemory(&lvi, sizeof(lvi));
  lvi.mask = LVIF_TEXT;
  bool empty = !*v[0];
  if(empty)
    lvi.iItem = ListView_GetItemCount(*lv);
  else
    lvi.iItem = atoi(v[0]);
  TCHAR t[256];
  _itot_s(lvi.iItem + 1, t, 10);
  lvi.pszText = t;
  int pos = ListView_InsertItem(*lv, &lvi);
  if(-1 == pos)
    return;

  lvi.iItem = pos;

  for(uint i = 1; i < MAX_COLUMN; ++i) {
    lvi.iSubItem = i;
    copyStrZ(t, v[i]);
    lvi.pszText = t;
    ListView_SetItem(*lv, &lvi);
    }
}
//----------------------------------------------------------------------------
void P_User::addEmptyRow(int ix)
{
  LVITEM lvi;
  ZeroMemory(&lvi, sizeof(lvi));
  lvi.mask = LVIF_TEXT;
  lvi.iItem = ix;
  TCHAR t[256];
  _itot_s(lvi.iItem + 1, t, 10);
  lvi.pszText = t;
  ListView_InsertItem(*lv, &lvi);
}
//----------------------------------------------------------------------------
void P_User::fillEmpty()
{
  int curr = ListView_GetItemCount(*lv);
  for (uint i = curr; i < MAX_USER; ++i)
  addEmptyRow(i);
}
//----------------------------------------------------------------------------
void P_User::parseBuff(LPCSTR buff)
{
  int pos = checkPos(buff, MATCH_USER, DIM_MATCH_USER);
  if (pos < 0) {
    fillEmpty();
    return;
    }
  pvvCharA target;
  uint n = splitParamA(target, buff + pos);
  if (n % 4) {
    fillEmpty();
    return;
    }
  for (uint i = 0; i < n; i += 4) {
    LPCSTR p[] = { &target[i], &target[i + 1], &target[i + 2], &target[i + 3] };
    parseRow(p);
  }
  fillEmpty();

  int curr = ListView_GetItemCount(*lv);
  for (uint i = curr; i < MAX_USER; ++i)
    addEmptyRow(i);

  ListView_SetItemState(*lv, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
  ListView_EnsureVisible(*lv, 0, TRUE);
}
//----------------------------------------------------------------------------
