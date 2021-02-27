//--------- trend.cpp -----------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <stdio.h>
#include <math.h>
#include "infoText.h"
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
//-------------------------------------------------------------------
#include "commonSimpleRow.h"
#include "svmProperty.h"
#include "plistbox.h"
#include "clipboard_control.h"
//----------------------------------------------------------------------------
#define MAX_ITEM  300
#define PAGE_SCROLL_LEN  MAX_GROUP_SR
#define MAX_V_SCROLL (MAX_ITEM - MAX_GROUP_SR)
//----------------------------------------------------------------------------
#define DIM_PRF  4
#define DIM_ADDR 5
#define DIM_TYPE 4
#define DIM_NORM 4
#define DIM_DEC  4
#define DIM_NBIT 4
#define DIM_OFFS 4
#define DIM_TEXT (4096 - (DIM_PRF + DIM_ADDR + DIM_TYPE + DIM_NORM + DIM_DEC + DIM_NBIT + DIM_OFFS + 22))
#define MAX_BUFF_ITEM (DIM_PRF + DIM_ADDR + DIM_TYPE + DIM_NORM + DIM_DEC + DIM_TEXT + DIM_NBIT + DIM_OFFS + 20)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define WM_CUSTOM_VSCROLL (WM_APP + 101)
//----------------------------------------------------------------------------
#define dGET_SET(a) public : double get##a() const { return a; } void set##a(double t##a) { a = t##a; } private: double a;
#define iGET_SET(a) public : int get##a() const { return a; } void set##a(int t##a) { a = t##a; } private: int a;
#define sGET_SET(a) public : LPCTSTR get##a() const { return a; } void set##a(LPCTSTR t##a) { delete[]a; a = str_newdup(t##a); } private: LPCTSTR a;
//----------------------------------------------------------------------------
class itemTrend
{
  public:
    itemTrend() : Prph(0), Addr(0), Type(0), Norm(0), Text(0), Dec(0), NBit(0), Offs(0) {}
    itemTrend(int idprph, int addr, int dataType, int idNorm, int dec, int nbit, double offs, LPCTSTR text) :
        Prph(idprph), Addr(addr), Type(dataType), Norm(idNorm), Dec(dec), NBit(nbit), Offs(offs),
        Text(str_newdup(text)) {}

    itemTrend(const itemTrend& other) { clone(other); }
    const itemTrend& operator=(const itemTrend& other) { clone(other); return *this; }

    ~itemTrend() { delete []Text; }

    int diff(const itemTrend* other);

    iGET_SET(Prph)
    iGET_SET(Addr)
    iGET_SET(Type)
    iGET_SET(NBit)
    dGET_SET(Offs)
    iGET_SET(Norm)
    iGET_SET(Dec)
    sGET_SET(Text)

  private:
    void clone(const itemTrend& other);
};
//----------------------------------------------------------------------------
#define RET_CHECKd(v) { \
  double result = v - other->v; \
  if(fabs(result) > 0.000005) \
    return result > 0 ? 1 : -1; }
//----------------------------------------------------------------------------
#define RET_CHECK(v) { \
  int result = v - other->v; \
  if(result) \
    return result; }
//----------------------------------------------------------------------------
#define RET_CHECK_STR(v) { \
  LPCTSTR t1 = v; \
  if(!t1)\
    t1 = _T("\0");\
  LPCTSTR t2 = other->v; \
  if(!t2)\
    t2 = _T("\0");\
  int result = _tcsicmp(t1, t2); \
  if(result) \
    return result; }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int itemTrend::diff(const itemTrend* other)
{
  RET_CHECK(Prph)
  RET_CHECK(Addr)
  RET_CHECK(Type)
  RET_CHECK(Norm)
  RET_CHECK(Dec)
  RET_CHECK(NBit)
  RET_CHECKd(Offs)
  RET_CHECK_STR(Text)
  return 0;
}
//----------------------------------------------------------------------------
typedef PVect<itemTrend*> pvItemTrend;
//----------------------------------------------------------------------------
class manField
{
  public:
    manField() {}
    ~manField() { flush(); }

    void loadFromListbox(HWND hwlb);
    void fillLB(HWND hwlb);

//    void flush() { flushPV(Field); }
    void load(const setOfString& set, int nData, uint id);
    bool save(P_File& pfCrypt, P_File& pfClear, uint id);

    int getElem() { return Field.getElem(); }
    int diff(const manField& other);
  private:
    pvItemTrend Field;
    itemTrend* allocItemByBuff(LPCTSTR buff);
    void fillBuffByItem(LPTSTR buff, itemTrend* item);
    void flush() { flushPV(Field); }
};
//----------------------------------------------------------------------------
int manField::diff(const manField& other)
{
  uint nElem = Field.getElem();
  if(other.Field.getElem() != nElem)
    return 1;
  for(uint i = 0; i < nElem; ++i)
    if(Field[i]->diff(other.Field[i]))
      return 1;
  return 0;
}
//----------------------------------------------------------------------------
#define MAX_NAME  40
#define MAX_PREFIX 6
#define MAX_EXT    6
//----------------------------------------------------------------------------
class manTrend
{
  public:
    manTrend();
    ~manTrend() { flushPV(forName); }

    void loadFromDialog(HWND hw);
    void fillDialog(HWND hw);

    void load(const setOfString& set, uint id, uint ix);
    bool save(P_File& pfCrypt, P_File& pfClear, uint id);

    LPCTSTR getStringForLB(LPTSTR buff);
    LPCTSTR makeStringForLB(LPTSTR buff, size_t lenBuff, HWND hw);

    int diff(const manTrend* other);

  private:
    TCHAR Name[MAX_NAME];
    int addrTimer;
    int resExport;
    bool History;
    int trendType;
    TCHAR Prefix[MAX_PREFIX];
    TCHAR Ext[MAX_EXT];
    int savePrph;
    int saveAddr;
    int saveType;
    int saveNBit;
    int saveOffs;

    manField Field;
    pvItemTrend forName;

    void checkExt();

