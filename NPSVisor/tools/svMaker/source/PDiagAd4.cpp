//----------- PDiagAd4.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "common.h"
//----------------------------------------------------------------------------
#include "PDiagAd4.h"
#include "pOwnBtnImageStd.h"
#include "pBitmap.h"
//-----------------------------------------------------------------
#define ad7CRYPT_HEAD "CRYPNPS"
#define ad7DIM_HEAD sizeof(CRYPT_HEAD)
//-----------------------------------------------------------------
#define ad7CRYPT_STEP     1
#define ad7DIM_KEY_CRYPT  8
//-----------------------------------------------------------------
DWORD load_CriptedTxt(LPCBYTE key, setOfString& set, LPCTSTR name);
//----------------------------------------------------------------------------
#define MAX_TEXT 255
//----------------------------------------------------------------------------
#define IP_TO_DWORD(b1, b2, b3, b4, dw) \
  ((dw) = ((DWORD)(b1) << 24) | ((DWORD)(b2) << 16) | ((DWORD)(b3) << 8) | (b4))
//----------------------------------------------------------------------------
#define IP_TO_DWORD_RET(b1, b2, b3, b4) \
  (((DWORD)(b1) << 24) | ((DWORD)(b2) << 16) | ((DWORD)(b3) << 8) | (b4))
//----------------------------------------------------------------------------
#define DWORD_TO_IP(b1, b2, b3, b4, dw) \
    { (b1) = (dw) >> 24; \
       (b2) = ((dw) >> 16) & 0xff; \
       (b3) = ((dw) >> 8) & 0xff;  \
       (b4) = (dw) & 0xff; }
//----------------------------------------------------------------------------
#define IP1(dw) ((dw) >> 24)
#define IP2(dw) (((dw) >> 16) & 0xff)
#define IP3(dw) (((dw) >> 8) & 0xff)
#define IP4(dw) ((dw) & 0xff)
//----------------------------------------------------------------------------
static
void makeAd4Filename(LPTSTR fname, uint prph, bool addSharp = true)
{
  dataProject& dp = getDataProject();
  _tcscpy_s(fname, _MAX_PATH, dp.newPath);
  LPTSTR p = fname + _tcslen(fname);
  if(addSharp)
    wsprintf(p, _T("\\#prph_%d_data.ad7"), prph);
  else
    wsprintf(p, _T("\\prph_%d_data.ad7"), prph);
}
//----------------------------------------------------------------------------
static
P_File* makeAd4File(uint prph)
{
  TCHAR path[_MAX_PATH];
  do { // prova a vedere se il file è bloccato dal programma prph_addr
    makeAd4Filename(path, prph);
    P_File pf(path);
    if(!pf.P_open())
      return 0;
    } while(false);

  makeAd4Filename(path, prph, false);
  P_File* pf = new P_File(path, P_CREAT);
  if(!pf->P_open()) {
    delete pf;
    return 0;
    }
  return pf;
}
//----------------------------------------------------------------------------
static
void makeAd4Set(setOfString& set, uint prph)
{
  TCHAR path[_MAX_PATH];
  makeAd4Filename(path, prph);
  BYTE key[ad7DIM_KEY_CRYPT + 2];
  wsprintfA((LPSTR)key, "p%s_%02d", "PRPH", prph);
  load_CriptedTxt(key, set, path);
}
//----------------------------------------------------------------------------
struct ad4Row
{
  DWORD db;
  DWORD addr;
  DWORD type;
  DWORD action;
  DWORD ipAddr;
  DWORD port;
  double vMin;
  double vMax;
  double vMinP;
  double vMaxP;
  TCHAR text[MAX_TEXT];
};
//----------------------------------------------------------------------------
struct infoHead
{
  LPCTSTR text;
  uint cx;
  uint align; // LVCFMT_CENTER, LVCFMT_LEFT, LVCFMT_RIGHT
};
//----------------------------------------------------------------------------
static infoHead head[] = {
    { _T("Tipo oggetto"), 100, LVCFMT_LEFT },
    { _T("Variabile"), 170, LVCFMT_CENTER },
    { _T("Utilizzo"), 60, LVCFMT_CENTER },
    { _T("IP"), 60, LVCFMT_CENTER },
    { _T("Porta"), 40, LVCFMT_CENTER },
    { _T("DB"), 40, LVCFMT_CENTER },
    { _T("Indirizzo"), 60, LVCFMT_CENTER },
    { _T("Tipo"), 40, LVCFMT_CENTER },
    { _T("Azione"), 40, LVCFMT_CENTER },
    { _T("Min,Max (sVisor)"), 60, LVCFMT_CENTER },
    { _T("Min,Max (Perif)"), 60, LVCFMT_CENTER },
    { _T("Descrizione"), 300, LVCFMT_LEFT },
    };
