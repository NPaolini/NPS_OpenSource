//--------- language_util.cpp ---------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "language_util.h"
#include "p_Vect.h"
#include "p_Txt.h"
#include "p_Util.h"
#include "dManageStdMsg.h"
#include "POwnBtnImagestd.h"
#include "PCustomListViewFullEdit.h"
#include "common.h"
//-------------------------------------------------------------------
class dManageEditLang : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    dManageEditLang(LPTSTR* text, PWin* parent, HINSTANCE hInst = 0) :
        baseClass(parent, IDD_EDIT_LANG, hInst),
        Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)),
        Brush3(CreateSolidBrush(bkgColor3)),
        Text(text)
        {}
    ~dManageEditLang();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    LPTSTR* Text;
    HBRUSH Brush;
    HBRUSH Brush2;
    HBRUSH Brush3;
    PBitmap* Bmp[2];
    void fillDataCtrl();
    void CmOk();
    int checkMatch(LPCTSTR p);
};
//-------------------------------------------------------------------
dManageEditLang::~dManageEditLang()
{
  destroy();
  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
  DeleteObject(HGDIOBJ(Brush3));
  for(uint i = 0; i < SIZE_A(Bmp); ++i)
    delete Bmp[i];
}
//-------------------------------------------------------------------
bool dManageEditLang::create()
{
  int idBmp[] = { IDB_BITMAP_OK, IDB_BITMAP_CANC };
  int idBtn[] = { IDOK, IDCANCEL };
  PVect<PBitmap*> vBmp;
  for(uint i = 0; i < SIZE_A(Bmp); ++i) {
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());
    vBmp[0] = Bmp[i];
    new POwnBtnImage(this, idBtn[i], vBmp, false);
    }

  if(!baseClass::create())
    return false;


  fillDataCtrl();

  return true;
}
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT dManageEditLang::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HBRUSH dManageEditLang::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
//  long id = GetWindowLong(hWndChild, GWL_ID);
  if(WM_CTLCOLORSTATIC == ctlType) {
    TCHAR buff[10] = _T("");
    GetWindowText(hWndChild, buff, SIZE_A(buff));
    if(_T('1') <= *buff && *buff <= _T('9')) {
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
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
//----------------------------------------------------------------------------
#if 1
  #define GLOBAL_PAGE_CHAR  _T('|')
  #define LANGUAGE_SEP_CHAR _T('§')
//----------------------------------------------------------------------------
  #define GLOBAL_PAGE_STR  _T("|")
  #define LANGUAGE_SEP_STR _T("§")
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
  #define GLOBAL_PAGE_CHAR_  |
  #define LANGUAGE_SEP_CHAR_ §
//----------------------------------------------------------------------------
  #define STRINGER(a) #a
  #define MAKE_STRINGER(a) _T(STRINGER(a))

  #define GLOBAL_PAGE_STR  MAKE_STRINGER(GLOBAL_PAGE_CHAR_)
  #define LANGUAGE_SEP_STR MAKE_STRINGER(LANGUAGE_SEP_CHAR_)

  #define GLOBAL_PAGE_CHAR  GLOBAL_PAGE_STR[0]
  #define LANGUAGE_SEP_CHAR LANGUAGE_SEP_STR[0]
#endif
//----------------------------------------------------------------------------
extern
void makeStdMsgSet(setOfString& set);
//----------------------------------------------------------------------------
void dManageEditLang::fillDataCtrl()
{
  setOfString Set;
  makeStdMsgSet(Set);

  LPCTSTR pl = Set.getString(ID_LANGUAGE);
  pl = findNextParam(pl, 1);
  if(pl) {
    pvvChar target;
    uint nElem = splitParam(target, pl);
    for(uint i = 0; i < MAX_LANG && i < nElem; ++i) {
      LPCTSTR p = &target[i];
      if(*p)
        SET_TEXT(IDC_STATIC_L1 + i, p);
      }
    }

  LPTSTR p = *Text;
  if(!p)
    return;
  TCHAR match = LANGUAGE_SEP_CHAR;
  if(match != *p) {
    SET_TEXT(IDC_EDIT_DESCR_1, p);
    return;
    }
  pvvChar target;
  uint nElem = splitParam(target, p + 1, *p);
  for(uint i = 0; i < MAX_LANG && i < nElem; ++i) {
    LPCTSTR p = &target[i];
    if(*p)
      SET_TEXT(IDC_EDIT_DESCR_1 + i, p);
    }
}
//----------------------------------------------------------------------------
#define FULL_MATCH 3
//----------------------------------------------------------------------------
int dManageEditLang::checkMatch(LPCTSTR p)
{
  int bits = 0;
  for(; *p; ++p)
    if(LANGUAGE_SEP_CHAR == *p)
      bits |= 1;
    else if(GLOBAL_PAGE_CHAR == *p)
      bits |= 2;
  return bits;
}
//----------------------------------------------------------------------------
void dManageEditLang::CmOk()
{
  PVect<LPTSTR> pv;
  pv.setDim(MAX_LANG);
  int loaded = 0;
  int foundMatch = 0;
  for(uint i = 0; i < MAX_LANG; ++i) {
    pv[i] = 0;
    HWND he = GetDlgItem(*this, IDC_EDIT_DESCR_1 + i);
    int len = SendMessage(he, WM_GETTEXTLENGTH, 0, 0);
    if(len > 0) {
      pv[i] = new TCHAR[len + 1];
      SendMessage(he, WM_GETTEXT, len + 1, (LPARAM)pv[i]);
      foundMatch |= checkMatch(pv[i]);
      ++loaded;
      }
    }
  if(!loaded)
    return;

  if(1 == loaded) {
    for(uint i = 0; i < MAX_LANG; ++i) {
      if(pv[i]) {
        delete []*Text;
        *Text = str_newdup(pv[i]);
        flushPV(pv);
        baseClass::CmOk();
        return;
        }
      }
    }
  if(FULL_MATCH == foundMatch) {
    flushPV(pv);
    MessageBox(*this, _T("I testi non possono contenere entrambi i simboli: '") LANGUAGE_SEP_STR _T("' e '") GLOBAL_PAGE_STR _T("'"), _T("Errore!"),
            MB_OK | MB_ICONSTOP);
    return;
    }
  uint totLen = 10;
  for(uint i = 0; i < MAX_LANG; ++i) {
    if(pv[i])
      totLen += _tcslen(pv[i]);
    }
  delete []*Text;
  *Text = new TCHAR[totLen + 1];
  LPTSTR p = *Text;
  *p = 0;
  LPCTSTR sep = (foundMatch & 1) ? GLOBAL_PAGE_STR : LANGUAGE_SEP_STR;
  for(uint i = 0; i < MAX_LANG; ++i) {
    _tcscat_s(p, totLen + 1, sep);
    if(pv[i])
      _tcscat_s(p, totLen + 1, pv[i]);
    }
  flushPV(pv);
  baseClass::CmOk();
}
//-------------------------------------------------------------------
bool getGlobalPageStringSet(setOfString& set);
bool getSectionAlarmStringSet(setOfString& set);
//----------------------------------------------------------------------------
#define FILE_DAT_GLOBAL_TXT _T("globalPageText") PAGE_EXT
//----------------------------------------------------------------------------
void makeGlobalPageStringPath(LPTSTR path)
{
  dataProject& dp = getDataProject();
  _tcscpy_s(path, _MAX_PATH, dp.newPath);
  appendPath(path, dp.systemPath);
  appendPath(path, FILE_DAT_GLOBAL_TXT);
}
//----------------------------------------------------------------------------
#define FILE_SECTION_ALARM_TXT _T("alarmSectionId.txs")
//----------------------------------------------------------------------------
void getSectionAlarmStringPath(LPTSTR path)
{
  dataProject& dp = getDataProject();
  _tcscpy_s(path, _MAX_PATH, dp.newPath);
  appendPath(path, dp.systemPath);
  appendPath(path, FILE_SECTION_ALARM_TXT);
}
//----------------------------------------------------------------------------
#define BKG_COLOR RGB(0xF0, 0xFF, 0xFF)
#define TXT_COLOR RGB(0x00, 0x00, 0xC0)
//----------------------------------------------------------------------------
class myLV_Edit_Lang : public PCustomListViewFullEdit
{
  private:
    typedef PCustomListViewFullEdit baseClass;
  public:
    myLV_Edit_Lang(PWin * parent, uint resid, HINSTANCE hinst = 0) :
         baseClass(parent, resid, hinst), modified(false)   {  }
    virtual bool getColorBkg(COLORREF& c) { c = BKG_COLOR; return true; }
    virtual bool getColorTextBkg(COLORREF& c) { c = BKG_COLOR; return true; }
    virtual bool getColorText(COLORREF& c) {  c = TXT_COLOR; return true; }
    bool isDirty() { return modified || baseClass::isDirty(); }
    virtual bool ProcessNotify(LPARAM lParam, LRESULT& result);
  protected:
    virtual void onNm_Click(NMITEMACTIVATE* pia);
    virtual bool canEditItem(int item, int subItem) const { return subItem > 1; }
//    virtual int verifyKey() { int res = manageKey::verifyKey(); return res ? res : baseClass::verifyKey(); }
//    virtual bool evKeyDown(UINT& key) { return manageKey::evKeyDown(key); }
//    virtual bool evKeyUp(UINT& key) { return manageKey::evKeyUp(key); }
//    virtual bool evChar(UINT& key) { return manageKey::evChar(key); }

    void manageLang(NMITEMACTIVATE* pia);
    bool modified;
};
//-----------------------------------------------------------
bool myLV_Edit_Lang::ProcessNotify(LPARAM lParam, LRESULT& result)
{
  LPNMHDR lpnmHdr = (LPNMHDR)lParam;
  switch(lpnmHdr->code) {
    case NM_DBLCLK:
      manageLang((NMITEMACTIVATE*)lParam);
      break;
    }
  return baseClass::ProcessNotify(lParam, result);
}
//-------------------------------------------------------------------
#define DIM_BUFF_LV (4096 * 2)
//-------------------------------------------------------------------
void myLV_Edit_Lang::manageLang(NMITEMACTIVATE* pia)
{
  LPTSTR buff = new TCHAR[DIM_BUFF_LV];
  ZeroMemory(buff, DIM_BUFF_LV);
  ListView_GetItemText(*this, pia->iItem, 2, buff, DIM_BUFF_LV - 1);
  dManageEditLang(&buff, this).modal();
  if(buff) {
    ListView_SetItemText(*this, pia->iItem, 2, buff);
    delete []buff;
    modified = true;
    }
}
//-------------------------------------------------------------------
void myLV_Edit_Lang::onNm_Click(NMITEMACTIVATE* pia)
{
  if(pia->uKeyFlags & LVKF_CONTROL)
    manageLang(pia);
  baseClass::onNm_Click(pia);
}
//-------------------------------------------------------------------
class dManageEditBaseIdText : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    dManageEditBaseIdText(PWin* parent, HINSTANCE hInst = 0) :
        baseClass(parent, IDD_EDIT_GLOBAL_PAGE_TEXT, hInst), List(0)
        {}
    ~dManageEditBaseIdText();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    myLV_Edit_Lang* List;
    virtual void fillDataCtrl() = 0;
    void addRow(uint id, LPCTSTR str);
    void CmOk();
    virtual void save() = 0;
    virtual LPCTSTR getTitleStr() = 0;
    virtual LPCTSTR getCodeStr() = 0;
    virtual LPCTSTR getTextStr() = 0;
};
//-------------------------------------------------------------------
dManageEditBaseIdText::~dManageEditBaseIdText()
{
  destroy();
}
//----------------------------------------------------------------------------
void dManageEditBaseIdText::CmOk()
{
  if(List->isDirty())
    save();
  baseClass::CmOk();
}
//-------------------------------------------------------------------
bool dManageEditBaseIdText::create()
{
  List = new myLV_Edit_Lang(this, IDC_LIST_TEXT);

  int idBmp[] = { IDB_BITMAP_OK, IDB_BITMAP_CANC };
  int idBtn[] = { IDOK, IDCANCEL };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true);
    }