    bool writeSimple(P_File& pf, HWND hw, uint idc, uint id);
};
//----------------------------------------------------------------------------
typedef PVect<manTrend*> pvManTrend;
//----------------------------------------------------------------------------
class dManageStdTrend : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdTrend(const setOfString& set, PWin* parent, HINSTANCE hInst = 0);
    ~dManageStdTrend();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
    uint needSaveBeforeClose();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    pvManTrend mTrend;
    void loadData();
    void loadAll();
    void addCurrData();
    void remCurrData();
    void saveCurrData(int pos = -1);
    void checkEnableSave();
    void checkEnableCodeName();
    void fillCBTypeTrend();
    void glob_move(bool up);

    void typeNameAdd();
    void typeNameRem();
    void typeNameMove(bool up);
    void fieldAdd(bool append);
    void fieldRem();
    void fieldMod();
    void fieldMove(bool up);

    void updateTotElem();
    bool existCurrData();
    bool checkDirtyAndSave(int pos = -1);

    int currPos;
    uint Dirty2;

    void copyFromLb();
    void pasteToLb();

};
//-------------------------------------------------------------------
basePage* allocStdTrend(const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdTrend(Set, parent, hInst);
}
//-------------------------------------------------------------------
dManageStdTrend::dManageStdTrend(const setOfString& set, PWin* parent, HINSTANCE hInst) :
        baseClass(set, parent, IDD_STD_MSG_TREND, hInst), currPos(0), Dirty2(0)
{
  new langEdit(this, IDC_EDIT_T_F_EXPORT);
}
//-------------------------------------------------------------------
dManageStdTrend::~dManageStdTrend()
{
  destroy();
  flushPV(mTrend);
}
//-------------------------------------------------------------------
bool dManageStdTrend::create()
{
  PListBox* lb = new PListBox(this, IDC_LIST_FIELD);

  int tabs[] = { DIM_PRF, DIM_ADDR, DIM_TYPE, DIM_NBIT, DIM_OFFS, DIM_NORM, DIM_DEC, DIM_TEXT };
  lb->SetTabStop(SIZE_A(tabs), tabs, 0);
  lb->SetColor(0, LABEL_COLOR);
  lb->SetColorSel(0, CYAN_COLOR);

  if(!baseClass::create())
    return false;

  lb->setIntegralHeight();
  fillCBTypeTrend();

  loadAll();
  checkEnableSave();
  checkEnableCodeName();
  SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_SETCURSEL, 0, 0);
  SendMessage(GetDlgItem(*this, IDC_LIST_FIELD), LB_SETCURSEL, 0, 0);
  updateTotElem();
  return true;
}
//-------------------------------------------------------------------
LRESULT dManageStdTrend::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_T_GLOB_ADD:
          addCurrData();
          break;
        case IDC_BUTTON_T_GLOB_REM:
          remCurrData();
          break;
        case IDC_BUTTON_T_GLOB_MOD:
          saveCurrData();
          break;
        case IDC_BUTTON_T_GLOB_UP:
          glob_move(true);
          break;
        case IDC_BUTTON_T_GLOB_DN:
          glob_move(false);
          break;
        case IDC_CHECK_T_ENABLE_SAVE:
          checkEnableSave();
          break;
        case IDC_COMBO_T_TYPE:
          switch(HIWORD(wParam)) {
            case CBN_SELCHANGE:
              checkEnableCodeName();
              break;
            }
          break;
        case IDC_LIST_T_ALL:
          switch(HIWORD(wParam)) {
            case LBN_SELCHANGE:
              loadData();
              break;
            }
          break;
        case IDC_BUTTON_T_TY_ADD:
          typeNameAdd();
          break;
        case IDC_BUTTON_T_TY_REM:
          typeNameRem();
          break;
        case IDC_BUTTON_T_TY_UP:
          typeNameMove(true);
          break;
        case IDC_BUTTON_T_TY_DN:
          typeNameMove(false);
          break;

        case IDC_BUTTON_T_F_MOD:
          fieldMod();
          break;
        case IDC_BUTTON_T_F_ADD:
          fieldAdd(true);
          break;
        case IDC_BUTTON_T_F_INSERT:
          fieldAdd(false);
          break;
        case IDC_BUTTON_T_F_REM:
          fieldRem();
          break;
        case IDC_BUTTON_T_F_UP:
          fieldMove(true);
          break;
        case IDC_BUTTON_T_F_DN:
          fieldMove(false);
          break;

        case IDC_BUTTON_T_F_COPY:
          copyFromLb();
          break;
        case IDC_BUTTON_T_F_PASTE:
          pasteToLb();
          break;

        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//------------------------------------------------------------------------------
void dManageStdTrend::typeNameAdd()
{
  TCHAR buff[100] = _T("\0");
  TCHAR t[20];
  uint idc[] = {
    IDC_EDIT_T_TY_PRF,
    IDC_EDIT_T_TY_ADDR,
    IDC_EDIT_T_TY_TYPE,
    IDC_EDIT_T_TY_LEN,
    };
  for(uint i = 0; i < SIZE_A(idc); ++i) {
    GET_TEXT(idc[i], t);
    if(!*t)
      return;
    _tcscat_s(buff, t);
    _tcscat_s(buff, _T(","));
    }
  int len = _tcslen(buff);
  buff[len - 1] = 0;
  SendMessage(GetDlgItem(*this, IDC_LIST_T_TYPE), LB_ADDSTRING, 0, (LPARAM)buff);
}
//------------------------------------------------------------------------------
int remFromLb(HWND hwLb, LPTSTR buff)
{
  int pos = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return -1;
  if(buff)
    SendMessage(hwLb, LB_GETTEXT, pos, (LPARAM)buff);
  int count = SendMessage(hwLb, LB_DELETESTRING, pos, 0);
  int isLast = 0;
  if(pos >= count) {
    pos = count - 1;
    SendMessage(hwLb, LB_SETCARETINDEX, 0, FALSE);
    isLast = 1;
    }
  SendMessage(hwLb, LB_SETCURSEL, pos, 0);
  SendMessage(hwLb, LB_SETCARETINDEX, pos, FALSE);
  return isLast;
}
//------------------------------------------------------------------------------
int moveOnLb(HWND hwLb, bool up)
{
  int pos = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return -1;
  int count = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  if(count < 2)
    return -1;
  if(up && !pos)
    return -1;
  if(!up && pos == count - 1)
    return -1;
  int len = SendMessage(hwLb, LB_GETTEXTLEN, pos, 0);

  smartPointerString buff(new TCHAR[len + 2], true);

  SendMessage(hwLb, LB_GETTEXT, pos, (LPARAM)(LPTSTR)buff);
  SendMessage(hwLb, LB_DELETESTRING, pos, 0);
  if(up)
    --pos;
  else
    ++pos;
  SendMessage(hwLb, LB_INSERTSTRING, pos, (LPARAM)(LPTSTR)buff);
  SendMessage(hwLb, LB_SETCURSEL, pos, 0);
  return 1;
}
//------------------------------------------------------------------------------
void dManageStdTrend::typeNameRem()
{
  remFromLb(GetDlgItem(*this, IDC_LIST_T_TYPE));
}
//------------------------------------------------------------------------------
void dManageStdTrend::typeNameMove(bool up)
{
  moveOnLb(GetDlgItem(*this, IDC_LIST_T_TYPE), up);
}
//------------------------------------------------------------------------------
void dManageStdTrend::updateTotElem()
{
  HWND hwnd = GetDlgItem(*this, IDC_LIST_FIELD);
  int count = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
  TCHAR buff[100];
  wsprintf(buff, _T("Totale elementi [%d]"), count);
  SET_TEXT(IDC_GROUPBOX_FIELD, buff);
}
//------------------------------------------------------------------------------
static uint Idcs_ItemTrend[] =  {
    IDC_EDIT_T_F_PRF,
    IDC_EDIT_T_F_ADDR,
    IDC_EDIT_T_F_TYPE,
    IDC_EDIT_T_F_NBIT,
    IDC_EDIT_T_F_OFFS,
    IDC_EDIT_T_F_NORM,
    IDC_EDIT_T_F_DEC,
    IDC_EDIT_T_F_EXPORT,
    };
