//-------- PShowValueWin.cpp --------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "PShowValueWin.h"
#include "p_Util.h"
#include "PSliderWin.h"
#include "p_DefBody.h"
//-------------------------------------------------------------------
#define IDC_LIST1 4321
#define DEF_FILE_SAVE_COORD _T("trendCoord.dat")
//-----------------------------------------------------------
#define preProcessMsgT(msg) do { \
  if(WM_KEYDOWN == msg.message) { \
    switch(msg.wParam) { \
      case VK_F1: \
      case VK_ESCAPE: \
        DestroyWindow(*getParentWin<PSliderWin>(this)); \
        break; \
      } \
    } \
  if(!toBool(IsDialogMessage(getHandle(), &msg))) \
    return baseClass::preProcessMsg(msg); \
  return true; \
} while(false)
//-----------------------------------------------------------
class myListViewShow : public PListView
{
  private:
    typedef PListView baseClass;
  public:
    myListViewShow(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, r, text, hinst) {}

    ~myListViewShow() { destroy(); }

  protected:
    bool preProcessMsg(MSG& msg) { preProcessMsgT(msg); }
};
//-------------------------------------------------------------------
PShowValueWin::~PShowValueWin()
{
  destroy();
  release(&Left);
  release(&Right);
}
//-------------------------------------------------------------------
struct infoColumn
{
  LPCTSTR title;
  int sz;
};
//-------------------------------------------------------------------
static
infoColumn titleList[] = {
  { _T("."), 150 },
  { _T("."), 100 },
  { _T("."), 100 },
  { _T("ð"), 100 },
};
//-------------------------------------------------------------------
struct infoSaveCoord
{
  int x;
  int y;
  int w;
  int h;
  int wc[SIZE_A(titleList)];
};
//------------------------------------------------------------------
class myManageRowOfBuff : public manageRowOfBuff
{
  private:
    typedef manageRowOfBuff baseClass;
  public:
    myManageRowOfBuff(LPCTSTR filename, PVect<LPCTSTR>& rows) :
      baseClass(filename), Rows(rows) {}
  protected:
    virtual bool manageLine(LPCTSTR row) { Rows[Rows.getElem()] = str_newdup(row); return true; }
  private:
    PVect<LPCTSTR>& Rows;
};
//-------------------------------------------------------------------
#define TOT_COLUMN SIZE_A(titleList)
#define LV_TXT_COLOR RGB(0, 0, 0xb3)
#define LV_BKG_COLOR RGB(0xf8, 0xe4, 0xb3)
//-------------------------------------------------------------------
static uint getNumBit(DWORD bit, uint maxV)
{
  uint n = 0;
  for(uint i = 0; i < maxV; ++i, bit >>= 1)
    n += bit & 1;
  return n;
}
//-------------------------------------------------------------------
static LPCTSTR getInt(LPCTSTR p, int& v)
{
  if(p) {
    v = _ttoi(p);
    return findNextParamTrim(p);
    }
  return 0;
}
//-------------------------------------------------------------------
static bool loadCoord(infoSaveCoord& isc, const PVect<LPCTSTR>& Rows, LPCTSTR pagename, LPCTSTR trendName)
{
  TCHAR buff[1024];
  wsprintf(buff, _T("%s,%s"), pagename, trendName);
  uint len = _tcslen(buff);
  uint nElem = Rows.getElem();
  if(nElem) {
    for(uint i = 0; i < nElem; ++i) {
      if(!cmpStr(buff, Rows[i], len)) {
        LPCTSTR p = Rows[i] + len + 1;
        p = getInt(p, isc.x);
        p = getInt(p, isc.y);
        p = getInt(p, isc.w);
        p = getInt(p, isc.h);
        p = getInt(p, isc.wc[0]);
        p = getInt(p, isc.wc[1]);
        p = getInt(p, isc.wc[2]);
        p = getInt(p, isc.wc[3]);
        return true;
        }
      }
    }
  return false;
}
//-------------------------------------------------------------------
bool PShowValueWin::create()
{
  List = new myListViewShow(this, IDC_LIST1, PRect(0, 0, 100, 100));

  double t = 1;
  PVect<LPCTSTR> Rows;
  myManageRowOfBuff(DEF_FILE_SAVE_COORD, Rows).run();
  infoSaveCoord isc;
  bool useSaved = false;
  do {
    P_DefBody* bd = getParentWin<P_DefBody>(this);
    if(bd) {
      LPCTSTR pagename = bd->getPageName();
      TCHAR t[32];
      if(!pagename) {
        wsprintf(t, _T("%d"), bd->Attr.id);
        pagename = t;
        }
      useSaved = loadCoord(isc, Rows, pagename, iTrend.trendName);
      }
    } while(false);

  flushPV(Rows);
  if(useSaved) {
    Attr.x = isc.x;
    Attr.y = isc.y;
    Attr.w = isc.w;
    Attr.h = isc.h;
    }
  else {
    t = iTrend.defStr.cx;
    t /= 87;

    Attr.w = (int)(t * 450) + (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXBORDER)) * 2;
    uint nShow = getNumBit(bitShow, varName.getElem());

    Attr.h = nShow * (GetSystemMetrics(SM_CYBORDER) + iTrend.defStr.cy)
      + GetSystemMetrics(SM_CYSMCAPTION) * 2
      + (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYBORDER)) * 2;
    }

  if(!baseClass::create())
    return false;
  List->setFont(iTrend.hFont, false);

  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

  if(useSaved) {
    for(int i = 0; i < TOT_COLUMN; ++i) {
      lvc.iSubItem = i;
      lvc.pszText = (LPTSTR)titleList[i].title;
      lvc.cx = isc.wc[i];
      lvc.fmt = LVCFMT_CENTER;
      if(-1 == ListView_InsertColumn(*List, i, &lvc))
        return false;
      }
    }
  else {
    for(int i = 0; i < TOT_COLUMN; ++i) {
      lvc.iSubItem = i;
      lvc.pszText = (LPTSTR)titleList[i].title;
      lvc.cx = (int)(titleList[i].sz * t);
      lvc.fmt = LVCFMT_CENTER;
      if(-1 == ListView_InsertColumn(*List, i, &lvc))
        return false;
      }
    }
  DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  ListView_SetExtendedListViewStyle(*List, dwExStyle);

  ListView_SetBkColor(*List, LV_BKG_COLOR);
  ListView_SetTextBkColor(*List, LV_BKG_COLOR);

  ListView_SetTextColor(*List, LV_TXT_COLOR);

  uint nElem = varName.getElem();
  DWORD bit = bitShow;
  TCHAR tBuff[_MAX_PATH];
  for(uint i = 0; i < nElem; ++i, bit >>= 1) {
    if(bit & 1) {
      LVITEM lvi;
      ZeroMemory(&lvi, sizeof(lvi));
      lvi.mask = LVIF_TEXT;
      lvi.iItem = 65535;
      lvi.iSubItem = 0;
      _tcscpy_s(tBuff, varName[i].name);
      lvi.pszText = tBuff;
      ListView_InsertItem(*List, &lvi);
      }
    }
  resize();
  return true;
}
//-----------------------------------------------------------
static void getWC(HWND hwnd, int* wc)
{
  HWND lw = GetDlgItem(hwnd, IDC_LIST1);
  for(uint i = 0; i < SIZE_A(titleList); ++i)
    wc[i] = ListView_GetColumnWidth(lw, i);
}
//-----------------------------------------------------------
static void save_Coord(HWND hwnd, LPCTSTR pagename, LPCTSTR trendName)
{
  PVect<LPCTSTR> Rows;
  TCHAR buff[1024];
  wsprintf(buff, _T("%s,%s"), pagename, trendName);
  uint len = _tcslen(buff);
  LPTSTR p = buff + len;
  int wc[SIZE_A(titleList)];
  getWC(hwnd, wc);

  PRect r;
  GetWindowRect(hwnd, r);
  wsprintf(p, _T(",%d,%d,%d,%d,%d,%d,%d,%d"), r.left, r.top, r.Width(), r.Height(),
    wc[0], wc[1], wc[2], wc[3]);
  *p = 0;
  do {
    myManageRowOfBuff(DEF_FILE_SAVE_COORD, Rows).run();
    } while(false);
  uint nElem = Rows.getElem();
  uint found = nElem;
  if(nElem) {
    for(uint i = 0; i < nElem; ++i) {
      if(!cmpStr(buff, Rows[i], len)) {
        found = i;
        break;
        }
      }
    if(found < nElem)
      delete []Rows[found];
    }
  *p = _T(',');
  Rows[found] = str_newdup(buff);

  P_File pf(DEF_FILE_SAVE_COORD, P_CREAT);
  if(pf.P_open()) {
    nElem = Rows.getElem();
    for(uint i = 0; i < nElem; ++i) {
      pf.P_writeString(Rows[i]);
      pf.P_writeString(_T("\r\n"));
      }
    }
  flushPV(Rows);
}
//-------------------------------------------------------------------
LRESULT PShowValueWin::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      do {
        P_DefBody* bd = getParentWin<P_DefBody>(this);
        if(bd) {
          LPCTSTR pagename = bd->getPageName();
          TCHAR t[32];
          if(!pagename) {
            wsprintf(t, _T("%d"), bd->Attr.id);
            pagename = t;
            }
          save_Coord(hwnd, pagename, iTrend.trendName);
          }
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
bool PShowValueWin::preProcessMsg(MSG& msg)
{
  preProcessMsgT(msg);
}
//-------------------------------------------------------------------
void PShowValueWin::release(PacketInfo** packet)
{
  freeListPacket& manPack = getFreePacket();
  if(*packet)
    manPack.release(*packet);
  *packet = 0;
}
//-------------------------------------------------------------------
void PShowValueWin::setInfo(PacketInfo* packet)
{
  uint subItem = packet->Side + 1;
  if(PacketInfo::epi_left == packet->Side) {
    release(&Left);
    Left = packet;
    }
  else {
    release(&Right);
    Right = packet;
    }

  LVCOLUMN lvc;
  lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
  lvc.iSubItem = subItem;
  TCHAR tmp[200];
  set_format_data(tmp, SIZE_A(tmp), packet->Ft, whichData(), _T("|"));
  lvc.pszText = tmp + 11;
  ListView_SetColumn(*List, subItem, &lvc);

  if(PacketInfo::epi_left == packet->Side) {
    tmp[10] = 0;
    lvc.iSubItem = 0;
    lvc.pszText = tmp;
    ListView_SetColumn(*List, 0, &lvc);
    }

  uint nElem = varName.getElem();
  DWORD bit = bitShow;
  for(uint i = 0, j = 0; i < nElem; ++i, bit >>= 1) {
    if(bit & 1) {
      makeFixedString(packet->Value[i], tmp, SIZE_A(tmp), varName[i].nDec);
      ListView_SetItemText(*List, j, subItem, tmp);
      if(Left && Right) {
        makeFixedString(Right->Value[i] - Left->Value[i], tmp, SIZE_A(tmp), varName[i].nDec);
        ListView_SetItemText(*List, j, SIZE_A(titleList) - 1, tmp);
        }
      ++j;
      }
    }
}
//-------------------------------------------------------------------