//  List->setFont(D_FONT(16, 0, 0, _T("arial")), true);

  if(!baseClass::create())
    return false;

  SetWindowText(*this, getTitleStr());
  LVCOLUMN lvCol;
  lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
  lvCol.cx = 0;
  lvCol.pszText = _T("");
  lvCol.fmt = LVCFMT_LEFT;
  ListView_InsertColumn(*List, 0, &lvCol);

  lvCol.cx = extent(*List, getCodeStr()) + 4;
  lvCol.pszText = (LPTSTR)getCodeStr();
  lvCol.fmt = LVCFMT_RIGHT;
  ListView_InsertColumn(*List, 1, &lvCol);

  PRect r;
  GetClientRect(*List, r);

  lvCol.cx = r.Width() - lvCol.cx - 20;
  lvCol.pszText = (LPTSTR)getTextStr();
  lvCol.fmt = LVCFMT_LEFT;
  ListView_InsertColumn(*List, 2, &lvCol);

  SetFocus(*List);
  fillDataCtrl();

  return true;
}
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT dManageEditBaseIdText::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          if(List->getEditHandle() == HWND(lParam))
            return 1;
          break;
        }
      break;

    case WM_NOTIFY:
      if(((NMHDR*)lParam)->idFrom == IDC_LIST_TEXT) {
        LRESULT result;
        if(List->ProcessNotify(lParam, result)) {
          SetWindowLongPtr(*this, DWLP_MSGRESULT, result);
          return result;
          }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void dManageEditBaseIdText::addRow(uint id, LPCTSTR str)
{
  TCHAR t[16] = {0};
  LVITEM lvItem;
  lvItem.mask = LVIF_TEXT;
  lvItem.pszText = t;
  lvItem.iItem = 65535;
  lvItem.iSubItem = 0;

  int ix = ListView_InsertItem(*List, &lvItem);
  wsprintf(t, _T("%d"), id);

  ListView_SetItemText(*List, ix, 1, t);
  ListView_SetItemText(*List, ix, 2, (LPTSTR)str);
}
//-------------------------------------------------------------------
class dManageEditGlobalPageText : public dManageEditBaseIdText
{
  private:
    typedef dManageEditBaseIdText baseClass;
  public:
    dManageEditGlobalPageText(LPTSTR* text, PWin* parent, HINSTANCE hInst = 0) :
        baseClass(parent, hInst), Text(text)
        {}
    ~dManageEditGlobalPageText();

  protected:

    LPTSTR* Text;
    void fillDataCtrl();
    void CmOk();
    void save();
    virtual LPCTSTR getTitleStr() { return _T("Immissione testi in lingua"); }
    virtual LPCTSTR getCodeStr() { return _T("Codice"); }
    virtual LPCTSTR getTextStr() { return _T("Testo collegato"); }
};
//-------------------------------------------------------------------
dManageEditGlobalPageText::~dManageEditGlobalPageText()
{
  destroy();
}
//----------------------------------------------------------------------------
#define MAX_TEXT_PAGE 1000
//----------------------------------------------------------------------------
void dManageEditGlobalPageText::fillDataCtrl()
{
  setOfString Set;
  getGlobalPageStringSet(Set);
  uint lastId = 0;
  if(Set.setFirst()) {
    do {
      uint id = Set.getCurrId();
      if(id > MAX_TEXT_PAGE)
        continue;
      if(id - lastId > 1) {
        for(uint i = lastId + 1; i < id; ++i)
          addRow(i, 0);
        }
      addRow(id, Set.getCurrString());
      lastId = id;
      } while(Set.setNext());
    }
  for(uint i = lastId + 1; i <= MAX_TEXT_PAGE; ++i)
    addRow(i, 0);
  if(Text && *Text) {
    uint ix = _ttoi(*Text + 1);
    ListView_SetItemState(*List, ix - 1, LVIS_SELECTED, LVIS_SELECTED);
    ListView_EnsureVisible(*List, ix - 1, false);
    }
}
//----------------------------------------------------------------------------
void dManageEditGlobalPageText::save()
{
  TCHAR buff[DIM_BUFF_LV];
  TCHAR path[_MAX_PATH];
  makeGlobalPageStringPath(path);
  do {
    P_File pf(path);
    pf.appendback();
    } while(false);
  P_File pf(path, P_CREAT);
  if(!pf.P_open()) {
    pf.unappendback();
    return;
    }
  TCHAR t[64];
  for(uint i = 0; i < MAX_TEXT_PAGE; ++i) {
    *buff = 0;
    ListView_GetItemText(*List, i, 2, (LPTSTR)buff, DIM_BUFF_LV);
    if(*buff) {
      wsprintf(t, _T("%d,"), i + 1);
      pf.P_writeString(t);
      pf.P_writeString(buff);
      pf.P_writeString(_T("\r\n"));
      }
    }
}
//----------------------------------------------------------------------------
void dManageEditGlobalPageText::CmOk()
{
  int sel = ListView_GetNextItem(*List, -1, LVNI_SELECTED);
  if(sel < 0 && Text)
    return;
  if(Text) {
    if(*Text) {
      uint ix = _ttoi(*Text + 1);
      if(ix != sel + 1) {
        delete[]*Text;
        TCHAR t[24] = _T("");
        wsprintf(t, _T("%c%d"), GLOBAL_PAGE_CHAR, sel + 1);
        *Text = str_newdup(t);
        }
      }
    else {
      TCHAR t[24] = _T("");
      wsprintf(t, _T("%c%d"), GLOBAL_PAGE_CHAR, sel + 1);
      *Text = str_newdup(t);
      }
    }
  baseClass::CmOk();
}
//-------------------------------------------------------------------
class dManageEditSectionAlarmText : public dManageEditBaseIdText
{
  private:
    typedef dManageEditBaseIdText baseClass;
  public:
    dManageEditSectionAlarmText(PWin* parent, HINSTANCE hInst = 0) :
        baseClass(parent, hInst)
        {}
    ~dManageEditSectionAlarmText();

  protected:

    void fillDataCtrl();
    void save();
    virtual LPCTSTR getTitleStr() { return _T("Immissione descrizione per Sezioni allarmi"); }
    virtual LPCTSTR getCodeStr() { return _T("Codice"); }
    virtual LPCTSTR getTextStr() { return _T("Descrizione Sezione"); }
};
//-------------------------------------------------------------------
dManageEditSectionAlarmText::~dManageEditSectionAlarmText()
{
  destroy();
}
//----------------------------------------------------------------------------
#define MAX_TEXT_PAGE 1000
//----------------------------------------------------------------------------
void dManageEditSectionAlarmText::fillDataCtrl()
{
  setOfString Set;
  getSectionAlarmStringSet(Set);
  uint lastId = 0;
  if(Set.setFirst()) {
    do {
      uint id = Set.getCurrId();
      addRow(id, Set.getCurrString());
      lastId = id;
      } while(Set.setNext());
    }
  for(uint i = lastId + 1; i <= MAX_TEXT_PAGE; ++i)
    addRow(i, 0);
}
//----------------------------------------------------------------------------
void dManageEditSectionAlarmText::save()
{
  TCHAR buff[DIM_BUFF_LV];
  TCHAR path[_MAX_PATH];
  getSectionAlarmStringPath(path);
  do {
    P_File pf(path);
    pf.appendback();
    } while(false);
  P_File pf(path, P_CREAT);
  if(!pf.P_open()) {
    pf.unappendback();
    return;
    }
  TCHAR t[64];
  for(uint i = 0; i < MAX_TEXT_PAGE; ++i) {
    *buff = 0;
    ListView_GetItemText(*List, i, 2, (LPTSTR)buff, DIM_BUFF_LV);
    if(*buff) {
      wsprintf(t, _T("%d,"), i + 1);
      pf.P_writeString(t);
      pf.P_writeString(buff);
      pf.P_writeString(_T("\r\n"));
      }
    }
}
//-------------------------------------------------------------------
LRESULT langEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      if(MK_CONTROL & wParam)
        runEditMulti();
      else if(GetKeyState(VK_MENU) & 0x8000)
        runGlobalPageTExt();
      break;
    case WM_LBUTTONDBLCLK:
//    case WM_RBUTTONUP:
      openMenuLang();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
};
//-------------------------------------------------------------------
void langEdit::openMenuLang()
{
  enum idMenu { idLang = 1000, idGlobal };
  menuPopup menu[] = {
    { MF_STRING, idLang, _T("editor lingua") },
    { MF_STRING, idGlobal, _T("stringhe globali") },
    };

  switch(popupMenu(*this, menu, SIZE_A(menu))) {
    case idLang:
      runEditMulti(true);
      break;
    case idGlobal:
      runGlobalPageTExt(true);
      break;
    }
}
//-------------------------------------------------------------------
void gRunGlobalPageTExt(PWin* owner)
{
  dManageEditGlobalPageText(0, owner).modal();
}
//-------------------------------------------------------------------
void gRunSectionAlarmText(PWin* owner)
{
  dManageEditSectionAlarmText(owner).modal();
}
//-------------------------------------------------------------------
void langEdit::runGlobalPageTExt(bool byMenu)
{
  LPTSTR buff = 0;
  int len = SendMessage(*this, WM_GETTEXTLENGTH, 0, 0);
  if(len > 0) {
    buff = new TCHAR[len + 1];
    SendMessage(*this, WM_GETTEXT, len + 1, (LPARAM)buff);
    }
  dManageEditGlobalPageText(&buff, this).modal();

  if(buff) {
    SendMessage(*this, WM_SETTEXT, 0, (LPARAM)buff);
    SendMessage(*this, EM_SETMODIFY, 1, 0);
    delete []buff;
    }
}
//-------------------------------------------------------------------
void langEdit::runEditMulti(bool byMenu)
{
  LPTSTR buff = 0;
  int len = SendMessage(*this, WM_GETTEXTLENGTH, 0, 0);
  if(len > 0) {
    buff = new TCHAR[len + 1];
    SendMessage(*this, WM_GETTEXT, len + 1, (LPARAM)buff);
    }
  if(len > 0 && *buff == GLOBAL_PAGE_CHAR)
    dManageEditGlobalPageText(&buff, this).modal();
  else
    dManageEditLang(&buff, this).modal();
  if(buff) {
    SendMessage(*this, WM_SETTEXT, 0, (LPARAM)buff);
    SendMessage(*this, EM_SETMODIFY, 1, 0);
    delete []buff;
    }
}
//-------------------------------------------------------------------
static int idNotLang[] = { 14, 25, 33, 44, 58 };