//------------------------------------------------------------------------------
static uint Len_ItemTrend[] =  {
      DIM_PRF,
      DIM_ADDR,
      DIM_TYPE,
      DIM_NBIT,
      DIM_OFFS,
      DIM_NORM,
      DIM_DEC,
      DIM_TEXT,
      };
//------------------------------------------------------------------------------
void dManageStdTrend::fieldAdd(bool append)
{
  TCHAR buff[MAX_BUFF_ITEM + 2];

  fillStr(buff, _T(' '), MAX_BUFF_ITEM);
  LPTSTR p = buff;
  TCHAR t[50];

  for(uint i = 0; i < SIZE_A(Idcs_ItemTrend); ++i) {
    GET_TEXT(Idcs_ItemTrend[i], t);
    if(!*t)
      return;
    copyStr(p, t, _tcslen(t));
    p += Len_ItemTrend[i];
    *p++ = _T('\t');
    }
  *p = 0;
  HWND hwnd = GetDlgItem(*this, IDC_LIST_FIELD);
  int sel = -1;
  if(!append)
    sel = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
  SendMessage(hwnd, LB_INSERTSTRING, sel, (LPARAM)buff);
  updateTotElem();
  Dirty |= 1;
}
//------------------------------------------------------------------------------
void dManageStdTrend::fieldRem()
{
  TCHAR buff[MAX_BUFF_ITEM + 2];

  if(-1 != remFromLb(GetDlgItem(*this, IDC_LIST_FIELD), buff)) {
    LPTSTR p = buff;
    for(uint i = 0; i < SIZE_A(Len_ItemTrend); ++i) {
      p[Len_ItemTrend[i]] = 0;
      lTrim(trim(p));
      SET_TEXT(Idcs_ItemTrend[i], p);
      p += Len_ItemTrend[i] + 1;
      }

    Dirty |= 1;
    }
  updateTotElem();
}
//------------------------------------------------------------------------------
class Trend_ModRow : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    Trend_ModRow(PWin* parent, LPTSTR buff, uint id = IDD_STD_MSG_TREND_MOD) :
      baseClass(parent, id), Buff(buff) {}
    ~Trend_ModRow() { destroy(); }
    bool create();
  protected:
    void CmOk();
    LPTSTR Buff;
};
//------------------------------------------------------------------------------
bool Trend_ModRow::create()
{
  new langEdit(this, IDC_EDIT_T_F_EXPORT);
  if(!baseClass::create())
    return false;
  PRect r;
  GetWindowRect(GetDlgItem(*this, IDC_EDIT_T_F_PRF), r);

  PRect r2;
  GetWindowRect(GetDlgItem(*getParent(), IDC_EDIT_T_F_PRF), r);
  int dx = r2.left - r.left;
  int dy = r2.top - r.top;
  GetWindowRect(*this, r);
  r.left -= dx;
  r.top -= dy;
  MapWindowPoints(0, *this, (LPPOINT)(LPRECT)r, 2);
  r.left -= GetSystemMetrics(SM_CXBORDER) * 2;
  r.top -= GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER) * 4;
  SetWindowPos(*this, 0, r.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

  LPTSTR p = Buff;
  for(uint i = 0; i < SIZE_A(Len_ItemTrend); ++i) {
    p[Len_ItemTrend[i]] = 0;
    lTrim(trim(p));
    SET_TEXT(Idcs_ItemTrend[i], p);
    p += Len_ItemTrend[i] + 1;
    }
  return true;
}
//------------------------------------------------------------------------------
void Trend_ModRow::CmOk()
{
  fillStr(Buff, _T(' '), MAX_BUFF_ITEM);
  LPTSTR p = Buff;
  TCHAR t[DIM_TEXT];

  for(uint i = 0; i < SIZE_A(Idcs_ItemTrend); ++i) {
    GET_TEXT(Idcs_ItemTrend[i], t);
    if(!*t)
      return;
    copyStr(p, t, _tcslen(t));
    p += Len_ItemTrend[i];
    *p++ = _T('\t');
    }
  *p = 0;
  baseClass::CmOk();
}
//------------------------------------------------------------------------------
void dManageStdTrend::fieldMod()
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_FIELD);
  int pos = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  TCHAR buff[MAX_BUFF_ITEM + 2];
  SendMessage(hwLb, LB_GETTEXT, pos, (LPARAM)buff);
  if(IDOK == Trend_ModRow(this, buff).modal()) {
    SendMessage(hwLb, LB_DELETESTRING, pos, 0);
    SendMessage(hwLb, LB_INSERTSTRING, pos, (LPARAM)buff);
    SendMessage(hwLb, LB_SETCURSEL, pos, 0);
    Dirty |= 1;
    }
}
//------------------------------------------------------------------------------
void dManageStdTrend::fieldMove(bool up)
{
  if(-1 != moveOnLb(GetDlgItem(*this, IDC_LIST_FIELD), up))
    Dirty |= 1;
}
//------------------------------------------------------------------------------
void dManageStdTrend::loadAll()
{
  do {
    LPCTSTR p = Set.getString(ID_GRAPH_MAX_REC_SHOW);
    if(!p)
      break;
    SET_TEXT(IDC_EDIT_MAX_REC, p);
    } while(false);
  do {
    LPCTSTR p = Set.getString(ID_MAX_TREND_DATA_DIM);
    if(!p)
      break;
    SET_TEXT(IDC_EDIT_MAX_DIM_FILES, p);
    } while(false);

  flushPV(mTrend);
  LPCTSTR p = Set.getString(INIT_TREND_DATA);
  if(!p)
    return;

  int nElem = 0;
  HWND hwLb = GetDlgItem(*this, IDC_LIST_T_ALL);
  do {
    int id = _ttoi(p);
    manTrend* mt = new manTrend;
    mt->load(Set, id, nElem);
    TCHAR buff[500];
    LPCTSTR plb = mt->getStringForLB(buff);
    if(!plb)
      delete mt;
    else {
      SendMessage(hwLb, LB_ADDSTRING, 0, (LPARAM)buff);
      mTrend[nElem] = mt;
      ++nElem;
      }
    p = findNextParam(p, 1);
    } while(p);

  if(nElem)
    mTrend[0]->fillDialog(*this);

}
//------------------------------------------------------------------------------
void dManageStdTrend::remCurrData()
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_T_ALL);
  int pos = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  int nElem = mTrend.getElem();
  if(pos >= nElem)
    return;
  manTrend* mt = mTrend.remove(pos);
  delete mt;
  pos = SendMessage(hwLb, LB_DELETESTRING, pos, 0) - 1;
  if(pos < 0 || pos >= nElem - 1)
    pos = 0;
  if(nElem > 1)
    mTrend[pos]->fillDialog(*this);
  currPos = pos;

  Dirty = 0;
}
//------------------------------------------------------------------------------
bool dManageStdTrend::existCurrData()
{
  int nElem = mTrend.getElem();
  if(!nElem)
    return false;
  TCHAR buff[500];
  LPCTSTR plb = mTrend[0]->makeStringForLB(buff, SIZE_A(buff), *this);
  if(!plb)
    return true;
  TCHAR test[500];
  for(int i = 0; i < nElem; ++i) {
    mTrend[i]->getStringForLB(test);
    if(!_tcsicmp(buff, test))
      return true;
    }
  return false;
}
//------------------------------------------------------------------------------
void dManageStdTrend::addCurrData()
{
  if(existCurrData())
    return;

  int nElem = mTrend.getElem();
  TCHAR buff[500];
  manTrend* mt = new manTrend;
  mt->loadFromDialog(*this);
  mt->getStringForLB(buff);
  int sel = SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_SETCURSEL, sel, 0);
  currPos = sel;
  mTrend[nElem] = mt;
  Dirty = 0;
}
//------------------------------------------------------------------------------
void dManageStdTrend::glob_move(bool up)
{
  HWND hwLb = GetDlgItem(*this, IDC_LIST_T_ALL);
  int sel = SendMessage(hwLb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  if(!sel && up)
    return;
  int nElem = mTrend.getElem();
  int sel2 = up ? sel - 1 : sel + 1;
  if(sel2 >= nElem)
    return;
  uint old = Dirty;
  Dirty = 0;
  TCHAR buff[500];
  SendMessage(hwLb, LB_GETTEXT, sel, (LPARAM)(LPTSTR)buff);
  SendMessage(hwLb, LB_DELETESTRING, sel, 0);
  SendMessage(hwLb, LB_INSERTSTRING, sel2, (LPARAM)(LPTSTR)buff);
  SendMessage(hwLb, LB_SETCURSEL, sel2, 0);
  manTrend* t = mTrend[sel];
  mTrend[sel] = mTrend[sel2];
  mTrend[sel2] = t;
  currPos = sel2;
  Dirty = old;
}
//------------------------------------------------------------------------------
void dManageStdTrend::saveCurrData(int forcePos)
{
  int pos;
  if(forcePos >= 0)
    pos = forcePos;
  else
     pos = SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  int nElem = mTrend.getElem();
  if(pos >= nElem)
    return;
  HWND hwLb = GetDlgItem(*this, IDC_LIST_T_ALL);
  TCHAR buff[100];
  SendMessage(hwLb, LB_GETTEXT, pos, (LPARAM)buff);
  mTrend[pos]->loadFromDialog(*this);

  TCHAR curr[100];
  if(mTrend[pos]->getStringForLB(curr)) {
    if(_tcsicmp(curr, buff)) {
//      if(existCurrData())
//        return;
      SendMessage(hwLb, LB_DELETESTRING, pos, 0);
      SendMessage(hwLb, LB_INSERTSTRING, pos, (LPARAM)curr);
      SendMessage(hwLb, LB_SETCURSEL, pos, 0);
      }
    }
  Dirty = 0;
}
//-------------------------------------------------------------------
uint dManageStdTrend::needSaveBeforeClose()
{
  // il secondo serve per ricordare lo stato nel caso si scegliesse annulla
  Dirty |= Dirty2;
  if(!Dirty && mTrend.getElem()) {
    manTrend mt;
    mt.loadFromDialog(*this);
    Dirty = mt.diff(mTrend[currPos]);
    }
  if(Dirty) {
    Dirty2 = Dirty;
    // per evitare la doppia richiesta in caso si scelga di salvare
    saveCurrData(currPos);
    return reqSave();
    }
  return 0;
}
//------------------------------------------------------------------------------
bool dManageStdTrend::checkDirtyAndSave(int pos)
{
  if(!Dirty && mTrend.getElem()) {
    manTrend mt;
    mt.loadFromDialog(*this);
    Dirty = mt.diff(mTrend[currPos]);
    }
  if(Dirty) {
    uint result = MessageBox(*this, _T("Vuoi salvare i dati prima di procedere?"),
              _T("[Trend] I dati sono stati modificati"),
              MB_YESNOCANCEL);
    if(IDCANCEL == result)
      return false;
    if(IDYES == result) {
      saveCurrData(currPos);
      if(pos >= 0)
        SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_SETCURSEL, pos, 0);
      }
    }
  return true;
}
//------------------------------------------------------------------------------
void dManageStdTrend::loadData()
{
  int pos = SendMessage(GetDlgItem(*this, IDC_LIST_T_ALL), LB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  int nElem = mTrend.getElem();
  if(pos >= nElem)
    return;
  if(!checkDirtyAndSave(pos))
    return;
  Dirty = 0;
  mTrend[pos]->fillDialog(*this);
  updateTotElem();
  checkEnableSave();
  checkEnableCodeName();
  currPos = pos;
}
//------------------------------------------------------------------------------
void dManageStdTrend::checkEnableSave()
{
  bool enable = IS_CHECKED(IDC_CHECK_T_ENABLE_SAVE);
  uint idc[] = {
    IDC_EDIT_T_EN_PRF,
    IDC_EDIT_T_EN_ADDR,
    IDC_EDIT_T_EN_TYPE,
    IDC_EDIT_T_EN_NBIT,
    IDC_EDIT_T_EN_OFFS,
    };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
}
//------------------------------------------------------------------------------
#define IX_NAMEBYCODE 3
//------------------------------------------------------------------------------
void dManageStdTrend::fillCBTypeTrend()
{
  LPCTSTR txt[] = {
    _T("Giornaliero"),
    _T("Mensile"),
    _T("Annuale"),
    _T("da Variabile"),
    };
  HWND hwnd = GetDlgItem(*this, IDC_COMBO_T_TYPE);
  for(uint i = 0; i < SIZE_A(txt); ++i)
    addStringToComboBox(hwnd, txt[i]);
}
//------------------------------------------------------------------------------
void dManageStdTrend::checkEnableCodeName()
{
  int pos = SendMessage(GetDlgItem(*this, IDC_COMBO_T_TYPE), CB_GETCURSEL, 0, 0);
  if(pos < 0)
    return;
  bool enable = IX_NAMEBYCODE == pos;
  uint idc[] = {
    IDC_EDIT_T_TY_PRF,
    IDC_EDIT_T_TY_ADDR,
    IDC_EDIT_T_TY_TYPE,
    IDC_EDIT_T_TY_LEN,
    IDC_LIST_T_TYPE,
    IDC_BUTTON_T_TY_ADD,
    IDC_BUTTON_T_TY_REM,
    IDC_BUTTON_T_TY_UP,
    IDC_BUTTON_T_TY_DN,
    };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
}
//----------------------------------------------------------------------------
HBRUSH dManageStdTrend::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    uint groupBox[] = {
      IDC_GROUPBOX_T,
      IDC_GROUPBOX_FIELD,
      };
    for(uint i = 0; i < SIZE_A(groupBox); ++i)
      if(GetDlgItem(*this, groupBox[i]) == hWndChild) {
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
//-------------------------------------------------------------------
#define OFFSET_TREND_CODE_BASE    10
#define BASE_TREND_CODE_DATA   10000
#define OFFSET_TREND_CODE_DATA  1000
//-------------------------------------------------------------------
#define MIN_REC_2_SHOW  500
#define MAX_REC_2_SHOW  100000
//-------------------------------------------------------------------
#define MIN_TREND_DATA_DIM 50
#define MAX_TREND_DATA_DIM  20000
//-------------------------------------------------------------------
bool dManageStdTrend::save(P_File& pfCrypt, P_File& pfClear)
{
  if(!checkDirtyAndSave())
    return false;
  do {
    int val;
    GET_INT(IDC_EDIT_MAX_REC, val);
    if(val <= 0)
      break;
    if(val < MIN_REC_2_SHOW)
      val = MIN_REC_2_SHOW;
    else if(val > MAX_REC_2_SHOW)
      val = MAX_REC_2_SHOW;
    TCHAR buff[64];
    wsprintf(buff, _T("%d,%d\r\n"), ID_GRAPH_MAX_REC_SHOW, val);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    } while(false);
  do {
    int val;
    GET_INT(IDC_EDIT_MAX_DIM_FILES, val);
    if(val <= 0)
      break;
    if(val < MIN_TREND_DATA_DIM)
      val = MIN_TREND_DATA_DIM;
    else if(val > MAX_TREND_DATA_DIM)
      val = MAX_TREND_DATA_DIM;
    TCHAR buff[64];
    wsprintf(buff, _T("%d,%d\r\n"), ID_MAX_TREND_DATA_DIM, val);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    } while(false);

/*
  do {
    LPCTSTR p = Set.getString(ID_MAX_TREND_DATA_DIM);
    if(!p)
      break;
    SET_TEXT(IDC_EDIT_MAX_DIM_FILES, p);
    } while(false);
*/

  int nElem = mTrend.getElem();
  if(!nElem)
    return true;

  TCHAR buff[20];
  wsprintf(buff, _T("%d"), INIT_TREND_DATA);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  int idInit = INIT_TREND_DATA + 1;
  for(int i = 0; i < nElem; ++i, idInit += OFFSET_TREND_CODE_BASE) {
    if(!writeStringChkUnicode(pfCrypt, _T(",")))
      return false;
    wsprintf(buff, _T("%d"), idInit);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }
  if(!writeStringChkUnicode(pfCrypt, _T("\r\n")))
    return false;

//  idInit = INIT_TREND_DATA + 1;
  for(int i = 0; i < nElem; ++i) {
    if(!mTrend[i]->save(pfCrypt, pfClear, i))
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
void dManageStdTrend::copyFromLb()
{
  uint tabs[] = { DIM_PRF, DIM_ADDR, DIM_TYPE, DIM_NBIT, DIM_OFFS, DIM_NORM, DIM_DEC, DIM_TEXT };
  infoClipControl iclb(GetDlgItem(*this, IDC_LIST_FIELD), tabs, SIZE_A(tabs));
  iclb.calc_dim_buff_and_alloc();
  myClipboard_ListBox clip(this);
  clip.copyFromControl(iclb);
}
//------------------------------------------------------------------------------
void dManageStdTrend::pasteToLb()
{
  uint tabs[] = { DIM_PRF, DIM_ADDR, DIM_TYPE, DIM_NBIT, DIM_OFFS, DIM_NORM, DIM_DEC, DIM_TEXT };
  infoClipControl iclb(GetDlgItem(*this, IDC_LIST_FIELD), tabs, SIZE_A(tabs));
  iclb.calc_dim_buff_and_alloc();
  myClipboard_ListBox clip(this);
  clip.pasteToControl(iclb);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define CLONE(a) a = other.a
//----------------------------------------------------------------------------
void itemTrend::clone(const itemTrend& other)
{
  if(&other == this)
    return;
  CLONE(Prph);
  CLONE(Addr);
  CLONE(Type);
  CLONE(Norm);
  CLONE(Dec);
  CLONE(NBit);
  CLONE(Offs);
  delete []Text;
  Text = str_newdup(other.Text);
}
//----------------------------------------------------------------------------
itemTrend* manField::allocItemByBuff(LPCTSTR buff)
{
  LPCTSTR p = buff;
  int prph = _ttoi(p);
  p += DIM_PRF + 1;
  int addr =  _ttoi(p);
  p += DIM_ADDR + 1;
  int type =  _ttoi(p);
  p += DIM_TYPE + 1;
  int nbit = _ttoi(p);
  p += DIM_NBIT + 1;
  double offs = _tstof(p);
  p += DIM_OFFS + 1;
  int norm =  _ttoi(p);
  p += DIM_NORM + 1;
  int dec =  _ttoi(p);
  p += DIM_DEC + 1;

  TCHAR t[DIM_TEXT + 1];
  copyStr(t, p, DIM_TEXT);
  t[DIM_TEXT] = 0;
  trim(t);
  itemTrend* item = new itemTrend(prph, addr, type, norm, dec, nbit, offs, t);
  return item;
}
//----------------------------------------------------------------------------
void manField::fillBuffByItem(LPTSTR buff, itemTrend* item)
{
  fillStr(buff, _T(' '), MAX_BUFF_ITEM);
  LPTSTR p = buff;
  TCHAR t[50];
  wsprintf(t, _T("%d"), item->getPrph());
  copyStr(p, t, _tcslen(t));
  p += DIM_PRF;
  *p++ = _T('\t');
  wsprintf(t, _T("%d"), item->getAddr());
  copyStr(p, t, _tcslen(t));
  p += DIM_ADDR;
  *p++ = _T('\t');
  wsprintf(t, _T("%d"), item->getType());
  copyStr(p, t, _tcslen(t));
  p += DIM_TYPE;
  *p++ = _T('\t');

  wsprintf(t, _T("%d"), item->getNBit());
  copyStr(p, t, _tcslen(t));
  p += DIM_NBIT;
  *p++ = _T('\t');
  _stprintf_s(t, _T("%f"), item->getOffs());
  zeroTrim(t);
  copyStr(p, t, _tcslen(t));
  p += DIM_OFFS;
  *p++ = _T('\t');

  wsprintf(t, _T("%d"), item->getNorm());
  copyStr(p, t, _tcslen(t));
  p += DIM_NORM;
  *p++ = _T('\t');
  wsprintf(t, _T("%d"), item->getDec());
  copyStr(p, t, _tcslen(t));
  p += DIM_DEC;
  *p++ = _T('\t');

  copyStr(p, item->getText(), _tcslen(item->getText()));
  p += DIM_TEXT;
  *p++ = 0;
}
//----------------------------------------------------------------------------
void manField::loadFromListbox(HWND hwlb)
{
  flush();
  int nElem = SendMessage(hwlb, LB_GETCOUNT, 0, 0);
  TCHAR buff[MAX_BUFF_ITEM];
  for(int i = 0; i < nElem; ++i) {
    SendMessage(hwlb, LB_GETTEXT, i, (LPARAM)buff);
    itemTrend* item = allocItemByBuff(buff);
    Field[i] = item;
    }
}
//----------------------------------------------------------------------------
void manField::fillLB(HWND hwlb)
{
  SendMessage(hwlb, LB_RESETCONTENT, 0, 0);
  int nElem = Field.getElem();
  TCHAR buff[MAX_BUFF_ITEM];
  for(int i = 0; i < nElem; ++i) {
    fillBuffByItem(buff, Field[i]);
    SendMessage(hwlb, LB_ADDSTRING, 0, (LPARAM)buff);
    }
}
//----------------------------------------------------------------------------
static void checkNewMode(const setOfString& set, LPCTSTR p, LPTSTR target, uint dim)
{
  if(!p)
    return;
  uint len = _tcslen(p);
  for(uint i = 0; i < len; ++i) {
    if(_T(',') == p[i])
      break;
    if(!_istdigit(p[i])) {
      _tcscpy_s(target, dim, p);
      return;
      }
    }
  DWORD id = _ttoi(p);
  if(id >= MIN_INIT_STD_MSG_TEXT) {
    LPCTSTR p2 = set.getString(id);
    if(p2) {
      for(uint i = 0; i < dim; ++i) {
        if(_T(',') == p2[i]) {
          target[i] = 0;
          break;
          }
        target[i] = p2[i];
        if(!p2[i])
          break;
        }
      }
    }
}
//----------------------------------------------------------------------------
void manField::load(const setOfString& set, int nData, uint id)
{
  flush();
  for(int i = 0; i < nData; ++i, ++id) {
    LPCTSTR p = set.getString(id);
    if(!p)
      break;
    int prph = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    int addr =  _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    int type =  _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    int norm =  _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    int dec =  _ttoi(p);
    p = findNextParamTrim(p);
    TCHAR t[4096] = _T("");
    checkNewMode(set, p, t, SIZE_A(t));
    int nbit = 0;
    double offs = 0;
    p = findNextParamTrim(p);
    if(p) {
      nbit = _ttoi(p);
      p = findNextParamTrim(p);
      if(p)
        offs = _tstof(p);
      }
    itemTrend* item = new itemTrend(prph, addr, type, norm, dec, nbit, offs, t);
    Field[i] = item;
    }
}
//----------------------------------------------------------------------------
bool manField::save(P_File& pfCrypt, P_File& pfClear, uint id)
{
  int nElem = Field.getElem();
  TCHAR buff[500];
  for(int i = 0; i < nElem; ++i, ++id) {
    uint idText = getGlobalStdMsgText();
    TCHAR t[64];
    if(PRPH_4_CONST == Field[i]->getPrph()) {
      _stprintf_s(t, _T("%0.5f"), Field[i]->getOffs());
      zeroTrim(t);
      }
    else
      wsprintf(t, _T("%d"), (int)(Field[i]->getOffs() + 0.5));

    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%s\r\n"), id,
        Field[i]->getPrph(),
        Field[i]->getAddr(),
        Field[i]->getType(),
        Field[i]->getNorm(),
        Field[i]->getDec(),
        idText,
        Field[i]->getNBit(),
        t
        );
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    wsprintf(buff, _T("%d,%s\r\n"), idText, Field[i]->getText());
    if(!writeStringChkUnicode(pfClear, pfCrypt, buff))
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
manTrend::manTrend() : addrTimer(0), History(false), trendType(0),
    savePrph(0),  saveAddr(0), saveType(0), saveNBit(0), saveOffs(0), resExport(0)
{
  Name[0] = 0;
  Prefix[0] = 0;
  Ext[0] = 0;
}
//----------------------------------------------------------------------------
int manTrend::diff(const manTrend* other)
{
  RET_CHECK(addrTimer)
  RET_CHECK(resExport)
  RET_CHECK(History)
  RET_CHECK(trendType)
  RET_CHECK(savePrph)
  RET_CHECK(saveAddr)
  RET_CHECK(saveType)
  RET_CHECK(saveNBit)
  RET_CHECK(saveOffs)
  RET_CHECK_STR(Name)
  RET_CHECK_STR(Prefix)
  RET_CHECK_STR(Ext)

  uint nElem = forName.getElem();
  if(other->forName.getElem() != nElem)
    return 1;
  for(uint i = 0; i < nElem; ++i)
    if(forName[i]->diff(other->forName[i]))
      return 1;

  if(Field.diff(other->Field))
    return 1;
  return 0;
/*
    TCHAR Name[MAX_NAME];
    int addrTimer;
    int resExport;
    bool History;
    int trendType;
    TCHAR Prefix[MAX_PREFIX];
    TCHAR Ext[MAX_EXT];
    int savePrph;
    int saveAddr;
    int saveType;
    int saveNBit;
    int saveOffs;

    manField Field;
    pvItemTrend forName;
*/
}
//----------------------------------------------------------------------------
bool manTrend::writeSimple(P_File& pf, HWND hwnd, uint idc, uint id)
{
  TCHAR buff[500];
  TCHAR t[20];
  GetWindowText(GetDlgItem(hwnd, idc), buff, SIZE_A(buff));
  if(!*buff)
    return true;
  wsprintf(t, _T("%d,"), id);
  if(!writeStringChkUnicode(pf, t))
    return false;
  if(!writeStringChkUnicode(pf, buff))
    return false;
  if(!writeStringChkUnicode(pf, _T("\r\n")))
    return false;
  return true;
}
//----------------------------------------------------------------------------
LPCTSTR manTrend::getStringForLB(LPTSTR buff)
{
  if(!*Name || !*Prefix || !*Ext)
    return 0;
  wsprintf(buff, _T("%s_?_%s, %s"), Prefix, Ext, Name);
  return buff;
}
//----------------------------------------------------------------------------
LPCTSTR manTrend::makeStringForLB(LPTSTR buff, size_t lenBuff, HWND hw)
{
  TCHAR t[100];
  if(!GetDlgItemText(hw, IDC_EDIT_T_PREFIX, t, SIZE_A(t)))
    return 0;
  wsprintf(buff, _T("%s_?_"), t);
  if(!GetDlgItemText(hw, IDC_EDIT_T_EXT, t, SIZE_A(t)))
    return 0;
  _tcscat_s(buff, lenBuff, t);
  _tcscat_s(buff, lenBuff, _T(", "));
  if(!GetDlgItemText(hw, IDC_EDIT_T_NAME, t, SIZE_A(t)))
    return 0;
  _tcscat_s(buff, lenBuff, t);
  return buff;
}
//----------------------------------------------------------------------------
void manTrend::fillDialog(HWND hwnd)
{
  SetDlgItemText(hwnd, IDC_EDIT_T_NAME, Name);
  SetDlgItemText(hwnd, IDC_EDIT_T_PREFIX, Prefix);
  SetDlgItemText(hwnd, IDC_EDIT_T_EXT, Ext);
  SetDlgItemInt(hwnd, IDC_EDIT_T_TIMER, addrTimer, false);
  SetDlgItemInt(hwnd, IDC_EDIT_T_RES_EXP, resExport, false);

  if(savePrph) {
    SetDlgItemInt(hwnd, IDC_EDIT_T_EN_PRF, savePrph, false);
    SetDlgItemInt(hwnd, IDC_EDIT_T_EN_ADDR, saveAddr, false);
    SetDlgItemInt(hwnd, IDC_EDIT_T_EN_TYPE, saveType, false);
    SetDlgItemInt(hwnd, IDC_EDIT_T_EN_NBIT, saveNBit, false);
    SetDlgItemInt(hwnd, IDC_EDIT_T_EN_OFFS, saveOffs, false);
    }
  SendMessage(GetDlgItem(hwnd, IDC_CHECK_T_ENABLE_SAVE), BM_SETCHECK,
        savePrph ? BST_CHECKED : BST_UNCHECKED, 0);

  SendMessage(GetDlgItem(hwnd, IDC_CHECK_T_HISTORY), BM_SETCHECK,
        History ? BST_CHECKED : BST_UNCHECKED, 0);

  SendMessage(GetDlgItem(hwnd, IDC_COMBO_T_TYPE), CB_SETCURSEL, trendType, 0);

  Field.fillLB(GetDlgItem(hwnd, IDC_LIST_FIELD));

  SendMessage(GetDlgItem(hwnd, IDC_LIST_T_TYPE), LB_RESETCONTENT, 0, 0);
  int nElem = forName.getElem();
  for(int i = 0; i < nElem; ++i) {
    TCHAR buff[100];
    wsprintf(buff, _T("%d,%d,%d,%d"),
        forName[i]->getPrph(),
        forName[i]->getAddr(),
        forName[i]->getType(),
        forName[i]->getDec()
        );
    SendMessage(GetDlgItem(hwnd, IDC_LIST_T_TYPE), LB_ADDSTRING, 0, (LPARAM)buff);
    }
}
//----------------------------------------------------------------------------
#define N_SZ(a) a, SIZE_A(a)
//----------------------------------------------------------------------------
void manTrend::loadFromDialog(HWND hwnd)
{
  GetDlgItemText(hwnd, IDC_EDIT_T_NAME, N_SZ(Name));
  GetDlgItemText(hwnd, IDC_EDIT_T_PREFIX, N_SZ(Prefix));
  GetDlgItemText(hwnd, IDC_EDIT_T_EXT, N_SZ(Ext));
  addrTimer = GetDlgItemInt(hwnd, IDC_EDIT_T_TIMER, 0, false);
  resExport = GetDlgItemInt(hwnd, IDC_EDIT_T_RES_EXP, 0, false);
  checkExt();
  bool checkSave = SendMessage(GetDlgItem(hwnd, IDC_CHECK_T_ENABLE_SAVE), BM_GETCHECK, 0, 0) ==
        BST_CHECKED;

  if(checkSave) {
    savePrph = GetDlgItemInt(hwnd, IDC_EDIT_T_EN_PRF, 0, false);
    saveAddr = GetDlgItemInt(hwnd, IDC_EDIT_T_EN_ADDR, 0, false);
    saveType = GetDlgItemInt(hwnd, IDC_EDIT_T_EN_TYPE, 0, false);
    saveNBit = GetDlgItemInt(hwnd, IDC_EDIT_T_EN_NBIT, 0, false);
    saveOffs = GetDlgItemInt(hwnd, IDC_EDIT_T_EN_OFFS, 0, false);
    }
  else {
    savePrph = 0;
    saveAddr = 0;
    saveType = 0;
    saveNBit = 0;
    saveOffs = 0;
    }

  History = SendMessage(GetDlgItem(hwnd, IDC_CHECK_T_HISTORY), BM_GETCHECK, 0, 0) == BST_CHECKED;

  trendType = SendMessage(GetDlgItem(hwnd, IDC_COMBO_T_TYPE), CB_GETCURSEL, 0, 0);

  Field.loadFromListbox(GetDlgItem(hwnd, IDC_LIST_FIELD));

  flushPV(forName);
  HWND hwLb = GetDlgItem(hwnd, IDC_LIST_T_TYPE);
  int nElem = SendMessage(hwLb, LB_GETCOUNT, 0, 0);
  for(int i = 0; i < nElem; ++i) {
    forName[i] = new itemTrend;
    TCHAR buff[100];
    SendMessage(hwLb, LB_GETTEXT, i, (LPARAM)buff);
    forName[i]->setPrph(_ttoi(buff));
    LPCTSTR p = findNextParam(buff, 1);
    forName[i]->setAddr(_ttoi(p));
    p = findNextParam(p, 1);
    forName[i]->setType(_ttoi(p));
    p = findNextParam(p, 1);
    forName[i]->setDec(_ttoi(p));
    }
}
//----------------------------------------------------------------------------
void manTrend::checkExt()
{
  if(*Ext && *Ext != _T('.')) {
    int len = _tcslen(Ext);
    for(int i = len + 1; i > 0; --i)
      Ext[i] = Ext[i - 1];
    Ext[0] = _T('.');
    }
}
//----------------------------------------------------------------------------
void manTrend::load(const setOfString& set, uint id, uint ix)
{
  LPCTSTR p = set.getString(id);
  if(!p)
    return;

// ind Timer Salvataggio, inizio dati, tipo trend, prefix, ext,cod2,nome del trend
//1001,20,1002,1,MH,.trd,1004,dati standard
  int initData = 0;
  int code2 = 0;
#if 1
  _stscanf_s(p, _T("%d,%d,%d"), &addrTimer, &initData, &trendType);
  p = findNextParam(p, 3);
  LPCTSTR p2 = findNextParam(p, 1);

  int len = p2 ? p2 - p - 1 : _tcslen(p);
  len = min((uint)len, SIZE_A(Prefix) - 1);
  copyStr(Prefix, p, len);
  Prefix[len] = 0;

  if(p2) {
    p = p2;
    p2 = findNextParam(p2, 1);
    len = p2 ? p2 - p - 1 : _tcslen(p);
    len = min((uint)len, SIZE_A(Ext) - 1);
    copyStr(Ext, p, len);
    Ext[len] = 0;
    if(p2)
      _stscanf_s(p2, _T("%d,%s"), &code2, Name, SIZE_A(Name));
    }
  if(!*Name)
    wsprintf(Name, _T("Trend_%d"), ix * OFFSET_TREND_CODE_BASE + 1 + INIT_TREND_DATA);
#else
  _stscanf(p, _T("%d,%d,%d,%s,%s,%d,%s"), &addrTimer, &initData, &trendType, Prefix,
            Ext, &code2, Name);
#endif

  checkExt();

  if(IX_NAMEBYCODE == trendType && code2) {
    p = set.getString(code2);
    int i = 0;
    while(p) {
      forName[i] = new itemTrend;
      forName[i]->setPrph(_ttoi(p));
      p = findNextParam(p, 1);
      if(!p)
        break;

      forName[i]->setAddr(_ttoi(p));
      p = findNextParam(p, 1);
      if(!p)
        break;

      forName[i]->setType(_ttoi(p));
      p = findNextParam(p, 1);
      if(!p)
        break;

      forName[i]->setDec(_ttoi(p));
      p = findNextParam(p, 1);
      ++i;
      }
    }
  p = set.getString(initData);
  if(!p)
    return;

//N° dati, indirizzo inizio dati, indirizzo per abilitazione salvataggio, risoluzione
//1002,15,10001,1003
  int nData = 0;
  _stscanf_s(p, _T("%d,%d,%d,%d"), &nData, &initData, &saveAddr, &resExport);

//idprf,addr,type,nbit,offset
//1003,2,0,1,1,5
  if(saveAddr) {
    p = set.getString(saveAddr);
    if(p)
      _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &savePrph, &saveAddr, &saveType, &saveNBit, &saveOffs);
    }

//firstCode,idprf,addr,dataType,idNorm,dec,testo per report
// 10001,2,30,3,0,2,testo
  Field.load(set, nData, initData);
}
//----------------------------------------------------------------------------
bool manTrend::save(P_File& pfCrypt, P_File& pfClear, uint idBase)
{
  uint id = idBase * OFFSET_TREND_CODE_BASE + 1 + INIT_TREND_DATA;
// ind Timer Salvataggio, inizio dati, tipo trend, prefix, ext,cod2,nome del trend
//1001,20,1002,1,MH,.trd,dati standard
  int code2 = 0;
  if(IX_NAMEBYCODE == trendType)
    code2 = id + 4;
  TCHAR buff[500];

  wsprintf(buff, _T("%d,%d,%d,%d,%s,%s,%d,%s\r\n"), id, addrTimer, id + 1, trendType, Prefix,
            Ext, code2, Name);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

  if(code2) {
    int nElem = forName.getElem();
    if(nElem) {
      wsprintf(buff, _T("%d"), code2);
      TCHAR t[50];
      for(int i = 0; i < nElem; ++i) {
        wsprintf(t, _T(",%d,%d,%d,%d"),
          forName[i]->getPrph(),
          forName[i]->getAddr(),
          forName[i]->getType(),
          forName[i]->getDec()
          );
        _tcscat_s(buff, t);
        }
      _tcscat_s(buff, _T("\r\n"));
      if(!writeStringChkUnicode(pfCrypt, buff))
        return false;
      }
    }

//N° dati, indirizzo inizio dati, indirizzo per abilitazione salvataggio
//1002,15,10001,1003
  int initData = idBase * OFFSET_TREND_CODE_DATA + 1 + BASE_TREND_CODE_DATA;
  wsprintf(buff, _T("%d,%d,%d,%d,%d\r\n"), id + 1, Field.getElem(),
              initData, savePrph ? id + 2 : 0, resExport);
  if(!writeStringChkUnicode(pfCrypt, buff))
    return false;

//idprf,addr,type,nbit,offset
//1003,2,0,1,1,5
  if(savePrph) {
    wsprintf(buff, _T("%d,%d,%d,%d,%d,%d\r\n"), id + 2,
            savePrph, saveAddr, saveType, saveNBit, saveOffs);
    if(!writeStringChkUnicode(pfCrypt, buff))
      return false;
    }
//firstCode,idprf,addr,dataType,idNorm,dec,testo per report
// 10001,2,30,3,0,2,testo
  return Field.save(pfCrypt, pfClear, initData);
}