//----------------------------------------------------------------------------
#define MAX_SET 20
#define MAX_COLUMN (SIZE_A(head))
//----------------------------------------------------------------------------
#define INIT_COLUMN_ADR 3
//----------------------------------------------------------------------------
#define MAX_COLUMN_ADR 11
//----------------------------------------------------------------------------
#define BKG_COLOR RGB(0xF0, 0xFF, 0xFF)
#define TXT_COLOR RGB(0x00, 0x00, 0xC0)
//----------------------------------------------------------------------------
class myLV_Edit : public PCustomListViewFullEdit
{
  private:
    typedef PCustomListViewFullEdit baseClass;
  public:
    myLV_Edit(PWin * parent, uint resid, HINSTANCE hinst = 0) :
         baseClass(parent, resid, hinst)   {  }
    virtual bool getColorBkg(COLORREF& c) { c = BKG_COLOR; return true; }
    virtual bool getColorTextBkg(COLORREF& c) { c = BKG_COLOR; return true; }
    virtual bool getColorText(COLORREF& c) {  c = TXT_COLOR; return true; }
  protected:
    virtual bool canEditItem(int item, int subItem) const { return subItem > 2; }
    virtual int verifyKey() { int res = manageKey::verifyKey(); return res ? res : baseClass::verifyKey(); }
    virtual bool evKeyDown(UINT& key) { return manageKey::evKeyDown(key); }
    virtual bool evKeyUp(UINT& key) { return manageKey::evKeyUp(key); }
    virtual bool evChar(UINT& key) { return manageKey::evChar(key); }

};
//----------------------------------------------------------------------------
class myBtn : public POwnBtnImageStd
{
  private:
    typedef POwnBtnImageStd baseClass;
  public:
    myBtn(PWin * parent, uint resid, PBitmap* image,
          uint fPos = wLeft, bool autoDelete = false, HINSTANCE hinst = 0) :
         baseClass(parent, resid, image, fPos, autoDelete, hinst)   {  }
  protected:
    virtual int verifyKey() { int res = manageKey::verifyKey(); return res ? res : baseClass::verifyKey(); }
    virtual bool evKeyDown(UINT& key) { return manageKey::evKeyDown(key); }
    virtual bool evKeyUp(UINT& key) { return manageKey::evKeyUp(key); }
    virtual bool evChar(UINT& key) { return manageKey::evChar(key); }
};
//----------------------------------------------------------------------------
PDiagAd4::PDiagAd4(PWin* parent, const PVect<infoAd4*>& set, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  Set(set)
{
  int idBmp[] = { IDB_BITMAP_OK, IDB_BITMAP_CANC };
  int idBtn[] = { IDOK, IDCANCEL };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new myBtn(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true);
    }
}
//----------------------------------------------------------------------------
PDiagAd4::~PDiagAd4()
{
  flushPV(oldValue);
}
//----------------------------------------------------------------------------
#define MIN_WIDTH  MinSize.cx
#define MIN_HEIGHT MinSize.cy
//----------------------------------------------------------------------------
extern void saveRegAd4(LPCTSTR buff);
extern void loadRegAd4(LPTSTR buff, size_t dim);
//----------------------------------------------------------------------------
bool PDiagAd4::create()
{
  lvE = new myLV_Edit(this, IDC_LISTVIEW_AD4);
  lvE->setFont(DEF_FONT(16), true);
  if(!baseClass::create())
    return false;
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));
  PRect minRect;
  GetWindowRect(*this, minRect);
  MinSize.cx = minRect.Width();
  MinSize.cy = minRect.Height();
  LPCTSTR p = 0;
  TCHAR buff[4096];
  loadRegAd4(buff, SIZE_A(buff));
  int maximized = 0;
  if(*buff) {
    int x;
    int y;
    int w;
    int h;
    _stscanf_s(buff, _T("%d,%d,%d,%d,%d"), &x, &y, &w, &h, &maximized);
    currRect = PRect(0, 0, w, h);
    currRect.MoveTo(x, y);
    SetWindowPos(*this, 0, x, y, w, h, SWP_NOZORDER);
    if(maximized)
      ShowWindow(*this, SW_MAXIMIZE);
    p = findNextParamTrim(buff, 5);
    }
  fillLV(p);
  if(!*buff)
    evSize(szResize);
  return true;
}
//----------------------------------------------------------------------------
void PDiagAd4::fillHeader(infoHead* set, uint nElem, LPCTSTR pDim)
{
  LVCOLUMN lvCol;
  for(uint i = 0; i < nElem; ++i) {
    lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    uint cx = set[i].cx;
    if(pDim) {
      cx = _ttoi(pDim);
      pDim = findNextParamTrim(pDim);
      }
    lvCol.cx = cx;
    lvCol.pszText = (LPTSTR)set[i].text;
    lvCol.fmt = set[i].align;
    ListView_InsertColumn(*lvE, i, &lvCol);
  }
}
//----------------------------------------------------------------------------
void PDiagAd4::addItem(PVect<LPTSTR>& set)
{
  int iLastIndex = ListView_GetItemCount(*lvE);

  LVITEM lvItem;
  lvItem.mask = LVIF_TEXT;
  lvItem.pszText = set[0];
  lvItem.iItem = iLastIndex;
  lvItem.iSubItem = 0;

  ListView_InsertItem(*lvE, &lvItem);

  int nElem = set.getElem();
  for(int i = 1; i < nElem; ++i)
    ListView_SetItemText(*lvE, iLastIndex, i, set[i]);
}
//----------------------------------------------------------------------------
void PDiagAd4::getItem(PVect<LPTSTR>& set, uint iItem)
{
  TCHAR buff[MAX_TEXT];
  for(uint i = INIT_COLUMN_ADR, j = 0; i < MAX_COLUMN; ++i, ++j) {
    buff[0] = 0;
    ListView_GetItemText(*lvE, iItem, i, buff, SIZE_A(buff));
    set[j] = *buff ? str_newdup(buff) : 0;
    }
}
//----------------------------------------------------------------------------
static uint addDword(uint ix, PVect<LPTSTR>& set, DWORD value)
{
  TCHAR buff[MAX_TEXT];
  wsprintf(buff, _T("%d"), value);
  set[ix] = str_newdup(buff);
  return ix + 1;
}
//----------------------------------------------------------------------------
static uint addReal(uint ix, PVect<LPTSTR>& set, double value1, double value2)
{
  TCHAR buff[MAX_TEXT];
  TCHAR t1[MAX_TEXT];
  TCHAR t2[MAX_TEXT];
  _stprintf_s(t1, _T("%f"), value1);
  _stprintf_s(t2, _T("%f"), value2);
  zeroTrim(t1);
  zeroTrim(t2);
  wsprintf(buff, _T("%s,%s"), t1, t2);
  set[ix] = str_newdup(buff);
  return ix + 1;
}
//----------------------------------------------------------------------------
static uint addIp(uint ix, PVect<LPTSTR>& set, DWORD value)
{
  TCHAR buff[MAX_TEXT];
  wsprintf(buff, _T("%d.%d.%d.%d"), IP1(value), IP2(value), IP3(value), IP4(value)) ;
  set[ix] = str_newdup(buff);
  return ix + 1;
}
//----------------------------------------------------------------------------
static void getStr(LPTSTR target, LPCTSTR source, uint len)
{
  if(source)
     copyStrZ(target, source, len);
  else
    target[0] = 0;
}
//----------------------------------------------------------------------------
static uint addText(uint ix, PVect<LPTSTR>& set, LPCTSTR txt, uint len)
{
  TCHAR buff[MAX_TEXT];
  len = min(len, MAX_TEXT - 1);
  getStr(buff, txt, len) ;
  set[ix] = str_newdup(buff);
  return ix + 1;
}
//----------------------------------------------------------------------------
void PDiagAd4::addAd4Info(PVect<LPTSTR>& set, ad4Row* row)
{
  if(!row)
    return;
  uint i = INIT_COLUMN_ADR;
  i = addIp(i, set, row->ipAddr);
  i = addDword(i, set, row->port);

  i = addDword(i, set, row->db);
  i = addDword(i, set, row->addr);
  i = addDword(i, set, row->type);
  i = addDword(i, set, row->action);

  i = addReal(i, set, row->vMin, row->vMax);
  i = addReal(i, set, row->vMinP, row->vMaxP);

  i = addText(i, set, row->text, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
static uint getInt(LPCTSTR p)
{
  if(!p)
    return 0;
  return _ttoi(p);
}
//----------------------------------------------------------------------------
static uint getIP(LPCTSTR p)
{
  if(!p)
    return 0;
  uint part[4] = { 0 };
  _stscanf_s(p, _T("%d.%d.%d.%d"), &part[0], &part[1], &part[2], &part[3]);
  return IP_TO_DWORD_RET(part[0], part[1], part[2], part[3]);
}
//----------------------------------------------------------------------------
static double getReal(LPCTSTR p)
{
  if(!p)
    return 0;
  return _tstof(p);
}
//----------------------------------------------------------------------------
void PDiagAd4::getAd4Info(const PVect<LPTSTR>& set, ad4Row& row)
{
   row.ipAddr = getIP(set[0]);
   row.port = getInt(set[1]);
   row.db = getInt(set[2]);
   row.addr = getInt(set[3]);
   row.type = getInt(set[4]);
   row.action = getInt(set[5]);
   LPCTSTR p = set[6];
   row.vMin = getReal(p);
   p = findNextParamTrim(p);
   row.vMax = getReal(p);
   p = set[7];
   row.vMinP = getReal(p);
   p = findNextParamTrim(p);
   row.vMaxP = getReal(p);
   getStr(row.text, set[8], MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
ad4Row* PDiagAd4::makeValue(LPCTSTR pRow)
{
  pvvChar target;
  uint nElem = splitParam(target, pRow);
  if(nElem < MAX_COLUMN_ADR)
    return 0;

  ad4Row* a4r = new ad4Row;
  ZeroMemory(a4r, sizeof(*a4r));

  a4r->db = getInt(&target[0]);
  a4r->addr = getInt(&target[1]);
  a4r->type = getInt(&target[2]);
  a4r->action = getInt(&target[3]);
  a4r->ipAddr = getInt(&target[4]);
  a4r->port = getInt(&target[5]);

  a4r->vMin = getReal(&target[6]);
  a4r->vMax = getReal(&target[7]);
  a4r->vMinP = getReal(&target[8]);
  a4r->vMaxP = getReal(&target[9]);
  getStr(a4r->text, &target[10], MAX_TEXT - 1);

  return a4r;
}
//----------------------------------------------------------------------------
void PDiagAd4::fillLV(LPCTSTR pDim)
{
  fillHeader(head, SIZE_A(head), pDim);

  setOfString* sSet[MAX_SET];
  ZeroMemory(sSet, sizeof(sSet));
  uint nElem = Set.getElem();
  oldValue.setDim(nElem);
  for(uint i = 0; i < nElem; ++i) {
    oldValue[i] = 0;
    uint prph = Set[i]->prph - 1;
    if(prph >= MAX_SET)
      continue;
    if(!sSet[prph]) {
      sSet[prph] = new setOfString;
      makeAd4Set(*sSet[prph], prph + 1);
      }
    TCHAR t1[512];
    PVect<LPTSTR> set;
    wsprintf(t1, _T("%s (Id %d)"), Set[i]->objName, Set[i]->id);
    set[0] = str_newdup(t1);
    wsprintf(t1, _T("Prph %d, Addr %d, Type %d, nBit %d, Offs %d"), Set[i]->prph, Set[i]->addr,
        Set[i]->type, Set[i]->nBit, Set[i]->offs);
    set[1] = str_newdup(t1);
    set[2] = str_newdup(Set[i]->infoVar);

    LPCTSTR p = sSet[prph]->getString(Set[i]->addr + 1);
    if(p) {
      oldValue[i] = makeValue(p);
      addAd4Info(set, oldValue[i]);
      }
    addItem(set);
    flushPAV(set);
    }
  for(uint i = 0; i < MAX_SET; ++i)
    delete sSet[i];
}
//----------------------------------------------------------------------------
void PDiagAd4::evSize(szAction act)
{
  if(szResize == act)
    GetWindowRect(*this, currRect);

  PRect rAll;
  GetClientRect(*this, rAll);
  PRect r;
  GetWindowRect(*lvE, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);

  r.right = rAll.Width() - r.left;
  r.bottom = rAll.Height() - r.left;
  SetWindowPos(*lvE, 0, 0, 0, r.Width(), r.Height(), SWP_NOMOVE | SWP_NOZORDER);
}
//----------------------------------------------------------------------------
void PDiagAd4::fillDim(LPTSTR t)
{
  t[0] = 0;
  LPTSTR p = t;
  for(uint i = 0; i < SIZE_A(head); ++i) {
    uint d = ListView_GetColumnWidth(*lvE, i);
    wsprintf(p, _T("%d,"), d);
    p += _tcslen(p);
    }
}
//----------------------------------------------------------------------------
void PDiagAd4::saveCurrCoords(HWND hwnd)
{
  if(lvE->getHandle()) {
    TCHAR buff[4096];
    WINDOWPLACEMENT wp;
    ZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    GetWindowPlacement(hwnd, &wp);
    int maximized = SW_MAXIMIZE == (wp.showCmd & SW_MAXIMIZE);
    wsprintf(buff, _T("%d,%d,%d,%d,%d,"), currRect.left, currRect.top, currRect.Width(), currRect.Height(), maximized);
    LPTSTR t = buff + _tcslen(buff);
    fillDim(t);
    saveRegAd4(buff);
    }
}
//----------------------------------------------------------------------------
LRESULT PDiagAd4::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      saveCurrCoords(hwnd);
      break;
    case WM_GETMINMAXINFO:
      if(MIN_WIDTH) {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
        lpmmi->ptMinTrackSize.x = MIN_WIDTH;
        lpmmi->ptMinTrackSize.y = MIN_HEIGHT;
        } while(false);
      break;

    case WM_SIZE:
      do {
        szAction act = szResize;
        if(SIZE_MAXIMIZED == wParam)
          act = szMaximized;
        evSize(act);
        } while(false);
      break;
    case WM_MOVE:
      currRect.MoveTo((short) LOWORD(lParam), (short) HIWORD(lParam));
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          if(lvE->getEditHandle() == HWND(lParam))
            return 1;
          break;
        }
      break;

    case WM_NOTIFY:
      if(((NMHDR*)lParam)->idFrom == IDC_LISTVIEW_AD4) {
        LRESULT result;
        if(lvE->ProcessNotify(lParam, result)) {
          SetWindowLongPtr(*this, DWLP_MSGRESULT, result);
          return result;
          }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
bool operator !=(const ad4Row& v1, const ad4Row& v2)
{
  int res = memcmp(&v1, &v2, (LPBYTE)&v1.text - (LPBYTE)&v1);
  if(res)
    return true;
  return toBool(_tcscmp(v1.text, v2.text));
}
//----------------------------------------------------------------------------
static LPTSTR makeRow(const ad4Row& ad4R)
{
  TCHAR buff[4096];
  _stprintf_s(buff, _T("%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%s"),
      ad4R.db,
      ad4R.addr,
      ad4R.type,
      ad4R.action,
      ad4R.ipAddr,
      ad4R.port,
      ad4R.vMin,
      ad4R.vMax,
      ad4R.vMinP,
      ad4R.vMaxP,
      ad4R.text);
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
static void saveSet(uint prph, P_File& pf, setOfString& set)
{
  if(!set.setFirst())
    return;
  do {
    set.writeCurrent(pf);
    } while(set.setNext());
  pf.P_close();
  TCHAR path[_MAX_PATH];
  makeAd4Filename(path, prph);

  BYTE key[ad7DIM_KEY_CRYPT + 2];
  wsprintfA((LPSTR)key, "p%s_%02d", "PRPH", prph);
  infoFileCrCopy ifcc;
  ifcc.key =       key;
  ifcc.dimKey =    ad7DIM_KEY_CRYPT;
  ifcc.step =      ad7CRYPT_STEP;
  ifcc.header =    (LPCBYTE)ad7CRYPT_HEAD;
  ifcc.lenHeader = ad7DIM_HEAD;
  if(cryptFileCopy(path, pf.get_name(), ifcc))
    DeleteFile(pf.get_name());
}
//----------------------------------------------------------------------------
#define MSG_NOT_CREATEFILE _T("Impossibile scrivere sul file della periferica %d,\r\nRiprovo?")
#define TITLE_NOT_CREATEFILE _T("File bloccato dal 'Prph_adr7_test'?")
//----------------------------------------------------------------------------
static DWORD msgNotOpen(PWin* owner, uint prph)
{
  TCHAR buff[2048];
  wsprintf(buff, MSG_NOT_CREATEFILE, prph);
  return MessageBox(*owner, buff, TITLE_NOT_CREATEFILE, MB_YESNOCANCEL | MB_ICONINFORMATION);
}
//----------------------------------------------------------------------------
void PDiagAd4::CmOk()
{
  uint nElem = oldValue.getElem();
  PVect<ad4Row*> tmp;
  tmp.setDim(nElem);

  setOfString* sSet[MAX_SET];
  ZeroMemory(sSet, sizeof(sSet));

  for(uint i = 0; i < nElem; ++i) {
    tmp[i] = 0;
    uint prph = Set[i]->prph - 1;
    if(prph >= MAX_SET)
      continue;
    PVect<LPTSTR> set;
    getItem(set, i);
    ad4Row ad4R;
    getAd4Info(set, ad4R);
    if(!oldValue[i] || ad4R != *oldValue[i]) {
      if(!sSet[prph]) {
        sSet[prph] = new setOfString;
        makeAd4Set(*sSet[prph], prph + 1);
        }
      tmp[i] = new ad4Row;
      *tmp[i] = ad4R;
      }
    flushPAV(set);
    }
  P_File* modAd4[MAX_SET] = { 0 };
  bool canExit = true;
  for(uint i = 0; i < nElem; ++i) {
    if(tmp[i]) {
      LPTSTR p = makeRow(*tmp[i]);
      uint prph = Set[i]->prph - 1;
      sSet[prph]->replaceString(Set[i]->addr + 1, p, true);
      while(!modAd4[prph]) {
        modAd4[prph] = makeAd4File(prph + 1);
        if(!modAd4[prph]) {
          DWORD res = msgNotOpen(this, prph + 1);
          if(IDCANCEL == res) {
            canExit = false;
            break;
            }
          else if(IDYES != res)
            break;
          }
        }

      }
    }
  if(canExit) {
    for(uint i = 0; i < SIZE_A(modAd4); ++i) {
      if(modAd4[i]) {
        saveSet(i + 1, *modAd4[i], *sSet[i]);
        safeDeleteP(modAd4[i]);
        }
      safeDeleteP(sSet[i]);
      }
    }
  else {
    for(uint i = 0; i < SIZE_A(modAd4); ++i) {
      safeDeleteP(modAd4[i]);
      safeDeleteP(sSet[i]);
      }
    }
  flushPV(tmp);

  if(canExit)
    baseClass::CmOk();
}
//----------------------------------------------------------------------------
void PDiagAd4::CmCancel()
{
  baseClass::CmCancel();
}
//----------------------------------------------------------------------------
DWORD load_CriptedTxt(LPCBYTE key, setOfString& set, LPCTSTR name)
{
  infoFileCr result;
  result.header = (LPCBYTE)ad7CRYPT_HEAD;
  result.lenHeader = ad7DIM_HEAD;

  if(decryptFile(name, key, ad7DIM_KEY_CRYPT, ad7CRYPT_STEP, result)) {
    set.add(result.dim, (LPSTR)result.buff, true);
    }
  else
    set.add(name + 1);
  return result.crc;
}
//----------------------------------------------------------------------------