static int idNotLangRange[] = {  240, 250, 570, 580 };
//-------------------------------------------------------------------
int langCheckEdit::checkRunEdit()
{
  switch(resId2) {
    case FORCE_NO_LANG:
      return -1;
    case FORCE_ON_LANG:
      return 1;
    case FORCE_NO_ALL:
      return -2;
    case FORCE_ON_GLOB:
      return 2;
    case FORCE_OFF:
      return 3;
    }

  int id = GetDlgItemInt(*getParent(), resId2, 0, 0);
  for(uint i = 0; i < SIZE_A(idNotLang); ++i)
    if(id == idNotLang[i])
      return -1;
  for(uint i = 0; i < SIZE_A(idNotLangRange); i += 2)
    if(id >= idNotLangRange[i] && id <= idNotLangRange[i + 1])
      return -1;
  return 0;
}
//-------------------------------------------------------------------
void langCheckEdit::runEditMulti(bool byMenu)
{
  if(byMenu) {
    baseClass::runEditMulti();
    return;
    }
  switch(checkRunEdit()) {
    case -1:
    case -2:
      return;
    case 0:
    case 2:
      baseClass::runGlobalPageTExt();
      return;
    }
  baseClass::runEditMulti();
}
//-------------------------------------------------------------------
void langCheckEdit::runGlobalPageTExt(bool byMenu)
{
  if(byMenu) {
    baseClass::runGlobalPageTExt();
    return;
    }
  switch(checkRunEdit()) {
    case -1:
    case -2:
      return;
    case 0:
    case 1:
      baseClass::runEditMulti();
      return;
    }
  baseClass::runGlobalPageTExt();
}
//-------------------------------------------------------------------
void langCheckEdit::openMenuLang()
{
  switch(checkRunEdit()) {
    case -1:
    case -2:
      return;
    case 0:
    case 3:
      baseClass::openMenuLang();
      break;
    case 1:
      baseClass::runEditMulti();
      break;
    case 2:
      baseClass::runGlobalPageTExt();
      break;
    }
}
//-------------------------------------------------------------------
#include "p_param_v.h"
//----------------------------------------------------------------------------
extern int getCurrLang();
//----------------------------------------------------------------------------
bool getGlobalPageStringSet(setOfString& set)
{
  TCHAR path[_MAX_PATH];
  makeGlobalPageStringPath(path);
  set.add(path);
  return set.getNumElem() > 0;
}//----------------------------------------------------------------------------
bool getSectionAlarmStringSet(setOfString& set)
{
  TCHAR path[_MAX_PATH];
  getSectionAlarmStringPath(path);
  set.add(path);
  return set.getNumElem() > 0;
}
//----------------------------------------------------------------------------
LPCTSTR getGlobalPageString(uint id)
{
  static TCHAR str[4096];
  setOfString set;
  if(!getGlobalPageStringSet(set))
    return 0;
  LPCTSTR p = set.getString(id);
  if(p) {
    _tcscpy_s(str, p);
    return str;
    }
  return 0;
}
//----------------------------------------------------------------------------
LPCTSTR getStringByglobalPage(LPCTSTR p, bool& needDelete)
{
  int v = _ttoi(p + 1);
  if(!v)
    return p;
  LPCTSTR p2 = getGlobalPageString(v);
  if(!p2)
    return p;
  return getStringByLangSimple(p2, needDelete);
}
//----------------------------------------------------------------------------
bool isGlobalPageString(LPCTSTR p)
{
  return p && GLOBAL_PAGE_CHAR == *p;
}
//----------------------------------------------------------------------------
LPCTSTR getStringByLangSimple(LPCTSTR p, bool& needDelete)
{
  needDelete = false;
  if(!p)
    return 0;

  const TCHAR match1 = GLOBAL_PAGE_CHAR;
  if(match1 == *p)
    return getStringByglobalPage(p, needDelete);

//  if(!getCurrLang())
//    return p;

  const TCHAR match2 = LANGUAGE_SEP_CHAR;
  if(match2 != *p)
    return p;

  uint currLang = getCurrLang();
  if(currLang)
    --currLang;
  uint len = extractParam(0, 0, p + 1, currLang, *p);
  while(len <= 1) {
    // se non esiste, torna il primo
    currLang = 0;
    len = extractParam(0, 0, p + 1, currLang, *p);
    if(len <= 1)
      return p;

    break;
    }
  needDelete = true;
  LPTSTR result = new TCHAR[ len + 1];
  len = extractParam(result, len + 1, p + 1, currLang, *p);
  return result;
}
//----------------------------------------------------------------------------
smartPointerConstString getStringByLangSimple(LPCTSTR source)
{
  bool needDelete;
  LPCTSTR p = getStringByLangSimple(source, needDelete);
  smartPointerConstString sp(p, needDelete);
  return sp;
}
//----------------------------------------------------------------------------
